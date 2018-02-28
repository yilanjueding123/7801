/******************************************************
 * mjpeg_stream_wifi.c
 *
 * Purpose: Motion JPEG streaming server --- wifi part
 *
 * Author: Craig Yang
 *
 * Date: 2015/10/12
 *
 * Copyright Generalplus Corp. ALL RIGHTS RESERVED.
 *
 * Version :
 * History :
 *
 *******************************************************/
#include "tcp.h"
#include "api.h"	// for LWIP
#include "stdio.h"
#include "string.h"
#include "gplib.h"
#include "application.h"
#include "mjpeg_stream_wifi.h"
#include "ip.h"
#include "tcp.h"
//#include "state_wifi.h"

#define TCP_NO_DELAY 0

#if 1
#define printf  DBG_PRINT
#else
#define printf(...)
#endif

// =============================================================================
// Define
// =============================================================================
#define STREAM_REQ_HEADER	"GET /?action=stream"

typedef struct mjw_frame_info_s
{
	INT32U	addr;
	INT32U	size;
	INT8U	addr_idx;
	INT8U	not_used[3];
} mjw_frame_info_t;

// =============================================================================
// Static variables
// =============================================================================
static INT8U mjpeg_tx_buf[MJPEG_BUF_SIZE] = {0};
static INT8U mjpeg_rx_buf[MJPEG_BUF_SIZE] = {0};
static mjw_frame_info_t frame_info;

// =============================================================================
// External variables
// =============================================================================
// extern mjpeg_ctl_t mjpeg_ctl_blk;

// =============================================================================
// Note: GP version of netconn_write
// =============================================================================
#define WIFI_SEND_TIME_OUT 500
static err_t   gp_netconn_write(struct netconn *pxClient, INT32U wifi_addr, INT32U wifi_size, INT8U flag)
{
	INT32U retry_cnt = WIFI_SEND_TIME_OUT;
	INT32U wlen,tlen = wifi_size;
	INT8U *pBuf = (INT8U*)wifi_addr;
	err_t err = ERR_OK;
	INT16U available;
	// printf("tlen = %d\r\n",tlen);
	while(tlen)
	{
		available = pxClient->pcb.tcp->snd_buf;
		if (available)
		{
			if(tlen > available)
				wlen = available;
			else
				wlen = tlen;

			err = netconn_write(pxClient, pBuf, wlen, flag);
			if(err != ERR_OK)
			{
				printf("JPEG data error: netconn_write fail(err=%d)\r\n", err);
				break;
			}
			tlen -= wlen;
			pBuf += wlen;
			retry_cnt = WIFI_SEND_TIME_OUT;
		}
		else
		{
			OSTimeDly(1);
			retry_cnt--;
			if(!retry_cnt)
			{
				printf("JPEG data timeout,%d\r\n",retry_cnt);
				err = ERR_TIMEOUT;
				break;
			}
		}
	}

	return err;
}

// =============================================================================
// Note: Create network connection and listen
// =============================================================================
INT32S mjpeg_wifi_start_server(mjpeg_ctl_t *ctl_blk)
{
	err_t lwip_err;
	INT32S ret = MJW_RET_FAIL;

	/* Create a new tcp connection handle */
	ctl_blk->mjpeg_server = netconn_new(NETCONN_TCP);
	if (ctl_blk->mjpeg_server != NULL)
	{
		printf("mjpeg_stream_task: mjpeg server connection created(%x)\r\n", ctl_blk->mjpeg_server);
	}
	else
	{
		printf("mjpeg_stream_task: mjpeg server connection creation fail\r\n");
		goto START_SERVER_EXIT;
	}

   #if TCP_NO_DELAY	
  /* Disable Nagle's algorithm */
  tcp_nagle_disable(ctl_blk->mjpeg_server->pcb.tcp);
  if (tcp_nagle_disabled(ctl_blk->mjpeg_server->pcb.tcp))
  {
		printf("mjpeg_stream_task: mjpeg server disable Nagle's algorithm\r\n");
	}
  #endif	
  
	/* Bind the tcp connection to specific port */
	lwip_err = netconn_bind(ctl_blk->mjpeg_server, IPADDR_ANY, MJPEG_SERVER_PORT);
	if (lwip_err == ERR_OK)
	{
		printf("mjpeg_stream_task: mjpeg server bind to port %d\r\n", MJPEG_SERVER_PORT);
	}
	else
	{
		printf("mjpeg_stream_task: mjpeg server bind fail (err=%d)\r\n", lwip_err);
		goto START_SERVER_EXIT;
	}

	/* Listen for incoming connection */
	lwip_err = netconn_listen(ctl_blk->mjpeg_server);
	if (lwip_err == ERR_OK)
	{
		ret = MJW_RET_SUCCESS;
		printf("mjpeg_stream_task: mjpeg server listen ok\r\n");
	}
	else
	{
		printf("mjpeg_stream_task: mjpeg server listen fail (err=%d)\r\n", lwip_err);
		goto START_SERVER_EXIT;
	}

	netconn_set_recvtimeout(ctl_blk->mjpeg_server, 100);  //  time out for accpet

START_SERVER_EXIT:
	if (ret != MJW_RET_SUCCESS)
		mjpeg_wifi_del_server_conn(ctl_blk);

	return ret;
}

// =============================================================================
// Note: Wait for a new connection
// =============================================================================
/* keepalive timeout = TCP_KEEPIDLE_DEFAULT + TCP_KEEPINTVL_DEFAULT * TCP_KEEPCNT_DEFAULT, RFC.1122 */
#define GP_SOCK_TCP_KEEPALIVE_IDLE	3000	/* 3000ms */
#define GP_SOCK_TCP_KEEPALIVE_INTVL	5000	/* 5000ms */
#define GP_SOCK_TCP_KEEPALIVE_CNT	1		/* 1 time */

INT32S mjpeg_wifi_wait_client_conn(mjpeg_ctl_t *ctl_blk)
{
	err_t lwip_err;
	INT32S ret;

	ctl_blk->mjpeg_client = NULL;

	lwip_err = netconn_accept(ctl_blk->mjpeg_server, &ctl_blk->mjpeg_client);
	if (lwip_err != ERR_OK)
	{
		if (lwip_err != ERR_TIMEOUT)
		{
			printf("netconn_accept mjpeg_server error(%d) time(%d)\r\n", lwip_err, ctl_blk->mjpeg_server->recv_timeout);
		}
	}
	if (lwip_err==ERR_OK)
	{
		ctl_blk->mjpeg_client->pcb.tcp->so_options |= (INT8U)SOF_KEEPALIVE;
		ctl_blk->mjpeg_client->pcb.tcp->keep_idle = GP_SOCK_TCP_KEEPALIVE_IDLE;
		ctl_blk->mjpeg_client->pcb.tcp->keep_intvl = GP_SOCK_TCP_KEEPALIVE_INTVL;
		ctl_blk->mjpeg_client->pcb.tcp->keep_cnt = GP_SOCK_TCP_KEEPALIVE_CNT;
	}

	ret = (lwip_err == ERR_OK) ? MJW_RET_SUCCESS : MJW_RET_FAIL;
	return ret;
}

// =============================================================================
// Note: Allocate buffer for receiving wifi data
// =============================================================================
INT32S mjpeg_wifi_alloc_rx_buff(mjpeg_ctl_t *ctl_blk)
{
	INT32S ret = MJW_RET_SUCCESS;
	err_t err;

	/* Get net buffer */
	err = netconn_recv(ctl_blk->mjpeg_client, &ctl_blk->mjpeg_rx_buf);
	if(err != ERR_OK)
	{
		printf("pxRxBuffer = NULL\r\n");
		ret = MJW_RET_FAIL;
	}

	return ret;
}

// =============================================================================
// Note: Free buffer for receiving wifi data
// =============================================================================
void mjpeg_wifi_free_rx_buff(mjpeg_ctl_t *ctl_blk)
{
	netbuf_delete(ctl_blk->mjpeg_rx_buf);
}

// =============================================================================
// Note: It's a streaming request if STREAM_REQ_HEADER can be detected
// =============================================================================
INT32S mjpeg_wifi_detect_stream_req(mjpeg_ctl_t *ctl_blk)
{
	INT8U* rxstring;
	INT32S ret = MJW_RET_FAIL;
	INT16U len = 0;

	/* Where is the data? */
	netbuf_data(ctl_blk->mjpeg_rx_buf, (void*)&rxstring, &len);
	if(len)
	{
		memset(mjpeg_tx_buf, 0, sizeof(mjpeg_tx_buf));
		memset(mjpeg_rx_buf, 0, sizeof(mjpeg_rx_buf));

		if(len > MJPEG_BUF_SIZE)
			len = MJPEG_BUF_SIZE;

		strncpy((char*)mjpeg_rx_buf, (char*)rxstring, len);
		printf("rxstring[%d]: \r\n%s\r\n", len, mjpeg_rx_buf);

		if(strstr((char*)rxstring, STREAM_REQ_HEADER) != NULL)
			ret = MJW_RET_SUCCESS;
		else
			printf("Canot find '%s' in HTML header!\r\n", STREAM_REQ_HEADER);
	}

	return ret;
}

// =============================================================================
// Note: Write streaming header to let app know how we will stream the following images
// =============================================================================
INT32S mjpeg_wifi_write_stream_header(mjpeg_ctl_t *ctl_blk)
{
	err_t err;
	INT32S ret;
	struct netconn *pxClient = ctl_blk->mjpeg_client;

	memset(mjpeg_tx_buf, 0, sizeof(mjpeg_tx_buf));
	sprintf((char*)mjpeg_tx_buf, "HTTP/1.0 200 OK\r\n" \
	        STD_HEADER \
	        "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
	        "\r\n" \
	        "--" BOUNDARY "\r\n");

	/* Write header back */
	err = gp_netconn_write(pxClient, (INT32U)mjpeg_tx_buf, (INT32U) strlen((char*)mjpeg_tx_buf), NETCONN_COPY);
	if(err != ERR_OK)
		printf("\r\nSend MIME header error %d\r\n", err);

	ret = (err == ERR_OK) ? MJW_RET_SUCCESS : MJW_RET_FAIL;
	return ret;
}

// =============================================================================
// Note: Get tick for the timestamp of a frame
// =============================================================================
static INT32U get_curr_tick(void)
{
	static INT32U first_frame_tick = 0;
	static INT32U last_frame_tick  = 0;	
	INT32U tick;

	tick = OSTimeGet();
	if (tick>=last_frame_tick)
	{
		DBG_PRINT("*");
		first_frame_tick = tick;
		last_frame_tick = tick + 100;	// 100tick = 100 * 10ms = 1 second
	}

	return tick;
}

// =============================================================================
// Note: Write frame start
// =============================================================================
void mjpeg_wifi_set_write_frame_info(mjpeg_write_data_t *info)
{
	frame_info.addr = info->mjpeg_addr;
	frame_info.size = info->mjpeg_size;
	frame_info.addr_idx = info->mjpeg_addr_idx;
}

// =============================================================================
// Note: Write frame start
// =============================================================================
INT32S mjpeg_wifi_write_frame_start(mjpeg_ctl_t *ctl_blk)
{
	err_t err;
	INT32S ret;
	INT32U tick;

	tick = get_curr_tick();
	#if 0
	sprintf((char*)mjpeg_tx_buf, "Content-Type: image/jpeg\r\n" \
	        "Content-Length: %d\r\n" \
	        "X-Timestamp: %d.%02d\r\n" \
	        "\r\n", frame_info.size, (INT32U)(tick/100), (INT32U)(tick%100));
	#else	// remove divide operator  josephhsieh@20160202
	sprintf((char*)mjpeg_tx_buf, "Content-Type: image/jpeg\r\n" \
	        "Content-Length: %d\r\n" \
	        "\r\n", frame_info.size);
	#endif

	err = gp_netconn_write(ctl_blk->mjpeg_client, (INT32U)mjpeg_tx_buf, (INT32U) strlen((char*)mjpeg_tx_buf), NETCONN_COPY);
	if(err != ERR_OK)
		printf("\r\nSend JPEG header error %d\r\n", err);

	ret = (err == ERR_OK) ? MJW_RET_SUCCESS : MJW_RET_FAIL;
	return ret;
}

// =============================================================================
// Note: Write frame data
// =============================================================================
INT32S mjpeg_wifi_write_frame_data(mjpeg_ctl_t *ctl_blk)
{
	err_t err;
	INT32S ret;

	err = gp_netconn_write(ctl_blk->mjpeg_client, frame_info.addr, frame_info.size, NETCONN_COPY);
	if(err != ERR_OK)
		printf("\r\nSend JPEG error %d\r\n", err);

	ret = (err == ERR_OK) ? MJW_RET_SUCCESS : MJW_RET_FAIL;
	return ret;
}

// =============================================================================
// Note: Write frame end
// =============================================================================
INT32S mjpeg_wifi_write_frame_end(mjpeg_ctl_t *ctl_blk)
{
	err_t err;
	INT32S ret;

	sprintf((char*)mjpeg_tx_buf, "\r\n--" BOUNDARY "\r\n");
	err = gp_netconn_write(ctl_blk->mjpeg_client, (INT32U)mjpeg_tx_buf, (INT32U) strlen((char*)mjpeg_tx_buf), NETCONN_COPY);
	if(err != ERR_OK)
		printf("*ERROR* Send %u end of JPEG error %d!!\r\n", frame_info.addr_idx, err);

	ret = (err == ERR_OK) ? MJW_RET_SUCCESS : MJW_RET_FAIL;
	return ret;
}

// =============================================================================
// Note: Close a client connection
// =============================================================================
void mjpeg_wifi_close_client_conn(mjpeg_ctl_t *ctl_blk)
{
	netconn_close(ctl_blk->mjpeg_client);
}

// =============================================================================
// Note: Delete a client connection when the client is disconnected
// =============================================================================
INT32S mjpeg_wifi_del_client_conn(mjpeg_ctl_t *ctl_blk)
{
	err_t lwip_err;
	INT32S ret;

	lwip_err = netconn_delete(ctl_blk->mjpeg_client);
	ret = (lwip_err == ERR_OK) ? MJW_RET_SUCCESS : MJW_RET_FAIL;

	return ret;
}

// =============================================================================
// Note: Close the server connection
// =============================================================================
void mjpeg_wifi_close_server_conn(mjpeg_ctl_t *ctl_blk)
{
	// What is this for ??
	sys_mbox_trypost(&(ctl_blk->mjpeg_server->acceptmbox), NULL);
	printf("mjpeg_wifi_close_server_conn\r\n");
}

// =============================================================================
// Note: Delete a server connection
// =============================================================================
INT32S mjpeg_wifi_del_server_conn(mjpeg_ctl_t *ctl_blk)
{
	err_t lwip_err;
	INT32S ret;

	lwip_err = netconn_delete(ctl_blk->mjpeg_server);
	if (lwip_err == ERR_OK)
	{
		ret = MJW_RET_SUCCESS;
		ctl_blk->mjpeg_server = NULL;
	}
	else
	{
		ret = MJW_RET_FAIL;
	}

	return ret;
}

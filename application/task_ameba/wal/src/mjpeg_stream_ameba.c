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
//#include "tcp.h"
//#include "api.h"	// for LWIP
#include "stdio.h"
#include "string.h"
#include "gplib.h"
#include "application.h"
#include "gspi_cmd.h"
#include "gp_cmd.h"
#include "mjpeg_stream_ameba.h"

#if 1
#define printf  DBG_PRINT
#else
#define printf(...)
#endif

// =============================================================================
// Define
// =============================================================================
typedef struct mjw_frame_info_s
{
	INT32U	addr;
	INT32U	size;
	INT8U	addr_idx;
	INT8U	not_used[3];
} mjw_frame_info_t;

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Static variables
// =============================================================================
static mjw_frame_info_t frame_info;
static mjpeg_ctl_t *mjpeg_ctl_blk_ptr;
static BOOLEAN is_first_ameba_mjs_enable = TRUE;
volatile static INT32U mjpeg_req_len = 0;
volatile static INT32U mjpeg_gspi_tx_addr = 0;	// Starting address of the next part of JPG to send to Ameba
volatile static INT32U mjpeg_gspi_tx_len = 0;	// Length of the next part of JPG to send to Ameba
volatile static BOOLEAN is_client_connected = FALSE;
volatile static BOOLEAN is_get_data_req = FALSE;	// TRUE if GSPI_MJPEG_GET_DATA_EVENT is received
volatile static BOOLEAN is_jpeg_data_pending_to_send = FALSE;	// TRUE if a new JPEG is ready to send
																// FALSE if a JPEG is finished sending

// =============================================================================
// External variables
// =============================================================================
extern GP_NET_APP_T gp_net_app_state;
																
// =============================================================================
// External functions
// =============================================================================

// =============================================================================
// Note: 
// =============================================================================
static void mjpeg_gspi_cmd_cbk(INT32U buf, INT32U len, INT32U event)
{
	if(event == GSPI_MJPEG_GET_DATA_EVENT)
	{
		mjpeg_req_len = len;
		is_get_data_req = TRUE;
	}
	else if (event == GSPI_MJPEG_START_EVENT)
	{
		is_client_connected = TRUE;
	}
	else if (event == GSPI_MJPEG_STOP_EVENT)
	{
		is_client_connected = FALSE;
		OSQPost(mjpeg_ctl_blk_ptr->mjpeg_frame_q, (void*)MJPEG_STOP_EVENT);
		DBG_PRINT("[%s]: send GSPI_MJPEG_STOP_EVENT to MJS\r\n", __func__);
	}
}	

// =============================================================================
// Note:
// =============================================================================
static void gspi_send_current_jpeg_data(void)
{
	INT32U transfer_len;
	
	if(mjpeg_gspi_tx_len != 0)
	{	
		transfer_len = mjpeg_req_len;
		if(transfer_len > mjpeg_gspi_tx_len)
		{
			transfer_len = mjpeg_gspi_tx_len;
		}
		
		//DBG_PRINT("send: txbuf 0x%x , len %d\r\n", mjpeg_gspi_tx_addr, transfer_len);

		gspi_transfer_mjpeg((INT8U*)mjpeg_gspi_tx_addr, transfer_len);

		mjpeg_gspi_tx_addr += transfer_len;
		mjpeg_gspi_tx_len -= transfer_len;
		
		if(mjpeg_gspi_tx_len == 0)
		{
			mjpeg_gspi_tx_addr = 0;
			is_jpeg_data_pending_to_send = FALSE;
			//DBG_PRINT(" EOF buf 0x%x len %d\r\n", frame_info.addr, frame_info.size);
		}	
		//DBG_PRINT(" tx[%d] ", mjpeg_gspi_tx_len);		
	}
}	

// =============================================================================
// Note:
// =============================================================================
static INT32S enable_ameba_mjpeg_streamer(void)
{
	INT32U cnt = 10;
	INT32S ret = 0;

	// Enable once only.
	if (!is_first_ameba_mjs_enable)
		return 0;

	is_first_ameba_mjs_enable = FALSE;
	
	/* Config GP socket command port */
	gspi_send_docmd(GP_CMD_CONFIG_MJPEG_STREAMER_PORT);
	gp_net_app_state.isupdated = 0;
	gspi_send_docmd(GP_CMD_ENABLE_MJPEG_STREAMER);
	cnt = 10;
	while((gp_net_app_state.isupdated == 0) && cnt)
	{
		cnt--;
		OSTimeDly(200);
	}

	if(!gp_net_app_state.mjpeg_streamer_state)
	{
		ret = (-1);
		DBG_PRINT("Enale Ameba MJPEG streamer failed. cnt=%u\r\n", cnt);
	}
	else
	{
		DBG_PRINT("Enale Ameba MJPEG streamer success\r\n");
	}

	return ret;
}

// =============================================================================
// Note:
// =============================================================================
INT32S mjpeg_wifi_start_server(mjpeg_ctl_t *ctl_blk)
{
	INT32S ret;

	/* Init GSPI command module */
	gspi_cmd_init();

	// mjpeg_gspi_cmd_cbk needs to know ctl_blk in order to send message to mjpeg streamer
	mjpeg_ctl_blk_ptr = ctl_blk;
	gspi_register_mjpeg_cmd_cbk((INT32U)mjpeg_gspi_cmd_cbk);

	ret = enable_ameba_mjpeg_streamer();

	return (ret == 0) ? MJW_RET_SUCCESS : MJW_RET_FAIL;
}

// =============================================================================
// Note: Wait for a new connection
// =============================================================================
INT32S mjpeg_wifi_wait_client_conn(mjpeg_ctl_t *ctl_blk)
{
	// Wait until Ameba tells us that a new client is connected
	DBG_PRINT("Waiting for mjs client conn\r\n");
	while (!is_client_connected)
	{
		OSTimeDly(50);
	}
	
	return MJW_RET_SUCCESS;
}

// =============================================================================
// Note: Allocate buffer for receiving wifi data
// =============================================================================
INT32S mjpeg_wifi_alloc_rx_buff(mjpeg_ctl_t *ctl_blk)
{
	// Ameba does not need to allocate RX buffer
	return MJW_RET_SUCCESS;
}

// =============================================================================
// Note: Free buffer for receiving wifi data
// =============================================================================
void mjpeg_wifi_free_rx_buff(mjpeg_ctl_t *ctl_blk)
{
	// Ameba does not need to free RX buffer
	return;
}

// =============================================================================
// Note: It's a streaming request if STREAM_REQ_HEADER can be detected
// =============================================================================
INT32S mjpeg_wifi_detect_stream_req(mjpeg_ctl_t *ctl_blk)
{
	// Ameba performs the check. MCU does not need to do it.
	return MJW_RET_SUCCESS;
}

// =============================================================================
// Note: Write streaming header to let app know how we will stream the following images
// =============================================================================
INT32S mjpeg_wifi_write_stream_header(mjpeg_ctl_t *ctl_blk)
{
	// Ameba writes stream header. MCU does not need to do it.
	return MJW_RET_SUCCESS;
}

// =============================================================================
// Note: Write frame start
// =============================================================================
void mjpeg_wifi_set_write_frame_info(mjpeg_write_data_t *info)
{
	frame_info.addr = info->mjpeg_addr;
	frame_info.size = info->mjpeg_size;
	frame_info.addr_idx = info->mjpeg_addr_idx;

	mjpeg_gspi_tx_addr = info->mjpeg_addr;
	mjpeg_gspi_tx_len = info->mjpeg_size;

	is_jpeg_data_pending_to_send = TRUE;
	
	//DBG_PRINT("Frame %u: addr(%x), len(%u)\r\n", frame_info.addr_idx, frame_info.addr, frame_info.size);
}

// =============================================================================
// Note: Write frame start
// =============================================================================
INT32S mjpeg_wifi_write_frame_start(mjpeg_ctl_t *ctl_blk)
{
	/* Send MJPEG header to client and triggle slave to get JPEG data from host */
	gspi_tx_cmd(GSPI_TX_MJPEG_NET_HEADER_CMD, frame_info.size);

	return MJW_RET_SUCCESS;
}

// =============================================================================
// Note: Write frame data
// =============================================================================
INT32S mjpeg_wifi_write_frame_data(mjpeg_ctl_t *ctl_blk)
{
	while (1)
	{
		// Stop sending data if there is not more JPEG data or the client is disconnected
		if (!is_jpeg_data_pending_to_send || !is_client_connected)
			break;

		// Wait for GSPI_MJPEG_GET_DATA_EVENT
		if (!is_get_data_req)
		{
			// Remove the delay to reduce the data spent on WiFi sending.
			//OSTimeDly(1);
			continue;
		}

		is_get_data_req = FALSE;
		gspi_send_current_jpeg_data();

		if (!is_jpeg_data_pending_to_send)
			break;
	}

	return MJW_RET_SUCCESS;
}

// =============================================================================
// Note: Write frame end
// =============================================================================
INT32S mjpeg_wifi_write_frame_end(mjpeg_ctl_t *ctl_blk)
{
	// Ameba writes frame end. MCU does not need to do it.
	return MJW_RET_SUCCESS;
}

// =============================================================================
// Note: Close a client connection
// =============================================================================
void mjpeg_wifi_close_client_conn(mjpeg_ctl_t *ctl_blk)
{
	// Ameba handles client connection. MCU does not need to do it.
	return;
}

// =============================================================================
// Note: Delete a client connection when the client is disconnected
// =============================================================================
INT32S mjpeg_wifi_del_client_conn(mjpeg_ctl_t *ctl_blk)
{
	// Ameba handles client connection. MCU does not need to do it.
	return MJW_RET_SUCCESS;
}

// =============================================================================
// Note: Close the server connection
// =============================================================================
void mjpeg_wifi_close_server_conn(mjpeg_ctl_t *ctl_blk)
{
	is_get_data_req = FALSE;
	is_client_connected = FALSE;
	is_jpeg_data_pending_to_send = FALSE;

	return;
}

// =============================================================================
// Note: Delete a server connection
// =============================================================================
INT32S mjpeg_wifi_del_server_conn(mjpeg_ctl_t *ctl_blk)
{
	// Ameba handles server connection. MCU does not need to do it.
	return MJW_RET_SUCCESS;
}

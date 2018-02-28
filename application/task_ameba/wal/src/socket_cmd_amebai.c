/******************************************************
 * socket_cmd_wifi.c
 *
 * Purpose: Accept commands via a TCP socket --- wifi part
 *
 * Author: Craig Yang
 *
 * Date: 2015/10/20
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
#include "socket_cmd_ameba.h"

#if 1
#define printf  DBG_PRINT
#else
#define printf(...)
#endif

// =============================================================================
// Define
// =============================================================================
#define	GP_CMD_TAG_SIZE				8
#define	GP_CMD_TYPE_SIZE			2
#define	GP_CMD_MODE_SIZE			1
#define	GP_CMD_ID_SIZE				1
#define	GP_CMD_PAYLAOD_SIZE			2
#define GP_CMD_HEADER_SIZE			(GP_CMD_TAG_SIZE+GP_CMD_TYPE_SIZE+GP_CMD_MODE_SIZE+GP_CMD_ID_SIZE+GP_CMD_PAYLAOD_SIZE)
#define GP_CMD_PAYLOAD_BUF_SIZE		256
#define GP_CMD_CMD_BUF_SIZE			(GP_CMD_HEADER_SIZE + GP_CMD_PAYLOAD_BUF_SIZE)

// =============================================================================
// Static variables
// =============================================================================
static BOOLEAN is_first_ameba_gp_socket_enable = TRUE;
static INT8U gp_sock_rx_buf[GP_CMD_CMD_BUF_SIZE];
static INT32U gp_sock_rx_buf_len = 0;

volatile static BOOLEAN is_client_connected = FALSE;
volatile static BOOLEAN is_recv_new_gp_socket_cmd = FALSE;

// =============================================================================
// External variables
// =============================================================================
extern GP_NET_APP_T gp_net_app_state;

// =============================================================================
// Note:
// =============================================================================
static void gp_socket_cmd_gspi_cbk(INT8U* buf, INT32U len, INT32U event)
{
	INT32U size = len;
	
	//DBG_PRINT("gp_socket_cmd_gspi_cbk len %d, event 0x%x\r\n", len , event);
	
	if(event == GP_SOCK_RXDONE_EVENT)
	{
		if(len)
		{
			if(size > GP_CMD_CMD_BUF_SIZE)
				size = GP_CMD_CMD_BUF_SIZE;
			
			is_recv_new_gp_socket_cmd = TRUE;

			gp_sock_rx_buf_len = size;
			gp_memcpy((INT8S*)gp_sock_rx_buf, (INT8S*)buf, gp_sock_rx_buf_len);
		}
	}
	else if (event == GP_SOCK_CLI_CONN_EVENT)
	{
		/* Client connected from GP socket */
		is_client_connected = TRUE;
		DBG_PRINT("Client connected to GP socket\r\n");
	}
	else if (event == GP_SOCK_CLI_DISC_EVENT)
	{
		/* Client disconnected from GP socket */
		is_client_connected = FALSE;
		DBG_PRINT("Client disconnected from GP socket\r\n");
	}
}

// =============================================================================
// Note:
// =============================================================================
static INT32S enable_ameba_gp_socket(void)
{
	INT32U cnt;
	INT32S ret = 0;

	// Enable once only.
	if (!is_first_ameba_gp_socket_enable)
		return 0;

	is_first_ameba_gp_socket_enable = FALSE;

	/* Config GP socket command port */
	gspi_send_docmd(GP_CMD_CONFIG_GP_SOCKET_PORT);
	gp_net_app_state.isupdated = 0;
	gspi_send_docmd(GP_CMD_ENABLE_GP_SOCKET);
	cnt = 10;
	while((gp_net_app_state.isupdated == 0) && cnt)
	{
		cnt--;
		OSTimeDly(200);
	}

	if(!gp_net_app_state.gp_socket_state)
	{
		ret = (-1);
		DBG_PRINT("Enale GP socket service failed\r\n");
	}
	else
	{
		DBG_PRINT("Enale GP socket service success\r\n");
	}

	return ret;
}

// =============================================================================
// Note: Create network connection and listen
// =============================================================================
INT32S socket_cmd_wifi_start_server(socket_cmd_ctl_t *ctl_blk)
{
	INT32S ret;

	/* Init GSPI command module */
	gspi_cmd_init();

	/* Register call back function for GSPI RX data */
	gspi_register_gp_sock_cmd_cbk((INT32U)gp_socket_cmd_gspi_cbk);

	ret = enable_ameba_gp_socket();

	return (ret == 0) ? SC_RET_SUCCESS : SC_RET_FAIL;
}

// =============================================================================
// Note: Wait for a new connection
// =============================================================================
INT32S socket_cmd_wifi_wait_client_conn(socket_cmd_ctl_t *ctl_blk)
{
	// Wait until Ameba tells us that a new client is connected
	DBG_PRINT("Waiting for gp socket cmd client conn\r\n");
	while (!is_client_connected)
	{
		OSTimeDly(50);
	}

	return SC_RET_SUCCESS;
}

// =============================================================================
// Note: Wait until new data from client is received. The received data will
//       be placed in 'buf'.
// =============================================================================
INT32S socket_cmd_wifi_recv_data(socket_cmd_ctl_t *ctl_blk, INT8U *buf, INT32U buf_size)
{
	// [TODO] Need a timeout scheme
	// Waiting for GP_SOCK_RXDONE_EVENT from Ameba
	//DBG_PRINT("socket_cmd_wifi_recv_data: buf=%x, gp_sock_rx_buf_ptr = %x\r\n", buf, gp_sock_rx_buf_ptr);
	while (!is_recv_new_gp_socket_cmd && is_client_connected)
	{
		OSTimeDly(10);
	}

	// Copy gp_sock_rx_buf_len rather than 'buf_size' because the size is
	// decided by GP_SOCK_RXDONE_EVENT.
	// [TODO] Cmd data is copied from gspi->gp_sock_rx_buf->buf. It does not seem efficient.
	gp_memcpy((INT8S*)buf, (INT8S*)gp_sock_rx_buf, gp_sock_rx_buf_len);

	if (is_recv_new_gp_socket_cmd)
		is_recv_new_gp_socket_cmd = FALSE;

	return SC_RET_SUCCESS;
}

// =============================================================================
// Note: Check if there is any new data to handle
// =============================================================================
INT32S socket_cmd_wifi_check_new_data(socket_cmd_ctl_t *ctl_blk)
{
	// [TODO] For raw data download which is not supported in Ameba yet.
	return SC_RET_FAIL;
}

// =============================================================================
// Note: It's a streaming request if STREAM_REQ_HEADER can be detected
// =============================================================================
INT32S socket_cmd_wifi_send_data(socket_cmd_ctl_t *ctl_blk, INT8U *tx_buf, INT16U len)
{
	// [TODO] Thumbnail, raw data, and 'get parameter file' are not supported yet.
	// For sending ACK/NACK only.
	gspi_tx_data(tx_buf, len, DATA_GP_SOCK_CMD_TYPE);
	return SC_RET_SUCCESS;
}

// =============================================================================
// Note: Return the size of client's available tx buffer so that we know how much data to send
// =============================================================================
INT32U socket_cmd_wifi_get_client_avail_tx_buf_size(socket_cmd_ctl_t *ctl_blk)
{
	// [TODO] For raw data download which is not supported in Ameba yet.
	return 0;
}

// =============================================================================
// Note: Close the client connection
// =============================================================================
void socket_cmd_wifi_close_client_conn(socket_cmd_ctl_t *ctl_blk)
{
	// Ameba handles client connection. MCU does not need to do it.
	return;
}

// =============================================================================
// Note: Delete a client connection when the client is disconnected
// =============================================================================
INT32S socket_cmd_wifi_del_client_conn(socket_cmd_ctl_t *ctl_blk)
{
	// Ameba handles client connection. MCU does not need to do it.
	return SC_RET_SUCCESS;
}

// =============================================================================
// Note: Close the server connection
// =============================================================================
void socket_cmd_wifi_close_server_conn(socket_cmd_ctl_t *ctl_blk)
{
	is_client_connected = FALSE;
	is_recv_new_gp_socket_cmd = FALSE;

	return;
}

// =============================================================================
// Note: Delete a server connection
// =============================================================================
INT32S socket_cmd_wifi_del_server_conn(socket_cmd_ctl_t *ctl_blk)
{
	// Ameba handles client connection. MCU does not need to do it.
	return SC_RET_SUCCESS;
}

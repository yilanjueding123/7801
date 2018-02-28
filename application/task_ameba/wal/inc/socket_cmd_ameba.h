/******************************************************
* socket_cmd_ameba.h
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
#ifndef __SOCKET_CMD_AMEBA_H__
#define __SOCKET_CMD_AMEBA_H__
#include "socket_cmd.h"

// Return code of socket cmd functions
#define SC_RET_SUCCESS		0
#define SC_RET_FAIL			(-1)
#define SC_RET_NO_DATA		(-2)
#define SC_RET_RECV_TIMEOUT	(-3)

// Public APIs
extern INT32S socket_cmd_wifi_start_server(socket_cmd_ctl_t *ctl_blk);
extern INT32S socket_cmd_wifi_wait_client_conn(socket_cmd_ctl_t *ctl_blk);
extern INT32S socket_cmd_wifi_recv_data(socket_cmd_ctl_t *ctl_blk, INT8U *buf, INT32U buf_size);
extern INT32S socket_cmd_wifi_check_new_data(socket_cmd_ctl_t *ctl_blk);
extern INT32S socket_cmd_wifi_send_data(socket_cmd_ctl_t *ctl_blk, INT8U *tx_buf, INT16U len);
extern INT32U socket_cmd_wifi_get_client_avail_tx_buf_size(socket_cmd_ctl_t *ctl_blk);
extern void socket_cmd_wifi_close_client_conn(socket_cmd_ctl_t *ctl_blk);
extern INT32S socket_cmd_wifi_del_client_conn(socket_cmd_ctl_t *ctl_blk);
extern void socket_cmd_wifi_close_server_conn(socket_cmd_ctl_t *ctl_blk);
extern INT32S socket_cmd_wifi_del_server_conn(socket_cmd_ctl_t *ctl_blk);

#endif	//__SOCKET_CMD_AMEBA_H__

/***************************************************************************
* gspi_cmd.h
*
* Purpose: Header file for GSPI command
*
* Author: Eugene Hsu
*
* Date: 2015/10/01
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
******************************************************************************/
#ifndef __GSPI_CMD_H__
#define __GSPI_CMD_H__

#define GSPI_MASTER_Q_NUM	16
#define GSPI_STACK_SIZE		1024
#define GSPI_DOCMD_SIZE		128
#define GSPI_DATA_TRANSFER_LEN	(2*1024)
#define GSPI_RX_BUF_SIZE      	(20*1024)

typedef void(*GSPI_GP_SOCK_CMD_CBK)(INT8U* buf, INT32U len, INT32U event);
typedef void(*GSPI_MJPEG_STREAMER_CBK)(INT32U buf, INT32U len, INT32U event);
typedef void(*GSPI_GP_CMD_CBK)(INT32U buf, INT32U len, INT32U event);

typedef enum GSPI_STATE_E
{
	GSPI_IDLE_STATE = 0,
	GSPI_TX_DATA_STATE,
	GSPI_RX_DATA_STATE
} GSPI_STATE_T;

typedef enum GSPI_EVENT_E
{
	GSPI_MJPEG_TX_EVENT = 1,
	GSPI_DOCMD_TX_EVENT,
	GSPI_GP_SOCK_CMD_TX_EVENT,
	GSPI_RX_EVENT,
	GSPI_MAX_EVENT
} GSPI_EVENT_T;

typedef enum GP_SOCK_EVENT_E
{
	GP_SOCK_TXDONE_EVENT,
	GP_SOCK_RXDONE_EVENT,
	GP_SOCK_CLI_CONN_EVENT,
	GP_SOCK_CLI_DISC_EVENT,
	GP_SOCK_RAW_DATA_DONE_EVENT,
	GP_SOCK_RAW_DATA_SEND_BACK_EVENT,
	GP_SOCK_RAW_DATA_SEND_BACK_DONE_EVENT
} GP_SOCK_EVENT_T;

/* This definition must be same in GSPI master and slave */
#define GSPI_CMD_START	0x100
typedef enum
{
	/* GSPI TX */
	GSPI_TX_MJPEG_CMD = GSPI_CMD_START,
	GSPI_TX_DOCMD_CMD,
	GSPI_TX_GP_SOCKET_CMD,				/* GSPI send data to GP socket command for ACK or data */
	GSPI_GET_GP_SOCKET_RAW_CMD,			/* GSPI command to get GP socket raw data */
	GSPI_TX_MJPEG_NET_HEADER_CMD,
	GSPI_TX_MJPEG_DATA_CMD,
	
	/* GSPI Slave request */
	GSPI_GET_JPEG_STREAMING_REQ = (GSPI_CMD_START + 0x100), 
	GSPI_STOP_JPEG_STREAMING_REQ,
	GSPI_GP_SOCKET_CMD_REQ,				/* GP socket command request */
	GSPI_GP_SOCKET_RAW_DATA_SEND_REQ,	/* GP RAW data send request */
	GSPI_MJPEG_GET_DATA_REQ,			/* Get JPEG data for streaming, each data length is A/B buffer size of MJPEG module */
	
	/* GSPI Slave response */
	GSPI_AP_MODE_RES = (GSPI_CMD_START + 0x200), 
	GSPI_STATION_MODE_RES,
	GSPI_NONE_MODE_RES, 
	GSPI_FPS_RES,
	GSPI_WIFI_SETTING_RES,
	GSPI_NET_APP_STATE_RES,
	GSPI_GP_SOCK_CLIENT_CON_RES,
	GSPI_GP_SOCK_CLIENT_DISC_RES,
	GSPI_GP_SOCKET_RAW_DATA_DONE_RES,	/* GP RAW data reveived done, notify host */
	
	/* Do something command */
	GSPI_DO_ATCMD_CMD = (GSPI_CMD_START + 0x300),
	
	GSPI_STOP_CMD = 0xFFFF
} GSPI_CMD_E;

typedef enum
{
	DATA_UNKNOWN_TYPE = 0,
	DATA_JPEG_TYPE,
	DATA_DOCMD_TYPE,
	DATA_GP_SOCK_CMD_TYPE
} DATA_TYPE_E;

typedef enum
{
	GSPI_MJPEG_STOP_EVENT = 0,
	GSPI_MJPEG_START_EVENT,
	GSPI_MJPEG_TX_DONE_EVENT,
	GSPI_MJPEG_GET_DATA_EVENT,
	GSPI_MJPEG_UPDATE_FRAME_EVENT
} GSPI_MJPEG_EVENT_E;

PACKED typedef struct GSPI_CMD_S
{
	INT8U	token[8];
	INT32U 	cmd;
	INT32U  datalen;
} GSPI_CMD_T;

typedef union gspi_do_cmd_s
{
	INT8U data[GSPI_DOCMD_SIZE];
	struct cmd_blk
	{
		INT8U atcmd[4];
		INT8U equal[1];
		INT8U parameter[123];
	} cmd;
} gspi_do_cmd_t;

#define GSPI_CMD_SIZE	(sizeof(GSPI_CMD_T))

extern INT8U gspi_do_cmd_buf[];

extern void gspi_cmd_init(void);
extern void gspi_tx_cmd(INT32U cmd, INT32U tolen);
extern void gspi_tx_data(INT8U* buf, INT32U len, INT32U type);
extern void gspi_send_docmd(const char* cmd);
extern void gspi_transfer_mjpeg(INT8U* buf, INT32U len);
extern void gspi_register_gp_sock_cmd_cbk(INT32U cbk);
extern void gspi_register_mjpeg_cmd_cbk(INT32U mjpegcbk);
#endif	//__GSPI_CMD_H__
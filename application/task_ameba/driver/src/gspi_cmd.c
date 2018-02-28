/************************************************************
* gspi_cmd.c
*
* Purpose: Command and data stream via GSPI demo code
*
* Author: Eugenehsu
*
* Date: 2015/08/27
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.10
* History :
*
************************************************************/
#include "string.h"
#include "stdio.h"
#include "project.h"
#include "application.h"
#include "avi_encoder_app.h"
#include "gspi_master_drv.h"
#include "gspi_cmd.h"
#include "gp_cmd.h"

/**********************************************
*	Definitions
**********************************************/
#define GSPI_CMD_TOKEN_0		'G'
#define GSPI_CMD_TOKEN_1		'S'
#define GSPI_CMD_TOKEN_2		'P'
#define GSPI_CMD_TOKEN_3		'I'
#define GSPI_CMD_TOKEN_4		'_'
#define GSPI_CMD_TOKEN_5		'C'
#define GSPI_CMD_TOKEN_6		'M'
#define GSPI_CMD_TOKEN_7		'D'

#define WLAN0_NAME	"wlan0"
#define WLAN1_NAME	"wlan1"

#define GSPI_LOOPBACK_TEST 0
/**********************************************
*	Extern variables and functions
**********************************************/
//extern OS_EVENT *video_stream_q;
extern void gspi_rx_task(void *parm);
extern void gp_cmd_gspi_cbk(INT32U buf, INT32U len, INT32U event);
extern void gp_cmd_init(void);
/*********************************************
*	Variables declaration
*********************************************/
static GSPI_GP_SOCK_CMD_CBK gspi_gp_sock_cmd_cbk = NULL;
static GSPI_MJPEG_STREAMER_CBK gspi_mjpeg_cbk = NULL;
static GSPI_GP_CMD_CBK gspi_gp_cmd_cbk = NULL;
static INT32U GSPITaskStack[GSPI_STACK_SIZE];
static OS_EVENT *gspi_cmd_q;
static OS_EVENT *gspi_docmd_sem;
static INT32U gspi_frist_init = 1;
static void *gspi_cmd_q_stack[GSPI_MASTER_Q_NUM];
static INT8U gspi_running = 0;
static GSPI_CMD_T pgspi_cmd = {0};
static INT32U rx_data_cnt = 0;
INT32U gspi_mjpeg_addr = 0;
INT32U gspi_mjpeg_len = 0;
INT32U gspi_gp_sock_cmd_buf_addr = 0;
INT32U gspi_gp_sock_cmd_buf_len = 0;

INT8U gspi_do_cmd_buf[GSPI_DOCMD_SIZE];

#if(GSPI_LOOPBACK_TEST == 1)
/* For test */
INT8U TX_DATA[PACK_SIZE];
INT8U txen = 0;
#endif
/*****************************************************
    Utility functions
******************************************************/
#if(GSPI_LOOPBACK_TEST == 1)
static void _gspi_rx_handler(INT8U* buf, INT32U len)
{
	INT32U i;
	INT8U success;
	
	success = 1;
	for(i=0; i<len; i++)
	{
		if(buf[i] != (INT8U)(i&0xFF))
		{
			success = 0;
			DBG_PRINT("Data lenth = %d, result %s, rx_buf[%d] = 0x%x, 0x%x\r\n", len, "failed", i, buf[i], (INT8U)(i&0xFF));
			break;
		}
	}	
	
	txen = 0;	
	if(success)
		DBG_PRINT("Data lenth = %d, result %s\r\n", len, "success");
}	
#else
static void _process_gspi_command(GSPI_CMD_T* cmd, INT8U* buf, INT32U size, INT32U isdata)
{	
	//DBG_PRINT("cmd 0x%x size 0x%x\r\n", cmd->cmd, size);
	switch(cmd->cmd)
	{			
		case GSPI_GET_JPEG_STREAMING_REQ:
			DBG_PRINT("GSPI_GET_JPEG_STREAMING_REQ got\r\n");
			if(gspi_mjpeg_cbk)
			{
				gspi_mjpeg_cbk(NULL, 0, GSPI_MJPEG_START_EVENT);
			}	
			break;
		
		case GSPI_STOP_JPEG_STREAMING_REQ:
			DBG_PRINT("GSPI_STOP_JPEG_STREAMING_REQ got\r\n");
			if(gspi_mjpeg_cbk)
			{
				gspi_mjpeg_cbk(NULL, 0, GSPI_MJPEG_STOP_EVENT);
			}	
			break;
		
		case GSPI_MJPEG_GET_DATA_REQ:
			if(gspi_mjpeg_cbk)
			{
				gspi_mjpeg_cbk(NULL, cmd->datalen, GSPI_MJPEG_GET_DATA_EVENT);
			}	
			break;
			
		case GSPI_GP_SOCK_CLIENT_CON_RES:
		case GSPI_GP_SOCK_CLIENT_DISC_RES:
			if(!isdata && gspi_gp_sock_cmd_cbk)
			{
				if(cmd->cmd == GSPI_GP_SOCK_CLIENT_CON_RES)
					gspi_gp_sock_cmd_cbk(NULL, 0, GP_SOCK_CLI_CONN_EVENT);
				else if(cmd->cmd == GSPI_GP_SOCK_CLIENT_DISC_RES)
					gspi_gp_sock_cmd_cbk(NULL, 0, GP_SOCK_CLI_DISC_EVENT);
			}
			break;
		
		case GSPI_GP_SOCKET_RAW_DATA_DONE_RES:
			if(!isdata && gspi_gp_sock_cmd_cbk)
			{
				gspi_gp_sock_cmd_cbk(NULL, cmd->datalen, GP_SOCK_RAW_DATA_DONE_EVENT);
			}	
			break;
		
		case GSPI_GP_SOCKET_RAW_DATA_SEND_REQ:
			if(isdata && gspi_gp_sock_cmd_cbk)
			{
				//DBG_PRINT("GSPI_GP_SOCKET_RAW_DATA_SEND_REQ cmd 0x%x cmdlen %d, size %d\r\n", cmd->cmd, cmd->datalen, size);
				gspi_gp_sock_cmd_cbk(buf, size, GP_SOCK_RAW_DATA_SEND_BACK_EVENT);
			}	
			break;
		
		case GSPI_GP_SOCKET_CMD_REQ:
			if(isdata)
			{
				//DBG_PRINT("GSPI_GP_SOCKET_CMD_REQ got\r\n");
				if(gspi_gp_sock_cmd_cbk)
				{
					gspi_gp_sock_cmd_cbk(buf, size, GP_SOCK_RXDONE_EVENT);
				}	
			}	
			break;
		
		case GSPI_AP_MODE_RES:
			//DBG_PRINT("GSPI_AP_MODE_RES got\r\n");
			gp_wifi_state.mode = WIFI_AP_MODE;
			break;
		
		case GSPI_NONE_MODE_RES:
			//DBG_PRINT("GSPI_DISCONNECT_MODE_RES got\r\n");
			gp_wifi_state.mode = WIFI_NONE_MODE;
			break;
		
		case GSPI_STATION_MODE_RES:
			//DBG_PRINT("GSPI_STATION_MODE_RES got\r\n");
			gp_wifi_state.mode = WIFI_STATION_MODE;
			break;
		
		case GSPI_FPS_RES:
			//DBG_PRINT("GSPI_FPS_RES isdata %d\r\n", isdata);
			if(isdata && gspi_gp_cmd_cbk)
			{
				
				gspi_gp_cmd_cbk((INT32U)buf, size, GSPI_FPS_RES);
#if 0
				/* Copy FPS infomation from WiFi module */
				GP_FPS_T* fpsptr = (GP_FPS_T*)buf;
				gp_fps.net_fps = fpsptr->net_fps;
				gp_fps.rssi = fpsptr->rssi;
#endif				
			}	
			break;
		
		case GSPI_WIFI_SETTING_RES:
			if(isdata)
			{
				GP_WIFI_SETTING_T* wifi_setting_ptr = (GP_WIFI_SETTING_T*)buf;
				/* Check interface name */
				if(!strcmp(wifi_setting_ptr->ifname, WLAN0_NAME))
				{
					/* Copy WiFi setting from WiFi module */
					memcpy((void*)&gp_wifi0_setting, buf, (sizeof(GP_WIFI_SETTING_T) - 1));
				}
				else if(!strcmp(wifi_setting_ptr->ifname, WLAN1_NAME))
				{
					/* Copy WiFi setting from WiFi module */
					memcpy((void*)&gp_wifi1_setting, buf, (sizeof(GP_WIFI_SETTING_T) - 1));
				}		
				
				gp_wifi0_setting.isupdated = 1; 
			}	
			break;
						
		case GSPI_NET_APP_STATE_RES:
			if(isdata)
			{
				/* Copy NET APP state information */
				memcpy((void*)&gp_net_app_state, buf, (sizeof(GP_NET_APP_T) - 1));
				gp_net_app_state.isupdated = 1;
			}	
			break;
						
		default:
			DBG_PRINT("_process_gspi_command: unknown cmd 0x%x\r\n", cmd->cmd);
			break;
	}
}

static void _gspi_rx_handler(INT8U* buf, INT32U len)
{
	GSPI_CMD_T* pcmd;

	pcmd = (GSPI_CMD_T*)(buf);
	if(pcmd->token[0] == GSPI_CMD_TOKEN_0 && pcmd->token[1] == GSPI_CMD_TOKEN_1 && pcmd->token[2] == GSPI_CMD_TOKEN_2 
		&& pcmd->token[3] == GSPI_CMD_TOKEN_3 && pcmd->token[4] == GSPI_CMD_TOKEN_4 && pcmd->token[5] == GSPI_CMD_TOKEN_5
		&& pcmd->token[6] == GSPI_CMD_TOKEN_6 && pcmd->token[7] == GSPI_CMD_TOKEN_7)
	{
		/* This is a command */
		pgspi_cmd.cmd = pcmd->cmd;
		pgspi_cmd.datalen = pcmd->datalen;
		rx_data_cnt = pgspi_cmd.datalen;
		_process_gspi_command(&pgspi_cmd, (INT8U*)(buf), len, 0);
	}
	else
	{
		if(rx_data_cnt)
		{
		/* This is data */
		_process_gspi_command(&pgspi_cmd, (INT8U*)(buf), len, 1);
			rx_data_cnt -= len;
		}
		else
		{
			DBG_PRINT("_gspi_rx_handler data zero cmd 0x%x, len %d\r\n", pcmd->cmd, pcmd->datalen);
		}	
	}	
}
#endif
void gspi_send_docmd(const char* cmd)
{
	INT8U err;
	/* Get semaphore first */
	OSSemPend(gspi_docmd_sem, 0, &err);
	memset(gspi_do_cmd_buf, 0, sizeof(gspi_do_cmd_buf));
	strcpy((char*)gspi_do_cmd_buf, cmd);
	gspi_tx_data((INT8U*)gspi_do_cmd_buf, (INT32U)strlen((char*)gspi_do_cmd_buf), DATA_DOCMD_TYPE);
	//OSTimeDly(30);	/* Delay 300ms */
}	

void gspi_register_gp_sock_cmd_cbk(INT32U cbk)
{
	gspi_gp_sock_cmd_cbk = (GSPI_GP_SOCK_CMD_CBK)cbk;
}	

void gspi_register_mjpeg_cmd_cbk(INT32U mjpegcbk)
{
	gspi_mjpeg_cbk = (GSPI_MJPEG_STREAMER_CBK)mjpegcbk;
}	
	
void gspi_register_gp_cmd_cbk(INT32U cbk)
{
	gspi_gp_cmd_cbk = (GSPI_GP_CMD_CBK)cbk;
}	
	
void gspi_tx_cmd(INT32U cmd, INT32U tolen)
{
	INT8S res = 0;
	INT32U txlen;
	GSPI_CMD_T gspi_cmd;
	
	/* Fill GSPI command header */
	gspi_cmd.token[0] = GSPI_CMD_TOKEN_0;
	gspi_cmd.token[1] = GSPI_CMD_TOKEN_1;
	gspi_cmd.token[2] = GSPI_CMD_TOKEN_2;
	gspi_cmd.token[3] = GSPI_CMD_TOKEN_3;
	gspi_cmd.token[4] = GSPI_CMD_TOKEN_4;
	gspi_cmd.token[5] = GSPI_CMD_TOKEN_5;
	gspi_cmd.token[6] = GSPI_CMD_TOKEN_6;
	gspi_cmd.token[7] = GSPI_CMD_TOKEN_7;
	
	txlen = sizeof(gspi_cmd);
	gspi_cmd.cmd = cmd;
	gspi_cmd.datalen = tolen;
	res = gspi_write_page((INT8U*)&gspi_cmd, txlen, 1);
	if(res)
	{
		DBG_PRINT("gspi_tx_cmd cmd 0x%x error!\r\n", cmd);
		// handle error msg here
	}
}	

void gspi_transfer_mjpeg(INT8U* buf, INT32U len)
{
	INT8S res = 0;
	INT32U i, cnt;
	INT32U remainder;
	INT8U* ptr;
	
	/* Send JPEG command */
	gspi_tx_cmd(GSPI_TX_MJPEG_DATA_CMD, len);
	
	/* Start to transfer MJPEG frame */
	cnt = len/GSPI_DATA_TRANSFER_LEN;
	remainder = len%GSPI_DATA_TRANSFER_LEN;
	//DBG_PRINT("gspi_transfer_mjpeg 0x%x len %d, cnt %d reaminder %d\r\n", buf, len, cnt, remainder);
	
	ptr = buf;
	for(i=0; i<cnt; i++)
	{
		res = gspi_write_page((INT8U*)ptr, GSPI_DATA_TRANSFER_LEN, 1);
		if(res)
		{
			DBG_PRINT("gspi_transfer_mjpeg error!\r\n");
			// handle error msg here
		}
		ptr += GSPI_DATA_TRANSFER_LEN;
	}

	if(remainder)
	{
		res = gspi_write_page((INT8U*)ptr, remainder, 1);
		if(res)
		{
			DBG_PRINT("gspi_transfer_mjpeg error!\r\n");
			// handle error msg here
		}
	}	
}	

static void _transfer_mjpeg(INT8U* buf, INT32U len)
{
	INT8S res = 0;
	INT32U i, cnt;
	INT32U remainder;
	INT8U* ptr;
	
	/* Send JPEG command */
	gspi_tx_cmd(GSPI_TX_MJPEG_CMD, len);
	
	/* Start to transfer MJPEG frame */
	cnt = len/GSPI_DATA_TRANSFER_LEN;
	remainder = len%GSPI_DATA_TRANSFER_LEN;
	//DBG_PRINT("Sent JPEG buf 0x%x len 0x%x, cnt %d reaminder %d\r\n", buf, len, cnt, remainder);
	
	ptr = buf;
	for(i=0; i<cnt; i++)
	{
		res = gspi_write_page((INT8U*)ptr, GSPI_DATA_TRANSFER_LEN, 1);
		if(res)
		{
			DBG_PRINT("_transfer_mjpeg error!\r\n");
			// handle error msg here
		}
		ptr += GSPI_DATA_TRANSFER_LEN;
	}

	if(remainder)
	{
		res = gspi_write_page((INT8U*)ptr, remainder, 1);
		if(res)
		{
			DBG_PRINT("_transfer_mjpeg error!\r\n");
			// handle error msg here
		}
	}	
}	

static void _transfer_docmd(INT8U* buf, INT32U len)
{
	INT8S res = 0;
	
	/* Send GSPI_TX_ATCMD_CMD */
	gspi_tx_cmd(GSPI_TX_DOCMD_CMD, len);
	
	/* Start to transfer AT command buffer */
	res = gspi_write_page((INT8U*)gspi_do_cmd_buf, len, 1);
	if(res)
	{
		DBG_PRINT("_transfer_docmd error!\r\n");
		// handle error msg here
	}
}

static void _transfer_gp_socket_cmd(INT8U* buf, INT32U len)
{
	INT8S res = 0;
	
	/* Send GSPI_TX_GP_SOCKET_CMD */
	gspi_tx_cmd(GSPI_TX_GP_SOCKET_CMD, len);
	
	/* Start to transfer GP socket command */
	res = gspi_write_page((INT8U*)buf, len, 1);
	if(res)
	{
		DBG_PRINT("_transfer_gp_socket_cmd error!\r\n");
		// handle error msg here
	}
}

void gspi_tx_data(INT8U* buf, INT32U len, INT32U type)
{
	INT32U msg;
	
	if(type == DATA_JPEG_TYPE)
	{
		msg = GSPI_MJPEG_TX_EVENT;
		gspi_mjpeg_addr = (INT32U)buf;
		gspi_mjpeg_len = len;
		OSQPost(gspi_cmd_q, (void *)msg);	
	}
	else if(type == DATA_DOCMD_TYPE)
	{
		msg = GSPI_DOCMD_TX_EVENT;
		OSQPost(gspi_cmd_q, (void *)msg);
	}
	else if(type == DATA_GP_SOCK_CMD_TYPE)
	{
		INT32U i;
		msg = GSPI_GP_SOCK_CMD_TX_EVENT;
		gspi_gp_sock_cmd_buf_addr = (INT32U)buf;
		gspi_gp_sock_cmd_buf_len = len;
		//DBG_PRINT("gspi_tx_data, addr 0x%x len 0x%x\r\n", gspi_gp_sock_cmd_buf_addr, gspi_gp_sock_cmd_buf_len);
		OSQPost(gspi_cmd_q, (void *)msg);
	}	
}	

void do_cmd_handler(int argc, char *argv[])
{
	if(argc == 2)
	{
		memset(gspi_do_cmd_buf, 0, sizeof(gspi_do_cmd_buf));
		strcpy((char*)gspi_do_cmd_buf, argv[1]);
		gspi_tx_data((INT8U*)gspi_do_cmd_buf, (INT32U)strlen((char*)gspi_do_cmd_buf), DATA_DOCMD_TYPE);
	}	
	else
	{
		DBG_PRINT("atcmd error: too %s\r\n", (argc==1)?"few argument":"many arguments");
	}
}

void gspi_handle_task(void *parm)
{
	INT8U err;
	INT32U msg_id;
	
	DBG_PRINT("Enter gspi_handle_task...\r\n");
	gspi_running = 1;
	
	while(gspi_running)
	{
		msg_id = (INT32U) OSQPend(gspi_cmd_q, 0, &err);
		if(err != OS_NO_ERR)
			continue;
			
		switch(msg_id)
		{
			case GSPI_MJPEG_TX_EVENT:
				//DBG_PRINT("GSPI_MJPEG_TX_EVENT, buf 0x%x, len 0x%x\r\n", gspi_mjpeg_addr, gspi_mjpeg_len);
				_transfer_mjpeg((INT8U*)gspi_mjpeg_addr, gspi_mjpeg_len);
				if(gspi_mjpeg_cbk)
				{
					/* MJPEG TX DONE then call back to MJPEG task */
					gspi_mjpeg_cbk(NULL, 0, GSPI_MJPEG_TX_DONE_EVENT);
				}	
				break;
			
			case GSPI_DOCMD_TX_EVENT:
				_transfer_docmd((INT8U*)gspi_do_cmd_buf, strlen((char*)gspi_do_cmd_buf));
				OSSemPost(gspi_docmd_sem);
				//DBG_PRINT("Send DOCMD %s\r\n", gspi_do_cmd_buf);
				break;
			
			case GSPI_GP_SOCK_CMD_TX_EVENT:
				_transfer_gp_socket_cmd((INT8U*)gspi_gp_sock_cmd_buf_addr, gspi_gp_sock_cmd_buf_len);
				if(gspi_gp_sock_cmd_cbk)
				{
					gspi_gp_sock_cmd_cbk((INT8U*)gspi_gp_sock_cmd_buf_addr, gspi_gp_sock_cmd_buf_len, GP_SOCK_TXDONE_EVENT);
				}
				gspi_gp_sock_cmd_buf_addr = 0;
				gspi_gp_sock_cmd_buf_len = 0;
				break;
			
			default:
				break;
		}
	}
	
	OSQDel(gspi_cmd_q, OS_DEL_ALWAYS, &err);
	OSTaskDel(GSPI_TASK_PRIORITY);
}	

void gspi_cmd_init(void)
{
	INT8S res = 0;
	INT8U err;
	INT32U i = 0,j = 0;
	
	if(gspi_frist_init == 0)
		return;
	
	DBG_PRINT("Init GSPI master module...\r\n");
	
	/* Init GSPI master module */	
	gspi_master_init();
	/* Register GSPI RX callback to handle RX data */
	gspi_register_rx_cbk((void*)_gspi_rx_handler);

	gp_cmd_init();
	
	gspi_register_gp_cmd_cbk((INT32U)gp_cmd_gspi_cbk);
	
	gspi_docmd_sem = OSSemCreate(1);
	
	gspi_cmd_q = OSQCreate(gspi_cmd_q_stack, GSPI_MASTER_Q_NUM);
	if(gspi_cmd_q == 0)
	{
		DBG_PRINT("Create gspi_cmd_q failed\r\n");	
		return;
	}
	
	err = OSTaskCreate(gspi_handle_task, (void *)NULL, &GSPITaskStack[GSPI_STACK_SIZE - 1], GSPI_TASK_PRIORITY);	
	if(err != OS_NO_ERR)
	{ 
		DBG_PRINT("Cannot create GSPI task\n\r");
		return;
	}
	
	gspi_frist_init = 0;
	
#if(GSPI_LOOPBACK_TEST == 1)

	// prepare test data (0x00-0xFF, 0x00-0xFF......)
	for(i=0;i<PACK_SIZE;i++)
	{
		TX_DATA[i] = (INT8U)i&0xFF;
	}	

	do
	{
		if(txen == 0)
		{
			txen = 1;
			res = gspi_write_page(TX_DATA, PACK_SIZE, 1);
			//res = gspi_write_page(TX_DATA, j, 1);
			if(res)
			{
				DBG_PRINT("_gspi_write_page: Error! \r\n");
				// handle error msg here
			}
			j++;
			if(j > PACK_SIZE)
				j = 1;
			
		}		
	}while(1);
#endif
}
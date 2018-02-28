/************************************************************
* gp_cmd.c
*
* Purpose: GP command to get or set the Ameba status
*
* Author: Eugenehsu
*
* Date: 2015/09/24
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
#include "gspi_master_drv.h"
#include "gspi_cmd.h"
#include "gp_cmd.h"

/**********************************************
*	Definitions
**********************************************/
#define GP_CMD_Q_NUM			16
#define GPCMD_STACK_SIZE		1024

typedef enum GP_CMD_EVENT_E
{
	GP_FSP_CMD_EVENT
} GP_CMD_EVENT_T;
/**********************************************
*	Extern variables and functions
**********************************************/
/*********************************************
*	Variables declaration
*********************************************/
static OS_EVENT *gp_cmd_q;
static OS_EVENT *gp_cmd_sem;
static void *gspi_cmd_q_stack[GP_CMD_Q_NUM];
static INT32U GPCmdTaskStack[GPCMD_STACK_SIZE];

GP_FPS_T	gp_fps = {0};
GP_WIFI_STATE_T		gp_wifi_state = {0};
GP_WIFI_SETTING_T	gp_wifi0_setting = {0};
GP_WIFI_SETTING_T	gp_wifi1_setting = {0};
GP_NET_APP_T gp_net_app_state = {0};

void gp_cmd_data_lock(void)
{
	INT8U err;
	OSSemPend(gp_cmd_sem, 0, &err);
}

void gp_cmd_data_unlock(void)
{
	OSSemPost(gp_cmd_sem);
}

void gp_cmd_gspi_cbk(INT32U buf, INT32U len, INT32U event)
{
	INT32U msg;
	GP_FPS_T* fpsptr;

	gp_cmd_data_lock();
	
	switch(event)
	{
		case GSPI_FPS_RES:
			fpsptr = (GP_FPS_T*)buf;
			gp_fps.net_fps = fpsptr->net_fps;
			gp_fps.rssi = fpsptr->rssi;
			msg = GP_FSP_CMD_EVENT;
			OSQPost(gp_cmd_q, (void *)msg);	
			break;
			
		default:
			break;	
	}
	gp_cmd_data_unlock();
}	

void gp_cmd_task(void *parm)
{
	INT8U err;
	INT32U msg_id;
	
	while(1)
	{
		msg_id = (INT32U) OSQPend(gp_cmd_q, 0, &err);
		if(err != OS_NO_ERR)
			continue;
		
		gp_cmd_data_lock();	
		switch(msg_id)
		{
			case GP_FSP_CMD_EVENT:
				DBG_PRINT("%2d NET FPS, RSSI %2d\r\n", gp_fps.net_fps, gp_fps.rssi);
				memset((void*)&gp_fps, 0, sizeof(GP_FPS_T));
				break;
			
			default:
				break;
		}
		gp_cmd_data_unlock();
	}

		
	if(gp_cmd_q)
	{
		OSQDel(gp_cmd_q, OS_DEL_ALWAYS, &err);
		gp_cmd_q = NULL;
	}	
	OSTaskDel(GPCMD_TASK_PRIORITY);
}	

void gp_cmd_init(void)
{
	INT8U err;
	
	gp_cmd_sem = OSSemCreate(1);
	
	gp_cmd_q = OSQCreate(gspi_cmd_q_stack, GP_CMD_Q_NUM);
	if(gp_cmd_q == 0)
	{
		DBG_PRINT("Create gp_cmd_q failed\r\n");	
		return;
	}
	
	err = OSTaskCreate(gp_cmd_task, (void *)NULL, &GPCmdTaskStack[GPCMD_STACK_SIZE - 1], GPCMD_TASK_PRIORITY);	
	if(err != OS_NO_ERR)
	{ 
		DBG_PRINT("Cannot create GSPI task\n\r");
		return;
	}
}	

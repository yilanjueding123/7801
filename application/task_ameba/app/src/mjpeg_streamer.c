/************************************************************
* mjpeg_streamer.c
*
* Purpose: Demo code for MJPEG streamer
*
* Author: Eugenehsu
*
* Date: 2015/10/01
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
#include "video_encoder.h"
#include "avi_encoder_app.h"
#include "gspi_cmd.h"
#include "gp_cmd.h"
#include "gspi_master_drv.h"

/**********************************************
*	Definitions
**********************************************/
#define PIC_SRC_CAMERA					0	// Get pic from camera
#define PIC_SRC_JPEG_LIST				1	// Get pic from JPEG list
#define PIC_SRC							PIC_SRC_CAMERA	

/**********************************************
*	Definitions
**********************************************/
#define MJPEGSTREAMER_STACK_SIZE	1024
#define MJPEG_FPS 					30
#define MJPEG_Q 					0
#define MJPEG_JPEG_WIDTH			320
#define MJPEG_JPEG_HEIGHT			240
#define MJPEG_STREAMER_Q_NUM		16

#define MJPEG_BUF_IDLE_STATE		0
#define MJPEG_BUF_GSPI_TX_STATE		1
/**********************************************
*	Extern variables and functions
**********************************************/
extern void jpeg_list_init(void);
extern void jpeg_list_get_next(INT8U **addr, INT32U *size);

/*********************************************
*	Variables declaration
*********************************************/
static OS_EVENT *mjpeg_sem;
static OS_EVENT *mjpeg_streamer_q = NULL;
static void *mjpeg_stremer_q_stack[MJPEG_STREAMER_Q_NUM];
static INT32U MjpegStreamerTaskStack[MJPEGSTREAMER_STACK_SIZE];
INT32U mjpeg_buf_addr = 0;
INT32U mjpeg_buf_len = 0;
INT32U mjpeg_buf_sta = MJPEG_BUF_IDLE_STATE;
INT32U send_mjpeg = 0;
INT32U mjpeg_req_len = 0;
volatile INT32U mjpeg_update_addr = 0;
volatile INT32U mjpeg_update_len = 0;
volatile INT32U mjpeg_gspi_tx_addr = 0;
volatile INT32U mjpeg_gspi_tx_len = 0;


#if PIC_SRC == PIC_SRC_JPEG_LIST
static INT16U mjpeg_fps = MJPEG_FPS;
#endif

static void mjpeg_video_encode_stop(void)
{
	tft_init();
	
	video_encode_exit();
	
	//video_encode_stream_stop();
}

static void mjpeg_gspi_cmd_cbk(INT32U buf, INT32U len, INT32U event)
{
	INT32U msg = event;
	
	if(event == GSPI_MJPEG_GET_DATA_EVENT)
	{
		mjpeg_req_len = len;
	}	
	OSQPost(mjpeg_streamer_q, (void*)msg);
}	

void mjpeg_update_current_jpeg(INT8U* buf, INT32U len)
{
	INT32U msg;
	
	if(mjpeg_buf_sta == MJPEG_BUF_IDLE_STATE && send_mjpeg)
	{
		msg = GSPI_MJPEG_UPDATE_FRAME_EVENT;
		mjpeg_update_addr = (INT32U)buf;
		mjpeg_update_len = len;
		OSQPost(mjpeg_streamer_q, (void*)msg);
	}
	else
	{
		//avi_encode_post_empty(video_stream_q, (INT32U)buf);
	}
}

void mjpeg_gspi_send_current_jpeg_data(void)
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
			mjpeg_buf_sta = MJPEG_BUF_IDLE_STATE;
			//DBG_PRINT(" EOF buf 0x%x len %d\r\n", mjpeg_buf_addr, mjpeg_buf_len);
		}	
		//DBG_PRINT(" tx[%d] ", mjpeg_gspi_tx_len);		
	}
}	

#if PIC_SRC == PIC_SRC_CAMERA
void mjpeg_streamer_task(void *parm)
{
	INT32U msg_id;
	INT8U err;
	
	while(1)
	{
		msg_id = (INT32U) OSQPend(mjpeg_streamer_q, 0, &err);
		if(err != OS_NO_ERR)
			continue;

		OSSemPend(mjpeg_sem, 0, &err);
					
		switch(msg_id)
		{
			case GSPI_MJPEG_STOP_EVENT:
				send_mjpeg = 0;
				/* Stop video encode */
				mjpeg_video_encode_stop();
				mjpeg_buf_sta = MJPEG_BUF_IDLE_STATE;
				mjpeg_gspi_tx_addr = 0;
				mjpeg_gspi_tx_len = 0;
				mjpeg_update_addr = 0;
				mjpeg_update_len = 0;
				if(mjpeg_buf_addr)
				{
					//avi_encode_post_empty(video_stream_q, mjpeg_buf_addr);
					mjpeg_buf_addr = 0;
				}
				memset((void*)&gp_fps, 0 ,sizeof(GP_FPS_T));
				break;
			
			case GSPI_MJPEG_START_EVENT:
				/* Start video encode */
				send_mjpeg = 1;
				mjpeg_buf_sta = MJPEG_BUF_IDLE_STATE;
				//mjpeg_video_encode_start();
				break;
			
			case GSPI_MJPEG_GET_DATA_EVENT:
				//DBG_PRINT("GSPI_MJPEG_GET_DATA_EVENT len %d\r\n", mjpeg_req_len);
				/* Send A/B buffer size of JPEG to slave*/
				mjpeg_gspi_send_current_jpeg_data();
				break;
			
			case GSPI_MJPEG_TX_DONE_EVENT:
				/* MJPEG TX done via GSPI */
				mjpeg_buf_sta = MJPEG_BUF_IDLE_STATE;
				break;
			
			case GSPI_MJPEG_UPDATE_FRAME_EVENT:
				/* Send from MJPEG encode task */
				mjpeg_buf_sta = MJPEG_BUF_GSPI_TX_STATE;
				/* release previous buffer */
				if(mjpeg_buf_addr)
				{
					//avi_encode_post_empty(video_stream_q, mjpeg_buf_addr);
				}
			
				mjpeg_buf_addr = (INT32U)mjpeg_update_addr;
				mjpeg_buf_len = mjpeg_update_len;
				
				mjpeg_gspi_tx_addr = (INT32U)mjpeg_update_addr;
				mjpeg_gspi_tx_len = mjpeg_update_len;
		
				/* Send MJPEG header to client and triggle slave to get JPEG data from host */
				//DBG_PRINT("TX HEADER CMD: buf 0x%x len %d\r\n", mjpeg_buf_addr, mjpeg_buf_len);
				gspi_tx_cmd(GSPI_TX_MJPEG_NET_HEADER_CMD, mjpeg_buf_len);	
				break;
				
			default:
				break;
		}
		OSSemPost(mjpeg_sem);	
	}
	
	if(mjpeg_streamer_q)
	{
		OSQDel(mjpeg_streamer_q, OS_DEL_ALWAYS, &err);
		mjpeg_streamer_q = NULL;
	}	
	OSTaskDel(MJPEG_STREAMER_PRIORITY);
}	
#else
static void mjpeg_streamer_task(void *pvParameters)
{
	INT8U *addr;
	INT32U size, one_sec_count=0;

	jpeg_list_init();

	while (1)
	{
		if(send_jpeg)
		{
			jpeg_list_get_next(&addr, &size);
			mjpeg_update_current_jpeg(addr, size);

			// Print FPS every second
			if (++one_sec_count == mjpeg_fps)
			{
				one_sec_count = 0;
				memset(gspi_do_cmd_buf, 0,GSPI_DOCMD_SIZE);
				strcpy((char*)gspi_do_cmd_buf, "GPGS=FPS");
				gspi_tx_data((INT8U*)gspi_do_cmd_buf, (INT32U)strlen((char*)gspi_do_cmd_buf), DATA_DOCMD_TYPE);
				DBG_PRINT("%2d NET FPS, %2d SPI FPS, RSSI %2d\r\n", gp_fps.net_fps, gp_fps.gspi_fps, gp_fps.rssi);
			}
		}

		OSTimeDly(1000/mjpeg_fps/10);
	}
}
#endif

void mjpeg_streamer_init(void)
{
	INT8U err;
	
	/* Init GSPI command module */
	gspi_cmd_init();
		
	gspi_register_mjpeg_cmd_cbk((INT32U)mjpeg_gspi_cmd_cbk);
	
	mjpeg_sem = OSSemCreate(1);
	
	if(mjpeg_streamer_q == NULL)
	{
		mjpeg_streamer_q = OSQCreate(mjpeg_stremer_q_stack, MJPEG_STREAMER_Q_NUM);
		if(mjpeg_streamer_q == 0)
		{
			DBG_PRINT("Create mjpeg_streamer_q failed\r\n");	
			return;
		}
	}	
	
	err = OSTaskCreate(mjpeg_streamer_task, (void *)NULL, &MjpegStreamerTaskStack[MJPEGSTREAMER_STACK_SIZE - 1], MJPEG_STREAMER_PRIORITY);	
	if(err != OS_NO_ERR)
	{ 
		DBG_PRINT("Cannot create MJPEG streamer task\n\r");
		return;
	}
}	
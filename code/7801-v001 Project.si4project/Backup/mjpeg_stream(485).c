/******************************************************
 * mjpeg_stream.c
 *
 * Purpose: Motion JPEG streaming server --- process/flow part
 *
 * Author: Eugene Hsu
 *
 * Date: 2015/07/01
 *
 * Copyright Generalplus Corp. ALL RIGHTS RESERVED.
 *
 * Version :
 * History :
 *
 *******************************************************/
#include "stdio.h"
#include "string.h"
#include "gplib.h"
#include "application.h"
#include "mjpeg_stream.h"
#include "mjpeg_stream_wifi.h"
#include "state_wifi.h"
#include "socket_cmd.h"
#include "host_config.h"
#include "host_apis.h"
#include "cmd_def.h"

#if 1
#define printf  DBG_PRINT
#else
#define printf(...)
#endif

#define MJ_STREAM_TIMEOUT 10000	// ms

// +++
extern OS_EVENT* my_AVIEncodeApQ;
extern MSG_Q_ID ApQ;
// ---
extern INT16U present_state;
extern INT32U mjpeg_service_ready_flag;
extern INT32U mjpeg_service_playstop_flag;
extern INT32U mjpeg_service_playjpeg_flag;

mjpeg_ctl_t mjpeg_ctl_blk =
{
	NULL,				/* Server connection */
	NULL,				/* Client connection */
	NULL,				/* RX buffer */
	NULL,				/* Message Q handler */
	MJPEG_CLIENT_STATE_DISCONNECT,	/* State */
	0,					/* FPS lost count */
	0					/* FPS zero count */
};

static INT32U mjpeg_stack[MJPEG_TASK_STACKSIZE];
static void *mjpeg_frame_q_stack[MJPEG_MSG_QUEUE_LEN];

static int mjpeg_service_enable = 0; /* mjpeg service enable flag */
static int mjpeg_service_start  = 0; /* mjpeg service status flag */
static unsigned int mjpeg_cnt = 0;

static volatile INT16S LossFrame_Cnt = 0;
static volatile INT16S chaQ_func_cnt = 0;

INT16S get_lossframe_cnt(void)
{
	return LossFrame_Cnt;
}

int mjpeg_frame_counter_set(int mode)
{
	switch(mode)
	{
		case 0:
			mjpeg_cnt = 0;
			break;
		case 1:
		default:
			mjpeg_cnt = mjpeg_cnt + 1;
			break;
	}
}

static int mjpeg_frame_rate_adj(void)
{
	static unsigned int time_idx = 0;
	static unsigned int mjpeg_framerate_phaseadder = 0x10000;
	static unsigned int adder = (unsigned int)(1.0*0x10000);	//22FPS // 25 fps
	//const unsigned int adder =  (unsigned int)(0.8*0x10000);	// 15 fps
	//const unsigned int adder =  (unsigned int)(0.5*0x10000);	// 12.5 fps
	int ret = 0;

	if ( (mjpeg_framerate_phaseadder>>16)!=0 )
	{
		ret = 1;
	}
	if (time_idx > 100)		// Because of 25fps, 100 is about 4 seconds
	{
		mjpeg_cnt = mjpeg_cnt >> 2;			// frames per second
		adder = (mjpeg_cnt+1) * 0xBA2; //22FPS   //0xA3D;		// 1 frame is about 0xA3D.  // 0x10000 / 25fps = 0xA3D;
		if (adder>0x10000)
		{
			adder = 0x10000;
		}
		else if (adder<0x8000)
		{
			adder = 0x8000;
		}
		DBG_PRINT("adder = 0x%x\r\n",adder);
		mjpeg_frame_counter_set(0);
		time_idx = 0;
	}
	mjpeg_framerate_phaseadder &= 0xFFFF;
	mjpeg_framerate_phaseadder += adder;
	time_idx++;

	return ret;
}

#include "rtp.h"
extern INT32U RES_WIFI_JPEG_THUMBNAIL_START;
extern INT32U RES_WIFI_JPEG_THUMBNAIL_END;
extern UDP_SRV_SOCKET_CTL_T rtp_server_ctl_blk;
INT32S mjpeg_send_picture(mjpeg_write_data_t* pMjpegWData)
{
	OS_CPU_SR cpu_sr;
	INT32S flag = 1;
	static INT32U stamp = 0;

	if ( (present_state&0xFF)!=BROWSE_MODE )
	{
	    flag = 1; //mjpeg_frame_rate_adj();
	}
	else
	{
		flag = 1;			// BROWSE_MODE (It can't be flag = 0, when play JPEG.)
	}
    if ((mjpeg_ctl_blk.mjpeg_state == MJPEG_CLIENT_STATE_CONNECT)&&(flag==1) )
    {
		LossFrame_Cnt = 0;
		//wifi_connect_led_fliker();
		#if PREVIEW_TCPIP
		OS_ENTER_CRITICAL();
        mjpeg_ctl_blk.mjpeg_state = MJPEG_CLIENT_STATE_SENDING_JPEG;
		OS_EXIT_CRITICAL();		
	 	cache_invalid_range(pMjpegWData->mjpeg_addr, pMjpegWData->mjpeg_size);  // 否則Wi-Fi會破圖		
        OSQPost(mjpeg_ctl_blk.mjpeg_frame_q, (void*)pMjpegWData);
		#else
	 	cache_invalid_range(pMjpegWData->mjpeg_addr, pMjpegWData->mjpeg_size);  // 否則Wi-Fi會破圖
			#if 0		// fixed pattern
			{
				INT32U begin = (INT32U)(&RES_WIFI_JPEG_THUMBNAIL_START);
				INT32U end = (INT32U)(&RES_WIFI_JPEG_THUMBNAIL_END);
				INT32U size = end - begin + 1;
				pMjpegWData->mjpeg_addr = (INT32U)(&RES_WIFI_JPEG_THUMBNAIL_START);
				pMjpegWData->mjpeg_size = size;
			}
			#endif

			stamp += (4*900);
			#if 1
			OSQPost(rtp_server_ctl_blk.udpsrv_tx_q, (void*)pMjpegWData);
			#else	// 優先級太高，會被Wi-Fi錄影時會被卡死
			mjpeg_wifi_set_write_frame_info(pMjpegWData);
			flag = rtp_check_sending_data();
			if (flag)
			{
				//DBG_PRINT("S:%d ",pMjpegWData->mjpeg_size);
				mjpeg_frame_counter_set(1);				
				rtp_send_whole_jpeg( (char*)(pMjpegWData->mjpeg_addr), pMjpegWData->mjpeg_size, stamp);
				mjpeg_service_playjpeg_flag = 1;
			}
			else
			{
				//DBG_PRINT("rtp stop\r\n");
			}
			OSQPost(my_AVIEncodeApQ, (void*)(AVIPACKER_MSG_VIDEO_WRITE_DONE|pMjpegWData->mjpeg_addr_idx));
			#endif

		#endif
        return 0;
    }
    else
    {
    	/*
        if(pMjpegWData->running_app == STATE_VIDEO_RECORD)
        {
            OSQPost(my_AVIEncodeApQ, (void*)(AVIPACKER_MSG_VIDEO_WRITE_DONE|pMjpegWData->mjpeg_addr_idx));
        }
        else if(pMjpegWData->running_app == STATE_BROWSE)
        {
            msgQSend(ApQ, AVIPACKER_MSG_VIDEO_WRITE_DONE, &(pMjpegWData->mjpeg_addr_idx), sizeof(INT8U), MSG_PRI_NORMAL);
        }
        */
        if (chaQ_func_cnt >= 50) 
        {
        	LossFrame_Cnt = 1;
        }
        //DBG_PRINT("L");		
        return 1;
    }
}

#if PREVIEW_TCPIP

void mjpeg_playback_stop(mjpeg_write_data_t* pMjpegWData)
{
    pMjpegWData->msg_id = MJPEG_STOP_EVENT;
    OSQPost(mjpeg_ctl_blk.mjpeg_frame_q, (void*)pMjpegWData);
}

// Return    0: Success
// Return (-1): Network busy
// Return (-2): 1. Failed to send data
//              2. AP stops JPEG sending
static INT32S mjpeg_process_stream(mjpeg_ctl_t *ctl_blk)
{
	INT8U os_err;
	INT32S ret = 0;
	mjpeg_write_data_t* pMJPEG_Write_Msg;
	OS_CPU_SR cpu_sr;

	if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
	{
		printf("\r\n mjstreamer doesn't allow\r\n");
		ret = -2;	// Don't delete mjpeg task@josephhsieh@20150827
		goto exit_service;
	}

	ret = mjpeg_wifi_alloc_rx_buff(ctl_blk);
	if (ret != MJW_RET_SUCCESS)
		goto exit;

	ret = mjpeg_wifi_detect_stream_req(ctl_blk);
	if (ret == MJW_RET_SUCCESS)
	{
		printf("do mjpeg streaming\r\n");

		mjpeg_wifi_write_stream_header(ctl_blk);

		/* Enable upper AP to send mjpeg via streamer */
		OS_ENTER_CRITICAL();
		mjpeg_ctl_blk.mjpeg_state = MJPEG_CLIENT_STATE_CONNECT;
		OS_EXIT_CRITICAL();
		mjpeg_ctl_blk.lostcnt = 0;
		mjpeg_ctl_blk.zerocnt = 0;
		printf("One client connected\r\n");

		gp_clear_modebak();
		msgQSend(ApQ, MSG_WIFI_CONNECTED, NULL, NULL, MSG_PRI_NORMAL);
		OSTimeDly(100);

		/* Start send JPEG frame */
		mjpeg_service_ready_flag = 1;	// notify sock_cmd (8081)
		
		ssv6xxx_set_rc_value(RC_RATEMASK, RC_DEFAULT_RATE_MSK);
        ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_ENABLE, 1);
	    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_SET_LAST_BMODE_RATE, 3);
	    
	    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_SET_LAST_BMODE_RETRY,0);
	    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_SET_RETRY_MAX, 3);
	    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_SET_MAIN_TRY, 1);
	    ssv6xxx_set_TXQ_SRC_limit(1,4);
		while(1)
		{
			pMJPEG_Write_Msg = (mjpeg_write_data_t*)OSQPend(mjpeg_ctl_blk.mjpeg_frame_q, 0, &os_err);
			mjpeg_wifi_set_write_frame_info(pMJPEG_Write_Msg);
			if (++chaQ_func_cnt >= 50) chaQ_func_cnt = 50;
			switch(pMJPEG_Write_Msg->msg_id)
			{
			case MJPEG_SEND_EVENT:
				{
					INT32U t = OSTimeGet();
					/* Start frame */
					mjpeg_wifi_write_frame_start(ctl_blk);
					/* Send frame */
					ret = mjpeg_wifi_write_frame_data(ctl_blk);
					if(ret != MJW_RET_SUCCESS)
					{
						ret = -2;		// Don't delete mjpeg task@josephhsieh@20150827
						goto exit;
					}
					/* End of frame */
					mjpeg_wifi_write_frame_end(ctl_blk);
					mjpeg_frame_counter_set(1);
				}

				OS_ENTER_CRITICAL();
				mjpeg_ctl_blk.mjpeg_state = MJPEG_CLIENT_STATE_CONNECT;
				OS_EXIT_CRITICAL();
                		mjpeg_service_playjpeg_flag = 1;
				break;

			case MJPEG_STOP_EVENT:
				printf("Stop JPEG Sending\r\n");
				ret = -2;		// Don't delete mjpeg task@josephhsieh@20150827
				goto exit;

			case MJPEG_NETWORK_BUSY_EVENT:
				ret = -1;
				printf("MJPEG TX busy, abort this connection\r\n");
				goto exit;

			default:
				break;
			}

			if(pMJPEG_Write_Msg->running_app == STATE_VIDEO_RECORD)
			{
				OSQPost(my_AVIEncodeApQ, (void*)(AVIPACKER_MSG_VIDEO_WRITE_DONE|pMJPEG_Write_Msg->mjpeg_addr_idx));
			}
			else if(pMJPEG_Write_Msg->running_app == STATE_BROWSE)
			{
				msgQSend(ApQ, AVIPACKER_MSG_VIDEO_WRITE_DONE, &(pMJPEG_Write_Msg->mjpeg_addr_idx), sizeof(INT8U), MSG_PRI_NORMAL);
			}

		}
	}

exit:
	mjpeg_wifi_free_rx_buff(ctl_blk);

exit_service:
	mjpeg_service_ready_flag = 0;	// notify sock_cmd (8081)
	chaQ_func_cnt = 0;
	ssv6xxx_set_rc_value(RC_RATEMASK, 0x1);
	mjpeg_wifi_close_client_conn(ctl_blk);
	OS_ENTER_CRITICAL();
	mjpeg_ctl_blk.mjpeg_state = MJPEG_CLIENT_STATE_DISCONNECT;
	OS_EXIT_CRITICAL();
	/* Exit this socket to accept a new connection */
	printf("Exit %s\r\n", __func__);
	return ret;
}

void mjpeg_stream_task(void *pvParameters)
{
	INT32S err;
	OS_CPU_SR cpu_sr;

	printf("Enter %s port %d\r\n", __func__, MJPEG_SERVER_PORT);

	mjpeg_ctl_blk.mjpeg_frame_q = OSQCreate(mjpeg_frame_q_stack, MJPEG_MSG_QUEUE_LEN);
	if(!mjpeg_ctl_blk.mjpeg_frame_q)
	{
		printf("Create mjpeg_frame_q failed\r\n");
		return;
	}
	else
	{
		printf("Create mjpeg_frame_q ok. mjpeg_ctl_blk.mjpeg_frame_q=%x\r\n", mjpeg_ctl_blk.mjpeg_frame_q);
	}

	OS_ENTER_CRITICAL();
	mjpeg_ctl_blk.mjpeg_state = MJPEG_CLIENT_STATE_DISCONNECT;
	OS_EXIT_CRITICAL();

	err = mjpeg_wifi_start_server(&mjpeg_ctl_blk);
	if (err != 0)
	{
		printf("start mjpeg wifi server failed \r\n");
		return;
	}

	mjpeg_service_enable =  0;
	mjpeg_service_start  = 0;

	for(;; )
	{
		if (mjpeg_service_enable == 0)
		{
			OSTimeDlyHMSM(0,0,0,100);
			continue;
		}
		mjpeg_service_start = 1;

		/* block for waiting client connect */
		err = mjpeg_wifi_wait_client_conn(&mjpeg_ctl_blk);
		if (err == MJW_RET_SUCCESS)
		{
			printf("mjpeg_stream_task: A new client connected\r\n");

			//ssv6xxx_set_rate_mask(RC_DEFAULT_RATE_MSK);//080 //3C8//7c8
			/* A client connected */
			// netconn_set_sendtimeout(mjpeg_ctl_blk.mjpeg_client, MJ_STREAM_TIMEOUT);
			err = mjpeg_process_stream(&mjpeg_ctl_blk);

			// +++
			if (mjpeg_ctl_blk.mjpeg_state == MJPEG_CLIENT_STATE_DISCONNECT)
			{
				if (present_state == STATE_BROWSE)		// browser mode
				{
					msgQSend(ApQ, MSG_WIFI_DISCONNECTED, NULL, NULL, MSG_PRI_NORMAL);
				}
				else 
				{
					DBG_PRINT("WIFI_DISCONNECTED preview off\r\n");
					video_encode_preview_off();
				}
			}
			// ---

			while(mjpeg_wifi_del_client_conn(&mjpeg_ctl_blk) != MJW_RET_SUCCESS)
			{
				printf("Unable to delete mjpeg_client connection\r\n");
				/* Delay 10 ms */
				OSTimeDly(MJPEG_SHORT_DELAY/10);
			}
			mjpeg_service_playstop_flag = 0;
		}
		else
		{
		}

		/* flush message Q */
		OSQFlush(mjpeg_ctl_blk.mjpeg_frame_q);
		mjpeg_service_start = 0;
	}
}

void mjpeg_service_init(void)
{
	INT8U err;

	err = OSTaskCreate(mjpeg_stream_task, NULL, (void *)&mjpeg_stack[MJPEG_TASK_STACKSIZE-1], MJPG_STREAMER_PRIORITY);
	if(err != OS_NO_ERR)
	{
		printf("mjpeg_stream_task create failed\r\n");
	}
	else
	{
		printf("mjpeg_stream_task created ok. pri=%d\r\n", MJPG_STREAMER_PRIORITY);
	}
}

void mjpeg_start_service(void)
{
    int times = 100;
    
    if (mjpeg_service_start == 0)
    {
	    mjpeg_service_enable = 1;
        
        /* waiting for mpjeg service start */
        while (mjpeg_service_start == 0)
        {
            OSTimeDlyHMSM(0,0,0,100);
            if (--times <= 0)
            {
                break;
            }
        }
        
        if (times <= 0)
        {
            printf("mjpeg service start failed\r\n");
        }
        else
        {
            printf("mjpeg service start success\r\n");
        }
    }
    else
    {
        //printf("mjpeg service already start\r\n");
    }
}

void mjpeg_stop_service(void)
{
    int times = 100;
    
    if (mjpeg_service_start == 1)
    {
	    mjpeg_service_enable = 0;

        if (mjpeg_service_ready_flag)
        {
            mjpeg_write_data_t stop_event;
            /* exit mjpeg stream process */
            mjpeg_playback_stop(&stop_event);
        }

        /* waiting for mpjeg service stop */
        while (mjpeg_service_start == 1)
        {
            OSTimeDlyHMSM(0,0,0,100);
            if (--times <= 0)
            {
                break;
            }
        }
        
        if (times <= 0)
        {
            printf("mjpeg service stop failed\r\n");
        }
        else
        {
            printf("mjpeg service stop success\r\n");
        }
    }
    else
    {
        //printf("mjpeg service already stop\r\n");
    }
}
#endif

void rtsp_connect_callback(void)
{
	ssv6xxx_set_rc_value(RC_RATEMASK, RC_DEFAULT_RATE_MSK);
	DBG_PRINT("RTSP datarate = 0x%x\r\n",RC_DEFAULT_RATE_MSK);
}

void rtsp_disconnect_callback(void)
{
	ssv6xxx_set_rc_value(RC_RATEMASK, 0x8);
	DBG_PRINT("RTSP datarate = 0x8\r\n");	
}


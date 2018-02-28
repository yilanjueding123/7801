#include "state_wifi.h"
#include "ap_display.h"
#include "socket_cmd.h"
#include "mjpeg_stream.h"
#include "wifi_abstraction_layer.h"
#include "application.h"
#if WIFI_FUNC_ENABLE
#include "Host_apis.h"
#endif
/****************************************************************************/
#if !PREVIEW_TCPIP
extern rtsp_ip_addr_set(char *ip);
#endif
/****************************************************************************/
WIFI_STATE_ARGS_t	WifiStateArgs = {0};
INT8U* display_wifi_frame = NULL;
static OS_EVENT		*sw_wifi_jpeg_sem = NULL;
static INT32U			wifi_display_buffer;
/****************************************************************************/
/*
 *	sw_wifi_jpeg_sem_init:
 */
void sw_wifi_jpeg_sem_init(void)
{
  if(sw_wifi_jpeg_sem == NULL) 
  {	
  	sw_wifi_jpeg_sem = OSSemCreate(1);
  }

}
/****************************************************************************/
/*
 *	sw_wifi_jpeg_lock:
 */
void sw_wifi_jpeg_lock(void)
{
	INT8U err;

	OSSemPend(sw_wifi_jpeg_sem, 0, &err);
}

/****************************************************************************/
/*
 *	sw_wifi_jpeg_unlock:
 */
void sw_wifi_jpeg_unlock(void)
{
	OSSemPost(sw_wifi_jpeg_sem);
}

/****************************************************************************/
/*
 *	Wifi_State_Get:
 */
INT32U Wifi_State_Get(void)
{
    return WifiStateArgs.Wifi_State_Flag;
}

/****************************************************************************/
/*
 *	Wifi_State_Set:
 */
void Wifi_State_Set(INT8U wifiState)
{
    WifiStateArgs.Wifi_State_Flag = wifiState;

    if(wifiState == WIFI_STATE_FLAG_CONNECT)
    {
    		//gpio_write_io(SPEAKER_EN, 0);		// turn off audio
		DBG_PRINT("wifi state set: connect\r\n");
		Wifi_mode_active();
	}
    else
    {
	DBG_PRINT("wifi state set: connect\r\n");
		Wifi_mode_shutdown();
    		//gpio_write_io(SPEAKER_EN, 1);	   // turn on audio
	}
}

/****************************************************************************/
/*
 *	Wifi_Connect:
 *  	Success : 0
 *      Fail	: 1
 */
INT32U Wifi_Connect(void)
{
    INT8U keyMode = WIFI_MODE_KEY;
    //INT32U type;

    msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_KEY_REGISTER, &keyMode, sizeof(INT8U), MSG_PRI_NORMAL);
  
    return 0;
}

/****************************************************************************/
/*
 *	Wifi_Disconnect:
 */
INT32U Wifi_Disconnect(void)
{
    INT8U keyMode = GENERAL_KEY;

    msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_KEY_REGISTER, &keyMode, sizeof(INT8U), MSG_PRI_NORMAL);

    return 0;
}

void Wifi_mode_active(void)
{
    //INT32U i,display_frame0;
	//INT32U buff_size;
	//INT8U curr_disp_dev;
	#if !PREVIEW_TCPIP	
	char user_ip[4];
	
	user_ip[0] = 192;
	user_ip[1] = 168;
	user_ip[2] = 26;
	user_ip[3] = 1;
	#endif
	
	/*
    for(i=0; i<50; ++i)
    {
        display_frame0 = ap_display_queue_get(display_isr_queue);
        if (display_frame0!=NULL)
            break;
        OSTimeDly(1);
    }
    for(i=0; i<TFT_WIDTH*TFT_HEIGHT; i++)
    {
        *(INT16U *)(display_frame0 + i*2) = 0;
    }
    ap_wifi_signal_setting_page_draw(display_frame0);
	*/
    DBG_PRINT("Wifi_mode_active\r\n");
    if (1)
    {
	 	//add call fucntion to turn on wifi
		wal_ldo_enable();
        if (wal_ap_mode_enable() != WAL_RET_SUCCESS)
        {
            DBG_PRINT("WiFi Turn ON Fail!!\r\n");
        }
        else
        {
            DBG_PRINT("WiFi Turn ON Ok!!\r\n");
			#if PREVIEW_TCPIP			
            mjpeg_start_service();
			#else
			rtp_start_service(WIFI_JPEG_WIDTH, WIFI_JPEG_HEIGHT);
			rtsp_ip_addr_set(user_ip);
			//10ms * dly  
			//udp要发送4个命令, 每个的延时时间
			rtsp_cmd_wait(80);
			rtsp_start_service();						
			#endif
            socket_cmd_start_service();
        }
    }

    OSTimeDly(50);
    /*0
    ssv6xxx_set_rc_value(RC_RATEMASK, RC_DEFAULT_RATE_MSK);
	{
		u32 param = ((RC_DEFAULT_UP_PF<<16)|RC_DEFAULT_DOWN_PF);
		ssv6xxx_set_rc_value(RC_PER, param);
	}
	*/
    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_ENABLE, 1);
    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_SET_LAST_BMODE_RATE, 3);     
    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_SET_LAST_BMODE_RETRY, 0);     
    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_SET_MAIN_TRY, 1);     
    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_SET_RETRY_MAX, 3);     
    ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_ENABLE, 1);
    OSTimeDly(100);
/*
	curr_disp_dev = ap_display_get_device();
	if(curr_disp_dev == DISP_DEV_TFT)
	{
	    for(i=0; i<50; ++i)
	    {
	        display_frame0 = ap_display_queue_get(display_isr_queue);
	        if (display_frame0!=NULL)
	            break;
	        OSTimeDly(1);
	    }
	    wifi_display_buffer = display_frame0;				
	    for(i=0; i<TFT_WIDTH*TFT_HEIGHT; i++)
	    {
	        *(INT16U *)(display_frame0 + i*2) = 0;
	    }
	    ap_wifi_display_setting_page_draw(display_frame0);

	    OSTimeDly(10);
    }
	
	//wait display_dma_queue empty
	while(1)
	{
		if(ap_display_queue_query(display_dma_queue) == 0)
		{
			break;
		}
	}

	// Free TFT Buffer 
	if(curr_disp_dev == DISP_DEV_TFT)
	{
		for(i=0; i<DISPLAY_BUF_NUM; i++)
		{
			if ( display_frame[i] && ((INT32U)display_frame[i] != R_TFT_FBI_ADDR))
			{
				gp_free(display_frame[i]);
				display_frame[i] = 0;
			}

			display_isr_queue[i] = 0;
			display_dma_queue[i] = 0;
		}
	}
	else if(curr_disp_dev == DISP_DEV_TV)
	{
		for(i=0; i<DISPLAY_BUF_NUM; i++)
		{
			if ( display_frame[i] && ((INT32U)display_frame[i] != R_TV_FBI_ADDR))
			{
				gp_free(display_frame[i]);
				display_frame[i] = 0;
			}

			display_isr_queue[i] = 0;
			display_dma_queue[i] = 0;
		}
	}

	buff_size = WIFI_JPEG_WIDTH*WIFI_JPEG_HEIGHT*2;
	display_wifi_frame = gp_malloc_align(buff_size*DISPLAY_BUF_NUM, 64);
	if(display_wifi_frame == NULL)
	{
		DBG_PRINT("wifi display frame fail \r\n");	
	}

	for(i=0; i<DISPLAY_BUF_NUM; i++)
	{
		if (ap_display_queue_put(display_isr_queue, (INT32U) (display_wifi_frame+i*buff_size)) == STATUS_FAIL) {
			DBG_PRINT("wifi INIT put Q FAIL.\r\n");
		}
	}
*/
	//---    
}

void Wifi_mode_shutdown()
{
	INT32U i;
	INT32U buff_size;
	//INT8U curr_disp_dev;

    DBG_PRINT("Wifi_mode_shutdown\r\n");
    wifi_display_buffer = NULL;	
    if (1)
    {
		socket_cmd_stop_service();
		#if PREVIEW_TCPIP
		mjpeg_stop_service();		
		#else
		rtp_stop_service();
		rtsp_stop_service();
		#endif

        if (wal_ap_mode_disable() != WAL_RET_SUCCESS)
        {
            DBG_PRINT("WiFi Turn OFF Fail!!\r\n");
        }
        else
        {
            DBG_PRINT("WiFi Turn OFF Ok!!\r\n");
        }
    }

    OSTimeDly(20);

   //+++ 

	if(display_wifi_frame)
	{
		gp_free(display_wifi_frame);
		display_wifi_frame = NULL;
	}
/*
   	buff_size = getDispDevBufSize();
   	curr_disp_dev = ap_display_get_device();
	if(curr_disp_dev == DISP_DEV_TFT)
	{
		for(i=0; i<DISPLAY_BUF_NUM; i++)
		{
			display_isr_queue[i] = 0;
			display_dma_queue[i] = 0;

			if(display_frame[i] == 0) // セR_TFT_FBI_ADDR 穦 display_isr_queue , ┮睲Θ0碞
			{
				display_frame[i] = gp_malloc_align(buff_size, 64);
				DBG_PRINT("Restore display_frame[%d] addr = 0x%x\r\n", i, display_frame[i]);	

				if (ap_display_queue_put(display_isr_queue, (INT32U)display_frame[i]) == STATUS_FAIL) {
					DBG_PRINT("Restore wifi INIT put Q FAIL.\r\n");
				}
			}
		}
	}
	else if(curr_disp_dev == DISP_DEV_TV)
	{
		for(i=0; i<TV_DISPLAY_BUF_NUM; i++)
		{
			display_isr_queue[i] = 0;
			display_dma_queue[i] = 0;

			if(display_frame[i] == 0) // セR_TFT_FBI_ADDR 穦 display_isr_queue , ┮睲Θ0碞
			{
				display_frame[i] = gp_malloc_align(buff_size, 64);
				DBG_PRINT("Restore display_frame[%d] addr = 0x%x\r\n", i, display_frame[i]);	

				if (ap_display_queue_put(display_isr_queue, (INT32U)display_frame[i]) == STATUS_FAIL) {
					DBG_PRINT("Restore wifi INIT put Q FAIL.\r\n");
				}
			}
		}
	}*/

}

#define WIFI_SYMBOL_AREA 0x2300

INT32S Wifi_symbol_show(INT16U new_symbol)
{
	INT32S ret = -1;
	short *p = (short*)wifi_display_buffer;
	INT32U i;
	static INT16U symbol = 0;
	

	if ( (p!=NULL)&&(symbol!=new_symbol) )
	{
		DBG_PRINT("wifi_display_act = 0x%x\r\n",wifi_display_buffer);
		symbol = new_symbol;		
		for (i=0;i<WIFI_SYMBOL_AREA;++i)
		{
			if (p[i]!=0x0000)
			{
				p[i] = symbol  ;	// 0xFFFF: white color, 0x1EBF: blue color
			}
		}
		ret = 0;
	}

	return ret;
}


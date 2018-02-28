#include "state_video_preview.h"
#include "ap_video_preview.h"
#include "ap_state_handling.h"
#include "ap_state_config.h"
#include "ap_video_record.h"
#include "ap_display.h"
#include "avi_encoder_app.h"
#include "my_video_codec_callback.h"
#include "my_avi_encoder_state.h"
#include "state_wifi.h"

//+++
INT8U pic_flag;
INT32U photo_check_time_loop_count = 0;
INT32U photo_check_time_preview_count = 0;

volatile INT8U g_sequence_start_flag = 0;
volatile INT8U g_sequence_cnt_flag = 0;

#define CONTINUOUS_SHOOTING_COUNT_MAX	5

//	prototypes
void state_video_preview_init(void);
void state_video_preview_exit(void);


extern INT8U ap_state_config_usb_mode_get(void);
extern int g_wifi_notify_pic_done;
extern void ap_video_preview_timedelay_cap_icon_set(INT8U enable);
extern void led_power_off(void);

void state_photo_check_timer_isr(void)
{
	photo_check_time_loop_count++;

	/*
		當是底下 MSG 馬上跳離
		MSG_APQ_CONNECT_TO_PC / MSG_APQ_MENU_KEY_ACTIVE / MSG_APQ_MODE
	*/	
	if((pic_flag == 2)||(pic_flag == 3)||(pic_flag == 4))
	{
		photo_check_time_loop_count = photo_check_time_preview_count;
	}
	
	if(photo_check_time_loop_count >= photo_check_time_preview_count)
	{
		msgQSend(ApQ, MSG_APQ_CAPTURE_PREVIEW_ON, NULL, NULL, MSG_PRI_NORMAL);	
		timer_stop(TIMER_C);
	}
}

extern INT8U ext_rtc_pwr_on_flag;

void state_video_preview_init(void)
{
	DBG_PRINT("video_preview state init enter\r\n");
	ap_video_preview_init();
}

volatile INT8U pic_down_flag=0;
extern volatile INT32U g_wifi_pic_work;
extern INT8U video_to_preview_get(void);
extern void ap_state_handling_power_off(INT32U wakeup_flag);
void state_video_preview_entry(void *para)
{
	EXIT_FLAG_ENUM exit_flag = EXIT_RESUME;
	INT32U msg_id, file_path_addr,led_type;
	INT32U *prev_state;
	STAudioConfirm *audio_temp;
	//INT8U continuous_shooting_count = 0;
	INT8U photo_check_time_flag;
	INT32U photo_check_time_start;
	INT32U photo_check_time_end;
	INT32U photo_check_time_count_ms;
	INT32U photo_check_ui_setting_ms;

	pic_flag = 0;
	prev_state = para;

	/*
		拍照模式下快速檢視
		0	關
		1	2秒
		2	5秒
	*/
	//連拍不啟動快速檢視

	if(ap_state_handling_storage_id_get() == NO_STORAGE) { //not to use burst when no SD card inserted
		photo_check_time_flag = ap_state_config_preview_get();
		if(ap_state_config_burst_get())
		{
			g_sequence_start_flag = TIME_LAPSE_ENABLE;
		}
		else
		{
			g_sequence_start_flag = TIME_LAPSE_DISABLE;
		}
	} else if(ap_state_config_burst_get()) {
		photo_check_time_flag = 0;
		g_sequence_start_flag = TIME_LAPSE_ENABLE;
	} else {
		photo_check_time_flag = ap_state_config_preview_get();
		g_sequence_start_flag = TIME_LAPSE_DISABLE;
	}

	/*
		從video recording / setting 進來都是將preview buffer 由dummy address 導到display address
	*/

	if(ext_rtc_pwr_on_flag) {
		ext_rtc_pwr_on_flag = 0;
		ap_video_capture_mode_switch(1, STATE_VIDEO_PREVIEW);
	} else {
	  #if (!ENABLE_SAVE_SENSOR_RAW_DATA)
		ap_video_capture_mode_switch(0, STATE_VIDEO_PREVIEW);
	  #else
	  	#if 1
		ap_video_capture_mode_switch(1, STATE_VIDEO_PREVIEW);
		#endif
	  #endif
	}

	#if 0
	if(*(INT32U *) para == STATE_STARTUP) { // 開機後進來
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_USBD_DETECT_INIT, NULL, NULL, MSG_PRI_NORMAL);
	}
	#endif
	if(*(INT32U *) para == STATE_VIDEO_RECORD)
	{
		if (video_to_preview_get())
		{
			msgQSend(ApQ, MSG_APQ_CAPTURE_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
		}
	}
	else if (*(INT32U *) para == STATE_BROWSE)
	{
		;
	}

	state_video_preview_init();
	DBG_PRINT("Video_preview_init!\r\n");

	while (exit_flag == EXIT_RESUME) {
		if (msgQReceive(ApQ, &msg_id, (void *) ApQ_para, AP_QUEUE_MSG_MAX_LEN) == STATUS_FAIL) {
			continue;
		}

		switch (msg_id) {
			case EVENT_APQ_ERR_MSG:
				audio_temp = (STAudioConfirm *)ApQ_para;
				if((audio_temp->result == AUDIO_ERR_DEC_FINISH) && (audio_temp->source_type == AUDIO_SRC_TYPE_APP_RS)){
					gpio_write_io(SPEAKER_EN, DATA_LOW);
				} else {
					audio_confirm_handler((STAudioConfirm *)ApQ_para);
				}
				break;
			case MSG_STORAGE_SERVICE_MOUNT:
				ap_state_handling_storage_id_set(ApQ_para[0]);
        		//ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
        		//ap_state_handling_icon_show_cmd(ICON_SD_CARD, NULL, NULL);
			    ap_state_handling_str_draw_exit();
				left_capture_num = cal_left_capture_num();
				#if DUAL_STREAM_FUNC_ENABLE
 				if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
				#endif
				{				
					left_capture_num_display(left_capture_num);
				}
        		DBG_PRINT("[Video Preview Mount OK]\r\n");
        		break;
        	case MSG_STORAGE_SERVICE_NO_STORAGE:
        		led_type = LED_NO_SDC;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
        		ap_state_handling_storage_id_set(ApQ_para[0]);
        		//ap_state_handling_icon_clear_cmd(ICON_SD_CARD, NULL, NULL);
        		//ap_state_handling_icon_show_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
				left_capture_num = cal_left_capture_num();
				#if DUAL_STREAM_FUNC_ENABLE
 				if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
 				#endif
 				{
					left_capture_num_display(left_capture_num);
				}
        		DBG_PRINT("[Video Preview Mount FAIL]\r\n");
        		break;

        	case MSG_APQ_MENU_KEY_ACTIVE:
				if(pic_flag == 0) {
				    ap_state_handling_str_draw_exit();
        			OSTimeDly(3);
					video_encode_preview_off();
					vid_enc_disable_sensor_clock();
					#if DUAL_STREAM_FUNC_ENABLE
	 				if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
	 				#endif
	 				{					
		       		  	OSQPost(StateHandlingQ, (void *) STATE_SETTING);
		        		exit_flag = EXIT_BREAK;
		        	}
				} else {
					pic_flag = 3;
				}
            	break;

			#if DUAL_STREAM_FUNC_ENABLE
			case MSG_APQ_WIFI_MENU_EXIT:
				ap_video_capture_mode_switch(0, STATE_VIDEO_PREVIEW);   	
				video_encode_preview_on();
				state_video_preview_init();

				//+++
				if(ap_state_handling_storage_id_get() == NO_STORAGE) { //not to use burst when no SD card inserted
					photo_check_time_flag = ap_state_config_preview_get();
					if(ap_state_config_burst_get())
					{
						g_sequence_start_flag = TIME_LAPSE_ENABLE;
					}
					else
					{
						g_sequence_start_flag = TIME_LAPSE_DISABLE;
					}
				} else if(ap_state_config_burst_get()) {
					photo_check_time_flag = 0;
					g_sequence_start_flag = TIME_LAPSE_ENABLE;
				} else {
					photo_check_time_flag = ap_state_config_preview_get();
					g_sequence_start_flag = TIME_LAPSE_ENABLE;
				}
				//---
				
			break;
			#endif
            case MSG_APQ_WIFI_SWITCH:
            	if(ap_video_record_sts_get() & VIDEO_RECORD_BUSY) break;
       			video_encode_preview_off();
				vid_enc_disable_sensor_clock();	           
            	if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
				{
					if(Wifi_Connect() == 0)
					{
						Wifi_State_Set(WIFI_STATE_FLAG_CONNECT);
					}
					led_type = LED_WIFI_ENABLE;
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				}
				else
				{
					if(Wifi_Disconnect() == 0)
					{
						Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);
					}
					led_type = LED_WIFI_DISABLE;
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				}    

		   		ap_video_capture_mode_switch(0, STATE_VIDEO_PREVIEW);
				DBG_PRINT("perview: wifi switch...\r\n");

				/*
				if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
				{
					video_encode_preview_on();
				}
				*/
				break;
        	case MSG_APQ_PREV_KEY_ACTIVE:	
        		#if DUAL_STREAM_FUNC_ENABLE
        			if(ap_display_get_device() == DISP_DEV_TV)
  					{
  						DBG_PRINT("TV display,drop prev key!!\r\n");
  						break;
  					}
					if(pic_flag != 0)
					{
						/*
							如有拍照連拍,將它次數歸零,不要再送MSG_APQ_CAPTURE_CONTINUOUS_SHOOTING
						*/					
						/*if(ap_state_config_burst_get() && (ap_state_handling_storage_id_get() != NO_STORAGE))
						{
							continuous_shooting_count = 0;
						}*/
					
						break;
					}
					if(g_sequence_start_flag==TIME_LAPSE_PROCESS)
					{
						g_sequence_start_flag=TIME_LAPSE_ENABLE;
						ap_video_preview_timedelay_cap_icon_set(0);
						g_sequence_cnt_flag=0;												
						g_wifi_notify_pic_done = 0xFFFFFFFF;		// stop sequence
						ap_peripheral_auto_off_force_disable_set(0);
					}
		       		video_encode_preview_off();
					vid_enc_disable_sensor_clock();

					if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
					{
						if(Wifi_Connect() == 0)
						{
							Wifi_State_Set(WIFI_STATE_FLAG_CONNECT);
						}
						led_type = LED_WIFI_ENABLE;
						msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					}
					else
					{
						if(Wifi_Disconnect() == 0)
						{
							Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);
						}
						led_type = LED_WIFI_DISABLE;
						msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					}    

			   		ap_video_capture_mode_switch(0, STATE_VIDEO_PREVIEW);

					if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
					{
						//video_encode_preview_on();
					}
					
        		#else
					if(pic_flag == 0)
					{
						if(msg_id == MSG_APQ_NEXT_KEY_ACTIVE)
						{
		        			ap_video_record_zoom_inout(0);
		        		}
		        		else
		        		{
		        			ap_video_record_zoom_inout(1);
		        		}
		        	}
	        	#endif
        	break;

  	     	case MSG_APQ_MODE:
				if(pic_flag == 0) {
				    ap_state_handling_str_draw_exit();
				    OSTimeDly(3);
					if(avi_encoder_state_get() & AVI_ENCODER_STATE_SENSOR)
					{
						video_preview_close(0);
						avi_encoder_state_clear(AVI_ENCODER_STATE_SENSOR);
					}		
					//if (gp_mode_flag_get() ==  0)
					//	OSQPost(StateHandlingQ, (void *) STATE_VIDEO_RECORD);
					//else		
      		  			OSQPost(StateHandlingQ, (void *) STATE_BROWSE);
	        		exit_flag = EXIT_BREAK;
	        	} else {
	        		pic_flag = 4;
	        	}
	        	break;

        	case MSG_APQ_VIDEO_RECORD_ACTIVE:
        		vid_enc_disable_sensor_clock();
  		  		OSQPost(StateHandlingQ, (void *) STATE_VIDEO_RECORD);
  		  		exit_flag = EXIT_BREAK;
  		  		DBG_PRINT("Preview mode to record mode\r\n");
#if 1 //KEY_FUNTION_TYPE == SAMPLE2
        		break;
#endif
#if C_MOTION_DETECTION == CUSTOM_ON
 			case MSG_APQ_MD_ACTIVE:
	        //啎擬耀宒奀ㄛ珂猁豖堤絞ヶ耀宒輛�踶樞鯆�宒ㄛ婬符夔羲ゐ痄雄淈聆
	        	vid_enc_disable_sensor_clock();
  		  		OSQPost(StateHandlingQ, (void *) STATE_VIDEO_RECORD);
  		  		exit_flag = EXIT_BREAK;	
	        	break;
#endif			
			case MSG_APQ_CAPTURE_KEY_ACTIVE:	
			case MSG_APQ_CAPTURE_CONTINUOUS_SHOOTING:
				if(ap_video_record_sts_get() & VIDEO_RECORD_MD) break; //痄雄淈聆奀, 潰聆善鼴桽瑩硉拸虴揭燴
			//G_PRINT("g_sequence_start_flag = %d\r\n",g_sequence_start_flag);
			//G_PRINT("pic_flag  = %d\r\n",pic_flag == 0);
				if(pic_flag == 0) {
					pic_down_flag = 1;
					//if(((msg_id == MSG_APQ_FUNCTION_KEY_ACTIVE)||(msg_id == MSG_APQ_CAPTURE_KEY_ACTIVE))&&(g_sequence_start_flag==TIME_LAPSE_PROCESS))
					if((msg_id == MSG_APQ_CAPTURE_KEY_ACTIVE)&&(g_sequence_start_flag==TIME_LAPSE_PROCESS))
					{
						g_sequence_start_flag=TIME_LAPSE_ENABLE;
						ap_video_preview_timedelay_cap_icon_set(0);
						g_sequence_cnt_flag=0;
						g_wifi_notify_pic_done = 0xFFFFFFFF;		// stop sequence									
						ap_peripheral_auto_off_force_disable_set(0);								
					}
					else
					{
						if(ap_video_preview_func_key_active() < 0) {
							pic_flag = 0;
							pic_down_flag = 0;
							ap_state_handling_file_creat_set(0);
						} 
						else 
						{
							//if((msg_id == MSG_APQ_FUNCTION_KEY_ACTIVE)||(msg_id == MSG_APQ_CAPTURE_KEY_ACTIVE))
							if(msg_id == MSG_APQ_CAPTURE_KEY_ACTIVE)
							{
								if(g_sequence_start_flag==TIME_LAPSE_ENABLE)
								{
									g_sequence_start_flag=TIME_LAPSE_PROCESS;
									//ap_video_preview_timedelay_cap_icon_set(1);
									ap_peripheral_auto_off_force_disable_set(1);
								}
								else if(g_sequence_start_flag==TIME_LAPSE_PROCESS)
								{
									g_sequence_start_flag=TIME_LAPSE_ENABLE;
									//ap_video_preview_timedelay_cap_icon_set(0);
									ap_peripheral_auto_off_force_disable_set(0);
								}
							}
							g_sequence_cnt_flag=0;
							pic_flag = 1;
							photo_check_time_start = OSTimeGet();
						}
					}
				}
                else 
                { 
                    //if(((msg_id == MSG_APQ_FUNCTION_KEY_ACTIVE)||(msg_id == MSG_APQ_CAPTURE_KEY_ACTIVE))&&(g_sequence_start_flag==TIME_LAPSE_PROCESS)) 
                    if((msg_id == MSG_APQ_CAPTURE_KEY_ACTIVE)&&(g_sequence_start_flag==TIME_LAPSE_PROCESS)) 
                    { 
						pic_flag = 8; 
						g_sequence_start_flag=TIME_LAPSE_ENABLE;
						//ap_video_preview_timedelay_cap_icon_set(0);
						g_sequence_cnt_flag=0;												
						g_wifi_notify_pic_done = 0xFFFFFFFF;		// stop sequence
						ap_peripheral_auto_off_force_disable_set(0);
						//DBG_PRINT("exception handle\r\n"); 
                    } 
                } 				
				break;

        	case MSG_STORAGE_SERVICE_PIC_REPLY:
        		if(ap_video_preview_reply_action((STOR_SERV_FILEINFO *) ApQ_para) < 0) {
        			pic_flag = 0;
        			pic_down_flag = 0;
					ap_state_handling_file_creat_set(0);
        			break;
        		}
        		if (ap_state_handling_file_creat_get())
				{
					g_wifi_pic_work = 1;
					led_type = LED_CAPTURE;
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					DBG_PRINT("--------Set_cap_mode------\r\n");
				}
				else
				{
					led_type = LED_NO_SDC;
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					DBG_PRINT("-----no sd----\r\n");
				}
        		file_path_addr = ((STOR_SERV_FILEINFO *) ApQ_para)->file_path_addr;
        		break;

        	case MSG_STORAGE_SERVICE_PIC_DONE:
				 g_wifi_notify_pic_done++;
        		ap_video_preview_reply_done(ApQ_para[0], file_path_addr);
				if(ap_state_handling_file_creat_get())
				{
					g_wifi_pic_work = 0;
					DBG_PRINT("-----set_cap_wait----\r\n");
	    		    led_type = LED_WAITING_CAPTURE;
				    msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);  
				}
				pic_down_flag = 0;
				ap_state_handling_file_creat_set(0);
				// 沒有啟動快速檢視,直接跑到MSG_APQ_CAPTURE_PREVIEW_ON
				if(photo_check_time_flag)
				{
					photo_check_time_end = OSTimeGet();
					photo_check_ui_setting_ms = ((photo_check_time_flag*3)-1)*1000; // ms

					photo_check_time_count_ms = (photo_check_time_end-photo_check_time_start)*10; //ms

					//當拍照花費時間小於UI設定,才啟動timer
					if(photo_check_time_count_ms < photo_check_ui_setting_ms)
					{
						photo_check_time_loop_count = 0;
						photo_check_time_preview_count = (photo_check_ui_setting_ms-photo_check_time_count_ms)/100;
						timer_msec_setup(TIMER_C, 100, 0, state_photo_check_timer_isr); // 100 ms
		        		break;
					}
				}
			case MSG_APQ_CAPTURE_PREVIEW_ON:			
        		if(pic_flag == 2) { // Connect To PC
				    ap_state_handling_str_draw_exit();
				    OSTimeDly(3);
        			video_encode_preview_off();
					vid_enc_disable_sensor_clock();
					
	   				#if DUAL_STREAM_FUNC_ENABLE
					if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
					{
						if(Wifi_Disconnect() == 0)
						{
							Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);
						}
					}
					#endif
        			
	        		ap_state_handling_connect_to_pc(STATE_VIDEO_PREVIEW);
        			break;
        		} else if(pic_flag == 3) { // MEMU Key
				    ap_state_handling_str_draw_exit();
				    OSTimeDly(3);
        		  	OSQPost(StateHandlingQ, (void *) STATE_SETTING);
	        		exit_flag = EXIT_BREAK;
        			break;
        		} else if(pic_flag == 4) { // MODE Key
				    ap_state_handling_str_draw_exit();
				    OSTimeDly(3);
      		  		OSQPost(StateHandlingQ, (void *) STATE_BROWSE);
	        		exit_flag = EXIT_BREAK;
        			break;
        		} else if(pic_flag == 5) { // HDMI insert
				    ap_state_handling_str_draw_exit();
				    OSTimeDly(3);

	   				#if DUAL_STREAM_FUNC_ENABLE
					if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
					{
						if(Wifi_Disconnect() == 0)
						{
							Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);
						}
					}
					#endif

      		  		OSQPost(StateHandlingQ, (void *) STATE_BROWSE);
					msgQSend(ApQ, MSG_APQ_HDMI_PLUG_IN, NULL, NULL, MSG_PRI_NORMAL);
	        		exit_flag = EXIT_BREAK;
        			break;
        		} else if((pic_flag == 6) || (pic_flag == 7)) { // TV plug in/out
        			video_encode_preview_off();
					vid_enc_disable_sensor_clock();

					if(pic_flag == 6)
					{
		   				#if DUAL_STREAM_FUNC_ENABLE
						if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
						{
							if(Wifi_Disconnect() == 0)
							{
								Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);
							}
						}
						#endif
						ap_state_handling_tv_init();
					}
					else
					{
						ap_state_handling_tv_uninit();
					}

			   		ap_video_capture_mode_switch(1, STATE_VIDEO_PREVIEW);
					video_capture_resolution_display();
					left_capture_num = cal_left_capture_num();
					left_capture_num_display(left_capture_num);

					pic_flag = 0;
					break;
        		}
        		
				/* sensor不關,將sensor 送出的資料引導到底層preview flow */
				#if ENABLE_SAVE_SENSOR_RAW_DATA
		   		ap_video_capture_mode_switch(0, STATE_VIDEO_RECORD);
				#else
		   		ap_video_capture_mode_switch(0, STATE_VIDEO_PREVIEW);
		   		#endif
				/*
					拍照模式下連拍
				*/
				if(ap_state_config_burst_get() && (ap_state_handling_storage_id_get() != NO_STORAGE))
				{
					/*continuous_shooting_count++;
					if(continuous_shooting_count < CONTINUOUS_SHOOTING_COUNT_MAX)
					{
        				msgQSend(ApQ, MSG_APQ_CAPTURE_CONTINUOUS_SHOOTING, NULL, NULL, MSG_PRI_NORMAL);
        			}
        			else
        			{
						continuous_shooting_count = 0;
        			}*/
        			if(g_sequence_start_flag==TIME_LAPSE_PROCESS)
        			{
        				g_sequence_cnt_flag=1;
        			}
				}

        		pic_flag = 0;
			
			break;
 		
        	case MSG_APQ_POWER_KEY_ACTIVE:
        	case MSG_APQ_SYS_RESET:
				video_encode_exit();
				OSTimeDly(10);        		
				ap_state_config_hang_mode_set(0x02);	//STATE_VIDEO_PREIVEW
				if(msg_id == MSG_APQ_POWER_KEY_ACTIVE) {
					if (ApQ_para[0] != 0) led_power_off();
	        		ap_state_handling_power_off(0);
	        	} else {
	        		ap_state_handling_power_off(1);
	        	}
        		break;
			case MSG_APQ_CHARGE_MODE:
				if(Wifi_Disconnect() == 0)
				{	
					Wifi_State_Set(0);
				}
				//ChargeMode_set(1);
				led_type = LED_CHARGEING;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				break;
#if C_BATTERY_DETECT == CUSTOM_ON
        	case MSG_APQ_BATTERY_LVL_SHOW:
        		ap_state_handling_battery_icon_show(ApQ_para[0]);
        		break;
        	case MSG_APQ_BATTERY_CHARGED_SHOW:
				ap_state_handling_charge_icon_show(1);
        		break;
        	case MSG_APQ_BATTERY_CHARGED_CLEAR:
				ap_state_handling_charge_icon_show(0);
        		break;        		
        	case MSG_APQ_BATTERY_LOW_SHOW:
        		ap_state_handling_clear_all_icon();
        		OSTimeDly(5);
				ap_state_handling_str_draw_exit();
				ap_state_handling_str_draw(STR_BATTERY_LOW, WARNING_STR_COLOR);
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_BATTERY_LOW_BLINK_START, NULL, NULL, MSG_PRI_NORMAL);
        		break;
#endif

        	case MSG_APQ_CONNECT_TO_PC:
        		led_type = LED_USB_CONNECT;//LED_RECORD;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				if(ap_display_get_device() != DISP_DEV_TFT) break;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TV_POLLING_STOP, NULL, NULL, MSG_PRI_NORMAL);
				OSTimeDly(3);
				if(pic_flag == 0) {
				    ap_state_handling_str_draw_exit();
				    OSTimeDly(3);
        			video_encode_preview_off();
					vid_enc_disable_sensor_clock();

	   				#if DUAL_STREAM_FUNC_ENABLE
					if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
					{
						if(Wifi_Disconnect() == 0)
						{
							Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);
						}
					}
					#endif
        			
	        		ap_state_handling_connect_to_pc(STATE_VIDEO_PREVIEW);
	        	} else {
	        		pic_flag = 2;
	        	}
        		break;

        	case MSG_APQ_DISCONNECT_TO_PC:
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TV_POLLING_START, NULL, NULL, MSG_PRI_NORMAL);
        		ap_state_handling_disconnect_to_pc();
        		OSTimeDly(100);
				/*
					先將web cam的preview 導出去,再切換成capture 的preview					
				*/
				pic_flag = 0;
				vid_enc_disable_sensor_clock();
	       		video_encode_preview_on();
				/* sensor不關,將sensor 送出的資料引導到底層preview flow */
		   		ap_video_capture_mode_switch(0, STATE_VIDEO_PREVIEW);
        		break;

			case MSG_APQ_NIGHT_MODE_KEY:
				audio_effect_play(EFFECT_CLICK);
				ap_state_handling_night_mode_switch();
				break;

			case MSG_APQ_USER_CONFIG_STORE:
				ap_state_config_store();
				break;

			case MSG_APQ_AUDIO_EFFECT_UP:
			case MSG_APQ_AUDIO_EFFECT_DOWN:
			    break;

			case MSG_APQ_HDMI_PLUG_IN:
				if(pic_flag == 0) {
				    ap_state_handling_str_draw_exit();
				    OSTimeDly(3);
        			video_encode_preview_off();
	        		vid_enc_disable_sensor_clock();

	   				#if DUAL_STREAM_FUNC_ENABLE
					if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
					{
						if(Wifi_Disconnect() == 0)
						{
							Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);
						}
					}
					#endif
	        		
      		  		OSQPost(StateHandlingQ, (void *) STATE_BROWSE);
					msgQSend(ApQ, MSG_APQ_HDMI_PLUG_IN, NULL, NULL, MSG_PRI_NORMAL);
	        		exit_flag = EXIT_BREAK;
	        	} else {
	        		pic_flag = 5;
	        	}
			break;

			
			//+++ TV_OUT_D1
			case MSG_APQ_TV_PLUG_OUT:
			case MSG_APQ_TV_PLUG_IN:
				video_capture_resolution_clear();
				left_capture_num_str_clear();

				if(pic_flag == 0) {
				    ap_state_handling_str_draw_exit();
				    OSTimeDly(3);
        			video_encode_preview_off();
					vid_enc_disable_sensor_clock();
					if(msg_id == MSG_APQ_TV_PLUG_IN)
					{
		   				#if DUAL_STREAM_FUNC_ENABLE
						if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
						{
							if(Wifi_Disconnect() == 0)
							{
								Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);
							}
						}
						#endif
						ap_state_handling_tv_init();
	        		}
	        		else
	        		{
						ap_state_handling_tv_uninit();
	        		}
			   		ap_video_capture_mode_switch(1, STATE_VIDEO_PREVIEW);
					video_capture_resolution_display();
					left_capture_num = cal_left_capture_num();
					left_capture_num_display(left_capture_num);
				} else {
					if(msg_id == MSG_APQ_TV_PLUG_IN) {
		        		pic_flag = 6;
		        	} else {
		        		pic_flag = 7;
		        	}
				}
			break;
			//---
			#if DUAL_STREAM_FUNC_ENABLE
				case MSG_WIFI_CONNECTED:
					DBG_PRINT("Connect WIFI \r\n");
		       		video_encode_preview_on();
				break;
				case MSG_WIFI_DISCONNECTED:
					DBG_PRINT("Disconnect WIFI \r\n");
		       		video_encode_preview_off();
				break;
			#endif	

#if C_SCREEN_SAVER == CUSTOM_ON
			case MSG_APQ_KEY_IDLE:
        		ap_state_handling_lcd_backlight_switch(0);
        		break;
			case MSG_APQ_KEY_WAKE_UP:
        		ap_state_handling_lcd_backlight_switch(1);
        		break;
#endif
			default:
				ap_state_common_handling(msg_id);
				break;
		}
	}

	state_video_preview_exit();
	if(photo_check_time_flag)
	{
		timer_stop(TIMER_C);
	}
}

void state_video_preview_exit(void)
{
	ap_video_preview_exit();

	DBG_PRINT("Exit video_preview state\r\n");
}

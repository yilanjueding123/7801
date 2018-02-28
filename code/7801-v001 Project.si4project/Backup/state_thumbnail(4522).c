#include "ap_state_config.h"
#include "ap_state_handling.h"
#include "state_thumbnail.h"
#include "ap_music.h"
#include "ap_display.h"

static INT8U delayed_action_flag = 0;

extern INT8U g_thumbnail_reply_action_flag;
extern void led_power_off(void);
extern void ap_video_capture_mode_switch(INT8U DoSensorInit, INT16U EnterAPMode);

//	prototypes
void state_thumbnail_init(void);
void state_thumbnail_exit(void);

void state_thumbnail_init(void)
{
	DBG_PRINT("Thumbnail state init enter\r\n");
	delayed_action_flag = 0;
	ap_thumbnail_init(0);
}

void state_thumbnail_entry(void *para)
{
	EXIT_FLAG_ENUM exit_flag = EXIT_RESUME;
	INT32U msg_id,led_type;
	INT16U play_index;
	STAudioConfirm *audio_temp;
	

	state_thumbnail_init();
	while (exit_flag == EXIT_RESUME) {
		if (msgQReceive(ApQ, &msg_id, (void *) ApQ_para, AP_QUEUE_MSG_MAX_LEN) == STATUS_FAIL) {
			continue;
		}

		switch (msg_id) {
			case EVENT_APQ_ERR_MSG:
				audio_temp = (STAudioConfirm *)ApQ_para;
				if ((audio_temp->result == AUDIO_ERR_DEC_FINISH) && (audio_temp->source_type == AUDIO_SRC_TYPE_APP_RS)){
					//gpio_write_io(SPEAKER_EN, DATA_LOW);
				} else {
					audio_confirm_handler((STAudioConfirm *)ApQ_para);
				}
				break;
			case MSG_STORAGE_SERVICE_MOUNT:
				delayed_action_flag = 0;
        		ap_state_handling_storage_id_set(ApQ_para[0]);
				ap_state_handling_str_draw_exit();
				ap_thumbnail_exit();
        		ap_thumbnail_init(1);
        		DBG_PRINT("[State Thumbnail Mount OK]\r\n");
        		break;
        	case MSG_STORAGE_SERVICE_NO_STORAGE:
				delayed_action_flag = 0;
				g_thumbnail_reply_action_flag = 1;
        		ap_state_handling_storage_id_set(ApQ_para[0]);
        		ap_thumbnail_sts_set(THUMBNAIL_UNMOUNT);
        		ap_thumbnail_stop_handle();
        		ap_thumbnail_no_media_show(STR_NO_SD);
        		DBG_PRINT("[State Thumbnail Mount FAIL]\r\n");
        		break;
        	case MSG_APQ_POWER_KEY_ACTIVE:
        		if (ApQ_para[0] != 0) led_power_off();
        		ap_state_handling_power_off(0);
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
        	case MSG_APQ_MODE:
				if(g_thumbnail_reply_action_flag) {
	        		OSQPost(StateHandlingQ, (void *) STATE_VIDEO_RECORD);
	        		exit_flag = EXIT_BREAK;
	        	}
        		break;

        	case MSG_APQ_HDMI_PLUG_IN:
				if(g_thumbnail_reply_action_flag) {
	        		play_index = ap_thumbnail_func_key_active();
	        		OSQPost(StateHandlingQ, (void *) ((play_index<<16) | STATE_BROWSE) );
					msgQSend(ApQ, MSG_APQ_HDMI_PLUG_IN, NULL, NULL, MSG_PRI_NORMAL);
	        		exit_flag = EXIT_BREAK;
	        	} else {
					delayed_action_flag = 3;
	        	}
	        	break;

        	case MSG_APQ_FUNCTION_KEY_ACTIVE:
				if(g_thumbnail_reply_action_flag) {
	        		play_index = ap_thumbnail_func_key_active();
	        		OSQPost(StateHandlingQ, (void *) ((play_index<<16) | STATE_BROWSE) );
	        		exit_flag = EXIT_BREAK;
	        	}
        		break;

        	case MSG_STORAGE_SERVICE_BROWSE_REPLY:
        		ap_thumbnail_reply_action((STOR_SERV_PLAYINFO *) ApQ_para);
        		if(g_thumbnail_reply_action_flag) {
        			if(delayed_action_flag == 1) {
						msgQSend(ApQ, MSG_APQ_TV_PLUG_IN, NULL, NULL, MSG_PRI_NORMAL);
        			} else if(delayed_action_flag == 2) {
						msgQSend(ApQ, MSG_APQ_TV_PLUG_OUT, NULL, NULL, MSG_PRI_NORMAL);
        			} else if(delayed_action_flag == 3) {
		        		play_index = ap_thumbnail_func_key_active();
		        		OSQPost(StateHandlingQ, (void *) ((play_index<<16) | STATE_BROWSE) );
						msgQSend(ApQ, MSG_APQ_HDMI_PLUG_IN, NULL, NULL, MSG_PRI_NORMAL);
		        		exit_flag = EXIT_BREAK;
        			}
        		}
        		break;
        	case MSG_APQ_NEXT_KEY_ACTIVE:
				if(g_thumbnail_reply_action_flag) {
	        		ap_thumbnail_next_key_active(ApQ_para[0]);
	        	}
        		break;
        	case MSG_APQ_PREV_KEY_ACTIVE:
				if(g_thumbnail_reply_action_flag) {
	        		ap_thumbnail_prev_key_active(ApQ_para[0]);
	        	}
        		break;
        	case MSG_APQ_CONNECT_TO_PC:
				if(ap_display_get_device() != DISP_DEV_TFT) break;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TV_POLLING_STOP, NULL, NULL, MSG_PRI_NORMAL);
				OSTimeDly(3);

        		play_index = ap_thumbnail_func_key_active();
        		OSQPost(StateHandlingQ, (void *) ((play_index<<16) | STATE_BROWSE) );
        		exit_flag = EXIT_BREAK;
        		break;
#if C_SCREEN_SAVER == CUSTOM_ON
			case MSG_APQ_KEY_IDLE:
        		ap_state_handling_lcd_backlight_switch(0);         	
        		break;
			case MSG_APQ_KEY_WAKE_UP:			
        		ap_state_handling_lcd_backlight_switch(1);        	
        		break;        		
#endif        		
        	case MSG_APQ_MENU_KEY_ACTIVE:
        		break;


#if C_BATTERY_DETECT == CUSTOM_ON        	
        	case MSG_APQ_BATTERY_LOW_SHOW:
 //       		ap_state_handling_clear_all_icon();
        		OSTimeDly(5);
				ap_state_handling_str_draw_exit();
				ap_state_handling_str_draw(STR_BATTERY_LOW, WARNING_STR_COLOR);
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_BATTERY_LOW_BLINK_START, NULL, NULL, MSG_PRI_NORMAL);
        		break;        		        		
#endif  

			case MSG_APQ_USER_CONFIG_STORE:
				ap_state_config_store();
				break;

			case MSG_APQ_TV_PLUG_OUT:
			case MSG_APQ_TV_PLUG_IN:
				if(g_thumbnail_reply_action_flag) {
					ap_thumbnail_exit();
					ap_state_handling_str_draw_exit();
					OSTimeDly(20);

					delayed_action_flag = 0;

					if(msg_id == MSG_APQ_TV_PLUG_IN) {
						ap_state_handling_tv_init();
					} else {
						ap_state_handling_tv_uninit();
					}
					OSTimeDly(20);
					ap_thumbnail_init(0);
					OSTimeDly(20);
				} else {
					if(msg_id == MSG_APQ_TV_PLUG_IN) {
						delayed_action_flag = 1;
					} else {
						delayed_action_flag = 2;
					}
				}
				break;

			default:
				ap_state_common_handling(msg_id);
				break;
		}
	}

	state_thumbnail_exit();
	if(msg_id == MSG_APQ_CONNECT_TO_PC){
		msgQFlush(ApQ);
	    msgQSend(ApQ, MSG_APQ_CONNECT_TO_PC, NULL, NULL, MSG_PRI_NORMAL);
	} else if(msg_id == MSG_APQ_MODE) {
		OSTimeDly(10);
		frame_mode_en = 0;
		ap_video_capture_mode_switch(0,STATE_VIDEO_RECORD);
	}
}

void state_thumbnail_exit(void)
{
	delayed_action_flag = 0;
	ap_thumbnail_exit();
	ap_state_handling_str_draw_exit();
	DBG_PRINT("Exit Thumbnail state\r\n");
}
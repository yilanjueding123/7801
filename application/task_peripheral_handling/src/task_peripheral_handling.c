#include "task_peripheral_handling.h"
#include "ap_state_config.h"
#include "ap_browse.h"
#include "ap_display.h"
#include "ap_peripheral_handling.h"

//+++
#include "LDWs.h"
#if (Enable_Lane_Departure_Warning_System)

extern LDWsParameter LDWPar;
extern INT32U LDWS_buf_addrs;

INT32U LDWS_Play_Voice_Start_Time = 0;
INT8U  LDWS_Show_Dbg_Msg = 0;
INT8U  LDWS_Key_On_flag = 0;
#endif
//---

#if PRINTF_WIFI_SPEED_ENABLE
	extern INT32U Wifi_Jpeg_Count;
	extern INT32U Wifi_Jpeg_Data_Byte;
	extern INT32U Wifi_Jpeg_Start_Time;
	extern void WifiSpeedBufUpdate(INT32U wifiSpeed, INT32U wifiDataRate);
#endif


const INT16U photo_timercnt[5]={0,3*32,5*32,10*32,20*32};//second

MSG_Q_ID PeripheralTaskQ = NULL;
void *peripheral_task_q_stack[PERIPHERAL_TASK_QUEUE_MAX];
static INT8U peripheral_para[PERIPHERAL_TASK_QUEUE_MAX_MSG_LEN] = {0};
INT8U screen_saver_enable = 0;
INT32U battery_charge_icon_blink_cnt = 0;
INT32U battery_low_blink_cnt = 0;
INT32U display_insert_sdc_cnt = 0;
INT32U motion_detect_peripheral_cnt = 0;
INT32U G_sensor_power_on_time = 0;

INT32U scaler_stopped_timer = 0;
INT32U sensor_error_power_off_timer = 0;
INT8U  usb_charge_cnt=0;
INT32U cdsp_overflow_count = 0;

#if TV_DET_ENABLE
extern INT8U tv_debounce_cnt;
#endif
extern volatile INT8U g_sequence_cnt_flag;
extern INT8U nWifiSta;

extern INT8U PreviewIsStopped;
extern INT8U adp_status;
extern INT32U gp_ae_awb_process(void);
extern void gp_isp_iso_set(INT32U iso);
void set_led_mode(LED_MODE_ENUM mode);

void task_peripheral_handling_init(void)
{
	if (PeripheralTaskQ==NULL) {
		PeripheralTaskQ = msgQCreate(PERIPHERAL_TASK_QUEUE_MAX, PERIPHERAL_TASK_QUEUE_MAX, PERIPHERAL_TASK_QUEUE_MAX_MSG_LEN);
	}
	ap_peripheral_init();

	
}
INT8U volatile card_space_less_flag=0;
extern INT8U usb_state_get(void);
//extern void mm_dump_to_file(void);
extern void ap_peripheral_charge_det(void);
void task_peripheral_handling_entry(void *para)
{
	INT32U msg_id, power_on_auto_rec_delay_cnt,photo_sequence_cnt,type;
	INT8U usb_detect_start, tick = 0, tv_polling_start = 0;
	INT8U aeawb_start,photo_tag;
	INT32U ret = 0;
	INT8U volume_show_time;
	static INT32U t = 0;
	static INT32U wifi_state_cnt = 0;

	aeawb_start = 1;
	volume_show_time = 0;
	usb_detect_start = 0;
	photo_sequence_cnt=0;

	if (PeripheralTaskQ==NULL) {
		PeripheralTaskQ = msgQCreate(PERIPHERAL_TASK_QUEUE_MAX, PERIPHERAL_TASK_QUEUE_MAX, PERIPHERAL_TASK_QUEUE_MAX_MSG_LEN);
	}
	while(1) {
		if(msgQReceive(PeripheralTaskQ, &msg_id, peripheral_para, PERIPHERAL_TASK_QUEUE_MAX_MSG_LEN) == STATUS_FAIL) {
			continue;
		}
        switch (msg_id) {
        	case MSG_PERIPHERAL_TASK_KEY_REGISTER:
        		ap_peripheral_key_register(peripheral_para[0]);
        		break;

        	case MSG_PERIPHERAL_TASK_USBD_DETECT_INIT:
        		usb_detect_start = 1;
				power_on_auto_rec_delay_cnt = 2*128/PERI_TIME_INTERVAL_AD_DETECT;	//2s				
        		break;

        	case MSG_PERIPHERAL_TASK_LED_SET:
        		//ap_peripheral_led_set(peripheral_para[0]);
        		set_led_mode(peripheral_para[0]);
				if(peripheral_para[0] ==LED_CARD_NO_SPACE)
					card_space_less_flag =1;
        		break;
        	case MSG_PERIPHERAL_TASK_LED_FLASH_SET:
				//ap_peripheral_led_flash_set();
        		break;
			case MSG_PERIPHERAL_TASK_LED_BLINK_SET:
				//ap_peripheral_led_blink_set();
				break;
			
			case MSG_PERIPHERAL_TASK_BATTERY_CHARGE_ICON_BLINK_START:
				battery_charge_icon_blink_cnt = 0x002f;
				break;
			case MSG_PERIPHERAL_TASK_BATTERY_CHARGE_ICON_BLINK_STOP:
				battery_charge_icon_blink_cnt = 0;
				break;
				
			case MSG_PERIPHERAL_TASK_BATTERY_LOW_BLINK_START:
				battery_low_blink_cnt = 0x00008; //0x001f; //间隔时间, 250ms 
				battery_low_blink_cnt |= 0x00900;//闪烁次数,  60次,   120/2
				break;
			case MSG_PERIPHERAL_TASK_BATTERY_LOW_BLINK_STOP:
				if(battery_low_blink_cnt){
					battery_low_blink_cnt=0;
				}
				break;
			case MSG_PERIPHERAL_TASK_DISPLAY_PLEASE_INSERT_SDC:
				display_insert_sdc_cnt = 0x006f;
				break;
			case MSG_PERIPHERAL_TASK_TIMER_VOLUME_ICON_SHOW:
				volume_show_time = peripheral_para[0]*20;
				break;
			
			case MSG_PERIPHERAL_TASK_G_SENSOR_POWER_ON_START:
				G_sensor_power_on_time = 0x0300;
				break;

#if USE_ADKEY_NO
        	case MSG_PERIPHERAL_TASK_AD_DETECT_CHECK:
        		ap_peripheral_key_judge();
				//-----------------------------------------------------------------------
				//if((s_usbd_pin)||(usb_state_get() == 1)||((usb_state_get()==2) && (ap_sd_status_get() == 0) && (wifi_status_get()!= 1)))
				if((s_usbd_pin)||(usb_state_get() == 1))
				{
					if(++usb_charge_cnt >31)
				 	{
			          	ap_peripheral_charge_det();
					 	usb_charge_cnt=0;
				 	}
				}
				
				if ((wifi_status_get() == 1)&&(usb_state_get() != 1))
				{
					if (++wifi_state_cnt >= 128)
					{
						wifi_state_cnt = 0;
						nWifiSta = ssv_wifi_module_conn_status();
					}
				}
				//-----------------------------------------------------------------------
        		#if 1 //GPDV_BOARD_VERSION != GPCV4247_WIFI
        		ap_peripheral_ad_key_judge();
        		#endif
        		ap_peripheral_config_store();

        		if(usb_detect_start == 1) {
        			ap_peripheral_adaptor_out_judge();
					//ap_peripheral_hdmi_detect();
        			if(power_on_auto_rec_delay_cnt) {
        				power_on_auto_rec_delay_cnt -= 1;
        				if(!power_on_auto_rec_delay_cnt) {
							// modify by josephhsieh@20150902
							/*
							if (usb_state_get() == 0)
							{
								if (ap_state_handling_storage_id_get()!=NO_STORAGE)
									msgQSend(ApQ, MSG_APQ_POWER_ON_AUTO_RECORD, NULL, NULL, MSG_PRI_NORMAL);
        					}
        					else if (usb_state_get() == 2)
        					{
        						ChargeMode_set(1); //充电模式
        					}
        					*/
        				}
        			}

					sensor_error_power_off_timer++;
					if(sensor_error_power_off_timer > 5*128/PERI_TIME_INTERVAL_AD_DETECT) { //5s
						sensor_error_power_off_timer = 0;

						if(cdsp_overflow_count > 5*3) { //over 3 times per second
							DBG_PRINT("cdsp overflow %d, Auto Power Off...\r\n", cdsp_overflow_count);
							msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, NULL, NULL, MSG_PRI_URGENT);
							//msgQSend(ApQ, MSG_APQ_SYS_RESET, NULL, NULL, MSG_PRI_URGENT);
						}
						cdsp_overflow_count = 0;
					}

					if(PreviewIsStopped == 0) {
						if(scaler_stopped_timer > 5*128/PERI_TIME_INTERVAL_AD_DETECT) {
							scaler_stopped_timer = 0;

							DBG_PRINT("scaler is stopped, Auto Power Off...\r\n");
							msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, NULL, NULL, MSG_PRI_URGENT);
							//msgQSend(ApQ, MSG_APQ_SYS_RESET, NULL, NULL, MSG_PRI_URGENT);
						} else {
							scaler_stopped_timer++;
						}
					}
        		}

				ap_TFT_backlight_tmr_check();
/*
        		tick++;
        		if (tick & 0x07) { // once per 250ms
					if(tv_polling_start) {
    	    			msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_TV_DETECT, NULL, NULL, MSG_PRI_NORMAL);
					}
        		}
        		
				if((tick & 0x1f) == 0) { //once per second
					if((usb_detect_start == 1) && (power_on_auto_rec_delay_cnt == 0)) {
		        		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_READ_GSENSOR, NULL, NULL, MSG_PRI_NORMAL);
		        	}
				}
*/
        		if (battery_charge_icon_blink_cnt){
        			battery_charge_icon_blink_cnt--;
    				if(!(battery_charge_icon_blink_cnt&0xfff)){
    					if(battery_charge_icon_blink_cnt&0x8000){
    						battery_charge_icon_blink_cnt&= ~0x8000;
    						ap_state_handling_icon_show_cmd(ICON_BATTERY_CHARGED, NULL, NULL);
    					}else{
    						battery_charge_icon_blink_cnt|= 0x8000;
    						ap_state_handling_icon_clear_cmd(ICON_BATTERY_CHARGED, NULL, NULL);
    					}

  						ap_browse_display_update_command();
    					battery_charge_icon_blink_cnt |= 0x002f;
    				}
        		}
        		if(battery_low_blink_cnt){
        			battery_low_blink_cnt --;
        			if(!(battery_low_blink_cnt&0x00ff)){
         				if(battery_low_blink_cnt&0xff00){
         					battery_low_blink_cnt-= 0x0100;
         				}else{
         					DBG_PRINT("Battery low Power off!\r\n");
         					type = 0;
							msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, &type, sizeof(INT32U), MSG_PRI_URGENT);
        					battery_low_blink_cnt = 0;
         					break;
         				}
         				if ((battery_low_blink_cnt >> 8) >= 8)
         				{
         					led_all_off();
         					led_wifi_off();
         				}
         				else
         				{
         					led_red_on();
         				}
         				
         				/*
	       				if(battery_low_blink_cnt&0x80000){
         					battery_low_blink_cnt&= ~0x80000;
         					//ap_state_handling_str_draw(STR_BATTERY_LOW, WARNING_STR_COLOR);
       						led_blue_off();
       						led_red_on();
       					}else{
        					battery_low_blink_cnt|= 0x80000;
        					//ap_state_handling_str_draw_exit();
       						led_blue_off();
       						led_red_on();
        				}
        				*/
        				battery_low_blink_cnt |= 0x0008;
        			}
        		}
        		/*
        		if(display_insert_sdc_cnt){
        			display_insert_sdc_cnt--;
        			if(!display_insert_sdc_cnt){
        				ap_state_handling_str_draw_exit();
        			}
        		}
        		*/
			  #if C_MOTION_DETECTION == CUSTOM_ON
        		if(motion_detect_peripheral_cnt){
        			motion_detect_peripheral_cnt--;
        			if(!motion_detect_peripheral_cnt){
        				ap_peripheral_motion_detect_judge();
        				motion_detect_peripheral_cnt= 2;
        			}
        		}
			  #endif
/*
				if(volume_show_time){
					volume_show_time -- ;
					if(!volume_show_time){
	 					msgQSend(ApQ, MSG_APQ_VOLUME_SHOW_END, NULL, NULL, MSG_PRI_NORMAL);
	 				}
 				}
*/
				if(aeawb_start) {
					ret = gp_ae_awb_process();
					if(ret != 0) {
						//DBG_PRINT(".");
					}
					else {
						//DBG_PRINT("\r\nae, awb process done!! \r\n");
						//DBG_PRINT("*");
					}
				}	
/*        		
				if(g_sequence_cnt_flag==1)
				{
					photo_tag = ap_state_config_burst_get();
					photo_sequence_cnt++;
					if(photo_sequence_cnt>=photo_timercnt[photo_tag])	
					{
						g_sequence_cnt_flag=0;
						photo_sequence_cnt=0;
						msgQSend(ApQ, MSG_APQ_AUDIO_EFFECT_CAMERA, NULL, NULL, MSG_PRI_NORMAL);
						msgQSend(ApQ, MSG_APQ_CAPTURE_CONTINUOUS_SHOOTING, NULL, NULL, MSG_PRI_NORMAL);
					}
				}
				else
				{
					if(photo_sequence_cnt!=0)
					{
						photo_sequence_cnt=0;
					}
				}	
*/

				#if PRINTF_WIFI_SPEED_ENABLE	
				if(Wifi_Jpeg_Start_Time > 0)
				{
					if((OSTimeGet()-Wifi_Jpeg_Start_Time) >= 100)
					{
						DBG_PRINT(">JC:%d fps %d KB \r\n",Wifi_Jpeg_Count,(Wifi_Jpeg_Data_Byte>>10));
						WifiSpeedBufUpdate(Wifi_Jpeg_Count,Wifi_Jpeg_Data_Byte);
						Wifi_Jpeg_Count = 0;
						Wifi_Jpeg_Data_Byte = 0;
						Wifi_Jpeg_Start_Time += 100;
					}									
				}
       			#endif
       			
				break;
#endif

            case MSG_PERIPHERAL_TV_POLLING_START:
                tv_polling_start = 1;
			#if TV_DET_ENABLE
               	tv_debounce_cnt = 0;
            #endif
                break;

            case MSG_PERIPHERAL_TV_POLLING_STOP:
                tv_polling_start = 0;
			#if TV_DET_ENABLE
               	tv_debounce_cnt = 0;
            #endif
                break;

			case MSG_PERIPHERAL_TASK_TV_DETECT:
				ap_peripheral_tv_detect();
				break;

			case MSG_PERIPHERAL_TASK_READ_GSENSOR:
				ap_peripheral_read_gsensor();
				break;

#if C_MOTION_DETECTION == CUSTOM_ON
//        	case MSG_PERIPHERAL_TASK_MOTION_DETECT_JUDGE:
//        		ap_peripheral_motion_detect_judge();
//        		break;
        	case MSG_PERIPHERAL_TASK_MOTION_DETECT_START:
        		ap_peripheral_motion_detect_start();
        		motion_detect_peripheral_cnt= 32;
        		break;
        	case MSG_PERIPHERAL_TASK_MOTION_DETECT_STOP:
        		ap_peripheral_motion_detect_stop();
        		motion_detect_peripheral_cnt= 0;
        		break;
        	case MSG_PERIPHERAL_TASK_MOTION_DETECT_DELAY:
        		motion_detect_peripheral_cnt= 0xff;
        		break;
#endif
#if C_SCREEN_SAVER == CUSTOM_ON
			case MSG_PERIPHERAL_TASK_LCD_BACKLIGHT_SET:
			ap_peripheral_clr_screen_saver_timer();
			ap_peripheral_lcd_backlight_set(peripheral_para[0]);
        		break;
			case MSG_PERIPHERAL_TASK_SCREEN_SAVER_ENABLE:
        		screen_saver_enable = 1;
        		break;
#endif
			case MSG_PERIPHERAL_TASK_NIGHT_MODE_SET:
        		ap_peripheral_night_mode_set(peripheral_para[0]);
				break;

        	case MSG_PERIPHERAL_USBD_EXIT:
        		usbd_exit = 1;
        		break;

		case MSG_PERIPHERAL_TASK_ISP_ISO_SET:
			gp_isp_iso_set(peripheral_para[0]);
			break;

		#if (Enable_Lane_Departure_Warning_System)
		case MSG_LDWS_DO:
		
//		  	gpio_write_io(IO_B15, 1);	

			LDWSmoothSobel_720P_GP420( (unsigned char*)LDWS_buf_addrs, LDWPar.sobelImg_ptr, LDWPar.workingImg_W, LDWPar.workingImg_H, &LDWPar); //get input image buffer
			houghTransform( LDWPar.sobelImg_ptr, LDWPar.workingImg_W, LDWPar.workingImg_H, LDWPar.countTable_ptr, &LDWPar);
			LDWs( LDWPar.countTable_ptr, LDWPar.ROI, &LDWPar);
			LDWTurnKey( &LDWPar, LDWPar.sobelImg_ptr, LDWPar.workingImg_W, LDWPar.workingImg_H,  LDWPar.ROI );				
			dynamicROI(&LDWPar);
			
//		  	gpio_write_io(IO_B15, 0);

		  	LDWS_Show_Dbg_Msg++;
		  	if(LDWS_Show_Dbg_Msg >= 5)
		  	{
#if 0
DBG_PRINT("*\r\n");
//DBG_PRINT(" changeLaneCntThre = %d\r\n",  LDWPar.changeLaneCntThre);
DBG_PRINT("(dotLineFlg_L, dotLineFlg_R) = (%d, %d) \r\n", LDWPar.dotLineFlg_L, LDWPar.dotLineFlg_R);
//DBG_PRINT("tunnelCnt: %d\r\n",LDWPar.tunnelCnt);
//DBG_PRINT("LDW_leaveFreewayCnt: %d\r\n",LDWPar.LDW_leaveFreewayCnt);
DBG_PRINT("LaneWidth = %d\r\n", LDWPar.LaneWidth);
DBG_PRINT(" (bestROI_X, bestROI_Y) = (%d, %d)  \r\n", LDWPar.bestROI_X, LDWPar.bestROI_Y);
DBG_PRINT("On (L, R) = (%d, %d), Off (L, R) = (%d, %d) \r\n", LDWPar.checkTurnOnCnt_L, LDWPar.checkTurnOnCnt_R , LDWPar.checkTurnOffCnt_L, LDWPar.checkTurnOffCnt_R);
//DBG_PRINT("checkTurnOnCntThre: %d \r\n",  LDWPar.checkTurnOnCntThre);
//DBG_PRINT("WarningCheckCnt: ( %d, %d ) \r\n", LDWPar.LLWarningCheckCnt, LDWPar.RLWarningCheckCnt);
DBG_PRINT("WarningCheckCntThre: ( %d, %d ) \r\n", LDWPar.LLWarningCheckCntThre, LDWPar.RLWarningCheckCntThre);
//DBG_PRINT("changeLaneFlg: %d \r\n", LDWPar.changeLaneFlg);
//DBG_PRINT("BLThre: %d , BRThre: %d\r\n",LDWPar.BLThre, LDWPar.BRThre);
DBG_PRINT("getMax: %d , getSec: %d\r\n",LDWPar.getMax, LDWPar.getSec);
//DBG_PRINT("LLineEnergyThre: %d , RLineEnergyThre: %d\r\n",LDWPar.LLineEnergyThre, LDWPar.RLineEnergyThre);
DBG_PRINT("LLDetectCnt: %d , LLDetectCnt: %d\r\n",LDWPar.LLDetectCheckCnt, LDWPar.RLDetectCheckCnt);
DBG_PRINT("ROIX: %d , ROIY: %d\r\n",LDWPar.ROI.x, LDWPar.ROI.y);
#endif		
				LDWS_Show_Dbg_Msg = 0;
			}


			if(LDWPar.LDW_keyFlg && !LDWS_Key_On_flag)
			{

				ap_state_handling_icon_show_cmd(ICON_VIDEO_LDW_SART, NULL, NULL);

				LDWS_Key_On_flag = 1;
				
				if((OSTimeGet()- LDWS_Play_Voice_Start_Time) >= 50)
				{
					if(ap_LDW_get_from_config(LDW_ONOFF_SOUND))
						audio_effect_play(EFFECT_LDW_TurnOn);
					LDWS_Play_Voice_Start_Time = OSTimeGet();
				}
			}
			else if(!LDWPar.LDW_keyFlg && LDWS_Key_On_flag)
			{

				ap_state_handling_icon_clear_cmd(ICON_VIDEO_LDW_SART, NULL, NULL);

				LDWS_Key_On_flag = 0;

				if((OSTimeGet()- LDWS_Play_Voice_Start_Time) >= 50)
				{
					if(ap_LDW_get_from_config(LDW_ONOFF_SOUND))
						audio_effect_play(EFFECT_LDW_TurnOff);
					LDWS_Play_Voice_Start_Time = OSTimeGet();
				}
			}

			if(LDWPar.LDW_keyFlg && LDWPar.LDWsAlarmFlg)
			{

				if((OSTimeGet()- LDWS_Play_Voice_Start_Time) >= 50)
				{
					audio_effect_play(EFFECT_LDW_Alarm);
					LDWS_Play_Voice_Start_Time = OSTimeGet();
				}
			}

			#endif
		break;
		
        	default:
        		break;
        }
    }
}

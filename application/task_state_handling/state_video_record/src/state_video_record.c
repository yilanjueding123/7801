#include "ap_state_config.h"
#include "state_video_record.h"
#include "ap_state_handling.h"
#include "ap_video_record.h"
#include "ap_display.h"
#include "avi_encoder_app.h"
#include "my_avi_encoder_state.h"
#include "state_wifi.h"

#include "my_video_codec_callback.h"

#include "LDWs.h"
#if (Enable_Lane_Departure_Warning_System)
INT8U LDWS_Enable_Flag = 0;
INT32S*	LDWs_workmem = NULL;	
extern LDWsParameter LDWPar;
extern INT8U ap_LDW_get_from_config(INT8U LDW_choice);
#endif

#if C_MOTION_DETECTION == CUSTOM_ON	
extern INT8U ap_peripheral_md_mode_get(void);
#endif
//	prototypes
INT8S state_video_record_init(INT32U state);
void state_video_record_exit(void);

extern void ap_video_record_md_icon_update(INT8U sts);
extern void ap_video_record_md_icon_clear_all(void);
extern void ap_video_capture_mode_switch(INT8U DoSensorInit, INT16U EnterAPMode);

extern STOR_SERV_FILEINFO curr_file_info;
extern STOR_SERV_FILEINFO next_file_info;

extern INT8U g_lock_current_file_flag;
extern INT8U g_cycle_record_continuing_flag;

extern INT8U ext_rtc_pwr_on_flag;

extern INT32U g_wifi_sd_full_flag;

#if (Enable_Lane_Departure_Warning_System)

LDW_DISPLAY ldw_dis;
INT32U red_line_postion;
INT32U ldw_line_postion_L_x;
INT32U ldw_line_postion_L_y;
INT32U ldw_line_postion_R_x;
INT32U ldw_line_postion_R_y;
void ap_get_ldw_display_coordinate(void)
{
	//coordinate transform
	ldw_dis.ldw_dis_LT_alarmP_X = divFun((LDWPar.LT_alarmP_X*LDWPar.scaleRate_F), 10) + LDWPar.ROI.x;
	ldw_dis.ldw_dis_LT_alarmP_Y = divFun((LDWPar.LT_alarmP_Y*LDWPar.scaleRate_F), 10) + LDWPar.ROI.y + LDWPar.ROI_initY;// + 10;
	ldw_dis.ldw_dis_LB_alarmP_X = divFun((LDWPar.LB_alarmP_X*LDWPar.scaleRate_F), 10) + LDWPar.ROI.x; 
	ldw_dis.ldw_dis_LB_alarmP_Y = divFun((LDWPar.LB_alarmP_Y*LDWPar.scaleRate_F), 10) + LDWPar.ROI.y + LDWPar.ROI_initY;//+ 10;
	ldw_dis.ldw_dis_LTP_X = divFun((LDWPar.LTP_X*LDWPar.scaleRate_F), 10) + LDWPar.ROI.x;
	ldw_dis.ldw_dis_LTP_Y = divFun((LDWPar.LTP_Y*LDWPar.scaleRate_F), 10) + LDWPar.ROI.y + LDWPar.ROI_initY;//+ 10;
	ldw_dis.ldw_dis_LBP_X = divFun((LDWPar.LBP_X*LDWPar.scaleRate_F), 10) + LDWPar.ROI.x;
	ldw_dis.ldw_dis_LBP_Y = divFun((LDWPar.LBP_Y*LDWPar.scaleRate_F), 10) + LDWPar.ROI.y + LDWPar.ROI_initY;//+ 10;

	ldw_dis.ldw_dis_RT_alarmP_X = divFun((LDWPar.RT_alarmP_X*LDWPar.scaleRate_F), 10) + LDWPar.ROI.x;
	ldw_dis.ldw_dis_RT_alarmP_Y = divFun((LDWPar.RT_alarmP_Y*LDWPar.scaleRate_F), 10) + LDWPar.ROI.y + LDWPar.ROI_initY;//+ 10;
	ldw_dis.ldw_dis_RB_alarmP_X = divFun((LDWPar.RB_alarmP_X*LDWPar.scaleRate_F), 10) + LDWPar.ROI.x; 
	ldw_dis.ldw_dis_RB_alarmP_Y = divFun((LDWPar.RB_alarmP_Y*LDWPar.scaleRate_F), 10) + LDWPar.ROI.y + LDWPar.ROI_initY;//+ 10;
	ldw_dis.ldw_dis_RTP_X = divFun((LDWPar.RTP_X*LDWPar.scaleRate_F), 10) + LDWPar.ROI.x;
	ldw_dis.ldw_dis_RTP_Y = divFun((LDWPar.RTP_Y*LDWPar.scaleRate_F), 10) + LDWPar.ROI.y + LDWPar.ROI_initY;//+ 10;
	ldw_dis.ldw_dis_RBP_X = divFun((LDWPar.RBP_X*LDWPar.scaleRate_F), 10) + LDWPar.ROI.x;
	ldw_dis.ldw_dis_RBP_Y = divFun((LDWPar.RBP_Y*LDWPar.scaleRate_F), 10) + LDWPar.ROI.y + LDWPar.ROI_initY;//+ 10;


	ldw_dis.ldw_dis_LT_alarmP_X = ldw_dis.ldw_dis_LT_alarmP_X*TFT_WIDTH/1280;
	ldw_dis.ldw_dis_LT_alarmP_Y = ldw_dis.ldw_dis_LT_alarmP_Y*TFT_HEIGHT/720;
	ldw_dis.ldw_dis_LB_alarmP_X = ldw_dis.ldw_dis_LB_alarmP_X*TFT_WIDTH/1280;
	ldw_dis.ldw_dis_LB_alarmP_Y = ldw_dis.ldw_dis_LB_alarmP_Y*TFT_HEIGHT/720;
	ldw_dis.ldw_dis_LTP_X = ldw_dis.ldw_dis_LTP_X*TFT_WIDTH/1280;
	ldw_dis.ldw_dis_LTP_Y = ldw_dis.ldw_dis_LTP_Y*TFT_HEIGHT/720;
	ldw_dis.ldw_dis_LBP_X = ldw_dis.ldw_dis_LBP_X*TFT_WIDTH/1280;
	ldw_dis.ldw_dis_LBP_Y = ldw_dis.ldw_dis_LBP_Y*TFT_HEIGHT/720;

	ldw_dis.ldw_dis_RT_alarmP_X = ldw_dis.ldw_dis_RT_alarmP_X*TFT_WIDTH/1280;
	ldw_dis.ldw_dis_RT_alarmP_Y = ldw_dis.ldw_dis_RT_alarmP_Y*TFT_HEIGHT/720;
	ldw_dis.ldw_dis_RB_alarmP_X = ldw_dis.ldw_dis_RB_alarmP_X*TFT_WIDTH/1280;
	ldw_dis.ldw_dis_RB_alarmP_Y = ldw_dis.ldw_dis_RB_alarmP_Y*TFT_HEIGHT/720;
	ldw_dis.ldw_dis_RTP_X = ldw_dis.ldw_dis_RTP_X*TFT_WIDTH/1280;
	ldw_dis.ldw_dis_RTP_Y = ldw_dis.ldw_dis_RTP_Y*TFT_HEIGHT/720;
	ldw_dis.ldw_dis_RBP_X = ldw_dis.ldw_dis_RBP_X*TFT_WIDTH/1280;
	ldw_dis.ldw_dis_RBP_Y = ldw_dis.ldw_dis_RBP_Y*TFT_HEIGHT/720;

	#if USE_PANEL_SCREEN_PROPOTION == PANEL_SCREEN_PROPOTION_4_3	//4:3
		ldw_dis.ldw_dis_LT_alarmP_Y = ldw_dis.ldw_dis_LT_alarmP_Y*3/4+30;
		ldw_dis.ldw_dis_LB_alarmP_Y = ldw_dis.ldw_dis_LB_alarmP_Y*3/4+30;
		ldw_dis.ldw_dis_RT_alarmP_Y = ldw_dis.ldw_dis_RT_alarmP_Y*3/4+30;
		ldw_dis.ldw_dis_RB_alarmP_Y = ldw_dis.ldw_dis_RB_alarmP_Y*3/4+30;

		ldw_dis.ldw_dis_LTP_Y = ldw_dis.ldw_dis_LTP_Y*3/4+30;
		ldw_dis.ldw_dis_LBP_Y = ldw_dis.ldw_dis_LBP_Y*3/4+30;
		ldw_dis.ldw_dis_RTP_Y = ldw_dis.ldw_dis_RTP_Y*3/4+30;
		ldw_dis.ldw_dis_RBP_Y = ldw_dis.ldw_dis_RBP_Y*3/4+30;

	#endif
}

void ap_get_ldw_display_freeze_coordinate(void)
{
	red_line_postion = 437*TFT_HEIGHT/720;

	#if USE_PANEL_SCREEN_PROPOTION == PANEL_SCREEN_PROPOTION_16_9
	#else
		red_line_postion = red_line_postion*3/4+30;
	#endif
	
	#if USE_PANEL_NAME == PANEL_T43
	ldw_line_postion_L_x = 123;
	ldw_line_postion_L_y = 226;
	ldw_line_postion_R_x = TFT_WIDTH-ldw_line_postion_L_x;
	ldw_line_postion_R_y = 226;
	#elif USE_PANEL_NAME == PANEL_400X240_I80
	ldw_line_postion_L_x = 103;
	ldw_line_postion_L_y = 210;
	ldw_line_postion_R_x = TFT_WIDTH-ldw_line_postion_L_x;
	ldw_line_postion_R_y = 210;
	#elif USE_PANEL_NAME == PANEL_T27P05_ILI8961
	ldw_line_postion_L_x = 82;
	ldw_line_postion_L_y = 210;
	ldw_line_postion_R_x = TFT_WIDTH-ldw_line_postion_L_x;
	ldw_line_postion_R_y = 210;
	#elif USE_PANEL_NAME == PANEL_T40P00_ILI9342C
	ldw_line_postion_L_x = 46;
	ldw_line_postion_L_y = 210;
	ldw_line_postion_R_x = TFT_WIDTH-ldw_line_postion_L_x;
	ldw_line_postion_R_y = 210;
	#else
	ldw_line_postion_L_x = 46;
	ldw_line_postion_L_y = 210;
	ldw_line_postion_R_x = TFT_WIDTH-ldw_line_postion_L_x;
	ldw_line_postion_R_y = 210;
	#endif	
	
	
	
}

#endif

INT8S state_video_record_init(INT32U state)
{
	DBG_PRINT("video_record state init enter\r\n");
	if (ap_video_record_init(state) == STATUS_OK) {
		return STATUS_OK;
	} else {
		return STATUS_FAIL;
	}
}

static INT8U prv_mode_flag = 0;
INT8U video_to_preview_get(void)
{
	return prv_mode_flag;
}

#if SUPORT_GET_JPG_BUF == CUSTOM_ON
INT32U user_get_jpeg_buf;
INT8U cyc_restart_flag=0;
INT32U user_get_jpeg_lenth;
extern INT32S ap_jpeg_get_one_frame_on_recording(INT32U addr,INT32U* lenth);
#endif	
extern void led_power_off(void);
void state_video_record_entry(void *para, INT32U state)
{
	EXIT_FLAG_ENUM exit_flag = EXIT_RESUME;
	INT32U msg_id,led_type;
    INT8U power_on_flag;
	STAudioConfirm *audio_temp;
	//+++
	#if (Enable_Lane_Departure_Warning_System)
	INT32S	workingMemSzie;
	INT32S	workingImg_W = 288;
	INT32S	workingImg_H = 88;
	#endif
#if C_MOTION_DETECTION == CUSTOM_ON
	INT8U md_check_tick, switch_flag;

	md_check_tick = switch_flag = 0;
#endif


 	power_on_flag = 0;
	
	if(ap_state_handling_storage_id_get()==NO_STORAGE)
	{
		led_type = LED_NO_SDC;
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
	}
	else
	{
		led_type = LED_WAITING_RECORD;
	}


	#if Enable_Lane_Departure_Warning_System
	LDWS_Enable_Flag = 0;
	if(ap_LDW_get_from_config(LDW_ON_OFF) &&((ap_state_config_video_resolution_get() == 0) || (ap_state_config_video_resolution_get() == 2)))
	{
		LDWS_Enable_Flag = 1;
	}
	#endif

	if(*(INT32U *) para == STATE_STARTUP) { // 秨诀秈ㄓ
		//ap_video_capture_mode_switch(1, STATE_VIDEO_RECORD); // 秨诀璶暗sensor init
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_USBD_DETECT_INIT, NULL, NULL, MSG_PRI_NORMAL);

		if(ext_rtc_pwr_on_flag) {
			ext_rtc_pwr_on_flag = 0;
			if(ap_state_config_hang_mode_get() & 0x80) {
				power_on_flag = 1; //auto record
			} else {
				power_on_flag = 0;
			}
		} else {
			power_on_flag = 1; //auto record
		}
	} else if((*(INT32U *) para == STATE_SETTING)|| (*(INT32U *) para == STATE_BROWSE)) { // 垫虫秈ㄓ
		ap_video_capture_mode_switch(0, STATE_VIDEO_RECORD); 
		/*#if DUAL_STREAM_FUNC_ENABLE
		if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
		#endif
		{
			video_encode_preview_on();
		}*/
	}
	else if (*(INT32U *) para ==STATE_VIDEO_PREVIEW)
	{
		ap_video_capture_mode_switch(0, STATE_VIDEO_RECORD); 
		
		#if C_MOTION_DETECTION == CUSTOM_ON	
		if (ap_peripheral_md_mode_get())
			msgQSend(ApQ, MSG_APQ_MD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
		else
		#endif
			msgQSend(ApQ, MSG_APQ_VIDEO_RECORD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
	}
	
	#if SUPORT_GET_JPG_BUF == CUSTOM_ON
	  user_get_jpeg_buf = (INT32U)gp_malloc_align(200*1024, 32);
  	#endif
	state_video_record_init(state);

	//+++
	#if Enable_Lane_Departure_Warning_System
	if(LDWS_Enable_Flag)
	{
		workingMemSzie = LDWs_get_memorySize(workingImg_W, workingImg_H, &LDWPar);
		LDWs_workmem = (INT32S*)gp_malloc_align(workingMemSzie,8);
		LDWPar.carClassMode = ap_LDW_get_from_config(LDW_CAR_TYPE);   // 0 = 免ó, 1 = ヰó, 2 = 疭蔼ó
		LDWPar.countryMode = ap_LDW_get_from_config(LDW_AREA_CHOICE);    // 0 = 芖, 1 = 嘲
		LDWPar.sensitiveMode = ap_LDW_get_from_config(LDW_SENSITIVITY);  // 0 = , 1 = 蔼稰
		LDWPar.turnOnSpeedMode = ap_LDW_get_from_config(LDW_START_SPEED);  // 0 = high, 1 = low
		LDWsInit(LDWs_workmem, &LDWPar);
		
//		ap_get_ldw_display_coordinate();
		ap_get_ldw_display_freeze_coordinate();
	}
	drvl2_sdc_card_long_polling_set(SD_USED_NUM ,LDWS_Enable_Flag);
	#endif
	//---


	while (exit_flag == EXIT_RESUME) {
		if (msgQReceive(ApQ, &msg_id, (void *) ApQ_para, AP_QUEUE_MSG_MAX_LEN) == STATUS_FAIL) {
			continue;
		}
		//DBG_PRINT("record Msg: 0x%X\r\n", msg_id);
		switch (msg_id) {
			case EVENT_APQ_ERR_MSG:
				audio_temp = (STAudioConfirm *)ApQ_para;
				if ((audio_temp->result == AUDIO_ERR_DEC_FINISH) && (audio_temp->source_type == AUDIO_SRC_TYPE_APP_RS)) {
					gpio_write_io(SPEAKER_EN, DATA_LOW);
				} else if((audio_temp->result == AUDIO_ERR_DEC_FINISH) && (audio_temp->source_type == AUDIO_SRC_TYPE_FS)) {
					audio_confirm_handler((STAudioConfirm *)ApQ_para);
				} else {
					audio_confirm_handler((STAudioConfirm *)ApQ_para);
				}
				break;
			case MSG_STORAGE_SERVICE_MOUNT:
        		ap_state_handling_storage_id_set(ApQ_para[0]);
				ap_state_handling_str_draw_exit();
        		//ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
        		//ap_state_handling_icon_show_cmd(ICON_SD_CARD, NULL, NULL);
        		ap_video_record_sts_set(~VIDEO_RECORD_UNMOUNT);
        		video_calculate_left_recording_time_enable();
				if (ap_state_config_md_get()) {
					//ap_state_handling_icon_show_cmd(ICON_MD_STS_0, NULL, NULL);
//					ap_state_handling_icon_clear_cmd(ICON_MOTION_DETECT_START, NULL, NULL);
					//ap_state_handling_icon_show_cmd(ICON_MOTION_DETECT, NULL, NULL);
					//ap_state_handling_icon_show_cmd(ICON_VIDEO_RECORD_MOTION_10SECOND, NULL, NULL);
					//ap_state_handling_icon_clear_cmd(ICON_VIDEO_RECORD_CYC_OFF+ap_state_config_record_time_get(), NULL, NULL);
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_MOTION_DETECT_START, NULL, NULL, MSG_PRI_NORMAL);
					ap_video_record_sts_set(VIDEO_RECORD_MD);
				}
//        		ap_music_update_icon_status();
        		DBG_PRINT("[Video Record Mount OK]\r\n");
        		break;
        	case MSG_STORAGE_SERVICE_NO_STORAGE:
        		led_type = LED_NO_SDC;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
        		ap_state_handling_storage_id_set(ApQ_para[0]);
   				//ap_music_update_icon_status();
				//ap_state_handling_mp3_index_show_zero_cmd();
				//ap_state_handling_mp3_total_index_show_zero_cmd();

   				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_AUTO_DEL_LOCK, NULL, NULL, MSG_PRI_NORMAL);   				
				ap_video_record_error_handle(NULL);	//if it is recording, stop it
#if C_MOTION_DETECTION == CUSTOM_ON
				if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
					//ap_video_record_md_icon_clear_all();
					ap_video_record_md_disable();
					ap_state_config_md_set(0);
				}
#endif
        		//video_calculate_left_recording_time_disable();
        		video_calculate_left_recording_time_enable();
        		ap_state_handling_str_draw_exit();
        		//ap_state_handling_icon_clear_cmd(ICON_SD_CARD, NULL, NULL);
        		//ap_state_handling_icon_show_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);

				g_lock_current_file_flag = 0;
				g_cycle_record_continuing_flag = 0;
				ap_state_handling_icon_clear_cmd(ICON_LOCKED, NULL, NULL);
				ap_state_handling_icon_clear_cmd(ICON_VIDEO_LDW_SART, NULL, NULL);

        		ap_video_record_sts_set(VIDEO_RECORD_UNMOUNT);
				power_on_flag = 0;
        		DBG_PRINT("[Video Record Mount FAIL]\r\n");
        		break;

#if C_MOTION_DETECTION == CUSTOM_ON
			case MSG_APQ_MOTION_DETECT_TICK:
				ap_video_record_md_tick(&md_check_tick,NULL);
				break;
			case MSG_APQ_MOTION_DETECT_ICON_UPDATE:
				ap_video_record_md_icon_update(ApQ_para[0]);
				break;
			case MSG_APQ_MOTION_DETECT_ACTIVE:
				ap_video_record_md_active(&md_check_tick,NULL);
				break;
			case MSG_APQ_MD_ACTIVE:
				if(ap_video_record_sts_get() & VIDEO_RECORD_MD) 
				{//移动侦测模式下
					if (ap_video_record_sts_get() & VIDEO_RECORD_BUSY)
					{//正在录像
						ap_video_record_func_key_active(msg_id);
					}
					//退出移动侦测模式
					ap_video_record_md_disable();
					ap_state_config_md_set(0);
					led_type = LED_WAITING_RECORD; 
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					DBG_PRINT("Stop Motion..\r\n");
					ap_peripheral_auto_off_force_disable_set(0);
				}
				else
				{//非移动侦测模式下
					if (ap_video_record_sts_get() & VIDEO_RECORD_BUSY)
					{//正在录像时
						//ap_video_record_func_key_active(msg_id);
						break;
					}
					else		
					{//开启移动侦测模式
						//移动侦测模式进入时的闪灯
						//led_type = LED_MOTION_READY;
						led_type = LED_MOTION_WAITING;
						msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
						ap_state_config_md_set(1);
					    msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_MOTION_DETECT_START, NULL, NULL, MSG_PRI_NORMAL);
					    ap_video_record_sts_set(VIDEO_RECORD_MD);
					    ap_peripheral_auto_off_force_disable_set(1);
					    DBG_PRINT("Start Motion..\r\n");
					}
				}
				break;	
#endif
        	case MSG_APQ_VIDEO_RECORD_ACTIVE:
        		if(ap_video_record_sts_get() & VIDEO_RECORD_MD) break; //移动侦测模式下, 检测到录像键值时无效处理
				if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)) {
 					//led_type = LED_WAITING_RECORD;
					//msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				}
				else
				{
					//led_type = LED_RECORD_READY;
					//msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				}
	        	ap_video_record_func_key_active(msg_id);
        		break;

			case MSG_APQ_POWER_ON_AUTO_RECORD:
#if C_MOTION_DETECTION == CUSTOM_ON
        		if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
					power_on_flag = 0;
        		}
        		else
#endif
				if(power_on_flag) {
					power_on_flag = 0;
					if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY) == 0)
					{//非录像状态下
						//led_video_reday(); //录像开始时的闪灯
						//led_type = LED_RECORD_READY;
						//msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					}
		        	ap_video_record_func_key_active(msg_id);
		        	DBG_PRINT("AUTO_Record_start!!!!");
		        }
				break;

        	case MSG_APQ_POWER_KEY_ACTIVE:
        	case MSG_APQ_SYS_RESET:
        		if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)) {
        			if(ap_video_record_func_key_active(msg_id)) {
        				break;
        			}
					ap_state_config_hang_mode_set(0x81);	//STATE_VIDEO_RECORD, recording
        		} else {
					ap_state_config_hang_mode_set(0x01);	//STATE_VIDEO_RECORD
        		}
        		OSTimeDly(10);
				video_encode_exit();
        		video_calculate_left_recording_time_disable();

				if(msg_id == MSG_APQ_SYS_RESET) {
	        		ap_state_handling_power_off(1);
	        	} else {
	        		if (ApQ_para[0] != 0) led_power_off();
	        		ap_state_handling_power_off(0);
	        	}
        		break;
			case MSG_APQ_CHARGE_MODE:
				if (ap_video_record_sts_get() & VIDEO_RECORD_BUSY)
				{//停止录像
					ap_video_record_func_key_active(msg_id);
				}
				if(Wifi_Disconnect() == 0)
				{//停止WIFI		
					Wifi_State_Set(0);
				}
				//ChargeMode_set(1);
				led_type = LED_CHARGEING;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				break;
        	case MSG_APQ_MENU_KEY_ACTIVE:
        		if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)) {
        			break;
        			if(ap_video_record_func_key_active(msg_id)) {
        				break;
        			}
        		}
#if C_MOTION_DETECTION == CUSTOM_ON
        		if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
					//ap_video_record_md_icon_clear_all();
					ap_video_record_md_disable();
					ap_state_config_md_set(0);
        		}
#endif
				ap_state_handling_str_draw_exit();
				OSTimeDly(10);	//wait to update display				
				power_on_flag = 0;

				video_encode_preview_off();
 				vid_enc_disable_sensor_clock();
				#if DUAL_STREAM_FUNC_ENABLE
 				if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
 				#endif
 				{
	        		OSQPost(StateHandlingQ, (void *) STATE_SETTING);
		     		exit_flag = EXIT_BREAK;
		     	}
        		break;

			#if DUAL_STREAM_FUNC_ENABLE
			case MSG_APQ_WIFI_MENU_EXIT:
				ap_video_capture_mode_switch(0, STATE_VIDEO_RECORD);   	
				video_encode_preview_on();				
				state_video_record_init(state);
			break;
			#endif


			case MSG_APQ_CAPTURE_KEY_ACTIVE:
	       	case MSG_APQ_MODE:
         		if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)) {
        			//ap_video_record_lock_current_file();
					//ap_state_handling_icon_show_cmd(ICON_LOCKED, NULL, NULL);
					//ap_video_record_func_key_active(msg_id);
					break;
				}
        		else
        		{
#if C_MOTION_DETECTION == CUSTOM_ON
	        		if(ap_video_record_sts_get() & VIDEO_RECORD_MD) { //移动侦测
						//ap_video_record_md_icon_clear_all();
						//ap_video_record_md_disable();
						//ap_state_config_md_set(0);
						break;
	        		}
#endif
					ap_state_handling_str_draw_exit();
			        video_calculate_left_recording_time_disable();
					OSTimeDly(3);
					power_on_flag = 0;

					if(avi_encoder_state_get() & AVI_ENCODER_STATE_SENSOR)
					{
						video_preview_close(0);
						avi_encoder_state_clear(AVI_ENCODER_STATE_SENSOR);
					}				
					if (msg_id == MSG_APQ_MODE) prv_mode_flag = 0;
					else prv_mode_flag = 1;
	        		OSQPost(StateHandlingQ, (void *) STATE_VIDEO_PREVIEW);
	        		exit_flag = EXIT_BREAK;
        		}
				break;

#if C_AUTO_DEL_FILE == CUSTOM_ON			
			case MSG_APQ_FREE_FILESIZE_CHECK_REPLY:
				if(*((INT32S *) ApQ_para) == STATUS_OK) {
					ap_video_record_start();
				} else {
					ap_state_handling_str_draw_exit();
					ap_state_handling_str_draw(STR_SD_FULL, WARNING_STR_COLOR);
					g_wifi_sd_full_flag = 1;
					video_calculate_left_recording_time_disable();
					led_type = LED_SDC_FULL;
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				}
				break;
#endif
        	case MSG_STORAGE_SERVICE_VID_REPLY:
        		if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY) == 0) {
					STOR_SERV_FILEINFO *file_info_ptr = (STOR_SERV_FILEINFO *) ApQ_para;
					INT8U type = FALSE;
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VID_CYCLIC_REQ, &type, sizeof(INT8U), MSG_PRI_NORMAL);
					close(file_info_ptr->file_handle);
					unlink((CHAR *) file_info_ptr->file_path_addr);
					file_info_ptr->file_handle = -1;
#if	GPS_TXT
					close(file_info_ptr->txt_handle);
					unlink((CHAR *) file_info_ptr->txt_path_addr);
					file_info_ptr->txt_handle = -1;
#endif
				} else {
	        		ap_video_record_reply_action((STOR_SERV_FILEINFO *) ApQ_para);
#if C_MOTION_DETECTION == CUSTOM_ON
    	    		switch_flag = 0;
#endif
				if(ap_state_handling_file_creat_get())
					{
					if ((ap_video_record_sts_get() & VIDEO_RECORD_MD))
				     led_type = LED_MOTION_DETECTION;
				     else
					 led_type = LED_RECORD;
		             msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					}

				}
        		break;

#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
			case MSG_STORAGE_SERVICE_VID_CYCLIC_REPLY:
        		if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY) == 0) {
					STOR_SERV_FILEINFO *file_info_ptr = (STOR_SERV_FILEINFO *) ApQ_para;
					INT8U type = FALSE;

					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VID_CYCLIC_REQ, &type, sizeof(INT8U), MSG_PRI_NORMAL);
					close(file_info_ptr->file_handle);
					unlink((CHAR *) file_info_ptr->file_path_addr);
					file_info_ptr->file_handle = -1;
#if GPS_TXT
					close(file_info_ptr->txt_handle);
					unlink((CHAR *) file_info_ptr->txt_path_addr);
					file_info_ptr->txt_handle = -1;
#endif
				} else {
	        		ap_video_record_cycle_reply_open((STOR_SERV_FILEINFO *) ApQ_para);
	        	}
        		break;

        	case MSG_APQ_CYCLIC_VIDEO_RECORD:
        		ap_video_record_cycle_reply_action();
        		break;
#endif
		#if SUPORT_GET_JPG_BUF == CUSTOM_ON
			case MSG_APQ_GET_JPEG_ON_RECORDING:
				if (ap_video_record_sts_get() & VIDEO_RECORD_BUSY)
        		{
        			INT32S ret;
					ret = ap_jpeg_get_one_frame_on_recording(user_get_jpeg_buf,&user_get_jpeg_lenth);
					if(ret==STATUS_FAIL){
						DBG_PRINT("Get one frame jpg fail !\r\n");
					}else{
						DBG_PRINT("Get one frame jpg OK.\r\n");
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_PIC_REQ, NULL, NULL, MSG_PRI_NORMAL);
					}
				}
				break;
        	case MSG_STORAGE_SERVICE_PIC_REPLY:
        		
				write(((STOR_SERV_FILEINFO *) ApQ_para)->file_handle, user_get_jpeg_buf, user_get_jpeg_lenth);					
				close(((STOR_SERV_FILEINFO *) ApQ_para)->file_handle);
       		
        		break;
		#endif

        	case MSG_APQ_NEXT_KEY_ACTIVE:
				#if DUAL_STREAM_FUNC_ENABLE
 				if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
 				#endif
 				{        	
	        		if((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)){
	        		  #if KEY_TYPE != KEY_TYPE2 && KEY_TYPE != KEY_TYPE4
	        			msgQSend(ApQ, MSG_APQ_SOS_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
	        		  #endif
	        		}else{
	        			ap_video_record_zoom_inout(0);
	        		}
	        	}
       			break;
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

				ap_video_capture_mode_switch(0, STATE_VIDEO_RECORD);   	
				DBG_PRINT("record: wifi switch...\r\n");
				/*
				if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
				{
					video_encode_preview_on();
				}
				*/
				break;
        	case MSG_APQ_PREV_KEY_ACTIVE:
      			{
      				#if DUAL_STREAM_FUNC_ENABLE
      					if(ap_display_get_device() == DISP_DEV_TV)
      					{
      						DBG_PRINT("TV display,drop prev key!!\r\n");
      						break;
      					}
		        		if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)) {
		        			if(ap_video_record_func_key_active(msg_id)) {
		        				break;
		        			}
		        		}
						#if C_MOTION_DETECTION == CUSTOM_ON
		        		if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
							//ap_video_record_md_icon_clear_all();
							ap_video_record_md_disable();
							ap_state_config_md_set(0);
		        		}
						#endif
						ap_state_handling_str_draw_exit();
						ap_video_record_clear_resolution_str();
						ap_video_record_clear_park_mode();
						OSTimeDly(3);

						video_encode_preview_off();
						vid_enc_disable_sensor_clock();
      					if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
      					{
      						if(Wifi_Connect() == 0)
      						{
      							Wifi_State_Set(WIFI_STATE_FLAG_CONNECT);

								
      						}
      					}
      					else
      					{
      						if(Wifi_Disconnect() == 0)
      						{
      							Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);

								
      						}
      					}

						ap_video_capture_mode_switch(0, STATE_VIDEO_RECORD);   	

						/*if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
						{
							video_encode_preview_on();
						}*/
						
      				#else
	      				INT8U temp;
	      				temp = ap_state_config_voice_record_switch_get();
	      				if(temp){
	      					ap_state_config_voice_record_switch_set(0);
	      				}else{
	       					ap_state_config_voice_record_switch_set(1);
	     				}
						if(ap_state_config_voice_record_switch_get() != 0) {
							ap_state_handling_icon_clear_cmd(ICON_MIC_OFF, NULL, NULL);
						} else {
							ap_state_handling_icon_show_cmd(ICON_MIC_OFF, NULL, NULL);
						}
	      				
	        			ap_video_record_zoom_inout(1);
        			#endif
        		}
        		break;

			case MSG_APQ_PARK_MODE_SET:
				/*if(ap_state_config_park_mode_G_sensor_get())
				{
					ap_state_config_park_mode_G_sensor_set(0);
					ap_video_record_clear_park_mode();
					DBG_PRINT("PARK MODE OFF\r\n");
				}
				else
				{
					ap_state_config_park_mode_G_sensor_set(1);
					ap_video_record_show_park_mode();
					DBG_PRINT("PARK MODE ON\r\n");
				}*/
				break;

			case MSG_APQ_CONNECT_TO_PC:
				DBG_PRINT("record: connect to pc...\r\n");
				if(ap_display_get_device() != DISP_DEV_TFT) break;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TV_POLLING_STOP, NULL, NULL, MSG_PRI_NORMAL);
				OSTimeDly(3);

				power_on_flag = 0;
				led_type =LED_USB_CONNECT;// LED_RECORD;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
//				audio_send_stop();
#if C_MOTION_DETECTION == CUSTOM_ON
				if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
					//ap_video_record_md_icon_clear_all();
					ap_video_record_md_disable();
					ap_state_config_md_set(0);
				}
#endif
        		if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY) != 0) {
        			ap_video_record_func_key_active(NULL);
        		}

        		video_calculate_left_recording_time_disable();
				ap_state_handling_str_draw_exit();

				/*
					盢瞷さpreview 闽奔ち传Θweb cam
				*/
				video_encode_preview_off();
        		vid_enc_disable_sensor_clock();

				//+++
				#if Enable_Lane_Departure_Warning_System
   				LDWS_Enable_Flag = 0;
				drvl2_sdc_card_long_polling_set(SD_USED_NUM ,LDWS_Enable_Flag);
   				#endif
   				//---

   				#if DUAL_STREAM_FUNC_ENABLE
				if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
				{
					if(Wifi_Disconnect() == 0)
					{
						Wifi_State_Set(WIFI_STATE_FLAG_DISCONNECT);
					}
				}
				#endif


        		ap_state_handling_connect_to_pc(STATE_VIDEO_RECORD);
        		break;

        	case MSG_APQ_DISCONNECT_TO_PC:
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TV_POLLING_START, NULL, NULL, MSG_PRI_NORMAL);
				power_on_flag = 0;
        		ap_state_handling_disconnect_to_pc();
	        	OSTimeDly(100);
				//if(ap_state_handling_storage_id_get() != NO_STORAGE) 
				{
	        		video_calculate_left_recording_time_enable();
				}
				/*
					盢web cam preview 闽传Θ video preview
				*/
				vid_enc_disable_sensor_clock();
	       		//video_encode_preview_on();
				/* sensorぃ闽,盢sensor 癳戈ま旧┏糷preview flow */
		   		ap_video_capture_mode_switch(0, STATE_VIDEO_RECORD);
				//+++
				#if Enable_Lane_Departure_Warning_System
				if(ap_LDW_get_from_config(LDW_ON_OFF) &&((ap_state_config_video_resolution_get() == 0) || (ap_state_config_video_resolution_get() == 2)))
				{
					LDWS_Enable_Flag = 1;
					drvl2_sdc_card_long_polling_set(SD_USED_NUM ,LDWS_Enable_Flag);
				}
				#endif
				//---
        		break;

        	case MSG_APQ_RECORD_SWITCH_FILE:
#if C_MOTION_DETECTION == CUSTOM_ON
        		if (!switch_flag) {
	        		switch_flag = 1;
	        		ap_video_record_func_key_active(msg_id);
	        		ap_video_record_func_key_active(msg_id);
	        	}
#endif
        		break;

        	case MSG_APQ_VDO_REC_RESTART:
				DBG_PRINT("VIDEO_REC_RESTART\r\n");
				cyc_restart_flag=1;
				ap_video_record_func_key_active(msg_id);
				cyc_restart_flag=0;
            case MSG_APQ_VDO_REC_STOP:
				ap_video_record_func_key_active(msg_id);

				if (msg_id == MSG_APQ_VDO_REC_STOP) {
				    ap_state_handling_str_draw_exit();
				    ap_state_handling_str_draw(STR_SD_FULL, WARNING_STR_COLOR);
				    g_wifi_sd_full_flag = 1;
		            video_calculate_left_recording_time_disable();
		            led_type = LED_SDC_FULL;
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				}
				break;

        	case MSG_APQ_AVI_PACKER_ERROR:
				ap_video_record_error_handle(NULL);

                // Dominant add. restart AVI record again.
                if(drvl2_sdc_live_response() == 0) {  // Card exist
					if (ap_state_config_md_get()) {
					    break;  // motion detect mode..... no need restart
					} else {
						INT64U  disk_free_size;

						disk_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20;
						if (disk_free_size < CARD_FULL_SIZE_RECORD) {
						    ap_state_handling_str_draw_exit();
						    ap_state_handling_str_draw(STR_SD_FULL, WARNING_STR_COLOR);
						    g_wifi_sd_full_flag = 1;
				            video_calculate_left_recording_time_disable();
							led_type = LED_SDC_FULL;
							msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					        msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
						} else {
							msgQSend(ApQ, MSG_APQ_VIDEO_RECORD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL); //restart
						}
					}
                } else {
#if C_MOTION_DETECTION == CUSTOM_ON
					// if no card, md detection function need to disable
					if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
						//ap_video_record_md_icon_clear_all();
						ap_video_record_md_disable();
						ap_state_config_md_set(0);
					}
#endif
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_STORAGE_CHECK, NULL, NULL, MSG_PRI_NORMAL);
                }
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
        		if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)) {
        			if(ap_video_record_func_key_active(msg_id)) {
        				break;
        			}
        		}
        		OSTimeDly(5);
        		ap_state_handling_clear_all_icon();
				ap_state_handling_str_draw_exit();
				ap_state_handling_str_draw(STR_BATTERY_LOW, WARNING_STR_COLOR);
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_BATTERY_LOW_BLINK_START, NULL, NULL, MSG_PRI_NORMAL);
        		break;        		        		
#endif

			case MSG_APQ_NIGHT_MODE_KEY:
				audio_effect_play(EFFECT_CLICK);
				ap_state_handling_night_mode_switch();
				break;

			/*
			case MSG_APQ_AUDIO_EFFECT_MENU:
			    if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY) == 0) {
			        ap_state_common_handling(msg_id);
			    }
			    break;
			*/

			case MSG_APQ_AUDIO_EFFECT_UP:
			case MSG_APQ_AUDIO_EFFECT_DOWN:
			    break;			

			case MSG_APQ_USER_CONFIG_STORE:
				ap_state_config_store();
				break;

			case MSG_APQ_SOS_KEY_ACTIVE:
				if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)) {
					//audio_effect_play(EFFECT_CLICK);
					ap_video_record_lock_current_file();
					ap_state_handling_icon_show_cmd(ICON_LOCKED, NULL, NULL);
				}
				break;

			case MSG_APQ_HDMI_PLUG_IN:
#if C_MOTION_DETECTION == CUSTOM_ON
        		if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
					//ap_video_record_md_icon_clear_all();
					ap_video_record_md_disable();
					ap_state_config_md_set(0);
        		}
#endif
				if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)) {
        			if(ap_video_record_func_key_active(msg_id)) {
        				break;
        			}
        		}
				ap_state_handling_str_draw_exit();
				video_calculate_left_recording_time_disable();
				power_on_flag = 0;

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
 				
        		OSQPost(StateHandlingQ, (void *) STATE_VIDEO_PREVIEW);
				msgQSend(ApQ, MSG_APQ_HDMI_PLUG_IN, NULL, NULL, MSG_PRI_NORMAL);
        		exit_flag = EXIT_BREAK;
			break;

			//+++ TV_OUT_D1
			case MSG_APQ_TV_PLUG_OUT:
			case MSG_APQ_TV_PLUG_IN:
        		if ((ap_video_record_sts_get() & VIDEO_RECORD_BUSY)) {
        			if(ap_video_record_func_key_active(msg_id)) {
        				break;
        			}
        		}
#if C_MOTION_DETECTION == CUSTOM_ON
        		if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
					//ap_video_record_md_icon_clear_all();
					ap_video_record_md_disable();
//					ap_state_config_md_set(0);
        		}
#endif
				ap_state_handling_str_draw_exit();
				ap_video_record_clear_resolution_str();
				ap_video_record_clear_park_mode();

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
				ap_video_capture_mode_switch(1, STATE_VIDEO_RECORD);
				video_encode_preview_on();
				ap_video_record_resolution_display();
				if(ap_state_config_park_mode_G_sensor_get()) {
					ap_video_record_show_park_mode();
				}
				power_on_flag = 0;
			break;
			//---

			#if DUAL_STREAM_FUNC_ENABLE
				case MSG_WIFI_CONNECTED: // Sent From VLC App 
					DBG_PRINT("Connect WIFI \r\n");
		       		video_encode_preview_on();
				break;
				case MSG_WIFI_DISCONNECTED: // Sent From VLC App 
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
				if(ap_video_record_sts_get() & VIDEO_RECORD_BUSY){
					if((msg_id == MSG_APQ_AUDIO_EFFECT_OK)||(msg_id == MSG_APQ_AUDIO_EFFECT_MODE)){
		        		ap_state_common_handling(msg_id);
		        	}
				}else{
		        	ap_state_common_handling(msg_id);
		        }
				break;
		}
	}
	state_video_record_exit();
}

void state_video_record_exit(void)
{
	//+++
	#if (Enable_Lane_Departure_Warning_System)
	if(LDWS_Enable_Flag)
	{
		if(LDWs_workmem != NULL)
		{
			gp_free((void *) LDWs_workmem);
			LDWs_workmem = NULL;
		}
	}
	LDWS_Enable_Flag = 0;
	drvl2_sdc_card_long_polling_set(SD_USED_NUM ,LDWS_Enable_Flag);	
	#endif
	
#if SUPORT_GET_JPG_BUF == CUSTOM_ON
	if(user_get_jpeg_buf != NULL){
		gp_free((void *)user_get_jpeg_buf);
		user_get_jpeg_buf = NULL;
	}
#endif

	ap_video_record_exit();
	DBG_PRINT("Exit video_record state\r\n");
}

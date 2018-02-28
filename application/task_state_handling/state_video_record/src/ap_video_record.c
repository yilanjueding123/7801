#include "ap_video_record.h"
#include "ap_state_config.h"
#include "ap_state_handling.h"
#include "avi_encoder_app.h"
#include "application.h"
#include "ap_display.h"
#include "LDWs.h"

STOR_SERV_FILEINFO curr_file_info;
INT8S video_record_sts;
extern INT32U g_wifi_sd_full_flag;

static STR_ICON_EXT resolution_str;
static STR_ICON_EXT park_mode_str;

#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
	static INT8U cyclic_record_timerid;
	STOR_SERV_FILEINFO next_file_info;
	static CHAR g_cycle_prev_file_path[24];
    static CHAR g_cycle_curr_file_path[24];
#endif
#if C_MOTION_DETECTION == CUSTOM_ON
	static INT8U motion_detect_timerid = 0xFF;
//prototype
	void ap_video_record_md_icon_clear_all(void);
	void ap_video_record_md_tick(INT8U *md_tick,INT32U state);
	INT8S ap_video_record_md_active(INT8U *md_tick,INT32U state);
	void ap_video_record_md_disable(void);
	void ap_video_record_md_icon_update(INT8U sts);
#endif
    INT8S g_lock_current_file_flag = 0;
	INT8U g_cycle_record_continuing_flag = 0;

//prototype
void video_calculate_left_recording_time_enable(void);
void video_calculate_left_recording_time_disable(void);

extern volatile INT32U g_last_time;
extern volatile INT32U g_videolapse_frame_cnt;
extern volatile INT8U  g_first_voice_frame;

extern void date_time_force_display(INT8U force_en,INT8U postion_flag);
void ap_video_record_show_park_mode(void)
{
	STRING_ASCII_INFO ascii_str;
	ascii_str.font_color = BLUE_COLOR;
	ascii_str.font_type = 0;
	ascii_str.pos_x = 0;
	ascii_str.pos_y = 0;
	ascii_str.str_ptr = "P";
	ascii_str.buff_h = park_mode_str.h = ASCII_draw_char_height;
	ascii_str.buff_w = park_mode_str.w = ASCII_draw_char_width*1;

	if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
		park_mode_str.pos_x = 32*6;
		park_mode_str.pos_y = 2;
	} else { //TV
		park_mode_str.pos_x = 450;
		park_mode_str.pos_y = 30;
	}
	ap_state_handling_ASCII_str_draw(&park_mode_str,&ascii_str);
}

void ap_video_record_clear_park_mode(void)
{
	ap_state_handling_ASCII_str_draw_exit(&park_mode_str,1);
}

void ap_video_record_resolution_display(void)
{
#if 0
	STRING_ASCII_INFO ascii_str;

	ascii_str.font_color = 0xffff;
	ascii_str.font_type = 0;
	ascii_str.pos_x = 0;
	ascii_str.pos_y = 0;

	if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
		resolution_str.pos_y = 35;
	} else { //TV
		resolution_str.pos_y = 80;
	}
	ascii_str.buff_h = resolution_str.h = ASCII_draw_char_height;
	if(ap_state_config_video_resolution_get() == 0) {
		ascii_str.str_ptr = "1080FHD";
		ascii_str.buff_w = resolution_str.w = ASCII_draw_char_width*7;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			resolution_str.pos_x = TFT_WIDTH-5-ASCII_draw_char_width*7;
		} else { //TV
		  #if TV_WIDTH == 720
			resolution_str.pos_x = TV_WIDTH-40-ASCII_draw_char_width*7;
  		  #elif TV_WIDTH == 640
			resolution_str.pos_x = TV_WIDTH-20-ASCII_draw_char_width*7;
  		  #endif
		}
	} else if(ap_state_config_video_resolution_get() == 1) {
		ascii_str.str_ptr = "1080P";
		ascii_str.buff_w = resolution_str.w = ASCII_draw_char_width*5;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			resolution_str.pos_x = TFT_WIDTH-5-ASCII_draw_char_width*5;
		} else { //TV
		  #if TV_WIDTH == 720
			resolution_str.pos_x = TV_WIDTH-40-ASCII_draw_char_width*5;
  		  #elif TV_WIDTH == 640
			resolution_str.pos_x = TV_WIDTH-20-ASCII_draw_char_width*5;
  		  #endif
		}
	} else if(ap_state_config_video_resolution_get() == 2) {
		ascii_str.str_ptr = "720P";
		ascii_str.buff_w = resolution_str.w = ASCII_draw_char_width*4;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			resolution_str.pos_x = TFT_WIDTH-5-ASCII_draw_char_width*4;
		} else { //TV
		  #if TV_WIDTH == 720
			resolution_str.pos_x = TV_WIDTH-40-ASCII_draw_char_width*4;
  		  #elif TV_WIDTH == 640
			resolution_str.pos_x = TV_WIDTH-20-ASCII_draw_char_width*4;
  		  #endif
		}
	} else if(ap_state_config_video_resolution_get() == 3) {
		ascii_str.str_ptr = "WVGA";
		ascii_str.buff_w = resolution_str.w = ASCII_draw_char_width*4;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			resolution_str.pos_x = TFT_WIDTH-5-ASCII_draw_char_width*4;
		} else { //TV
		  #if TV_WIDTH == 720
			resolution_str.pos_x = TV_WIDTH-40-ASCII_draw_char_width*4;
  		  #elif TV_WIDTH == 640
			resolution_str.pos_x = TV_WIDTH-20-ASCII_draw_char_width*4;
  		  #endif
		}
	} else if(ap_state_config_video_resolution_get() == 4) {
		ascii_str.str_ptr = "VGA";
		ascii_str.buff_w = resolution_str.w = ASCII_draw_char_width*3;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			resolution_str.pos_x = TFT_WIDTH-5-ASCII_draw_char_width*3;
		} else { //TV
		  #if TV_WIDTH == 720
			resolution_str.pos_x = TV_WIDTH-40-ASCII_draw_char_width*3;
  		  #elif TV_WIDTH == 640
			resolution_str.pos_x = TV_WIDTH-20-ASCII_draw_char_width*3;
  		  #endif
		}
	} else {
		ascii_str.str_ptr = "QVGA";
		ascii_str.buff_w = resolution_str.w = ASCII_draw_char_width*4;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			resolution_str.pos_x = TFT_WIDTH-5-ASCII_draw_char_width*4;
		} else { //TV
		  #if TV_WIDTH == 720
			resolution_str.pos_x = TV_WIDTH-40-ASCII_draw_char_width*4;
  		  #elif TV_WIDTH == 640
			resolution_str.pos_x = TV_WIDTH-20-ASCII_draw_char_width*4;
  		  #endif
		}
	}
	ap_state_handling_ASCII_str_draw(&resolution_str,&ascii_str);
#else
	if(ap_state_config_video_resolution_get() == 0) {
		ap_state_handling_icon_show_cmd(ICON_1080P30, NULL, NULL);
	} else if(ap_state_config_video_resolution_get() == 1) {
		ap_state_handling_icon_show_cmd(ICON_720P60, NULL, NULL);
	} else if(ap_state_config_video_resolution_get() == 2) {
		ap_state_handling_icon_show_cmd(ICON_720P30, NULL, NULL);
	} else if(ap_state_config_video_resolution_get() == 3) {
		ap_state_handling_icon_show_cmd(ICON_WVGA, NULL, NULL);
	} else if(ap_state_config_video_resolution_get() == 4) {
		ap_state_handling_icon_show_cmd(ICON_VGA, NULL, NULL);
	} else {
		ap_state_handling_icon_show_cmd(ICON_VGA, NULL, NULL);
	}
#endif
}

void ap_video_record_clear_resolution_str(void)
{
	ap_state_handling_ASCII_str_draw_exit(&resolution_str,1);
}

INT8S ap_video_record_init(INT32U state)
{
	//INT8U night_mode;
	INT8U temp;

	OSQPost(DisplayTaskQ, (void *) MSG_DISPLAY_TASK_EFFECT_INIT);
	/*if(ap_state_config_video_resolution_get() <= 2) {
		date_time_force_display(1, DISPLAY_DATE_TIME_RECORD);
	} else {
		date_time_force_display(1, DISPLAY_DATE_TIME_RECORD2);
	}*/
	date_time_force_display(1, DISPLAY_DATE_TIME_RECORD);
	temp = ap_state_config_videolapse_get();
	ap_state_handling_icon_show_cmd(ICON_REC_YELLOW+temp, NULL, NULL);

	if(temp==0)
	{		
	if(ap_state_config_voice_record_switch_get() != 0) {
		ap_state_handling_icon_clear_cmd(ICON_MIC_OFF, NULL, NULL);
			ap_state_handling_icon_show_cmd(ICON_MIC_ON, NULL, NULL);
	} else {
		ap_state_handling_icon_show_cmd(ICON_MIC_OFF, NULL, NULL);
			ap_state_handling_icon_clear_cmd(ICON_MIC_ON, NULL, NULL);
	}

	ap_video_record_resolution_display();
		video_calculate_left_recording_time_enable();

	temp = ap_state_config_record_time_get();
		//if(temp) {
			ap_state_handling_icon_show_cmd(ICON_VIDEO_RECORD_CYC_OFF+temp, NULL, NULL);
		//}
	}
	
	/*temp = ap_state_config_park_mode_G_sensor_get();
	if(temp) {
		ap_video_record_show_park_mode();
	}
		
	temp = ap_state_config_ev_get();
	ap_state_handling_icon_show_cmd(ICON_VIDEO_EV6+temp, NULL, NULL);
	gp_cdsp_set_ev_val(temp);	//0:+2, 1:+5/3, 2:+4/3, 3:+1.0, 4:+2/3, 5:+1/3, 6:+0.0, 7:-1/3, 8:-2/3, 9:-1.0, 10:-4/3, 11:-5/3, 12:-2.0 
	*/
/*
#if Enable_Lane_Departure_Warning_System == 1
	temp = ap_state_config_LDW_get(LDW_ON_OFF);
	if(temp){
		ap_state_handling_icon_show_cmd(ICON_VIDEO_LDW_SART, NULL, NULL);
	}
#endif
*/

#if C_BATTERY_DETECT == CUSTOM_ON && USE_ADKEY_NO
	ap_state_handling_current_bat_lvl_show();
	ap_state_handling_current_charge_icon_show();
#endif

#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
	cyclic_record_timerid = 0xFF;
	next_file_info.file_handle = -1;
#if GPS_TXT
	next_file_info.txt_handle = -1;
#endif
#endif

/*	if(ap_state_handling_night_mode_get()) {
		ap_state_handling_icon_show_cmd(ICON_NIGHT_MODE_ENABLED, NULL, NULL);
		ap_state_handling_icon_clear_cmd(ICON_NIGHT_MODE_DISABLED, NULL, NULL);
		night_mode = 1;
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_NIGHT_MODE_SET, &night_mode, sizeof(INT8U), MSG_PRI_NORMAL);
	} else {
		ap_state_handling_icon_show_cmd(ICON_NIGHT_MODE_DISABLED, NULL, NULL);
		ap_state_handling_icon_clear_cmd(ICON_NIGHT_MODE_ENABLED, NULL, NULL);
		night_mode = 0;
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_NIGHT_MODE_SET, &night_mode, sizeof(INT8U), MSG_PRI_NORMAL);
	}
*/
	g_lock_current_file_flag = 0;
	g_cycle_record_continuing_flag = 0;
	//ap_state_handling_icon_clear_cmd(ICON_LOCKED, NULL, NULL);

	ap_video_record_sts_set(0);
	if (ap_state_handling_storage_id_get() == NO_STORAGE) {
		//ap_state_handling_icon_show_cmd(ICON_VIDEO_RECORD, ICON_INTERNAL_MEMORY, NULL);
		ap_video_record_sts_set(VIDEO_RECORD_UNMOUNT);
		//video_calculate_left_recording_time_disable();
#if C_MOTION_DETECTION == CUSTOM_ON
		if (ap_state_config_md_get()) {
			//ap_video_record_md_icon_clear_all();
			ap_video_record_md_disable();
			ap_state_config_md_set(0);
		}
#endif
		return STATUS_FAIL;
	} else {
		//ap_state_handling_icon_show_cmd(ICON_VIDEO_RECORD, ICON_SD_CARD, NULL);
		ap_video_record_sts_set(~VIDEO_RECORD_UNMOUNT);
		//video_calculate_left_recording_time_enable();
#if C_MOTION_DETECTION == CUSTOM_ON
		if (ap_state_config_md_get()) {
			//ap_state_handling_icon_show_cmd(ICON_MD_STS_0, NULL, NULL);
			//ap_state_handling_icon_show_cmd(ICON_MOTION_DETECT, NULL, NULL);
			//ap_state_handling_icon_show_cmd(ICON_VIDEO_RECORD_MOTION_10SECOND, NULL, NULL);
			temp = ap_state_config_record_time_get();
			//ap_state_handling_icon_clear_cmd(ICON_VIDEO_RECORD_CYC_OFF+temp, NULL, NULL);
			msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_MOTION_DETECT_START, NULL, NULL, MSG_PRI_NORMAL);
			ap_video_record_sts_set(VIDEO_RECORD_MD);
		}
#endif
		return STATUS_OK;
	}
}

void ap_video_record_exit(void)
{
	ap_video_record_clear_resolution_str();
	ap_video_record_clear_park_mode();

	date_time_force_display(0, DISPLAY_DATE_TIME_RECORD);

}

void ap_video_record_sts_set(INT8S sts)
{
	if (sts > 0) {
		video_record_sts |= sts;
	} else {
		video_record_sts &= sts;
	}
}

#if C_MOTION_DETECTION == CUSTOM_ON
void ap_video_record_md_icon_clear_all(void)
{
	INT8U temp;
  #if 0
	OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_ICON_CLEAR | (ICON_MD_STS_0 | (ICON_MD_STS_1<<8) | (ICON_MD_STS_2<<16))));
	OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_ICON_CLEAR | (ICON_MD_STS_3 | (ICON_MD_STS_4<<8) | (ICON_MD_STS_5<<16))));
  #else
    ap_state_handling_icon_clear_cmd(ICON_MOTION_DETECT, NULL, NULL);
    ap_state_handling_icon_clear_cmd(ICON_VIDEO_RECORD_MOTION_10SECOND, NULL, NULL);
    temp = ap_state_config_record_time_get();
	ap_state_handling_icon_show_cmd(ICON_VIDEO_RECORD_CYC_OFF+temp, NULL, NULL);
  #endif
}
void ap_video_record_md_tick(INT8U *md_tick,INT32U state)
{
	++*md_tick;
	if (*md_tick > MD_STOP_TIME) {
		*md_tick = 0;
		if (motion_detect_timerid != 0xFF) {
			sys_kill_timer(motion_detect_timerid);
			motion_detect_timerid = 0xFF;
		}
		if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
			ap_video_record_error_handle(state);
	//		ap_state_handling_icon_clear_cmd(ICON_MOTION_DETECT_START, NULL, NULL);
	        DBG_PRINT("Motion detect md_tick Show icon!\r\n");
			//ap_state_handling_icon_show_cmd(ICON_MOTION_DETECT, NULL, NULL);
		}
	}
}

INT8S ap_video_record_md_active(INT8U *md_tick,INT32U state)
{
	*md_tick = 0;
	if (video_record_sts & VIDEO_RECORD_UNMOUNT) {
		return STATUS_FAIL;
	}
    if ((ap_state_config_record_time_get()==0) && (vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20) < CARD_FULL_SIZE_RECORD) 
    {
        DBG_PRINT ("Card full, MD Auto Disable2!!!\r\n");
       // ap_video_record_md_icon_clear_all();
    	ap_video_record_md_disable();
    	ap_state_config_md_set(0); 
        ap_video_record_error_handle(state);  // super stop MD record
        return	STATUS_FAIL;
    }
	if (!(video_record_sts & VIDEO_RECORD_BUSY)) {
		if (ap_video_record_func_key_active(state)) {
			return STATUS_FAIL;
		} else {
			if (motion_detect_timerid == 0xFF) {
				motion_detect_timerid = VIDEO_PREVIEW_TIMER_ID;
				sys_set_timer((void*)msgQSend, (void*)ApQ, MSG_APQ_MOTION_DETECT_TICK, motion_detect_timerid, MOTION_DETECT_CHECK_TIME_INTERVAL);
			}
		}
	}
	return STATUS_OK;
}

void ap_video_record_md_disable(void)
{
	if (motion_detect_timerid != 0xFF) {
		sys_kill_timer(motion_detect_timerid);
		motion_detect_timerid = 0xFF;
	}
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_MOTION_DETECT_STOP, NULL, NULL, MSG_PRI_NORMAL);
	ap_video_record_sts_set(~VIDEO_RECORD_MD);
}

void ap_video_record_md_icon_update(INT8U sts)
{
#if 0
	if (ap_state_config_md_get()) {
		OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_MD_ICON_SHOW | (ICON_MD_STS_0 + sts)));
	}
#endif
}

#endif
extern INT8U cyc_restart_flag;

extern INT8U ap_peripheral_low_vol_get(void);
volatile INT8U video_down_flag = 0;
INT8S ap_video_record_func_key_active(INT32U event)
{
#if C_AUTO_DEL_FILE == CUSTOM_ON
	INT8U type = FALSE;
#endif
	INT32U led_type;
	INT8U temp;
	INT64U disk_free_size;
	
	video_down_flag = 1;

	if (video_record_sts & VIDEO_RECORD_BUSY) {
		if((event == MSG_APQ_VIDEO_RECORD_ACTIVE) ||(event == MSG_APQ_FUNCTION_KEY_ACTIVE) || (event == MSG_APQ_MODE) || (event == MSG_APQ_MENU_KEY_ACTIVE)||(event == MSG_APQ_CAPTURE_KEY_ACTIVE)/* || (event == MSG_APQ_POWER_KEY_ACTIVE)*/) {
			if((curr_file_info.file_handle == -1) || (next_file_info.file_handle == -1)) {
				video_down_flag = 0;
				return STATUS_FAIL;
			} else {
#if C_MOTION_DETECTION == CUSTOM_ON
				if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
					//ap_video_record_md_icon_clear_all();
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_MOTION_DETECT_DELAY, NULL, NULL, MSG_PRI_NORMAL);
					ap_video_record_md_disable();
					ap_state_config_md_set(0);
				}
#endif
			}
		}
#if C_AUTO_DEL_FILE == CUSTOM_ON
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK_SWITCH, &type, sizeof(INT8U), MSG_PRI_URGENT);
#endif
        timer_counter_force_display(0); // dominant add,function key event, must disable timer OSD 
		//led_type = LED_WAITING_RECORD;
		//msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
		ap_peripheral_auto_off_force_disable_set(0);
		temp = ap_state_config_videolapse_get();
		if(temp==0)
		{
        video_calculate_left_recording_time_enable();
		}
		if (video_encode_stop() != START_OK) {
			close(curr_file_info.file_handle);
			unlink((CHAR *) curr_file_info.file_path_addr);
			curr_file_info.file_handle = -1;
			DBG_PRINT("Video Record Stop Fail\r\n");
		} else {
			if(g_lock_current_file_flag > 0) {
			  #if RENAME_LOCK_FILE
				CHAR temp_file_name[24];
			  #endif

				_setfattr((CHAR *) curr_file_info.file_path_addr, D_RDONLY);
				g_lock_current_file_flag = 0;

			  #if RENAME_LOCK_FILE
				gp_memcpy((INT8S *)temp_file_name, (INT8S *)curr_file_info.file_path_addr, sizeof(temp_file_name));
				#if !LOCK_FILE_NAME
				temp_file_name[0] = 'L'; temp_file_name[1] = 'O'; temp_file_name[2] = 'C'; temp_file_name[3] = 'K';
				#else
				temp_file_name[0] = 'S'; temp_file_name[1] = 'O'; temp_file_name[2] = 'S'; temp_file_name[3] = '0';
				#endif
				_rename((char *)curr_file_info.file_path_addr, temp_file_name);
				gp_memcpy((INT8S *)curr_file_info.file_path_addr, (INT8S *)temp_file_name, sizeof(temp_file_name));
			  #endif
			}
#if GPS_TXT
			close(curr_file_info.txt_handle);
			curr_file_info.txt_handle = -1;
#endif
		}
#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
		if (cyclic_record_timerid != 0xFF) {
			sys_kill_timer(cyclic_record_timerid);
			cyclic_record_timerid = 0xFF;
		}
		if (next_file_info.file_handle != -1) {
			INT8U type = FALSE;
			
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VID_CYCLIC_REQ, &type, sizeof(INT8U), MSG_PRI_NORMAL);
			close(next_file_info.file_handle);
			unlink((CHAR *) next_file_info.file_path_addr);
			next_file_info.file_handle = -1;
#if GPS_TXT
			close(next_file_info.txt_handle);
			unlink((CHAR *) next_file_info.txt_path_addr);
			next_file_info.txt_handle = -1;
#endif
		}
#endif
		g_lock_current_file_flag = 0;
		g_cycle_record_continuing_flag = 0;
		
		if(ap_state_handling_storage_id_get()!= NO_STORAGE)
		{			
			if(!cyc_restart_flag && ap_state_handling_file_creat_get())
			{
				DBG_PRINT("EVENT_message=%d\r\n",event);
				if(event != MSG_APQ_VDO_REC_STOP)
				{
					if (ap_peripheral_low_vol_get() != 1)
					{
						if (video_record_sts & VIDEO_RECORD_MD)
						{
							if (ap_state_config_md_get())
								led_type = LED_MOTION_WAITING;
							else
							    led_type = LED_WAITING_RECORD;
						}
						else
						{
							led_type = LED_WAITING_RECORD;
						}
						msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
						DBG_PRINT("------video_wait-----\r\n");
					}
				}
			}
		}
		else
		{
			led_type = LED_NO_SDC;
			msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
		}
		
		temp = ap_state_config_videolapse_get();
		//_state_handling_icon_clear_cmd(ICON_REC+temp, ICON_LOCKED, NULL);
		//_state_handling_icon_clear_cmd(ICON_VIDEO_LDW_SART, NULL, NULL);
		//_state_handling_icon_show_cmd(ICON_REC_YELLOW+temp, NULL, NULL);

		ap_video_record_sts_set(~VIDEO_RECORD_BUSY);
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
		g_last_time = 0;
		g_videolapse_frame_cnt=0;
		ap_state_handling_file_creat_set(0);
		g_first_voice_frame=0;
		DBG_PRINT("Video Record Stop\r\n");
		video_down_flag = 0;
		return STATUS_OK;
	} else {
		if ((video_record_sts & VIDEO_RECORD_UNMOUNT) == 0) {
			video_down_flag = 1;
#if C_MOTION_DETECTION == CUSTOM_ON
			
			if (video_record_sts == 0 || video_record_sts == VIDEO_RECORD_MD) {
				INT64U  disk_free_size;

				if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
					if((event == MSG_APQ_VIDEO_RECORD_ACTIVE) || (event == MSG_APQ_FUNCTION_KEY_ACTIVE) || (event == MSG_APQ_MODE) || (event == MSG_APQ_MENU_KEY_ACTIVE) ) {
						//ap_video_record_md_icon_clear_all();
						ap_video_record_md_disable();
						ap_state_config_md_set(0);
					}
				}
				disk_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20;
		        DBG_PRINT("\r\n[Disk free %dMB]\r\n", disk_free_size);
				if (disk_free_size < CARD_FULL_SIZE_RECORD) {
					if (ap_state_config_record_time_get() == 0) {
						ap_state_handling_str_draw_exit();
						ap_state_handling_str_draw(STR_SD_FULL, WARNING_STR_COLOR);
						g_wifi_sd_full_flag = 1;
						video_calculate_left_recording_time_disable();
						led_type = LED_SDC_FULL;
						msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					} else {
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FREE_FILESIZE_CHECK, NULL, NULL, MSG_PRI_NORMAL);
					}
					video_down_flag = 0;
					return STATUS_OK;
				}
				video_down_flag = 0;
				ap_video_record_sts_set(VIDEO_RECORD_BUSY);
				ap_peripheral_auto_off_force_disable_set(1);
				curr_file_info.file_handle = -1;
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VID_REQ, NULL, NULL, MSG_PRI_NORMAL);
			
	    	    if (ap_state_config_md_get()) {
					//ap_state_handling_icon_show_cmd(ICON_MD_STS_0, NULL, NULL);
//					ap_state_handling_icon_clear_cmd(ICON_MOTION_DETECT, NULL, NULL);
//					ap_state_handling_icon_show_cmd(ICON_MOTION_DETECT_START, NULL, NULL);
	                if (!(video_record_sts & VIDEO_RECORD_MD)) {
						msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_MOTION_DETECT_START, NULL, NULL, MSG_PRI_NORMAL);
						ap_video_record_sts_set(VIDEO_RECORD_MD);
					}

	                if (motion_detect_timerid == 0xFF) {
						motion_detect_timerid = VIDEO_PREVIEW_TIMER_ID;
						sys_set_timer((void*)msgQSend, (void*)ApQ, MSG_APQ_MOTION_DETECT_TICK, motion_detect_timerid, MOTION_DETECT_CHECK_TIME_INTERVAL);
					}
				}
			} else {
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
				video_down_flag = 0;
				return STATUS_FAIL;
			}
		} else if (video_record_sts & VIDEO_RECORD_UNMOUNT) {
			if (ap_state_config_md_get()) {
				if(ap_video_record_sts_get() & VIDEO_RECORD_MD) {
					//ap_video_record_md_icon_clear_all();
					ap_video_record_md_disable();
			    	ap_state_config_md_set(0);
				} else {
					//ap_state_handling_icon_show_cmd(ICON_MD_STS_0, NULL, NULL);
//					ap_state_handling_icon_clear_cmd(ICON_MOTION_DETECT_START, NULL, NULL);
//					ap_state_handling_icon_show_cmd(ICON_MOTION_DETECT, NULL, NULL);
					msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_MOTION_DETECT_START, NULL, NULL, MSG_PRI_NORMAL);
					ap_video_record_sts_set(VIDEO_RECORD_MD);
				}
			}
			ap_state_handling_str_draw_exit();
			ap_state_handling_str_draw(STR_INSERT_SDC, WARNING_STR_COLOR);
			msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_DISPLAY_PLEASE_INSERT_SDC, NULL, NULL, MSG_PRI_NORMAL);
			
			ap_peripheral_auto_off_force_disable_set(0);
		#else
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		disk_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20;
		DBG_PRINT("\r\n[Disk free %dMB]\r\n", disk_free_size);
		if (disk_free_size < CARD_FULL_SIZE_RECORD) {
			if (ap_state_config_record_time_get() == 0) 
			{
				ap_state_handling_str_draw_exit();
				ap_state_handling_str_draw(STR_SD_FULL, WARNING_STR_COLOR);
				video_calculate_left_recording_time_disable();
				led_type = LED_SDC_FULL;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				video_down_flag = 0;
				return STATUS_OK;
			}
		}
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		ap_video_record_sts_set(VIDEO_RECORD_BUSY);
		ap_peripheral_auto_off_force_disable_set(1);
		curr_file_info.file_handle = -1;
#if GPS_TXT
		curr_file_info.txt_handle = -1;
#endif
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VID_REQ, NULL, NULL, MSG_PRI_NORMAL);
		#endif
		}
	}
	video_down_flag = 0;
	return STATUS_OK;
}

void ap_video_record_start(void)
{
	ap_state_handling_str_draw_exit();
	ap_video_record_sts_set(VIDEO_RECORD_BUSY);
	ap_peripheral_auto_off_force_disable_set(1);
	curr_file_info.file_handle = -1;
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VID_REQ, NULL, NULL, MSG_PRI_NORMAL);
#if C_MOTION_DETECTION == CUSTOM_ON
    if (ap_state_config_md_get())
    {
		//ap_state_handling_icon_show_cmd(ICON_MD_STS_0, NULL, NULL);
//		ap_state_handling_icon_clear_cmd(ICON_MOTION_DETECT, NULL, NULL);
//		ap_state_handling_icon_show_cmd(ICON_MOTION_DETECT_START, NULL, NULL);
        if (!(video_record_sts & VIDEO_RECORD_MD))
        {
			msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_MOTION_DETECT_START, NULL, NULL, MSG_PRI_NORMAL);
			ap_video_record_sts_set(VIDEO_RECORD_MD);
		}
        if (motion_detect_timerid == 0xFF) 
        {
			motion_detect_timerid = VIDEO_PREVIEW_TIMER_ID;
			sys_set_timer((void*)msgQSend, (void*)ApQ, MSG_APQ_MOTION_DETECT_TICK, motion_detect_timerid, MOTION_DETECT_CHECK_TIME_INTERVAL);
		}
	}
#endif
}

#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
void ap_video_record_cycle_reply_open(STOR_SERV_FILEINFO *file_info_ptr)
{
	if (file_info_ptr->file_handle != -1) {
		gp_memcpy((INT8S *) &next_file_info, (INT8S *) file_info_ptr, sizeof(STOR_SERV_FILEINFO));
	} else {
		DBG_PRINT("Cyclic video record open file Fail\r\n");
	}
}

void ap_video_record_cycle_reply_action(void)
{
	MEDIA_SOURCE src;
	INT8U i, *next_ptr, *curr_ptr;
#if C_AUTO_DEL_FILE == CUSTOM_OFF
	INT32U size, tag, total_size,led_type;
//#else
//	INT8U type = TRUE;
#endif
	INT8U type = TRUE;
	INT8U temp;
	INT32U led_type;

	temp = ap_state_config_videolapse_get();
	if (next_file_info.file_handle != -1) {
		type = FALSE;
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK_SWITCH, &type, sizeof(INT8U), MSG_PRI_URGENT);

		src.type_ID.FileHandle = next_file_info.file_handle;
		src.type = SOURCE_TYPE_FS;
		src.Format.VideoFormat = MJPEG;

        timer_counter_force_display(1);

#if C_AUTO_DEL_FILE == CUSTOM_ON
#if GPS_TXT
		if(video_encode_fast_stop_and_start(src, next_file_info.txt_handle) != START_OK)
#else
		if(video_encode_fast_stop_and_start(src, -1) != START_OK)
#endif
		{
			close(src.type_ID.FileHandle);
			unlink((CHAR *) next_file_info.file_path_addr);
			next_file_info.file_handle = -1;
#if GPS_TXT
			close(next_file_info.txt_handle);
			unlink((CHAR *) next_file_info.txt_path_addr);
			next_file_info.txt_handle = -1;
#endif

			g_lock_current_file_flag = 0;
			g_cycle_record_continuing_flag = 0;

			ap_state_handling_icon_clear_cmd(ICON_LOCKED, NULL, NULL);
			ap_state_handling_icon_clear_cmd(ICON_REC+temp, NULL, NULL);
			ap_state_handling_icon_show_cmd(ICON_REC_YELLOW+temp, NULL, NULL);
	        timer_counter_force_display(0);

			led_type = LED_WAITING_RECORD;
		    msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);

            ap_video_record_sts_set(~VIDEO_RECORD_BUSY);
            msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);

			DBG_PRINT("Video Record Fail1\r\n");
			return;

		} else {
			if(g_lock_current_file_flag > 0) {
			  #if RENAME_LOCK_FILE
				CHAR temp_file_name[24];
			  #endif

				_setfattr((CHAR *) curr_file_info.file_path_addr, D_RDONLY);

			  #if RENAME_LOCK_FILE
				gp_memcpy((INT8S *)temp_file_name, (INT8S *)curr_file_info.file_path_addr, sizeof(temp_file_name));
				#if !LOCK_FILE_NAME
				temp_file_name[0] = 'L'; temp_file_name[1] = 'O'; temp_file_name[2] = 'C'; temp_file_name[3] = 'K';
				#else
				temp_file_name[0] = 'S'; temp_file_name[1] = 'O'; temp_file_name[2] = 'S'; temp_file_name[3] = '0';
				#endif
				_rename((char *)curr_file_info.file_path_addr, temp_file_name);
				gp_memcpy((INT8S *)curr_file_info.file_path_addr, (INT8S *)temp_file_name, sizeof(temp_file_name));
			  #endif
			}

			g_lock_current_file_flag--;
			if(g_lock_current_file_flag <= 0){
				g_lock_current_file_flag = 0;
				ap_state_handling_icon_clear_cmd(ICON_LOCKED, NULL, NULL);
            }

#if GPS_TXT
			close(curr_file_info.txt_handle);
#endif
			next_ptr = (INT8U*) next_file_info.file_path_addr;
			curr_ptr = (INT8U*) curr_file_info.file_path_addr;
			for (i=0; i < 24; i++) {
				g_cycle_curr_file_path[i] = *(next_ptr+i);
				g_cycle_prev_file_path[i] = *(curr_ptr+i);
				*(curr_ptr+i) = g_cycle_curr_file_path[i];
			}
#if GPS_TXT
			gp_memcpy((INT8S *)curr_file_info.txt_path_addr, (INT8S *)next_file_info.txt_path_addr, 24);
#endif
			curr_file_info.file_handle = next_file_info.file_handle;
#if GPS_TXT
			curr_file_info.txt_handle = next_file_info.txt_handle;
#endif
			curr_file_info.storage_free_size = next_file_info.storage_free_size;
		}

		g_cycle_record_continuing_flag = 1;
		ap_state_handling_icon_show_cmd(ICON_REC+temp, NULL, NULL);
		ap_state_handling_icon_clear_cmd(ICON_REC_YELLOW+temp, NULL, NULL);
		next_file_info.file_handle = -1;	//open file handle
		type = TRUE;
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK_SWITCH, &type, sizeof(INT8U), MSG_PRI_NORMAL);
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VID_CYCLIC_REQ, &type, sizeof(INT8U), MSG_PRI_NORMAL);
#else
		if (cyclic_record_timerid != 0xFF) {
			sys_kill_timer(cyclic_record_timerid);
			cyclic_record_timerid = 0xFF;
		}
		if (ap_state_config_record_time_get() == 0) {
			tag = 255;
		} else {
			tag = ap_state_config_record_time_get();
		}
		total_size = vfsTotalSpace(MINI_DVR_STORAGE_TYPE) >> 20;
		if( (total_size <= 1024) && ((ap_state_config_record_time_get() == 4) || (ap_state_config_record_time_get() == 5)) ){
			size = 300;
		}else{
			size = (FRAME_SIZE*30*60*tag) >> 10;
		}
		if (video_record_sts & VIDEO_RECORD_BUSY) {
			if (video_encode_stop() != START_OK) {
				close(curr_file_info.file_handle);
				unlink((CHAR *) curr_file_info.file_path_addr);
				DBG_PRINT("Video Record Stop Fail\r\n");
                ap_video_record_sts_set(~VIDEO_RECORD_BUSY);
			}
            msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, &size, sizeof(INT32U), MSG_PRI_NORMAL);
		}
#endif
	}
}
#endif

INT32S ap_video_record_del_reply(INT32S ret,INT32U state)
{
	if (ret == STATUS_FAIL) {
		ap_video_record_error_handle(state);
		return STATUS_FAIL;
	} else {
		return STATUS_OK;
	}
}

void ap_video_record_error_handle(INT32U state)
{
	if (video_record_sts & VIDEO_RECORD_BUSY) {
		ap_video_record_func_key_active(state);
	}
}

INT8U ap_video_record_sts_get(void)
{
	return	video_record_sts;
}

INT8U ap_video_record_MD_sts_get(void)
{
	if (video_record_sts & VIDEO_RECORD_MD) {
		return 0;
	} else {
		return 1;
	}
}

void ap_video_record_reply_action(STOR_SERV_FILEINFO *file_info_ptr)
{
	MEDIA_SOURCE src;
    INT32U disk_free_size;
    INT32U led_type;

#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
	INT32U time_interval, value;
	//INT8U temp[6] = {0, 1, 2, 3, 5, 10};
	INT8U temp[4] = {0, 3, 5, 10};
	INT16U temp1[6] = {25, 50, 125, 250, 750, 1500};
	INT8U tc_flag;
	INT16U tw_value;
	
	tc_flag = ap_state_config_videolapse_get();
	value = temp[ap_state_config_record_time_get()];
	if (value == 0) {
		time_interval = 255 * VIDEO_RECORD_CYCLE_TIME_INTERVAL + 16;  // dominant add 64 to record more
		// FAT32:60min(<4GB), FAT16:30min(<2GB), but FAT16 SDC is always less than 2GB
		bkground_del_disable(1);  // back ground auto delete disable
	} else {
        bkground_del_disable(0);  // back ground auto delete enable
        if(tc_flag==0)
        {
        	time_interval = value * VIDEO_RECORD_CYCLE_TIME_INTERVAL + 16;  // dominant add 32 to record more
        }
        else
        {
        	tw_value = temp1[tc_flag-1];
        	time_interval = tw_value * value * VIDEO_RECORD_CYCLE_TIME_INTERVAL + 16;  // dominant add 32 to record more
        }
    }
#endif

	if (file_info_ptr) {
		gp_memcpy((INT8S *) &curr_file_info, (INT8S *) file_info_ptr, sizeof(STOR_SERV_FILEINFO));
	} else {
		curr_file_info.storage_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20;
	}
	src.type_ID.FileHandle = curr_file_info.file_handle;
	src.type = SOURCE_TYPE_FS;
	src.Format.VideoFormat = MJPEG;
	if (src.type_ID.FileHandle >= 0) {
        timer_counter_force_display(1);
		video_calculate_left_recording_time_disable();
        ap_state_handling_icon_show_cmd(ICON_REC+tc_flag, NULL, NULL);
        ap_state_handling_icon_clear_cmd(ICON_REC_YELLOW+tc_flag, NULL, NULL);
#if GPS_TXT
		if (video_encode_start(src, curr_file_info.txt_handle) != START_OK)
#else
		if (video_encode_start(src,  -1) != START_OK)
#endif
		{
			INT8U type = FALSE;

            timer_counter_force_display(0);
	        video_calculate_left_recording_time_enable();
			ap_peripheral_auto_off_force_disable_set(0);
			ap_state_handling_icon_clear_cmd(ICON_REC+tc_flag, NULL, NULL);
			ap_state_handling_icon_show_cmd(ICON_REC_YELLOW+tc_flag, NULL, NULL);

			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VID_CYCLIC_REQ, &type, sizeof(INT8U), MSG_PRI_NORMAL);
			close(src.type_ID.FileHandle);
			unlink((CHAR *) curr_file_info.file_path_addr);
			curr_file_info.file_handle = -1;
#if GPS_TXT
			close(curr_file_info.txt_handle);
			unlink((CHAR *) curr_file_info.txt_path_addr);
			curr_file_info.txt_handle = -1;
#endif
			if (ap_peripheral_low_vol_get() != 1)
			{
				led_type = LED_WAITING_RECORD;
			    msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
			}

	        ap_video_record_sts_set(~VIDEO_RECORD_BUSY);
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
			DBG_PRINT("Video Record Fail\r\n");
		} else {
#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
			INT8U type = TRUE;

			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VID_CYCLIC_REQ, &type, sizeof(INT8U), MSG_PRI_NORMAL);
			//if (cyclic_record_timerid == 0xFF) 
			{
				cyclic_record_timerid = VIDEO_RECORD_CYCLE_TIMER_ID;
				sys_set_timer((void*)msgQSend, (void*)ApQ, MSG_APQ_CYCLIC_VIDEO_RECORD, cyclic_record_timerid, time_interval);
			}
#endif
#if C_AUTO_DEL_FILE == CUSTOM_ON
			{
				INT8U type = TRUE;
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK_SWITCH, &type, sizeof(INT8U), MSG_PRI_NORMAL);

		       	if(curr_file_info.storage_free_size < bkground_del_thread_size_get()) {
			        disk_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20;
			        DBG_PRINT("\r\n[Bkground Del Detect (DskFree: %d MB)]\r\n", disk_free_size);

			        if (bkground_del_disable_status_get() == 0) {
						INT32U temp;

			       		temp = 0xFFFFFFFF;
				        msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VIDEO_FILE_DEL, &temp, sizeof(INT32U), MSG_PRI_NORMAL);
			        }
				}
			}
#endif
			//ap_state_handling_led_blink_on();
			//led_type = LED_RECORD;
			//msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
		}
	} else {
		led_type = LED_SDC_FULL;
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
        ap_video_record_sts_set(~VIDEO_RECORD_BUSY);
		ap_peripheral_auto_off_force_disable_set(0);
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
		DBG_PRINT("Video Record Stop [NO free space]\r\n");
	}
}

void video_calculate_left_recording_time_enable(void)
{
	INT64U freespace, left_recording_time;

	if (ap_state_handling_storage_id_get() == NO_STORAGE)
	{
		left_recording_time = 0;
	}
	else
	{
		freespace = vfsFreeSpace(MINI_DVR_STORAGE_TYPE);

		if(freespace > (CARD_FULL_SIZE_RECORD<<20)) freespace -= (CARD_FULL_SIZE_RECORD<<20);
		else freespace = 0;

		if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) || (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)) {
			left_recording_time = freespace / (3*1024*1024);	//estimate one frame = 100KB, so one second video takes 100KB*30 = 3MB
		} else if (my_pAviEncVidPara->encode_width == AVI_WIDTH_720P) {
			left_recording_time = freespace / (2458*1024);		//estimate one frame = 80KB, so one second video takes 80KB*30 = 2.4MB
		} else {
			left_recording_time = freespace / (1843*1024);	//estimate one frame = 60KB, so one second video takes 60KB*30 = 1.8MB
		}
	}

	OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_LEFT_REC_TIME_DRAW | left_recording_time));
}

void video_calculate_left_recording_time_disable(void)
{	
	#if GPDV_BOARD_VERSION != GPCV4247_WIFI
	OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_LEFT_REC_TIME_CLEAR));
    #endif
}

extern INT32U ap_display_timer_rec_time_get(void);
void ap_video_record_lock_current_file(void)
{
	//INT8U temp[6] = {0, 1, 2, 3, 5, 10};
	INT8U temp[4] = {0, 3, 5, 10};
	INT8U record_config = temp[ap_state_config_record_time_get()];
	INT32U recording_time;
	
	recording_time = ap_display_timer_rec_time_get();
	if((recording_time < 10) && g_cycle_record_continuing_flag) {
	  #if RENAME_LOCK_FILE
		CHAR temp_file_name[24];
	  #endif

		_setfattr((CHAR *)g_cycle_prev_file_path, D_RDONLY);

	  #if RENAME_LOCK_FILE
		gp_memcpy((INT8S *)temp_file_name, (INT8S *)g_cycle_prev_file_path, sizeof(temp_file_name));
		#if !LOCK_FILE_NAME
		temp_file_name[0] = 'L'; temp_file_name[1] = 'O'; temp_file_name[2] = 'C'; temp_file_name[3] = 'K';
		#else
		temp_file_name[0] = 'S'; temp_file_name[1] = 'O'; temp_file_name[2] = 'S'; temp_file_name[3] = '0';
		#endif
		_rename((char *)g_cycle_prev_file_path, temp_file_name);
		gp_memcpy((INT8S *)g_cycle_prev_file_path, (INT8S *)temp_file_name, sizeof(temp_file_name));
	  #endif
	}

	if(!record_config){
		g_lock_current_file_flag = 1;
		return;
	}

	if(recording_time > (record_config*60-10)) {
		g_lock_current_file_flag = 2;
	} else {
		g_lock_current_file_flag = 1;	//set file attribute only after close file
	}	
}


INT32S ap_video_record_zoom_inout(INT8U inout)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;

#if 0
	//+++ Not support digital zoom : 0:1080FHD  1:1080P
	err = ap_state_config_video_resolution_get();
	if((err == 0)|| (err == 1))
	{
		return nRet;
	}
#else
	return nRet;
#endif
	
	if(inout){
		POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_ZOOM_IN, my_avi_encode_ack_m, 5000, msg, err);	
	}else{
		POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_ZOOM_OUT, my_avi_encode_ack_m, 5000, msg, err);	
	}
	Return:	
		return nRet;
}

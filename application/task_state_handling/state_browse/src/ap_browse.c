#include "ap_browse.h"
#include "ap_state_handling.h"
#include "ap_music.h"
#include "ap_state_resource.h"
#include "ap_state_config.h"
#include "task_video_decoder_cfg.h"
#include "ap_display.h"
#include "string.h"
#include "my_video_codec_callback.h"
#include "state_wifi.h"

#if DUAL_STREAM_FUNC_ENABLE
extern void Disp_MJpeg_To_Wifi(INT32U dispAddr);
#endif


INT8S browse_sts;
static INT8U browse_display_timerid = 0xFF;
static INT16U browse_total_file_num;

STOR_SERV_PLAYINFO browse_curr_avi;
INT8U g_browser_reply_action_flag = 0;
INT8S browser_play_speed;
INT8U  g_AVI_resolution;
static STR_ICON_EXT browse_play_speed_str;
static STR_ICON_EXT browse_resolution_str;
static 	TIME_T	browse_file_date_time;
static  INT16U wifi_playback_file_index = 0;

//	prototypes
INT32S ap_browse_mjpeg_decode(void);
void ap_browse_resolution_string_draw(STOR_SERV_PLAYINFO *info_ptr, INT16U wAVIWidth);


enum {
	STR_HDMI_LANGUAGE,
	STR_HDMI_VIDEO,
	STR_HDMI_QVGA,
	STR_HDMI_VGA,
	STR_HDMI_WVGA,
	STR_HDMI_720P,
	STR_HDMI_1080P,
	STR_HDMI_1080FHD,
	STR_HDMI_VGA1,
	STR_HDMI_1_3_M,
	STR_HDMI_2MHD,
	STR_HDMI_3M,
	STR_HDMI_5M,
	STR_HDMI_8M,
	STR_HDMI_10M,
	STR_HDMI_12M,
	STR_HDMI_2X,
	STR_HDMI_4X,
	STR_HDMI_8X,
	STR_HDMI_M2X,
	STR_HDMI_M4X,
	STR_HDMI_M8X,
	STR_HDMI_AVI,
	STR_HDMI_JPG,
	STR_HDMI_MOVI,
	STR_HDMI_PICT
};

void ap_browse_init(INT32U prev_state, INT16U play_index)
{
	INT32U search_type_with_index, search_type;

	if (prev_state == STATE_SETTING) {
		if(play_index == 0xA5A5) {
			search_type = STOR_SERV_SEARCH_PREV;	//Daniel modified for returning to previous one after deleting the current one
		} else {
			search_type = STOR_SERV_SEARCH_ORIGIN;
		}
	} else if(prev_state == STATE_BROWSE) {
		search_type = STOR_SERV_SEARCH_ORIGIN;
	} else {
		search_type = STOR_SERV_SEARCH_INIT;
	}

	OSQPost(DisplayTaskQ, (void *) MSG_DISPLAY_TASK_EFFECT_INIT);
	///ap_state_handling_icon_show_cmd(ICON_PLAYBACK, NULL, NULL);

	if(vid_dec_entry() < 0) {
		DBG_PRINT("Failed to init motion jpeg task\r\n");
	}

	#if C_BATTERY_DETECT == CUSTOM_ON && USE_ADKEY_NO
		ap_state_handling_current_bat_lvl_show();
		ap_state_handling_current_charge_icon_show();
	#endif

	browse_sts = 0;
	if (ap_state_handling_storage_id_get() == NO_STORAGE) {
		ap_browse_sts_set(BROWSE_UNMOUNT);
	} else {
		ap_browse_sts_set(~BROWSE_UNMOUNT);
	}


	g_browser_reply_action_flag = 0;

	#if DUAL_STREAM_FUNC_ENABLE
	if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
	{
		g_browser_reply_action_flag = 1;

	}
	else
	#endif
	{
		if(prev_state == STATE_THUMBNAIL) {
			search_type_with_index = STOR_SERV_SEARCH_GIVEN | (play_index << 16);
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_BROWSE_REQ, (void *)&search_type_with_index, sizeof(INT32U), MSG_PRI_NORMAL);
		} else {
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_BROWSE_REQ, (void *)&search_type, sizeof(INT32U), MSG_PRI_NORMAL);
		}
	}
}

void ap_browse_string_icon_clear(void)
{
	browser_play_speed = 0;
	ap_browse_fast_play_icon_show(browser_play_speed);
	ap_state_handling_ASCII_str_draw_exit(&browse_resolution_str,1);
}

void ap_browse_exit(INT32U next_state_msg)
{
	if (browse_sts == BROWSE_PLAYBACK_BUSY) {
		if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV) {
			audio_wav_pause();
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
		} else if(vid_dec_pause() == STATUS_OK){
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
		}
	}

	if (browse_sts & BROWSE_PLAYBACK_BUSY) {
		if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV) {
			ap_state_handling_icon_clear_cmd(ICON_PLAY, ICON_PAUSE, NULL);
			ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
			if(g_AVI_resolution == AVI_RES_VGA) {
				ap_state_handling_icon_show_cmd(ICON_PLAY, NULL, NULL);
			} else {
				ap_state_handling_icon_show_cmd(ICON_PLAY1, NULL, NULL);
			}
			ap_browse_wav_stop();
			ap_browse_display_timer_draw();
		} else {
			ap_browse_mjpeg_stop();
       		ap_browse_mjpeg_play_end();
		}
 		close(browse_curr_avi.file_handle);
		OSTimeDly(5);
	}

	if((next_state_msg == MSG_APQ_MODE) || (next_state_msg == MSG_APQ_POWER_KEY_ACTIVE)) {
		ap_state_handling_icon_clear_cmd(ICON_PLAYBACK, ICON_PLAY, ICON_PAUSE);
		ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_PLAY, ICON_PLAYBACK_PAUSE, ICON_PLAYBACK_MOVIE);
		
	} else if((next_state_msg == MSG_APQ_CONNECT_TO_PC) || (next_state_msg == MSG_APQ_HDMI_PLUG_IN) || (next_state_msg == MSG_APQ_HDMI_PLUG_OUT) || 
			(next_state_msg == MSG_APQ_TV_PLUG_IN) || (next_state_msg == MSG_APQ_TV_PLUG_OUT)) {
		ap_state_handling_icon_clear_cmd(ICON_PLAY, ICON_PAUSE, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_PLAY, ICON_PLAYBACK_PAUSE, ICON_PLAYBACK_MOVIE);
	}

	#if DUAL_STREAM_FUNC_ENABLE
	if(next_state_msg != MSG_APQ_WIFI_DISCONNECT)
	#endif
	{
		date_time_force_display(0,DISPLAY_DATE_TIME_BROWSE);
		ap_browse_string_icon_clear();
		vid_dec_exit();
	}

	browse_sts = 0;
}

void ap_browse_sts_set(INT8S sts)
{
	if (sts > 0) {
		browse_sts |= sts;
	} else {
		browse_sts &= sts;
	}
}

INT8S ap_browse_sts_get(void)
{
	return 	browse_sts;
}

INT8U ap_browse_get_curr_file_type(void)
{
	return browse_curr_avi.file_type;
}

void ap_browse_func_key_active(void)
{
	INT32S ret = 0;
	INT32U led_type;

	if((browse_sts & BROWSE_UNMOUNT)||(browse_curr_avi.file_type == TK_IMAGE_TYPE_JPEG)){
		return;
	}

	ap_state_handling_icon_clear_cmd(ICON_PAUSE, ICON_PLAY, NULL);
	ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
	////ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_PLAY, ICON_PLAYBACK_PAUSE, NULL);

	if (!browse_sts) {
		if(browse_total_file_num) {
			ap_browse_sts_set(BROWSE_PLAYBACK_BUSY);
			if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV) {
				ap_browse_wav_play();
				if(g_AVI_resolution == AVI_RES_VGA) {
					ap_state_handling_icon_show_cmd(ICON_PAUSE, NULL, NULL);
				} else {
					ap_state_handling_icon_show_cmd(ICON_PAUSE1, NULL, NULL);
				}
				ap_browse_display_timer_draw();
				OSTimeDly(10);
			} else{
				ret = ap_browse_mjpeg_decode();
			 	if (ret != STATUS_FAIL) {
			 		INT8U curr_disp_dev;
					curr_disp_dev = ap_display_get_device();
					if (curr_disp_dev==DISP_DEV_HDMI) {
						ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_MOVIE, NULL, NULL);
						date_time_force_display(0,DISPLAY_DATE_TIME_BROWSE);
						ap_state_handling_ASCII_str_draw_exit(&browse_resolution_str, 1);
					}else{
					}
					if(browse_curr_avi.file_type != TK_IMAGE_TYPE_JPEG) {
						//ap_state_handling_icon_show_cmd(ICON_PLAYBACK_PAUSE, NULL, NULL);
						ap_state_handling_icon_show_cmd(ICON_PLAY, NULL, NULL);
						ap_state_handling_icon_clear_cmd(ICON_PLAYBACK, NULL, NULL);
						ap_state_handling_icon_clear_cmd(ICON_PAUSE, NULL, NULL);
					}
					OSTimeDly(10);
				}
			}
		}
	} else if (browse_sts == BROWSE_PLAYBACK_BUSY) {
		ap_browse_sts_set(BROWSE_PLAYBACK_PAUSE);
		gpio_write_io(SPEAKER_EN, 0);
		if(browse_curr_avi.file_type != TK_IMAGE_TYPE_JPEG) {
			//ap_state_handling_icon_show_cmd(ICON_PLAYBACK_PLAY, NULL, NULL);
			ap_state_handling_icon_show_cmd(ICON_PAUSE, NULL, NULL);
			ap_state_handling_icon_clear_cmd(ICON_PLAYBACK, NULL, NULL);
			ap_state_handling_icon_clear_cmd(ICON_PLAY, NULL, NULL);
		}
		ap_peripheral_auto_off_force_disable_set(0);
		led_type = LED_WAITING_RECORD;
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
		if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV) {
			audio_wav_pause();
			ap_browse_display_timer_draw();
			OSTimeDly(10);
		} else {
			OSTimeDly(10);	//wait to update display icon
			ret = vid_dec_pause();
			if(ret == STATUS_OK) {
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
			}
		}
	} else if (browse_sts == (BROWSE_PLAYBACK_BUSY | BROWSE_PLAYBACK_PAUSE)) {
		ap_browse_sts_set(~BROWSE_PLAYBACK_PAUSE);
		if(browse_curr_avi.file_type != TK_IMAGE_TYPE_JPEG) {
			//ap_state_handling_icon_show_cmd(ICON_PLAYBACK_PAUSE, NULL, NULL);
			ap_state_handling_icon_show_cmd(ICON_PLAY, NULL, NULL);
			ap_state_handling_icon_clear_cmd(ICON_PLAYBACK, NULL, NULL);
			ap_state_handling_icon_clear_cmd(ICON_PAUSE, NULL, NULL);
		}
		ap_peripheral_auto_off_force_disable_set(1);
		//ap_state_handling_led_blink_on();

		if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV){
			audio_wav_resume();
			ap_browse_display_timer_draw();
			OSTimeDly(10);
		} else if(vid_dec_resume() == STATUS_OK) {
			#if DUAL_STREAM_FUNC_ENABLE
			if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
			{
				gpio_write_io(SPEAKER_EN, 1);
			}
			#endif
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_NORMAL);
			OSTimeDly(10);	//wait to update display icon
		}
	}
}

void ap_browse_mjpeg_stop(void)
{
	INT32U led_type;

	ap_browse_sts_set(~(BROWSE_PLAYBACK_BUSY | BROWSE_PLAYBACK_PAUSE));
	ap_peripheral_auto_off_force_disable_set(0);
	led_type = LED_WAITING_RECORD;
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
	timer_counter_force_display_browser(0);

	vid_dec_stop();
	vid_dec_parser_stop();

	//gpio_write_io(SPEAKER_EN, DATA_LOW);
}

void ap_browse_mjpeg_play_end(void)
{
	browser_play_speed = 0;
	ap_browse_fast_play_icon_show(browser_play_speed);
	
	ap_state_handling_icon_clear_cmd(ICON_PAUSE, ICON_PLAY, NULL);
	ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
	////ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_PLAY, ICON_PLAYBACK_PAUSE, NULL);
	ap_browse_volume_icon_clear_all();
	if(browse_curr_avi.file_type != TK_IMAGE_TYPE_JPEG) {
		//ap_state_handling_icon_show_cmd(ICON_PLAYBACK_PLAY, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_PLAYBACK, NULL, NULL);
	}
}

void ap_browse_volume_icon_clear_all(void)
{
	INT8U i;
	ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_VOLUME_TITLE,NULL,NULL);
	for(i=0;i<8;i++){
		ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_VOLUME_WHITE1+i,NULL,NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_VOLUME_READ1+i,NULL,NULL);
	}
}

void ap_browse_volume_icon_show(INT8U vol)
{	
	INT8U i,volume_icon_show_time;
	ap_browse_volume_icon_clear_all();	
	ap_state_handling_icon_show_cmd(ICON_PLAYBACK_VOLUME_TITLE,NULL,NULL);
	for(i=0;i<8;i++){
		ap_state_handling_icon_show_cmd(ICON_PLAYBACK_VOLUME_WHITE1+i,NULL,NULL);
	}
	for(i=0;i<vol;i++){
		ap_state_handling_icon_show_cmd(ICON_PLAYBACK_VOLUME_READ1+i,NULL,NULL);
	}
	volume_icon_show_time = 200;
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_TIMER_VOLUME_ICON_SHOW, &volume_icon_show_time, sizeof(INT8U), MSG_PRI_NORMAL);
}


void ap_browse_volume_up_down(INT8U direction)
{
	INT8U vol;
	vol = ap_state_config_volume_get();
	if(direction) {//vol down
		if(vol >0){
			vol--;
		}
	}else{ //vol up
		if(vol < MAX_VOLUME_LEVEL){
			vol++;
		}
	}
	DBG_PRINT("vol = %d \r\n", vol);
	ap_browse_volume_icon_show(vol);
	audio_vol_set(vol);
	ap_state_config_volume_set(vol);
}

void ap_browse_fast_play_icon_show(INT8S play_speed)
{
	ap_state_handling_ASCII_str_draw_exit(&browse_play_speed_str,1);
	if(play_speed)
	{
		if (ap_display_get_device()==DISP_DEV_HDMI) {
			STRING_INFO str_info = {0};
			t_STRING_TABLE_STRUCT str_res;

			if(play_speed>0) {
				if(play_speed == 3) {
					str_info.str_idx = STR_HDMI_8X;
				} else if(play_speed == 2) {
					str_info.str_idx = STR_HDMI_4X;
				} else if(play_speed == 1) {
					str_info.str_idx = STR_HDMI_2X;
				}
			} else if(play_speed<0) {
				if(play_speed == -1) {
					str_info.str_idx = STR_HDMI_M2X;
				} else if(play_speed == -2) {
					str_info.str_idx = STR_HDMI_M4X;
				} else if(play_speed == -3) {
					str_info.str_idx = STR_HDMI_M8X;
				}
			}

			str_info.language = 12;
			ap_state_resource_string_resolution_get(&str_info, &str_res);
			str_res.string_width += str_res.string_width & 0x0001;

			str_info.font_color = 0xff80;
			str_info.font_type = 0;
			str_info.buff_w = browse_play_speed_str.w = str_res.string_width;
			str_info.buff_h = browse_play_speed_str.h = str_res.string_height;

			browse_play_speed_str.pos_x = 560;
			browse_play_speed_str.pos_y = 596;

			ap_state_handling_ASCII_str_draw_HDMI(&browse_play_speed_str, &str_info);

		} else {
			STRING_ASCII_INFO ascii_str;

			ascii_str.font_color = 0xffff;
			ascii_str.font_type = 0;
			ascii_str.pos_x = 0;
			ascii_str.pos_y = 0;
			if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
				browse_play_speed_str.pos_x = 100;
				browse_play_speed_str.pos_y = 210;
			} else { //TV
				browse_play_speed_str.pos_x = 200;
				browse_play_speed_str.pos_y = 400;
			}
			ascii_str.buff_h = browse_play_speed_str.h = ASCII_draw_char_height;
			if(play_speed>0){
				if(play_speed == 3) {
					ascii_str.str_ptr = "8X";
				} else if(play_speed == 2) {
					ascii_str.str_ptr = "4X";
				} else if(play_speed == 1) {
					ascii_str.str_ptr = "2X";
				}
				ascii_str.buff_w = browse_play_speed_str.w = ASCII_draw_char_width*2;
			}else if(play_speed<0){
				if(play_speed == -1) {
					ascii_str.str_ptr = "-2X";
				} else if(play_speed == -2) {
					ascii_str.str_ptr = "-4X";
				} else if(play_speed == -3) {
					ascii_str.str_ptr = "-8X";
				}
				ascii_str.buff_w = browse_play_speed_str.w = ASCII_draw_char_width*3;
			}
			ap_state_handling_ASCII_str_draw(&browse_play_speed_str,&ascii_str);
		}
	}
}



void ap_browse_next_key_active(INT8U err_flag)
{
	INT32U search_type = STOR_SERV_SEARCH_NEXT;
	
	if (err_flag) {
		search_type |= (err_flag << 8);
	}

	if(!(browse_sts & BROWSE_PLAYBACK_PAUSE)) {
		if(browse_sts & BROWSE_PLAYBACK_BUSY) {
			ap_browse_volume_up_down(0);
			return;
		}
	}

	if((browse_sts & BROWSE_UNMOUNT)||(browse_total_file_num == 1)||(browse_total_file_num == 0)) {
		if(browse_sts & BROWSE_PLAYBACK_BUSY) {
			if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV) {
				ap_state_handling_icon_clear_cmd(ICON_PLAY, ICON_PAUSE, NULL);
				ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
				if(g_AVI_resolution == AVI_RES_VGA) {
					ap_state_handling_icon_show_cmd(ICON_PLAY, NULL, NULL);
				} else {
					ap_state_handling_icon_show_cmd(ICON_PLAY1, NULL, NULL);
				}
				ap_browse_wav_stop();
				ap_browse_display_timer_draw();
			} else {
				ap_browse_mjpeg_stop();
	       		ap_browse_mjpeg_play_end();
			}
	 		close(browse_curr_avi.file_handle);

			g_browser_reply_action_flag = 0;
			search_type = STOR_SERV_SEARCH_ORIGIN;
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_BROWSE_REQ, (void *)&search_type, sizeof(INT32U), MSG_PRI_NORMAL);
		}

		if(!err_flag) {
			audio_effect_play(EFFECT_CLICK);
		}
		return;
	}

	if (browse_sts & (BROWSE_PLAYBACK_BUSY | BROWSE_PLAYBACK_PAUSE)) {
		if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV) {
			ap_browse_wav_stop();
		} else {
			ap_browse_mjpeg_stop();
			browser_play_speed = 0;
			ap_browse_fast_play_icon_show(browser_play_speed);
			ap_browse_volume_icon_clear_all();
		}

 		close(browse_curr_avi.file_handle);		
	}

	g_browser_reply_action_flag = 0;
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_BROWSE_REQ, (void *) &search_type, sizeof(INT32U), MSG_PRI_NORMAL);
	if(!err_flag) {
		audio_effect_play(EFFECT_CLICK);
	}
}

void ap_browse_prev_key_active(INT8U err_flag)
{
	INT32U search_type = STOR_SERV_SEARCH_PREV;

	if (err_flag) {
		search_type |= (err_flag << 8);
	}

	if(!(browse_sts & BROWSE_PLAYBACK_PAUSE)) {
		if(browse_sts & BROWSE_PLAYBACK_BUSY) {
			ap_browse_volume_up_down(1);
			return;
		}
	}

	if((browse_sts & BROWSE_UNMOUNT)||(browse_total_file_num == 1)||(browse_total_file_num == 0)){
		if(browse_sts & BROWSE_PLAYBACK_BUSY) {
			if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV){
				ap_state_handling_icon_clear_cmd(ICON_PLAY, ICON_PAUSE, NULL);
				ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
				if(g_AVI_resolution == AVI_RES_VGA) {
					ap_state_handling_icon_show_cmd(ICON_PLAY, NULL, NULL);
				} else {
					ap_state_handling_icon_show_cmd(ICON_PLAY1, NULL, NULL);
				}
				ap_browse_wav_stop();
				ap_browse_display_timer_draw();
			} else {
				ap_browse_mjpeg_stop();
	       		ap_browse_mjpeg_play_end();
			}
	 		close(browse_curr_avi.file_handle);

			g_browser_reply_action_flag = 0;
			search_type = STOR_SERV_SEARCH_ORIGIN;
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_BROWSE_REQ, (void *)&search_type, sizeof(INT32U), MSG_PRI_NORMAL);
		}

		if(!err_flag) {
			audio_effect_play(EFFECT_CLICK);
		}
		return;
	}

	if (browse_sts & (BROWSE_PLAYBACK_BUSY | BROWSE_PLAYBACK_PAUSE)) {
		if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV) {
			ap_browse_wav_stop();
		}else{
			ap_browse_mjpeg_stop();
			browser_play_speed = 0;
			ap_browse_fast_play_icon_show(browser_play_speed);
			ap_browse_volume_icon_clear_all();
		}

 		close(browse_curr_avi.file_handle);
	}

	g_browser_reply_action_flag = 0;
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_BROWSE_REQ, (void *) &search_type, sizeof(INT32U), MSG_PRI_NORMAL);
	if(!err_flag) {
		audio_effect_play(EFFECT_CLICK);
	}
}

void ap_browse_get_file_date_time(TIME_T *osd_time)
{
	osd_time->tm_year  = browse_file_date_time.tm_year;
	osd_time->tm_mon  = browse_file_date_time.tm_mon;
	osd_time->tm_mday  = browse_file_date_time.tm_mday;
	osd_time->tm_hour  = browse_file_date_time.tm_hour;
	osd_time->tm_min  = browse_file_date_time.tm_min;
	osd_time->tm_sec  = browse_file_date_time.tm_sec;
}



void ap_browse_decode_file_date_time(INT32U dos_date_time)
{

	INT16U dos_date,dos_time;
	INT16U year;
	INT8U month,day;
	INT8U hour,minute,second;
	

	dos_date = dos_date_time&0xffff;
	dos_time = (dos_date_time>>16)&0xffff;
	dosdate_decode(dos_date, &year, &month, &day);
	dostime_decode(dos_time, &hour, &minute, &second);
	browse_file_date_time.tm_year = year;
	browse_file_date_time.tm_mon = month;
	browse_file_date_time.tm_mday = day;
	browse_file_date_time.tm_hour = hour;
	browse_file_date_time.tm_min = minute;
	browse_file_date_time.tm_sec = second;

	/*
	DBG_PRINT("year = %d, ",browse_file_date_time.tm_year);
	DBG_PRINT("mon = %d, ",browse_file_date_time.tm_mon);
	DBG_PRINT("day = %d, ",browse_file_date_time.tm_mday);
	DBG_PRINT("hour = %d, ",browse_file_date_time.tm_hour);
	DBG_PRINT("min = %d, ",browse_file_date_time.tm_min);
	DBG_PRINT("sec = %d\r\n",browse_file_date_time.tm_sec);
	*/
}



void ap_browse_decode_file_date_time_1(INT32U dos_date_time,TIME_T *time_info)
{

	INT16U dos_date,dos_time;
	INT16U year;
	INT8U month,day;
	INT8U hour,minute,second;
	

	dos_date = dos_date_time&0xffff;
	dos_time = (dos_date_time>>16)&0xffff;
	dosdate_decode(dos_date, &year, &month, &day);
	dostime_decode(dos_time, &hour, &minute, &second);
	time_info->tm_year = year;
	time_info->tm_mon = month;
	time_info->tm_mday = day;
	time_info->tm_hour = hour;
	time_info->tm_min = minute;
	time_info->tm_sec = second;
}



void ap_browse_reply_action(STOR_SERV_PLAYINFO *info_ptr)
{
	INT32S ret, logo_img_ptr;
	IMAGE_DECODE_STRUCT img_info;
	INT32U size;
	INT16U logo_fd;
	INT16U wAVIWidth, temp;
	struct stat_t buf_tmp;
	INT32U display_addr = 0;

	ap_browse_sts_set(BROWSE_DECODE_BUSY);
	if (info_ptr->err_flag == STOR_SERV_OPEN_OK)
	{
		browse_total_file_num = info_ptr->total_file_number;

		#if 1//GPDV_BOARD_VERSION != GPCV1248_MINI
		ap_state_handling_str_draw_exit();
		ap_state_handling_icon_clear_cmd(ICON_PAUSE, ICON_PLAY, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_PLAY, ICON_PLAYBACK_PAUSE, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAYBACK, ICON_PLAYBACK_MOVIE, NULL);
		#endif

		gp_memcpy((INT8S *)&browse_curr_avi, (INT8S *)info_ptr, sizeof(STOR_SERV_PLAYINFO));

		if (info_ptr->file_type == TK_IMAGE_TYPE_WAV)
		{
			g_AVI_resolution = 0;

			stat((CHAR *)info_ptr->file_path_addr, &buf_tmp);	//check this file is Locked or not
			ap_browse_decode_file_date_time(buf_tmp.st_mtime);
			if(buf_tmp.st_mode & D_RDONLY){
			   	ap_state_handling_icon_show_cmd(ICON_LOCKED1, NULL, NULL);
			} else {
			   	ap_state_handling_icon_clear_cmd(ICON_LOCKED1, NULL, NULL);
			}

			//for WAV files
			logo_fd = nv_open((INT8U *) "AUDIO_REC_BG.JPG");
			if (logo_fd != 0xFFFF) {
				size = nv_rs_size_get(logo_fd);
				logo_img_ptr = (INT32S) gp_malloc(size);
				if (!logo_img_ptr) {
					DBG_PRINT("State browser allocate jpeg input buffer fail.[%d]\r\n", size);
					return;
				}
				if (nv_read(logo_fd, (INT32U) logo_img_ptr, size)) {
					DBG_PRINT("Failed to read resource_header in ap_browse_reply_action()\r\n");
					gp_free((void *) logo_img_ptr);
					return;
				}

				do {
					display_addr = ap_display_queue_get(display_isr_queue);
					OSTimeDly(10);
				} while(!display_addr);

				img_info.image_source = (INT32S) logo_img_ptr;
				img_info.source_size = size;
				img_info.source_type = TK_IMAGE_SOURCE_TYPE_BUFFER;
				img_info.output_format = C_SCALER_CTRL_OUT_RGB565;
				img_info.output_ratio = 0;
				img_info.out_of_boundary_color = 0x008080;
				img_info.output_buffer_width = TFT_WIDTH;
				img_info.output_buffer_height = TFT_HEIGHT;
				img_info.output_image_width = TFT_WIDTH;
				img_info.output_image_height = TFT_HEIGHT;
				img_info.output_buffer_pointer = display_addr;
				if (jpeg_buffer_decode_and_scale(&img_info) == STATUS_FAIL) {
					gp_free((void *) logo_img_ptr);
					ap_display_queue_put(display_isr_queue, display_addr);
					DBG_PRINT("State browser decode AUDIO_REC_BG.jpeg file fail.\r\n");
					return;
				}
				if(g_AVI_resolution == AVI_RES_VGA) {
					ap_state_handling_icon_show_cmd(ICON_PLAY, NULL, NULL);
				} else {
					ap_state_handling_icon_show_cmd(ICON_PLAY1, NULL, NULL);
				}
				ap_browse_show_file_name(1, display_addr);
				ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);

				if ((ap_display_get_device()==DISP_DEV_TFT) || (ap_display_get_device()==DISP_DEV_TV)) {
					OSQPost(DisplayTaskQ, (void *) (display_addr|MSG_DISPLAY_TASK_MJPEG_DRAW));
				} else if (ap_display_get_device()==DISP_DEV_HDMI) {
					OSQPost(DisplayTaskQ, (void *) (display_addr|MSG_DISPLAY_TASK_HDMI_MJPEG_DRAW));
				}
				gp_free((void *) logo_img_ptr);
			}
			close(info_ptr->file_handle);
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
			date_time_force_display(1, DISPLAY_DATE_TIME_BROWSE);
		}
		else	//for AVI and JPG files
		{
			if(info_ptr->file_type == TK_IMAGE_TYPE_MOTION_JPEG)
			{
				#if DUAL_STREAM_FUNC_ENABLE
				if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
				{
					close(info_ptr->file_handle);
					ap_browse_sts_set(~BROWSE_DECODE_BUSY);
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
					return;
				}
				else
				#endif
				{
					lseek(info_ptr->file_handle, 176, SEEK_SET);
					wAVIWidth = 0;
					if (read(info_ptr->file_handle, (INT32U) &wAVIWidth, 2) != 2) {
					    return;
					}
					if(wAVIWidth==640) {
						g_AVI_resolution = AVI_RES_VGA ;
					} else if(wAVIWidth==1280) {
						g_AVI_resolution = AVI_RES_720P ;
					} else if(wAVIWidth==1920) {
						g_AVI_resolution = AVI_RES_1080P ;
					}
				}
			} else {
				lseek(info_ptr->file_handle, 9, SEEK_SET);
				wAVIWidth = 0;
				if(read(info_ptr->file_handle, (INT32U) &wAVIWidth, 2) != 2) {
				    return;
				}
				temp = wAVIWidth>>8;
				wAVIWidth = wAVIWidth<<8|temp;
			}

			do {
				display_addr = ap_display_queue_get(display_isr_queue);
				OSTimeDly(10);
			} while(!display_addr);

			ret = ap_state_handling_jpeg_decode(info_ptr, display_addr); // get 1st jpeg of AVI
			
			if(ret == STATUS_OK) {
				stat((CHAR *)info_ptr->file_path_addr, &buf_tmp);	//check this file is Locked or not
				ap_browse_decode_file_date_time(buf_tmp.st_mtime);
				if(buf_tmp.st_mode & D_RDONLY) {
				   	ap_state_handling_icon_show_cmd(ICON_LOCKED1, NULL, NULL);
				} else {
				   	ap_state_handling_icon_clear_cmd(ICON_LOCKED1, NULL, NULL);
				}
				#if DUAL_STREAM_FUNC_ENABLE
				if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
				#endif
				{
					ap_browse_resolution_string_draw(&browse_curr_avi, wAVIWidth);
					ap_browse_show_file_name(1, display_addr);
				}
				ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
				if ((ap_display_get_device()==DISP_DEV_TFT) || (ap_display_get_device()==DISP_DEV_TV)) {
					#if DUAL_STREAM_FUNC_ENABLE
						if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
						{
							Disp_MJpeg_To_Wifi(display_addr);
							ret = 1;
						}
						else
					#endif
						{
							ret = OSQPost(DisplayTaskQ, (void *) (display_addr|MSG_DISPLAY_TASK_MJPEG_DRAW));
						}
				} else if (ap_display_get_device()==DISP_DEV_HDMI) {
					ret = OSQPost(DisplayTaskQ, (void *) (display_addr|MSG_DISPLAY_TASK_HDMI_MJPEG_DRAW));
				}
				if(ret) {
					ap_display_queue_put(display_isr_queue, display_addr);
				}

				#if DUAL_STREAM_FUNC_ENABLE
					if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
				#endif
					{
						date_time_force_display(1, DISPLAY_DATE_TIME_BROWSE);
					}
			} else {
				INT8U err = 1;

				ap_display_queue_put(display_isr_queue, display_addr);
				if (browse_total_file_num == 1) {
					browse_total_file_num = 0;
					ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
					#if DUAL_STREAM_FUNC_ENABLE
	 				if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
	 				#endif
	 				{					
						ap_browse_no_media_show(STR_NO_MEDIA);
					}
				} else {
					if (info_ptr->search_type == STOR_SERV_SEARCH_NEXT) {
						msgQSend(ApQ, MSG_APQ_NEXT_KEY_ACTIVE, &err, sizeof(INT8U), MSG_PRI_NORMAL);
					} else {
						msgQSend(ApQ, MSG_APQ_PREV_KEY_ACTIVE, &err, sizeof(INT8U), MSG_PRI_NORMAL);
					}
				}
				DBG_PRINT("[SKIP]State browse decode file fail.\r\n");
			}
			close(info_ptr->file_handle);
		}
	} else if (info_ptr->err_flag == STOR_SERV_NO_MEDIA) {
		browse_total_file_num = 0;
		if (ap_state_handling_storage_id_get() == NO_STORAGE) {
			#if DUAL_STREAM_FUNC_ENABLE
			if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
			#endif
			{
				ap_browse_no_media_show(STR_NO_SD);
			}
		} else {
			ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
			#if DUAL_STREAM_FUNC_ENABLE
			if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
			#endif
			{			
				ap_browse_no_media_show(STR_NO_MEDIA);
			}
		}
	} else {
		browse_total_file_num = 0;
		if (ap_state_handling_storage_id_get() == NO_STORAGE) {
			#if DUAL_STREAM_FUNC_ENABLE
			if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
			#endif
			{
				ap_browse_no_media_show(STR_NO_SD);
			}
		} else {
			#if DUAL_STREAM_FUNC_ENABLE
			if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
			#endif
			{
				ap_state_handling_icon_clear_cmd(ICON_PAUSE, ICON_PLAY, NULL);
				ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
				ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_PLAY, ICON_PLAYBACK_PAUSE, NULL);
				ap_state_handling_icon_clear_cmd(ICON_PLAYBACK, ICON_PLAYBACK_MOVIE, NULL);
				
				ap_state_handling_icon_show_cmd(ICON_PLAYBACK, NULL, NULL);
				date_time_force_display(0,DISPLAY_DATE_TIME_BROWSE);
				ap_browse_volume_icon_clear_all();
	    		ap_browse_string_icon_clear();

				ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
				
				ap_browse_no_media_show(STR_NO_MEDIA);
			}

			if (info_ptr->err_flag == STOR_SERV_DECODE_ALL_FAIL) {
				close(info_ptr->file_handle);
			}
		}
	}
	ap_browse_sts_set(~BROWSE_DECODE_BUSY);
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
}


void ap_browse_resolution_string_draw(STOR_SERV_PLAYINFO *info_ptr, INT16U wAVIWidth)
{
	INT8U osd_mode_flag;
	ap_state_handling_ASCII_str_draw_exit(&browse_resolution_str, 1);

	osd_mode_flag = ap_state_config_osd_mode_get();
	if (ap_display_get_device()==DISP_DEV_HDMI) {
		STRING_INFO str_info = {0};
		t_STRING_TABLE_STRUCT str_res;

		if (info_ptr->file_type != TK_IMAGE_TYPE_JPEG) {
			ap_state_handling_icon_show_cmd(ICON_PLAYBACK_MOVIE, ICON_PLAYBACK_PLAY, NULL);

			if(wAVIWidth==320) {
				str_info.str_idx = STR_HDMI_QVGA;
			} else if (wAVIWidth==640) {
				str_info.str_idx = STR_HDMI_VGA;
			} else if (wAVIWidth==848) {
				str_info.str_idx = STR_HDMI_WVGA;
			} else if (wAVIWidth==1280) {
				str_info.str_idx = STR_HDMI_720P;
			} else if (wAVIWidth==1440) {
				str_info.str_idx = STR_HDMI_1080P;
			} else if (wAVIWidth==1920) {
				str_info.str_idx = STR_HDMI_1080FHD;
			}
		} else {
			ap_state_handling_icon_show_cmd(ICON_PLAYBACK, NULL, NULL);

			if(wAVIWidth==640) {
				str_info.str_idx = STR_HDMI_VGA1;
			} else if (wAVIWidth==1280) {
				str_info.str_idx = STR_HDMI_1_3_M;
			} else if (wAVIWidth==1920) {
				str_info.str_idx = STR_HDMI_2MHD;
			} else if (wAVIWidth==2048) {
				str_info.str_idx = STR_HDMI_3M;
			} else if (wAVIWidth==2592) {
				str_info.str_idx = STR_HDMI_5M;
			} else if (wAVIWidth==3264) {
				str_info.str_idx = STR_HDMI_8M;
			} else if (wAVIWidth==3648) {
				str_info.str_idx = STR_HDMI_10M;
			} else if (wAVIWidth==4032) {
				str_info.str_idx = STR_HDMI_12M;
			}
		}

		str_info.language = 12;
		ap_state_resource_string_resolution_get(&str_info, &str_res);
		str_res.string_width += str_res.string_width & 0x0001;

		str_info.font_color = 0xff80;
		str_info.font_type = 0;
		str_info.buff_w = browse_resolution_str.w = str_res.string_width;
		str_info.buff_h = browse_resolution_str.h = str_res.string_height;

		browse_resolution_str.pos_x = 1280-30-browse_resolution_str.w-C_ICON_HDMI_OFFSET;
		browse_resolution_str.pos_y = 70;

		if(wAVIWidth) {
			ap_state_handling_ASCII_str_draw_HDMI(&browse_resolution_str, &str_info);
		}

	} else {
		STRING_ASCII_INFO ascii_str;
		ascii_str.font_color = 0xffff;
		ascii_str.font_type = 0;
		ascii_str.pos_x = 0;
		ascii_str.pos_y = 0;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			browse_resolution_str.pos_y = 35;
		} else { //TV
			browse_resolution_str.pos_y = 80;
		}
		ascii_str.buff_h = browse_resolution_str.h = ASCII_draw_char_height;

		if(info_ptr->file_type != TK_IMAGE_TYPE_JPEG) {
			//ap_state_handling_icon_show_cmd(ICON_PLAYBACK_MOVIE, ICON_PLAYBACK_PLAY, NULL);
			ap_state_handling_icon_clear_cmd(ICON_CAPTURE, NULL, NULL);
			if(osd_mode_flag)
			{
				ap_state_handling_icon_show_cmd(ICON_PLAYBACK, NULL, NULL);
			}
			else
			{
				ap_state_handling_icon_show_cmd(ICON_PLAYBACK_PLAY, NULL, NULL);
			}

			if(wAVIWidth==320) {
				ascii_str.str_ptr = "QVGA";
			} else if (wAVIWidth==640) {
				ascii_str.str_ptr = "VGA";
			} else if (wAVIWidth==848) {
				ascii_str.str_ptr = "WVGA";
			} else if (wAVIWidth==1280) {
				ascii_str.str_ptr = "720P";
			} else if (wAVIWidth==1440) {
				ascii_str.str_ptr = "1080P";
			} else if (wAVIWidth==1920) {
				ascii_str.str_ptr = "1080FHD";
			}
		} else {
			ap_state_handling_icon_clear_cmd(ICON_PLAYBACK, NULL, NULL);
			if(osd_mode_flag)
			{
				ap_state_handling_icon_show_cmd(ICON_CAPTURE, NULL, NULL);
			}
			else
			{
				ap_state_handling_icon_show_cmd(ICON_PLAYBACK_PLAY, NULL, NULL);
			}

			if(wAVIWidth==640) {
				ascii_str.str_ptr = "VGA";
			} else if (wAVIWidth==1280) {
				ascii_str.str_ptr = "1.3M";
			} else if (wAVIWidth==1920) {
				ascii_str.str_ptr = "2MHD";
			} else if (wAVIWidth==2048) {
				ascii_str.str_ptr = "3M";
			} else if (wAVIWidth==2592) {
				ascii_str.str_ptr = "5M";
			} else if (wAVIWidth==3264) {
				ascii_str.str_ptr = "8M";
			} else if (wAVIWidth==3648) {
				ascii_str.str_ptr = "10M";
			} else if (wAVIWidth==4032) {
				ascii_str.str_ptr = "12M";
			}
		}

		ascii_str.buff_w = browse_resolution_str.w = ASCII_draw_char_width*strlen(ascii_str.str_ptr);

		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			browse_resolution_str.pos_x = TFT_WIDTH-5-browse_resolution_str.w;
		} else { //TV
		  #if TV_WIDTH == 720
			browse_resolution_str.pos_x = TV_WIDTH-40-browse_resolution_str.w;
		  #elif TV_WIDTH == 640
			browse_resolution_str.pos_x = TV_WIDTH-15-browse_resolution_str.w;
		  #endif
		}

		if(wAVIWidth) {
			ap_state_handling_ASCII_str_draw(&browse_resolution_str, &ascii_str);
		}
	}
}

void ap_browse_show_file_name(INT8U enable, INT32U buf_addr)
{
	if(enable){
		STRING_ASCII_INFO ascii_str;
		if(ap_display_get_device() == DISP_DEV_TFT) {
			ascii_str.font_color = BROWSE_FILE_NAME_COLOR;
			ascii_str.font_type = 0;
			ascii_str.pos_x = BROWSE_FILE_NAME_START_X;
			ascii_str.pos_y = BROWSE_FILE_NAME_START_Y;
			ascii_str.str_ptr = (CHAR *) browse_curr_avi.file_path_addr;
			ascii_str.buff_w = TFT_WIDTH;
			ascii_str.buff_h = TFT_HEIGHT;
			ap_state_resource_string_ascii_draw((INT16U *) buf_addr, &ascii_str, RGB565_DRAW);
		} else if(ap_display_get_device() == DISP_DEV_TV) {
			ascii_str.font_color = BROWSE_FILE_NAME_COLOR;
			ascii_str.font_type = 0;
			ascii_str.pos_x = BROWSE_FILE_NAME_START_X_TV;
			ascii_str.pos_y = BROWSE_FILE_NAME_START_Y_TV;
			ascii_str.str_ptr = (CHAR *) browse_curr_avi.file_path_addr;
			ascii_str.buff_w = TV_WIDTH;
			ascii_str.buff_h = TV_HEIGHT;
			ap_state_resource_string_ascii_draw((INT16U *) buf_addr, &ascii_str, RGB565_DRAW);
		} else { //HDMI
			ascii_str.font_color = 0xff80; //not used for YUV420_DRAW, only for YUYV_DRAW
			ascii_str.font_type = 1; //big ascii
			ascii_str.pos_x = 600;
			ascii_str.pos_y = 30;
			ascii_str.str_ptr = (CHAR *) browse_curr_avi.file_path_addr;
			ascii_str.buff_w = 1280;
			ascii_str.buff_h = 720;
#ifdef HDMI_JPG_DECODE_AS_GP420
			ap_state_resource_string_ascii_draw((INT16U *) buf_addr, &ascii_str, YUV420_DRAW);
#elif defined(HDMI_JPG_DECODE_AS_YUV422)
            ap_state_resource_string_ascii_draw((INT16U *) buf_addr, &ascii_str, YUYV_DRAW);
#endif			
		}
	} else {
		INT32S logo_img_ptr;
		IMAGE_DECODE_STRUCT img_info;
		INT32U size;
		INT16U logo_fd;

		logo_fd = nv_open((INT8U *) "AUDIO_REC_BG.JPG");
		if (logo_fd != 0xFFFF) {
			size = nv_rs_size_get(logo_fd);
			logo_img_ptr = (INT32S) gp_malloc(size);
			if (!logo_img_ptr) {
				DBG_PRINT("State browser allocate jpeg input buffer fail.[%d]\r\n", size);
				return;
			}
			if (nv_read(logo_fd, (INT32U) logo_img_ptr, size)) {
				DBG_PRINT("Failed to read resource_header in ap_browse_reply_action()\r\n");
				gp_free((void *) logo_img_ptr);
				return;
			}
			img_info.image_source = (INT32S) logo_img_ptr;
			img_info.source_size = size;
			img_info.source_type = TK_IMAGE_SOURCE_TYPE_BUFFER;
			img_info.output_format = C_SCALER_CTRL_OUT_RGB565;
			img_info.output_ratio = 0;
			img_info.out_of_boundary_color = 0x008080;
			img_info.output_buffer_width = TFT_WIDTH;
			img_info.output_buffer_height = TFT_HEIGHT;
			img_info.output_image_width = TFT_WIDTH;
			img_info.output_image_height = TFT_HEIGHT;
			img_info.output_buffer_pointer = buf_addr;
			if (jpeg_buffer_decode_and_scale(&img_info) == STATUS_FAIL) {
				gp_free((void *) logo_img_ptr);
				DBG_PRINT("State browser decode AUDIO_REC_BG.jpeg file fail.\r\n");
				return;
			}
			gp_free((void *) logo_img_ptr);
		}
	}
}

void ap_browse_wav_play(void)
{
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_NORMAL);

	browse_curr_avi.file_handle = open((CHAR *) browse_curr_avi.file_path_addr, O_RDONLY);
	if (browse_curr_avi.file_handle >= 0) {
		//gpio_write_io(SPEAKER_EN, 1);
		audio_wav_play(browse_curr_avi.file_handle);
		timer_counter_force_display_browser(1);
		ap_peripheral_auto_off_force_disable_set(1);
		//ap_state_handling_led_blink_on();
		ap_browse_display_timer_start();
		//ap_browse_show_file_name(0);
	} else {
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
	}
}


void ap_browse_wav_stop(void)
{
	INT32U led_type;

	ap_browse_sts_set(~(BROWSE_PLAYBACK_BUSY | BROWSE_PLAYBACK_PAUSE));
	ap_peripheral_auto_off_force_disable_set(0);
	led_type = LED_WAITING_RECORD;
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
	ap_browse_display_timer_stop();
	timer_counter_force_display_browser(0);
	//ap_browse_show_file_name(1);
	audio_wav_stop();
	
	if ((ap_display_get_device()==DISP_DEV_TFT) || (ap_display_get_device()==DISP_DEV_TV)) {
		//OSQPost(DisplayTaskQ, (void *) (browse_output_buff|MSG_DISPLAY_TASK_JPEG_DRAW));	
	} else if (ap_display_get_device()==DISP_DEV_HDMI) {
		//OSQPost(DisplayTaskQ, (void *) (browse_output_buff|MSG_DISPLAY_TASK_HDMI_JPEG_DRAW));
	}
	
	//close(browse_curr_avi.file_handle); //wwj mark, closed by audio player
	//gpio_write_io(SPEAKER_EN, DATA_LOW);
}

static INT8S browse_sts_tmp;
INT8S ap_browse_stop_handle(void)
{
	browse_sts_tmp = browse_sts;
	if (browse_sts & (BROWSE_PLAYBACK_BUSY | BROWSE_PLAYBACK_PAUSE)) {
		if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV){
			ap_browse_wav_stop();
		} else {
			ap_browse_mjpeg_stop();
			if(browse_curr_avi.file_type != TK_IMAGE_TYPE_JPEG) {
				//ap_state_handling_icon_clear_cmd(ICON_PLAYBACK_PLAY, ICON_PLAYBACK_PAUSE, NULL);
				ap_state_handling_icon_show_cmd(ICON_PLAYBACK, NULL, NULL);
			}
		}
   		close(browse_curr_avi.file_handle);
		ap_state_handling_icon_clear_cmd(ICON_PAUSE, ICON_PLAY, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
	} else {
		ap_state_handling_icon_clear_cmd(ICON_PLAY, NULL, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAY1, NULL, NULL);
	}
	return browse_sts_tmp;
}

void ap_browse_display_update_command(void)
{
	INT32U search_type;

	#if DUAL_STREAM_FUNC_ENABLE
	if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
	#endif
	{    					
	    if(g_browser_reply_action_flag &&
		  (!s_usbd_pin) &&
		  ((browse_sts & (BROWSE_PLAYBACK_BUSY + BROWSE_PLAYBACK_PAUSE)) == 0)
	    ) {
			g_browser_reply_action_flag = 0;
			search_type = STOR_SERV_SEARCH_ORIGIN;
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_BROWSE_REQ, (void *)&search_type, sizeof(INT32U), MSG_PRI_NORMAL);
		}
	}
}

void ap_browse_no_media_show(INT16U str_type)
{
	if ((ap_display_get_device()==DISP_DEV_TFT) || (ap_display_get_device()==DISP_DEV_TV)) {

		INT32U i, *buff_ptr, cnt, display_addr;

		display_addr = 0;
		do {
			display_addr = ap_display_queue_get(display_isr_queue);
			OSTimeDly(10);
		} while(!display_addr);

		buff_ptr = (INT32U *)display_addr;
		cnt = getDispDevBufSize()>>2;
		for (i=0; i<cnt; i++) {
			*buff_ptr++ = 0;
		}

	#if C_BATTERY_DETECT == CUSTOM_ON && USE_ADKEY_NO
		ap_state_handling_current_bat_lvl_show();
		ap_state_handling_current_charge_icon_show();
	#endif
		ap_state_handling_str_draw_exit();
		ap_state_handling_str_draw(str_type, WARNING_STR_COLOR);
		ap_state_handling_icon_clear_cmd(ICON_PLAY, NULL, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAY1, NULL, NULL);
	   	ap_state_handling_icon_clear_cmd(ICON_LOCKED1, NULL, NULL);

		OSQPost(DisplayTaskQ, (void *) (display_addr|MSG_DISPLAY_TASK_MJPEG_DRAW));
	} else if (ap_display_get_device()==DISP_DEV_HDMI) {
	
#ifdef HDMI_JPG_DECODE_AS_GP420	
		INT32U display_addr;

		INT32U x, y, offset_pixel, offset_pixel_tmp;
		INT8U Y0, Y1, U, V;

		display_addr = 0;
		do {
			display_addr = ap_display_queue_get(display_isr_queue);
			OSTimeDly(10);
		} while(!display_addr);

		Y0 = Y1 = 0;	//black
		U = V = 0x80;	//black

		offset_pixel_tmp = 0;
		for(y=0 ; y<HDMI_HEIGHT ; y++) {
			for(x=0; x<(HDMI_WIDTH/2); x++) {
				offset_pixel = offset_pixel_tmp + x*2;

				if((y >= 180 && y < 540) && (x >= 160 && x < 480)) { //blue
					*(INT8U *)((INT8U *)display_addr + offset_pixel) = 0x1d;
					*(INT8U *)((INT8U *)display_addr + offset_pixel + 1) = 0x1d;
				} else { //black
					*(INT8U *)((INT8U *)display_addr + offset_pixel) = Y0;
					*(INT8U *)((INT8U *)display_addr + offset_pixel + 1) = Y1;
				}
			}

			if((y&0x0001) == 0) {
				offset_pixel_tmp += HDMI_WIDTH;
				for(x=0; x<(HDMI_WIDTH/2); x++) {
					offset_pixel = offset_pixel_tmp + x*2;
					if((y >= 180 && y < 540) && (x >= 160 && x < 480)) { //blue
						*(INT8U *)((INT8U *)display_addr + offset_pixel) = 0xff;
						*(INT8U *)((INT8U *)display_addr + offset_pixel + 1) = 0x6b;
					} else { //black
						*(INT8U *)((INT8U *)display_addr + offset_pixel) = U;
						*(INT8U *)((INT8U *)display_addr + offset_pixel + 1) = V;
					}
				}
			}
			offset_pixel_tmp += HDMI_WIDTH;
		}

	#if C_BATTERY_DETECT == CUSTOM_ON && USE_ADKEY_NO
		ap_state_handling_current_bat_lvl_show();
		ap_state_handling_current_charge_icon_show();
	#endif
		ap_state_handling_str_draw_exit();
		ap_state_handling_str_draw_HDMI(str_type, 0xff80);	//white
		ap_state_handling_icon_clear_cmd(ICON_PLAY, NULL, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAY1, NULL, NULL);
	   	ap_state_handling_icon_clear_cmd(ICON_LOCKED1, NULL, NULL);

		OSQPost(DisplayTaskQ, (void *) (display_addr|MSG_DISPLAY_TASK_HDMI_MJPEG_DRAW));
#elif defined(HDMI_JPG_DECODE_AS_YUV422)

		INT32U display_addr;
		INT32U x, y, offset_pixel, offset_pixel_tmp;

		display_addr = 0;
		do {
			display_addr = ap_display_queue_get(display_isr_queue);
			OSTimeDly(10);
		} while(!display_addr);


		offset_pixel_tmp = 0;
		for(y=0 ; y<HDMI_HEIGHT ; y++) {
			for(x=0; x<HDMI_WIDTH; x+=2) {
				offset_pixel = offset_pixel_tmp + x;
        
				if(((y >= 150) && (y < HDMI_HEIGHT-150)) && ((x >= 200) && x < (HDMI_WIDTH-200))) { //blue
					*(INT32U *)((INT16U *)display_addr + offset_pixel) = 0x1DFF1D6B ;
				} else { //black
					*(INT32U *)((INT16U *)display_addr + offset_pixel) = 0x00800080 ;
				}
			}
			offset_pixel_tmp += HDMI_WIDTH;
		}

	#if C_BATTERY_DETECT == CUSTOM_ON && USE_ADKEY_NO
		ap_state_handling_current_bat_lvl_show();
		ap_state_handling_current_charge_icon_show();
	#endif
		ap_state_handling_str_draw_exit();
		ap_state_handling_str_draw_HDMI(str_type, 0xff80);	//white
		ap_state_handling_icon_clear_cmd(ICON_PLAY, NULL, NULL);
		ap_state_handling_icon_clear_cmd(ICON_PLAY1, NULL, NULL);
	   	ap_state_handling_icon_clear_cmd(ICON_LOCKED1, NULL, NULL);

		OSQPost(DisplayTaskQ, (void *) (display_addr|MSG_DISPLAY_TASK_HDMI_MJPEG_DRAW));
#endif		
	}
}


void ap_browse_display_timer_start(void)
{
	if (browse_display_timerid == 0xFF) {
		browse_display_timerid = VIDEO_RECORD_CYCLE_TIMER_ID;
		sys_set_timer((void*)msgQSend, (void*)ApQ, MSG_APQ_DISPLAY_DRAW_TIMER, browse_display_timerid, BROWSE_DISPLAY_TIMER_INTERVAL);
	}
}

void ap_browse_display_timer_stop(void)
{
	if (browse_display_timerid != 0xFF) {
		sys_kill_timer(browse_display_timerid);
		browse_display_timerid = 0xFF;
	}
}

void ap_browse_display_timer_draw(void)
{
	/*
	if(browse_curr_avi.file_type == TK_IMAGE_TYPE_WAV){
		if ((ap_display_get_device()==DISP_DEV_TFT) || (ap_display_get_device()==DISP_DEV_TV)) {
			OSQPost(DisplayTaskQ, (void *) (browse_output_buff|MSG_DISPLAY_TASK_WAV_TIME_DRAW));	
		} else if (ap_display_get_device()==DISP_DEV_HDMI) {
			OSQPost(DisplayTaskQ, (void *) (browse_output_buff|MSG_DISPLAY_TASK_HDMI_WAV_TIME_DRAW));
		}
	} else {
		if ((ap_display_get_device()==DISP_DEV_TFT) || (ap_display_get_device()==DISP_DEV_TV)) {
			OSQPost(DisplayTaskQ, (void *) (browse_output_buff|MSG_DISPLAY_TASK_JPEG_DRAW));	
		} else if (ap_display_get_device()==DISP_DEV_HDMI) {
			OSQPost(DisplayTaskQ, (void *) (browse_output_buff|MSG_DISPLAY_TASK_HDMI_JPEG_DRAW));
		}
	} */
}

INT32S ap_browse_mjpeg_decode(void)
{
	INT32S ret = 0;

	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_NORMAL);

	browse_curr_avi.file_handle = open((CHAR *) browse_curr_avi.file_path_addr, O_RDONLY);
	if (browse_curr_avi.file_handle >= 0) {
		if (vid_dec_parser_start(browse_curr_avi.file_handle, 1, NULL)) {
			ret = STATUS_FAIL;
		}
		if (ret != STATUS_FAIL) {
			if (audio_playing_state_get() == STATE_PLAY) {
				audio_send_pause();
				OSTimeDly(2*MAX_DAC_BUFFERS);
			}
	  	}
		if (vid_dec_start()) {
			ret = STATUS_FAIL;
		}
		if (ret == STATUS_FAIL) {
			ap_browse_mjpeg_stop();
			close(browse_curr_avi.file_handle);
			//if ( (audio_playing_state_get() == STATE_IDLE) || (audio_playing_state_get() == STATE_PAUSED) ){
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
			//}
		} else {
			#if DUAL_STREAM_FUNC_ENABLE
			if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
			{
				gpio_write_io(SPEAKER_EN, 1);
				OSTimeDly(10);
			}
			#endif
			timer_counter_force_display_browser(1);
			ap_peripheral_auto_off_force_disable_set(1);
			//ap_state_handling_led_blink_on();
		}
	} else {
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
	}
	return ret;
}

INT32S ap_browse_wifi_file_index_set(INT16U num)
{
	wifi_playback_file_index = num;
	return 0;
}

INT16U ap_browse_wifi_file_index_get(void)
{
	return wifi_playback_file_index;
}


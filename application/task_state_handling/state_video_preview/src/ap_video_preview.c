#include "ap_state_config.h"
#include "ap_video_preview.h"
#include "ap_state_handling.h"
#include "ap_music.h"
#include "application.h"
#include "my_video_codec_callback.h"
#include "ap_peripheral_handling.h"
#include "ap_display.h"
#include "state_wifi.h"

static STR_ICON_EXT	capture_resolution_str;
static STR_ICON_EXT left_capture_num_str;

static INT32U pic_size[3][8] = {//unit: KB
		{1410, 1220, 1030, 753, 529, 385, 227, 88}, //fine
		{1120,  996,  844, 603, 433, 310, 195, 79}, //normal
		{ 810,  692,  577, 396, 273, 142,  95, 43}  //economy
};

INT32U last_pic_size = 0;//unit: KB
INT32U last_pic_size_temp = 0;

INT8U frame_mode_en = 0;

INT32U peri_video_resolution = 0;

INT32U left_capture_num = 0;

extern INT32U g_wifi_sd_full_flag;

extern void sensor_set_fps(INT32U fpsValue);
void ap_video_capture_mode_switch(INT8U DoSensorInit, INT16U EnterAPMode)
{
	VIDEO_ARGUMENT arg;
	INT32U video_resolution;
	INT32U CropWValue;
	INT32U CropHValue;
	INT32U ZoomSetpValue;
	
	arg.bScaler = 1;
	arg.bSensorDoInit = DoSensorInit;

	if(EnterAPMode == STATE_VIDEO_PREVIEW)
	{
		video_resolution = ap_state_config_pic_size_get();

		if(peri_video_resolution != video_resolution)
		{
			video_preview_zoom_to_zero();
			peri_video_resolution = video_resolution;
		}
		
		switch(video_resolution)
		{
			case 0: // 4032*3024
			case 1: // 3648*2736
			case 2: // 3264*2448
			case 3: // 2592*1944
			case 4: // 2048*1536
			case 6: // 1280*960
			case 7: // 640*480
			    arg.SensorWidth = 960; 
			    arg.SensorHeight  = AVI_HEIGHT_720P; 
			break;
			case 5: // 1920*1080
			    arg.SensorWidth = AVI_WIDTH_720P; 
			    arg.SensorHeight  = AVI_HEIGHT_720P; 
			break;
		}	
		
		ZoomSetpValue = video_preview_zoom_setp_get();
		sensor_crop_W_H_get(&CropWValue,&CropHValue);
		if(ZoomSetpValue > 0)
		{
		    arg.ClipWidth = arg.SensorWidth-(ZoomSetpValue*CropWValue); 
		    arg.ClipHeight =  arg.SensorHeight-(ZoomSetpValue*CropHValue); 
		}
		else
		{
		    arg.ClipWidth = arg.SensorWidth; 
		    arg.ClipHeight =  arg.SensorHeight; 
		}
		
	}
	else if(EnterAPMode == STATE_VIDEO_RECORD)
	{
		video_preview_zoom_to_zero();

		#if ENABLE_SAVE_SENSOR_RAW_DATA
		video_resolution = 2;
		#else
		video_resolution = ap_state_config_video_resolution_get();
		#endif
		//++++++++++++++++++++++++++++++++++++
		//if (video_resolution < 2) sensor_set_fps(22);
		//else sensor_set_fps(25);		
		sensor_set_fps(20);
		//++++++++++++++++++++++++++++++++++++
		switch(video_resolution)
		{
			case 0: //1080FHD 1920X1080
				arg.TargetWidth = AVI_WIDTH_1080FHD;
				arg.TargetHeight = AVI_HEIGHT_1080FHD;
			    arg.SensorWidth = AVI_WIDTH_720P; 
			    arg.SensorHeight  = AVI_HEIGHT_720P; 
				
			break;
			case 1: //1080P 1440X1080
				arg.TargetWidth = AVI_WIDTH_1080P;
				arg.TargetHeight = AVI_HEIGHT_1080P;
			    arg.SensorWidth = 960; 
			    arg.SensorHeight  = AVI_HEIGHT_720P; 
			break;
			case 2: //720P 1280X720
			default:
				arg.TargetWidth = AVI_WIDTH_720P;
				arg.TargetHeight = AVI_HEIGHT_720P;
			    arg.SensorWidth = AVI_WIDTH_720P; 
			    arg.SensorHeight  = AVI_HEIGHT_720P; 
			break;
			case 3: //WVGA 848X480
				arg.TargetWidth = AVI_WIDTH_WVGA;
				arg.TargetHeight = AVI_HEIGHT_WVGA;
			    arg.SensorWidth = AVI_WIDTH_WVGA; 
			    arg.SensorHeight  = AVI_HEIGHT_WVGA; 
			break;
			case 4: //VGA 640X480
				arg.TargetWidth = AVI_WIDTH_VGA;
				arg.TargetHeight = AVI_HEIGHT_VGA;
			    arg.SensorWidth = AVI_WIDTH_VGA; 
			    arg.SensorHeight  = AVI_HEIGHT_VGA; 
			break;
			case 5: //QVGA 320X240
				arg.TargetWidth = AVI_WIDTH_QVGA;
				arg.TargetHeight = AVI_HEIGHT_QVGA;
			    arg.SensorWidth = AVI_WIDTH_QVGA; 
			    arg.SensorHeight  = AVI_HEIGHT_QVGA; 
			break;
		}

	    arg.ClipWidth = arg.SensorWidth; 
	    arg.ClipHeight = arg.SensorHeight; 
		
	}
	else if(EnterAPMode == STATE_CONNECT_TO_PC)
	{
		video_preview_zoom_to_zero();

		arg.TargetWidth = AVI_WIDTH_720P;
		arg.TargetHeight = AVI_HEIGHT_720P;
	    arg.SensorWidth = AVI_WIDTH_720P; 
	    arg.SensorHeight  = AVI_HEIGHT_720P; 

	    arg.ClipWidth = arg.SensorWidth; 
	    arg.ClipHeight = arg.SensorHeight; 
	}

	if(ap_display_get_device()==DISP_DEV_TV)
	{
		arg.DisplayWidth = TV_WIDTH;
		arg.DisplayHeight = TV_HEIGHT;
		arg.DisplayBufferWidth = TV_WIDTH;
		arg.DisplayBufferHeight = TV_HEIGHT;	
	}
	else
	{
		#if DUAL_STREAM_FUNC_ENABLE
		if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
		{
			arg.DisplayWidth = WIFI_JPEG_WIDTH;
			arg.DisplayHeight = WIFI_JPEG_HEIGHT;
			arg.DisplayBufferWidth = WIFI_JPEG_WIDTH;
			arg.DisplayBufferHeight = WIFI_JPEG_HEIGHT;	
		}
		else
		#endif
		{			
			arg.DisplayWidth = TFT_WIDTH;
			arg.DisplayHeight = TFT_HEIGHT;
			arg.DisplayBufferWidth = TFT_WIDTH;
			arg.DisplayBufferHeight = TFT_HEIGHT;	
		}
	}

	arg.bEnterApMode = EnterAPMode;
	arg.VidFrameRate = AVI_FRAME_RATE;
	arg.AudSampleRate = AUD_REC_SAMPLING_RATE;
	arg.OutputFormat = IMAGE_OUTPUT_FORMAT_RGB565; 

	video_encode_preview_start(arg);
}

#define ICON_CAPTURE_OFFSET	10
void ap_video_preview_init(void)
{	
	//INT8U night_mode;
	INT8U temp;

	OSQPost(DisplayTaskQ, (void *) MSG_DISPLAY_TASK_EFFECT_INIT);
	//DBG_PRINT("ap_video_preview_init():MSG_SCALER_TASK_PREVIEW_ON\r\n");	
	
	/*if (ap_state_handling_storage_id_get() == NO_STORAGE) {
		ap_state_handling_icon_show_cmd(ICON_CAPTURE, ICON_INTERNAL_MEMORY, NULL);
	} else {
		ap_state_handling_icon_show_cmd(ICON_CAPTURE, ICON_SD_CARD, NULL);
	}*/

	ap_state_handling_icon_show_cmd(ICON_CAPTURE, NULL, NULL);
	video_capture_resolution_clear(); 
	video_capture_resolution_display();
	left_capture_num = cal_left_capture_num();
	left_capture_num_display(left_capture_num);

	/*temp = ap_state_config_ev1_get();
	ap_state_handling_icon_show_cmd(ICON_CAPTURE_EV6+temp, NULL, NULL);
	gp_cdsp_set_ev_val(temp);	//0:+2, 1:+5/3, 2:+4/3, 3:+1.0, 4:+2/3, 5:+1/3, 6:+0.0, 7:-1/3, 8:-2/3, 9:-1.0, 10:-4/3, 11:-5/3, 12:-2.0 
	
	temp = ap_state_config_white_balance_get();
	ap_state_handling_icon_show_cmd(ICON_CAPTURE_WHITE_BALANCE_AUTO+temp, NULL, NULL);
	
	temp = ap_state_config_iso_get();
	ap_state_handling_icon_show_cmd(ICON_CAPTURE_ISO_AUTO+temp, NULL, NULL);
	
	temp = ap_state_config_anti_shaking_get();
	if(temp) {
		ap_state_handling_icon_show_cmd(ICON_CAPTURE_ANTI_SHAKING, NULL, NULL);
	}*/

	temp = ap_state_config_burst_get();
	if(temp) {
		ap_state_handling_icon_show_cmd(ICON_CAP_MODE_3S_Y+temp-1, NULL, NULL);
	}
	
	/*temp = ap_state_config_quality_get();
	ap_state_handling_icon_show_cmd(ICON_CAPTURE_QUALITY_FINE+temp, NULL, NULL);*/

#if C_BATTERY_DETECT == CUSTOM_ON && USE_ADKEY_NO
	ap_state_handling_current_bat_lvl_show();
	ap_state_handling_current_charge_icon_show();
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

}

void ap_video_preview_timedelay_cap_icon_set(INT8U enable)
{
	INT8U temp;
	
	temp = ap_state_config_burst_get();	
	if(temp)
	{
		if(enable)
		{
			ap_state_handling_icon_clear_cmd(ICON_CAP_MODE_3S_Y+temp-1, NULL, NULL);	
			ap_state_handling_icon_show_cmd(ICON_CAP_MODE_3S_R+temp-1, NULL, NULL);
		}
		else
		{
			ap_state_handling_icon_clear_cmd(ICON_CAP_MODE_3S_R+temp-1, NULL, NULL);	
			ap_state_handling_icon_show_cmd(ICON_CAP_MODE_3S_Y+temp-1, NULL, NULL);
		}
	}
}

void ap_video_preview_timedelay_cap_icon_clear(void)
{
	INT8U temp;
	
	temp = ap_state_config_burst_get();	
	if(temp)
	{
		ap_state_handling_icon_clear_cmd(ICON_CAP_MODE_3S_R+temp-1, NULL, NULL);	
		ap_state_handling_icon_clear_cmd(ICON_CAP_MODE_3S_Y+temp-1, NULL, NULL);
	}
}

void ap_video_preview_exit(void)
{
	ap_state_handling_str_draw_exit();
	video_capture_resolution_clear();
	left_capture_num_str_clear();
	ap_state_handling_icon_clear_cmd(ICON_CAPTURE, NULL, NULL);
	ap_video_preview_timedelay_cap_icon_clear();
}

void video_preview_card_full_handle(void)
{
	INT32U led_type;
    ap_state_handling_str_draw_exit();
    ap_state_handling_str_draw(STR_SD_FULL, WARNING_STR_COLOR);
    g_wifi_sd_full_flag = 1;
    msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
    led_type =LED_CARD_NO_SPACE;// LED_CARD_NO_SPACE;//LED_SDC_FULL;
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
}

INT32S ap_video_preview_func_key_active(void)
{
	INT32U led_type; 
    if (ap_state_handling_storage_id_get() != NO_STORAGE) {
    	if ((vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20) < CARD_FULL_MB_SIZE) {
	        DBG_PRINT ("Card full, key action avoid!!!\r\n");
	        video_preview_card_full_handle();
		{  // modify by josephhsieh@20151026  for GoPlusCam
		   gp_capture_card_full();
		   ap_state_config_burst_set(0);
	        }
	        return	STATUS_FAIL;
	    }
    } else {
		ap_state_handling_str_draw_exit();
		//ap_state_handling_str_draw(STR_INSERT_SDC, WARNING_STR_COLOR);
		//msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_DISPLAY_PLEASE_INSERT_SDC, NULL, NULL, MSG_PRI_NORMAL);
		//return	STATUS_FAIL;
		led_type = LED_NO_SDC;
	    msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
    	return	STATUS_FAIL;
    }

	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_PIC_REQ, NULL, NULL, MSG_PRI_NORMAL);
	return	STATUS_OK;
}

INT32S ap_video_preview_reply_action(STOR_SERV_FILEINFO *file_info_ptr)
{
	MEDIA_SOURCE src;
	INT32U file_path_addr = file_info_ptr->file_path_addr;
	INT32U led_type;
	//INT16S free_size = file_info_ptr->storage_free_size;
	
	src.type_ID.FileHandle = file_info_ptr->file_handle;
	src.type = SOURCE_TYPE_FS;
	src.Format.VideoFormat = MJPEG;
	if (src.type_ID.FileHandle >=0/* && free_size > 1*/) {
		if (ap_state_config_preview_get()) {
			OSQPost(DisplayTaskQ, (void *) MSG_DISPLAY_TASK_PIC_EFFECT);
		} else {
			OSQPost(DisplayTaskQ, (void *) MSG_DISPLAY_TASK_PIC_PREVIEW_EFFECT);
		}
		if (video_encode_capture_picture(src) != START_OK) {
			unlink((CHAR *) file_path_addr);
			return	STATUS_FAIL;
		} else {
			//ap_state_handling_led_flash_on();
			return	STATUS_OK;
		}
	} else {
		ap_state_handling_str_draw_exit();
		if (src.type_ID.FileHandle >= 0) {
			g_wifi_sd_full_flag = 1;
			ap_state_handling_str_draw(STR_SD_FULL, WARNING_STR_COLOR);
			close(src.type_ID.FileHandle);
			led_type =LED_CARD_NO_SPACE;//LED_CARD_NO_SPACE;// LED_SDC_FULL;
			msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
		} else {
			ap_state_handling_str_draw(STR_NO_SD, WARNING_STR_COLOR);
		}
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
		return	STATUS_FAIL;
	}
}

void ap_video_preview_reply_done(INT8U ret, INT32U file_path_addr)
{
    struct stat_t buf_tmp;
    INT32U temp;
    INT32S temp1;
    INT64U free_space;

	if (!ret) {
		stat((CHAR *) file_path_addr, &buf_tmp);
		temp = buf_tmp.st_size >> 10;
		temp1 = temp - last_pic_size_temp;
		last_pic_size_temp = last_pic_size_temp+temp1;
		DBG_PRINT("Capture OK, Size=%d kbtye\r\n",buf_tmp.st_size>>10);
	} else {
		unlink((CHAR *) file_path_addr);
		DBG_PRINT("Capture Fail\r\n");
	}
	free_space = vfsFreeSpace(MINI_DVR_STORAGE_TYPE);
	DBG_PRINT("free_space(Mbyte)=%d \r\n",(free_space>>20)-CARD_FULL_MB_SIZE);
	if((free_space>>20)<20)  //<20Mbyte
	{
		DBG_PRINT("left_capture_num old=%d \r\n",left_capture_num);
		left_capture_num = cal_left_capture_num();
		DBG_PRINT("left_capture_num=%d \r\n",left_capture_num);
	}else{
		if(left_capture_num){
			left_capture_num--;
		}
	}

	#if DUAL_STREAM_FUNC_ENABLE
	if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
	#endif
	{
		left_capture_num_display(left_capture_num);
	}
	
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
}

void ap_video_preview_left_capture_num_thinking()
{
	if(last_pic_size_temp){
		last_pic_size = last_pic_size_temp;
	}
}
void video_capture_resolution_display(void)
{
	INT8U temp;
	STRING_ASCII_INFO ascii_str;

	ascii_str.font_color = 0x001F;
	ascii_str.font_type = 0;
	ascii_str.pos_x = 0;
	ascii_str.pos_y = 0;

	if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
		capture_resolution_str.pos_y = 5;
	} else { //TV
		capture_resolution_str.pos_y = 28;
	}
	ascii_str.buff_h = capture_resolution_str.h = ASCII_draw_char_height;

	temp = ap_state_config_pic_size_get();
	if (temp == 0) {
		ascii_str.str_ptr = "12M";
		ascii_str.buff_w = capture_resolution_str.w = ASCII_draw_char_width*3;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			capture_resolution_str.pos_x = 10+33;//TFT_WIDTH-ICON_CAPTURE_OFFSET-ASCII_draw_char_width*3;
		} else { //TV
		  #if TV_WIDTH == 720
			capture_resolution_str.pos_x = TV_WIDTH-45-ASCII_draw_char_width*3;
		  #elif TV_WIDTH == 640
			capture_resolution_str.pos_x = 20+33;//TV_WIDTH-20-ASCII_draw_char_width*3;
		  #endif
		}
	} else if (temp == 1) {
		ascii_str.str_ptr = "10M";
		ascii_str.buff_w = capture_resolution_str.w = ASCII_draw_char_width*3;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			capture_resolution_str.pos_x = 10+33;//TFT_WIDTH-ICON_CAPTURE_OFFSET-ASCII_draw_char_width*3;
		} else { //TV
		  #if TV_WIDTH == 720
			capture_resolution_str.pos_x = TV_WIDTH-45-ASCII_draw_char_width*3;
		  #elif TV_WIDTH == 640
			capture_resolution_str.pos_x = 20+33;//TV_WIDTH-20-ASCII_draw_char_width*3;
		  #endif
		}
	} else if (temp == 2) {
		ascii_str.str_ptr = "8M";
		ascii_str.buff_w = capture_resolution_str.w = ASCII_draw_char_width*2;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			capture_resolution_str.pos_x = 10+33;//TFT_WIDTH-ICON_CAPTURE_OFFSET-ASCII_draw_char_width*2;
		} else { //TV
		  #if TV_WIDTH == 720
			capture_resolution_str.pos_x = TV_WIDTH-45-ASCII_draw_char_width*2;
		  #elif TV_WIDTH == 640
			capture_resolution_str.pos_x = 20+33;//TV_WIDTH-20-ASCII_draw_char_width*2;
		  #endif
		}
	} else if (temp == 3) {
		ascii_str.str_ptr = "5M";
		ascii_str.buff_w = capture_resolution_str.w = ASCII_draw_char_width*2;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			capture_resolution_str.pos_x = 10+33;//TFT_WIDTH-ICON_CAPTURE_OFFSET-ASCII_draw_char_width*2;
		} else { //TV
		  #if TV_WIDTH == 720
			capture_resolution_str.pos_x = TV_WIDTH-45-ASCII_draw_char_width*2;
		  #elif TV_WIDTH == 640
			capture_resolution_str.pos_x = 20+33;//TV_WIDTH-20-ASCII_draw_char_width*2;
		  #endif
		}
	} else if (temp == 4) {
		ascii_str.str_ptr = "3M";
		ascii_str.buff_w = capture_resolution_str.w = ASCII_draw_char_width*2;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			capture_resolution_str.pos_x = 10+33;//TFT_WIDTH-ICON_CAPTURE_OFFSET-ASCII_draw_char_width*2;
		} else { //TV
		  #if TV_WIDTH == 720
			capture_resolution_str.pos_x = TV_WIDTH-45-ASCII_draw_char_width*2;
		  #elif TV_WIDTH == 640
			capture_resolution_str.pos_x = 20+33;//TV_WIDTH-20-ASCII_draw_char_width*2;
		  #endif
		}
	} else if (temp == 5) {
		ascii_str.str_ptr = "2MHD";
		ascii_str.buff_w = capture_resolution_str.w = ASCII_draw_char_width*4;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			capture_resolution_str.pos_x = 10+33;//TFT_WIDTH-ICON_CAPTURE_OFFSET-ASCII_draw_char_width*2;
		} else { //TV
		  #if TV_WIDTH == 720
			capture_resolution_str.pos_x = TV_WIDTH-45-ASCII_draw_char_width*2;
		  #elif TV_WIDTH == 640
			capture_resolution_str.pos_x = 20+33;//TV_WIDTH-20-ASCII_draw_char_width*2;
		  #endif
		}
	} else if (temp == 6) {
		ascii_str.str_ptr = "1.3M";
		ascii_str.buff_w = capture_resolution_str.w = ASCII_draw_char_width*4;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			capture_resolution_str.pos_x = 10+33;//TFT_WIDTH-ICON_CAPTURE_OFFSET-ASCII_draw_char_width*4;
		} else { //TV
		  #if TV_WIDTH == 720
			capture_resolution_str.pos_x = TV_WIDTH-45-ASCII_draw_char_width*4;
		  #elif TV_WIDTH == 640
			capture_resolution_str.pos_x = 20+33;//TV_WIDTH-20-ASCII_draw_char_width*4;
		  #endif
		}
	} else {
		ascii_str.str_ptr = "VGA";
		ascii_str.buff_w = capture_resolution_str.w = ASCII_draw_char_width*3;
		if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
			capture_resolution_str.pos_x = 10+33;//TFT_WIDTH-ICON_CAPTURE_OFFSET-ASCII_draw_char_width*3;
		} else { //TV
		  #if TV_WIDTH == 720
			capture_resolution_str.pos_x = TV_WIDTH-45-ASCII_draw_char_width*3;
		  #elif TV_WIDTH == 640
			capture_resolution_str.pos_x = 20+33;//TV_WIDTH-20-ASCII_draw_char_width*3;
		  #endif
		}
	}
	ascii_str.buff_w += 2;				//for draw font frame
	ascii_str.buff_h += 2;				//for draw font frame
	capture_resolution_str.w += 2;		//for draw font frame
	capture_resolution_str.h += 2;		//for draw font frame
	ap_state_handling_ASCII_str_draw(&capture_resolution_str,&ascii_str);
}


INT32U cal_left_capture_num(void)
{
	INT64U freespace, left_num, temp_pic_size;
	if (ap_state_handling_storage_id_get() == NO_STORAGE) {
		#if (defined RAMDISK_EN) && (RAMDISK_EN == 1)
		left_num = 1;
		#else
		left_num = 0;
		#endif
	} else {
		ap_video_preview_left_capture_num_thinking();
		freespace = vfsFreeSpace(MINI_DVR_STORAGE_TYPE);

		if(freespace > (CARD_FULL_MB_SIZE<<20)) freespace -= (CARD_FULL_MB_SIZE<<20);
		else freespace = 0;

		if(freespace == 0) {
			left_num = 0;
		} else {
			if(last_pic_size == 0)  temp_pic_size = pic_size[ap_state_config_quality_get()][ap_state_config_pic_size_get()];
			else temp_pic_size = last_pic_size;

			left_num = (freespace>>10)/temp_pic_size;
			if(left_num > 99999) left_num = 99999;
		}
	}
	return left_num;
}


void left_capture_num_display(INT32U left_num)
{
	STRING_ASCII_INFO ascii_str;
	char digit[6] = {0};

	left_capture_num_str_clear();


	digit[0] = (left_num /10000) + 0x30;
	left_num %= 10000;

	digit[1] = (left_num /1000) + 0x30;
	left_num %= 1000;
	
	digit[2] = (left_num /100) + 0x30;
	left_num %= 100;

	digit[3] = (left_num /10) + 0x30;
	left_num %= 10;

	digit[4] = left_num + 0x30;

	ascii_str.font_color = 0xffff;
	ascii_str.font_type = 0;
	ascii_str.pos_x = 0;
	ascii_str.pos_y = 0;

	if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
		left_capture_num_str.pos_y = 212;
	} else { //TV
		left_capture_num_str.pos_y = 430;
	}
	ascii_str.buff_h = left_capture_num_str.h = ASCII_draw_char_height;

	ascii_str.str_ptr = digit;
	ascii_str.buff_w = left_capture_num_str.w = ASCII_draw_char_width*5;

	if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
		left_capture_num_str.pos_x = 10;//TFT_WIDTH-ICON_CAPTURE_OFFSET-ASCII_draw_char_width*5;
	} else { //TV
	  #if TV_WIDTH == 720
		left_capture_num_str.pos_x = 20;//TV_WIDTH-45-ASCII_draw_char_width*5;
	  #elif TV_WIDTH == 640
		left_capture_num_str.pos_x = 20;//TV_WIDTH-15-ASCII_draw_char_width*5;
	  #endif
	}

	ascii_str.buff_w += 2;				//for draw font frame
	ascii_str.buff_h += 2;				//for draw font frame
	left_capture_num_str.w += 2;		//for draw font frame
	left_capture_num_str.h += 2;		//for draw font frame
	ap_state_handling_ASCII_str_draw(&left_capture_num_str,&ascii_str);
}

void  video_capture_resolution_clear(void)
{
	ap_state_handling_ASCII_str_draw_exit(&capture_resolution_str,1);
}

void  left_capture_num_str_clear(void)
{
	ap_state_handling_ASCII_str_draw_exit(&left_capture_num_str,1);
}



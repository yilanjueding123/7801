#include "application.h"
#include "gplib.h"
#include "ap_setting.h"
#include "ap_state_config.h"
#include "ap_state_handling.h"
#include "avi_encoder_app.h"
#include "ap_state_resource.h"
#include "ap_music.h"
#include "audio_encoder.h"
#include "ap_storage_service.h"
#include "ap_display.h"
#include "stdio.h"
#include "string.h"
#include "ap_peripheral_handling.h"
#include "LDWs.h"
#include "socket_cmd.h"

// For state setting
//static INT32U sub_menu_2;
//static INT32U sub_menu_3;
//static INT32U sub_menu_4;
//static INT32U selectbar_middle;
//static INT32U background_c;
//static INT32U select_bar_bb;
//static INT32U background_b;
//static INT32U selectbar_long;
//static INT32U background;

#define MENU_ITEM_PRINTF_DEBUG_ENABLE		0

static SETTINGFUNC setting_category_set_fptr[4];
static SETUP_ITEM_INFO setting_item;
static INT32U setting_frame_buff;
static INT32U setting_browse_frame_buff = 0;
static INT16U state_str;
static INT8U setup_date_time[7];
static INT8U setup_date_time_counter = 0;
static INT8U g_setting_delete_choice = 0;
static INT8U g_setting_lock_choice = 0;
static INT8U g_setting_LDW_choice = 0;
static INT8U g_setting_LDW_value = 0;
static INT8U g_setting_LDW_flag = 0;
static INT16U g_manu_stage = 0;

extern char wifi_ssid[];
extern char wifi_password[];
char wifi_menu_set[]="00000000";

INT8U MODE_KEY_flag=0;

//	prototypes
void ap_setting_page_draw(INT32U state,INT32U state1, INT8U *tag);
void ap_setting_sub_menu_draw(INT8U curr_tag);
void ap_setting_background_draw(INT32U state,INT32U state1,STRING_INFO *str, INT8U flag);
void ap_setting_page_page_number(INT8U *tag,INT8U flag);
void ap_setting_video_record_set(void);
void ap_setting_photo_capture_set(void);
void ap_setting_browse_set(void);
void ap_setting_basic_set(void);
void ap_setting_general_state_draw(INT32U state,STRING_INFO *str, INT8U *tag);
INT8U ap_setting_right_menu_active(STRING_INFO *str, INT8U type, INT8U *sub_tag);
void ap_setting_no_sdc_show(void);
void ap_setting_busy_show(void);
void ap_setting_icon_draw(INT16U *frame_buff, INT16U *icon_stream, DISPLAY_ICONSHOW *icon_info, INT8U type);
void ap_setting_date_time_string_process(char *dt_str1,INT8U dt_tag);
void ap_setting_frame_buff_display(void);
INT8U CalendarCalculateDays(INT8U month, INT8U year);
void ap_setting_value_set_from_user_config(void);
void ap_setting_sensor_sccb_cmd_set(INT8U *table_ptr, INT16U idx, INT8U cmd_num);
void ap_setting_sensor_command_switch(INT16U cmd_addr, INT16U reg_bit, INT8U enable);
void ap_setting_show_software_version(void);
extern void gp_cdsp_set_exp_freq(INT32U freq_num);
#if WIFI_SSID_ADD_MACADDR == 1
extern void char_transfer(char *dst,char *src,int len);
extern char mac_addr[];
#endif

void ap_setting_init(INT32U prev_state,INT32U prev_state1, INT8U *tag)
{		
	IMAGE_DECODE_STRUCT img_info;
	INT32U background_image_ptr;
	INT32U size;
	INT16U logo_fd;
	INT8S* dest;
	INT8S* src;
	INT32U Temp;
	MODE_KEY_flag = 1;
	prev_state = prev_state&0x1ff;

	// modify by josephhsieh@140512 (consult from tristanyang)
	setting_frame_buff = (INT32U) gp_malloc_align(TFT_WIDTH * TFT_HEIGHT * 2, 64);

	if (!setting_frame_buff) {
		DBG_PRINT("State setting allocate frame buffer fail.\r\n");
	}

	if (prev_state == STATE_BROWSE) {
		INT32U search_type = STOR_SERV_SEARCH_ORIGIN;

		setting_browse_frame_buff = (INT32U) gp_malloc_align(getDispDevBufSize(), 64);
		if (!setting_browse_frame_buff) {
			DBG_PRINT("State setting allocate browse frame buffer fail.\r\n");
		} else if (ap_state_handling_storage_id_get() == NO_STORAGE) {
			STOR_SERV_PLAYINFO dummy_info = {0};
			dummy_info.err_flag = STOR_SERV_NO_MEDIA;
			dummy_info.file_handle = -1;
//			msgQSend(ApQ, MSG_STORAGE_SERVICE_BROWSE_REPLY, NULL, NULL, MSG_PRI_NORMAL); //to give a null pointer is incorrect, wwj commnet
			msgQSend(ApQ, MSG_STORAGE_SERVICE_BROWSE_REPLY, &dummy_info, sizeof(STOR_SERV_PLAYINFO), MSG_PRI_NORMAL); //wwj modify
		} else {
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_BROWSE_REQ, (void *) &search_type, sizeof(INT32U), MSG_PRI_NORMAL);
		}
	} else if(prev_state == STATE_AUDIO_RECORD) {
		setting_browse_frame_buff = (INT32U) gp_malloc_align(TFT_WIDTH * TFT_HEIGHT * 2, 64);
		if (!setting_browse_frame_buff) {
			DBG_PRINT("State setting allocate browse frame buffer fail.\r\n");
		}
		logo_fd = nv_open((INT8U *) "AUDIO_REC_BG.JPG");
		if (logo_fd != 0xFFFF) {
			size = nv_rs_size_get(logo_fd);
			background_image_ptr = (INT32U) gp_malloc(size);
			if (!background_image_ptr) {
				DBG_PRINT("State audio record allocate jpeg input buffer fail.[%d]\r\n", size);
			} else {
				if (nv_read(logo_fd, background_image_ptr, size)) {
					DBG_PRINT("Failed to read resource_header in ap_audio_record_init\r\n");
				}
				img_info.image_source = (INT32S) background_image_ptr;
				img_info.source_size = size;
				img_info.source_type = TK_IMAGE_SOURCE_TYPE_BUFFER;
				img_info.output_format = C_SCALER_CTRL_OUT_RGB565;
				img_info.output_ratio = 0;
				img_info.out_of_boundary_color = 0x008080;
				img_info.output_buffer_width = TFT_WIDTH;
				img_info.output_buffer_height = TFT_HEIGHT;
				img_info.output_image_width = TFT_WIDTH;
				img_info.output_image_height = TFT_HEIGHT;
				img_info.output_buffer_pointer = setting_browse_frame_buff;
				if (jpeg_buffer_decode_and_scale(&img_info) == STATUS_FAIL) {
					DBG_PRINT("State audio record decode jpeg file fail.\r\n");
				}
				gp_free((void *) background_image_ptr);
			}
		} else {
			gp_memset((void *) setting_browse_frame_buff, 80, TFT_WIDTH * TFT_HEIGHT * 2);
		}
	} else if((prev_state == STATE_VIDEO_RECORD) || (prev_state == STATE_VIDEO_PREVIEW)) {
		if(ap_display_get_device()==DISP_DEV_TFT) {
			setting_browse_frame_buff = (INT32U) gp_malloc_align(TFT_WIDTH * TFT_HEIGHT * 2, 64);
			dest = (INT8S*)setting_browse_frame_buff;
		#if USE_PANEL_NAME == PANEL_400X240_I80
			  #if DISPLAY_BUF_NUM == 1
				Temp = (INT32U) display_frame[0];
			  #endif
		#else
			Temp = R_TFT_FBI_ADDR;
		#endif
			src = (INT8S*)Temp;
			gp_memcpy(dest, src, TFT_WIDTH * TFT_HEIGHT * 2);
		} else {
			setting_browse_frame_buff = (INT32U) gp_malloc_align(TV_WIDTH * TV_HEIGHT * 2, 64);
			dest = (INT8S*)setting_browse_frame_buff;
			Temp = R_TV_FBI_ADDR;
			src = (INT8S*)Temp;
			gp_memcpy(dest, src, TV_WIDTH * TV_HEIGHT * 2);
		}
	}
	setting_category_set_fptr[0] = (SETTINGFUNC) ap_setting_video_record_set;
	setting_category_set_fptr[1] = (SETTINGFUNC) ap_setting_photo_capture_set;
	setting_category_set_fptr[2] = (SETTINGFUNC) ap_setting_browse_set;
	setting_category_set_fptr[3] = (SETTINGFUNC) ap_setting_basic_set;
	if (prev_state != STATE_BROWSE) {
		ap_setting_page_draw(prev_state,prev_state1, tag);
	}
	setup_date_time_counter = 0;
	g_setting_LDW_choice = LDW_ON_OFF;
	g_setting_LDW_value = ap_state_config_LDW_get(LDW_ON_OFF);
}

void ap_setting_exit(INT32U prev_state)
{
	OS_Q_DATA OSQData;

	OSQPost(DisplayTaskQ, (void *) MSG_DISPLAY_TASK_SETTING_EXIT);
	OSQQuery(DisplayTaskQ, &OSQData);
	while(OSQData.OSMsg != NULL) {
		OSTimeDly(3);
		OSQQuery(DisplayTaskQ, &OSQData);
	}

	if (setting_browse_frame_buff) {
		gp_free((void *) setting_browse_frame_buff);
		setting_browse_frame_buff = 0;
	}
	if (setting_frame_buff) {
		gp_free((void *) setting_frame_buff);
		setting_frame_buff = 0;
	}

	setup_date_time_counter = 0;
	g_setting_delete_choice = 0;
	g_setting_lock_choice = 0;
	MODE_KEY_flag = 2;

	DBG_PRINT("setting exit\r\n");
}


INT32U ap_setting_show_GPZP_file(INT8U *path, INT16U *frame_buff, DISPLAY_ICONSHOW *icon_info, INT8U type)
{
	INT32U size, read_buf;
	INT16U logo_fd;
	INT8U *zip_buf;
	INT32U icon_buffer;
	
	logo_fd = nv_open(path);
	if (logo_fd != 0xFFFF) {
		size = nv_rs_size_get(logo_fd);
		read_buf = (INT32S) gp_malloc(size);
		if (!read_buf) {
			DBG_PRINT("allocate BACKGROUND.GPZP read buffer fail.[%d]\r\n", size);
			return STATUS_FAIL;
		}

		if (nv_read(logo_fd, (INT32U) read_buf, size)) {
			DBG_PRINT("Failed to read icon_buffer resource\r\n");
			gp_free((void *) read_buf);
			read_buf = 0;
			return STATUS_FAIL;
		}

		icon_buffer = (INT32S) gp_malloc(icon_info->icon_w*icon_info->icon_h*2);
		if (!icon_buffer) {
			gp_free((void *) read_buf);
			gp_free((void *) icon_buffer);
			read_buf = 0;
			icon_buffer = 0;
			DBG_PRINT("setting allocate icon_buffer buffer fail.[%d]\r\n", size);
			return STATUS_FAIL;
		}

		zip_buf = (INT8U *)read_buf;
		if (gpzp_decode((INT8U *) &zip_buf[4], (INT8U *) icon_buffer) == STATUS_FAIL) {
			DBG_PRINT("Failed to unzip GPZP file\r\n");
			gp_free((void *) read_buf);
			gp_free((void *) icon_buffer);
			read_buf = 0;
			icon_buffer = 0;
			return STATUS_FAIL;
		}

		gp_free((void *) read_buf);
		read_buf = 0;
		ap_setting_icon_draw(frame_buff, (INT16U *)icon_buffer, icon_info, type);
		gp_free((void *) icon_buffer);
		icon_buffer = 0;
		return STATUS_OK;
	}else{
		return STATUS_FAIL;
	}
}

void ap_setting_reply_action(INT32U state,INT32U state1, INT8U *tag, STOR_SERV_PLAYINFO *info_ptr)
{
	INT32U i, *buff_ptr, color_data, cnt;
	INT32S ret, logo_img_ptr;
	IMAGE_DECODE_STRUCT img_info;
	INT32U size;
	INT16U logo_fd;

	if ((info_ptr->err_flag == STOR_SERV_OPEN_OK) && (ap_state_handling_storage_id_get() != NO_STORAGE)) {
		if (info_ptr->file_type == TK_IMAGE_TYPE_WAV) {
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
					DBG_PRINT("Failed to read resource_header in ap_startup_init\r\n");
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
				img_info.output_buffer_pointer = setting_browse_frame_buff;
				if (jpeg_buffer_decode_and_scale(&img_info) == STATUS_FAIL) {
					gp_free((void *) logo_img_ptr);
					DBG_PRINT("State browser decode AUDIO_REC_BG.jpeg file fail.\r\n");
					return;
				}
				gp_free((void *) logo_img_ptr);
				close(info_ptr->file_handle);
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
			} else {
				DBG_PRINT("Failed to nv_open browser decode AUDIO_REC_BG.jpg!\r\n");
			}
		}
		else	//for AVI and JPG files
		{
			ret = ap_state_handling_jpeg_decode(info_ptr, setting_browse_frame_buff);
			if (ret != STATUS_OK) {
				buff_ptr = (INT32U *) setting_browse_frame_buff;
				color_data = 0x11a4 | (0x11a4<<16);
				cnt = getDispDevBufSize() >> 2;
				for (i=0 ; i<cnt ; i++) {
					*buff_ptr++ = color_data;
				}
			}
			close(info_ptr->file_handle);
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
		}
	} else {
		buff_ptr = (INT32U *) setting_browse_frame_buff;
		color_data = 0x11a4 | (0x11a4<<16);
		cnt = getDispDevBufSize() >> 2;
		for (i=0 ; i<cnt ; i++) {
			*buff_ptr++ = color_data;
		}
//		ap_state_handling_str_draw_exit();
//		ap_state_handling_str_draw(STR_NO_MEDIA, WARNING_STR_COLOR);
		if (info_ptr->err_flag == STOR_SERV_DECODE_ALL_FAIL) {
			close(info_ptr->file_handle);
		}
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
	}
	ap_setting_page_draw(state,state1, tag);
}

void ap_setting_page_draw(INT32U state,INT32U state1, INT8U *tag)
{
	DISPLAY_ICONSHOW icon = {320, 240, TRANSPARENT_COLOR, 0, 0};
	STRING_INFO str = {0};
	INT32U x,offset_data, *ptr;
	
	offset_data = (TFT_WIDTH*TFT_HEIGHT) >> 1;
	ptr = (INT32U *) setting_frame_buff;
	for (x=0 ; x<offset_data ; x++) {
		*(ptr++) = 0x8C718C71;
	}
	ap_setting_show_GPZP_file((INT8U*)"BACKGROUND.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);

	if(MODE_KEY_flag == 0){

		icon.icon_w = ICON_SELECT_BAR_WIDTH;
		icon.icon_h = ICON_SELECT_BAR_HEIGHT;
		icon.pos_x = 0;//8+5;
		icon.pos_y = ICON_SELECT_BAR_START_Y + (*tag - ((*tag/6)*6))*29;

		ap_setting_show_GPZP_file((INT8U*)"SELECTBAR_LONG.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);

	}
	str.font_type = 0;	
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;	
	str.font_color = 0xFFFF;
	if(state == STATE_AUDIO_RECORD) {
		setting_category_set_fptr[STATE_VIDEO_PREVIEW & 0xF]();
	} else {
		setting_category_set_fptr[state & 0xF]();
	}
	ap_setting_background_draw(state,state1,&str, 1);
	ap_setting_page_page_number(tag,1);
	ap_setting_general_state_draw(state,&str, tag);
	ap_setting_frame_buff_display();
}
void ap_setting_page_draw_no_display(INT32U state,INT32U state1, INT8U *tag,INT8U flag)
{
	DISPLAY_ICONSHOW icon = {320, 240, TRANSPARENT_COLOR, 0, 0};
	STRING_INFO str = {0};
	INT32U x,offset_data, *ptr;

	offset_data = (TFT_WIDTH*TFT_HEIGHT) >> 1;
	ptr = (INT32U *) setting_frame_buff;
	for (x=0 ; x<offset_data ; x++) {
		*(ptr++) = 0x8C718C71;
	}
	ap_setting_show_GPZP_file((INT8U*)"BACKGROUND.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);

	if(MODE_KEY_flag == 0){

		icon.icon_w = ICON_SELECT_BAR_WIDTH;
		icon.icon_h = ICON_SELECT_BAR_HEIGHT;
		icon.pos_x = 0;//8+5;
		icon.pos_y = ICON_SELECT_BAR_START_Y + (*tag - ((*tag/6)*6))*29;

		ap_setting_show_GPZP_file((INT8U*)"SELECTBAR_LONG.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);

	}
	str.font_type = 0;	
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;	
	str.font_color = 0xFFFF;
	if(state == STATE_AUDIO_RECORD) {
		setting_category_set_fptr[STATE_VIDEO_PREVIEW & 0xF]();
	} else {
		setting_category_set_fptr[state & 0xF]();
	}
	ap_setting_background_draw(state,state1,&str, 1);
	ap_setting_page_page_number(tag,flag);
	ap_setting_general_state_draw(state,&str, tag);
}


void ap_USB_setting_page_draw(INT32U state,INT32U state1, INT8U *tag, INT32U buff_addr)
{
	DISPLAY_ICONSHOW icon = {320, 240, TRANSPARENT_COLOR, 0, 0};
	STRING_INFO str = {0};
	INT32U i;
	t_STRING_TABLE_STRUCT str_res;
	
	ap_setting_show_GPZP_file((INT8U*)"USBBACKGROUND.GPZP", (INT16U *)buff_addr, &icon, SETTING_ICON_NORMAL_DRAW);

	str.font_type = 0;	
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;	
	str.font_color = 0xFFFF;
	str.str_idx = STR_USB;
	ap_state_resource_string_resolution_get(&str, &str_res);
	str.pos_x = (320 - str_res.string_width)-10;
	str.pos_y = 11;
	ap_state_resource_string_draw((INT16U *)buff_addr, &str, RGB565_DRAW);
	
	/*icon.icon_w = ICON_TOP_BAR_WIDTH;
	icon.icon_h = ICON_TOP_BAR_HEIGHT;
	icon.pos_x = ICON_TOP_BAR_START_X;
	icon.pos_y = ICON_TOP_BAR_START_Y;
	ap_setting_show_GPZP_file((INT8U*)"TOPBAR.GPZP",(INT16U *)buff_addr,&icon,SETTING_ICON_NORMAL_DRAW);*/

	icon.icon_w = 42;
	icon.icon_h = 28;
	icon.pos_x = ICON_TOP_BAR_START_X+((ICON_TOP_BAR_WIDTH-icon.icon_w)>>1);
	icon.pos_y = 8;
	ap_setting_icon_draw((INT16U *)buff_addr, icon_usb, &icon, SETTING_ICON_BLUE1_COLOR);

	icon.icon_w = ICON_BACKGROUND_LEFT_WIDTH;
	icon.icon_h = ICON_BACKGROUND_LEFT_HEIGHT;
	icon.pos_y = ICON_BACKGROUND_LEFT_START_Y;
	icon.pos_x = ICON_BACKGROUND_LEFT_START_X+20;
	ap_setting_icon_draw((INT16U *)buff_addr, icon_background_up, &icon, SETTING_ICON_NORMAL_DRAW);
	icon.pos_x = ICON_BACKGROUND_LEFT_START_X+40;
	ap_setting_icon_draw((INT16U *)buff_addr, icon_background_down, &icon, SETTING_ICON_NORMAL_DRAW);

	icon.pos_x = ICON_BACKGROUND_LEFT_START_X+60+32;
	ap_setting_icon_draw((INT16U *)buff_addr, icon_background_ok, &icon, SETTING_ICON_NORMAL_DRAW);


	icon.icon_w = ICON_SELECT_BAR_WIDTH;
	icon.icon_h = ICON_SELECT_BAR_HEIGHT;
	icon.pos_x = 8+5;
	icon.pos_y = ICON_SELECT_BAR_START_Y + (*tag - ((*tag/4)*4))*40+3;
	ap_setting_show_GPZP_file((INT8U*)"SELECTBAR_LONG.GPZP",(INT16U *)buff_addr, &icon, SETTING_ICON_NORMAL_DRAW);


	setting_item.item_start = STR_MASS_STORAGE;
	setting_item.item_max = 2;
	state_str = STR_USB;
	{
		str.pos_x = ICON_SETTING_HEAD_START_X+ICON_SETTING_HEAD_WIDTH+5;
		str.pos_y = ICON_SETTING_HEAD_START_Y+7;
		str.str_idx = ((*tag/4)*4) + setting_item.item_start;

		for (i=0; i<2; i++) {
			if(str.str_idx == setting_item.item_start+*tag){
				str.font_color = BLUE_COLOR;
			}else{
				str.font_color = 0xffff;
			}
			ap_state_resource_string_draw((INT16U *)buff_addr, &str, RGB565_DRAW);
			str.str_idx++;
			str.pos_x = ICON_SETTING_HEAD_START_X+ICON_SETTING_HEAD_WIDTH+5;
			str.pos_y += 40;
		}
	}
	OSQPost(DisplayTaskQ, (void *) (buff_addr|MSG_DISPLAY_TASK_USB_SETTING_DRAW));
}

void ap_wifi_signal_setting_page_draw(INT32U buff_addr)
{
	DISPLAY_ICONSHOW icon = {320, 240, TRANSPARENT_COLOR, 0, 0};
	
	ap_setting_show_GPZP_file((INT8U*)"WIFISIGNAL.GPZP", (INT16U *)buff_addr, &icon, SETTING_ICON_NORMAL_DRAW);

	OSQPost(DisplayTaskQ, (void *) (buff_addr|MSG_DISPLAY_TASK_USB_SETTING_DRAW));
}

void ap_wifi_display_setting_page_draw(INT32U buff_addr)
{
	DISPLAY_ICONSHOW icon = {320, 240, TRANSPARENT_COLOR, 0, 0};
	STRING_ASCII_INFO ascii_str;
	INT16U String_width; 	
	char dt_str[] = "SSID:";
	#if WIFI_ENCRYPTION_METHOD == WPA2
	char dt_str1[] = "WPA2:";
	#else
	char dt_str1[] = "WEP:";
	#endif
	char dt_str2[] = "Press Up";
	char dt_str3[] = "to return";
	#if WIFI_SSID_ADD_MACADDR == 1
	char temp_ssid[GPSOCK_WiFi_Name_Length+12+1];
	INT8U ssid_length;
	#endif
	
	ap_setting_show_GPZP_file((INT8U*)"WIFIBACKGROUND.GPZP", (INT16U *)buff_addr, &icon, SETTING_ICON_NORMAL_DRAW);

	String_width = ASCII_draw_char_width;
	ascii_str.font_color = 0xFFFF;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ascii_str.pos_x = 0;
	ascii_str.pos_y = 60;
	ascii_str.str_ptr = dt_str;
	ap_state_resource_string_ascii_draw((INT16U *)buff_addr, &ascii_str, RGB565_DRAW);
	
	#if WIFI_SSID_ADD_MACADDR == 1
	for(ssid_length=0;ssid_length<GPSOCK_WiFi_Name_Length;ssid_length++)
	{
		if(wifi_ssid[ssid_length]==0)
		{
			break;
		}
	}
	gp_memcpy((INT8S*)temp_ssid,(INT8S*)wifi_ssid, ssid_length);
	char_transfer(&temp_ssid[ssid_length],mac_addr,6);
	temp_ssid[ssid_length+12]=0x00;
	#endif
	String_width = ASCII_draw_char_width*5;
	ascii_str.font_color = 0xFFFF;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ascii_str.pos_x = String_width;
	ascii_str.pos_y = 60;
	#if WIFI_SSID_ADD_MACADDR == 1
	ascii_str.str_ptr = temp_ssid;
	#else
	ascii_str.str_ptr = wifi_ssid;
	#endif
	ap_state_resource_string_ascii_draw((INT16U *)buff_addr, &ascii_str, RGB565_DRAW);
	
	ascii_str.font_color = 0xFFFF;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ascii_str.pos_x = 0;
	ascii_str.pos_y = 90;
	ascii_str.str_ptr = dt_str1;
	ap_state_resource_string_ascii_draw((INT16U *)buff_addr, &ascii_str, RGB565_DRAW);
	
	ascii_str.font_color = 0xFFFF;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ascii_str.pos_x = String_width;
	ascii_str.pos_y = 90;
	ascii_str.str_ptr = wifi_password;
	ap_state_resource_string_ascii_draw((INT16U *)buff_addr, &ascii_str, RGB565_DRAW);

	icon.icon_w = 28;
	icon.icon_h = 28;
	icon.pos_x = 0;
	icon.pos_y = 0;
	ap_setting_icon_draw((INT16U *)buff_addr, icon_wifi_signal, &icon, SETTING_ICON_BLUE1_COLOR);

	ascii_str.font_color = RED_COLOR;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ascii_str.pos_x = 60;
	ascii_str.pos_y = 150;
	ascii_str.str_ptr = dt_str2;
	ap_state_resource_string_ascii_draw((INT16U *)buff_addr, &ascii_str, RGB565_DRAW);
	
	ascii_str.font_color = RED_COLOR;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ascii_str.pos_x = 60;
	ascii_str.pos_y = 150+ASCII_draw_char_height;
	ascii_str.str_ptr = dt_str3;
	ap_state_resource_string_ascii_draw((INT16U *)buff_addr, &ascii_str, RGB565_DRAW);

	OSQPost(DisplayTaskQ, (void *) (buff_addr|MSG_DISPLAY_TASK_USB_SETTING_DRAW));
}

void ap_USB_setting_display_ok(void)
{
	INT32U i, display_frame0;
	STRING_INFO str = {0};

	t_STRING_TABLE_STRUCT str_res;

	for (i=0;i<50;++i){
		display_frame0 = ap_display_queue_get(display_isr_queue);
		if (display_frame0!=NULL)
			break;
		OSTimeDly(1);
	}
	for(i=0; i<TFT_WIDTH*TFT_HEIGHT; i++) {
		*(INT16U *)(display_frame0 + i*2) = BLUE_COLOR;
	}


	str.font_type = 0;	
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;	
	str.font_color = 0xFFFF;

	if(ap_state_config_usb_mode_get() == 0){
		str.str_idx = STR_MASS_STORAGE;
	}else{
		str.str_idx = STR_PC_CAMERA;
	}
	ap_state_resource_string_resolution_get(&str, &str_res);
	str.pos_x = (TFT_WIDTH - str_res.string_width)/2;
	str.pos_y = (TFT_HEIGHT-str_res.string_height)/2;
	ap_state_resource_string_draw((INT16U *)display_frame0, &str, RGB565_DRAW);

    OSQPost(DisplayTaskQ, (void *) (display_frame0|MSG_DISPLAY_TASK_USB_SETTING_DRAW));
}

void ap_setting_sub_menu_draw(INT8U curr_tag)
{
	STRING_INFO sub_str = {0};
	DISPLAY_ICONSHOW icon;
	INT16U draw_cnt, i;
	t_STRING_TABLE_STRUCT str_res;

	icon.transparent = TRANSPARENT_COLOR;
	icon.icon_w = ICON_SELECTBOX_SUB_WIDTH;
	icon.icon_h = ICON_SELECTBOX_SUB_HEIGHT;
	icon.pos_x = ICON_SELECTBOX_SUB_START_X;
	icon.pos_y = ICON_SELECTBOX_SUB_START_Y;

	ap_setting_show_GPZP_file((INT8U*)"SELECTBOX_SUB.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);

	sub_str.font_type = 0;	
	sub_str.buff_w = TFT_WIDTH;
	sub_str.buff_h = TFT_HEIGHT;
	sub_str.language = ap_state_config_language_get() - 1;	
	sub_str.font_color = 0xFFFF;
	sub_str.str_idx = g_manu_stage;
	ap_state_resource_string_resolution_get(&sub_str, &str_res);
	sub_str.pos_x = TFT_WIDTH-str_res.string_width-10;//(320 - str_res.string_width)-10;
	sub_str.pos_y = 7;
	ap_state_resource_string_draw((INT16U *)setting_frame_buff, &sub_str, RGB565_DRAW);

	icon.icon_w = ICON_SELECTBAR_MIDD_WIDTH;
	icon.icon_h = ICON_SELECTBAR_MIDD_HEIGHT;
	icon.pos_x = ICON_SELECTBAR_MIDD_START_X;	
	if((setting_item.stage == STR_DEFAULT_SETTING)||(setting_item.stage == STR_FORMAT1)){
		icon.pos_y = ICON_SELECTBOX_SUB_START_Y+(ICON_SELECTBOX_SUB_HEIGHT - ((ICON_SELECTBOX_SUB_HEIGHT/4)*2));
	}else{
		icon.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT/4-ICON_SELECTBAR_MIDD_HEIGHT)>>1);
	}
	i = curr_tag & 0x3;
	icon.pos_y = icon.pos_y+i*(ICON_SELECTBOX_SUB_HEIGHT/4);
	ap_setting_show_GPZP_file((INT8U*)"SELECTBAR_MIDD.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);

	sub_str.font_type = 0;	
	sub_str.buff_w = TFT_WIDTH;
	sub_str.buff_h = TFT_HEIGHT;
	sub_str.font_color = 0xFFFF;
	if (setting_item.sub_item_start != STR_MULTI_LANGUAGE) {
		sub_str.language = ap_state_config_language_get() - 1;
		sub_str.str_idx = ((curr_tag>>2) << 2) + setting_item.sub_item_start;
		draw_cnt = setting_item.sub_item_max - (sub_str.str_idx - setting_item.sub_item_start);
	} else {
		sub_str.str_idx = STR_MULTI_LANGUAGE;
		sub_str.language = ((curr_tag>>2) << 2) + LCD_EN;
		draw_cnt = setting_item.sub_item_max - (sub_str.language - setting_item.sub_item_start);
	}	
	if (draw_cnt > 4) {
		draw_cnt = 4;
	}

	ap_state_resource_string_resolution_get(&sub_str, &str_res);
	sub_str.pos_x = ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-str_res.string_width)>>1);	
	if((setting_item.stage == STR_DEFAULT_SETTING)||(setting_item.stage == STR_FORMAT1)){
		sub_str.pos_y = ICON_SELECTBOX_SUB_START_Y+(ICON_SELECTBOX_SUB_HEIGHT - ((ICON_SELECTBOX_SUB_HEIGHT/4)*2))+((ICON_SELECTBOX_SUB_HEIGHT/4-str_res.string_height)>>1);
	}else{
		sub_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT/4-str_res.string_height)>>1);
	}
	for (i=0; i<draw_cnt; i++) {
		if((curr_tag & 0x3) == i)
		{
			sub_str.font_color = BLUE_COLOR;
		}else{
			sub_str.font_color = 0xffff;
		}
		ap_state_resource_string_draw((INT16U *)setting_frame_buff, &sub_str, RGB565_DRAW);
		if (setting_item.sub_item_start != STR_MULTI_LANGUAGE) {
			sub_str.str_idx++;
		} else {
			sub_str.language++;
		}
		ap_state_resource_string_resolution_get(&sub_str, &str_res);
		sub_str.pos_x = ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-str_res.string_width)>>1);
		sub_str.pos_y += ICON_SELECTBOX_SUB_HEIGHT/4;
	}
	if((setting_item.stage == STR_DEFAULT_SETTING)||(setting_item.stage == STR_FORMAT1)){
		sub_str.font_color = 0xFFFF;
		if(setting_item.stage == STR_DEFAULT_SETTING){
			sub_str.str_idx = STR_DEFAULT_SETTING_SHOW_1;
		} else if(setting_item.stage == STR_FORMAT1){
			sub_str.str_idx = STR_FORMAT_SHOW_1;
		}
		ap_state_resource_string_resolution_get(&sub_str, &str_res);
		sub_str.pos_x = ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-str_res.string_width)>>1);	
		sub_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT/4-str_res.string_height)>>1);
		for (i=0; i<2; i++) {
			ap_state_resource_string_draw((INT16U *)setting_frame_buff, &sub_str, RGB565_DRAW);
			if (setting_item.sub_item_start != STR_MULTI_LANGUAGE) {
				sub_str.str_idx++;
			} else {
				sub_str.language++;
			}
			ap_state_resource_string_resolution_get(&sub_str, &str_res);
			sub_str.pos_x = ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-str_res.string_width)>>1);
			sub_str.pos_y += str_res.string_height;
		}
	}
	
	ap_setting_frame_buff_display();
}

#define icon_date_time_offset		3
void ap_setting_date_time_menu_draw(INT8U tag)
{
	STRING_INFO str = {0};
	STRING_ASCII_INFO ascii_str;
	DISPLAY_ICONSHOW icon;
	DISPLAY_ICONSHOW cursor_icon = {320, 234, TRANSPARENT_COLOR, 0, 0};
//	INT32U size, read_buf;
//	INT16U logo_fd;
//	INT8U *zip_buf;
	INT32U i;
	char Data_str[]= "2000 / 00 / 00";
	char Time_str[]= "00 : 00 : 00";
	char Data_Time_Mode1[] = "YY";
	char Data_Time_Mode2[] = "/MM/DD";
	char select_str[] = "0000";
	char select_str1[] = "00";
	INT16U String_width; 	
	INT16U String_width1; 	
	INT16U String_height;
	INT16U String_pos_x;
//	INT16U String_pos_y;
	INT16U Icon_pos_x;	
	INT16U Icon_pos_y;
	char dt_str[] = "2000 / 00 / 00   00 : 00 : 00";
	t_STRING_TABLE_STRUCT str_res;
	
	icon.transparent = TRANSPARENT_COLOR;
	icon.icon_w = ICON_SELECTBOX_SUB_WIDTH;
	icon.icon_h = ICON_SELECTBOX_SUB_HEIGHT;
	icon.pos_x = ICON_SELECTBOX_SUB_START_X;	
	icon.pos_y = ICON_SELECTBOX_SUB_START_Y;

	ap_setting_show_GPZP_file((INT8U*)"SELECTBOX_SUB.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);
	

	str.font_type = 0;	
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;	
	str.font_color = 0xFFFF;
	str.str_idx = g_manu_stage;
	ap_state_resource_string_resolution_get(&str, &str_res);
	str.pos_x = TFT_WIDTH-str_res.string_width-10;//(320 - str_res.string_width)-10;
	str.pos_y = 7;
	ap_state_resource_string_draw((INT16U *)setting_frame_buff, &str, RGB565_DRAW);

	String_height = ASCII_draw_char_height;


	str.font_type = 0;	
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;	
	str.font_color = 0xFFFF;
	ap_setting_date_time_string_process(dt_str,SETTING_DATE_TIME_DRAW_ALL);

	for(i=0;i<14;i++)
	{
		Data_str[i] = *(dt_str+i);
	}

	for(i=0;i<12;i++)
	{
		Time_str[i] = *(dt_str+17+i);
	}

	if(setup_date_time[6] == 0){	//yy/mm/dd
		strncpy(Data_Time_Mode1,"YY",strlen("YY"));
		strncpy(Data_Time_Mode2,"/MM/DD",strlen("/MM/DD"));
	}else if(setup_date_time[6] == 1){	//dd/mm/yy
		strncpy(Data_Time_Mode1,"DD",strlen("DD"));
		strncpy(Data_Time_Mode2,"/MM/YY",strlen("/MM/YY"));
	}else if(setup_date_time[6] == 2){	//mm/dd/yy
		strncpy(Data_Time_Mode1,"MM",strlen("MM"));
		strncpy(Data_Time_Mode2,"/DD/YY",strlen("/DD/YY"));
	}

	String_width = ASCII_draw_char_width*14;
	ascii_str.font_color = 0xFFFF;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ascii_str.pos_x = ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
	ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
	ascii_str.str_ptr = Data_str;
	ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);

	String_width = ASCII_draw_char_width*12;
	ascii_str.pos_x = ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
	ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*2+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);

	ascii_str.str_ptr = Time_str;
	ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);

	String_width = ASCII_draw_char_width*8;
	ascii_str.pos_x = ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
	ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT*2+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*3+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1)-icon_date_time_offset;

	String_width = ASCII_draw_char_width*2;
	ascii_str.str_ptr = Data_Time_Mode1;
	ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);
	ascii_str.pos_x = ascii_str.pos_x+ASCII_draw_char_width*2+3;
	ascii_str.str_ptr = Data_Time_Mode2;
	ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);




	ascii_str.font_color = BLUE_COLOR;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;


	icon.transparent = TRANSPARENT_COLOR;
	cursor_icon.icon_w = 32;//16;
	cursor_icon.icon_h = 16;

	if(setup_date_time[6] == 0)	//yy/mm/dd
	{
		if(tag ==0)
		{
			String_width = ASCII_draw_char_width*14;	//
			String_width1 = ASCII_draw_char_width*4;	//year
			String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
			Icon_pos_x	 =String_pos_x-((ICON_YEAR_SELECTBAR_WIDTH-String_width1)>>1);	
			Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4);

			icon.icon_w = ICON_YEAR_SELECTBAR_WIDTH;
			icon.icon_h = ICON_YEAR_SELECTBAR_HEIGHT;
			icon.pos_x =Icon_pos_x;	
			icon.pos_y =Icon_pos_y+icon_date_time_offset;
			ap_setting_show_GPZP_file((INT8U*)"YEAR_SELECTBAR.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);
			
			cursor_icon.pos_x = Icon_pos_x+((ICON_YEAR_SELECTBAR_WIDTH-32)>>1);//16
			cursor_icon.pos_y = Icon_pos_y+icon_date_time_offset-16;
			ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_up, &cursor_icon, SETTING_ICON_NORMAL_DRAW);
			cursor_icon.pos_y += (ICON_YEAR_SELECTBAR_HEIGHT+16);
			ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_down, &cursor_icon, SETTING_ICON_NORMAL_DRAW);
			
			for(i=0;i<4;i++){
				select_str[i] = *(dt_str+i);
			}
			ascii_str.pos_x = String_pos_x;
			ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
			ascii_str.str_ptr = select_str;
			ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);

		}else{
			
			icon.icon_w = ICON_DAY_SELECTBAR_WIDTH;
			icon.icon_h = ICON_DAY_SELECTBAR_HEIGHT;
			if(tag == 1){
				String_width = ASCII_draw_char_width*14;	
				String_width1 = ASCII_draw_char_width*2;	//day
				
				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1)+(ASCII_draw_char_width*7);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset;
				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+7+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);

			}else if(tag == 2){
				String_width = ASCII_draw_char_width*14;	
				String_width1 = ASCII_draw_char_width*2;	//day

				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1)+(ASCII_draw_char_width*12);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset;
				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+12+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
				
			}else if(tag == 3){
				String_width = ASCII_draw_char_width*12;	
				String_width1 = ASCII_draw_char_width*2;	//time
				
				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+(((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)*2)+ICON_DAY_SELECTBAR_HEIGHT;
				
				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+17+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*2+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
			}else if(tag == 4){
				String_width = ASCII_draw_char_width*12;	
				String_width1 = ASCII_draw_char_width*2;	//time

				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1)+(ASCII_draw_char_width*5);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+(((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)*2)+ICON_DAY_SELECTBAR_HEIGHT;

				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+17+5+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*2+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
			}else if(tag == 5){
				String_width = ASCII_draw_char_width*12;	
				String_width1 = ASCII_draw_char_width*2;	//time

				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1)+(ASCII_draw_char_width*10);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+(((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)*2)+ICON_DAY_SELECTBAR_HEIGHT;

				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+17+10+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*2+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
			}else if(tag == 6){
				String_width = ASCII_draw_char_width*8;	
				String_width1 = ASCII_draw_char_width*2;	//mode

				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+(((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)*3)+ICON_DAY_SELECTBAR_HEIGHT*2-3;

				for(i=0;i<2;i++)
				{
					select_str1[i] = *(Data_Time_Mode1+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT*2+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*3+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1)-icon_date_time_offset;
			}
			icon.pos_x =Icon_pos_x;	
			icon.pos_y =Icon_pos_y;
			ap_setting_show_GPZP_file((INT8U*)"DAY_SELECTBAR.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);
			
			cursor_icon.pos_x = Icon_pos_x+((ICON_DAY_SELECTBAR_WIDTH-32)>>1);//16
			cursor_icon.pos_y = Icon_pos_y-16;
			ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_up, &cursor_icon, SETTING_ICON_NORMAL_DRAW);
			cursor_icon.pos_y += (ICON_DAY_SELECTBAR_HEIGHT+16);
			ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_down, &cursor_icon, SETTING_ICON_NORMAL_DRAW);

			ascii_str.str_ptr = select_str1;
			ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);
			
		}
	}else{	//dd/mm/yy    or  //mm/dd/yy
		if(tag ==2)
		{
			String_width = ASCII_draw_char_width*14;	//
			String_width1 = ASCII_draw_char_width*4;	//year
			String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1)+(ASCII_draw_char_width*10);
			Icon_pos_x	 =String_pos_x-((ICON_YEAR_SELECTBAR_WIDTH-String_width1)>>1);	
			Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4);

			icon.icon_w = ICON_YEAR_SELECTBAR_WIDTH;
			icon.icon_h = ICON_YEAR_SELECTBAR_HEIGHT;
			icon.pos_x =Icon_pos_x;	
			icon.pos_y =Icon_pos_y+icon_date_time_offset;
			ap_setting_show_GPZP_file((INT8U*)"YEAR_SELECTBAR.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);
			
			cursor_icon.pos_x = Icon_pos_x+((ICON_YEAR_SELECTBAR_WIDTH-32)>>1);//16
			cursor_icon.pos_y = Icon_pos_y+icon_date_time_offset-16;
			ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_up, &cursor_icon, SETTING_ICON_NORMAL_DRAW);
			cursor_icon.pos_y += (ICON_YEAR_SELECTBAR_HEIGHT+16);
			ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_down, &cursor_icon, SETTING_ICON_NORMAL_DRAW);
			for(i=0;i<4;i++){
				select_str[i] = *(dt_str+10+i);
			}
			ascii_str.pos_x = String_pos_x;
			ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
			ascii_str.str_ptr = select_str;
			ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);

		}else{
			
			icon.icon_w = ICON_DAY_SELECTBAR_WIDTH;
			icon.icon_h = ICON_DAY_SELECTBAR_HEIGHT;
			if(tag == 1){
				String_width = ASCII_draw_char_width*14;	
				String_width1 = ASCII_draw_char_width*2;	//day
				
				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1)+(ASCII_draw_char_width*5);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset;

				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+5+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);

			}else if(tag == 0){
				String_width = ASCII_draw_char_width*14;	
				String_width1 = ASCII_draw_char_width*2;	//day

				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset;

				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)+icon_date_time_offset+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
				
			}else if(tag == 3){
				String_width = ASCII_draw_char_width*12;	
				String_width1 = ASCII_draw_char_width*2;	//time
				
				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+(((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)*2)+ICON_DAY_SELECTBAR_HEIGHT;
				
				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+17+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*2+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
			}else if(tag == 4){
				String_width = ASCII_draw_char_width*12;	
				String_width1 = ASCII_draw_char_width*2;	//time

				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1)+(ASCII_draw_char_width*5);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+(((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)*2)+ICON_DAY_SELECTBAR_HEIGHT;

				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+17+5+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*2+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
			}else if(tag == 5){
				String_width = ASCII_draw_char_width*12;	
				String_width1 = ASCII_draw_char_width*2;	//time

				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1)+(ASCII_draw_char_width*10);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+(((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)*2)+ICON_DAY_SELECTBAR_HEIGHT;

				for(i=0;i<2;i++){
					select_str1[i] = *(dt_str+17+10+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*2+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
			}else if(tag == 6){
				String_width = ASCII_draw_char_width*8;	
				String_width1 = ASCII_draw_char_width*2;	//mode

				String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
				Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
				Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+(((ICON_SELECTBOX_SUB_HEIGHT-(ICON_DAY_SELECTBAR_HEIGHT*3))/4)*3)+ICON_DAY_SELECTBAR_HEIGHT*2-3;

				for(i=0;i<2;i++){
					select_str1[i] = *(Data_Time_Mode1+i);
				}
				ascii_str.pos_x = String_pos_x;
				ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+ICON_YEAR_SELECTBAR_HEIGHT*2+((ICON_SELECTBOX_SUB_HEIGHT-(ICON_YEAR_SELECTBAR_HEIGHT*3))/4)*3+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1)-icon_date_time_offset;
			}
			icon.pos_x =Icon_pos_x;	
			icon.pos_y =Icon_pos_y;
			ap_setting_show_GPZP_file((INT8U*)"DAY_SELECTBAR.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);
			
			cursor_icon.pos_x = Icon_pos_x+((ICON_DAY_SELECTBAR_WIDTH-32)>>1);//16
			cursor_icon.pos_y = Icon_pos_y-16;
			ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_up, &cursor_icon, SETTING_ICON_NORMAL_DRAW);
			cursor_icon.pos_y += (ICON_DAY_SELECTBAR_HEIGHT+16);
			ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_down, &cursor_icon, SETTING_ICON_NORMAL_DRAW);
			
			ascii_str.str_ptr = select_str1;
			ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);
			
		}
	}

	ap_setting_frame_buff_display();

}

#define icon_date_time_offset		3
void ap_setting_wifi_menu_draw(INT8U tag ,char *string)
{
	STRING_INFO str = {0};
	STRING_ASCII_INFO ascii_str;
	DISPLAY_ICONSHOW icon;
	DISPLAY_ICONSHOW cursor_icon = {320, 234, TRANSPARENT_COLOR, 0, 0};
	INT32U i;
	char select_str1[] = "0";
	INT16U String_width; 	
	INT16U String_width1; 	
	INT16U String_height;
	INT16U String_pos_x;
//	INT16U String_pos_y;
	INT16U Icon_pos_x;	
	INT16U Icon_pos_y;
	char dt_str[] = "1 2 3 4 5 6 7 8";
	char dt_str1[] = "1 2 3 4 5";
	INT8U tag_arry[] = {0, 2, 4, 6, 8, 10, 12, 14};
	t_STRING_TABLE_STRUCT str_res;
	
	icon.transparent = TRANSPARENT_COLOR;
	icon.icon_w = ICON_SELECTBOX_SUB_WIDTH;
	icon.icon_h = ICON_SELECTBOX_SUB_HEIGHT;
	icon.pos_x = ICON_SELECTBOX_SUB_START_X;	
	icon.pos_y = ICON_SELECTBOX_SUB_START_Y;

	ap_setting_show_GPZP_file((INT8U*)"SELECTBOX_SUB.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);
	

	str.font_type = 0;	
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;	
	str.font_color = 0xFFFF;
	str.str_idx = g_manu_stage;
	ap_state_resource_string_resolution_get(&str, &str_res);
	str.pos_x = TFT_WIDTH-str_res.string_width-10;//(320 - str_res.string_width)-10;
	str.pos_y = 7;
	ap_state_resource_string_draw((INT16U *)setting_frame_buff, &str, RGB565_DRAW);

	String_height = ASCII_draw_char_height;


	str.font_type = 0;	
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;	
	str.font_color = 0xFFFF;

	#if WIFI_ENCRYPTION_METHOD == WPA2
	for (i=0 ; i<=7 ; i++) {
		*(dt_str + tag_arry[i]) = *(string+i);
	}
	#else
	if(g_manu_stage == STR_WIFI_PASSWORD)
	{
		for (i=0 ; i<=4 ; i++) {
			*(dt_str1 + tag_arry[i]) = *(string+i);
		}
	}
	else
	{
		for (i=0 ; i<=7 ; i++) {
			*(dt_str + tag_arry[i]) = *(string+i);
		}
	}
	#endif

	#if WIFI_ENCRYPTION_METHOD == WPA2
	String_width = ASCII_draw_char_width*15;	
	#else
	if(g_manu_stage == STR_WIFI_PASSWORD)
	{
		String_width = ASCII_draw_char_width*9;
	}
	else
	{
		String_width = ASCII_draw_char_width*15;
	}
	#endif
	ascii_str.font_color = 0xFFFF;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ascii_str.pos_x = ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1);
	ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-ICON_DAY_SELECTBAR_HEIGHT)>>1)+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);
	#if WIFI_ENCRYPTION_METHOD == WPA2
	ascii_str.str_ptr = dt_str;
	#else
	if(g_manu_stage == STR_WIFI_PASSWORD)
	{
		ascii_str.str_ptr = dt_str1;
	}
	else
	{
		ascii_str.str_ptr = dt_str;
	}
	#endif
	ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);

	ascii_str.font_color = BLUE_COLOR;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;


	icon.transparent = TRANSPARENT_COLOR;
	cursor_icon.icon_w = 32;//16;
	cursor_icon.icon_h = 16;

	icon.icon_w = ICON_DAY_SELECTBAR_WIDTH;
	icon.icon_h = ICON_DAY_SELECTBAR_HEIGHT;

	#if WIFI_ENCRYPTION_METHOD == WPA2
	String_width = ASCII_draw_char_width*15;	
	#else
	if(g_manu_stage == STR_WIFI_PASSWORD)
	{
		String_width = ASCII_draw_char_width*9;
	}
	else
	{
		String_width = ASCII_draw_char_width*15;
	}
	#endif
	String_width1 = ASCII_draw_char_width;	
	
	String_pos_x =ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-String_width)>>1)+(ASCII_draw_char_width*tag_arry[tag]);
	Icon_pos_x	 =String_pos_x-((ICON_DAY_SELECTBAR_WIDTH-String_width1)>>1);	
	Icon_pos_y   =ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-ICON_YEAR_SELECTBAR_HEIGHT)>>1);
	#if WIFI_ENCRYPTION_METHOD == WPA2
	select_str1[0] = *(dt_str+tag_arry[tag]);
	#else
	if(g_manu_stage == STR_WIFI_PASSWORD)
	{
		select_str1[0] = *(dt_str1+tag_arry[tag]);
	}
	else
	{
		select_str1[0] = *(dt_str+tag_arry[tag]);
	}
	#endif
	ascii_str.pos_x = String_pos_x;
	ascii_str.pos_y = ICON_SELECTBOX_SUB_START_Y+((ICON_SELECTBOX_SUB_HEIGHT-ICON_YEAR_SELECTBAR_HEIGHT)>>1)+((ICON_YEAR_SELECTBAR_HEIGHT-String_height)>>1);

	icon.pos_x =Icon_pos_x;	
	icon.pos_y =Icon_pos_y;
	ap_setting_show_GPZP_file((INT8U*)"DAY_SELECTBAR.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);
	
	cursor_icon.pos_x = Icon_pos_x+((ICON_DAY_SELECTBAR_WIDTH-32)>>1);//16
	cursor_icon.pos_y = Icon_pos_y-16;
	ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_up, &cursor_icon, SETTING_ICON_NORMAL_DRAW);
	cursor_icon.pos_y += (ICON_DAY_SELECTBAR_HEIGHT+16);
	ap_setting_icon_draw((INT16U *)setting_frame_buff, ui_down, &cursor_icon, SETTING_ICON_NORMAL_DRAW);

	ascii_str.str_ptr = select_str1;
	ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);
	ap_setting_frame_buff_display();

}

void ap_setting_background_icon_string_draw(INT8U flag)
{
	DISPLAY_ICONSHOW icon = {32, 32, TRANSPARENT_COLOR, 0, 0};
	
	icon.icon_w = ICON_BACKGROUND_MODE_WIDTH;
	icon.icon_h = ICON_BACKGROUND_MODE_HEIGHT;
	icon.pos_x = ICON_BACKGROUND_MODE_START_X;
	icon.pos_y = ICON_BACKGROUND_MODE_START_Y;
	ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_background_mode, &icon, SETTING_ICON_NORMAL_DRAW);

	icon.icon_w = ICON_BACKGROUND_LEFT_WIDTH;
	icon.icon_h = ICON_BACKGROUND_LEFT_HEIGHT;
	icon.pos_x = ICON_BACKGROUND_LEFT_START_X;
	icon.pos_y = ICON_BACKGROUND_LEFT_START_Y;
//	ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_background_left, &icon, SETTING_ICON_NORMAL_DRAW);
	icon.pos_x = ICON_BACKGROUND_LEFT_START_X+20;
	ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_background_up, &icon, SETTING_ICON_NORMAL_DRAW);
	icon.pos_x = ICON_BACKGROUND_LEFT_START_X+40;
	ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_background_down, &icon, SETTING_ICON_NORMAL_DRAW);
//	icon.pos_x = ICON_BACKGROUND_LEFT_START_X+60;
//	ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_background_right, &icon, SETTING_ICON_NORMAL_DRAW);

	icon.pos_x = ICON_BACKGROUND_LEFT_START_X+60+32;
	ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_background_ok, &icon, SETTING_ICON_NORMAL_DRAW);
}

void ap_setting_background_draw(INT32U state,INT32U state1,STRING_INFO *str, INT8U flag)
{
	DISPLAY_ICONSHOW icon = {32, 32, TRANSPARENT_COLOR, 0, 0};
	t_STRING_TABLE_STRUCT str_res;
	INT8U type;
	
	icon.icon_w = ICON_TOP_BAR_WIDTH;
	icon.icon_h = ICON_TOP_BAR_HEIGHT;
	icon.pos_y = ICON_TOP_BAR_START_Y;
	if (state == STATE_SETTING) {
		str->str_idx = STR_SETUP;
		ap_state_resource_string_resolution_get(str, &str_res);
		str->pos_x = (320 - str_res.string_width)-10;
		str->pos_y = 23;
		ap_state_resource_string_draw((INT16U *)setting_frame_buff, str, RGB565_DRAW);
		icon.pos_x = ICON_TOP_BAR_START_X+50;

	} else{
		str->str_idx = state_str;
		ap_state_resource_string_resolution_get(str, &str_res);
		str->pos_x = 40;//(320 - str_res.string_width)-10;
		str->pos_y = 7;
		ap_state_resource_string_draw((INT16U *)setting_frame_buff, str, RGB565_DRAW);
		icon.pos_x = ICON_TOP_BAR_START_X;

	}
	//ap_setting_show_GPZP_file((INT8U*)"TOPBAR.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);

	//ap_setting_background_icon_string_draw(flag);

	
	icon.icon_w = 28;
	icon.icon_h = 28;
	icon.pos_x = ICON_TOP_BAR_START_X;//+((ICON_TOP_BAR_WIDTH-icon.icon_w)>>1);
	icon.pos_y = ICON_TOP_BAR_START_Y;
	/*if (state == STATE_SETTING) {
		type = SETTING_ICON_NORMAL_DRAW;
	}else{
		type = SETTING_ICON_BLUE1_COLOR;
	}
	if(state1 == STATE_VIDEO_RECORD){
		ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_video, &icon, type);
	}else if(state1 == STATE_VIDEO_PREVIEW){
		ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_capture, &icon, type);
	}else if(state1 == STATE_BROWSE){
		ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_review, &icon, type);
	}
	icon.pos_x = ICON_TOP_BAR_START_X+((ICON_TOP_BAR_WIDTH-icon.icon_w)>>1)+50;
	icon.pos_y = 10;
	if (state == STATE_SETTING) {
		type = SETTING_ICON_BLUE1_COLOR;
	}else{
		type = SETTING_ICON_NORMAL_DRAW;
	}
	ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_base_setting, &icon, type);*/
	type = SETTING_ICON_NORMAL_DRAW;
	ap_setting_icon_draw((INT16U *)setting_frame_buff, icon_base_setting, &icon, type);


}

void ap_setting_page_page_number(INT8U *tag,INT8U flag)
{
	char page_num[]= "1/1";
	char temp;
	INT8U type;
	STRING_ASCII_INFO ascii_str;

	if(flag){
		type = setting_item.item_max/6;
		if(setting_item.item_max>(type*6))
			type += 1;
		temp = type+0x30;
		page_num[2] = temp;
		
		type = (*tag+1)/6;
		if((*tag+1)>(type*6))
			type += 1;
		temp = type+0x30;
		page_num[0] = temp;

		ascii_str.font_color = 0xffff;
		ascii_str.font_type = 0;
		ascii_str.buff_w = TFT_WIDTH;
		ascii_str.buff_h = TFT_HEIGHT;
		ascii_str.pos_x = TFT_WIDTH - ASCII_draw_char_width*3-10;
		ascii_str.pos_y = ICON_BACKGROUND_MODE_START_Y+7;
		ascii_str.str_ptr = page_num;
		ap_state_resource_string_ascii_draw((INT16U *)setting_frame_buff, &ascii_str, RGB565_DRAW);
	}
}

void ap_setting_video_record_set(void)
{
	/*setting_item.item_start = STR_RESOLUTION;
#if Enable_Lane_Departure_Warning_System == 1
	setting_item.item_max = 8;
#else
	setting_item.item_max = 7;
#endif
	state_str = STR_VIDEO;*/
}

void ap_setting_photo_capture_set(void)
{	
	/*setting_item.item_start = STR_RESOLUTION2;
	setting_item.item_max = 10;
	state_str = STR_CAPTURE;*/
}

void ap_setting_browse_set(void)
{
	setting_item.item_start = STR_RESOLUTION;
	setting_item.item_max = 32;
	state_str = STR_SETUP;
}

void ap_setting_basic_set(void)
{
	/*setting_item.item_start = STR_PARK_MODE;
#if TV_OUT_MENU
	setting_item.item_max = 12;
#else
	setting_item.item_max = 11;
#endif*/
}
extern INT16U *setting_record_table[];
extern INT16U *setting_capture_table[];
extern INT16U *setting_play_table[];
extern INT16U *setting_base_table[];
void ap_setting_general_state_draw(INT32U state,STRING_INFO *str, INT8U *tag)
{
	DISPLAY_ICONSHOW icon = {ICON_SETTING_HEAD_WIDTH, ICON_SETTING_HEAD_HEIGHT, TRANSPARENT_COLOR, 0, 0};
//	t_STRING_TABLE_STRUCT str_res;
	INT16U draw_cnt, i;
	INT32U offset;
	INT32U icon_setting_table;
	INT8U type;
	
	icon.pos_x = ICON_SETTING_HEAD_START_X;
	icon.pos_y = ICON_SETTING_HEAD_START_Y;
	str->pos_x = icon.pos_x+icon.icon_w+7;
	str->pos_y = icon.pos_y+6;
	str->str_idx = ((*tag/6)*6) + setting_item.item_start;
	draw_cnt = setting_item.item_max - (str->str_idx - setting_item.item_start);
	if (draw_cnt > 6) {
		draw_cnt = 6;
	}
	for (i=0; i<draw_cnt; i++) {
		offset = str->str_idx - setting_item.item_start;
		if(state == STATE_VIDEO_RECORD){
			icon_setting_table = (INT32U)setting_record_table[offset];
		}else if(state == STATE_VIDEO_PREVIEW){
			icon_setting_table = (INT32U)setting_capture_table[offset];
		}else if(state == STATE_BROWSE){
			icon_setting_table = (INT32U)setting_play_table[offset];
		}else if(state == STATE_SETTING){
			icon_setting_table = (INT32U)setting_base_table[offset];
		}
		if(MODE_KEY_flag == 0){
			if(str->str_idx == setting_item.item_start+*tag){
				type = SETTING_ICON_BLUE_COLOR;
				str->font_color = BLUE_COLOR;
			}else{
				type = SETTING_ICON_NORMAL_DRAW;
				str->font_color = 0xffff;
			}
		}else{
			type = SETTING_ICON_NORMAL_DRAW;
			str->font_color = 0xffff;
		}
		ap_setting_icon_draw((INT16U *)setting_frame_buff, (INT16U *)icon_setting_table, &icon, type);

		ap_state_resource_string_draw((INT16U *)setting_frame_buff, str, RGB565_DRAW);
		str->str_idx++;
		str->pos_x = icon.pos_x+icon.icon_w+7;
		str->pos_y += 29;
		icon.pos_y = str->pos_y-6;
		
	}

}

extern void gp_cdsp_set_awb_by_config(INT32U awb_value);
extern void ap_iso_set_by_config(INT32U iso);
void ap_edge_set_by_config(INT8U edge);
extern void bkground_del_disable(INT32U disable1_enable0);
extern void sensor_set_fps(INT32U fpsValue);
INT8U ap_setting_right_menu_active(STRING_INFO *str, INT8U type, INT8U *sub_tag)
{
	TIME_T	g_time;
	INT8U curr_tag;
	INT8U i;
	
	switch (str->str_idx) {
		case STR_RESOLUTION:
			curr_tag = ap_state_config_video_resolution_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_1080FHD;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 5; //6; // Remove QVGA 
				setting_item.sub_item_start = STR_1080FHD;
				g_manu_stage = STR_RESOLUTION;
			} else if (type == FUNCTION_ACTIVE){
				ap_state_config_video_resolution_set(str->font_color);
				//if (str->font_color < 2) sensor_set_fps(20);
				//else sensor_set_fps(25);
				sensor_set_fps(20);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_RESOLUTION:%d\r\n",str->font_color);
				#endif
			}
			break;		
		case STR_VIDEOLAPSE:
			curr_tag = ap_state_config_videolapse_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 7;	
				setting_item.sub_item_start = STR_OFF;
				g_manu_stage = STR_VIDEOLAPSE;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_videolapse_set(str->font_color);
                //ap_storage_service_del_thread_mb_set();
			}
			break;	
		case STR_LOOPRECORDING:
			curr_tag = ap_state_config_record_time_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF1;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 6;	
				setting_item.sub_item_start = STR_OFF1;
				g_manu_stage = STR_LOOPRECORDING;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_record_time_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_LOOPRECORDING:%d\r\n",str->font_color);
				#endif
				
				#if 1
				if(str->font_color==0) {
                    bkground_del_disable(1);
                } else {
                	bkground_del_disable(0);
                }
                #endif
                ap_storage_service_del_thread_mb_set();
			}
			break;
		case STR_WDR:
			curr_tag = ap_state_config_wdr_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;	
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_WDR;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_wdr_set(str->font_color);
			}
			break;
		case STR_MOTIONDETECT:
			curr_tag = ap_state_config_md_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_MOTIONDETECT;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_md_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_MOTIONDETECT:%d\r\n",str->font_color);
				#endif
			}
			break;
		case STR_RECORD_AUDIO:	
			curr_tag = ap_state_config_voice_record_switch_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_RECORD_AUDIO;
			} else if (type == SUB_MENU_MOVE) {
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_voice_record_switch_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_RECORD_AUDIO:%d\r\n",str->font_color);
				#endif
			}
			break;
		case STR_DATE_STAMP:
			curr_tag = ap_state_config_date_stamp_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_DATE_STAMP;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_date_stamp_set(str->font_color);
				ap_state_config_capture_date_stamp_set(str->font_color);
			}
			break;

		case STR_RESOLUTION2:
			curr_tag = ap_state_config_pic_size_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_12M;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 8;
				setting_item.sub_item_start = STR_12M;
				g_manu_stage = STR_RESOLUTION2;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_pic_size_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_RESOLUTION2:%d\r\n",str->font_color);
				#endif
			}
			break;
		case STR_SEQUENCE:
			curr_tag = ap_state_config_burst_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 5;
				setting_item.sub_item_start = STR_SINGLE;
				g_manu_stage = STR_SEQUENCE;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_burst_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_SEQUENCE:%d\r\n",str->font_color);
				#endif
			}
			break;
		case STR_QUALITY:
			curr_tag = ap_state_config_quality_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_FINE;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 3;
				setting_item.sub_item_start = STR_FINE;
				g_manu_stage = STR_QUALITY;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_quality_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_QUALITY:%d\r\n",str->font_color);
				#endif
			}
			break;
		case STR_SHARPNESS:
			curr_tag = ap_state_config_sharpness_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_STRONG;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 3;
				setting_item.sub_item_start = STR_STRONG;
				g_manu_stage = STR_SHARPNESS;
			} else if (type == SUB_MENU_MOVE) {
/*				if(str->font_color == 1){
					ap_setting_sensor_command_switch(0xAC, 0x20, 1);
				}else{
					ap_setting_sensor_command_switch(0xAC, 0x20, 1);
					ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_Sharpness, str->font_color, sizeof(ap_setting_Sharpness)/setting_item.sub_item_max);
				}
*/
			} else {
				ap_edge_set_by_config(str->font_color);	//// 0: soft, 3:standard, 2: sharp, 1: more sharp
				ap_state_config_sharpness_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_SHARPNESS:%d\r\n",str->font_color);
				#endif
			}
			break;
		case STR_WHITE_BALANCE:
			curr_tag = ap_state_config_white_balance_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_AUTO;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 5;
				setting_item.sub_item_start = STR_AUTO;
				g_manu_stage = STR_WHITE_BALANCE;
			} else if (type == SUB_MENU_MOVE) {
/*				if(str->font_color == 0){
					ap_setting_sensor_command_switch(0x13, 0x02, 1);
				}else{
					ap_setting_sensor_command_switch(0x13, 0x02, 0);
				}				
				ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_White_Balance, str->font_color, sizeof(ap_setting_White_Balance)/setting_item.sub_item_max);
*/
			} else {
				gp_cdsp_set_awb_by_config(str->font_color);
				ap_state_config_white_balance_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_WHITE_BALANCE:%d\r\n",str->font_color);
				#endif
			}
			break;
			
		case STR_ISO:
			curr_tag = ap_state_config_iso_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_AUTO2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 3;
				setting_item.sub_item_start = STR_AUTO2;
				g_manu_stage = STR_ISO;
			} else if (type == SUB_MENU_MOVE) {
/*
				if(str->font_color == 0){
					ap_setting_sensor_command_switch(0x13, 0x04, 1);
				}else{
					ap_setting_sensor_command_switch(0x13, 0x04, 0);
				}			
				ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_ISO_data, str->font_color, sizeof(ap_setting_ISO_data)/setting_item.sub_item_max);				
*/

			} else {
				ap_iso_set_by_config(str->font_color);
				ap_state_config_iso_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_ISO:%d\r\n",str->font_color);
				#endif
			}
			break;
			
		case STR_EV:
			curr_tag = ap_state_config_ev_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_P2_0;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 13;
				setting_item.sub_item_start = STR_P2_0;
				g_manu_stage = STR_EV;
			} else if (type == SUB_MENU_MOVE) {
				ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_EV_data, str->font_color, sizeof(ap_setting_EV_data)/setting_item.sub_item_max);
			} else {
				gp_cdsp_set_ev_val(str->font_color);	//0:+2, 1:+5/3, 2:+4/3, 3:+1.0, 4:+2/3, 5:+1/3, 6:+0.0, 7:-1/3, 8:-2/3, 9:-1.0, 10:-4/3, 11:-5/3, 12:-2.0 
				ap_state_config_ev_set(str->font_color);
				ap_state_config_ev1_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_EV:%d\r\n",str->font_color);
				#endif
			}
			break;

		/*case STR_EV1:
			curr_tag = ap_state_config_ev1_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_P2_0;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 13;
				setting_item.sub_item_start = STR_P2_0;
			} else if (type == SUB_MENU_MOVE) {
//				ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_EV_data, str->font_color, sizeof(ap_setting_EV_data)/setting_item.sub_item_max);
			} else {
				gp_cdsp_set_ev_val(str->font_color);	//0:+2, 1:+5/3, 2:+4/3, 3:+1.0, 4:+2/3, 5:+1/3, 6:+0.0, 7:-1/3, 8:-2/3, 9:-1.0, 10:-4/3, 11:-5/3, 12:-2.0 
				ap_state_config_ev1_set(str->font_color);
			}
			break;*/
		case STR_ANTI_SHAKING:
			curr_tag = ap_state_config_anti_shaking_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_ANTI_SHAKING;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_anti_shaking_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_ANTI_SHAKING:%d\r\n",str->font_color);
				#endif
			}
			break;
		/*case STR_QUICK_REVIEW:
			curr_tag = ap_state_config_preview_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF3;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 3;
				setting_item.sub_item_start = STR_OFF3;
				g_manu_stage = STR_QUICK_REVIEW;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_preview_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_QUICK_REVIEW:%d\r\n",str->font_color);
				#endif
			}
		
			break;*/
			
		case STR_TV_SWITCH:
			curr_tag = ap_state_config_tv_switch_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_TV_SWITCH;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_tv_switch_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_TV_SWITCH:%d\r\n",str->font_color);
				#endif
			}
		
			break;
			
/*		case STR_COLOR:
			curr_tag = ap_state_config_color_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_STANDARD1;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_STANDARD1;
			} else if (type == SUB_MENU_MOVE) {
				if(str->font_color == 0){
					ap_setting_sensor_command_switch(0xA6, 0x01, 0);
				}else{
					ap_setting_sensor_command_switch(0xA6, 0x01, 1);
					ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_Color, str->font_color, sizeof(ap_setting_Color)/setting_item.sub_item_max);
				}
			} else {
				ap_state_config_color_set(str->font_color);
			}
			break;
		case STR_SATURATION:
			curr_tag = ap_state_config_saturation_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_HIGH;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 3;
				setting_item.sub_item_start = STR_HIGH;
			} else if (type == SUB_MENU_MOVE) {
				ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_Saturation, str->font_color, sizeof(ap_setting_Saturation)/setting_item.sub_item_max);
			} else {
				ap_state_config_saturation_set(str->font_color);
			}
			break;
*/
			
		/*case STR_CAPTURE_DATE_STAMP:
			
			curr_tag = ap_state_config_capture_date_stamp_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF3;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 3;
				setting_item.sub_item_start = STR_OFF3;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_capture_date_stamp_set(str->font_color);
			}
			break;*/
		case STR_LANGUAGE:
			curr_tag = ap_state_config_language_get() - 1;
			if (type == STRING_DRAW) {
				str->str_idx = STR_MULTI_LANGUAGE;
				str->language = curr_tag;
				ap_state_resource_string_draw((INT16U *)setting_frame_buff, str, RGB565_DRAW);
				return curr_tag;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 3;//ap_state_resource_language_num_get() - 2;
				setting_item.sub_item_start = STR_MULTI_LANGUAGE;
				g_manu_stage = STR_LANGUAGE;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_language_set(str->font_color);
			}
			break;	
			
		case STR_DATE_TIME:
			if (type == STRING_DRAW) {
//				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.stage = STR_DATE_TIME;
				g_manu_stage = STR_DATE_TIME;
				cal_time_get(&g_time);

				setup_date_time[6] = ap_state_config_data_time_mode_get();
				if(setup_date_time[6] == 0)
				{
					setting_item.sub_item_max = 100;
					setting_item.sub_item_start = 0;
					setup_date_time[0] = g_time.tm_year - 2000;
					setup_date_time[1] = g_time.tm_mon;
					setup_date_time[2] = g_time.tm_mday;
				}else if(setup_date_time[6] == 1){
					setting_item.sub_item_start = 1;
					setting_item.sub_item_max = CalendarCalculateDays(setup_date_time[1], setup_date_time[0]) + 1;
					setup_date_time[0] = g_time.tm_mday;
					setup_date_time[1] = g_time.tm_mon;
					setup_date_time[2] = g_time.tm_year - 2000;
				}else if(setup_date_time[6] == 2){
					setting_item.sub_item_start = 1;
					setting_item.sub_item_max = 13;
					setup_date_time[0] = g_time.tm_mon;
					setup_date_time[1] = g_time.tm_mday;
					setup_date_time[2] = g_time.tm_year - 2000;
				}else{
					setup_date_time[6] = 0;
					setting_item.sub_item_max = 100;
					setting_item.sub_item_start = 0;
					setup_date_time[0] = g_time.tm_year - 2000;
					setup_date_time[1] = g_time.tm_mon;
					setup_date_time[2] = g_time.tm_mday;
				}
				setup_date_time[3] = g_time.tm_hour;
				setup_date_time[4] = g_time.tm_min;
				setup_date_time[5] = g_time.tm_sec;
				
				ap_setting_date_time_menu_draw(0);
				return 0;
			} else if (type == FUNCTION_ACTIVE) {

				 {
					*sub_tag += 1;
					if(*sub_tag == 7)
						*sub_tag=0;
					if(setup_date_time[6] == 0){
						if(*sub_tag == 0){
							setting_item.sub_item_max = 100;
							setting_item.sub_item_start = 14;
						}else if (*sub_tag == 1) {
							setting_item.sub_item_start = 1;
							setting_item.sub_item_max = 13;
						} else if (*sub_tag == 2) {
							setting_item.sub_item_start = 1;
							setting_item.sub_item_max = CalendarCalculateDays(setup_date_time[1], setup_date_time[0]) + 1;
						} else if (*sub_tag == 3) {
							setting_item.sub_item_start = 0;
							setting_item.sub_item_max = 24;
						} else if(*sub_tag ==6){
							setting_item.sub_item_start = 0;
							setting_item.sub_item_max = 3;
						} else {
							setting_item.sub_item_start = 0;
							setting_item.sub_item_max = 60;
						}
					}else if(setup_date_time[6] == 1){
						if(*sub_tag == 0){
							setting_item.sub_item_start = 1;
							setting_item.sub_item_max = CalendarCalculateDays(setup_date_time[1], setup_date_time[0]) + 1;
						}else if (*sub_tag == 1) {
							setting_item.sub_item_start = 1;
							setting_item.sub_item_max = 13;
						} else if (*sub_tag == 2) {
							setting_item.sub_item_max = 100;
							setting_item.sub_item_start = 14;
						} else if (*sub_tag == 3) {
							setting_item.sub_item_start = 0;
							setting_item.sub_item_max = 24;
						} else if(*sub_tag ==6){
							setting_item.sub_item_start = 0;
							setting_item.sub_item_max = 3;
						} else {
							setting_item.sub_item_start = 0;
							setting_item.sub_item_max = 60;
						}
					}else if(setup_date_time[6] == 2){
						if(*sub_tag == 0){
							setting_item.sub_item_start = 1;
							setting_item.sub_item_max = 13;
						}else if (*sub_tag == 1) {
							setting_item.sub_item_start = 1;
							setting_item.sub_item_max = CalendarCalculateDays(setup_date_time[1], setup_date_time[0]) + 1;
						} else if (*sub_tag == 2) {
							setting_item.sub_item_max = 100;
							setting_item.sub_item_start = 14;
						} else if (*sub_tag == 3) {
							setting_item.sub_item_start = 0;
							setting_item.sub_item_max = 24;
						} else if(*sub_tag ==6){
							setting_item.sub_item_start = 0;
							setting_item.sub_item_max = 3;
						} else {
							setting_item.sub_item_start = 0;
							setting_item.sub_item_max = 60;
						}
					}
					ap_setting_date_time_menu_draw(*sub_tag);
				}
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_DATE_TIME2:%d\r\n",str->font_color);
				#endif
				
			}
			break;
			
		case STR_CAR_MODE:
			curr_tag = ap_state_config_car_mode_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_CAR_MODE;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_car_mode_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_CAR_MODE:%d\r\n",str->font_color);
				#endif
			}
		
			break;
			
		case STR_WIFI_SSID:
			if (type == STRING_DRAW) {
//				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.stage = STR_WIFI_SSID;
				g_manu_stage = STR_WIFI_SSID;		
				setting_item.sub_item_start = '0';
				setting_item.sub_item_max = 123;	// '{' 122-->'z'
				for (i=0 ; i<GPSOCK_WiFi_Name_Length ; i++) {
					wifi_menu_set[i] = wifi_ssid[i];
				}
				ap_setting_wifi_menu_draw(0,wifi_menu_set);
				return 0;
			} else if (type == FUNCTION_ACTIVE) {
				 {
					*sub_tag += 1;
					if(*sub_tag == 8)
						*sub_tag=0;
					setting_item.sub_item_start = '0';
					setting_item.sub_item_max = 123;	
					ap_setting_wifi_menu_draw(*sub_tag,wifi_menu_set);
				}
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_WIFI_SSID:%d\r\n",str->font_color);
				#endif
				
			}
			break;
			
		case STR_WIFI_PASSWORD:
			if (type == STRING_DRAW) {
//				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.stage = STR_WIFI_PASSWORD;
				g_manu_stage = STR_WIFI_PASSWORD;	
				setting_item.sub_item_start = '0';
				setting_item.sub_item_max = 123;	// '{' 122-->'z'
				#if WIFI_ENCRYPTION_METHOD == WPA2
				for (i=0 ; i<=7 ; i++) {
					wifi_menu_set[i] = wifi_password[i];
				}	
				#else
				for (i=0 ; i<=4 ; i++) {
					wifi_menu_set[i] = wifi_password[i];
				}
				#endif		
				ap_setting_wifi_menu_draw(0,wifi_menu_set);
				return 0;
			} else if (type == FUNCTION_ACTIVE) {
				 {
					*sub_tag += 1;
					#if WIFI_ENCRYPTION_METHOD == WPA2
					if(*sub_tag == 8)
						*sub_tag=0;
					#else
					if(*sub_tag == 5)
						*sub_tag=0;
					#endif
					setting_item.sub_item_start = '0';
					setting_item.sub_item_max = 123;	
					ap_setting_wifi_menu_draw(*sub_tag,wifi_menu_set);
				}
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_WIFI_PASSWORD:%d\r\n",str->font_color);
				#endif
				
			}
			break;
			
		case STR_OSD_MODE:
			curr_tag = ap_state_config_osd_mode_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;	
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_OSD_MODE;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_osd_mode_set(str->font_color);
			}
			break;
		
		case STR_AUTO_POWER_OFF:
			curr_tag = ap_state_config_auto_off_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF90;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 3;
				setting_item.sub_item_start = STR_OFF90;
				g_manu_stage = STR_AUTO_POWER_OFF;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_auto_off_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_AUTO_POWER_OFF:%d\r\n",str->font_color);
				#endif
			}
			break;
		case STR_SCREEN_SAVER:
			curr_tag = ap_state_config_auto_off_TFT_BL_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF91;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 4;
				setting_item.sub_item_start = STR_OFF91;
				g_manu_stage = STR_SCREEN_SAVER;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_auto_off_TFT_BL_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_SCREEN_SAVER:%d\r\n",str->font_color);
				#endif
			}
			break;
		
		case STR_BEEP_SOUND:		
			curr_tag = ap_state_config_beep_sound_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;

			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_BEEP_SOUND;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_beep_sound_set(str->font_color);
			}
			break;
		
#if TV_OUT_MENU
		case STR_TV_MODE:
			curr_tag = ap_state_config_tv_out_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_NTSC;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_NTSC;
				g_manu_stage = STR_TV_MODE;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_tv_out_set(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_TV_MODE:%d\r\n",str->font_color);
				#endif
			}
			break;
#endif
		
		case STR_FREQUENCY:
			curr_tag = ap_state_config_light_freq_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_50HZ;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_50HZ;
				g_manu_stage = STR_FREQUENCY;
			} else if (type == SUB_MENU_MOVE) {
				//ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_Light_Freq, str->font_color, sizeof(ap_setting_Light_Freq)/setting_item.sub_item_max);			
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_light_freq_set(str->font_color);
				gp_cdsp_set_exp_freq(str->font_color);
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_FREQUENCY:%d\r\n",str->font_color);
				#endif

			}
			break;
			
		case STR_ROTATE:
			curr_tag = ap_state_config_rotate_get();
			/*if(curr_tag==0)
			{
				curr_tag=1;
			}
			else
			{
				curr_tag=0;
			}*/
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;	
				setting_item.sub_item_start = STR_OFF2;
				g_manu_stage = STR_ROTATE;
			} else if (type == FUNCTION_ACTIVE) {
				/*if(str->font_color==0)
				{
					str->font_color=1;
				}
				else
				{
					str->font_color=0;
				}*/
				ap_state_config_rotate_set(str->font_color);
			}
			break;
			
		case STR_DELETE:
			if (type == STRING_DRAW) {
//				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_DELETE_CURRENT;
				setting_item.stage = STR_DELETE;
				g_manu_stage = STR_DELETE;
			} else if (type == SECOND_SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
				setting_item.stage = 0;
				g_setting_delete_choice = str->font_color;
			} else if (type == FUNCTION_ACTIVE) {
				if(g_setting_delete_choice == 0){
					if (str->font_color == 1) {
						setting_item.stage = 0xAA55;	//wwj add
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VIDEO_FILE_DEL, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
					}
				} else if (g_setting_delete_choice == 1){
					if (str->font_color == 1) {
						ap_setting_busy_show();
						setting_item.stage = 0xAA55;	//wwj add
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FILE_DEL_ALL, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
						OSTimeDly(50);
					}
				}
			}
			break;
		/*case STR_PROTECT:
			if (type == STRING_DRAW) {
//				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 4;
				setting_item.sub_item_start = STR_LOCK_CURRENT;
				setting_item.stage = STR_PROTECT;
				g_manu_stage = STR_PROTECT;
			} else if (type == SECOND_SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
				setting_item.stage = 0;
				g_setting_lock_choice = str->font_color;
			} else if (type == FUNCTION_ACTIVE) {
				if(g_setting_lock_choice == 0){
					if (str->font_color == 1) {
						setting_item.stage = 0xAA55;	//wwj add
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_LOCK_ONE, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
					}
				} else if (g_setting_lock_choice == 1){
					if (str->font_color == 1) {
						setting_item.stage = 0xAA55;	//wwj add
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_UNLOCK_ONE, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
					}				
				} else if (g_setting_lock_choice == 2){
					if (str->font_color == 1) {
						ap_setting_busy_show();
						setting_item.stage = 0xAA55;	//wwj add
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_LOCK_ALL, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
						OSTimeDly(50);
					}				
				} else if (g_setting_lock_choice == 3){
					if (str->font_color == 1) {
						ap_setting_busy_show();
						setting_item.stage = 0xAA55;	//wwj add
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_UNLOCK_ALL, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
						OSTimeDly(50);
					}				
				}
			}
			break;*/						
/*		case STR_DELETE_ONE:
			if (type == STRING_DRAW) {
				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
			} else if (type == FUNCTION_ACTIVE) {
				if (str->font_color == 1) {
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_VIDEO_FILE_DEL, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
				}
			}
			break;
		case STR_DELETE_ALL:
			if (type == STRING_DRAW) {
				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
			} else if (type == FUNCTION_ACTIVE) {
				if (str->font_color == 1) {
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FILE_DEL_ALL, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
				}
			}
			break;
		case STR_LOCK_ONE:
			if (type == STRING_DRAW) {
				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
			} else if (type == FUNCTION_ACTIVE) {
				if (str->font_color == 1) {
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_LOCK_ONE, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
				}
			}
			break;
		case STR_LOCK_ALL:
			if (type == STRING_DRAW) {
				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
			} else if (type == FUNCTION_ACTIVE) {
				if (str->font_color == 1) {
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_LOCK_ALL, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
				}
			}
			break;
		case STR_UNLOCK_ONE:
			if (type == STRING_DRAW) {
				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
			} else if (type == FUNCTION_ACTIVE) {
				if (str->font_color == 1) {
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_UNLOCK_ONE, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
				}
			}
			break;
		case STR_UNLOCK_ALL:
			if (type == STRING_DRAW) {
				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
			} else if (type == FUNCTION_ACTIVE) {
				if (str->font_color == 1) {
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_UNLOCK_ALL, &str->font_color, sizeof(INT32U), MSG_PRI_NORMAL);
				}
			}
			break;					
			
		case STR_THUMBNAIL:
			if (type == STRING_DRAW) {
				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
			} else if (type == FUNCTION_ACTIVE) {
				if (str->font_color == 1) {
					msgQSend(ApQ, MSG_APQ_INIT_THUMBNAIL, NULL, NULL, MSG_PRI_NORMAL);
				}
			}
			break;
		case STR_VOLUME:
			curr_tag = ap_state_config_volume_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_VOLUME_0;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 7;
				setting_item.sub_item_start = STR_VOLUME_0;
			} else if (type == SUB_MENU_MOVE) {
				audio_vol_set(str->font_color);
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_volume_set(str->font_color);
				audio_vol_set(str->font_color);
			}
			break;
			
			
*/
		/*case STR_PARK_MODE:
			
			curr_tag = ap_state_config_park_mode_G_sensor_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF2;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 4;	
				setting_item.sub_item_start = STR_OFF2;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_park_mode_G_sensor_set(str->font_color);
			}
			break;*/


		
		/*case STR_FLASH:
			curr_tag = ap_state_config_flash_LED_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_OFF1;


			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_OFF1;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_flash_LED_set(str->font_color);
				ap_peripheral_night_mode_set(str->font_color);
			}
			
			break;*/
//		case STR_FORMAT:
		case STR_FORMAT1:
			if (type == STRING_DRAW) {
//				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
				setting_item.stage = STR_FORMAT1;
				g_manu_stage = STR_FORMAT1;
			} else if (type == FUNCTION_ACTIVE) {
				setting_item.stage = 0;
				if (str->font_color == 1){
					if(ap_state_handling_storage_id_get() == NO_STORAGE){
						ap_setting_no_sdc_show();
						return 0;
					}else{					
		//				if(fm_tx_status_get() == 1){
		//					msgQSend(AudioTaskQ, MSG_AUD_STOP, NULL, 0, MSG_PRI_NORMAL);
		//				}
						setting_item.stage = 0x55AA;
						return 0;
					}					
				}
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_FORMAT1:%d\r\n",str->font_color);
				#endif
			}
			break;
		
		case STR_DEFAULT_SETTING:
			if (type == STRING_DRAW) {
//				str->str_idx = STR_NEXT_MENU;
			} else if (type == SUB_MENU_DRAW) {
				curr_tag = 0;
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_CANCEL;
				setting_item.stage = STR_DEFAULT_SETTING;
				g_manu_stage = STR_DEFAULT_SETTING;
		} else if (type == FUNCTION_ACTIVE) {
				setting_item.stage = 0;
				if (str->font_color == 1) {
					ap_state_config_default_set();
/*					ap_state_config_factory_date_get(&setup_date_time[0]);
					ap_state_config_factory_time_get(&setup_date_time[3]);
					g_time.tm_year = setup_date_time[0] + 2000;
					g_time.tm_mon = setup_date_time[1];
					g_time.tm_mday = setup_date_time[2];
					g_time.tm_hour = setup_date_time[3];
					g_time.tm_min = setup_date_time[4];
					g_time.tm_sec = setup_date_time[5];
					cal_time_set(g_time);					
*/					ap_setting_value_set_from_user_config();
				}
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_DEFAULT_SETTING:%d\r\n",str->font_color);
				#endif
			}
			break;
		case STR_VERSION:
			curr_tag = 0;
			if (type == STRING_DRAW) {
//				str->str_idx = STR_NEXT_MENU;
//				str->pos_x = 130;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.stage = STR_VERSION;
				g_manu_stage = STR_VERSION;
				ap_setting_show_software_version();
				return 0;
			} else if (type == FUNCTION_ACTIVE) {
				#if MENU_ITEM_PRINTF_DEBUG_ENABLE
				DBG_PRINT("STR_VERSION:%d\r\n",str->font_color);
				#endif
				return 0;
			}
			break;



/*
		case STR_USB:
			curr_tag = ap_state_config_usb_mode_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_MASS_STORAGE;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 2;
				setting_item.sub_item_start = STR_MASS_STORAGE;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_usb_mode_set(str->font_color);
			}
			break;
			
		case STR_MD_SENSITIVITY:
			curr_tag = ap_state_config_motion_detect_get();
			if (type == STRING_DRAW) {
				str->str_idx = curr_tag + STR_HIGH;
			} else if (type == SUB_MENU_DRAW) {
				setting_item.sub_item_max = 3;
				setting_item.sub_item_start = STR_HIGH;
			} else if (type == FUNCTION_ACTIVE) {
				ap_state_config_motion_detect_set(str->font_color);
			}
			break;
*/
		default:
			break;
	}
	if (type == STRING_DRAW) {
		ap_state_resource_string_draw((INT16U *)setting_frame_buff, str, RGB565_DRAW);
	} else if ((type == SUB_MENU_DRAW) || (type == SECOND_SUB_MENU_DRAW)) {
		ap_setting_sub_menu_draw(curr_tag);
	} else if (type == FUNCTION_ACTIVE) {
		ap_state_config_store();
	}
	return curr_tag;
}

void ap_setting_func_key_active(INT8U *tag, INT8U *sub_tag, INT32U state,INT32U state1)
{
	STRING_INFO str_fake = {0};
	
	if(MODE_KEY_flag == 1){
		return;
	}
	if (setting_item.stage == 0x55AA) {
		return;
	}
	str_fake.str_idx = *tag + setting_item.item_start;
	if (*sub_tag == 0xFF) {
		ap_setting_page_draw_no_display(state,state1, tag,0);
		*sub_tag = ap_setting_right_menu_active(&str_fake, SUB_MENU_DRAW, NULL);
	} else {
		str_fake.font_color = *sub_tag;
		///if ((setting_item.stage == STR_DELETE) || (setting_item.stage == STR_PROTECT)) {
		if (setting_item.stage == STR_DELETE) {
			ap_setting_right_menu_active(&str_fake, SECOND_SUB_MENU_DRAW, (INT8U *)sub_tag);
			*sub_tag = 0;
			return;
		}
		/*if(setting_item.stage == STR_LDW){
			if(g_setting_LDW_flag==1){
				ap_setting_right_menu_active(&str_fake, SUB_MENU_DRAW, (INT8U *)sub_tag);
			}else{
				ap_setting_right_menu_active(&str_fake, SECOND_SUB_MENU_DRAW, (INT8U *)sub_tag);
			}
			*sub_tag = 0;
			return;
		}*/
		ap_setting_right_menu_active(&str_fake, FUNCTION_ACTIVE, (INT8U *)sub_tag);
		if ((setting_item.stage == STR_DATE_TIME)||(setting_item.stage == STR_WIFI_SSID)||(setting_item.stage == STR_WIFI_PASSWORD)) {
			return;
		} else if(setting_item.stage == STR_VERSION){
			setting_item.stage = 0;
		} else if (setting_item.stage == 0x55AA) {
			ap_setting_page_draw(state,state1, tag);
			ap_setting_busy_show();
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FORMAT_REQ, NULL, NULL, MSG_PRI_NORMAL);
			*sub_tag = 0xFF;
			return;
		} else if (setting_item.stage == 0xAA55) {
			setting_item.stage = 0x55AA;
		}
		*sub_tag = 0xFF;
		ap_setting_page_draw(state,state1, tag);		
	}
}

void ap_setting_direction_key_active(INT8U *tag, INT8U *sub_tag, INT32U key_type, INT32U state,INT32U state1)
{
	STRING_INFO str_fake = {0};
	INT8U total, *tag_ptr, bound;
	INT32U key_type_temp;
	
	if(MODE_KEY_flag == 1){
		MODE_KEY_flag = 0;
		ap_setting_page_draw(state,state1, tag);
		return;
	}
	if(setting_item.stage == STR_VERSION) {
		return;
	}
	key_type_temp = MSG_APQ_NEXT_KEY_ACTIVE;
	if (setting_item.stage == 0x55AA) {
		return;
	}
	str_fake.str_idx = *tag + setting_item.item_start;	
	bound = 0;
	if (*sub_tag == 0xFF) {
		total = setting_item.item_max;
		tag_ptr = tag;
	} else {
		if (setting_item.stage == STR_DATE_TIME) {
			total = setting_item.sub_item_max;
			tag_ptr = &setup_date_time[*sub_tag];
			bound = setting_item.sub_item_start;
			key_type_temp = MSG_APQ_PREV_KEY_ACTIVE;
		}else if ((setting_item.stage == STR_WIFI_SSID)||(setting_item.stage == STR_WIFI_PASSWORD)) {
			total = setting_item.sub_item_max;
			tag_ptr = (INT8U *)&wifi_menu_set[*sub_tag];
			bound = setting_item.sub_item_start;
			key_type_temp = MSG_APQ_PREV_KEY_ACTIVE;		
		} else if(setting_item.stage == STR_VERSION) {
			tag_ptr = sub_tag;

		/*} else if(setting_item.stage == STR_LDW) {
			total = setting_item.sub_item_max;
			if(g_setting_LDW_flag == 1){
				tag_ptr = &g_setting_LDW_value;
			}else{
				tag_ptr = &g_setting_LDW_choice;
			}*/
		} else {
			total = setting_item.sub_item_max;
			tag_ptr = sub_tag;
		}
	}
	if ((setting_item.stage == STR_WIFI_SSID)||(setting_item.stage == STR_WIFI_PASSWORD))
	{
		if (key_type == key_type_temp) {
			if (*tag_ptr == 0) {
				*tag_ptr = '/';
			}
			(*tag_ptr)++;
			if(*tag_ptr == ':')
			{
				*tag_ptr = 'A';
			}
			else if(*tag_ptr == '[')
			{
				*tag_ptr = 'a';
			}
			if (*tag_ptr >= total) {
				*tag_ptr = bound;
			}
		} else {
			if (*tag_ptr == 0) {
				*tag_ptr = total;
			}
			if (*tag_ptr == bound) {
				*tag_ptr = total;
			}
			if(*tag_ptr == 'a')
			{
				*tag_ptr = '[';
			}
			else if(*tag_ptr == 'A')
			{
				*tag_ptr = ':';
			}
			(*tag_ptr)--;
		}
	}
	else
	{
		if (key_type == key_type_temp) {
			(*tag_ptr)++;
			if (*tag_ptr >= total) {
				*tag_ptr = bound;
			}
		} else {
			if (*tag_ptr == bound) {
				*tag_ptr = total;
			}
			(*tag_ptr)--;
		}
	}
	if (*sub_tag == 0xFF) {
		ap_setting_page_draw(state,state1, tag);
	} else {
		if (setting_item.stage == STR_DATE_TIME) {
			if(*sub_tag == 6)
			{
				if (key_type == key_type_temp) {
					INT8U temp;
					if(setup_date_time[6] == 0)
					{
						temp = setup_date_time[2];
						setup_date_time[2] = setup_date_time[1];
						setup_date_time[1] = setup_date_time[0];
						setup_date_time[0] = temp;
						
	//					setup_date_time[0] = g_time.tm_year - 2000;
	//					setup_date_time[1] = g_time.tm_mon;
	//					setup_date_time[2] = g_time.tm_mday;
					}else if(setup_date_time[6] == 1){
						temp = setup_date_time[0];
						setup_date_time[0] = setup_date_time[2];
						setup_date_time[2] = temp;

	//					setup_date_time[0] = g_time.tm_mday;
	//					setup_date_time[1] = g_time.tm_mon;
	//					setup_date_time[2] = g_time.tm_year - 2000;
					}else if(setup_date_time[6] == 2){
						temp = setup_date_time[0];
						setup_date_time[0] = setup_date_time[1];
						setup_date_time[1] = temp;

	//					setup_date_time[0] = g_time.tm_mon;
	//					setup_date_time[1] = g_time.tm_mday;
	//					setup_date_time[2] = g_time.tm_year - 2000;
					}
				}else{
					INT8U temp;
					if(setup_date_time[6] == 0)
					{
						temp = setup_date_time[0];
						setup_date_time[0] = setup_date_time[2];
						setup_date_time[2] = temp;
						
	//					setup_date_time[0] = g_time.tm_year - 2000;
	//					setup_date_time[1] = g_time.tm_mon;
	//					setup_date_time[2] = g_time.tm_mday;
					}else if(setup_date_time[6] == 1){
						temp = setup_date_time[0];
						setup_date_time[0] = setup_date_time[1];
						setup_date_time[1] = temp;

	//					setup_date_time[0] = g_time.tm_mday;
	//					setup_date_time[1] = g_time.tm_mon;
	//					setup_date_time[2] = g_time.tm_year - 2000;
					}else if(setup_date_time[6] == 2){
						temp = setup_date_time[0];
						setup_date_time[0] = setup_date_time[1];
						setup_date_time[1] = setup_date_time[2];
						setup_date_time[2] = temp;

	//					setup_date_time[0] = g_time.tm_mon;
	//					setup_date_time[1] = g_time.tm_mday;
	//					setup_date_time[2] = g_time.tm_year - 2000;
					}
				}
			}
			ap_setting_date_time_menu_draw(*sub_tag);
		} else if ((setting_item.stage == STR_WIFI_SSID)||(setting_item.stage == STR_WIFI_PASSWORD)){
			ap_setting_wifi_menu_draw(*sub_tag,wifi_menu_set);
		} else if(setting_item.stage == STR_VERSION){
		/*} else if(setting_item.stage == STR_LDW){

			if(g_setting_LDW_flag == 1){
				ap_setting_sub_menu_draw(g_setting_LDW_value);
				str_fake.font_color = g_setting_LDW_value;
				ap_setting_right_menu_active(&str_fake, SUB_MENU_MOVE, NULL);
			}else{
				ap_setting_sub_menu_draw(g_setting_LDW_choice);
				str_fake.font_color = g_setting_LDW_choice;
				ap_setting_right_menu_active(&str_fake, SUB_MENU_MOVE, NULL);
			}*/
		} else {
			ap_setting_sub_menu_draw(*sub_tag);
			str_fake.font_color = *sub_tag;
			ap_setting_right_menu_active(&str_fake, SUB_MENU_MOVE, NULL);
		}
	}
}

void ap_USB_setting_direction_key_active(INT8U *tag, INT8U *sub_tag, INT32U key_type, INT32U state,INT32U state1)
{
	STRING_INFO str_fake;
	INT8U total, *tag_ptr, bound;
	INT32U key_type_temp;
	INT32U display_frame0;
	INT32U i;
	
	key_type_temp = MSG_APQ_NEXT_KEY_ACTIVE;
	if (setting_item.stage == 0x55AA) {
		return;
	}
	str_fake.str_idx = *tag + setting_item.item_start;	
	bound = 0;
	if (*sub_tag == 0xFF) {
		total = setting_item.item_max;
		tag_ptr = tag;
	} else {
		if (setting_item.stage != STR_DATE_TIME) {
			total = setting_item.sub_item_max;
			tag_ptr = sub_tag;
		} else {
			total = setting_item.sub_item_max;
			tag_ptr = &setup_date_time[*sub_tag];
			bound = setting_item.sub_item_start;
			key_type_temp = MSG_APQ_PREV_KEY_ACTIVE;
		}
	}
	if (key_type == key_type_temp) {
		(*tag_ptr)++;
		if (*tag_ptr >= total) {
			*tag_ptr = bound;
		}
	} else {
		if (*tag_ptr == bound) {
			*tag_ptr = total;
		}
		(*tag_ptr)--;
	}
	if (*sub_tag == 0xFF) {
		for (i=0;i<50;++i){
			display_frame0 = ap_display_queue_get(display_isr_queue);
			if (display_frame0!=NULL)
				break;
			OSTimeDly(1);
		}
		for(i=0; i<TFT_WIDTH*TFT_HEIGHT; i++) {
			*(INT16U *)(display_frame0 + i*2) = 0;
		}
		ap_USB_setting_page_draw(state,state1, tag, display_frame0);
	} else {
	}
}



EXIT_FLAG_ENUM ap_setting_menu_key_active(INT8U *tag, INT8U *sub_tag, INT32U *state, INT32U *state1)
{
	if (setting_item.stage == 0x55AA) {
		return EXIT_RESUME;
	}
	if (*sub_tag == 0xFF) {
		*tag = 0;
		if (*state != STATE_SETTING) {
			*state = STATE_SETTING;
		} else {
			*state = *state1;
			OSQPost(StateHandlingQ, (void *) *state);
			return EXIT_BREAK;
		}
	} else {
		TIME_T	g_time;
		if(setting_item.stage == STR_DATE_TIME){
				if(setup_date_time[6]==0){
					g_time.tm_year = setup_date_time[0] + 2000;
					g_time.tm_mon = setup_date_time[1];
					g_time.tm_mday = setup_date_time[2];
				}else if(setup_date_time[6] == 1){
					g_time.tm_year = setup_date_time[2] + 2000;
					g_time.tm_mon = setup_date_time[1];
					g_time.tm_mday = setup_date_time[0];
				}else if(setup_date_time[6] == 2){
					g_time.tm_year = setup_date_time[2] + 2000;
					g_time.tm_mon = setup_date_time[0];
					g_time.tm_mday = setup_date_time[1];
				}				

				g_time.tm_hour = setup_date_time[3];
				g_time.tm_min = setup_date_time[4];
				g_time.tm_sec = setup_date_time[5];
				cal_time_set(g_time);
			    ap_state_config_data_time_mode_set(setup_date_time[6]);
#if ENABLE_CHECK_RTC == 1
			    ap_state_config_data_time_save_set();
#endif
		} 	
		*sub_tag = 0xFF;
		setting_item.stage = 0;
		g_setting_LDW_choice = 0;
		g_setting_LDW_value = 0;
		g_setting_LDW_flag = 0;
		setup_date_time_counter = 0;
		ap_setting_value_set_from_user_config();
	}
	ap_setting_page_draw(*state,*state1, tag);
	return EXIT_RESUME;
}

EXIT_FLAG_ENUM ap_setting_mode_key_active(INT32U next_state, INT8U *sub_tag)
{
	if (setting_item.stage == 0x55AA) {
		return EXIT_RESUME;
	} else {
		TIME_T	g_time;
		if(setting_item.stage == STR_DATE_TIME){
				if(setup_date_time[6]==0){
					g_time.tm_year = setup_date_time[0] + 2000;
					g_time.tm_mon = setup_date_time[1];
					g_time.tm_mday = setup_date_time[2];
				}else if(setup_date_time[6] == 1){
					g_time.tm_year = setup_date_time[2] + 2000;
					g_time.tm_mon = setup_date_time[1];
					g_time.tm_mday = setup_date_time[0];
				}else if(setup_date_time[6] == 2){
					g_time.tm_year = setup_date_time[2] + 2000;
					g_time.tm_mon = setup_date_time[0];
					g_time.tm_mday = setup_date_time[1];
				}				

				g_time.tm_hour = setup_date_time[3];
				g_time.tm_min = setup_date_time[4];
				g_time.tm_sec = setup_date_time[5];
				cal_time_set(g_time);
			    ap_state_config_data_time_mode_set(setup_date_time[6]);
#if ENABLE_CHECK_RTC == 1
				ap_state_config_data_time_save_set();
#endif
		}
		 	
		*sub_tag = 0xFF;
		setting_item.stage = 0;
		g_setting_LDW_choice = 0;
		g_setting_LDW_value = 0;
		g_setting_LDW_flag = 0;
		setup_date_time_counter = 0;
		//OSQPost(StateHandlingQ, (void *) next_state);
		OSQPost(StateHandlingQ, (void *) STATE_VIDEO_RECORD);
		ap_setting_value_set_from_user_config();
		return EXIT_BREAK;
	}
}

EXIT_FLAG_ENUM ap_setting_mode_key_active1(INT8U *tag, INT8U *sub_tag, INT32U state,INT32U next_state)
{
	EXIT_FLAG_ENUM exit_flag = EXIT_RESUME;
	INT8U i;
	
	if(setting_item.stage == STR_DATE_TIME)
	{
		TIME_T	g_time;
		if(setup_date_time[6]==0){
			g_time.tm_year = setup_date_time[0] + 2000;
			g_time.tm_mon = setup_date_time[1];
			g_time.tm_mday = setup_date_time[2];
		}else if(setup_date_time[6] == 1){
			g_time.tm_year = setup_date_time[2] + 2000;
			g_time.tm_mon = setup_date_time[1];
			g_time.tm_mday = setup_date_time[0];
		}else if(setup_date_time[6] == 2){
			g_time.tm_year = setup_date_time[2] + 2000;
			g_time.tm_mon = setup_date_time[0];
			g_time.tm_mday = setup_date_time[1];
		}				

		g_time.tm_hour = setup_date_time[3];
		g_time.tm_min = setup_date_time[4];
		g_time.tm_sec = setup_date_time[5];
		cal_time_set(g_time);
	    ap_state_config_data_time_mode_set(setup_date_time[6]);
#if ENABLE_CHECK_RTC == 1
		ap_state_config_data_time_save_set();
#endif		
	    setting_item.stage=0;
	    *sub_tag = 0xFF;
	    ap_setting_page_draw(state,next_state, tag);	
	}
	else if ((setting_item.stage == STR_WIFI_SSID)||(setting_item.stage == STR_WIFI_PASSWORD))
	{
		if(setting_item.stage == STR_WIFI_SSID)
		{
			for (i=0 ; i<=7 ; i++) {
				wifi_ssid[i] = wifi_menu_set[i];
			}
		}
		else if(setting_item.stage == STR_WIFI_PASSWORD)
		{
			#if WIFI_ENCRYPTION_METHOD == WPA2
			for (i=0 ; i<=7 ; i++) {
				wifi_password[i] = wifi_menu_set[i];
			}
			#else
			for (i=0 ; i<=4 ; i++) {
				wifi_password[i] = wifi_menu_set[i];
			}
			#endif
		}
	    setting_item.stage=0;
	    *sub_tag = 0xFF;
	    ap_setting_page_draw(state,next_state, tag);	
	}
	else
	{
		exit_flag = ap_setting_mode_key_active(next_state, sub_tag);
	}
	
	return exit_flag;
}

void ap_setting_format_reply(INT8U *tag, INT32U state,INT32U state1)
{
	setting_item.stage = 0;
	ap_setting_page_draw(state,state1, tag);
	//MP3 function reset value
//	if(fm_tx_status_get() == 1){
		ap_music_reset();
//	}	
}

void ap_setting_other_reply(void)
{
	setting_item.stage = 0;
	//MP3 function reset value
//	if(fm_tx_status_get() == 1){
		ap_music_reset();
//	}	
}
void ap_setting_no_sdc_show(void)
{
	// INT32U size, read_buf;
	// INT16U logo_fd;
	DISPLAY_ICONSHOW icon;
	t_STRING_TABLE_STRUCT str_res;
	STRING_INFO str = {0};
	// INT8U *zip_buf;
	INT8U type;	
	
	str.font_type = 0;
	str.str_idx = STR_NO_SD;
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;
	ap_state_resource_string_resolution_get(&str, &str_res);
	str.pos_x = (TFT_WIDTH - str_res.string_width) >> 1;
	str.pos_y = (TFT_HEIGHT - str_res.string_height) >> 1;
	str.font_color = 0xFFFF;

	icon.pos_x = (320-200)/2;
	icon.pos_y = (240-88)/2;
	icon.transparent = TRANSPARENT_COLOR;
	icon.icon_w = 200;
	icon.icon_h = 88;
	ap_setting_show_GPZP_file((INT8U*)"INSERTSDC.GPZP",(INT16U *)setting_frame_buff,&icon,1);

	ap_state_resource_string_draw((INT16U *)setting_frame_buff, &str, RGB565_DRAW);
	
	ap_setting_frame_buff_display();
	DBG_PRINT("no SDC show waiting...\r\n");
	type = USBD_DETECT;
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_KEY_REGISTER, &type, sizeof(INT8U), MSG_PRI_NORMAL);	
	OSTimeDly(200);
	type = GENERAL_KEY;
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_KEY_REGISTER, &type, sizeof(INT8U), MSG_PRI_NORMAL);
	DBG_PRINT("no SDC show OK\r\n");
}

void ap_setting_busy_show(void)
{
	// INT32U size, read_buf;
	// INT16U logo_fd;
	DISPLAY_ICONSHOW icon;
	t_STRING_TABLE_STRUCT str_res;
	STRING_INFO str = {0};
	// INT8U *zip_buf;
	
	str.font_type = 0;
	str.str_idx = STR_PLEASE_WAIT;
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;
	ap_state_resource_string_resolution_get(&str, &str_res);
	str.pos_x = (TFT_WIDTH - str_res.string_width) >> 1;
	str.pos_y = (TFT_HEIGHT - str_res.string_height) >> 1;
	str.font_color = 0xFFFF;

	icon.pos_x = (320-200)/2;
	icon.pos_y = (240-88)/2;
	icon.transparent = TRANSPARENT_COLOR;
	icon.icon_w = 200;
	icon.icon_h = 88;
	ap_setting_show_GPZP_file((INT8U*)"INSERTSDC.GPZP",(INT16U *)setting_frame_buff,&icon,1);
	ap_state_resource_string_draw((INT16U *)setting_frame_buff, &str, RGB565_DRAW);
	ap_setting_frame_buff_display();
}

void ap_setting_protected_show(void)
{
	// INT32U size, read_buf;
	// INT16U logo_fd;
	DISPLAY_ICONSHOW icon;
	t_STRING_TABLE_STRUCT str_res;
	STRING_INFO str = {0};
	// INT8U *zip_buf;
	
	str.font_type = 0;
	str.str_idx = STR_PROTECTED;
	str.buff_w = TFT_WIDTH;
	str.buff_h = TFT_HEIGHT;
	str.language = ap_state_config_language_get() - 1;
	ap_state_resource_string_resolution_get(&str, &str_res);
	str.pos_x = (TFT_WIDTH - str_res.string_width) >> 1;
	str.pos_y = (TFT_HEIGHT - str_res.string_height) >> 1;
	str.font_color = 0xFFFF;

	icon.pos_x = (320-200)/2;
	icon.pos_y = (240-88)/2;
	icon.transparent = TRANSPARENT_COLOR;
	icon.icon_w = 200;
	icon.icon_h = 88;
	ap_setting_show_GPZP_file((INT8U*)"INSERTSDC.GPZP",(INT16U *)setting_frame_buff,&icon,1);
	ap_state_resource_string_draw((INT16U *)setting_frame_buff, &str, RGB565_DRAW);
	ap_setting_frame_buff_display();
}

void ap_setting_del_protect_file_show(INT8U *tag, INT32U state,INT32U state1)
{

	ap_setting_protected_show();
	OSTimeDly(50);
	setting_item.stage = 0;
	ap_setting_page_draw(state,state1, tag);
				        			
}
void ap_setting_icon_draw(INT16U *frame_buff, INT16U *icon_stream, DISPLAY_ICONSHOW *icon_info, INT8U type)
{
	INT32U x, y, offset_pixel, offset_pixel_tmp, offset_data, offset_data_tmp;
	
	offset_data = 0;
	for (y=0 ; y<icon_info->icon_h ; y++) {
		offset_data_tmp = y*icon_info->icon_w;
		offset_pixel_tmp = (icon_info->pos_y + y)*TFT_WIDTH + icon_info->pos_x;
		for (x=0 ; x<icon_info->icon_w ; x++) {
			if (type) {
				offset_data = offset_data_tmp + x;
			}
			if (*(icon_stream + offset_data) == icon_info->transparent) {
				continue;
			}
			offset_pixel = offset_pixel_tmp + x;
			if (offset_pixel < TFT_WIDTH*TFT_HEIGHT) {
				if(type == SETTING_ICON_BLUE1_COLOR){
					if(*(icon_stream + offset_data) == 0xffff)
					{
						*(frame_buff + offset_pixel) = BLUE1_COLOR;		//0x001f
					}else{
						*(frame_buff + offset_pixel) = *(icon_stream + offset_data);
					}
				}else if(type == SETTING_ICON_BLUE_COLOR){
					if(*(icon_stream + offset_data) == 0xffff)
					{
						*(frame_buff + offset_pixel) = BLUE_COLOR;		//
					}else{
						*(frame_buff + offset_pixel) = *(icon_stream + offset_data);
					}
				}else{
					*(frame_buff + offset_pixel) = *(icon_stream + offset_data);
				}
			}
		}
	}
}

void ap_setting_icon_draw_tv(INT16U *frame_buff, INT16U *icon_stream, DISPLAY_ICONSHOW *icon_info, INT8U type)
{
	INT32U x, y, offset_pixel, offset_pixel_tmp, offset_data, offset_data_tmp;
	
	offset_data = 0;
	for (y=0 ; y<icon_info->icon_h ; y++) {
		offset_data_tmp = y*icon_info->icon_w;
		offset_pixel_tmp = (icon_info->pos_y + y)*TV_WIDTH + icon_info->pos_x;
		for (x=0 ; x<icon_info->icon_w ; x++) {
			if (type) {
				offset_data = offset_data_tmp + x;
			}
			if (*(icon_stream + offset_data) == icon_info->transparent) {
				continue;
			}
			offset_pixel = offset_pixel_tmp + x;
			if (offset_pixel < TV_WIDTH*TV_HEIGHT) {
				if(type == SETTING_ICON_BLUE1_COLOR){
					if(*(icon_stream + offset_data) == 0xffff)
					{
						*(frame_buff + offset_pixel) = BLUE1_COLOR;		//0x001f
					}else{
						*(frame_buff + offset_pixel) = *(icon_stream + offset_data);
					}
				}else if(type == SETTING_ICON_BLUE_COLOR){
					if(*(icon_stream + offset_data) == 0xffff)
					{
						*(frame_buff + offset_pixel) = BLUE_COLOR;		//
					}else{
						*(frame_buff + offset_pixel) = *(icon_stream + offset_data);
					}
				}else{
					*(frame_buff + offset_pixel) = *(icon_stream + offset_data);
				}
			}
		}
	}
}



void ap_setting_date_time_string_process(char * dt_str1,INT8U dt_tag)
{
	INT8U i, data_tmp, days;

	
	if(setup_date_time[6] == 0){	//YY/MM/DD
		char dt_str[] = "2000 / 00 / 00   00 : 00 : 00";
		INT8U tag[] = {2, 3, 7, 8, 12, 13, 17, 18, 22, 23, 27, 28};
		days = CalendarCalculateDays(setup_date_time[1], setup_date_time[0]);  //arg0 month,arg1 year
		if (setup_date_time[2] > days) {
			setup_date_time[2] = days;
		}
		if (dt_tag == SETTING_DATE_TIME_DRAW_ALL) {
			for (i=0 ; i<=5 ; i++) {
				data_tmp = setup_date_time[i]/10;
				*(dt_str + tag[(i<<1)]) = data_tmp + 0x30;
				data_tmp = (setup_date_time[i] - (10*data_tmp));
				*(dt_str + tag[(i<<1)+1]) = data_tmp + 0x30;
			}
		} else {
			data_tmp = setup_date_time[dt_tag]/10;
			*(dt_str + tag[(dt_tag<<1)]) = data_tmp + 0x30;
			data_tmp = (setup_date_time[dt_tag] - (10*data_tmp));
			*(dt_str + tag[(dt_tag<<1)+1]) = data_tmp + 0x30;
		}
		strncpy(dt_str1,dt_str,strlen(dt_str));
	}else if(setup_date_time[6] == 1) {	//DD/MM/YY
		char dt_str[] = "00 / 00 / 2000   00 : 00 : 00";
		INT8U tag[] = {0, 1, 5, 6, 12, 13, 17, 18, 22, 23, 27, 28};
		days = CalendarCalculateDays(setup_date_time[1], setup_date_time[2]);  //arg0 month,arg1 year
		if (setup_date_time[0] > days) {
			setup_date_time[0] = days;
		}
		if (dt_tag == SETTING_DATE_TIME_DRAW_ALL) {
			for (i=0 ; i<=5 ; i++) {
				data_tmp = setup_date_time[i]/10;
				*(dt_str + tag[(i<<1)]) = data_tmp + 0x30;
				data_tmp = (setup_date_time[i] - (10*data_tmp));
				*(dt_str + tag[(i<<1)+1]) = data_tmp + 0x30;
			}
		} else {
			data_tmp = setup_date_time[dt_tag]/10;
			*(dt_str + tag[(dt_tag<<1)]) = data_tmp + 0x30;
			data_tmp = (setup_date_time[dt_tag] - (10*data_tmp));
			*(dt_str + tag[(dt_tag<<1)+1]) = data_tmp + 0x30;
		}
		strncpy(dt_str1,dt_str,strlen(dt_str));
	}else if(setup_date_time[6] == 2) {	//MM/DD/YY
		char dt_str[] = "00 / 00 / 2000   00 : 00 : 00";
		INT8U tag[] = {0, 1, 5, 6, 12, 13, 17, 18, 22, 23, 27, 28};
		days = CalendarCalculateDays(setup_date_time[0], setup_date_time[2]);  //arg0 month,arg1 year
		if (setup_date_time[1] > days) {
			setup_date_time[1] = days;
		}
		if (dt_tag == SETTING_DATE_TIME_DRAW_ALL) {
			for (i=0 ; i<=5 ; i++) {
				data_tmp = setup_date_time[i]/10;
				*(dt_str + tag[(i<<1)]) = data_tmp + 0x30;
				data_tmp = (setup_date_time[i] - (10*data_tmp));
				*(dt_str + tag[(i<<1)+1]) = data_tmp + 0x30;
			}
		} else {
			data_tmp = setup_date_time[dt_tag]/10;
			*(dt_str + tag[(dt_tag<<1)]) = data_tmp + 0x30;
			data_tmp = (setup_date_time[dt_tag] - (10*data_tmp));
			*(dt_str + tag[(dt_tag<<1)+1]) = data_tmp + 0x30;
		}
		strncpy(dt_str1,dt_str,strlen(dt_str));
	}
}

void ap_setting_frame_buff_display(void)
{
	OSQPost(DisplayTaskQ, (void *) (setting_frame_buff | MSG_DISPLAY_TASK_SETTING_DRAW));
	if (setting_browse_frame_buff) {
		OSQPost(DisplayTaskQ, (void *) (setting_browse_frame_buff | MSG_DISPLAY_TASK_JPEG_DRAW));
	}
}

INT8U CalendarCalculateDays(INT8U month, INT8U year)
{
	INT8U DaysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31}; 
	 
	year += 2000;
	if ((year&0x3) || (!(year%100) && (year%400))) {
		return DaysInMonth[month - 1];
	} else {
		if (month == 2) {
			return 29;
		} else {
			return DaysInMonth[month - 1];
		}
	}
}

void ap_setting_value_set_from_user_config(void)
{
#ifdef	__OV7725_DRV_C__
	/*	//wwj mark
	if(ap_state_config_night_mode_get() == 1) {
		gpio_write_io(IR_CTRL, DATA_HIGH);	
	} else {
		gpio_write_io(IR_CTRL, DATA_LOW);	
	} */	
	ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_EV_data, ap_state_config_ev_get(), 4);
	
	ap_setting_sensor_command_switch(0xAC, 0x20, 1);
	ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_Sharpness, ap_state_config_sharpness_get(), 2);
	ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_Saturation, ap_state_config_saturation_get(), 4);
	if(ap_state_config_color_get() == 0){
		ap_setting_sensor_command_switch(0xA6, 0x01, 0);
	}else{
		ap_setting_sensor_command_switch(0xA6, 0x01, 1);
	}	
	ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_Color, ap_state_config_color_get(), 4);
	if(ap_state_config_white_balance_get() == 0){
		ap_setting_sensor_command_switch(0x13, 0x02, 1);
	}else{
		ap_setting_sensor_command_switch(0x13, 0x02, 0);
	}
	ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_White_Balance, ap_state_config_white_balance_get(), 6);
	if(ap_state_config_iso_get() == 0){
		ap_setting_sensor_command_switch(0x13, 0x04, 1);
	}else{
		ap_setting_sensor_command_switch(0x13, 0x04, 0);
	}	
	ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_ISO_data, ap_state_config_iso_get(), 2);
	ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_Scene_Mode, ap_state_config_scene_mode_get(), 4);
	ap_setting_sensor_sccb_cmd_set((INT8U *) ap_setting_Light_Freq, ap_state_config_light_freq_get(), 4);
#endif

	//volume
	audio_vol_set(ap_state_config_volume_get());
	
	//mic switch
	/*	// modify josephhsieh@140822    // voice record :OFF  => put silence to avi file
	if(ap_state_config_voice_record_switch_get() != 0) {
		audio_encode_audio_format_set(WAVE_FORMAT_PCM);
	} else {
		audio_encode_audio_format_set(0);
	}
	*/

	//TODO: light frequency
	
	
	//storage service del thread MB set
	ap_storage_service_del_thread_mb_set();
}

void ap_setting_sensor_sccb_cmd_set(INT8U *table_ptr, INT16U idx, INT8U cmd_num)
{
/* Use CDSP API instead of setting sensor here
	INT32U i;

	for (i=0; i<cmd_num; i+=2) {
		sccb_write(SLAVE_ID, *(table_ptr + idx*cmd_num + i), *(table_ptr + idx*cmd_num + (i+1)));
	}
*/
}

void ap_setting_sensor_command_switch(INT16U cmd_addr, INT16U reg_bit, INT8U enable)
{
/* Use CDSP API instead of setting sensor here
	INT16U temp;	

	if(enable == 1)
	{	
		temp = sccb_read(SLAVE_ID, cmd_addr);
		temp |= reg_bit;	//Enable
		sccb_write(SLAVE_ID, cmd_addr, temp);
	}else{
		temp = sccb_read(SLAVE_ID, cmd_addr);
		temp &= ~(reg_bit);	//Disable
		sccb_write(SLAVE_ID, cmd_addr, temp);
	}
*/
}



INT8U Get_cmputer_build_time(INT8U *buf);

void ap_setting_show_software_version()
{
	

//	STRING_INFO str;
	STRING_ASCII_INFO ascii_str;
//	DISPLAY_ICONSHOW cursor_icon = {320, 234, TRANSPARENT_COLOR, 0, 0};
//	INT8U cursor_offset_x[] = {0, 67, 122, 178, 232};
//	INT32U size, read_buf;
//	INT16U logo_fd;
//	INT8U *zip_buf;
	INT8U temp1[sizeof(__DATE__)];
	INT8U *date;
	INT16U i;
	DISPLAY_ICONSHOW icon;
	
	STRING_INFO sub_str = {0};
	t_STRING_TABLE_STRUCT str_res;

	icon.transparent = TRANSPARENT_COLOR;
	icon.icon_w = ICON_SELECTBOX_SUB_WIDTH;
	icon.icon_h = ICON_SELECTBOX_SUB_HEIGHT;
	icon.pos_x = ICON_SELECTBOX_SUB_START_X;	
	icon.pos_y = ICON_SELECTBOX_SUB_START_Y;

	ap_setting_show_GPZP_file((INT8U*)"SELECTBOX_SUB.GPZP",(INT16U *)setting_frame_buff,&icon,SETTING_ICON_NORMAL_DRAW);

	sub_str.font_type = 0;	
	sub_str.buff_w = TFT_WIDTH;
	sub_str.buff_h = TFT_HEIGHT;
	sub_str.language = ap_state_config_language_get() - 1;	
	sub_str.font_color = 0xFFFF;
	sub_str.str_idx = g_manu_stage;
	ap_state_resource_string_resolution_get(&sub_str, &str_res);
	sub_str.pos_x = (320 - str_res.string_width)-10;
	sub_str.pos_y = 7;
	ap_state_resource_string_draw((INT16U *)setting_frame_buff, &sub_str, RGB565_DRAW);

	date = temp1;
	Get_cmputer_build_time(date);
	
	ascii_str.font_color = 255;
	ascii_str.font_type = 0;
	ascii_str.pos_x = ICON_SELECTBOX_SUB_START_X+50;
	ascii_str.pos_y = 110;
	ascii_str.str_ptr = "20140115 V1.0 ";
	for(i=0;i<8;i++)
	{
		*(ascii_str.str_ptr+i) = *(date+i);
	}
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ap_state_resource_string_ascii_draw((INT16U *) setting_frame_buff, &ascii_str, RGB565_DRAW);	

	ap_setting_frame_buff_display();

}


INT8U Get_cmputer_build_time(INT8U *buf)
{
	INT8U temp[] = __DATE__ ;
	INT8U *temp0;
	INT8U month[4]={0};
	INT8U date[3]={0};
	INT8U year[5]={0};
	INT16U i,j,k;
	INT8U Months[12][6]={"Jan01","Feb02","Mar03","Apr04","May05","Jun06","Jul07","Aug08","Sep09","Oct10","Nov11","Dec12"};
	
	for(i=0;i<sizeof(__DATE__);i++){
		buf[i] = temp[i];
	}
	temp0 = buf;
//	print_string("BUILD TIME: "__DATE__"-"__TIME__"\r\n" );
//	print_string("data:%s\r\n", temp );
//	print_string("data0:%s\r\n", temp0 );
	for(i=0;i<3;i++)
		month[i] = *(temp0+i);
	
	for(i=0;i<2;i++)
		date[i] = *(temp0+4+i);
	
	for(i=0;i<4;i++)
		year[i] = *(temp0+7+i);
	
//	print_string("month = %s\r\n", month );
//	print_string("date = %s\r\n", date );
//	print_string("year = %s\r\n", year );
	
	for(j=0;j<12;j++)
	{
//		gp_strncmp();
		for(i=0;i<3;i++)
		{
			if(month[i] == Months[j][i])
			{
				if(i == 2)
					break;
			}
			else
			{
				i = 0;
				break;
			}
			
		}
		if(i == 2)
		{
			for(k=0;k<2;k++)
			{
				month[k] = Months[j][i+1+k];	
			}
//			print_string("month = %s\r\n", month );
			break;
		}
	}
	
	for(i=0;i<4;i++)
		*(temp0+i) = year[i];
	for(i=0;i<2;i++)
		*(temp0+4+i) = month[i];
	for(i=0;i<2;i++)
		*(temp0+6+i) = date[i];
	
		*(temp0+8) = ' ';

//	print_string("data:%s\r\n", temp );
//	print_string("data0:%s\r\n", temp0 );
	
	return 0;
}


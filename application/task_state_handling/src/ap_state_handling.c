#include "drv_l1_rtc.h"
#include "ap_state_handling.h"
#include "ap_state_config.h"
#include "ap_state_resource.h"
#include "ap_music.h"
#include "ap_display.h"
#include "avi_encoder_app.h"
#include "task_video_decoder.h"

extern void usb_disconnect_wait(void);
// Image decoding relative global definitions and variables
static INT32U jpeg_output_format;
static INT8U jpeg_output_ratio;             // 0=Fit to output_buffer_width and output_buffer_height, 1=Maintain ratio and fit to output_buffer_width or output_buffer_height, 2=Same as 1 but without scale up
static INT16U jpeg_output_buffer_width;
static INT16U jpeg_output_buffer_height;
static INT16U jpeg_output_image_width;
static INT16U jpeg_output_image_height;
static INT32U out_of_boundary_color;
static INT32U jpeg_output_buffer_pointer;

static INT8U thumbnail_exist;
static INT16U thumbnail_width;
static INT16U thumbnail_height;
static INT16U thumbnail_yuv_mode;

static INT32U jpeg_output_h_cnt, jpeg_scale_h_size;
static FP32   jpeg_scale_h_fifo, jpeg_CorrectScaleHSize;
// Control variables
static INT32U jpeg_fifo_line_num = 32; // Value is 0(Disable)/16/32/64/128/256

// JPEG variables
static INT16U jpeg_valid_width, jpeg_valid_height, jpeg_image_yuv_mode;
static INT32U jpeg_extend_width, jpeg_extend_height;

// Buffer addresses
static INT32U jpeg_out_y, jpeg_out_cb, jpeg_out_cr;
static INT32U scaler_out_buffer;

static STR_ICON sh_str;
static INT8U sh_curr_storage_id;
static INT8U File_creat_ok_temp=0;

#if C_BATTERY_DETECT == CUSTOM_ON 
	static INT8U bat_icon = 3;
	static INT8U charge_icon = 0;
#endif

#if C_POWER_OFF_LOGO == CUSTOM_ON
	static INT32S poweroff_logo_img_ptr;
	INT32U poweroff_logo_decode_buff = 0;
#endif

INT16U present_state;
extern INT8U display_str_background;
extern void date_time_force_display(INT8U force_en,INT8U postion_flag);
extern void ap_video_capture_mode_switch(INT8U DoSensorInit, INT16U EnterAPMode);

INT32S ap_state_handling_str_draw(INT16U str_index, INT16U str_color)
{
	#if GPDV_BOARD_VERSION != GPCV4247_WIFI 
	INT32U i, size;
	t_STRING_TABLE_STRUCT str_res;
	STRING_INFO str = {0};
	
	if (sh_str.addr) {
		gp_free((void *) sh_str.addr);
	}
	str.font_type = 0;
	str.str_idx = str_index;
	str.language = ap_state_config_language_get() - 1;
	if (ap_state_resource_string_resolution_get(&str, &str_res)) {
		DBG_PRINT("String resolution get fail\r\n");
		return STATUS_FAIL;
	}
	str.buff_w = sh_str.w = str_res.string_width;
	str.buff_h = sh_str.h = str_res.string_height;
	str.pos_x = str.pos_y = 0;
	str.font_color = str_color;
	size = sh_str.w*sh_str.h;
	sh_str.addr = (INT32U) gp_malloc(size << 1);
	if (!sh_str.addr) {
		DBG_PRINT("String memory malloc fail\r\n");
		return STATUS_FAIL;
	}

	for (i=0 ; i<size ; i++) {
		*((INT16U *) sh_str.addr + i) = TRANSPARENT_COLOR;
	}
	ap_state_resource_string_draw((INT16U *) sh_str.addr, &str, RGB565_DRAW);
	display_str_background = 1;
	OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_STRING_DRAW | ((INT32U) &sh_str)));
	#endif
	return STATUS_OK;
}

INT32S ap_state_handling_str_draw_HDMI(INT16U str_index, INT16U str_color)
{
	INT32U i, j, size;
	t_STRING_TABLE_STRUCT str_res;
	STRING_INFO str = {0};

	if (sh_str.addr) {
		gp_free((void *) sh_str.addr);
	}
	str.font_type = 1; //big ascii
	str.str_idx = str_index;
	str.language = LCD_EN;
	if (ap_state_resource_string_resolution_get(&str, &str_res)) {
		DBG_PRINT("String resolution get fail\r\n");
		return STATUS_FAIL;
	}
	str.buff_w = sh_str.w = (str_res.string_width+1) & 0xfffe;
	str.buff_h = sh_str.h = str_res.string_height;
	str.pos_x = str.pos_y = 0;
	str.font_color = str_color;
	size = sh_str.w*sh_str.h;
	sh_str.addr = (INT32U) gp_malloc(size << 1);
	if (!sh_str.addr) {
		DBG_PRINT("String memory malloc fail\r\n");
		return STATUS_FAIL;
	}

	for(i=0; i<str.buff_h; i++) {
		for (j=0 ; j<(str.buff_w/2) ; j++) {
			*(INT32U *)((INT32U *)(sh_str.addr + i*str.buff_w*2) + j) = TRANSPARENT_COLOR_YUYV;
		}
	}
	ap_state_resource_string_draw((INT16U *) sh_str.addr, &str, YUYV_DRAW);
	display_str_background = 1;
	OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_STRING_DRAW | ((INT32U) &sh_str)));
	return STATUS_OK;
}


void ap_state_handling_str_draw_exit(void)
{
	#if GPDV_BOARD_VERSION != GPCV4247_WIFI
	if (sh_str.addr) {
		OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_STRING_DRAW | 0xFFFFFF));
		OSTimeDly(3);
		gp_free((void *) sh_str.addr);
		sh_str.addr = 0;
	}
	#endif
	display_str_background = 0;
}


INT32S ap_state_handling_ASCII_str_draw_HDMI(STR_ICON_EXT *str_ptr, STRING_INFO *str_info)
{
	INT32U i, size;

	if (str_ptr->addr) {
		gp_free((void *) str_ptr->addr);
	}
	
	size = str_ptr->w*str_ptr->h;
	str_ptr->addr = (INT32U) gp_malloc(size << 1);
	if (!str_ptr->addr) {
		DBG_PRINT("String memory malloc fail\r\n");
		return STATUS_FAIL;
	}
	for (i=0 ; i<size/2 ; i++) {
		*(INT32U *)((INT32U *)str_ptr->addr + i) = TRANSPARENT_COLOR_YUYV;
	}
	ap_state_resource_string_draw((INT16U *) str_ptr->addr, str_info, YUYV_DRAW);
	OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_STRING_ICON_DRAW | ((INT32U) str_ptr)));
	return STATUS_OK;
}


INT32S ap_state_handling_ASCII_str_draw(STR_ICON_EXT *str_ptr,STRING_ASCII_INFO *str_ascii_info)
{
	INT32U i, size;

	if (str_ptr->addr) {
		gp_free((void *) str_ptr->addr);
	}
	
	size = str_ptr->w*str_ptr->h;
	str_ptr->addr = (INT32U) gp_malloc(size << 1);
	if (!str_ptr->addr) {
		DBG_PRINT("String memory malloc fail\r\n");
		return STATUS_FAIL;
	}
	for (i=0 ; i<size ; i++) {
		*((INT16U *) str_ptr->addr + i) = TRANSPARENT_COLOR;
	}
	ap_state_resource_string_ascii_draw((INT16U *) str_ptr->addr, str_ascii_info, RGB565_DRAW);	
	OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_STRING_ICON_DRAW | ((INT32U) str_ptr)));
	return STATUS_OK;
}

void ap_state_handling_ASCII_str_draw_exit(STR_ICON_EXT *str_ptr,INT8U wait)
{
	if (str_ptr->addr) {
		OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_STRING_ICON_CLEAR | ((INT32U) str_ptr)));
		if(wait==1){
			do
			{
				OSTimeDly(3);
			}while(str_ptr->num);
		}else{
			OSTimeDly(3);
		}
		gp_free((void *) str_ptr->addr);
		str_ptr->addr = 0;
	}
}

//If you want to draw over 3 icons, pls call this function again.
void ap_state_handling_icon_show_cmd(INT8U cmd1, INT8U cmd2, INT8U cmd3)
{
	#if 0	
	if( ((cmd1 < ICON_MP3_PLAY) || (cmd1 > ICON_MP3_STOP)) && ((cmd2 < ICON_MP3_PLAY) || (cmd2 > ICON_MP3_STOP)) && ((cmd3 < ICON_MP3_PLAY) || (cmd3 > ICON_MP3_STOP)) ){
		OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_ICON_SHOW | (cmd1 | (cmd2<<8) | (cmd3<<16))));
	}else{
		if( ((present_state == STATE_VIDEO_PREVIEW) || (present_state == STATE_VIDEO_RECORD))/* && (fm_tx_status_get() == 1)*/ ){
			OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_ICON_SHOW | (cmd1 | (cmd2<<8) | (cmd3<<16))));
		}
	}
	#endif
}

void ap_state_handling_icon_clear_cmd(INT8U cmd1, INT8U cmd2, INT8U cmd3)
{	
	#if 0
	if( ((cmd1 < ICON_MP3_PLAY) || (cmd1 > ICON_MP3_STOP)) && ((cmd2 < ICON_MP3_PLAY) || (cmd2 > ICON_MP3_STOP)) && ((cmd3 < ICON_MP3_PLAY) || (cmd3 > ICON_MP3_STOP)) ){
		OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_ICON_CLEAR | (cmd1 | (cmd2<<8) | (cmd3<<16))));
	}else{
		if( ((present_state == STATE_VIDEO_PREVIEW) || (present_state == STATE_VIDEO_RECORD))/* && (fm_tx_status_get() == 1)*/ ){
			OSQPost(DisplayTaskQ, (void *) (MSG_DISPLAY_TASK_ICON_CLEAR | (cmd1 | (cmd2<<8) | (cmd3<<16))));
		}
	}
	#endif
}
extern INT8U USB_select_entry(void *para,INT32U buff_addr);
void ap_state_handling_connect_to_pc(INT32U prev_state)
{
	INT8U type;
	//INT32U i;
	//INT32U display_frame0;
	INT8U temp;

	/*
	#if (GPDV_BOARD_VERSION != GPCV4247_WIFI)
	for(i=0; i<50; ++i) {
		display_frame0 = ap_display_queue_get(display_isr_queue);
		if (display_frame0!=NULL)
			break;
		OSTimeDly(1);
	}
	for(i=0; i<TFT_WIDTH*TFT_HEIGHT; i++) {
		*(INT16U *)(display_frame0 + i*2) = 0;
	}
	#endif
	*/
	s_usbd_pin = 1;
	
	//#if (GPDV_BOARD_VERSION != GPCV4247_WIFI)
	////temp = USB_select_entry(&prev_state,display_frame0);
	//#else
	OSTimeDly(50);
	temp = 0;
	//#endif

	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_NORMAL);
	type = FALSE;
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK_SWITCH, &type, sizeof(INT8U), MSG_PRI_URGENT);
	type = USBD_DETECT;
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_KEY_REGISTER, &type, sizeof(INT8U), MSG_PRI_NORMAL);	
    
    if (ap_state_handling_storage_id_get() == NO_STORAGE) temp = 1;
    
	if (temp == 1) {
   		ap_video_capture_mode_switch(0, STATE_CONNECT_TO_PC);
		OSQPost(USBTaskQ, (void *) MSG_USBCAM_PLUG_IN);
	} else {
		OSQPost(USBTaskQ, (void *) MSG_USBD_PLUG_IN);
	}
}

void ap_state_handling_disconnect_to_pc(void)
{
	INT8U type = GENERAL_KEY;

	s_usbd_pin = 0;
	DBG_PRINT("-----usb disconnect to pc-----\r\n");
	usb_disconnect_wait();

	if (ap_state_config_usb_mode_get() == 1) {
		msgQSend(StorageServiceQ, MSG_STORAGE_USBD_PCAM_EXIT, NULL, NULL, MSG_PRI_NORMAL);
	} else {
		msgQSend(StorageServiceQ, MSG_STORAGE_USBD_EXIT, NULL, NULL, MSG_PRI_NORMAL);
	}
	
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_KEY_REGISTER, &type, sizeof(INT8U), MSG_PRI_NORMAL);
}

void ap_state_handling_storage_id_set(INT8U stg_id)
{
	sh_curr_storage_id = stg_id;
}

INT8U ap_state_handling_storage_id_get(void)
{
	return sh_curr_storage_id;
}
void ap_state_handling_file_creat_set(INT8U id)
{
	File_creat_ok_temp = id;

}
INT8U ap_state_handling_file_creat_get(void)
{
    return File_creat_ok_temp;
}
/*
void ap_state_handling_led_on(void)
{	
	INT8U type = DATA_HIGH;
	
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT8U), MSG_PRI_NORMAL);
}

void ap_state_handling_led_off(void)
{	
	INT8U type = DATA_LOW;
	
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT8U), MSG_PRI_NORMAL);
}

void ap_state_handling_led_flash_on(void)
{	
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_FLASH_SET, NULL, NULL, MSG_PRI_NORMAL);
}

void ap_state_handling_led_blink_on(void)
{	
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_BLINK_SET, NULL, NULL, MSG_PRI_NORMAL);
}
*/
extern void sys_power_handler(void);
extern drvl2_sd_card_remove(void);
extern void system_da_ad_pll_en_set(BOOLEAN);
extern void ap_display_string_icon_clear_all(void);

void ap_state_handling_clear_all_icon(void)
{
	ap_display_string_icon_clear_all();
	date_time_force_display(0,0);
	OSQPost(DisplayTaskQ, (void *) MSG_DISPLAY_TASK_EFFECT_INIT);
}

INT8U display_str_battery_low = 0;
#ifdef PWM_CTR_LED 
extern void ap_peripheral_PWM_LED_high(void);
extern void ap_peripheral_PWM_OFF(void);
#endif

void ext_rtc_alarm_isr_callback(void);
void ext_rtc_wakeup_isr_callback(void);

void ap_state_handling_power_off(INT32U wakeup_flag)
{
#if C_POWER_OFF_LOGO == CUSTOM_ON
	INT32U  size;
	INT16U  logo_fd;
	INT32U  cnt;
	INT32S  status;
	
	if (sys_pwr_key1_read()) return; //连接USB供电时不需要关机
    	//gpio_write_io(SPEAKER_EN, 1);	   // turn on audio
	if(!wakeup_flag) {
		ap_state_handling_clear_all_icon();
		audio_effect_play(EFFECT_POWER_OFF);
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_BATTERY_LOW_BLINK_STOP, NULL, NULL, MSG_PRI_NORMAL);
		OSTimeDly(10);
		logo_fd = nv_open((INT8U *) "POWER_OFF_LOGO.JPG");
		if (logo_fd != 0xFFFF) {
			do {
				DBG_PRINT("Q");
				poweroff_logo_decode_buff = ap_display_queue_get(display_isr_queue);
				OSTimeDly(10);
			} while(!poweroff_logo_decode_buff);

			size = nv_rs_size_get(logo_fd);
			poweroff_logo_img_ptr = (INT32S) gp_malloc(size);
			if(poweroff_logo_img_ptr) {
				if (nv_read(logo_fd, (INT32U) poweroff_logo_img_ptr, size) == 0) {
					if(ap_display_get_device()==DISP_DEV_HDMI) { //HDMI
					  #ifdef HDMI_JPG_DECODE_AS_GP420				
						status = video_decode_jpeg_as_gp420((INT8U *)poweroff_logo_img_ptr, size, 1280, 720, poweroff_logo_decode_buff);
					  #elif defined(HDMI_JPG_DECODE_AS_YUV422)
	                    status = video_decode_jpeg_as_gp422((INT8U *)poweroff_logo_img_ptr, size, 1280, 720, poweroff_logo_decode_buff);
					  #endif					
					} else {
						IMAGE_DECODE_STRUCT img_info;

						img_info.image_source = (INT32S) poweroff_logo_img_ptr;
						img_info.source_size = size;
						img_info.source_type = TK_IMAGE_SOURCE_TYPE_BUFFER;
						img_info.output_format = C_SCALER_CTRL_OUT_RGB565;
						img_info.output_ratio = 0;
						img_info.out_of_boundary_color = 0x008080;

						if(ap_display_get_device()==DISP_DEV_TFT) { //TFT
							img_info.output_buffer_width = TFT_WIDTH;
							img_info.output_buffer_height = TFT_HEIGHT;
							img_info.output_image_width = TFT_WIDTH;
							img_info.output_image_height = TFT_HEIGHT;
						} else { //TV
							img_info.output_buffer_width = TV_WIDTH;
							img_info.output_buffer_height = TV_HEIGHT;
							img_info.output_image_width = TV_WIDTH;
							img_info.output_image_height = TV_HEIGHT;
						}
						img_info.output_buffer_pointer = poweroff_logo_decode_buff;
						status = jpeg_buffer_decode_and_scale(&img_info);
					}

					if(status != STATUS_FAIL) {
						if(ap_display_get_device()==DISP_DEV_HDMI) { //HDMI
							OSQPost(DisplayTaskQ, (void *) (poweroff_logo_decode_buff|MSG_DISPLAY_TASK_HDMI_MJPEG_DRAW));
						} else {
							OSQPost(DisplayTaskQ, (void *) (poweroff_logo_decode_buff|MSG_DISPLAY_TASK_MJPEG_DRAW));
						}
						OSTimeDly(100);
					} else {
						DBG_PRINT("Poweroff decode jpeg file fail.\r\n");
					}
				} else {
					DBG_PRINT("Failed to read resource_header in Poweroff\r\n");
				}
				gp_free((void *) poweroff_logo_img_ptr);
			} else {
				DBG_PRINT("Poweroff allocate jpeg input buffer fail.[%d]\r\n", size);
			}
		}
		if(display_str_battery_low){
			display_str_battery_low = 0;
			ap_state_handling_str_draw_exit();
			ap_state_handling_str_draw(STR_BATTERY_LOW, WARNING_STR_COLOR);
		}
		cnt = 0;
		if(ap_state_config_beep_sound_get()==0) {
			OSTimeDly(200);
		} else {
			while(dac_dma_status_get() == 1) {
				OSTimeDly(2);
				if (cnt++ >= 250) { //5 sec
				    break;
				}
			}
		}
	}
#endif
	DBG_PRINT("State handling power off...\r\n");
	
	if(Wifi_Disconnect() == 0)
	{	
		if (Wifi_State_Get() != 0)
		{					
			Wifi_State_Set(0);
		}
	}
	
	//Wifi_mode_shutdown();
	//gpio_write_io(WIFI_LDO_EN, DATA_LOW);	// disable Wi-Fi
	//gpio_write_io(SPEAKER_EN, DATA_LOW); //turn off local speaker before dac_disable();
	if(ap_display_get_device()==DISP_DEV_HDMI) { //HDMI
		ap_state_handling_hdmi_uninit();
	}
#if ENABLE_CHECK_RTC == 1
	ap_state_config_data_time_power_off_check();
	ap_state_config_data_time_save_set();
#endif
	if(wakeup_flag)
	{
		t_rtc time, alarm_time;

		ext_rtc_set_reliable(0x55);
		ext_rtc_time_get(&time);
		DBG_PRINT("start - %d: %d: %d: \r\n", time.rtc_hour, time.rtc_min, time.rtc_sec);

		time.rtc_sec += 5;  //wakeup after delay 5s since power off here
		if(time.rtc_sec >= 60) {
			time.rtc_min += 1;
			if(time.rtc_min >= 60) {
				time.rtc_hour += 1;
				if(time.rtc_hour >= 24) {
					time.rtc_day += 1;
				}
			}
		}

		alarm_time.rtc_day = time.rtc_day;
		alarm_time.rtc_hour = time.rtc_hour;
		alarm_time.rtc_min = time.rtc_min;
		alarm_time.rtc_sec = time.rtc_sec;

	    ext_rtc_alarm_enable(&alarm_time, 1, ext_rtc_alarm_isr_callback);
	    ext_rtc_wakeup_int_enable(1, ext_rtc_wakeup_isr_callback);
	} else {
	    ext_rtc_alarm_enable(NULL, 0, NULL);
	    ext_rtc_wakeup_int_enable(0, NULL);
	}

  #if _DRV_L1_GSENSOR == 1
	{
		INT8U gsensor_level;
	
		gsensor_level = ap_state_config_park_mode_G_sensor_get();
	    G_Sensor_park_mode_init(gsensor_level);
		
    }
  #endif
	ap_state_wifi_ssid_password_save_set();
	ap_state_config_store_power_off();
	DBG_PRINT("POWER OFF CONFIG STORE!\r\n");
	R_INT_GMASK = 1;
	tft_backlight_en_set(FALSE);
	extab_enable_set(EXTA,FALSE);
	
	video_encode_sensor_stop();
	rtc_int_set(GPX_RTC_HR_IEN|GPX_RTC_MIN_IEN|GPX_RTC_SEC_IEN|GPX_RTC_HALF_SEC_IEN,
               (RTC_DIS&GPX_RTC_HR_IEN)|(RTC_DIS&GPX_RTC_MIN_IEN)|(RTC_DIS&GPX_RTC_SEC_IEN)|(RTC_DIS&GPX_RTC_HALF_SEC_IEN));
 	rtc_schedule_disable();
//	ppu_init();
	dac_disable();
	spi_disable(SPI_0);
	adc_vref_enable_set(FALSE);
	system_da_ad_pll_en_set(FALSE);
	drvl2_sd_card_remove();
	/* set SD data pin to low */
	/*
	gpio_write_io(IO_C6, 0);
    gpio_write_io(IO_C8, 0);
    gpio_write_io(IO_C9, 0);
	*/
	timer_stop(0); /* stop timer0 */
    timer_stop(1); /* stop timer1 */
    timer_stop(2); /* stop timer2 */
    timer_stop(3); /* stop timer2 */
    time_base_stop(0); /* stop timebase A */
    time_base_stop(1);
    time_base_stop(2); /* stop timebase C */

	if ( PWR_KEY_TYPE == READ_FROM_GPIO)
	{
		while ( gpio_read_io(PW_KEY) );
	}
	else
  	{
		if (PW_KEY==PWR_KEY0) while(sys_pwr_key0_read());
		else if (PW_KEY==PWR_KEY1) while(sys_pwr_key1_read());
  	}

	tft_tft_en_set(FALSE);

/* 秨 Wi-Fi ノ display buffer 琌ぃ遏 睲0旧璓籠奔癘拘砰
	if(ap_display_get_device()==DISP_DEV_TFT) {
		INT32U *buff_ptr, i, buff_size;

		buff_ptr = (INT32U *)display_frame[0];
		buff_size = getDispDevBufSize();
		buff_size *= DISPLAY_BUF_NUM;
		buff_size >>= 2;
		for (i=0; i<buff_size; i++) {
			*buff_ptr++ = 0;
		}
	}
*/

	sys_weak_6M_set(TRUE);
	cache_disable();
	//system_set_pll(6);

	sys_power_handler();
}


void ext_rtc_alarm_isr_callback(void)
{
    DBG_PRINT("Ext RTC Alarm interrupt\r\n");
}

void ext_rtc_wakeup_isr_callback(void)
{
    DBG_PRINT("Ext RTC wakeup interrupt\r\n");
}

#if C_SCREEN_SAVER == CUSTOM_ON
void ap_state_handling_lcd_backlight_switch(INT8U enable)
{
	INT8U type;
	
	if(enable){
		type = BL_ON;
	}else{
		type = BL_OFF;
	}	
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LCD_BACKLIGHT_SET, &type, sizeof(INT8U), MSG_PRI_NORMAL);
}
#endif

static INT8U  night_mode_status = 0;
void ap_state_handling_night_mode_switch(void)
{
	if(night_mode_status) {
		night_mode_status = 0;
	} else {
		night_mode_status = 1;
	}

/*	if(night_mode_status) {
		ap_state_handling_icon_show_cmd(ICON_NIGHT_MODE_ENABLED, NULL, NULL);
		ap_state_handling_icon_clear_cmd(ICON_NIGHT_MODE_DISABLED, NULL, NULL);
	} else {
		ap_state_handling_icon_show_cmd(ICON_NIGHT_MODE_DISABLED, NULL, NULL);
		ap_state_handling_icon_clear_cmd(ICON_NIGHT_MODE_ENABLED, NULL, NULL);
	}
*/	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_NIGHT_MODE_SET, &night_mode_status, sizeof(INT8U), MSG_PRI_NORMAL);
}


INT8U ap_state_handling_night_mode_get(void)
{
	return night_mode_status;
}

INT32S ap_state_handling_jpeg_decode(STOR_SERV_PLAYINFO *info_ptr, INT32U jpg_output_addr)
{
	INT32U input_buff=0, size, shift_byte, data_tmp;
	INT32S status;
	SCALER_MAS scaler_mas;

	if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG) { // JPEG
		size = info_ptr->file_size;
		shift_byte = 0;
		lseek(info_ptr->file_handle, shift_byte, SEEK_SET);
	} else { // AVI

		shift_byte = 4096;//272; //modified by wwj
		lseek(info_ptr->file_handle, shift_byte, SEEK_SET);
		if (read(info_ptr->file_handle, (INT32U) &data_tmp, 4) != 4) {
			return STATUS_FAIL;
		}

		read(info_ptr->file_handle, (INT32U) &size, 4);
		if ((data_tmp == AUDIO_STREAM) || (size == 0)) {
			do {
				shift_byte += (size + 8);
				lseek(info_ptr->file_handle, shift_byte, SEEK_SET);
				read(info_ptr->file_handle, (INT32U) &data_tmp, 4);
				read(info_ptr->file_handle, (INT32U) &size, 4);
			} while ((data_tmp == AUDIO_STREAM) || (data_tmp == JUNK_DATA) || (size == 0));
		} else if (data_tmp != VIDEO_STREAM) {
			return STATUS_FAIL;
		}
		
    	input_buff = (INT32U) gp_malloc(size);
    	if (!input_buff) {
    		DBG_PRINT("State browse allocate jpeg input buffer fail.[%d]\r\n", size);
    		return STATUS_FAIL;
    	}
    	if (read(info_ptr->file_handle, input_buff, size) <= 0) {
    		gp_free((void *) input_buff);
    		DBG_PRINT("State browse read jpeg file fail.\r\n");
    		return STATUS_FAIL;
    	}		
	}
	
	gp_memset((INT8S *) &scaler_mas, 0, sizeof(SCALER_MAS));
	scaler_mas.mas_0 = MAS_EN_READ|MAS_EN_WRITE;
	scaler_mas_set(SCALER_1, &scaler_mas);

	if (ap_display_get_device()==DISP_DEV_HDMI) {		// HDMI
#ifdef HDMI_JPG_DECODE_AS_GP420	
	    if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG) 
    		status = video_decode_jpeg_as_gp420_by_piece(info_ptr->file_handle, size, 1280, 720, jpg_output_addr);
    	else
    	    status = video_decode_jpeg_as_gp420((INT8U *) input_buff, size, 1280, 720, jpg_output_addr);
#elif defined(HDMI_JPG_DECODE_AS_YUV422)
	    if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG) 
    		status = video_decode_jpeg_as_gp422_by_piece(info_ptr->file_handle, size, 1280, 720, jpg_output_addr);
    	else
    	    status = video_decode_jpeg_as_gp422((INT8U *) input_buff, size, 1280, 720, jpg_output_addr);
#endif
	} else if (ap_display_get_device()==DISP_DEV_TV) {	// TV, RGB565
	    if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG) 
	        status = video_decode_jpeg_as_rgb565_by_piece(info_ptr->file_handle, size, TV_WIDTH, TV_HEIGHT, jpg_output_addr);
	    else
		    status = video_decode_jpeg_as_rgb565((INT8U *) input_buff, size, TV_WIDTH, TV_HEIGHT, jpg_output_addr);
		
	} else {	// DISP_DEV_TFT, RGB565
	    if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG) 
	        status = video_decode_jpeg_as_rgb565_by_piece(info_ptr->file_handle, size, TFT_WIDTH, TFT_HEIGHT, jpg_output_addr);
	    else
		    status = video_decode_jpeg_as_rgb565((INT8U *) input_buff, size, TFT_WIDTH, TFT_HEIGHT, jpg_output_addr);
	}

    if (input_buff)
    	gp_free((void *) input_buff);

	if (status) {
		return STATUS_FAIL;
	}
	return STATUS_OK;
}

INT32S ap_state_handling_jpeg_decode_thumbnail(STOR_SERV_PLAYINFO *info_ptr, INT32U jpg_output_addr)
{
	INT32U input_buff=0, size, shift_byte, data_tmp;
	INT32S status;
	SCALER_MAS scaler_mas;

	if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG) { // JPEG
		size = info_ptr->file_size;
		shift_byte = 0;
		lseek(info_ptr->file_handle, shift_byte, SEEK_SET);
	} else { // AVI

		shift_byte = 4096;//272; //modified by wwj
		lseek(info_ptr->file_handle, shift_byte, SEEK_SET);
		if (read(info_ptr->file_handle, (INT32U) &data_tmp, 4) != 4) {
			return STATUS_FAIL;
		}

		read(info_ptr->file_handle, (INT32U) &size, 4);
		if ((data_tmp == AUDIO_STREAM) || (size == 0)) {
			do {
				shift_byte += (size + 8);
				lseek(info_ptr->file_handle, shift_byte, SEEK_SET);
				read(info_ptr->file_handle, (INT32U) &data_tmp, 4);
				read(info_ptr->file_handle, (INT32U) &size, 4);
			} while ((data_tmp == AUDIO_STREAM) || (data_tmp == JUNK_DATA) || (size == 0));
		} else if (data_tmp != VIDEO_STREAM) {
			return STATUS_FAIL;
		}
		
    	input_buff = (INT32U) gp_malloc(size);
    	if (!input_buff) {
    		DBG_PRINT("State browse allocate jpeg input buffer fail.[%d]\r\n", size);
    		return STATUS_FAIL;
    	}
    	if (read(info_ptr->file_handle, input_buff, size) <= 0) {
    		gp_free((void *) input_buff);
    		DBG_PRINT("State browse read jpeg file fail.\r\n");
    		return STATUS_FAIL;
    	}		
	}
	
	gp_memset((INT8S *) &scaler_mas, 0, sizeof(SCALER_MAS));
	scaler_mas.mas_0 = MAS_EN_READ|MAS_EN_WRITE;
	scaler_mas_set(SCALER_1, &scaler_mas);
	
    if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG)
    { 
		status = video_decode_jpeg_as_yuv422_by_piece(info_ptr->file_handle, size, 320, 240, jpg_output_addr);
	}
	else
	{
	    status = video_decode_jpeg_as_yuv422((INT8U *) input_buff, size, 320, 240, jpg_output_addr);
	}

    if (input_buff)
    	gp_free((void *) input_buff);

	if (status) {
		return STATUS_FAIL;
	}
	return STATUS_OK;
}

INT32S scaler_set_parameters(void)
{
	FP32   N;
	INT32U factor;

	jpeg_output_h_cnt = 0;
	jpeg_scale_h_size = 0;
	jpeg_scale_h_fifo = 0;
	jpeg_CorrectScaleHSize = 0;
	if (jpeg_output_buffer_pointer) {
	    scaler_out_buffer = jpeg_output_buffer_pointer;
	} else {
	    scaler_out_buffer = (INT32U) gp_malloc((jpeg_output_buffer_width*jpeg_output_buffer_height)<<1);
	    if (!scaler_out_buffer) {
		    return -1;
		}
	}
	
	if (jpeg_output_image_width && jpeg_output_image_height) {
		goto __exit;
	} else {
		if (!jpeg_output_image_width) {
	        jpeg_output_image_width = jpeg_output_buffer_width;
	    }
	    if (!jpeg_output_image_height) {
	        jpeg_output_image_height = jpeg_output_buffer_height;
	    }
		
		if (jpeg_output_ratio == 0x0) {      // Fit to output buffer width and height
			goto __exit; 
		} else if (jpeg_output_ratio==2 && jpeg_valid_width<=jpeg_output_image_width && jpeg_valid_height<=jpeg_output_image_height) {
			jpeg_output_image_width = jpeg_valid_width;
    		jpeg_output_image_height = jpeg_valid_height;
		} else {						// Fit to output buffer width or height
			if (jpeg_output_image_height*jpeg_valid_width > jpeg_output_image_width*jpeg_valid_height) {
      			factor = (jpeg_valid_width<<16)/jpeg_output_image_width;
      			jpeg_output_image_height = (jpeg_valid_height<<16)/factor;
      		} else {
      			factor = (jpeg_valid_height<<16)/jpeg_output_image_height;
      			jpeg_output_image_width = (jpeg_valid_width<<16)/factor;
      		}
		}
	}
	
__exit:
	/* compute jpeg decode times */
	N = (FP32)jpeg_valid_height / (FP32)jpeg_fifo_line_num;
	
	/* compute scaler fifo output height with twice jpeg decode */
	jpeg_scale_h_fifo = (jpeg_output_image_height << 1) / N;
	return 0;
}

INT32S scaler_fifo_once(void)
{
	static BOOLEAN bTwiceFinal;
	BOOLEAN bFinal;
	INT32S scaler_status;
	INT32S temp, N;
	INT32S jpeg_valid_h;
	INT32S output_h, output_buf_h;
	INT32U output_addr;
	FP32   remain;
	
	/* add jpeg fifo decode count */ 
	jpeg_output_h_cnt += jpeg_fifo_line_num;
	N = jpeg_output_h_cnt / jpeg_fifo_line_num;
	
	if(jpeg_output_h_cnt >= jpeg_valid_height) {
		/* fine truning jpeg decode output height */
		bFinal = TRUE;
		if(jpeg_image_yuv_mode == C_JPG_CTRL_YUV422 ||
			jpeg_image_yuv_mode == C_JPG_CTRL_YUV422V) {
			goto __jpeg_scale_once;
		}
		
		/* check jpeg decode twice finsih */
		remain = (FP32)jpeg_valid_height / (FP32)jpeg_fifo_line_num;
		temp = remain;
		remain -= temp;
		
		if((temp % 2) && remain) {
			goto __jpeg_scale_once;
		}
		
		if((temp % 2) == 0) {
			goto __jpeg_scale_once;
		}
		
		if(bTwiceFinal == 0) {
			bTwiceFinal = 1;
			jpeg_output_h_cnt -= jpeg_fifo_line_num;
			return C_SCALER_STATUS_DONE;
		} else {
			goto __jpeg_scale_once;	
		} 
	} else if((N % 2) == 0) {
		/* jpeg decode twice and scale once */ 
		bFinal = FALSE;
		bTwiceFinal = FALSE;
	} else {
		return C_SCALER_STATUS_DONE;
	}

__jpeg_scale_once:	
	/* cal output address, for YUYV, RGRB or RGB565 */ 
	output_addr = scaler_out_buffer + jpeg_output_buffer_width * jpeg_scale_h_size * 2;
	
	/* check final scale output height */
	if(bFinal) {
		temp = jpeg_valid_height - (jpeg_output_h_cnt - jpeg_fifo_line_num);
		if(temp > 0) {
			if(N % 2) {
				N -= 1;
				jpeg_valid_h = temp;
			} else {
				N -= 2;
				jpeg_valid_h = temp + jpeg_fifo_line_num;
			}
		} else {
			return C_SCALER_STATUS_DONE;
		}
		
		/* set the remainder height to output_h and output_buf_h */
		output_h = jpeg_output_image_height - jpeg_scale_h_size;
		output_buf_h = jpeg_output_buffer_height - jpeg_scale_h_size;
	} else {
		jpeg_valid_h = jpeg_fifo_line_num << 1;
		
		jpeg_CorrectScaleHSize += jpeg_scale_h_fifo;
		jpeg_scale_h_size += (INT32U)jpeg_scale_h_fifo;
		
		output_h = (INT32S)jpeg_scale_h_fifo;
		output_buf_h = (INT32S)jpeg_scale_h_fifo;
		
		//adject offset
		temp = jpeg_CorrectScaleHSize - jpeg_scale_h_size;
		if(temp > 0) {
			jpeg_scale_h_size += temp;
			output_h += temp;
			output_buf_h += temp;
		}
	}
	
	scaler_input_pixels_set(UNDEFINE_SCALER, jpeg_extend_width, jpeg_valid_h);
	scaler_input_visible_pixels_set(UNDEFINE_SCALER, jpeg_valid_width, jpeg_valid_h);				
	scaler_output_pixels_set(UNDEFINE_SCALER, (jpeg_valid_width<<16) / jpeg_output_image_width, 
							(jpeg_valid_h<<16) / output_h, 
							jpeg_output_buffer_width, 
							output_buf_h);	
							
//	scaler_input_addr_set(jpeg_out_y, NULL, NULL);
   	scaler_output_addr_set(UNDEFINE_SCALER, output_addr, NULL, NULL);  	
   	scaler_fifo_line_set(UNDEFINE_SCALER, C_SCALER_CTRL_IN_FIFO_DISABLE);
	scaler_YUV_type_set(UNDEFINE_SCALER, C_SCALER_CTRL_TYPE_YCBCR);
	scaler_input_format_set(UNDEFINE_SCALER, C_SCALER_CTRL_IN_YUYV);
	scaler_output_format_set(UNDEFINE_SCALER, jpeg_output_format);
	scaler_out_of_boundary_mode_set(UNDEFINE_SCALER, 1);	
	scaler_out_of_boundary_color_set(UNDEFINE_SCALER, out_of_boundary_color);
	
	while(1)
	{
		scaler_status = scaler_wait_idle(UNDEFINE_SCALER);
	  	if (scaler_status == C_SCALER_STATUS_STOP) {
			if (scaler_start(UNDEFINE_SCALER)) {
				DBG_PRINT("[%s, %d]:Failed to call scaler_start\r\n", __func__, __LINE__);
				break;
			}
	  	} else if (scaler_status & C_SCALER_STATUS_DONE) {
	  		// Scaler might finish its job before JPEG does when image is zoomed in.
	  		scaler_stop(UNDEFINE_SCALER);
	  		break;
	  	} else if (scaler_status & (C_SCALER_STATUS_TIMEOUT|C_SCALER_STATUS_INIT_ERR)) {
			DBG_PRINT("[%s, %d]:Scaler failed to finish its job\r\n", __func__, __LINE__);
			break;
	  	} else {
	  		DBG_PRINT("[%s, %d]:Un-handled Scaler status!\r\n", __func__, __LINE__);
	  		break;
	  	}
  	}
  	
  	return scaler_status;
}

INT32S jpeg_buffer_decode_and_scale(IMAGE_DECODE_STRUCT *img_decode_struct)
{
	INT32S parse_status;
	INT8U *p_vlc;
	INT32U left_len;
  	INT32S jpeg_status;
  	INT32S scaler_status;
	INT8U scaler_done;
	SCALER_MAS scaler0_mas;

	if (!img_decode_struct) {
		return STATUS_FAIL;
	}
	
    jpeg_output_format = img_decode_struct->output_format;
    jpeg_output_ratio = 0;
    jpeg_output_buffer_width = img_decode_struct->output_buffer_width;
    jpeg_output_buffer_height = img_decode_struct->output_buffer_height;
    jpeg_output_image_width = img_decode_struct->output_image_width;
    jpeg_output_image_height = img_decode_struct->output_image_height;
    out_of_boundary_color = img_decode_struct->out_of_boundary_color;
    jpeg_output_buffer_pointer = img_decode_struct->output_buffer_pointer;

	// Initiate software header parser and hardware engine
	jpeg_decode_init();

	parse_status = jpeg_decode_parse_header((INT8U *) img_decode_struct->image_source, img_decode_struct->source_size);
    if (parse_status != JPEG_PARSE_OK) {
		DBG_PRINT("Parse header failed. Skip this file\r\n");
		img_decode_struct->decode_status = STATUS_FAIL;

        return STATUS_FAIL;
	}

	jpeg_valid_width = jpeg_decode_image_width_get();
	jpeg_valid_height = jpeg_decode_image_height_get();
	jpeg_image_yuv_mode = jpeg_decode_image_yuv_mode_get();
	jpeg_extend_width = jpeg_decode_image_extended_width_get();
	jpeg_extend_height = jpeg_decode_image_extended_height_get();

	thumbnail_exist = jpeg_decode_thumbnail_exist_get();		// 0=No thumbnail image, 1=Thumbnail image exists
	thumbnail_width = jpeg_decode_thumbnail_width_get();
	thumbnail_height = jpeg_decode_thumbnail_height_get();
	thumbnail_yuv_mode = jpeg_decode_thumbnail_yuv_mode_get();

	if(thumbnail_exist && jpeg_image_yuv_mode == C_JPG_CTRL_YUV422V) {
		scaler_status = jpeg_extend_width / 8;
		if(scaler_status % 2) {	//odd
			thumbnail_exist = 1;	//GPL32705 YUV422V Width/8 is ODD must use thumbnail to decode. 
		} else {	//even
			thumbnail_exist = 0;
		}
	} else {
		thumbnail_exist = 0; 	//force to no thumbnail image
	}
	
	if(thumbnail_exist) {

		jpeg_decode_init();
		jpeg_decode_thumbnail_image_enable();
	   	parse_status = jpeg_decode_parse_header((INT8U *) img_decode_struct->image_source, img_decode_struct->source_size);
		if (parse_status != JPEG_PARSE_OK) {
			DBG_PRINT("Parse header failed. Skip this file\r\n");
			img_decode_struct->decode_status = STATUS_FAIL;

		  	return STATUS_FAIL;
		}

	    jpeg_valid_width = thumbnail_width;
		jpeg_valid_height = thumbnail_height;
		jpeg_image_yuv_mode = thumbnail_yuv_mode;
		jpeg_extend_width = jpeg_decode_thumbnail_extended_width_get();
		jpeg_extend_height = jpeg_decode_thumbnail_extended_height_get();

	} else if (jpeg_decode_image_progressive_mode_get()) {
	  	jpeg_status = STATUS_FAIL;
		img_decode_struct->decode_status = jpeg_status;

		return jpeg_status;

	} 
	
	// Set Jpeg H/W pre scaledown.
//	jpeg_pre_scaledown_set();
	//jpeg_using_scaler_mode_enable();
	
	jpeg_memory_allocate(jpeg_fifo_line_num); // 32
	if (!jpeg_out_y) {
		DBG_PRINT("Failed to allocate memory in jpeg_memory_allocate()\r\n");
		img_decode_struct->decode_status = -2;

		return -2;
	}

	if (jpeg_decode_output_set(jpeg_out_y, jpeg_out_cb, jpeg_out_cr, jpeg_fifo_line_num)) {
		DBG_PRINT("Failed to call jpeg_decode_output_set()\r\n");
		img_decode_struct->decode_status = STATUS_FAIL;
	  	gp_free((void *) jpeg_out_y);
	  	if (jpeg_image_yuv_mode != C_JPG_CTRL_GRAYSCALE) {
	  		gp_free((void *) jpeg_out_cb);
	  		gp_free((void *) jpeg_out_cr);
	  	}

		return STATUS_FAIL;
	}

	//p_vlc = NULL; //jpeg_decode_image_vlc_addr_get();
	p_vlc = (INT8U *)jpeg_decode_image_vlc_addr_get();
  	if (((INT32U) p_vlc) >= (img_decode_struct->image_source+img_decode_struct->source_size)) {
  		DBG_PRINT("VLC address exceeds file range\r\n");
  		img_decode_struct->decode_status = STATUS_FAIL;
	  	gp_free((void *) jpeg_out_y);
	  	if (jpeg_image_yuv_mode != C_JPG_CTRL_GRAYSCALE) {
	  		gp_free((void *) jpeg_out_cb);
	  		gp_free((void *) jpeg_out_cr);
	  	}

        return STATUS_FAIL;
  	}
	left_len = img_decode_struct->source_size ; //- (((INT32U) p_vlc) - img_decode_struct->image_source);
	jpeg_decode_vlc_maximum_length_set(left_len);

	// Now start JPEG decoding
	if (jpeg_decode_once_start(p_vlc, left_len)) {
		DBG_PRINT("Failed to call jpeg_decode_once_start()\r\n");
		img_decode_struct->decode_status = STATUS_FAIL;
	  	if (jpeg_image_yuv_mode != C_JPG_CTRL_GRAYSCALE) {
	  		gp_free((void *) jpeg_out_cb);
	  		gp_free((void *) jpeg_out_cr);
	  	}
	  	gp_free((void *) jpeg_out_y);

        return STATUS_FAIL;
  	}

  	// Initiate Scaler
  	scaler_init(MJ_DECODE_SCALER);

	gp_memset((INT8S*)&scaler0_mas,0,sizeof(SCALER_MAS));
	scaler0_mas.mas_0 = MAS_EN_READ|MAS_EN_WRITE;
	scaler_mas_set(SCALER_0,&scaler0_mas);
	
	scaler_done = 0;
	// Setup Scaler
	if(jpeg_fifo_line_num != 0) {
		jpeg_scaler_set_parameters(jpeg_fifo_line_num);
	} else { 
		scaler_set_parameters();
	}
  	if (!scaler_out_buffer) {
		jpeg_decode_stop();
		DBG_PRINT("Failed to allocate scaler_out_buffer\r\n");
		img_decode_struct->decode_status = STATUS_FAIL;
	  	gp_free((void *) jpeg_out_y);
	  	if (jpeg_image_yuv_mode != C_JPG_CTRL_GRAYSCALE) {
	  		gp_free((void *) jpeg_out_cb);
	  		gp_free((void *) jpeg_out_cr);
	  	}

		return STATUS_FAIL;
  	}

	while (1) {
		jpeg_status = jpeg_decode_status_query(1);

		if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {
		  	// Wait until scaler finish its job
		  	if(jpeg_fifo_line_num != 0) { // Frame Mode
		  	while (!scaler_done) {
		  		scaler_status = scaler_wait_idle(MJ_DECODE_SCALER);
		  		if (scaler_status == C_SCALER_STATUS_STOP) {
					if (scaler_start(MJ_DECODE_SCALER)) {
						DBG_PRINT("Failed to call scaler_start\r\n");
						break;
					}
				} else if (scaler_status & C_SCALER_STATUS_DONE) {
					break;
				} else if (scaler_status & (C_SCALER_STATUS_TIMEOUT|C_SCALER_STATUS_INIT_ERR)) {
					DBG_PRINT("Scaler failed to finish its job\r\n");
					break;
				} else if (scaler_status & C_SCALER_STATUS_INPUT_EMPTY) {
		  			if (scaler_restart(MJ_DECODE_SCALER)) {
						DBG_PRINT("Failed to call scaler_restart\r\n");
						break;
					}
		  		} else {
			  		DBG_PRINT("Un-handled Scaler status!\r\n");
			  		break;
			  	}
		  	}
			} else {
				scaler_status = scaler_fifo_once();
			}
			break;
		}		// if (jpeg_status & C_JPG_STATUS_DECODE_DONE)

		if (jpeg_status & C_JPG_STATUS_OUTPUT_FULL) {
		  	// Start scaler to handle the full output FIFO now
		  	if(jpeg_fifo_line_num != 0) { // Frame Mode
		  	if (!scaler_done) {
				  	scaler_status = scaler_wait_idle(MJ_DECODE_SCALER);
			  	if (scaler_status == C_SCALER_STATUS_STOP) {
						if (scaler_start(MJ_DECODE_SCALER)) {
						DBG_PRINT("Failed to call scaler_start\r\n");
						break;
					}
			  	} else if (scaler_status & C_SCALER_STATUS_DONE) {
		  			// Scaler might finish its job before JPEG does when image is zoomed in.
		  			scaler_done = 1;
				} else if (scaler_status & (C_SCALER_STATUS_TIMEOUT|C_SCALER_STATUS_INIT_ERR)) {
					DBG_PRINT("Scaler failed to finish its job\r\n");
					break;
				} else if (scaler_status & C_SCALER_STATUS_INPUT_EMPTY) {
				  		if (scaler_restart(MJ_DECODE_SCALER)) {
						DBG_PRINT("Failed to call scaler_restart\r\n");
						break;
					}
			  	} else {
			  		DBG_PRINT("Un-handled Scaler status!\r\n");
			  		break;
			  	}
			}
			} else {
				scaler_status = scaler_fifo_once();
			}
			// Now restart JPEG to output to next FIFO
	  		if (jpeg_decode_output_restart()) {
	  			DBG_PRINT("Failed to call jpeg_decode_output_restart()\r\n");
	  			break;
	  		}
		}		// if (jpeg_status & C_JPG_STATUS_OUTPUT_FULL)

		if (jpeg_status & C_JPG_STATUS_STOP) {
			DBG_PRINT("JPEG is not started!\r\n");
			break;
		}
		if (jpeg_status & C_JPG_STATUS_TIMEOUT) {
			DBG_PRINT("JPEG execution timeout!\r\n");
			break;
		}
		if (jpeg_status & C_JPG_STATUS_INIT_ERR) {
			DBG_PRINT("JPEG init error!\r\n");
			break;
		}
		if (jpeg_status & C_JPG_STATUS_RST_VLC_DONE) {
			DBG_PRINT("JPEG Restart marker number is incorrect!\r\n");
			jpeg_status = C_JPG_STATUS_DECODE_DONE;
			scaler_status = C_SCALER_STATUS_DONE;
			break;
		}
		if (jpeg_status & C_JPG_STATUS_RST_MARKER_ERR) {
			DBG_PRINT("JPEG Restart marker sequence error!\r\n");
			jpeg_status = C_JPG_STATUS_DECODE_DONE;
			scaler_status = C_SCALER_STATUS_DONE;
			break;
		}
		// Check whether we have to break decoding this image
		/*if (image_task_handle_remove_request(cmd_id) > 0) {
			break;
		}*/
	}

	jpeg_decode_stop();

	scaler_stop(MJ_DECODE_SCALER);

	gp_free((void *) jpeg_out_y);
	if (jpeg_image_yuv_mode != C_JPG_CTRL_GRAYSCALE) {
  		gp_free((void *) jpeg_out_cb);
  		gp_free((void *) jpeg_out_cr);
  	}

	if ((jpeg_status & C_JPG_STATUS_DECODE_DONE) && (scaler_status & C_SCALER_STATUS_DONE)) {
	    if (!jpeg_output_buffer_pointer) {
	        jpeg_output_buffer_pointer = scaler_out_buffer;
	    }
	    cache_invalid_range(scaler_out_buffer, (jpeg_output_buffer_width*jpeg_output_buffer_height)<<1);
	} else {
		img_decode_struct->decode_status = STATUS_FAIL;
    	if (!jpeg_output_buffer_pointer) {
    	    gp_free((void *) scaler_out_buffer);
    	}

        return STATUS_FAIL;
	}

    img_decode_struct->decode_status = STATUS_OK;
    if (!img_decode_struct->output_buffer_pointer && jpeg_output_buffer_pointer) {
        img_decode_struct->output_buffer_pointer = jpeg_output_buffer_pointer;
    }
    img_decode_struct->output_image_width = jpeg_output_image_width;
    img_decode_struct->output_image_height = jpeg_output_image_height;
	return STATUS_OK;
}

void jpeg_memory_allocate(INT32U fifo)
{
	INT32U jpeg_output_y_size;
	INT32U jpeg_output_cb_cr_size;
	INT16U cbcr_shift;//, factor;

	if (fifo) {
		jpeg_output_y_size = jpeg_extend_width*jpeg_fifo_line_num*2*2; //YUYV
		jpeg_output_cb_cr_size = 0;
	} else {
		jpeg_output_y_size = jpeg_extend_width * jpeg_extend_height;
		jpeg_output_cb_cr_size = jpeg_output_y_size >> cbcr_shift;
	}
	
  	jpeg_out_y = (INT32U) gp_malloc_align(jpeg_output_y_size, 32);
  	if (!jpeg_out_y) {
  		return;
  	}
  	
  	if(jpeg_output_cb_cr_size) {
  		jpeg_out_cb = (INT32U) gp_malloc_align(jpeg_output_cb_cr_size, 8);
  		if (!jpeg_out_cb) {
  			gp_free((void *) jpeg_out_y);
  			jpeg_out_y = NULL;
  			return;
  		}
  		jpeg_out_cr = (INT32U) gp_malloc_align(jpeg_output_cb_cr_size, 8);
  		if (!jpeg_out_cr) {
  			gp_free((void *) jpeg_out_cb);
  			jpeg_out_cb = NULL;
  			gp_free((void *) jpeg_out_y);
  			jpeg_out_y = NULL;
  			return;
  		}
  	} else {
  		jpeg_out_cb = 0;
  		jpeg_out_cr = 0;
  	}
}

void jpeg_scaler_set_parameters(INT32U fifo)
{
	INT32U factor;
//	COLOR_MATRIX_CFG color_matrix;
//	umi_scaler_color_matrix_load();

/*	scaler_color_matrix_switch(1);
	color_matrix.SCM_A11 = 0x0100;
	color_matrix.SCM_A12 = 0x0000;
	color_matrix.SCM_A13 = 0x0000;
	color_matrix.SCM_A21 = 0x0000;
	color_matrix.SCM_A22 = 0x0180;
	color_matrix.SCM_A23 = 0x0000;
	color_matrix.SCM_A31 = 0x0000;
	color_matrix.SCM_A32 = 0x0000;
	color_matrix.SCM_A33 = 0x0180;
	scaler_color_matrix_config(&color_matrix);
*/


	scaler_input_pixels_set(UNDEFINE_SCALER, jpeg_extend_width, jpeg_extend_height);
	scaler_input_visible_pixels_set(UNDEFINE_SCALER, jpeg_valid_width, jpeg_valid_height);
	if (jpeg_output_image_width && jpeg_output_image_height) {
	    scaler_output_pixels_set(UNDEFINE_SCALER, (jpeg_valid_width<<16)/jpeg_output_image_width, (jpeg_valid_height<<16)/jpeg_output_image_height, jpeg_output_buffer_width, jpeg_output_buffer_height);
	} else {
	    if (!jpeg_output_image_width) {
	        jpeg_output_image_width = jpeg_output_buffer_width;
	    }
	    if (!jpeg_output_image_height) {
	        jpeg_output_image_height = jpeg_output_buffer_height;
	    }
    	if (jpeg_output_ratio == 0x0) {      // Fit to output buffer width and height
      		scaler_output_pixels_set(UNDEFINE_SCALER, (jpeg_valid_width<<16)/jpeg_output_image_width, (jpeg_valid_height<<16)/jpeg_output_image_height, jpeg_output_buffer_width, jpeg_output_buffer_height);
    	} else if (jpeg_output_ratio==2 && jpeg_valid_width<=jpeg_output_image_width && jpeg_valid_height<=jpeg_output_image_height) {
    		scaler_output_pixels_set(UNDEFINE_SCALER, 1<<16, 1<<16, jpeg_output_buffer_width, jpeg_output_buffer_height);
    		jpeg_output_image_width = jpeg_valid_width;
    		jpeg_output_image_height = jpeg_output_buffer_height;
    	} else {						// Fit to output buffer width or height
      		if (jpeg_output_image_height*jpeg_valid_width > jpeg_output_image_width*jpeg_valid_height) {
      			factor = (jpeg_valid_width<<16)/jpeg_output_image_width;
      			jpeg_output_image_height = (jpeg_valid_height<<16)/factor;
      		} else {
      			factor = (jpeg_valid_height<<16)/jpeg_output_image_height;
      			jpeg_output_image_width = (jpeg_valid_width<<16)/factor;
      		}
      		scaler_output_pixels_set(UNDEFINE_SCALER, factor, factor, jpeg_output_buffer_width, jpeg_output_buffer_height);
      	}
    }

	scaler_input_A_addr_set(UNDEFINE_SCALER,jpeg_out_y, 0, 0);
	scaler_input_B_addr_set(UNDEFINE_SCALER,0, 0, 0);

	if (jpeg_output_buffer_pointer) {
	    scaler_out_buffer = jpeg_output_buffer_pointer;
	} else {
	    scaler_out_buffer = (INT32U) gp_malloc((jpeg_output_buffer_width*jpeg_output_buffer_height)<<1);
	    if (!scaler_out_buffer) {
		    return;
		}
	}

DBG_PRINT("jpeg_scaler_set_parameters() call scaler_output_addr_set()\r\n");
   	scaler_output_addr_set(UNDEFINE_SCALER, scaler_out_buffer, NULL, NULL);
   	scaler_fifo_line_set(UNDEFINE_SCALER, fifo);
	scaler_YUV_type_set(UNDEFINE_SCALER, C_SCALER_CTRL_TYPE_YCBCR);
	scaler_input_format_set(UNDEFINE_SCALER, C_SCALER_CTRL_IN_YUYV);
	scaler_output_format_set(UNDEFINE_SCALER, jpeg_output_format);
	scaler_out_of_boundary_color_set(UNDEFINE_SCALER, out_of_boundary_color);
}

INT32S ap_state_common_handling(INT32U msg_id)
{		
	switch(msg_id) {
		case MSG_APQ_AUDIO_EFFECT_CAMERA:
			return(audio_effect_play(EFFECT_CAMERA));
			break;
		case MSG_APQ_AUDIO_EFFECT_OK:
		case MSG_APQ_AUDIO_EFFECT_MENU:
		case MSG_APQ_AUDIO_EFFECT_UP:
		case MSG_APQ_AUDIO_EFFECT_DOWN:
		case MSG_APQ_AUDIO_EFFECT_MODE:
			return(audio_effect_play(EFFECT_CLICK));
			break;
		default:
			break;
	}
	return 0;
}

void ap_state_handling_calendar_init(void)
{
	INT8U dt[3];
	TIME_T  tm;

	ap_state_config_factory_date_get(dt);

	tm.tm_year = dt[0] + 2000;
	tm.tm_mon = dt[1];
	tm.tm_mday = dt[2];

	ap_state_config_factory_time_get(dt);

	tm.tm_hour = dt[0];
	tm.tm_min = dt[1];
	tm.tm_sec = dt[2];

	rtc_ext_to_int_set();

	cal_day_store_get_register(ap_state_config_base_day_set,ap_state_config_base_day_get,ap_state_config_store);
	cal_factory_date_time_set(&tm);
	calendar_init();
}

#if C_BATTERY_DETECT == CUSTOM_ON 
void ap_state_handling_battery_icon_show(INT8U bat_lvl)
{
	if (bat_lvl < 5) {
		if (bat_icon != bat_lvl) {
			ap_state_handling_icon_clear_cmd((bat_icon + 1), NULL, NULL);
			bat_icon = bat_lvl;
		}
		if(!charge_icon){
			ap_state_handling_icon_show_cmd((bat_lvl + 1), NULL, NULL);
		}
	}
}

void ap_state_handling_charge_icon_show(INT8U charge_flag)
{
	if(charge_flag) {
		charge_icon = 1;
		ap_state_handling_icon_show_cmd(ICON_BATTERY_CHARGED, NULL, NULL);
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_BATTERY_CHARGE_ICON_BLINK_START, NULL, NULL, MSG_PRI_NORMAL);
		ap_state_handling_icon_clear_cmd((bat_icon + 1), NULL, NULL);
	} else {
		charge_icon = 0;
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_BATTERY_CHARGE_ICON_BLINK_STOP, NULL, NULL, MSG_PRI_NORMAL);
		ap_state_handling_icon_clear_cmd(ICON_BATTERY_CHARGED, NULL, NULL);
		ap_state_handling_icon_show_cmd((bat_icon + 1), NULL, NULL);
	}
}

void ap_state_handling_current_bat_lvl_show(void)
{	
	if(!charge_icon){
		ap_state_handling_icon_show_cmd((bat_icon + 1), NULL, NULL);
	}
}

void ap_state_handling_current_charge_icon_show(void)
{
/*	if(charge_icon) {
		ap_state_handling_icon_show_cmd(ICON_BATTERY_CHARGED, NULL, NULL);
	} else {
		ap_state_handling_icon_clear_cmd(ICON_BATTERY_CHARGED, NULL, NULL);
	}
*/
}


INT32S ap_state_handling_tv_init(void)
{
		INT32S nRet, msg;
		INT8U err;

		POST_MESSAGE(DisplayTaskQ, MSG_DISPLAY_TASK_TV_INIT, display_task_ack_m, 5000, msg, err);	
		nRet = STATUS_OK;
Return:
		return nRet;
}

INT32S ap_state_handling_tv_uninit(void)
{
		INT32S nRet, msg;
		INT8U err;

		POST_MESSAGE(DisplayTaskQ, MSG_DISPLAY_TASK_TV_UNINIT, display_task_ack_m, 5000, msg, err);	
		nRet = STATUS_OK;
Return:
		return nRet;
}

INT32S ap_state_handling_hdmi_init(void)
{
		INT32S nRet, msg;
		INT8U err;

		POST_MESSAGE(DisplayTaskQ, MSG_DISPLAY_TASK_HDMI_INIT, display_task_ack_m, 5000, msg, err);	
		nRet = STATUS_OK;
Return:
		return nRet;
}

INT32S ap_state_handling_hdmi_uninit(void)
{
		INT32S nRet, msg;
		INT8U err;

		POST_MESSAGE(DisplayTaskQ, MSG_DISPLAY_TASK_HDMI_UNINIT, display_task_ack_m, 5000, msg, err);	
		nRet = STATUS_OK;
Return:
		return nRet;
}



#endif


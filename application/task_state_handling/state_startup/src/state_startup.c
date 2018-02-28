#include "state_startup.h"
#include "ap_music.h"
#include "ap_state_config.h"
#include "ap_state_handling.h"
#include "ap_peripheral_handling.h"
#include "ap_display.h"
#if WIFI_FUNC_ENABLE
#include "wifi_abstraction_layer.h"
#include "socket_cmd.h"
#include "mjpeg_stream.h"
#include "Host_apis.h"
#endif

#if C_LOGO == CUSTOM_ON
	static INT32S startup_logo_img_ptr;
	static INT32U startup_logo_decode_buff;
#endif

static INT8U  audio_done = 0;

//	prototypes
void state_startup_init(void);
void state_startup_exit(void);

INT8U I80_diplay_flag = 0;
void TEST_Display_Data_180_degree_set(INT8U enable)
{
	I80_diplay_flag = enable;
}
INT8U TEST_Display_Data_180_degree_get()
{
	return I80_diplay_flag;
}

extern Display_Data_Conv(INT32U input_buff, INT32U output_buff, INT16U dev_width, INT16U dev_height);
extern Display_Data_Conv_180(INT32U input_buff, INT32U output_buff, INT16U dev_width, INT16U dev_height);
INT32U I80_diplay_buff = 0;
INT32U TEST_Display_Data_Conv(INT32U input_buff,INT16U dev_width, INT16U dev_height )
{
	if(!TEST_Display_Data_180_degree_get()){
		Display_Data_Conv(input_buff,I80_diplay_buff,dev_width*2,dev_height*2);
	}else{
		Display_Data_Conv_180(input_buff,I80_diplay_buff,dev_width*2,dev_height*2);
	}
	return 0;
}

INT8U Check_PowerOn_Battery_Value(void)
{
	INT32U battery_value;
	INT8U  dc_in;
	INT8U power_low = 0;
	
	adc_init(); 
	adc_vref_enable_set(TRUE); 
	adc_conv_time_sel(4); 
	adc_manual_ch_set(AD_BAT_DETECT_PIN); 
	
	gpio_init_io(ADP_OUT_PIN,GPIO_INPUT);
  	gpio_set_port_attribute(ADP_OUT_PIN, ATTRIBUTE_LOW);
  	gpio_write_io(ADP_OUT_PIN, DATA_LOW);

	R_ADC_MADC_CTRL |= 0x70;         /* start manual sample */ 
	while(!(R_ADC_MADC_CTRL & 0x8000)) ; /* wait ready */ 
	R_ADC_MADC_CTRL |= 0x8000; 

	battery_value = R_ADC_MADC_DATA >> 4;      // 0=0V, 1023=3.3V 
         
	dc_in = gpio_read_io(ADP_OUT_PIN); 

	//DBG_PRINT("!!!!battery_value = %d,dc_in = %d\n",battery_value,dc_in);   
	//if( ((dc_in==1)&&((battery_value>2250)||(battery_value<1250))) || ((dc_in==0)&&(battery_value>2200)) )
	if((battery_value <= 2015) && (dc_in == 0))   //这里数值需要根据实际量测到的低电压值做调整；如果电压低于3.5就关机
	{
		power_low = 1;
	}
	else
	{
		power_low = 0;
	}
	
	return power_low;
}

extern INT8U ext_rtc_pwr_on_flag;
extern INT8U PowerOn_Low_Check;

void state_startup_init(void)
{
	IMAGE_DECODE_STRUCT img_info;
	INT32U size;
	INT16U	logo_fd;
	#if WIFI_FUNC_ENABLE
	INT32U buff_size, buff_get;
	#endif
	
	DBG_PRINT("Startup state init enter\r\n");
#if TV_DET_ENABLE
   	if(tv_plug_status_get()) {
		ap_state_handling_tv_init();
	}
#endif

	//ap_music_effect_resource_init();
	audio_vol_set(ap_state_config_volume_get());

    #if C_LOGO == CUSTOM_ON
	if(!ext_rtc_pwr_on_flag)
	{
	    startup_logo_decode_buff = (INT32U) gp_malloc_align(getDispDevBufSize(), 64);
    	if (!startup_logo_decode_buff) {
    		DBG_PRINT("State startup allocate jpeg output buffer fail.\r\n");
    		return;
    	}
    	logo_fd = nv_open((INT8U *) "POWER_ON_LOGO.JPG");
    	if (logo_fd != 0xFFFF) {
    		size = nv_rs_size_get(logo_fd);
    		startup_logo_img_ptr = (INT32S) gp_malloc(size);
    		if (!startup_logo_img_ptr) {
    			DBG_PRINT("State startup allocate jpeg input buffer fail.[%d]\r\n", size);
    			gp_free((void *) startup_logo_decode_buff);
    			return;
    		}
    		if (nv_read(logo_fd, (INT32U) startup_logo_img_ptr, size)) {
    			DBG_PRINT("Failed to read resource_header in ap_startup_init\r\n");
    			gp_free((void *) startup_logo_img_ptr);
    			gp_free((void *) startup_logo_decode_buff);
    			return;
    		}
    		img_info.image_source = (INT32S) startup_logo_img_ptr;
    		img_info.source_size = size;
    		img_info.source_type = TK_IMAGE_SOURCE_TYPE_BUFFER;
    		img_info.output_format = C_SCALER_CTRL_OUT_RGB565;
    		img_info.output_ratio = 0;
    		img_info.out_of_boundary_color = 0x008080;

			if(ap_display_get_device()==DISP_DEV_TV) {
	    		img_info.output_buffer_width = TV_WIDTH;
	    		img_info.output_buffer_height = TV_HEIGHT;
	    		img_info.output_image_width = TV_WIDTH;
	    		img_info.output_image_height = TV_HEIGHT;
			} else {
	    		img_info.output_buffer_width = TFT_WIDTH;
	    		img_info.output_buffer_height = TFT_HEIGHT;
	    		img_info.output_image_width = TFT_WIDTH;
	    		img_info.output_image_height = TFT_HEIGHT;
	    	}
    		img_info.output_buffer_pointer = startup_logo_decode_buff;
    		if (jpeg_buffer_decode_and_scale(&img_info) == STATUS_FAIL) {
    			gp_free((void *) startup_logo_img_ptr);
    			gp_free((void *) startup_logo_decode_buff);
    			DBG_PRINT("State startup decode jpeg file fail.\r\n");
    			return;
    		}

			#if WIFI_FUNC_ENABLE
				buff_size = getDispDevBufSize();
				buff_get = ap_display_queue_get(display_isr_queue);
				if(buff_get)
				{
					dma_buffer_copy(startup_logo_decode_buff, buff_get, buff_size, 0, 0);
			  		ap_display_queue_put(display_dma_queue, buff_get);
			  	}
			#else
	    		OSQPost(DisplayTaskQ, (void *) (startup_logo_decode_buff|MSG_DISPLAY_TASK_JPEG_DRAW));
    		#endif
    		OSTimeDly(5);
    	} else {
    		DBG_PRINT("open POWER_ON_LOGO.JPG fail.\r\n");
    	}
    	gp_free((void *) startup_logo_decode_buff);
	}
    #endif

	if(ap_display_get_device() == DISP_DEV_TFT)
	{
		tft_backlight_en_set(TRUE);
	}

	if (PowerOn_Low_Check) {		//power off due to low battery 
		//ap_state_handling_str_draw_exit();
		//ap_state_handling_str_draw(STR_BATTERY_LOW, WARNING_STR_COLOR);
		//OSTimeDly(30);		// Delay and then power off
		DBG_PRINT("\r\n>>>>Low battery voltage!!!!>>>>\r\n");
		while (gpio_read_io(PW_KEY)) OSTimeDly(5); 
		tft_backlight_en_set(FALSE);
		ap_state_handling_power_off(0);
    }

    #if C_LOGO == CUSTOM_ON
	if(!ext_rtc_pwr_on_flag) {
		if(ap_display_get_device()==DISP_DEV_TV) {
			OSTimeDly(100);
		} else {
			//OSTimeDly(10);
		}
	}
	#endif
	
	if(!ext_rtc_pwr_on_flag) {
		if (audio_effect_play(EFFECT_POWER_ON)) {
			audio_done++;
		}
	} else {
		audio_done++;
	}

    #if C_LOGO == CUSTOM_ON
	if(!ext_rtc_pwr_on_flag) {
		gp_free((void *) startup_logo_img_ptr);
		OSTimeDly(50);
	}
	#endif
}

extern void audio_task_init(void);
extern void filesrv_task_init(void);
extern void task_storage_service_init(void);
extern void ap_video_capture_mode_switch(INT8U DoSensorInit, INT16U EnterAPMode);
extern void socket_cmd_service_init(void);
extern void mjpeg_service_init(void);

#include "lwipopts.h"
void state_startup_entry(void *para)
{
	EXIT_FLAG_ENUM exit_flag = EXIT_RESUME;
	INT32U msg_id;
	STAudioConfirm *audio_confirm;
	INT8U flag1 = 0;

	task_display_init();
	audio_task_init();
	task_peripheral_handling_init();
	ap_startup_init();
	filesrv_task_init();
	task_storage_service_init();
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_STORAGE_CHECK, NULL, NULL, MSG_PRI_NORMAL);
	ap_music_effect_resource_init();


	state_startup_init();
	/*#if 1 
	ap_video_capture_mode_switch(1, STATE_VIDEO_RECORD); // 秨诀璶暗sensor init
	#endif

	#if 0
	ap_video_capture_mode_switch(1, STATE_VIDEO_PREVIEW); // 秨诀璶暗sensor init
	#endif*/

	while (exit_flag == EXIT_RESUME) {
		if (msgQReceive(ApQ, &msg_id, (void *) ApQ_para, AP_QUEUE_MSG_MAX_LEN) == STATUS_FAIL) {
			continue;
		}
		switch (msg_id) {
			case EVENT_APQ_ERR_MSG:
			    audio_confirm = (STAudioConfirm*) ApQ_para;
			    if (audio_confirm->result_type == MSG_AUD_PLAY_RES && audio_confirm->result != AUDIO_ERR_NONE) {
			        audio_done++;
			    }
			    if ((audio_confirm->result == AUDIO_ERR_DEC_FINISH) && (audio_confirm->source_type == AUDIO_SRC_TYPE_APP_RS)){
					gpio_write_io(SPEAKER_EN, DATA_LOW);
				}
			    break;

			case MSG_STORAGE_SERVICE_MOUNT:
			case MSG_STORAGE_SERVICE_NO_STORAGE:
				if(!flag1){
				flag1 = 1;
				ap_state_handling_storage_id_set(ApQ_para[0]);
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
#endif
			default:
				break;
		}
		if((audio_done == 1) && flag1) {
			exit_flag = EXIT_BREAK;
		}
	}

    ap_state_firmware_upgrade();

//	R_SYSTEM_CTRL |= 0x30;			//27M => weak mode : |= 0x30
//	R_SYSTEM_CLK_CTRL |= 0x1000;	//32768	=>   mode : |= 0x1000

	#if WIFI_FUNC_ENABLE
	//RAY: Turn on WIFI
	wal_ldo_disable();
	OSTimeDly(1);
	wal_ldo_enable();
	wal_init(0, NULL);		//加载网卡驱动
    socket_cmd_service_init();
	#if PREVIEW_TCPIP	
	mjpeg_service_init();
	#else
	DBG_PRINT("MEMP_NUM_NETBUF=%d, PBUF_POOL_SIZE=%d\r\n",MEMP_NUM_NETBUF,PBUF_POOL_SIZE);	
	#endif

	#if POWER_ON_WIFI_ENABLE
	{
		/*
		if (wal_ap_mode_enable() != WAL_RET_SUCCESS) {
			DBG_PRINT("wifi init fail!!\r\n");
		}		
		else {
			DBG_PRINT("wifi init okl!!\r\n");	
			mjpeg_start_service();
			socket_cmd_start_service();
		}
		*/
		Wifi_State_Set(1);
	}
	#else
	{	// ﹍て闽 Wi-Fi
		wal_ap_mode_enable();
		if (wal_ap_mode_disable() != WAL_RET_SUCCESS) {
			DBG_PRINT("WiFi Turn OFF Fail!!\r\n");
		}
		else {
			DBG_PRINT("WiFi Turn OFF Ok!!\r\n");
		}
	}
	#endif
	#endif
	#if 1 
	ap_video_capture_mode_switch(1, STATE_VIDEO_RECORD); // 秨诀璶暗sensor init
	#endif

	#if 0
	ap_video_capture_mode_switch(1, STATE_VIDEO_PREVIEW); // 秨诀璶暗sensor init
	#endif

	state_startup_exit();
}


void state_startup_exit(void)
{
	INT8U temp;

    //DBG_PRINT("Exit Startup state\r\n");
	//gpio_write_io(SPEAKER_EN, DATA_LOW);
//-----G_sensor
	temp = ap_state_config_G_sensor_get();
   ap_gsensor_set_sensitive(temp);
	temp = ap_gsensor_power_on_get_status();
	if(temp){// g_sensor power on
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_G_SENSOR_POWER_ON_START, NULL, NULL, MSG_PRI_NORMAL);
	}

//------Flsh LED
	temp = ap_state_config_flash_LED_get();	
	ap_peripheral_night_mode_set(temp);

	if(ext_rtc_pwr_on_flag) {
		INT8U temp;

		temp = ap_state_config_hang_mode_get();
		temp &= 0x0f;
		if(temp == 1) {
			OSQPost(StateHandlingQ, (void *) STATE_VIDEO_RECORD);
		} else { //temp == 2
			OSQPost(StateHandlingQ, (void *) STATE_VIDEO_PREVIEW);
		}
	} else {
	  #if ENABLE_SAVE_SENSOR_RAW_DATA
		OSQPost(StateHandlingQ, (void *) STATE_VIDEO_PREVIEW);
	  #else
	  	#if 1 
		OSQPost(StateHandlingQ, (void *) STATE_VIDEO_RECORD);
		#endif

	  	#if 0 
		OSQPost(StateHandlingQ, (void *) STATE_VIDEO_PREVIEW);
		#endif

		#if 0
		OSQPost(StateHandlingQ, (void *) STATE_BROWSE);
		#endif
	  #endif
	}
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TV_POLLING_START, NULL, NULL, MSG_PRI_NORMAL);
}

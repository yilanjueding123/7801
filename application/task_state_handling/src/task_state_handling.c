#include "ap_state_config.h"
#include "ap_state_resource.h"

#define STATE_HANDLING_QUEUE_MAX			5
#define AP_QUEUE_MAX    					128

MSG_Q_ID ApQ;
OS_EVENT *StateHandlingQ;
INT8U ApQ_para[AP_QUEUE_MSG_MAX_LEN];
void *state_handling_q_stack[STATE_HANDLING_QUEUE_MAX];

//	prototypes
void state_handling_init(void);

INT32U display_battery_low_frame0;

void state_handling_init(void)
{
	INT32S config_load_flag;

	StateHandlingQ = OSQCreate(state_handling_q_stack, STATE_HANDLING_QUEUE_MAX);
	ApQ = msgQCreate(AP_QUEUE_MAX, AP_QUEUE_MAX, AP_QUEUE_MSG_MAX_LEN);
	
	nvmemory_init();
	config_load_flag = ap_state_config_load();
	ap_state_resource_init();
	ap_state_config_initial(config_load_flag);
#if ENABLE_CHECK_RTC == 1
	if (config_load_flag != STATUS_FAIL)
	{
		ap_state_config_data_time_check();
	}
#endif
	ap_state_wifi_ssid_password_save_get();
	ap_state_handling_calendar_init();

	ap_state_handling_storage_id_set(NO_STORAGE);
	
	//ap_state_config_language_set(LCD_SCH);//for test
	//ap_state_config_record_time_set(3);//for test
	ap_state_config_wdr_set(0);
	//ap_state_config_osd_mode_set(1);
	ap_state_config_rotate_set(SENSOR_FLIP);
	ap_state_config_burst_set(0);	//Á¬ÅÄoff
	ap_state_config_videolapse_set(0);
	//ap_state_config_record_time_set(3);
	ap_state_config_tv_switch_set(0); //tv switch off
/*
	{
		DISPLAY_ICONSHOW icon = {200, 88, TRANSPARENT_COLOR, 0, 0};
		//INT32U i, *buff_ptr, color_data, cnt;
		INT32U display_frame0;

		display_battery_low_frame0 = (INT32U) gp_malloc_align(icon.icon_w * icon.icon_h * 2, 64);
		DBG_PRINT("batteryLow_display_buf_addr = 0x%x \r\n",display_frame0);
*/		
		/*buff_ptr = (INT32U *) display_battery_low_frame0;
		color_data = 0x8C71ul|(0x8C71ul<<16);
		cnt = (icon.icon_w * icon.icon_h * 2) >> 2;
		for (i=0 ; i<cnt ; i++) {
			*buff_ptr++ = color_data;
		}*/
/*
		DBG_PRINT("batteryLowshow! \r\n");

		{
			INT32U size, read_buf;
			INT16U logo_fd;
			INT8U *zip_buf;
			INT32U icon_buffer;

			logo_fd = nv_open((INT8U*)"INSERTSDC.GPZP");
			if (logo_fd != 0xFFFF) {
				size = nv_rs_size_get(logo_fd);
				read_buf = (INT32S) gp_malloc(size);
				if (!read_buf) {
					DBG_PRINT("allocate BACKGROUND.GPZP read buffer fail.[%d]\r\n", size);
				}

				if (nv_read(logo_fd, (INT32U) read_buf, size)) {
					DBG_PRINT("Failed to read icon_buffer resource\r\n");
					gp_free((void *) read_buf);
				}

				zip_buf = (INT8U *)read_buf;
				if (gpzp_decode((INT8U *) &zip_buf[4], (INT8U *) display_battery_low_frame0) == STATUS_FAIL) {
					DBG_PRINT("Failed to unzip GPZP file\r\n");
					gp_free((void *) read_buf);
					gp_free((void *) icon_buffer);
					read_buf = 0;
					icon_buffer = 0;
				}

				gp_free((void *) read_buf);
			}
		}
	}
	*/
	DBG_PRINT("state_handling_init OK!\r\n"); 
}

void state_handling_entry(void *para)
{
	INT32U msg_id, prev_state;
	INT8U err;

	msg_id = 0;
	state_handling_init();
	OSQPost(StateHandlingQ, (void *) STATE_STARTUP);

	while(1) {
		prev_state = msg_id;
		msg_id = (INT32U) OSQPend(StateHandlingQ, 0, &err);
		if((!msg_id) || (err != OS_NO_ERR)) {
        	continue;
        }
		present_state = msg_id & 0x1FF;
		DBG_PRINT("present_state = 0x%08X\r\n",present_state); 
		switch(msg_id & 0x1FF) {
			case STATE_STARTUP:
				state_startup_entry((void *) &prev_state);
				break;
			case STATE_VIDEO_PREVIEW:
				state_video_preview_entry((void *) &prev_state);
				break;
			case STATE_VIDEO_RECORD:
				state_video_record_entry((void *) &prev_state, msg_id);
				break;
			case STATE_AUDIO_RECORD:
				//state_audio_record_entry((void *) &prev_state);
				break;	
			case STATE_BROWSE:
				state_browse_entry((void *) &prev_state, (msg_id & 0xFFFF0000)>>16);
				msg_id &= 0x0000ffff;
				break;
			case STATE_THUMBNAIL:
				state_thumbnail_entry((void *) &prev_state);
				break;
			case STATE_SETTING:
				state_setting_entry((void *) &prev_state);
				break;
			default:
				break;
		}
	}
}
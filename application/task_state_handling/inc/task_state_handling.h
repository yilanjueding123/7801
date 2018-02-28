#ifndef __TASK_STATE_HANDLING_H__
#define __TASK_STATE_HANDLING_H__

#include "application.h"

#define TRANSPARENT_COLOR				0x8c71
#ifdef HDMI_JPG_DECODE_AS_GP420
    #define TRANSPARENT_COLOR_YUYV			0x8c808c80
#elif defined(HDMI_JPG_DECODE_AS_YUV422)    
    #define TRANSPARENT_COLOR_YUYV			0x8c80
#endif

#define VIDEO_STREAM				0x63643030
#define AUDIO_STREAM				0x62773130
#define JUNK_DATA					0x4b4e554a

#define CARD_FULL_MB_SIZE			6ULL 	// <= 6MB is mean card full
#define CARD_FULL_SIZE_RECORD		15ULL	//30ULL	// <= 30MB

#define EFFECT_CLICK     0
#define EFFECT_CAMERA    1
#define EFFECT_POWER_ON  2
#define EFFECT_POWER_OFF 3
#define EFFECT_FILE_LOCK 4
#define EFFECT_BEEP		 5
#define EFFECT_LDW_TurnOn		6
#define EFFECT_LDW_TurnOff		7
#define EFFECT_LDW_Alarm		8

typedef struct {
	INT16U	string_width;
	INT16U	string_height;
} t_STRING_TABLE_STRUCT;

typedef struct{
    INT16U font_color;
    INT16U font_type;
    INT16S pos_x;
    INT16S pos_y;
    INT16U buff_w;
    INT16U buff_h;
    char *str_ptr;
}STRING_ASCII_INFO;

typedef struct{
	INT8U b_alarm_flag;
	INT8U B_alarm_hour;
	INT8U B_alarm_minute;
	INT8U Alarm_Music_idx[3];
} ALARM_STRUCT,*pALARM_STRUCT;

typedef struct{
	INT8U mode;
	INT8U on_hour;
	INT8U on_minute;
	INT8U off_hour;
	INT8U off_minute;
} POWERONOFF_STRUCT, *pPOWERONOFF_STRUCT;

typedef struct
{
	INT32U Frequency;
} FM_STATION_FORMAT;

#define FM_STATION_NUM	20

typedef struct
{
	FM_STATION_FORMAT 	FM_Station[FM_STATION_NUM];
	INT8U				FM_Station_index;
} FM_INFO;

typedef struct {
     INT8U  language;
     INT8U  date_format_display;
     INT8U  time_format_display;
     INT8U  week_format_display;
     INT8U  lcd_backlight;
     INT8U  sound_volume;
     INT8U  current_storage;
     INT8U  current_photo_index[3];
     INT8U  slideshow_duration;
     INT8U  slideshow_bg_music[3];
     INT8U  slideshow_transition;
     INT8U  slideshow_photo_date_on;
     INT8U  calendar_duration;
     INT8U  calendar_displaymode;
     INT8U  thumbnail_mode;
     INT8U  music_play_mode;
     INT8U  music_on_off;
     INT8U  midi_exist_mode;
     INT8U  factory_date[3];                //yy,mm,dd
     INT8U  factory_time[3];                //hh:mm:ss
     ALARM_STRUCT	nv_alarm_struct1;
     ALARM_STRUCT	nv_alarm_struct2;
     INT8U	alarm_modual;
     INT8U	full_screen;
     INT8U	sleep_time;
     POWERONOFF_STRUCT powertime_onoff_struct1;
     POWERONOFF_STRUCT powertime_onoff_struct2;
     INT8U powertime_modual;
	 INT8S powertime_active_id;
	 INT8U powertime_happened;
//     copy_file_counter;
//     INT32U Pre_Pow_off_state;
     INT8U video_resolution;
     INT8U reserved0;
     INT8U reserved1;
     INT8U reserved2;
     INT8U reserved3;
     INT8U reserved4;
     INT8U wdr_on_off;
     INT8U reserved6;

     FM_INFO FM_struct;
	 INT8U	save_as_logo_flag;
	 INT8S	alarm_volume1;
	 INT8S	alarm_volume2;
	 INT8U	alarm_mute1;
	 INT8U	alarm_mute2;
     INT8U	ui_style;
     INT32U base_day;
     INT8U  ifdirty;
     INT8U  user_wifi_ssid[10];
     INT8U  user_wifi_password[10];
} USER_ITEMS;

typedef struct {
    USER_ITEMS item ;
	INT8U crc[4];
	INT8U  DUMMY_BYTE[512-sizeof(USER_ITEMS)-4];
} SYSTEM_USER_OPTION, *pSYSTEM_USER_OPTION;

extern OS_EVENT *StateHandlingQ;
extern void state_handling_entry(void *para);
extern void state_startup_entry(void *para);
extern void state_video_preview_entry(void *para);
extern void state_video_record_entry(void *para, INT32U state);
extern void state_audio_record_entry(void *para);
extern void state_browse_entry(void *para, INT16U play_index);
extern void state_setting_entry(void *para);
extern void state_thumbnail_entry(void *para);

extern void ap_state_handling_night_mode_switch(void);
extern INT8U ap_state_handling_night_mode_get(void);
extern INT32S ap_state_handling_str_draw(INT16U str_index, INT16U str_color);
extern INT32S ap_state_handling_str_draw_HDMI(INT16U str_index, INT16U str_color);
extern void ap_state_handling_str_draw_exit(void);
extern void ap_state_handling_icon_show_cmd(INT8U cmd1, INT8U cmd2, INT8U cmd3);
extern void ap_state_handling_icon_clear_cmd(INT8U cmd1, INT8U cmd2, INT8U cmd3);
extern void ap_state_handling_connect_to_pc(INT32U prev_state);
extern void ap_state_handling_disconnect_to_pc(void);
extern void ap_state_handling_storage_id_set(INT8U stg_id);
extern INT8U ap_state_handling_storage_id_get(void);
extern void ap_state_config_store(void);
extern void ap_state_config_md_set(INT8U md);
extern INT8U ap_state_config_md_get(void);
extern void ap_state_config_quality_set(INT8U quality);
extern INT8U ap_state_config_quality_get(void);
extern INT8U ap_state_config_voice_record_switch_get(void);
extern void ap_state_music_set(INT32U music_index);
extern INT32U ap_state_music_get(void);
extern void ap_state_music_play_mode_set(INT8U music_play_mode);
extern INT8U ap_state_music_play_mode_get(void);
extern INT8U ap_state_config_video_resolution_get(void);
extern void ap_state_config_video_resolution_set(INT8U resolution);
extern void ap_state_config_videolapse_set(INT8U lapse);
extern INT8U ap_state_config_videolapse_get(void);
extern void ap_state_config_osd_mode_set(INT8U osdmode);
extern INT8U ap_state_config_osd_mode_get(void);
extern void ap_state_config_rotate_set(INT8U rotate);
extern INT8U ap_state_config_rotate_get(void);
extern void ap_state_config_tv_switch_set(INT8U preview);
extern INT8U ap_state_config_tv_switch_get(void);
extern void ap_state_config_car_mode_set(INT8U preview);
extern INT8U ap_state_config_car_mode_get(void);
extern INT8U ap_state_handling_file_creat_get(void);

extern INT32U ap_state_config_wifi_ssid_get(void);
extern void ap_state_config_wifi_ssid_set(INT8S *name);
extern INT32U ap_state_config_wifi_pwd_get(void);
extern void ap_state_config_wifi_pwd_set(INT8S *name);

/* ap music */
typedef struct {
	INT8U   aud_state;
	BOOLEAN mute;
	INT8U   volume;
	INT8U   play_style;
}audio_status_st;

typedef enum
{
	STATE_IDLE,
	STATE_PLAY,
	STATE_PAUSED,
	STATE_AUD_FILE_ERR
}AUDIO_STATE_ENUM;

typedef enum
{
//	PLAY_ONCE,
	PLAY_SEQUENCE,
	PLAY_REPEAT,
	PLAY_MIDI,
	PLAY_SPECIAL_REPEAT,
	PLAY_SPECIAL_BYNAME_REPEAT,
	PLAY_SHUFFLE_MODE
}AUDIO_PLAY_STYLE;

//extern void ap_state_handling_led_on(void);
//extern void ap_state_handling_led_off(void);
extern void ap_state_handling_power_off(INT32U wakeup_flag);
//extern void ap_state_handling_led_flash_on(void);
//extern void ap_state_handling_led_blink_on(void);
extern void ap_state_handling_calendar_init(void);

#if C_BATTERY_DETECT == CUSTOM_ON 
	extern void ap_state_handling_battery_icon_show(INT8U bat_lvl);
	extern void ap_state_handling_charge_icon_show(INT8U charge_flag);
	extern void ap_state_handling_current_bat_lvl_show(void);
	extern void ap_state_handling_current_charge_icon_show(void);
#endif

extern INT16U present_state;

//extern INT32S avi_encode_put_display_start_frame(INT32U *buff_addr);
extern INT32U avi_encode_get_jpeg_enc_buf_addr(void);
extern INT32S ap_state_firmware_upgrade(void);

#endif//__TASK_STATE_HANDLING_H__

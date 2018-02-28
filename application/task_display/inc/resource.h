#include "task_display.h"

extern INT16U icon_battery0[];			//1
extern INT16U icon_battery1[];			//2
extern INT16U icon_battery2[];			//3
extern INT16U icon_battery3[];			//4
extern INT16U icon_battery4[];
//extern INT16U icon_battery_red[];		//5
extern INT16U icon_capture[];			//6
extern INT16U icon_video[];				//7
extern INT16U icon_review[];			//8
extern INT16U icon_base_setting[];
extern INT16U icon_red_light[];			//9
extern INT16U icon_rec_yellow[];			//9
extern INT16U icon_1_second_yellow[];
extern INT16U icon_1_second_red[];
extern INT16U icon_2_second_yellow[];
extern INT16U icon_2_second_red[];
extern INT16U icon_3_second_yellow[];
extern INT16U icon_3_second_red[];
extern INT16U icon_5_second_yellow[];
extern INT16U icon_5_second_red[];
extern INT16U icon_10_second_yellow[];
extern INT16U icon_10_second_red[];
extern INT16U icon_20_second_yellow[];
extern INT16U icon_20_second_red[];
extern INT16U icon_30_second_yellow[];
extern INT16U icon_30_second_red[];
extern INT16U icon_60_second_yellow[];
extern INT16U icon_60_second_red[];
extern INT16U icon_play[];				//10
extern INT16U icon_pause[];				//11
extern INT16U icon_motion_detect[];		//12
extern INT16U icon_mic_off[];			//13
extern INT16U icon_mic_on[];	
extern INT16U icon_ir_on[];				//14
extern INT16U icon_video1[];			//15
extern INT16U icon_focus[];				//16
extern INT16U icon_sd_card[];							//17
extern INT16U icon_internal_memory[];
extern INT16U icon_no_sd_card[];			//17
extern INT16U icon_video_record_cyc_off[];
extern INT16U icon_video_record_cyc_1minute[];
extern INT16U icon_video_record_cyc_2minute[];
extern INT16U icon_video_record_cyc_3minute[];
extern INT16U icon_video_record_cyc_5minute[];
extern INT16U icon_video_record_cyc_10minute[];
extern INT16U icon_video_record_motion_10second[];
extern INT16U icon_1080P30[];
extern INT16U icon_720P60[];
extern INT16U icon_720P30[];
extern INT16U icon_wvga[];
extern INT16U icon_vga[];
extern INT16U icon_video_EV6[];
extern INT16U icon_video_EV5[];
extern INT16U icon_video_EV4[];
extern INT16U icon_video_EV3[];
extern INT16U icon_video_EV2[];
extern INT16U icon_video_EV1[];
extern INT16U icon_video_EV0[];
extern INT16U icon_video_EV_1[];
extern INT16U icon_video_EV_2[];
extern INT16U icon_video_EV_3[];
extern INT16U icon_video_EV_4[];
extern INT16U icon_video_EV_5[];
extern INT16U icon_video_EV_6[];
extern INT16U icon_video_ldw_active[];
extern INT16U icon_motion_detect_start[];	//18
extern INT16U icon_usb_connect[];			//19
extern INT16U icon_scroll_bar[];			//20
extern INT16U icon_scroll_bar_idx[];		//21
extern INT16U icon_md_sts0[];				//22
extern INT16U icon_md_sts1[];				//23
extern INT16U icon_md_sts2[];				//24
extern INT16U icon_md_sts3[];				//25
extern INT16U icon_md_sts4[];				//26
extern INT16U icon_md_sts5[];				//27
extern INT16U icon_battery_charged[];
extern INT16U icon_locked[];
extern INT16U icon_audio_record[];
extern INT16U icon_2x[];					//41
extern INT16U icon_4x[];					//42
extern INT16U icon_forward[];				//45
extern INT16U icon_backward[];				//46

extern INT16U ui_up[];
extern INT16U ui_left[];
extern INT16U ui_right[];
extern INT16U ui_down[];

extern INT16U icon_background_mode[];
extern INT16U icon_background_up[];
extern INT16U icon_background_down[];
extern INT16U icon_background_left[];
extern INT16U icon_background_right[];
extern INT16U icon_background_ok[];
extern INT16U icon_usb[];
extern INT16U icon_wifi_signal[];

extern INT16U ui_text_0[];
extern INT16U ui_text_1[];
extern INT16U ui_text_2[];
extern INT16U ui_text_3[];
extern INT16U ui_text_4[];
extern INT16U ui_text_5[];
extern INT16U ui_text_6[];
extern INT16U ui_text_7[];
extern INT16U ui_text_8[];
extern INT16U ui_text_9[];
extern INT16U ui_text_klino	[];	//	"/"
extern INT16U ui_text_colon[];	//	":"

extern INT8U hdmi_ui_text_0[];
extern INT8U hdmi_ui_text_1[];
extern INT8U hdmi_ui_text_2[];
extern INT8U hdmi_ui_text_3[];
extern INT8U hdmi_ui_text_4[];
extern INT8U hdmi_ui_text_5[];
extern INT8U hdmi_ui_text_6[];
extern INT8U hdmi_ui_text_7[];
extern INT8U hdmi_ui_text_8[];
extern INT8U hdmi_ui_text_9[];
extern INT8U hdmi_ui_text_klino	[];	//	"/"
extern INT8U hdmi_ui_text_colon[];	//	":"

/*// For state setting
extern INT16U sub_menu_2[];
extern INT16U sub_menu_3[];
extern INT16U sub_menu_4[];
extern INT16U background_c[];
extern INT16U select_bar_bb[];
extern INT16U background_b[];
extern INT16U background_a[];

extern INT16U selectbar_middle[];
extern INT16U selectbar_long[];
extern INT16U topbar[];
*/


extern INT16U icon_manu_base_date_time[];
extern INT16U icon_manu_base_auto_power_off[];
extern INT16U icon_manu_base_screen_saver[];
extern INT16U icon_manu_base_delay_power_on[];
extern INT16U icon_manu_base_beep_sound[];
extern INT16U icon_manu_base_language[];
extern INT16U icon_manu_base_TV_mode[];
extern INT16U icon_manu_base_car_mode[];
extern INT16U icon_manu_base_frequency[];
extern INT16U icon_manu_base_format[];
extern INT16U icon_manu_base_default_setting[];
extern INT16U icon_manu_base_version[];
extern INT16U icon_manu_base_wifi_ssid[];
extern INT16U icon_manu_base_wifi_password[];

extern INT16U icon_playback_volume_title[];
extern INT16U icon_playback_volume_red[];
extern INT16U icon_playback_volume_white[];


//--------Thumbnail--------------//
extern INT16U thumbnail_lock_icon[];
extern INT16U icon_playback_movie[];
//extern INT16U thumbnail_video_icon[];
extern INT16U thumbnail_cursor_3x3_96x64[];
extern INT16U thumbnail_cursor_3x3_black_96x64[];

//-------------MP3---------------//
extern INT16U icon_mp3_play[];
extern INT16U icon_mp3_pause[];
extern INT16U icon_mp3_play_one[];
extern INT16U icon_mp3_play_all[];
extern INT16U icon_mp3_index[];
extern INT16U icon_mp3_volume[];
extern INT16U icon_mp3_mute[];
extern INT16U icon_mp3_stop[];

extern INT16U mp3_index_0[];
extern INT16U mp3_index_1[];
extern INT16U mp3_index_2[];
extern INT16U mp3_index_3[];
extern INT16U mp3_index_4[];
extern INT16U mp3_index_5[];
extern INT16U mp3_index_6[];
extern INT16U mp3_index_7[];
extern INT16U mp3_index_8[];
extern INT16U mp3_index_9[];
extern INT16U mp3_index_dot[];
extern INT16U mp3_index_MHz[];

extern INT16U mp3_input_0[];
extern INT16U mp3_input_1[];
extern INT16U mp3_input_2[];
extern INT16U mp3_input_3[];
extern INT16U mp3_input_4[];
extern INT16U mp3_input_5[];
extern INT16U mp3_input_6[];
extern INT16U mp3_input_7[];
extern INT16U mp3_input_8[];
extern INT16U mp3_input_9[];

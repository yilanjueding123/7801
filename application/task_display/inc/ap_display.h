#ifndef __AP_DISPLAY_H__
#define __AP_DISPLAY_H__

#include "task_display.h"


#define PHOTO_CAPTURE_TIME_INTERVAL		128	//128 = 1s


#define POSTION_Record_Time_X			33//200+45
#define POSTION_Record_Time_Y			1//5

#if (TV_WIDTH == 720)
  #define POSTION_Record_Time_X_tv		615
#elif(TV_WIDTH == 640)
  #define POSTION_Record_Time_X_tv		65//560
#endif

#define POSTION_Record_Time_Y_tv		25//30

#define C_ICON_HDMI_OFFSET 	30

#define POSTION_Date_Time_Record_X		1//100-20
#define POSTION_Date_Time_Record_Y		211//210
#define POSTION_Date_Time_Record_X2		POSTION_Date_Time_Record_X-5
#define POSTION_Date_Time_Record_Y2		POSTION_Date_Time_Record_Y-5
#define POSTION_Date_Time_Brose_X		280-32
#define POSTION_Date_Time_Brose_Y		190-20

#define POSTION_Date_Time_Record_X_tv		120*2
#define POSTION_Date_Time_Record_Y_tv		215*2
#define POSTION_Date_Time_Record_X2_tv		POSTION_Date_Time_Record_X_tv
#define POSTION_Date_Time_Record_Y2_tv		POSTION_Date_Time_Record_Y_tv

#if TV_WIDTH == 720
  #define POSTION_Date_Time_Brose_X_tv		675-64
#elif TV_WIDTH == 640
  #define POSTION_Date_Time_Brose_X_tv		625-64
#endif

#define POSTION_Date_Time_Brose_Y_tv		400-40

#define POSTION_Record_Time_X_HDMI		(1280-40-160)-C_ICON_HDMI_OFFSET
#define POSTION_Record_Time_Y_HDMI		30
#define POSTION_Date_Time_Brose_X_HDMI	(1280-30-156)-C_ICON_HDMI_OFFSET
#define POSTION_Date_Time_Brose_Y_HDMI	520

// display device ID
#define DISP_DEV_TFT	1	// 320x240
#define DISP_DEV_TV		2  	// 640x480
#define DISP_DEV_HDMI	3  	// 1280x720
extern INT8U ap_display_get_device(void);

extern void ap_display_init(void);
extern void ap_display_tv_init(void);
extern void ap_display_tv_uninit(void);
extern void ap_display_hdmi_init(void);
extern void ap_display_hdmi_uninit(void);
extern void ap_display_effect_init(void);
extern void ap_display_setting_frame_buff_set(INT32U frame_buff);
//extern void ap_display_icon_move(DISPLAY_ICONMOVE *icon);
extern void ap_display_icon_sts_set(INT32U msg);
extern void ap_display_icon_sts_clear(INT32U msg);
extern void ap_display_buff_copy_and_draw(INT32U buff_addr, INT16U src_type);
extern void ap_display_hdmi_buff_copy_and_draw(INT32U buff_addr, INT16U src_type);
extern void ap_display_timer(void);
extern void ap_display_effect_sts_set(INT8U type);
extern void ap_display_string_draw(STR_ICON *str_info);
extern void ap_display_video_preview_end(void);
extern void ap_display_mp3_index_sts_set(INT8U type);
extern void ap_display_mp3_input_num_sts_set(INT8U type);
extern void timer_counter_force_display(INT8U force_en);
extern void ap_display_left_rec_time_draw(INT32U time, INT8U flag);
extern void ap_display_timer_browser(INT8U type);
extern void timer_counter_force_display_browser(INT8U force_en);
extern void date_time_force_display(INT8U force_en,INT8U postion_flag);
extern INT32U getDispDevBufSize(void);
extern void ap_display_hdmi_rec_date_time_draw_browse(INT16U *frame_buff, INT16U pos_x, INT16U pos_y);

extern INT32S vid_dec_get_total_time(void);
extern INT32S vid_dec_get_current_time(void);
extern INT32S audio_dac_get_curr_play_time(void);
extern INT32S audio_dac_get_total_time(void);
extern INT8U audio_playing_state_get(void);

extern INT8U display_mp3_volume;
extern INT16U display_mp3_play_index;
extern INT16U display_mp3_total_index;
extern INT16U display_mp3_FM_channel;
extern INT16U display_mp3_input_num;

extern INT32U ap_display_queue_get(INT32U *queue);
extern INT32S ap_display_queue_put(INT32U *queue, INT32U data);
extern INT32U ap_display_queue_query(INT32U *queue);

extern void ap_display_icon_draw(INT16U *frame_buff, INT16U *icon_stream, DISPLAY_ICONSHOW *icon_info);
extern void ap_display_hdmi_icon_draw(INT16U *frame_buff, INT16U *icon_stream, DISPLAY_ICONSHOW *icon_info);

void ap_display_rec_date_time_draw(INT16U *frame_buff);

extern INT8U *display_frame[];
extern INT32U display_isr_queue[];
extern INT32U display_dma_queue[];  //FIFO of display buffer address for interrupt

extern void ap_display_string_icon_start_draw(STR_ICON_EXT *str_info);
extern void ap_display_string_icon_clear(STR_ICON_EXT *str_info);

#endif

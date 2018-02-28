#include "task_state_handling.h"


extern INT32S ap_state_handling_str_draw(INT16U str_index, INT16U str_color);
extern INT32S ap_state_handling_str_draw_HDMI(INT16U str_index, INT16U str_color);
extern void ap_state_handling_str_draw_exit(void);
extern void ap_state_handling_icon_show_cmd(INT8U cmd1, INT8U cmd2, INT8U cmd3);
extern void ap_state_handling_icon_clear_cmd(INT8U cmd1, INT8U cmd2, INT8U cmd3);
extern void ap_state_handling_connect_to_pc(INT32U prev_state);
extern void ap_state_handling_disconnect_to_pc(void);
extern void ap_state_handling_storage_id_set(INT8U stg_id);
extern INT8U ap_state_handling_storage_id_get(void);
//extern void ap_state_handling_led_on(void);
//extern void ap_state_handling_led_off(void);
//extern void ap_state_handling_led_flash_on(void);
//extern void ap_state_handling_led_blink_on(void);
extern void ap_state_handling_calendar_init(void);
extern void ap_state_handling_file_creat_set(INT8U id);
extern INT8U ap_state_handling_file_creat_get(void);

#if C_BATTERY_DETECT == CUSTOM_ON 
	extern void ap_state_handling_battery_icon_show(INT8U bat_lvl);
	extern void ap_state_handling_charge_icon_show(INT8U charge_flag);
	extern void ap_state_handling_current_bat_lvl_show(void);
	extern void ap_state_handling_current_charge_icon_show(void);
#endif

extern void ap_state_handling_night_mode_switch(void);
extern INT8U ap_state_handling_night_mode_get(void);
extern INT32S ap_state_common_handling(INT32U msg_id);
extern void ap_state_handling_lcd_backlight_switch(INT8U enable);
extern INT32S ap_state_handling_jpeg_decode(STOR_SERV_PLAYINFO *info_ptr, INT32U jpg_output_addr);

extern void ap_state_handling_clear_all_icon(void);
extern void ap_state_handling_power_off(INT32U wakeup_flag);
extern INT32S jpeg_buffer_decode_and_scale(IMAGE_DECODE_STRUCT *img_decode_struct);
extern void jpeg_memory_allocate(INT32U fifo);
extern void jpeg_scaler_set_parameters(INT32U fifo);
extern INT16U present_state;

extern INT32S ap_state_handling_ASCII_str_draw(STR_ICON_EXT *str_ptr,STRING_ASCII_INFO *str_ascii_info);
extern INT32S ap_state_handling_ASCII_str_draw_HDMI(STR_ICON_EXT *str_ptr, STRING_INFO *str_info);
extern void ap_state_handling_ASCII_str_draw_exit(STR_ICON_EXT *str_ptr,INT8U wait);
extern INT32U display_battery_low_frame0;

extern INT32S ap_state_handling_tv_init(void);
extern INT32S ap_state_handling_tv_uninit(void);
extern INT32S ap_state_handling_hdmi_init(void);
extern INT32S ap_state_handling_hdmi_uninit(void);

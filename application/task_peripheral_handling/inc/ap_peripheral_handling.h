#include "task_peripheral_handling.h"


typedef void (*KEYFUNC)(INT16U *tick_cnt_ptr);

typedef struct
{
	KEYFUNC key_function;
	KEYFUNC fast_key_fun;
	INT16U key_io;
	INT16U key_cnt;	
	INT16U key_active;
	INT16U key_long_flag;
}KEYSTATUS;

extern void ap_peripheral_init(void);
extern void ap_peripheral_key_judge(void);
extern void ap_peripheral_adaptor_out_judge(void);
extern void ap_peripheral_key_register(INT8U type);
extern void ap_peripheral_gsensor_data_register(void );
extern void ap_peripheral_motion_detect_judge(void);
extern void ap_peripheral_motion_detect_start(void);
extern void ap_peripheral_motion_detect_stop(void);

extern void ap_peripheral_ok_key_exe(INT16U *tick_cnt_ptr);
extern void ap_peripheral_function_key_exe(INT16U *tick_cnt_ptr);
extern void ap_peripheral_menu_key_exe(INT16U *tick_cnt_ptr);
extern void ap_peripheral_prev_key_exe(INT16U *tick_cnt_ptr);
extern void ap_peripheral_next_key_exe(INT16U *tick_cnt_ptr);
extern void ap_peripheral_md_key_exe(INT16U *tick_cnt_ptr);

extern void ap_peripheral_charge_det(void);
extern void ap_peripheral_lcd_backlight_set(INT8U type);
extern void ap_peripheral_night_mode_set(INT8U type);

extern void ap_TFT_backlight_tmr_check(void);
extern void ap_peripheral_tv_detect(void);
extern void ap_peripheral_read_gsensor(void);

extern void ap_peripheral_ad_key_judge(void);
extern void ap_peripheral_battery_check_calculate(void);
extern void ap_peripheral_config_store(void);

//extern void ap_peripheral_led_set(INT8U type);
//extern void ap_peripheral_led_flash_set(void);
//extern void ap_peripheral_led_blink_set(void);

extern INT8U ap_video_record_sts_get(void);

extern void ap_peripheral_hdmi_detect(void);
extern void ap_peripheral_clr_screen_saver_timer(void);

extern void LED_pin_init(void);
extern void set_led_mode(LED_MODE_ENUM mode);
extern void LED_blanking_isr(void);
extern void led_red_on(void);
extern void led_blue_on(void);
extern void led_yellow_on(void);
extern void led_ir_on(void);
extern void led_blue_on(void);
extern void led_all_off(void);
extern void led_ir_off(void);
extern void led_red_off(void);
extern void led_blue_off(void);
extern void led_yellow_off(void);
extern INT8U ap_peripheral_low_vol_get(void);
extern void led_power_off(void);
extern void led_lvd_power_on(void);
extern void set_led_mode(LED_MODE_ENUM mode);

#if TV_DET_ENABLE
extern INT8U tv_plug_status_get(void);
#endif
extern void ap_peripheral_md_mode_set(INT8U sta);
extern INT8U ap_peripheral_md_mode_get(void);
extern INT32U video_busy_cnt_get(void);
extern void video_busy_cnt_clr(void);

extern void ChargeMode_set(INT8U m);
extern INT8U ChargeMode_get(void);

#ifndef __AP_SETTING_H__
#define __AP_SETTING_H__

#include "state_setting.h"
#include "drv_l2_spi_flash.h"

#define STRING_DRAW					0
#define SUB_MENU_DRAW				0x1
#define FUNCTION_ACTIVE				0x2
#define SUB_MENU_MOVE				0x3
#define SECOND_SUB_MENU_DRAW		0x4

#define SETTING_DATE_TIME_DRAW_ALL	0xFF

#define SETTING_ICON_SINGLE_COLOR	0x0
#define SETTING_ICON_NORMAL_DRAW	0x1
#define SETTING_ICON_BLUE_COLOR		0x2
#define SETTING_ICON_BLUE1_COLOR	0x3


#define ICON_SELECT_BAR_WIDTH		320//296
#define ICON_SELECT_BAR_HEIGHT		30//40
#define ICON_SELECT_BAR_START_X		0
#define ICON_SELECT_BAR_START_Y		35//50

#define ICON_TOP_BAR_WIDTH			48
#define ICON_TOP_BAR_HEIGHT			32
#define ICON_TOP_BAR_START_X			10//13
#define ICON_TOP_BAR_START_Y			3//10

#define ICON_BACKGROUND_MODE_WIDTH			64
#define ICON_BACKGROUND_MODE_HEIGHT			32
#define ICON_BACKGROUND_MODE_START_X			13
#define ICON_BACKGROUND_MODE_START_Y			240-32

#define ICON_BACKGROUND_LEFT_WIDTH			28//32
#define ICON_BACKGROUND_LEFT_HEIGHT			28//32
#define ICON_BACKGROUND_LEFT_START_X			ICON_BACKGROUND_MODE_START_X+110
#define ICON_BACKGROUND_LEFT_START_Y			ICON_BACKGROUND_MODE_START_Y

#define ICON_SELECTBOX_SUB_WIDTH	256
#define ICON_SELECTBOX_SUB_HEIGHT	160
#define ICON_SELECTBOX_SUB_START_X	50+5
#define ICON_SELECTBOX_SUB_START_Y	46

#define ICON_SELECTBAR_MIDD_WIDTH	214//216
#define ICON_SELECTBAR_MIDD_HEIGHT	30//32
#define ICON_SELECTBAR_MIDD_START_X	ICON_SELECTBOX_SUB_START_X+((ICON_SELECTBOX_SUB_WIDTH-ICON_SELECTBAR_MIDD_WIDTH)>>1)
#define ICON_SELECTBAR_MIDD_START_Y	ICON_SELECTBOX_SUB_START_Y+32

#define ICON_YEAR_SELECTBAR_WIDTH	64
#define ICON_YEAR_SELECTBAR_HEIGHT	32
#define ICON_YEAR_SELECTBAR_START_X	ICON_SELECTBOX_SUB_START_X+30
#define ICON_YEAR_SELECTBAR_START_Y	ICON_SELECTBOX_SUB_START_Y+32

#define ICON_DAY_SELECTBAR_WIDTH	32
#define ICON_DAY_SELECTBAR_HEIGHT	32
#define ICON_DAY_SELECTBAR_START_X	ICON_SELECTBOX_SUB_START_X+30
#define ICON_DAY_SELECTBAR_START_Y	ICON_SELECTBOX_SUB_START_Y+70


#define ICON_SETTING_HEAD_WIDTH		28//32
#define ICON_SETTING_HEAD_HEIGHT	28//32
#define ICON_SETTING_HEAD_START_X	10//18
#define ICON_SETTING_HEAD_START_Y	37//53


typedef void (*SETTINGFUNC)(void);

INT8U ap_setting_EV_data[13][4] = {
	{0xAB, 0x06, 0x9B, 0x3C},
	{0xAB, 0x06, 0x9B, 0x34},
	{0xAB, 0x06, 0x9B, 0x2C},
	{0xAB, 0x06, 0x9B, 0x24},
	{0xAB, 0x06, 0x9B, 0x1C},
	{0xAB, 0x06, 0x9B, 0x14},
	{0xAB, 0x06, 0x9B, 0x0C},
	{0xAB, 0x06, 0x9B, 0x04},
	{0xAB, 0x0D, 0x9B, 0x04},
	{0xAB, 0x0D, 0x9B, 0x0C},
	{0xAB, 0x0D, 0x9B, 0x14},
	{0xAB, 0x0D, 0x9B, 0x1C},
	{0xAB, 0x0D, 0x9B, 0x24},
};
/*
INT8U ap_setting_Sharpness[3][2] = {
	{0x8F, 0x0A},
	{0x8F, 0x05},
	{0x8F, 0x00}
};
*/
INT8U ap_setting_Sharpness[3][2] = {
	{0x8F, 0x05},
	{0x8F, 0x01},
	{0x8F, 0x00}
};

INT8U ap_setting_ISO_data[3][2] = {
	{0x00, 0x00},
	{0x00, 0x00},
	{0x00, 0x20}
};
/*
INT8U ap_setting_Saturation[3][4] = {
	{0xA7, 0x65, 0xA8, 0x60},
	{0xA7, 0x55, 0xA8, 0x46},
	{0xA7, 0x35, 0xA8, 0x30}
};
*/
INT8U ap_setting_Saturation[3][4] = {
	{0xA7, 0x50, 0xA8, 0x50},
	{0xA7, 0x44, 0xA8, 0x44},
	{0xA7, 0x30, 0xA8, 0x30}
};

INT8U ap_setting_Color[4][4] = {
	{0xA9, 0x80, 0xAA, 0x80},
	{0xA9, 0x00, 0xAA, 0x00},
	{0xA9, 0x90, 0xAA, 0x90},
	{0xA9, 0x40, 0xAA, 0x40}
};

INT8U ap_setting_White_Balance[5][6] = {
	{0x01, 0x80, 0x02, 0x40, 0x03, 0x00},
	{0x01, 0xFF, 0x02, 0x55, 0x03, 0x85},
	{0x01, 0xFF, 0x02, 0x55, 0x03, 0x85},
	{0x01, 0xFF, 0x02, 0x40, 0x03, 0x70},
	{0x01, 0xFF, 0x02, 0x50, 0x03, 0x80}
};

/*
INT8U ap_setting_Scene_Mode[2][2] = {
	{0x0E, 0xA5},
	{0x0E, 0xA5}
};
*/
/*
INT8U ap_setting_Scene_Mode[2][4] = {
	{0x24, 0x40, 0x25, 0x30},
	{0x24, 0x50, 0x25, 0x40}
};
*/
INT8U ap_setting_Scene_Mode[2][4] = {
	{0x24, 0x60, 0x25, 0x50},
	{0x24, 0x70, 0x25, 0x60}
};

/*
INT8U ap_setting_Light_Freq[2][4] = {
	{0x22, 0x98, 0x23, 0x02},
	{0x22, 0x98, 0x23, 0x02}
};
INT8U ap_setting_Light_Freq[2][4] = {
	{0x33, 0x66, 0x23, 0x03},
	{0x33, 0x00, 0x23, 0x02}
};
*/
INT8U ap_setting_Light_Freq[2][4] = {
	{0x22, 0x99, 0x23, 0x02},
	{0x22, 0x7f, 0x23, 0x02}
};

extern void ap_setting_init(INT32U prev_state,INT32U prev_state1, INT8U *tag);
extern void ap_setting_exit(INT32U prev_state);
extern INT32U ap_setting_show_GPZP_file(INT8U *path,INT16U *frame_buff, DISPLAY_ICONSHOW *icon_info, INT8U type);
extern void ap_setting_reply_action(INT32U state,INT32U state1, INT8U *tag, STOR_SERV_PLAYINFO *info_ptr);
extern void ap_setting_page_draw(INT32U state,INT32U state1, INT8U *tag);
extern void ap_setting_sub_menu_draw(INT8U curr_tag);
extern void ap_setting_func_key_active(INT8U *tag, INT8U *sub_tag, INT32U state,INT32U state1);
extern void ap_setting_direction_key_active(INT8U *tag, INT8U *sub_tag, INT32U key_type, INT32U state,INT32U state1);
extern void ap_USB_setting_direction_key_active(INT8U *tag, INT8U *sub_tag, INT32U key_type, INT32U state,INT32U state1);
extern void ap_USB_setting_display_ok(void);
extern EXIT_FLAG_ENUM ap_setting_menu_key_active(INT8U *tag, INT8U *sub_tag, INT32U *state, INT32U *state1);
extern EXIT_FLAG_ENUM ap_setting_mode_key_active(INT32U next_state, INT8U *sub_tag);
extern EXIT_FLAG_ENUM ap_setting_mode_key_active1(INT8U *tag, INT8U *sub_tag, INT32U state,INT32U next_state);
extern void ap_setting_other_reply(void);
extern void ap_setting_sensor_command_switch(INT16U cmd_addr, INT16U reg_bit, INT8U enable);
extern void ap_setting_value_set_from_user_config(void);
extern void ap_setting_frame_buff_display(void);
extern void ap_setting_del_protect_file_show(INT8U *tag, INT32U state,INT32U state1);

#endif


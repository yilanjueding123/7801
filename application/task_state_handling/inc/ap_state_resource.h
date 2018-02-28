#include "task_state_handling.h"

typedef struct {
	CHAR  id[4];
	INT32U  version;
	INT32U  reserve0;
	INT16U	irkey_num;
	INT16U	rbkey_state_num;
	INT16U	rbkey_num;
	INT16U	osd_menu_num;
	INT16U	osd_menu_item_num;
	INT16U	main_menu_num;
	INT16U	main_menu_item_num;
	INT16U	palette_item;
	INT16U	image_item;
	INT16U	audio_item;
	INT16U	audio_open_item;
	INT16U	language_num;
	INT16U	string_item;
	INT8U	str_ch_width[20];	//1 byte or 2 byte
	INT16U	font_item;
	INT16U	auto_demo_num;
	INT16U	others_bin_num;
	INT32U	offset_startup_ctrl;
	INT32U	offset_ir_key;
	INT32U	offset_rubber_key;
	INT32U	offset_osd_menu;
	INT32U	offset_main_menu;
	INT32U	offset_popmessage;
	INT32U	offset_audio;
	INT32U	offset_palette;
	INT32U	offset_image;
	INT32U	offset_video;
	INT32U	offset_string[20];
	INT32U	offset_font[20];
	INT32U	offset_sys_image_idx;
	INT32U	offset_factor_default_option;
	INT32U	offset_auto_demo;
	INT32U	offset_others_bin;
	INT32U	offset_multi_language_str;
	INT32U	offset_multi_language_font;
	INT32U  reserve1[8];
} t_GP_RESOURCE_HEADER;

typedef struct {
	INT16U	length;
	INT32U	raw_data_offset;
} t_STRING_STRUCT;

typedef struct {
	INT32U	length;
	INT32U	compress_length;	// font compress 1021 neal
	INT32U	raw_data_offset;
} t_FONT_STRUCT;

typedef struct {
	INT8U	font_width;
	INT8U	font_height;
	INT8U	bytes_per_line;
	INT32U	font_content;
} t_FONT_TABLE_STRUCT;

extern INT32S ap_state_resource_init(void);
extern void ap_state_resource_exit(void);
extern INT32S ap_state_resource_string_resolution_get(STRING_INFO *str_info, t_STRING_TABLE_STRUCT *str_res);
extern INT32S ap_state_resource_string_draw(INT16U *frame_buff, STRING_INFO *str_info, INT8U draw_type);
extern INT32S ap_state_resource_string_ascii_draw(INT16U *frame_buff, STRING_ASCII_INFO *str_ascii_info, INT8U draw_type);
extern INT16U ap_state_resource_language_num_get(void);
extern INT32S ap_state_resource_user_option_load(SYSTEM_USER_OPTION *user_option);extern INT16S ap_state_resource_time_stamp_position_x_get(void);
extern INT16S ap_state_resource_time_stamp_position_y_get(void);

#ifndef __STATE_SETTING_H__
#define __STATE_SETTING_H__

#include "task_state_handling.h"

typedef struct{
    INT8U item_start;
    INT8U item_max;
    INT8U sub_item_start;
    INT8U sub_item_max;
    INT16U stage;
}SETUP_ITEM_INFO;

extern void state_setting_entry(void *para);
extern void ap_setting_init(INT32U prev_state,INT32U prev_state1, INT8U *tag);
extern void ap_setting_exit(INT32U prev_state);
extern void ap_setting_format_reply(INT8U *tag, INT32U state,INT32U state1);
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
extern void ap_setting_format_reply(INT8U *tag, INT32U state,INT32U state1);
extern void ap_setting_other_reply(void);
extern void ap_setting_sensor_command_switch(INT16U cmd_addr, INT16U reg_bit, INT8U enable);
extern void ap_setting_value_set_from_user_config(void);
extern void ap_setting_frame_buff_display(void);
extern void ap_setting_del_protect_file_show(INT8U *tag, INT32U state,INT32U state1);


#endif //__STATE_SETTING_H__

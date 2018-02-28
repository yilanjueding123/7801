#include "task_state_handling.h"

#define BROWSE_UNMOUNT			0x1
#define BROWSE_DECODE_BUSY		0x2
#define BROWSE_PLAYBACK_BUSY	0x4
#define BROWSE_PLAYBACK_PAUSE	0x8

extern void state_browse_entry(void *para, INT16U play_index);
extern void ap_browse_init(INT32U prev_state, INT16U play_index);
extern void ap_browse_exit(INT32U next_state_msg);
extern void ap_browse_func_key_active(void);
extern void ap_browse_mjpeg_stop(void);
extern void ap_browse_mjpeg_play_end(void);
extern void ap_browse_next_key_active(INT8U err_flag);
extern void ap_browse_prev_key_active(INT8U err_flag);
extern void ap_browse_reply_action(STOR_SERV_PLAYINFO *info_ptr);
extern void ap_browse_sts_set(INT8S sts);
extern INT8S ap_browse_stop_handle(void);
extern void ap_browse_no_media_show(INT16U str_type);
extern INT8S ap_browse_sts_get(void);
extern void ap_browse_display_update_command(void);
extern INT8U g_browser_reply_action_flag;
extern void ap_browse_display_timer_start(void);
extern void ap_browse_display_timer_stop(void);
extern void ap_browse_display_timer_draw(void);
extern void ap_browse_wav_play(void);
extern void ap_browse_wav_stop(void);
extern INT8S browser_play_speed;
extern INT8U ap_browse_get_curr_file_type(void);
extern void ap_browse_show_file_name(INT8U enable, INT32U buf_addr);

#include "state_video_preview.h"

#define AVI_FRAME_RATE					30		// Frame per second

extern void ap_video_preview_init(void);
extern void ap_video_preview_exit(void);
extern INT32S ap_video_preview_func_key_active(void);
extern INT32S ap_video_preview_reply_action(STOR_SERV_FILEINFO *file_info_ptr);
extern void ap_video_preview_reply_done(INT8U ret, INT32U file_path_addr);
extern void ap_video_preview_left_capture_num_thinking(void);
extern void ap_video_capture_mode_switch(INT8U DoSensorInit, INT16U EnterAPMode);
extern void left_capture_num_display(INT32U left_num);
extern void left_capture_num_str_clear(void);
extern void video_capture_resolution_display(void);
extern void video_capture_resolution_clear(void);


#include "task_state_handling.h"

#define VIDEO_PREVIEW_UNMOUNT				0x1

#define VIDEO_PREVIEW_MD_TIME_INTERVAL		64

#define CONTINUOUS_SHOOTING_COUNT_MAX	5

#define TIME_LAPSE_DISABLE            0
#define TIME_LAPSE_ENABLE             1
#define TIME_LAPSE_PROCESS            2

extern void state_video_preview_entry(void *para);
extern void ap_video_preview_init(void);
extern void ap_video_preview_exit(void);
extern INT32S ap_video_preview_func_key_active(void);
extern INT32S ap_video_preview_reply_action(STOR_SERV_FILEINFO *file_info_ptr);
extern void ap_video_preview_reply_done(INT8U ret, INT32U file_path_addr);
extern void ap_video_preview_left_capture_num_thinking(void);
extern void left_capture_num_display(INT32U left_num);
extern INT32U cal_left_capture_num(void);
extern void left_capture_num_str_clear(void);
extern void video_capture_resolution_display(void);
extern void video_capture_resolution_clear(void);

extern INT32U left_capture_num;

#include "state_video_record.h"

#define VIDEO_RECORD_CYCLE_TIME_INTERVAL		128*60	//128 = 1s
#define MOTION_DETECT_CHECK_TIME_INTERVAL		128

extern INT8S ap_video_record_init(INT32U state);
extern void ap_video_record_exit(void);
#if C_MOTION_DETECTION == CUSTOM_ON	
	extern void ap_video_record_md_tick(INT8U *md_tick,INT32U state);
	extern INT8S ap_video_record_md_active(INT8U *md_tick,INT32U state);
	extern void ap_video_record_md_disable(void);
#endif	
extern INT8S ap_video_record_func_key_active(INT32U event);
extern void ap_video_record_start(void);
extern void ap_video_record_sts_set(INT8S sts);
#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
	extern void ap_video_record_cycle_reply_open(STOR_SERV_FILEINFO *file_info_ptr);
	extern void ap_video_record_cycle_reply_action(void);
#endif
extern INT32S ap_video_record_del_reply(INT32S ret,INT32U state);
extern void ap_video_record_error_handle(INT32U state);
extern INT8U ap_video_record_sts_get(void);
extern INT8U ap_video_record_MD_sts_get(void);
extern void bkground_del_disable(INT32U disable1_enable0);
extern INT32U bkground_del_thread_size_get(void);
extern INT8S bkground_del_disable_status_get(void);
extern void ap_video_record_lock_current_file(void);

extern void video_calculate_left_recording_time_enable(void);
extern void video_calculate_left_recording_time_disable(void);

extern INT32S ap_video_record_zoom_inout(INT8U inout);

extern void ap_video_record_clear_park_mode(void);
extern void ap_video_record_show_park_mode(void);
extern void ap_video_record_clear_resolution_str(void);
extern void ap_video_record_resolution_display(void);

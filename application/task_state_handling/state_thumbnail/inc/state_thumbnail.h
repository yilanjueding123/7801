#include "task_state_handling.h"

#define THUMBNAIL_UNMOUNT			0x1
#define THUMBNAIL_DECODE_BUSY		0x2
#define THUMBNAIL_PLAYBACK_BUSY		0x4
#define THUMBNAIL_PLAYBACK_PAUSE	0x8

extern void state_thumbnail_entry(void *para);
extern void ap_thumbnail_init(INT8U flag);
extern void ap_thumbnail_exit(void);
extern INT16U ap_thumbnail_func_key_active(void);
extern void ap_thumbnail_next_key_active(INT8U err_flag);
extern void ap_thumbnail_prev_key_active(INT8U err_flag);
extern void ap_thumbnail_reply_action(STOR_SERV_PLAYINFO *info_ptr);
extern void ap_thumbnail_sts_set(INT8S sts);
extern INT8S ap_thumbnail_stop_handle(void);
extern void ap_thumbnail_no_media_show(INT16U str_type);
extern void ap_thumbnail_connect_to_pc(void);
extern void ap_thumbnail_disconnect_to_pc(void);
extern void ap_thumbnail_clean_frame_buffer(void);
extern INT8S ap_thumbnail_sts_get(void);
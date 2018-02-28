#include "application.h"

#define STORAGE_SERVICE_QUEUE_MAX			64
#define STORAGE_SERVICE_QUEUE_MAX_MSG_LEN	128

extern MSG_Q_ID StorageServiceQ;
extern void ap_storage_service_init(void);
extern INT32S ap_storage_service_storage_mount(void);
#if C_AUTO_DEL_FILE == CUSTOM_ON
	extern void ap_storage_service_freesize_check_switch(INT8U type);
	extern INT32S ap_storage_service_freesize_check_and_del(void);
	extern void ap_storage_service_free_filesize_check(void);
#endif
extern void task_storage_service_entry(void *para);
extern void ap_storage_service_file_open_handle(INT32U type);
extern void ap_storage_service_timer_start(void);
extern void ap_storage_service_timer_stop(void);
extern void ap_storage_service_play_req(STOR_SERV_PLAYINFO *info_ptr, INT32U req_msg);
#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
	extern void ap_storage_service_cyclic_record_file_open_handle(INT8U type);
#endif
extern void FileSrvRead(P_TK_FILE_SERVICE_STRUCT para);
extern void ap_storage_service_usb_plug_in(void);
extern void ap_storage_service_format_req(BOOLEAN is_reply);
extern void ap_storage_service_file_lock_one(void);
extern void ap_storage_service_file_lock_all(void);
extern void ap_storage_service_file_unlock_one(void);
extern void ap_storage_service_file_unlock_all(void);

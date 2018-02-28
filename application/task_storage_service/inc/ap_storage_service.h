#include "task_storage_service.h"
#include "stdio.h"

#define STORAGE_TIME_INTERVAL_MOUNT		128	//128 = 1s
#define BKGROUND_DETECT_INTERVAL  (15*128)  // 15 second
#define BKGROUND_DEL_INTERVAL     (1*128)   // 1 second
#define STORAGE_TIME_INTERVAL_FREESIZE	128*15//128*15	//128 = 1s
#define PLAYFILE_INDEX_TABLE_SIZE		(9999/512+1)		// Each 512 files creates a record.

extern void ap_storage_service_init(void);
extern INT32S ap_storage_service_storage_mount(void);
#if C_AUTO_DEL_FILE == CUSTOM_ON
	extern void ap_storage_service_freesize_check_switch(INT8U type);
	extern INT32S ap_storage_service_freesize_check_and_del(void);
	extern void ap_storage_service_free_filesize_check(void);
#endif
extern void ap_storage_service_file_open_handle(INT32U type);
extern void ap_storage_service_timer_start(void);
extern void ap_storage_service_timer_stop(void);
extern void ap_storage_service_play_req(STOR_SERV_PLAYINFO *info_ptr, INT32U req_msg);
#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
	extern void ap_storage_service_cyclic_record_file_open_handle(INT8U type);
#endif
extern void ap_storage_service_usb_plug_in(void);
extern INT8U ap_state_config_date_stamp_get(void);
extern void ap_state_config_date_stamp_set(INT8U flag);
extern void ap_storage_service_file_del(INT32U idx);
extern void ap_storage_service_file_delete_all(void);
extern void ap_storage_service_file_lock_one(void);
extern void ap_storage_service_file_lock_all(void);
extern void ap_storage_service_file_unlock_one(void);
extern void ap_storage_service_file_unlock_all(void);
extern void ap_storage_service_format_req(BOOLEAN is_reply);
extern void ap_storage_service_del_thread_mb_set(void);
extern INT16S ap_storage_service_playfilelist_req(INT8U *buff_addr, INT16U play_file_index);
extern INT16S ap_storage_service_indexfile_req(INT16U playfile_order, INT16U *playfile_index_table);
extern INT32S ap_storage_service_download_file_req(INT16U file_index,STOR_SERV_DOWNLOAD_FILEINFO *dw_file_info,INT8U *buf_addr,INT32U *buf_size);
extern INT32S ap_storage_service_download_file_block_size(INT32U size);
extern INT16U ap_storage_service_playfile_totalcount(INT16U *playfile_index_table);
extern void * ap_storage_service_thumbnail_req(INT16U file_index,INT32U *buf_size);
extern INT32U bkground_del_thread_size_get(void);

#include "ap_storage_service.h"
#include "socket_cmd.h"

MSG_Q_ID StorageServiceQ = NULL;
void *storage_service_q_stack[STORAGE_SERVICE_QUEUE_MAX];
static INT8U storage_service_para[STORAGE_SERVICE_QUEUE_MAX_MSG_LEN];

STOR_SERV_PLAYINFO play_info = {0};

extern INT8U usbd_storage_exit;
extern INT8U device_plug_phase;
#ifdef SDC_DETECT_PIN
extern void ap_peripheral_SDC_detect_init(void);
#endif

void task_storage_service_init(void)
{
#ifdef SDC_DETECT_PIN
	ap_peripheral_SDC_detect_init();
#endif
	if (StorageServiceQ==NULL) {
	    StorageServiceQ = msgQCreate(STORAGE_SERVICE_QUEUE_MAX, STORAGE_SERVICE_QUEUE_MAX, STORAGE_SERVICE_QUEUE_MAX_MSG_LEN);
	}
	ap_storage_service_init();

	DBG_PRINT(" task_storage_service_init OK!\r\n");
}

void task_storage_service_entry(void *para)
{
	INT32U msg_id;
	INT32S ret;
	INT32U wifi_dl_cnt = 0;

	if (StorageServiceQ==NULL) {
		StorageServiceQ = msgQCreate(STORAGE_SERVICE_QUEUE_MAX, STORAGE_SERVICE_QUEUE_MAX, STORAGE_SERVICE_QUEUE_MAX_MSG_LEN);
	}
	//task_storage_service_init();
	while(1) {
		if (msgQReceive(StorageServiceQ, &msg_id, storage_service_para, STORAGE_SERVICE_QUEUE_MAX_MSG_LEN) == STATUS_FAIL) {
			continue;
		}


#if GP_SOCK_DOUBLE_BUFFER_DOWNLOAD

	if (wifi_dl_state!=WIFI_DOWNLOAD_IDLE)
	{
		if ((download_file_info.file_size > wifi_dl_cur_size)||(wifi_dl_cnt==0))  // wifi_dl_cnt=0 first time
		{
			wifi_dl_cnt++;
			if ( (curr_buf==1)&&(buf_dl_flag[1]==0) )
			{
				// Download
				ret = ap_storage_service_download_file_req(wifi_dl_play_file_index,&download_file_info,wifi_dl_buf_B+64,&(buf_dl_size[1]));	// +64(memory align) for speed up
				if (ret!=0)
				{
					buf_dl_size[1] = 0xFFFFFFFF;
					DBG_PRINT("download buffer B fail\r\n");
				}
				// Switch index
				buf_dl_flag[1] = 1;
				curr_buf = 0;
			}
			else if ( (curr_buf==0)&&(buf_dl_flag[0]==0) )
			{
				// Download
				ret = ap_storage_service_download_file_req(wifi_dl_play_file_index,&download_file_info,wifi_dl_buf_A+64,&(buf_dl_size[0]));	// +64(memory align) for speed up
				if (ret!=0)
				{
					buf_dl_size[0] = 0xFFFFFFFF;
					DBG_PRINT("download buffer A fail\r\n");
				}
				// Switch index
				buf_dl_flag[0] = 1;
				curr_buf = 1;
			}
			else
			{
				DBG_PRINT("L%d%d ",curr_buf,buf_dl_flag[0],buf_dl_flag[1]);
			}
		}
		else
		{
			wifi_dl_cnt = 0;
			buf_dl_flag[curr_buf] = 0xFF;
			DBG_PRINT("F\r\n");
		}		
	}
	else
	{
		wifi_dl_cnt = 0;
		// DBG_PRINT("task_storage_service loop\r\n");
	}

#endif


        switch (msg_id) {
        	case MSG_STORAGE_SERVICE_STORAGE_CHECK:
        		//DBG_PRINT(" s_usbd_pin = 0x%x\r\n",s_usbd_pin);
        		if (!s_usbd_pin) {
        			ap_storage_service_storage_mount();
        		}
        		break;
        	case MSG_STORAGE_SERVICE_USB_IN:
        		ap_storage_service_usb_plug_in();
        		break;
#if C_AUTO_DEL_FILE == CUSTOM_ON
        	case MSG_STORAGE_SERVICE_FREESIZE_CHECK_SWITCH:
        		ap_storage_service_freesize_check_switch(storage_service_para[0]);
        		break;
        	case MSG_STORAGE_SERVICE_FREESIZE_CHECK:
        		ap_storage_service_freesize_check_and_del();
        		break;
        	case MSG_STORAGE_SERVICE_FREE_FILESIZE_CHECK:
				ap_storage_service_free_filesize_check();
        		break;

        	case MSG_STORAGE_SERVICE_AUTO_DEL_LOCK:
        		ap_storage_service_freesize_check_switch(FALSE);
        		break;
#endif
        	case MSG_STORAGE_SERVICE_VIDEO_FILE_DEL:
        		ap_storage_service_file_del(*((INT32U *) storage_service_para));
        		break;
        	case MSG_STORAGE_SERVICE_FILE_DEL_ALL:
        		ap_storage_service_file_delete_all();
        		break;

        	case MSG_STORAGE_SERVICE_LOCK_ONE:
        		ap_storage_service_file_lock_one();
        		break;
        	case MSG_STORAGE_SERVICE_LOCK_ALL:
        		ap_storage_service_file_lock_all();
        		break;
        	case MSG_STORAGE_SERVICE_UNLOCK_ONE:
        		ap_storage_service_file_unlock_one();
        		break;
        	case MSG_STORAGE_SERVICE_UNLOCK_ALL:
        		ap_storage_service_file_unlock_all();
        		break;

        	case MSG_STORAGE_SERVICE_TIMER_START:
        		ap_storage_service_timer_start();
        		break;
        	case MSG_STORAGE_SERVICE_TIMER_STOP:
        		ap_storage_service_timer_stop();
        		break;
        	case MSG_STORAGE_SERVICE_AUD_REQ:
        	case MSG_STORAGE_SERVICE_PIC_REQ:
        	//case MSG_STORAGE_SERVICE_PIC_REQ2:
        	case MSG_STORAGE_SERVICE_VID_REQ:
        		ap_storage_service_file_open_handle(msg_id);
        		break;
#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON
			case MSG_STORAGE_SERVICE_VID_CYCLIC_REQ:
				ap_storage_service_cyclic_record_file_open_handle(storage_service_para[0]);
				break;
#endif
        	case MSG_STORAGE_SERVICE_THUMBNAIL_REQ:
        	case MSG_STORAGE_SERVICE_BROWSE_REQ:
        		ap_storage_service_play_req(&play_info, *((INT32U*) storage_service_para));
        		break;
        	case MSG_STORAGE_SERVICE_FORMAT_REQ:
     			ap_storage_service_format_req(TRUE);
        		break;        		
        	case MSG_FILESRV_FS_READ:
				FileSrvRead((P_TK_FILE_SERVICE_STRUCT)storage_service_para);
				break;

			case MSG_STORAGE_USBD_EXIT:
			case MSG_STORAGE_USBD_PCAM_EXIT:
				device_plug_phase = 0;
				usbd_storage_exit = 1;
				disk_safe_exit(MINI_DVR_STORAGE_TYPE);
				break; 		

        	default:
        		break;
        }
    }
}

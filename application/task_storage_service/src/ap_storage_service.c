#include "ap_storage_service.h"
#include "ap_state_config.h"
#include "ap_state_handling.h"
#include "fs_driver.h"
#include "avi_encoder_app.h"
#include "state_wifi.h"

#define AVI_REC_MAX_BYTE_SIZE   0x70000000  //1879048192 Bytes//78643200//4B00000
#define AP_STG_MAX_FILE_NUMS    625

extern INT8U screen_saver_enable;
static INT32S g_jpeg_index;
extern STOR_SERV_PLAYINFO play_info;
static INT32S g_avi_index;
static INT32S g_wav_index;
static INT32S g_file_index;
static INT16S g_play_index;
static INT16U g_file_num;
static INT16U g_err_cnt;
static INT16U g_same_index_num;
static INT16S g_latest_avi_file_index;
static INT16S g_latest_jpg_file_index;
static INT32U g_avi_file_time = 0;
static INT32U g_jpg_file_time;
static INT8U g_avi_index_9999_exist;
static INT32U g_avi_file_oldest_time = 0;
static INT16S g_oldest_avi_file_index;
static CHAR g_file_path[24];
static CHAR g_next_file_path[24];
#if GPS_TXT
static CHAR g_txt_path[24];
static CHAR g_next_txt_path[24];
#endif
static INT8U curr_storage_id;
static INT8U storage_mount_timerid;
//static INT8U err_handle_timerid;
#if C_AUTO_DEL_FILE == CUSTOM_ON
	static INT8U storage_freesize_timerid;
	static INT8U first_storage_check_flag=0;
#endif

INT8U usbd_storage_exit;
INT8U device_plug_phase=0;

static INT16U avi_file_table[625];
static INT16U jpg_file_table[625];
static INT16U wav_file_table[625];
static INT32U g_wav_file_time;
static INT32U BkDelThreadMB;
static INT8U sd_upgrade_file_flag = 0;

st_storage_file_node_info FNodeInfo[MAX_SLOT_NUMS];
/*static*/ INT8U ap_step_work_start=0;
static INT32S retry_del_idx=-1;
static INT8U retry_del_counts=0;

static INT8S bkground_del_disble=0;
static INT8U card_first_dete=1;
extern volatile INT8U card_space_less_flag;

//	prototypes
INT32S get_file_final_wav_index(INT8U count_total_num_enable);
INT32S get_file_final_avi_index(INT8U count_total_num_enable);
INT32S get_file_final_jpeg_index(INT8U count_total_num_enable);
INT16U get_deleted_file_number(void);
INT16U get_same_index_file_number(void);
extern void gp_sd_size_set(INT32U Val);

extern void ap_browse_decode_file_date_time_1(INT32U dos_date_time,TIME_T *time_info);

extern INT32S ap_state_handling_jpeg_decode_thumbnail(STOR_SERV_PLAYINFO *info_ptr, INT32U jpg_output_addr);
extern INT32S Encode_Disp_Buf_To_Jpeg(INT32U dispAddr, INT32U jpegAddrs,INT32U jpegWidth,INT32U jpegHeight, INT32U jpegMaxVlcSize, INT32U* retVlcSize);

void ap_storage_service_init(void)
{
#if C_AUTO_DEL_FILE == CUSTOM_ON
	storage_freesize_timerid = 0xFF;
#endif
	g_play_index = -1;
	storage_mount_timerid = STORAGE_SERVICE_MOUNT_TIMER_ID;
	sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_STORAGE_CHECK, storage_mount_timerid, STORAGE_TIME_INTERVAL_MOUNT);
}

void bkground_del_disable(INT32U disable1_enable0)
{
    bkground_del_disble=disable1_enable0;
}

INT8S bkground_del_disable_status_get(void)
{
    return bkground_del_disble;
}

#if C_AUTO_DEL_FILE == CUSTOM_ON
void ap_storage_service_freesize_check_switch(INT8U type)
{
	if (type == TRUE) {
		if (storage_freesize_timerid == 0xFF) {
			storage_freesize_timerid = STORAGE_SERVICE_FREESIZE_TIMER_ID;
			sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, BKGROUND_DETECT_INTERVAL);
			first_storage_check_flag=0;
		}
	} else {
		if (storage_freesize_timerid != 0xFF) {
			sys_kill_timer(storage_freesize_timerid);
			storage_freesize_timerid = 0xFF;
		}
	}
}

void ap_storage_service_del_thread_mb_set(void)
{
	//INT8U	temp[6] = {0, 1, 2, 3, 5, 10};
	INT8U temp[4] = {0, 3, 5, 10};
    INT32U	SDC_MB_Size;

	if(ap_state_config_video_resolution_get() >= 2) { //720P and below
		BkDelThreadMB = temp[ap_state_config_record_time_get()]*100;	//100MB per minute
		SDC_MB_Size = drvl2_sd_sector_number_get()/2048;

		if (BkDelThreadMB>(SDC_MB_Size/2) || (BkDelThreadMB==0)) {
			BkDelThreadMB=100;
		}
	} else {
		BkDelThreadMB = temp[ap_state_config_record_time_get()]*200;	//200MB per minute
		SDC_MB_Size = drvl2_sd_sector_number_get()/2048;

		if (BkDelThreadMB>(SDC_MB_Size/2) || (BkDelThreadMB==0)) {
			BkDelThreadMB=200;
		}
	}
}

INT32U bkground_del_thread_size_get(void)
{
	if (bkground_del_disble != 1) {
		return BkDelThreadMB;
	} else {
		return 0;
	}
}

INT8U storage_sd_upgrade_file_flag_get(void)
{
	return sd_upgrade_file_flag;
}

void ap_storage_service_free_filesize_check()
{
	struct f_info file_info;
    //struct stat_t buf_tmp;
	INT32S nRet, ret;
	INT64U total_size, temp_size;

	total_size = 0;//vfsFreeSpace(MINI_DVR_STORAGE_TYPE);
	ret = STATUS_FAIL;
	nRet = _findfirst("*.avi", &file_info, D_ALL);
	if (nRet >= 0) {
		while (1) {
			//stat((CHAR *) file_info.f_name, &buf_tmp);
			//if(!(buf_tmp.st_mode & D_RDONLY)) {

			if(gp_strncmp((INT8S*)file_info.f_name, (INT8S *)"MOVI", 4) == 0) {
				if(!(file_info.f_attrib & _A_RDONLY)) {	//modified by wwj, stat() spent too long time
					total_size += file_info.f_size;
					temp_size = total_size >> 20;
					if(temp_size >= CARD_FULL_SIZE_RECORD) {
						ret = STATUS_OK;
						break;
					}
				}
			}

			nRet = _findnext(&file_info);
			if (nRet < 0) {
				break;
			}
		}
	}
	msgQSend(ApQ, MSG_APQ_FREE_FILESIZE_CHECK_REPLY, &ret, sizeof(INT32S), MSG_PRI_NORMAL);
}

extern void gp_sd_size_set(INT32U Val);
INT32S ap_storage_service_freesize_check_and_del(void)
{
	CHAR f_name[24];
#if GPS_TXT
	CHAR f_name1[24];
#endif
	INT32U i, j;
	INT32S del_index,ret;
	INT64U  disk_free_size;
    INT32S  step_ret;   
    INT16S del_ret;
    struct stat_t buf_tmp;

    ret = STATUS_OK;
    if(storage_freesize_timerid == 0xff) return ret;

    if(ap_step_work_start==0)
    {
		disk_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20;
		ap_storage_service_del_thread_mb_set();
		gp_sd_size_set(disk_free_size);
RE_DEL:
        DBG_PRINT("\r\n[Bkgnd Del Detect (DskFree: %d MB)]\r\n",disk_free_size);

        if (bkground_del_disble == 1) {
			if(first_storage_check_flag == 0)
				{
			     sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, STORAGE_TIME_INTERVAL_FREESIZE);//
			     first_storage_check_flag=1;
				}
            if (disk_free_size <= 50) {
                sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, 128);
                // if sdc redundant size less than CARD_FULL_SIZE_RECORD, STOP recording now.
                if(disk_free_size < CARD_FULL_SIZE_RECORD) {
                    AviPacker_Break_Set(pAviEncPara->AviPackerCur->avi_workmem, 1);
                    msgQSend(ApQ, MSG_APQ_VDO_REC_STOP, NULL, NULL, MSG_PRI_NORMAL);
                    sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, BKGROUND_DETECT_INTERVAL);
                }
            } else if (current_movie_Byte_size_get(pAviEncPara->AviPackerCur->avi_workmem)>AVI_REC_MAX_BYTE_SIZE) {
                DBG_PRINT ("AVI Max Size:%d MB Attend\r\n",current_movie_Byte_size_get(pAviEncPara->AviPackerCur->avi_workmem)>>20);
                msgQSend(ApQ, MSG_APQ_VDO_REC_RESTART, NULL, NULL, MSG_PRI_NORMAL);
            }
            return 0;
        }

    	if (disk_free_size <= BkDelThreadMB) {
            if(g_avi_index_9999_exist) {
			    del_index = g_oldest_avi_file_index;
				for(i=0; i<10000; i++) {
					if (avi_file_table[del_index/16] & (1<<(del_index%16))) {
						sprintf((char *)f_name, (const char *)"MOVI%04d.avi", del_index);
						stat(f_name, &buf_tmp);
						if(buf_tmp.st_mode != D_RDONLY) {
    						break;
    					}
					}
					del_index += 1;
					if(del_index > 9999) {
						del_index = 0;
					}
				}

				if(i==10000) {
					del_index = -1;
				} else {
					g_oldest_avi_file_index = del_index + 1;
					if(g_oldest_avi_file_index > 9999) {
						g_oldest_avi_file_index = 0;
					}
				}
		    } else {
    		    del_index = -1;
    		    for (i=0 ; i<AP_STG_MAX_FILE_NUMS ; i++) {
        			if (avi_file_table[i]) {
        				for (j=0 ; j<16 ; j++) {
        					if (avi_file_table[i] & (1<<j)) {
        						del_index = (i << 4) + j;
        						sprintf((char *)f_name, (const char *)"MOVI%04d.avi", del_index);
        						del_ret = stat(f_name, &buf_tmp);
        						if((del_ret != 0) || (buf_tmp.st_mode == D_RDONLY)) {
        							del_index = -1;
        						} else {
	        						break;
	        					}
        					}
        				}
        				if (del_index != -1) {
        					break;
        				}
        			}
    		    }
            }
  
            if (del_index == -1 || gp_strcmp((INT8S*)f_name,(INT8S*)g_file_path) == 0) {
        		if (disk_free_size <= 50) {
        		    msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, NULL, NULL, MSG_PRI_NORMAL);
        			bkground_del_disble = 1;
        		} else if(disk_free_size < 200) {
	                sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, 128);
        		}
                return STATUS_FAIL;
            }
    		DBG_PRINT("\r\nDel <%s>\r\n", f_name);

#if GPS_TXT
			gp_memcpy((INT8S *)f_name1, (INT8S *)f_name, 24);
			f_name1[9] = 't'; f_name1[10] = 'x'; f_name1[11] = 't'; f_name1[12] = 0;
			unlink(f_name1);
#endif
            unlink_step_start();
            del_ret = unlink(f_name);
    		if (del_ret< 0) {
                if (retry_del_idx<0) {
                    retry_del_idx = del_index;
                } else {
                    retry_del_counts++;
                }

                if (retry_del_counts > 2) {
                    retry_del_idx = -1;  // reset retry index
                    retry_del_counts = 0;  // reset retry counts
    			    avi_file_table[del_index >> 4] &= ~(1 << (del_index & 0xF));
    			    ///g_file_num--;
                    DBG_PRINT("Del Fail, avoid\r\n");
                } else {
                    DBG_PRINT("Del Fail, retry\r\n");
                }

    			ret = STATUS_FAIL;
                sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, 128);
    		} else {
    		    retry_del_idx = -1;  // reset retry index
    		    retry_del_counts = 0;  // reset retry counts
    		    ap_step_work_start=1;
                sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, BKGROUND_DEL_INTERVAL);
    			avi_file_table[del_index >> 4] &= ~(1 << (del_index & 0xF));
    			g_file_num--;
    			DBG_PRINT("Step Del Init OK\r\n");

                if(!g_avi_index_9999_exist) {
					g_oldest_avi_file_index = del_index + 1;
					if(g_oldest_avi_file_index > 9999) {
						g_oldest_avi_file_index = 0;
					}
                }
		    }

            if (disk_free_size <= 70ULL) {
                unlink_step_flush();
                ap_step_work_start = 0;
                if ((vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20) <70ULL) {
                    DBG_PRINT ("Re-Delete\r\n");
                    goto RE_DEL;
                }
            }
            ret = STATUS_OK;
        }
    } else {
        step_ret = unlink_step_work();
        if(step_ret != 0) {
            sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, BKGROUND_DEL_INTERVAL);
            //DBG_PRINT ("StepDel Continue\r\n");
        } else {
            ap_step_work_start = 0;
            DBG_PRINT ("StepDel Done\r\n");
            disk_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20;
            gp_sd_size_set(disk_free_size);
            DBG_PRINT("\r\n[Dominant (DskFree: %d MB)]\r\n", disk_free_size);
            if (disk_free_size > BkDelThreadMB) {
                sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, BKGROUND_DETECT_INTERVAL);
            } else {
                sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK, storage_freesize_timerid, 128);
            }
        }
    }
	return ret;
}

#endif

void ap_storage_service_file_del(INT32U idx)
{
	INT32S ret, i;
	INT32U max_temp;
	struct stat_t buf_tmp;
	INT64U  disk_free_size;

	if (idx == 0xFFFFFFFF) {
#if C_AUTO_DEL_FILE == CUSTOM_ON	
		ret = ap_storage_service_freesize_check_and_del();
#endif		
	} else {
		close(play_info.file_handle);

		stat(g_file_path, &buf_tmp);
		if(buf_tmp.st_mode & D_RDONLY){
			ret = 0x55;
			msgQSend(ApQ, MSG_APQ_SELECT_FILE_DEL_REPLY, &ret, sizeof(INT32S), MSG_PRI_NORMAL);
			return;
		}
		if (unlink(g_file_path) < 0) {
			DBG_PRINT("Delete file fail.\r\n");
			ret = STATUS_FAIL;
		} else {
			DBG_PRINT("Delete file OK.\r\n");
#if GPS_TXT
			gp_memcpy((INT8S *)g_txt_path, (INT8S *)g_file_path, sizeof(g_file_path));
			g_txt_path[9] = 't'; g_txt_path[10] = 'x'; g_txt_path[11] = 't'; g_txt_path[12] = 0;
			unlink(g_txt_path);
#endif
			g_jpeg_index = 0;
            g_avi_index = 0;
            g_file_index = 0;
            g_file_num = 0;
            g_same_index_num = 0;
			g_wav_index = 0;
			//g_play_index = -1;	//Daniel marked for returning to previous one after deleting the current one
			for (i=0 ; i<AP_STG_MAX_FILE_NUMS ; i++) {
				avi_file_table[i] = 0;
				jpg_file_table[i] = 0;
				wav_file_table[i] = 0;
			}
			get_file_final_avi_index(1);
			get_file_final_jpeg_index(1);
			get_file_final_wav_index(1);

			if(g_avi_index_9999_exist)
			{
				if(g_avi_file_time > g_jpg_file_time)
				{
                    max_temp = g_avi_index;
					if (g_avi_file_time > g_wav_file_time) {
                    	g_file_index = max_temp;
					}else{
                    	g_file_index = g_wav_index;
                	}
				} else {
                    max_temp = g_jpeg_index;
					if (g_jpg_file_time > g_wav_file_time) {
                    	g_file_index = max_temp;
					}else{
                    	g_file_index = g_wav_index;
                	}                        
				}
			} else {
                if (g_avi_index > g_jpeg_index) {
                    max_temp = g_avi_index;
				} else {
                    max_temp = g_jpeg_index;
                }

                if (max_temp > g_wav_index) {
                    g_file_index = max_temp;
				} else {
                    g_file_index = g_wav_index;
				}
			}
			ret = STATUS_OK;
		}
		g_same_index_num = get_same_index_file_number();
		msgQSend(ApQ, MSG_APQ_SELECT_FILE_DEL_REPLY, &ret, sizeof(INT32S), MSG_PRI_NORMAL);
	}

	disk_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20;
	gp_sd_size_set(disk_free_size);
}

void ap_storage_service_file_delete_all(void)
{
	struct f_info file_info;
	//struct stat_t buf_tmp;
	INT8U	locked_files_exist = 0;
	INT32S nRet, i, ret;
	INT32U max_temp;
	INT64U  disk_free_size;

//check if there is any locked video file
	nRet = _findfirst("*.avi", &file_info, D_ALL);
	if (nRet < 0) {
		locked_files_exist = 0;
	}
	else
	{
		while (1) 
		{
			//stat((CHAR *) file_info.f_name, &buf_tmp);
			//if(buf_tmp.st_mode & D_RDONLY) {		//skip if it's locked
			if(file_info.f_attrib & _A_RDONLY) {	//modified by wwj, stat() spent too long time
				locked_files_exist = 1;
				break;
			} 
			nRet = _findnext(&file_info);
			if (nRet < 0) {
				locked_files_exist = 0;
				break;
			}
			continue;
		}
	}

    g_file_index = 0;
    g_file_num = 0;
    g_wav_index = 0;
	for (i=0 ; i<AP_STG_MAX_FILE_NUMS ; i++) {
		avi_file_table[i] = 0;
		jpg_file_table[i] = 0;
		wav_file_table[i] = 0;
	}
	g_play_index = -1;
	g_avi_index_9999_exist = g_avi_file_time = g_avi_file_oldest_time = 0;
//AVI	
    g_avi_index = -1;
	nRet = _findfirst("*.avi", &file_info, D_ALL);
	if (nRet < 0) {
		g_avi_index++;
	}
	else
	{
		g_avi_index = 0;
		while (1) 
		{
			//stat((CHAR *) file_info.f_name, &buf_tmp);
			//if(buf_tmp.st_mode & D_RDONLY) {		//skip if it's locked
			if(file_info.f_attrib & _A_RDONLY) {	//modified by wwj, stat() spent too long time
				nRet = _findnext(&file_info);
				if (nRet < 0) {
					break;
				}
				continue;
			}

			unlink((CHAR *) file_info.f_name);
#if GPS_TXT
			gp_memcpy((INT8S *)g_txt_path, (INT8S *)file_info.f_name, sizeof(g_txt_path));
			g_txt_path[9] = 't'; g_txt_path[10] = 'x'; g_txt_path[11] = 't'; g_txt_path[12] = 0;
			unlink(g_txt_path);
#endif
			//if (unlink((CHAR *) file_info.f_name) == 0) 
			//{
				nRet = _findnext(&file_info);
				if (nRet < 0) {
					break;
				}
				//continue;
			//}
		}
	}
//JPEG	
	g_jpeg_index = -1;
	g_jpg_file_time = 0;
	nRet = _findfirst("*.jpg", &file_info, D_ALL);
	if (nRet < 0) {
		g_jpeg_index++;
	}
	else
	{
		g_jpeg_index = 0;
		while (1) 
		{
			unlink((CHAR *) file_info.f_name);
			//if (unlink((CHAR *) file_info.f_name)==0) 
			//{
				nRet = _findnext(&file_info);
				if (nRet < 0) {
					break;
				}
				//continue;
			//}
		}
	}

//WAV
 	g_wav_index = -1;
	g_wav_file_time = 0;

	nRet = _findfirst("*.wav", &file_info, D_ALL);
	if (nRet < 0) {
		g_wav_index++;
	}
	else
	{
		g_wav_index = 0;
		while (1) 
		{
			unlink((CHAR *) file_info.f_name);
			//if (unlink((CHAR *) file_info.f_name)==0) 
			//{
				nRet = _findnext(&file_info);
				if (nRet < 0) {
					break;
				}
				//continue;
			//}
		}
	}
//Scan again if locked_files_exist
	if(locked_files_exist)
	{
		get_file_final_avi_index(1);
		get_file_final_jpeg_index(1);
		get_file_final_wav_index(1);

        if(g_avi_index_9999_exist)
        {
			if(g_avi_file_time > g_jpg_file_time)
			{
				max_temp = g_avi_index;
				if (g_avi_file_time > g_wav_file_time) {
					g_file_index = max_temp;
				} else {
					g_file_index = g_wav_index;
				}
			}
			else
			{
                max_temp = g_jpeg_index;
				if (g_jpg_file_time > g_wav_file_time) {
					g_file_index = max_temp;
				} else {
					g_file_index = g_wav_index;
				}                        
			}
		}
        else
        {
			if (g_avi_index > g_jpeg_index) {
				max_temp = g_avi_index;
			} else {
				max_temp = g_jpeg_index;
			}

			if (max_temp>g_wav_index) {
				g_file_index = max_temp;
			} else {
				g_file_index = g_wav_index;
			}
		}	
	}	
	
	ret = STATUS_OK;
	g_same_index_num = 0;
	msgQSend(ApQ, MSG_APQ_FILE_DEL_ALL_REPLY, &ret, sizeof(INT32S), MSG_PRI_NORMAL);

	disk_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20;
	gp_sd_size_set(disk_free_size);
}

void ap_storage_service_file_lock_one(void)
{
	INT32S ret;

	if ( (avi_file_table[g_play_index >> 4] & (1 << (g_play_index & 0xF)))) {	//only lock video files
	  #if RENAME_LOCK_FILE
		CHAR temp_file_name[24];
	  #endif

		_setfattr(g_file_path, D_RDONLY);

	  #if RENAME_LOCK_FILE
		gp_memcpy((INT8S *)temp_file_name, (INT8S *)g_file_path, sizeof(temp_file_name));
		#if !LOCK_FILE_NAME
		temp_file_name[0] = 'L'; temp_file_name[1] = 'O'; temp_file_name[2] = 'C'; temp_file_name[3] = 'K';
		#else
		temp_file_name[0] = 'S'; temp_file_name[1] = 'O'; temp_file_name[2] = 'S'; temp_file_name[3] = '0';
		#endif
		_rename((char *)g_file_path, temp_file_name);
		gp_memcpy((INT8S *)g_file_path, (INT8S *)temp_file_name, sizeof(temp_file_name));
	  #endif
	}
	ret = STATUS_OK;
	msgQSend(ApQ, MSG_APQ_FILE_LOCK_ONE_REPLY, &ret, sizeof(INT32S), MSG_PRI_NORMAL);	
}

void ap_storage_service_file_lock_all(void)
{
	struct f_info file_info;
	INT32S nRet, ret;

	nRet = _findfirst("*.avi", &file_info, D_ALL);
	if (nRet < 0) {
		ret = STATUS_FAIL;
	}
	else
	{
		while (1) 
		{
		  #if RENAME_LOCK_FILE
			CHAR temp_file_name[24];
		  #endif

			_setfattr((CHAR *) file_info.f_name, D_RDONLY);

		  #if RENAME_LOCK_FILE
			gp_memcpy((INT8S *)temp_file_name, (INT8S *)file_info.f_name, sizeof(temp_file_name));
			#if !LOCK_FILE_NAME
			temp_file_name[0] = 'L'; temp_file_name[1] = 'O'; temp_file_name[2] = 'C'; temp_file_name[3] = 'K';
			#else
			temp_file_name[0] = 'S'; temp_file_name[1] = 'O'; temp_file_name[2] = 'S'; temp_file_name[3] = '0';
			#endif
			_rename((char *)file_info.f_name, temp_file_name);
		  #endif

			nRet = _findnext(&file_info);
			if (nRet < 0) {
				break;
			}
			continue;
		}
	}
	ret = STATUS_OK;
	msgQSend(ApQ, MSG_APQ_FILE_LOCK_ALL_REPLY, &ret, sizeof(INT32S), MSG_PRI_NORMAL);
}

void ap_storage_service_file_unlock_one(void)
{
	INT32S ret;
	if ( (avi_file_table[g_play_index >> 4] & (1 << (g_play_index & 0xF)))){	//only unlock video files
	  #if RENAME_LOCK_FILE
		CHAR temp_file_name[24];
	  #endif

		_setfattr(g_file_path, D_NORMAL);

	  #if RENAME_LOCK_FILE
		gp_memcpy((INT8S *)temp_file_name, (INT8S *)g_file_path, sizeof(temp_file_name));
		temp_file_name[0] = 'M'; temp_file_name[1] = 'O'; temp_file_name[2] = 'V'; temp_file_name[3] = 'I';
		_rename((char *)g_file_path, temp_file_name);
		gp_memcpy((INT8S *)g_file_path, (INT8S *)temp_file_name, sizeof(temp_file_name));
	  #endif
	}
	ret = STATUS_OK;
	msgQSend(ApQ, MSG_APQ_FILE_UNLOCK_ONE_REPLY, &ret, sizeof(INT32S), MSG_PRI_NORMAL);
}

void ap_storage_service_file_unlock_all(void)
{
	struct f_info file_info;
	INT32S nRet, ret;
  #if RENAME_LOCK_FILE
	CHAR temp_file_name[24];
  #endif

	nRet = _findfirst("*.avi", &file_info, D_ALL);
	if (nRet < 0) {
		ret = STATUS_FAIL;
	}
	else
	{
		while (1) 
		{
			_setfattr((CHAR *) file_info.f_name, D_NORMAL);

		  #if RENAME_LOCK_FILE
			gp_memcpy((INT8S *)temp_file_name, (INT8S *)file_info.f_name, sizeof(temp_file_name));
			temp_file_name[0] = 'M'; temp_file_name[1] = 'O'; temp_file_name[2] = 'V'; temp_file_name[3] = 'I';
			_rename((char *)file_info.f_name, temp_file_name);
		  #endif

			nRet = _findnext(&file_info);
			if (nRet < 0) {
				break;
			}
			continue;
		}
	}
	ret = STATUS_OK;
	msgQSend(ApQ, MSG_APQ_FILE_UNLOCK_ALL_REPLY, &ret, sizeof(INT32S), MSG_PRI_NORMAL);
}

void ap_storage_service_timer_start(void)
{
	if (storage_mount_timerid == 0xff) {
		storage_mount_timerid = STORAGE_SERVICE_MOUNT_TIMER_ID;
		sys_set_timer((void*)msgQSend, (void*)StorageServiceQ, MSG_STORAGE_SERVICE_STORAGE_CHECK, storage_mount_timerid, STORAGE_TIME_INTERVAL_MOUNT);
	}
}

void ap_storage_service_timer_stop(void)
{
	if (storage_mount_timerid != 0xff) {
		sys_kill_timer(storage_mount_timerid);
		storage_mount_timerid = 0xff;
	}
}

void ap_storage_service_usb_plug_in(void)
{
	device_plug_phase = 0;
}

static INT8U g_updata_flag = 0;
#define OSD_SWITCH_MODE 0
static INT32S save_example_time_to_disk(void)
{
	INT16S fd;
	INT32U addr;
	INT8U *p;
	
	return STATUS_OK;
	
	#if 0
	fd = open("C:\\TAG.TXT", O_RDWR|O_TRUNC|O_CREAT);
	#else
	fd = open("C:\\time.txt", O_RDWR|O_TRUNC|O_CREAT);
	#endif
	if(fd < 0) return STATUS_FAIL;

	addr = (INT32U)gp_malloc(20+2);
	if(!addr)
	{
		close(fd);
		return STATUS_FAIL;
	}
		
	//gp_strcpy((INT8S*)video_info->AudSubFormat, (INT8S *)"adpcm");
	p = (INT8U*)addr;
	sprintf((char *)p, (const char *)"2016-10-01 23:59:59   ");
	#if OSD_SWITCH_MODE
	if (ap_storage_service_osd_flag_get() == 1) p[20] = 'Y'; 
	else p[20] = 'N';
	write(fd, addr, 20+2);
	#else
	write(fd, addr, 20);
	#endif
	close(fd);
	gp_free((void*) addr);
	
	return STATUS_OK;
}

extern void save_COPYRIGHT_MESSAGE_to_disk(void);
extern void save_VERSION_NUMBER_to_disk(void);
extern void OpenPrintMsgSet(INT8U flag);
static INT32S rtc_time_get_and_start(void)
{
	INT8U  read_flag=0;
	INT8U  data;
	INT8U  *pdata;
	INT16S fd;
	INT16U wtemp;
	INT32U addr;
	INT32S nRet;
	TIME_T	time_set;
	
	#if 0
	fd = open("C:\\TAG.TXT", O_RDONLY);
	#else
	fd = open("C:\\time.txt", O_RDONLY);
	#endif

	if(fd < 0)
	{
		//DBG_PRINT("OPEN time.txt FAIL!!!!!\r\n");
		goto Fail_Return_3;
	}
	 	
	addr = (INT32U)gp_malloc(20+3);
	if(!addr)
    {
		goto Fail_Return_2;
	}	
    else
	{
		nRet = read(fd, addr, 20+2);
		if(nRet <= 0) goto Fail_Return;
	}
		
	read_flag=1;
	//-------------------------------设置OSD显示---------------------------------
	/*
	#if OSD_SWITCH_MODE
	if (gp_strncmp((INT8S*)(addr+19), (INT8S *)" N",2) == 0) ap_storage_service_osd_flag_set(0);
	else if (gp_strncmp((INT8S*)(addr+19), (INT8S *)" Y",2) == 0) ap_storage_service_osd_flag_set(1);
	else g_updata_flag = 1;
	#endif
	*/
	//-------------------------------设置OSD显示---------------------------------
	
	nRet = gp_strncmp((INT8S*)addr, (INT8S *)"COPYFIGHT MESSAGE? ", 19);
	if (nRet==0) { save_COPYRIGHT_MESSAGE_to_disk(); return STATUS_FAIL; }
	nRet = gp_strncmp((INT8S*)addr, (INT8S *)"VERSION_NUMBER??", 16);
	if (nRet==0) { save_VERSION_NUMBER_to_disk(); return STATUS_FAIL; }
	nRet = gp_strncmp((INT8S*)addr, (INT8S *)"SENSOR?", 7);
	if (nRet==0) { OpenPrintMsgSet(1); return STATUS_FAIL; }
	
	nRet = gp_strncmp((INT8S*)addr, (INT8S *)"2016-10-01 23:59:59 ", 19);	//返回0表示参数1和参数2的内容完全相同;
	if (nRet==0)
	{
		if (g_updata_flag == 1) goto Fail_Return_3;
		goto Fail_Return;
	}
	read_flag=0;

	pdata = (INT8U*)addr;
	//year
	wtemp = 0;
	data = *pdata++;
	data -= 0x30;
	wtemp += data*1000;
	
	data = *pdata++;
	data -= 0x30;
	wtemp += data*100;
	
	data = *pdata++;
	data -= 0x30;
	wtemp += data*10;
	
	data = *pdata++;
	data -= 0x30;
	wtemp += data*1;
	if((wtemp > 2026) || (wtemp < 2016)) goto Fail_Return;
	time_set.tm_year = wtemp;
	
	//skip -		
	pdata++;	
	
	//month
	wtemp = 0;
	data = *pdata++;
	data -= 0x30;
	wtemp += data*10;
	
	data = *pdata++;
	data -= 0x30;
	wtemp += data*1;
	if(wtemp>12) goto Fail_Return;
	time_set.tm_mon = wtemp;
			
	//skip -		
	pdata++;
	
	//day
	wtemp = 0;
	data = *pdata++;
	data -= 0x30;
	wtemp += data*10;
	
	data = *pdata++;
	data -= 0x30;
	wtemp += data*1;
	if(wtemp>31) goto Fail_Return;
	time_set.tm_mday = wtemp;
	
	//skip space		
	pdata++;
	
	//hour
	wtemp = 0;
	data = *pdata++;
	data -= 0x30;
	wtemp += data*10;
	
	data = *pdata++;
	data -= 0x30;
	wtemp += data*1;
	if(wtemp>23) goto Fail_Return;
	time_set.tm_hour = wtemp;
			
	//skip :	
	pdata++;
			
	//minute
	wtemp = 0;
	data = *pdata++;
	data -= 0x30;
	wtemp += data*10;
	
	data = *pdata++;
	data -= 0x30;
	wtemp += data*1;
	if(wtemp>59) goto Fail_Return;
	time_set.tm_min = wtemp;
	
	//skip :	
	pdata++;
			
	//second
	wtemp = 0;
	data = *pdata++;
	data -= 0x30;
	wtemp += data*10;
	
	data = *pdata++;
	data -= 0x30;
	wtemp += data*1;
	if(wtemp>59) goto Fail_Return;
	time_set.tm_sec = wtemp;
	
	if(fd>=0) close(fd);
	gp_free((void*) addr);
	cal_time_set(time_set);
	ap_state_handling_calendar_init();
	cal_time_get(&time_set);

	save_example_time_to_disk();
	return STATUS_OK;
	
	Fail_Return:
		gp_free((void*) addr);
	Fail_Return_2:
		if(fd>=0) close(fd);
	Fail_Return_3:
	ap_state_handling_calendar_init();
	cal_time_get(&time_set);
	if( read_flag == 0)
	{
	 	save_example_time_to_disk();
	}
	unlink("C:\\time.txt");
	//---------------------------------------------------------------------
	if (g_updata_flag == 1) 
	{
		g_updata_flag = 0;
		save_example_time_to_disk();  //OSD设置格式错误时重新生成格式文件
	}
	//---------------------------------------------------------------------

	return STATUS_FAIL;

}

#ifdef SDC_DETECT_PIN
extern INT32S ap_peripheral_SDC_at_plug_OUT_detect(void);
extern INT32S ap_peripheral_SDC_at_plug_IN_detect(void);
#endif
extern void gp_sd_size_set(INT32U Val);
extern void ap_sd_status_set(INT8U flag);
INT32S ap_storage_service_storage_mount(void)
{
	INT32S nRet, i,led_type= -1;
	INT32U size, max_temp;
	INT16S fd;

	if(storage_mount_timerid == 0xff) return	STATUS_FAIL;
	//DBG_PRINT("MOUNT\r\n");
	if (device_plug_phase == 0) {
		ap_storage_service_timer_stop();				//prohibit too many MSG_STORAGE_SERVICE_STORAGE_CHECK sent during sd mounting
#ifndef SDC_DETECT_PIN
		nRet = _devicemount(MINI_DVR_STORAGE_TYPE);
#else
		nRet = ap_peripheral_SDC_at_plug_OUT_detect();
//		DBG_PRINT("nRet= %d\r\n",nRet);
		if(!nRet){
			 _devicemount(MINI_DVR_STORAGE_TYPE);
		}
#endif
	} else {
#ifndef SDC_DETECT_PIN
		nRet = drvl2_sdc_live_response();
#else
		nRet = ap_peripheral_SDC_at_plug_IN_detect();
#endif
		if(nRet != 0) {
			ap_storage_service_timer_stop();				//prohibit too many MSG_STORAGE_SERVICE_STORAGE_CHECK sent during sd mounting
			nRet = _devicemount(MINI_DVR_STORAGE_TYPE);
		}
	}

	if (nRet < 0) {
		device_plug_phase = 0;  // plug out phase
		card_first_dete=2;

		if (curr_storage_id != NO_STORAGE) {
			//INT16S ret;

			// add by josephhsieh@140717 // 璝棵辊玂臔秨璶翴獹璉
			if(screen_saver_enable) {
				screen_saver_enable = 0;
	        		ap_state_handling_lcd_backlight_switch(1);
			}
			#if (defined RAMDISK_EN) && (RAMDISK_EN == 1)
			_devicemount(RAMDISK_TYPE);
			ret = sformat(RAMDISK_TYPE, 32*1024*1024/512, C_RAMDISK_SIZE/512);
			if(ret != 0) {
				DBG_PRINT("RAM disk format fail...\r\n");
			}

			mkdir("D:\\DCIM");
			chdir("D:\\DCIM");
			#endif
			g_jpeg_index = g_avi_index = g_wav_index = g_file_index = 0;
			g_file_num = 0;
			for (i=0 ; i<625 ; i++) {
				avi_file_table[i] = jpg_file_table[i] = wav_file_table[i] = 0;
			}
			card_space_less_flag =0;

			curr_storage_id = NO_STORAGE;
			msgQSend(ApQ, MSG_STORAGE_SERVICE_NO_STORAGE, &curr_storage_id, sizeof(INT8U), MSG_PRI_NORMAL);
		} else {
			chdir("D:\\DCIM");
		}

		ap_storage_service_timer_start();		//enable again
		return STATUS_FAIL;
	} else {
	  
	  if(card_first_dete)
				 {
				  card_space_less_flag =0;
				  if(card_first_dete ==1)
				  led_type=LED_CARD_DETE_SUC;
				  else
				  led_type=LED_WAITING_RECORD;	 
				  ap_sd_status_set(2);
				  card_first_dete=0;
				  //msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
				 }
	  
        device_plug_phase = 1;  // plug in phase
		
		if (curr_storage_id != MINI_DVR_STORAGE_TYPE) {

			// add by josephhsieh@140717 // 璝棵辊玂臔秨璶翴獹璉
			if(screen_saver_enable) {
				screen_saver_enable = 0;
	        		ap_state_handling_lcd_backlight_switch(1);
			}

			if(curr_storage_id == NO_STORAGE) {
			#if (defined RAMDISK_EN) && (RAMDISK_EN == 1)
				_deviceunmount(RAMDISK_TYPE);
			#endif
			}
			size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE)>>20;
			gp_sd_size_set(size);
			DBG_PRINT("Mount OK, free size [%d]\r\n", size);
			curr_storage_id = MINI_DVR_STORAGE_TYPE;
			if (size < CARD_FULL_MB_SIZE)
			{
            	  led_type=LED_CARD_NO_SPACE;	
			
			}
			else
				card_space_less_flag =0;

			rtc_time_get_and_start();
			mkdir("C:\\DCIM");
			chdir("C:\\DCIM");
			g_jpeg_index = g_avi_index = g_wav_index = g_file_index = 0;
			g_file_num = 0;
			for (i=0 ; i<625 ; i++) {
				avi_file_table[i] = jpg_file_table[i] = wav_file_table[i] = 0;
			}
			get_file_final_avi_index(1);
			get_file_final_jpeg_index(1);
			get_file_final_wav_index(1);

			if(g_avi_index_9999_exist)
			{
				if(g_avi_file_time > g_jpg_file_time)
				{
                    max_temp = g_avi_index;
					if (g_avi_file_time > g_wav_file_time) {
                    	g_file_index = max_temp;
					}else{
                    	g_file_index = g_wav_index;
                	}
				} else {
                    max_temp = g_jpeg_index;
					if (g_jpg_file_time > g_wav_file_time) {
                    	g_file_index = max_temp;
					}else{
                    	g_file_index = g_wav_index;
                	}                        
				}
			} else {
                if (g_avi_index > g_jpeg_index) {
                    max_temp = g_avi_index;
				} else {
                    max_temp = g_jpeg_index;
                }

                if (max_temp > g_wav_index) {
                    g_file_index = max_temp;
				} else {
                    g_file_index = g_wav_index;
				}
			}

			if (sd_upgrade_file_flag == 0) {
				INT8U  FileName_Path[3+5+8+4+1];				
				struct f_info	file_info;
				char   p[4];
				chdir("C:\\");
				//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				sprintf((char*)p,(const char*)"%04d",PRODUCT_NUM);
                 gp_strcpy((INT8S*)FileName_Path,(INT8S*)"C:\\JH_");
                 gp_strcat((INT8S*)FileName_Path,(INT8S*)p);
                 gp_strcat((INT8S*)FileName_Path,(INT8S*)"*.bin");
				//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				
				 nRet = _findfirst((CHAR*)FileName_Path, &file_info ,D_ALL); //查找bin文件
				//fd = open((CHAR*)"C:\\gp_cardvr_upgrade.bin", O_RDONLY);
				
				//if (fd < 0) {
				if (nRet < 0) {
					sd_upgrade_file_flag = 1; //no need upgrade
					chdir("C:\\DCIM");
					DBG_PRINT("\r\nTF no upgrade file!!!\r\n");
					nRet = _findfirst("C:\\DefSet.cfg", &file_info ,D_ALL); //查找bin文件
					if (nRet >= 0) 
					{
						ap_state_config_restore(); //恢复默认设置
						DBG_PRINT("\r\nRestore the default Settings!!!\r\n");
					}
				} else {
					close(fd);
					sd_upgrade_file_flag = 2; //want to upgrade
				}
			}

			ap_storage_service_del_thread_mb_set();
			msgQSend(ApQ, MSG_STORAGE_SERVICE_MOUNT, &curr_storage_id, sizeof(INT8U), MSG_PRI_NORMAL);
		} else {
			chdir("C:\\DCIM");
		}
		ap_storage_service_timer_start();		//enable again
		if(led_type != -1)
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
			
		return STATUS_OK;
	}
}

void ap_storage_service_file_open_handle(INT32U req_type)
{
	INT32U reply_type;
	STOR_SERV_FILEINFO file_info;
	INT32U files_cnt =0 ;
	
	ap_state_handling_file_creat_set(0);
	ap_storage_service_timer_stop();

	file_info.storage_free_size = (vfsFreeSpace(curr_storage_id) >> 20);

	while ((avi_file_table[g_file_index >> 4] & (1 << (g_file_index & 0xF))) ||
		(jpg_file_table[g_file_index >> 4] & (1 << (g_file_index & 0xF))) ||
		(wav_file_table[g_file_index >> 4] & (1 << (g_file_index & 0xF))) )
	{
		g_file_index++;
		if(g_file_index > 9999) {
			g_file_index = 0;
		}
		if (files_cnt>=10000) {
			DBG_PRINT("No filename can use.\r\n");	// add by josephhsieh@20160328
			return;
		}
		files_cnt++;
	}

    switch (req_type)
    {
    	case MSG_STORAGE_SERVICE_VID_REQ:
			reply_type = MSG_STORAGE_SERVICE_VID_REPLY;
			sprintf((char *)g_file_path, (const char *)"MOVI%04d.avi", g_file_index);
		    file_info.file_handle = open (g_file_path, O_WRONLY|O_CREAT|O_TRUNC);
    		
    		if (file_info.file_handle >= 0)
            {
            	ap_state_handling_file_creat_set(1);
				file_info.file_path_addr = (INT32U) g_file_path;
				avi_file_table[g_file_index >> 4] |= 1 << (g_file_index & 0xF);
				g_file_num++;
				g_file_index++;
				if(g_file_index == 10000){
					g_file_index = 0;
					g_avi_index_9999_exist = 1;
				}				
				g_avi_index = g_file_index;
				DBG_PRINT("FileName = %s\r\n", file_info.file_path_addr);

#if GPS_TXT
				gp_memcpy((INT8S *)g_txt_path, (INT8S *)g_file_path, sizeof(g_file_path));
				g_txt_path[9] = 't'; g_txt_path[10] = 'x'; g_txt_path[11] = 't'; g_txt_path[12] = 0;
				file_info.txt_path_addr = (INT32U)g_txt_path;

				F_OS_AdjustCrtTimeEnable();
				file_info.txt_handle = open(g_txt_path, O_WRONLY|O_CREAT|O_TRUNC);
				F_OS_AdjustCrtTimeDisable();
#endif
			}
            break;

        case MSG_STORAGE_SERVICE_PIC_REQ:
			reply_type = MSG_STORAGE_SERVICE_PIC_REPLY;

			if (curr_storage_id != NO_STORAGE) {
				#if ENABLE_SAVE_SENSOR_RAW_DATA
				sprintf((char *)g_file_path, (const char *)"PICT%04d.dat", g_file_index);
				#else
				sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", g_file_index);
				#endif
				file_info.file_handle = open (g_file_path, O_WRONLY|O_CREAT|O_TRUNC);
				if (file_info.file_handle >= 0) {
					ap_state_handling_file_creat_set(1);
					file_info.file_path_addr = (INT32U) g_file_path;
					jpg_file_table[g_file_index >> 4] |= 1 << (g_file_index & 0xF);
					g_file_num++;
					g_file_index++;
					if(g_file_index == 10000){
						g_file_index = 0;
						g_avi_index_9999_exist = 1;
					}
					g_jpeg_index = g_file_index;
					DBG_PRINT("FileName = %s\r\n", file_info.file_path_addr);
				}
			} else {
				g_file_index = 0;
				#if ENABLE_SAVE_SENSOR_RAW_DATA
				sprintf((char *)g_file_path, (const char *)"PICT%04d.dat", g_file_index);
				#else
				sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", g_file_index);
				#endif
				unlink(g_file_path); //because O_TRUNC is disabled
				file_info.file_path_addr = (INT32U) g_file_path;
				file_info.file_handle = open (g_file_path, O_WRONLY|O_CREAT|O_TRUNC);
				if(file_info.file_handle >= 0) {
					jpg_file_table[g_file_index >> 4] |= 1 << (g_file_index & 0xF);
					g_file_num = 1;
				}
			}
            break;

    	case MSG_STORAGE_SERVICE_AUD_REQ:
    		reply_type = MSG_STORAGE_SERVICE_AUD_REPLY;
       #if AUD_REC_FORMAT == AUD_FORMAT_WAV
	    	sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_file_index);//20110927
       #else
		    sprintf((char *)g_file_path, (const char *)"RECR%04d.mp3", g_file_index);//20110927
       #endif
    		//sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_file_index);
    		file_info.file_handle = open (g_file_path, O_WRONLY|O_CREAT|O_TRUNC);
    		if (file_info.file_handle >= 0) {
    			ap_state_handling_file_creat_set(1);
    			file_info.file_path_addr = (INT32U) g_file_path;
				wav_file_table[g_file_index >> 4] |= 1 << (g_file_index & 0xF);
				g_file_num++;
    			g_file_index++;
				if(g_file_index == 10000){
					g_file_index = 0;
					g_avi_index_9999_exist = 1;
				}
    			g_wav_index = g_file_index;
    			DBG_PRINT("FileName = %s\r\n", file_info.file_path_addr);
			}
            break;
        default:
            DBG_PRINT ("UNKNOW STORAGE SERVICE\r\n");
            break;
    }

	msgQSend(ApQ, reply_type, &file_info, sizeof(STOR_SERV_FILEINFO), MSG_PRI_NORMAL);
}


#if C_CYCLIC_VIDEO_RECORD == CUSTOM_ON

void ap_storage_service_cyclic_record_file_open_handle(INT8U type)
{
	INT32U reply_type;
	STOR_SERV_FILEINFO file_info;

	if (type == TRUE) {
		ap_storage_service_timer_stop();

		while ((avi_file_table[g_file_index >> 4] & (1 << (g_file_index & 0xF))) ||
			(jpg_file_table[g_file_index >> 4] & (1 << (g_file_index & 0xF))) ||
			(wav_file_table[g_file_index >> 4] & (1 << (g_file_index & 0xF))) )
		{
			g_file_index++;
			if(g_file_index > 9999) {
				g_file_index = 0;
			}
		}

		reply_type = MSG_STORAGE_SERVICE_VID_CYCLIC_REPLY;
		sprintf((char *)g_next_file_path, (const char *)"MOVI%04d.avi", g_file_index);
		file_info.file_handle = open (g_next_file_path, O_WRONLY|O_CREAT|O_TRUNC);
		if (file_info.file_handle >= 0) {
			file_info.file_path_addr = (INT32U) g_next_file_path;
			DBG_PRINT("FileName = %s\r\n", file_info.file_path_addr);
			avi_file_table[g_file_index >> 4] |= 1 << (g_file_index & 0xF);
			g_file_num++;
			g_file_index++;
			if(g_file_index == 10000){
				g_file_index = 0;
				g_avi_index_9999_exist = 1;
			}
			g_avi_index = g_file_index;
#if GPS_TXT
			gp_memcpy((INT8S *)g_next_txt_path, (INT8S *)g_next_file_path, sizeof(g_next_file_path));
			g_next_txt_path[9] = 't'; g_next_txt_path[10] = 'x'; g_next_txt_path[11] = 't'; g_next_txt_path[12] = 0;
			file_info.txt_path_addr = (INT32U)g_next_txt_path;

			F_OS_AdjustCrtTimeEnable();
			file_info.txt_handle = open(g_next_txt_path, O_WRONLY|O_CREAT|O_TRUNC);
			F_OS_AdjustCrtTimeDisable();
#endif
		}
		msgQSend(ApQ, reply_type, &file_info, sizeof(STOR_SERV_FILEINFO), MSG_PRI_NORMAL);
	} else {
		g_file_index--;
		if(g_file_index < 0) g_file_index = 9999;
		if(g_file_num > 0) g_file_num--;
		avi_file_table[g_file_index >> 4] &= ~(1 << (g_file_index & 0xF));
		g_avi_index = g_file_index;
	}
}
#endif



void ap_storage_service_play_req(STOR_SERV_PLAYINFO *info_ptr, INT32U req_msg)
{
	INT16S index_tmp, i = 0, k, l;
	INT8U type = (req_msg & 0xFF), err_flag = ((req_msg & 0xFF00) >> 8);
	INT16U given_play_index = ((req_msg>>16) & 0xFFFF);
	struct stat_t buf_tmp;

	ap_storage_service_timer_stop();

	info_ptr->search_type = type;

	#if DUAL_STREAM_FUNC_ENABLE
	if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
	{
		g_play_index = given_play_index;
	}
	#endif

	
	if (type == STOR_SERV_SEARCH_INIT) {
		g_play_index = -1;
	}
	if (g_play_index < 0) {
		get_file_final_avi_index(0);
		get_file_final_jpeg_index(0);
		get_file_final_wav_index(0);
		if(g_avi_index_9999_exist){
			if(g_avi_file_time > g_jpg_file_time)
			{
				if(g_avi_file_time > g_wav_file_time)
				{
					info_ptr->file_type = TK_IMAGE_TYPE_MOTION_JPEG;
					g_play_index = g_avi_index - 1;
					if(g_play_index < 0) {
						g_play_index = 9999;
					}
					sprintf((char *)g_file_path, (const char *)"MOVI%04d.avi", g_play_index);
				} else {
					info_ptr->file_type = TK_IMAGE_TYPE_WAV;
					g_play_index = g_wav_index - 1;
					if(g_play_index < 0) {
						g_play_index = 9999;
					}
					sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_play_index);				
				}
			}
			else
			{
				if(g_jpg_file_time > g_wav_file_time)
				{
					info_ptr->file_type = TK_IMAGE_TYPE_JPEG;
					g_play_index = g_jpeg_index - 1;
					if(g_play_index < 0) {
						g_play_index = 9999;
					}
					sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", g_play_index);
				}else{
					info_ptr->file_type = TK_IMAGE_TYPE_WAV;
					g_play_index = g_wav_index - 1;
					if(g_play_index < 0) {
						g_play_index = 9999;
					}
					sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_play_index);
				
				}
			}
		} else {
			if (g_jpeg_index > g_avi_index) 
			{
				if(g_jpeg_index > g_wav_index)
				{
					info_ptr->file_type = TK_IMAGE_TYPE_JPEG;
					g_play_index = g_jpeg_index - 1;
					if(g_play_index < 0) {
						g_play_index = 9999;
					}
					sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", g_play_index);
				}else{
					info_ptr->file_type = TK_IMAGE_TYPE_WAV;
					g_play_index = g_wav_index - 1;
					if(g_play_index < 0) {
						g_play_index = 9999;
					}
					sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_play_index);				
				}
			} else if (g_jpeg_index < g_avi_index) {
				if(g_avi_index > g_wav_index)
				{
					info_ptr->file_type = TK_IMAGE_TYPE_MOTION_JPEG;
					g_play_index = g_avi_index - 1;
					if(g_play_index < 0) {
						g_play_index = 9999;
					}
					sprintf((char *)g_file_path, (const char *)"MOVI%04d.avi", g_play_index);
				}else{
					info_ptr->file_type = TK_IMAGE_TYPE_WAV;
					g_play_index = g_wav_index - 1;
					if(g_play_index < 0) {
						g_play_index = 9999;
					}
					sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_play_index);				
				}
			} else if(g_wav_index) {
				info_ptr->file_type = TK_IMAGE_TYPE_WAV;
				g_play_index = g_wav_index - 1;
				if(g_play_index < 0) {
					g_play_index = 9999;
				}
				sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_play_index);			
			} else {
				info_ptr->err_flag = STOR_SERV_NO_MEDIA;
				msgQSend(ApQ, MSG_STORAGE_SERVICE_BROWSE_REPLY, info_ptr, sizeof(STOR_SERV_PLAYINFO), MSG_PRI_NORMAL);
				return;
			}
		}
	} 
	else
	{
		if (type == STOR_SERV_SEARCH_PREV) {
			INT8U flag = 0; //wwj add

			index_tmp = g_play_index - 1;
			if (index_tmp < 0) {
				index_tmp = 9999;
			}
			k = index_tmp >> 4;
			l = index_tmp & 0xF;
			while (i <= 626) {
				i++;
				if (avi_file_table[k] || jpg_file_table[k] || wav_file_table[k]) {
					for ( ; l>=0 ; l--) {
						if (avi_file_table[k] & (1<<l)) {
							g_play_index = (k << 4) + l;
							info_ptr->file_type = TK_IMAGE_TYPE_MOTION_JPEG;
							sprintf((char *)g_file_path, (const char *)"MOVI%04d.avi", g_play_index);
							flag = 1;  //wwj add
							break;
						}
						if (jpg_file_table[k] & (1<<l)) {
							g_play_index = (k << 4) + l;
							info_ptr->file_type = TK_IMAGE_TYPE_JPEG;
							sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", g_play_index);
							flag = 1;  //wwj add
							break;
						}
						if (wav_file_table[k] & (1<<l)) {
							g_play_index = (k << 4) + l;
							info_ptr->file_type = TK_IMAGE_TYPE_WAV;
							sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_play_index);
							flag = 1;  //wwj add
							break;
						}						
					}
					//l = 0xF; //wwj mark
					//if (index_tmp != g_play_index - 1) {
					if(flag) {
						break;
					}
				}
				l = 0xF; //wwj add
				k--;
				if (k < 0) {
					k = 624;
				}
			}
			if (i > 626) {
				info_ptr->err_flag = STOR_SERV_NO_MEDIA;
				g_err_cnt = 0;
				msgQSend(ApQ, MSG_STORAGE_SERVICE_BROWSE_REPLY, info_ptr, sizeof(STOR_SERV_PLAYINFO), MSG_PRI_NORMAL);
				return;
			}
		} else if (type == STOR_SERV_SEARCH_NEXT) {
			INT8U flag = 0; //wwj add

			index_tmp = g_play_index + 1;
			if (index_tmp > 9999) {
				index_tmp = 0;
			}
			k = index_tmp >> 4;
			l = index_tmp & 0xF;
			while (i <= 626) {
				i++;
				if (avi_file_table[k] || jpg_file_table[k] || wav_file_table[k]) {
					for ( ; l<0x10 ; l++) {
						if (avi_file_table[k] & (1<<l)) {
							g_play_index = (k << 4) + l;
							info_ptr->file_type = TK_IMAGE_TYPE_MOTION_JPEG;
							sprintf((char *)g_file_path, (const char *)"MOVI%04d.avi", g_play_index);
							flag = 1; //wwj add
							break;
						}
						if (jpg_file_table[k] & (1<<l)) {
							g_play_index = (k << 4) + l;
							info_ptr->file_type = TK_IMAGE_TYPE_JPEG;
							sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", g_play_index);
							flag = 1; //wwj add
							break;
						}
						if (wav_file_table[k] & (1<<l)) {
							g_play_index = (k << 4) + l;
							info_ptr->file_type = TK_IMAGE_TYPE_WAV;
							sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_play_index);
							flag = 1; //wwj add
							break;
						}						
					}
					//l = 0x0; //wwj mark
					//if (index_tmp != g_play_index + 1) {
					if(flag) {
						break;
					}
				}
				l = 0x0; //wwj add
				k++;
				if (k > 624) {
					k = 0;
				}
			}
			if (i > 626) {
				info_ptr->err_flag = STOR_SERV_NO_MEDIA;
				g_err_cnt = 0;
				msgQSend(ApQ, MSG_STORAGE_SERVICE_BROWSE_REPLY, info_ptr, sizeof(STOR_SERV_PLAYINFO), MSG_PRI_NORMAL);
				return;
			}
		} else {
			if (type == STOR_SERV_SEARCH_GIVEN) {
				g_play_index = given_play_index;
			}
			k = g_play_index >> 4;
			l = g_play_index & 0xF;
			if (avi_file_table[k] & (1<<l)) {
				info_ptr->file_type = TK_IMAGE_TYPE_MOTION_JPEG;
				sprintf((char *)g_file_path, (const char *)"MOVI%04d.avi", g_play_index);
			} else if (jpg_file_table[k] & (1<<l)) {
				info_ptr->file_type = TK_IMAGE_TYPE_JPEG;
				sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", g_play_index);
			} else if (wav_file_table[k] & (1<<l)) {
				info_ptr->file_type = TK_IMAGE_TYPE_WAV;
				sprintf((char *)g_file_path, (const char *)"RECR%04d.wav", g_play_index);
			}
		}
	}
DBG_PRINT("File Path=%s\r\n", g_file_path);	
	info_ptr->file_path_addr = (INT32U) g_file_path;
	info_ptr->file_handle = open(g_file_path, O_RDONLY);
	if (info_ptr->file_handle >= 0) {
		stat(g_file_path, &buf_tmp);
		info_ptr->file_size = buf_tmp.st_size;
		if (!err_flag) {
			info_ptr->err_flag = STOR_SERV_OPEN_OK;
			g_err_cnt = 0;
		} else {
			g_err_cnt++;
			if (g_err_cnt <= g_file_num) {
				info_ptr->err_flag = STOR_SERV_OPEN_OK;
			} else {
				info_ptr->err_flag = STOR_SERV_DECODE_ALL_FAIL;
			}
		}
	} else {
	  #if RENAME_LOCK_FILE
		if(info_ptr->file_type == TK_IMAGE_TYPE_MOTION_JPEG) {
			#if !LOCK_FILE_NAME
			g_file_path[0] = 'L'; g_file_path[1] = 'O'; g_file_path[2] = 'C'; g_file_path[3] = 'K';
			#else
			g_file_path[0] = 'S'; g_file_path[1] = 'O'; g_file_path[2] = 'S'; g_file_path[3] = '0';
			#endif
			info_ptr->file_handle = open(g_file_path, O_RDONLY);
			if(info_ptr->file_handle < 0) {
				info_ptr->err_flag = STOR_SERV_OPEN_FAIL;
				g_err_cnt = 0;
			} else {
				stat(g_file_path, &buf_tmp);
				info_ptr->file_size = buf_tmp.st_size;
				if (!err_flag) {
					info_ptr->err_flag = STOR_SERV_OPEN_OK;
					g_err_cnt = 0;
				} else {
					g_err_cnt++;
					if (g_err_cnt <= g_file_num) {
						info_ptr->err_flag = STOR_SERV_OPEN_OK;
					} else {
						info_ptr->err_flag = STOR_SERV_DECODE_ALL_FAIL;
					}
				}
			}
		} else
	  #endif
		{
			info_ptr->err_flag = STOR_SERV_OPEN_FAIL;
			g_err_cnt = 0;
		}
	}
	info_ptr->deleted_file_number = get_deleted_file_number();
	info_ptr->play_index = g_play_index;
	info_ptr->total_file_number = g_file_num - g_same_index_num;
	msgQSend(ApQ, MSG_STORAGE_SERVICE_BROWSE_REPLY, info_ptr, sizeof(STOR_SERV_PLAYINFO), MSG_PRI_NORMAL);
}

INT16S ap_storage_service_playfilelist_req(INT8U *buff_addr, INT16U play_file_index)
{
	INT16S index_tmp, i = 0, k, l;
	struct stat_t buf_tmp;
	INT8U *p_buf_addr;
	INT8U *p_file_info;
	INT8U  play_file_index_cnt, play_file_err_cnt,flag;
	STOR_SERV_PLAYINFO info_ptr;
	TIME_T time_info;
	INT16U palyload_size;
	INT8U  avi_file_type;
	
	p_buf_addr = buff_addr;
	p_file_info = buff_addr+3;
	play_file_index_cnt = 0;
	play_file_err_cnt = 0;	
	
	ap_storage_service_timer_stop();
	
	for(play_file_index_cnt = 0;play_file_index_cnt<16;play_file_index_cnt++)
	{
		flag = 0;
		index_tmp = play_file_index+1;
		if (index_tmp > 9999) {
				index_tmp = 0;
			}
		k = index_tmp >> 4;
		l = index_tmp & 0xF;
		while (i <= 626) {
			i++;
			if (avi_file_table[k] || jpg_file_table[k]) {
				for ( ; l<0x10 ; l++) {
					if (avi_file_table[k] & (1<<l)) {
						play_file_index = (k << 4) + l;
						info_ptr.file_type = TK_IMAGE_TYPE_MOTION_JPEG;
						sprintf((char *)g_file_path, (const char *)"MOVI%04d.avi", play_file_index);
						flag = 1; //wwj add
						break;
					}
					if (jpg_file_table[k] & (1<<l)) {
						play_file_index = (k << 4) + l;
						info_ptr.file_type = TK_IMAGE_TYPE_JPEG;
						sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", play_file_index);
						flag = 1; //wwj add
						break;
					}					
				}
				if(flag) {
					break;
				}
			}
			l = 0x0; //wwj add
			k++;
			if (k > 624) {
				break;
			}
		}
		if(flag)
		{
			info_ptr.file_path_addr = (INT32U) g_file_path;
			info_ptr.file_handle = open(g_file_path, O_RDONLY);
			if(info_ptr.file_type == TK_IMAGE_TYPE_MOTION_JPEG) 
			{ 
				if(info_ptr.file_handle<0) 
				{ 
					#if !LOCK_FILE_NAME
					sprintf((char *)g_file_path, (const char *)"LOCK%04d.avi", play_file_index); 
					#else
					sprintf((char *)g_file_path, (const char *)"SOS0%04d.avi", play_file_index); 
					#endif
					info_ptr.file_path_addr = (INT32U) g_file_path; 
					info_ptr.file_handle = open(g_file_path, O_RDONLY); 
					if(info_ptr.file_handle >= 0)
					{
						#if !LOCK_FILE_NAME
						avi_file_type = 'L';//'L'
						#else
						avi_file_type = 'S';//'S'
						#endif
					}
				} 
				else
				{
					avi_file_type = 'A';
				}
			} 
			if (info_ptr.file_handle >= 0) {
				fstat(info_ptr.file_handle, &buf_tmp);
				close(info_ptr.file_handle);
				info_ptr.file_size = buf_tmp.st_size/1024;
				ap_browse_decode_file_date_time_1(buf_tmp.st_mtime,&time_info);
				if(info_ptr.file_type == TK_IMAGE_TYPE_MOTION_JPEG)
				{
					*(p_file_info++) = avi_file_type;
				}
				else if(info_ptr.file_type == TK_IMAGE_TYPE_JPEG)
				{
					*(p_file_info++) = 'J';
				}
				*(p_file_info++) = play_file_index &0xFF;
				*(p_file_info++) = (play_file_index >>8)&0xFF;
				*(p_file_info++) = time_info.tm_year - 2000;
				*(p_file_info++) = time_info.tm_mon;
				*(p_file_info++) = time_info.tm_mday;
				*(p_file_info++) = time_info.tm_hour;
				*(p_file_info++) = time_info.tm_min;
				*(p_file_info++) = time_info.tm_sec;
				*(p_file_info++) = info_ptr.file_size & 0xFF;
				*(p_file_info++) = (info_ptr.file_size>>8) & 0xFF;
				*(p_file_info++) = (info_ptr.file_size>>16) & 0xFF;
				*(p_file_info++) = (info_ptr.file_size>>24) & 0xFF;
			}
			else {
				play_file_err_cnt++;
			}
		}
		else
		{
			break;
		}
	}

	if (play_file_err_cnt == 0)	// NO error
	{
		palyload_size = 1+13*play_file_index_cnt;
		*(p_buf_addr++) = palyload_size &0xFF;
		*(p_buf_addr++) = (palyload_size>>8) &0xFF;
		*p_buf_addr     = play_file_index_cnt;
	}
	else
	{	// maybe remove SD card
		palyload_size = 0xFFFF;
	}

	ap_storage_service_timer_start();
	
	return palyload_size;
}

INT16S ap_storage_service_indexfile_req(INT16U playfile_order, INT16U *playfile_index_table)
{
	//INT32S i;
	INT32S order_level; 
       INT16S k, l;
       INT16U play_file_index;
       INT16U playfile_count = 0;	   

	if (playfile_order==0)
	{
		play_file_index = 0xFFFF;	// error
		goto AP_STORAGE_SERVICE_INDEXFILE_REQ_END;
	}

	if (playfile_order>9999)
	{
		playfile_order = 0;
	}
	order_level = (playfile_order>>9);   // Each 512 files creates a record.
	playfile_order = playfile_order - (order_level<<9);		// Each 512 files creates a record.
	play_file_index = playfile_index_table[order_level];
	if ( (playfile_order==0)&&(order_level!=0) )
	{
		goto AP_STORAGE_SERVICE_INDEXFILE_REQ_END;
	}
	play_file_index++;
	if (play_file_index>9999)
	{
		play_file_index = 0;
	}

       k = play_file_index >> 4; 
       l = play_file_index & 0xF; 
       while (1) { 
           if (avi_file_table[k] || jpg_file_table[k]) { 
              for ( ; l<0x10 ; l++) { 
                    if (avi_file_table[k] & (1<<l)) { 
                        playfile_count++; 
			   if (playfile_count >= playfile_order)
			   {
	                        play_file_index = (k << 4) + l; 
			   	   goto AP_STORAGE_SERVICE_INDEXFILE_REQ_END;
			   }
                     } 
                     if (jpg_file_table[k] & (1<<l)) { 
                          playfile_count++;
			     if (playfile_count >= playfile_order)
			     {
     		                play_file_index = (k << 4) + l;
			   	  goto AP_STORAGE_SERVICE_INDEXFILE_REQ_END;							
			     }
                     }                                         
              } 
           }

	    l = 0x0; //wwj add 
           k++; 
           if (k > 624) { 
		  play_file_index = 0xFFFF;	// error		   	
                break; 
           } 			   
	
       } 

AP_STORAGE_SERVICE_INDEXFILE_REQ_END:
	return (INT16S)play_file_index;
	
}


INT16U ap_storage_service_playfile_totalcount(INT16U *playfile_index_table) 
{ 
        INT16U playfile_total_conut,play_file_index; 
        INT16S index_tmp, k, l; 
        INT8U flag; 
        
        playfile_total_conut = 0; 
        play_file_index = 0xffff; 
        
        while(1) 
        { 
                flag = 0; 
                index_tmp = play_file_index+1; 
                if (index_tmp > 9999) { 
                                index_tmp = 0; 
                        } 
                k = index_tmp >> 4; 
                l = index_tmp & 0xF; 
                while (1) { 
                        if (avi_file_table[k] || jpg_file_table[k]) { 
                                for ( ; l<0x10 ; l++) { 
                                        if (avi_file_table[k] & (1<<l)) { 
                                                play_file_index = (k << 4) + l; 
                                                playfile_total_conut++; 
                                                flag=1; 
                                                break; 
                                        } 
                                        if (jpg_file_table[k] & (1<<l)) { 
                                                play_file_index = (k << 4) + l; 
                                                playfile_total_conut++; 
                                                flag=1; 
                                                break; 
                                        }                                         
                                } 

			   	    if ( (playfile_total_conut>0)&&(playfile_total_conut&0x1FF)==0x0)	// Each 512 files creates a record.
			   	    {
			   		playfile_index_table[playfile_total_conut>>9] = play_file_index;        // Each 512 files creates a record.
			   	    }

                                if(flag) { 
			                                	if(play_file_index==9999)
																				{
																					k++;
																				}
                                        break; 
                                } 
                        }
                        l = 0x0; //wwj add 
                        k++; 
                        if (k > 624) { 
                                break; 
                        } 
                } 
                if (k > 624) { 
                        break; 
                } 
        } 

        return playfile_total_conut; 
} 


#define MAX_THUMBNAIL_SIZE  (30*1024)
void * ap_storage_service_thumbnail_req(INT16U file_index,INT32U *buf_size)
{
	INT16U t_playfile_index;
	INT16S k, l;
	struct stat_t buf_tmp;
	STOR_SERV_PLAYINFO info_ptr;
	INT32S ret;
	INT32U display_addr = 0,jpg_addr;

	ap_storage_service_timer_stop();

	t_playfile_index = file_index;
	k = t_playfile_index >> 4;
	l = t_playfile_index & 0xF;
	if (avi_file_table[k] & (1<<l)) {
		info_ptr.file_type = TK_IMAGE_TYPE_MOTION_JPEG;
		sprintf((char *)g_file_path, (const char *)"MOVI%04d.avi", t_playfile_index);
	} else if (jpg_file_table[k] & (1<<l)) {
		info_ptr.file_type = TK_IMAGE_TYPE_JPEG;
		sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", t_playfile_index);
	} 
	
	info_ptr.file_path_addr = (INT32U) g_file_path;
	info_ptr.file_handle = open(g_file_path, O_RDONLY);
	if(info_ptr.file_type == TK_IMAGE_TYPE_MOTION_JPEG) 
	{ 
		if(info_ptr.file_handle<0) 
		{ 
			#if !LOCK_FILE_NAME
			sprintf((char *)g_file_path, (const char *)"LOCK%04d.avi", t_playfile_index);
			#else
			sprintf((char *)g_file_path, (const char *)"SOS0%04d.avi", t_playfile_index);
			#endif 
			info_ptr.file_path_addr = (INT32U) g_file_path; 
			info_ptr.file_handle = open(g_file_path, O_RDONLY); 
		} 
	} 
	if(info_ptr.file_handle >= 0) 
	{
		fstat(info_ptr.file_handle, &buf_tmp);
		info_ptr.file_size = buf_tmp.st_size;
		jpg_addr = (INT32U) gp_malloc_align(MAX_THUMBNAIL_SIZE, 64);
		display_addr = (INT32U) gp_malloc_align(320*240*2, 64);

		ret = ap_state_handling_jpeg_decode_thumbnail(&info_ptr, display_addr); // get 1st jpeg of AVI
		close(info_ptr.file_handle);
		
		ap_storage_service_timer_start();
		if(ret == STATUS_OK) 
		{
			ret = Encode_Disp_Buf_To_Jpeg(display_addr,jpg_addr+64,320,240,MAX_THUMBNAIL_SIZE -64,buf_size);
			if (display_addr)
    			gp_free((void *) display_addr);
			if(ret == 0)
			{
				return ((void *) jpg_addr);
			}
			else
			{
    			if(jpg_addr)
    				gp_free((void *) jpg_addr);
				return (NULL);
			}
		} 
		else 
		{
			if (display_addr)
    			gp_free((void *) display_addr);
    		if(jpg_addr)
    				gp_free((void *) jpg_addr);
			return (NULL);
		}
	}
	else
	{
		//fail
		ap_storage_service_timer_start();
		return (NULL);
	}
}

static INT32U g_sd_read_size = (10*1024);
INT32S ap_storage_service_download_file_block_size(INT32U size)
{
	g_sd_read_size = size;
	return 0;
}

INT32S ap_storage_service_download_file_req(INT16U file_index,STOR_SERV_DOWNLOAD_FILEINFO *dw_file_info,INT8U *buf_addr,INT32U *buf_size)
{
	INT16U t_playfile_index;
	INT16S k, l;
	struct stat_t buf_tmp;
	STOR_SERV_PLAYINFO info_ptr;

	t_playfile_index = file_index;
	
	ap_storage_service_timer_stop();
	
	if((t_playfile_index==dw_file_info->play_index)&&(dw_file_info->file_size != 0))
	{
		dw_file_info->file_handle = open((CHAR *)dw_file_info->file_path_addr, O_RDONLY);
		if(dw_file_info->file_handle >= 0) 
		{
			lseek(dw_file_info->file_handle, dw_file_info->file_offset, SEEK_SET);
			*buf_size = read(dw_file_info->file_handle, (INT32U) buf_addr, g_sd_read_size);
			dw_file_info->file_offset += *buf_size;
			//DBG_PRINT("offset = %d,readsize = %d\r\n",dw_file_info->file_offset,*buf_size);
			close(dw_file_info->file_handle);
			ap_storage_service_timer_start();
			return 0;
		}
		else
		{
			ap_storage_service_timer_start();
			return -1;
		}
	}
	else
	{	
		k = t_playfile_index >> 4;
		l = t_playfile_index & 0xF;
		if (avi_file_table[k] & (1<<l)) {
			info_ptr.file_type = TK_IMAGE_TYPE_MOTION_JPEG;
			sprintf((char *)g_file_path, (const char *)"MOVI%04d.avi", t_playfile_index);
		} else if (jpg_file_table[k] & (1<<l)) {
			info_ptr.file_type = TK_IMAGE_TYPE_JPEG;
			sprintf((char *)g_file_path, (const char *)"PICT%04d.jpg", t_playfile_index);
		} 
		
		info_ptr.file_path_addr = (INT32U) g_file_path;
		info_ptr.file_handle = open(g_file_path, O_RDONLY);
		if(info_ptr.file_type == TK_IMAGE_TYPE_MOTION_JPEG)
		{
			if(info_ptr.file_handle<0)
			{
				#if !LOCK_FILE_NAME
				sprintf((char *)g_file_path, (const char *)"LOCK%04d.avi", t_playfile_index);
				#else
				sprintf((char *)g_file_path, (const char *)"SOS0%04d.avi", t_playfile_index);
				#endif
				info_ptr.file_path_addr = (INT32U) g_file_path;
				info_ptr.file_handle = open(g_file_path, O_RDONLY);
			}
		}
		dw_file_info->play_index = t_playfile_index;
		dw_file_info->file_path_addr = (INT32U) g_file_path;
		dw_file_info->file_handle = info_ptr.file_handle;
		dw_file_info->file_offset = 0;
		if(info_ptr.file_handle >= 0) 
		{
			fstat(info_ptr.file_handle, &buf_tmp);
			info_ptr.file_size = buf_tmp.st_size;
			dw_file_info->file_size = buf_tmp.st_size;
			//DBG_PRINT("totalsize = %d\r\n",dw_file_info->file_size);
			*buf_size = read(info_ptr.file_handle, (INT32U) buf_addr, g_sd_read_size);
			dw_file_info->file_offset += *buf_size;
			//DBG_PRINT("offset = %d,readsize = %d\r\n",dw_file_info->file_offset,*buf_size);
			close(info_ptr.file_handle);
			ap_storage_service_timer_start();
			return 0;
		}
		else
		{
			ap_storage_service_timer_start();
			return -1;
		}
	}
}

INT32S get_file_final_avi_index(INT8U count_total_num_enable)
{
	CHAR  *pdata;
	INT32S nRet, temp;
	INT32U temp_time;
	struct f_info file_info;	
	//struct stat_t buf_tmp;
	
	g_avi_index = -1;
	g_avi_index_9999_exist = g_avi_file_time = g_avi_file_oldest_time = 0;
	nRet = _findfirst("*.avi", &file_info, D_ALL);
	if (nRet < 0) {
		g_avi_index++;
		return g_avi_index;
	}
	while (1) {	
		pdata = (CHAR *) file_info.f_name;
		// Remove 0KB AVI files
		if (file_info.f_size<512 && unlink((CHAR *) file_info.f_name)==0) {
			nRet = _findnext(&file_info);
			if (nRet < 0) {
				break;
			}
			continue;
		}

        //stat((CHAR *) file_info.f_name, &buf_tmp);	//deleted by wwj, it spent too long time
	  #if !RENAME_LOCK_FILE
		if (gp_strncmp((INT8S *) pdata, (INT8S *) "MOVI", 4) == 0)
	  #else
		if ((gp_strncmp((INT8S *) pdata, (INT8S *) "MOVI", 4) == 0)
		#if !LOCK_FILE_NAME
			|| (gp_strncmp((INT8S *) pdata, (INT8S *) "LOCK", 4) == 0))
		#else
			|| (gp_strncmp((INT8S *) pdata, (INT8S *) "SOS0", 4) == 0))
		#endif
	  #endif
		{
			temp = (*(pdata + 4) - 0x30)*1000;
			temp += (*(pdata + 5) - 0x30)*100;
			temp += (*(pdata + 6) - 0x30)*10;
			temp += (*(pdata + 7) - 0x30);
			if (temp < 10000) {
				avi_file_table[temp >> 4] |= 1 << (temp & 0xF);
				if(count_total_num_enable){
					g_file_num++;
				}
				if (temp > g_avi_index) {
					g_avi_index = temp;
				}
				temp_time = (file_info.f_date<<16)|file_info.f_time;
				if((!g_avi_file_time) || (temp_time > g_avi_file_time)) {
					g_avi_file_time = temp_time;
					g_latest_avi_file_index = temp;
				}
				if( ((!g_avi_file_oldest_time) || (temp_time < g_avi_file_oldest_time)) && /*((buf_tmp.st_mode & D_RDONLY) == 0)*/((file_info.f_attrib & _A_RDONLY) == 0) ){
					g_avi_file_oldest_time = temp_time;
					g_oldest_avi_file_index = temp;
				}
			}
		}
		nRet = _findnext(&file_info);
		if (nRet < 0) {
			break;	
		}
	}
	g_avi_index++;
	if(g_avi_index > 9999) {
		g_avi_index_9999_exist = 1;
		g_avi_index = g_latest_avi_file_index+1;
		if(g_avi_index > 9999) {
			g_avi_index = 0;
		}
	}
	
	return g_avi_index;
}

INT32S get_file_final_jpeg_index(INT8U count_total_num_enable)
{
	CHAR  *pdata;
	struct f_info   file_info;
	INT32S nRet, temp;
	INT32U temp_time;
	
	g_jpeg_index = -1;
	g_jpg_file_time = 0;

	#if ENABLE_SAVE_SENSOR_RAW_DATA		
	nRet = _findfirst("*.dat", &file_info, D_ALL);
	#else
	nRet = _findfirst("*.jpg", &file_info, D_ALL);
	#endif
	if (nRet < 0) {
		g_jpeg_index++;
		return g_jpeg_index;
		
	}	
	while (1) {	
		pdata = (CHAR*)file_info.f_name;
		// Remove 0KB JPG files
		if (file_info.f_size<256 && unlink((CHAR *) file_info.f_name)==0) {
			nRet = _findnext(&file_info);
			if (nRet < 0) {
				break;
			}
			continue;
		}

		if (gp_strncmp((INT8S *) pdata, (INT8S *) "PICT", 4) == 0) {
			temp = (*(pdata + 4) - 0x30)*1000;
			temp += (*(pdata + 5) - 0x30)*100;
			temp += (*(pdata + 6) - 0x30)*10;
			temp += (*(pdata + 7) - 0x30);
			if (temp < 10000) {
				jpg_file_table[temp >> 4] |= 1 << (temp & 0xF);
				if(count_total_num_enable){
					g_file_num++;
				}
				if (temp > g_jpeg_index) {
					g_jpeg_index = temp;
				}
				
				temp_time = (file_info.f_date<<16)|file_info.f_time;
				if( (!g_jpg_file_time) || (temp_time > g_jpg_file_time) ){
					g_jpg_file_time = temp_time;
					g_latest_jpg_file_index = temp;
				}			
			}
		}		
		nRet = _findnext(&file_info);
		if (nRet < 0) {
			break;
		}
	}
	g_jpeg_index++;
	if( (g_jpeg_index > 9999) || (g_avi_index_9999_exist == 1) ){
		g_avi_index_9999_exist = 1;

		g_jpeg_index = g_latest_jpg_file_index+1;
		if(g_jpeg_index > 9999) {
			g_jpeg_index = 0;
		}

		g_avi_index = g_latest_avi_file_index+1;
		if(g_avi_index > 9999) {
			g_avi_index = 0;
		}
	}
	
	return g_jpeg_index;
}

INT32S get_file_final_wav_index(INT8U count_total_num_enable)
{
	CHAR  *pdata;
	struct f_info   file_info;
    INT16S latest_file_index;
	INT32S nRet, temp;
    INT32U temp_time;

	g_wav_index = -1;
	g_wav_file_time = 0;

	nRet = _findfirst("*.wav", &file_info, D_ALL);
	if (nRet < 0) {
		g_wav_index++;
		return g_wav_index;
	}
	while (1) {
		pdata = (CHAR*)file_info.f_name;
		// Remove 0KB WAV files
		if (file_info.f_size==0 && unlink((CHAR *) file_info.f_name)==0) {
			nRet = _findnext(&file_info);
			if (nRet < 0) {
				break;
			}
			continue;
		}

		if (gp_strncmp((INT8S *) pdata, (INT8S *) "RECR", 4) == 0) {
			temp = (*(pdata + 4) - 0x30)*1000;
			temp += (*(pdata + 5) - 0x30)*100;
			temp += (*(pdata + 6) - 0x30)*10;
			temp += (*(pdata + 7) - 0x30);

			if (temp < 10000)
            {
				wav_file_table[temp >> 4] |= 1 << (temp & 0xF);
				if(count_total_num_enable){
					g_file_num++;
				}

    			if (temp > g_wav_index) {
    				g_wav_index = temp;
    			}

                temp_time = (file_info.f_date<<16)|file_info.f_time;

                if( (!g_wav_file_time) || (temp_time > g_wav_file_time) ){
					g_wav_file_time = temp_time;
					latest_file_index = temp;
				}
			}
		}

		nRet = _findnext(&file_info);
		if (nRet < 0) {
			break;
		}
	}
	g_wav_index++;
	if( (g_wav_index > 9999) || (g_avi_index_9999_exist == 1) ){
		g_avi_index_9999_exist = 1;

		g_wav_index = latest_file_index+1;
		if(g_wav_index > 9999) {
			g_wav_index = 0;
		}
		
		g_avi_index = g_latest_avi_file_index+1;
		if(g_avi_index > 9999) {
			g_avi_index = 0;
		}

		g_jpeg_index = g_latest_jpg_file_index+1;
		if(g_jpeg_index > 9999) {
			g_jpeg_index = 0;
		}
	}

	return g_wav_index;	
}

INT16U get_deleted_file_number(void)
{
	INT16U i;
	INT16U deleted_file_number = 0;
	
	for(i=0; i<g_play_index+1; i++)
	{
        if( ((avi_file_table[(g_play_index-i)>>4] & (1<<((g_play_index-i)&0xF))) == 0) && 
          ((jpg_file_table[(g_play_index-i)>>4] & (1<<((g_play_index-i)&0xF))) == 0) &&
          ((wav_file_table[(g_play_index-i)>>4] & (1<<((g_play_index-i)&0xF))) == 0))
		{
			deleted_file_number++;
		}
	}
	return deleted_file_number;
}


INT16U get_same_index_file_number(void)
{
	INT16U i, j;
	INT16U same_index_file_number = 0;
	
	for(i=0; i<AP_STG_MAX_FILE_NUMS; i++)
	{
		for(j=0; j<16; j++)
		{
			if( (avi_file_table[i] & (1<<j)) && (jpg_file_table[i] & (1<<j)) && (wav_file_table[i] & (1<<j)) ){
				same_index_file_number++;
			}
		}
	}
	return same_index_file_number;
}

void ap_storage_service_format_req(BOOLEAN is_reply)
{
	INT32U i;
    INT32U max_temp;

	if(drvl2_sd_sector_number_get()<=2097152) {  // Dominant  1GB card should use FAT16
		_format(MINI_DVR_STORAGE_TYPE, FAT16_Type);
	} else {
		_format(MINI_DVR_STORAGE_TYPE, FAT32_Type);
	}

	mkdir("C:\\DCIM");
	chdir("C:\\DCIM");
	g_jpeg_index = 0;
    g_avi_index = 0;
    g_file_index = 0;
    g_file_num = 0;
    g_wav_index = 0;
	g_play_index = -1;
	for (i=0 ; i<AP_STG_MAX_FILE_NUMS ; i++) {
		avi_file_table[i] = 0;
		jpg_file_table[i] = 0;
        wav_file_table[i] = 0;
	}
	get_file_final_avi_index(1);
	get_file_final_jpeg_index(1);
	get_file_final_wav_index(1);

	if(g_avi_index_9999_exist) {
		if(g_avi_file_time > g_jpg_file_time) {
            max_temp = g_avi_index;
			if (g_avi_file_time > g_wav_file_time) {
            	g_file_index = max_temp;
			} else {
            	g_file_index = g_wav_index;
        	}
		} else {
            max_temp = g_jpeg_index;
			if (g_jpg_file_time > g_wav_file_time) {
            	g_file_index = max_temp;
			} else {
            	g_file_index = g_wav_index;
        	}                        
		}
	} else {
	    if (g_avi_index > g_jpeg_index) {
	        max_temp = g_avi_index;
		} else {
	        max_temp = g_jpeg_index;
	    }

	    if (max_temp > g_wav_index) {
	        g_file_index = max_temp;
		}else{
			g_file_index = g_wav_index;
		}
	}
	FNodeInfo[SD_SLOT_ID].audio.MaxFileNum = 0;
	if (is_reply)
		msgQSend(ApQ, MSG_STORAGE_SERVICE_FORMAT_REPLY, NULL, NULL, MSG_PRI_NORMAL);
}

void FileSrvRead(P_TK_FILE_SERVICE_STRUCT para)
{
	INT32S read_cnt;
    
    if (para->rev_seek) {
    	if(para->FB_seek)
    	{
	        lseek(para->fd, -para->rev_seek, SEEK_CUR);
	        DBG_PRINT("reverse seek- %d\r\n",para->rev_seek);
        }
        else
        {
 	        lseek(para->fd, para->rev_seek, SEEK_CUR);
	        DBG_PRINT("reverse seek+ %d\r\n",para->rev_seek);
       }
    }
	read_cnt = read(para->fd, para->buf_addr, para->buf_size);
	if(para->result_queue)
	{
		OSQPost(para->result_queue, (void *) read_cnt);
	}
}


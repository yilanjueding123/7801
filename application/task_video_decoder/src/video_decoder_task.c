#include "ap_display.h"
#include "application.h"
#include "task_video_decoder.h"
#include "my_video_codec_callback.h"
#include "state_wifi.h"


#define C_VIDEO_DECODE_STACK_SIZE		512	
#define C_VIDEO_Q_ACCEPT_MAX			5

#if DUAL_STREAM_FUNC_ENABLE
extern void Disp_MJpeg_To_Wifi(INT32U dispAddr);
#endif


/*os task stack */
INT32U	video_decode_stack[C_VIDEO_DECODE_STACK_SIZE];

/* os task queue */
OS_EVENT *vid_dec_q;
OS_EVENT *vid_dec_ack_m;
void *vid_dec_q_buffer[C_VIDEO_Q_ACCEPT_MAX];

INT32S video_decode_task_start(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;
	SEND_MESSAGE(vid_dec_q, MSG_VID_DEC_START, vid_dec_ack_m, 0, msg, err);
Return:		
	return nRet;
}

INT32S video_decode_task_restart(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;
	SEND_MESSAGE(vid_dec_q, MSG_VID_DEC_RESTART, vid_dec_ack_m, 0, msg, err);
Return:		
	return nRet;
}

INT32S video_decode_task_stop(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;
	SEND_MESSAGE(vid_dec_q, MSG_VID_DEC_STOP, vid_dec_ack_m, 0, msg, err);
Return:	
	return nRet;
}

INT32S video_decode_task_nth_frame(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;
	SEND_MESSAGE(vid_dec_q, MSG_VID_DEC_NTH, vid_dec_ack_m, 0, msg, err);
Return:	
	return nRet;
}

INT32S  video_decode_task_create(INT8U proi)
{
	INT8U  err;
	INT32S nRet;
	
	vid_dec_q = OSQCreate(vid_dec_q_buffer, C_VIDEO_Q_ACCEPT_MAX);
	if(!vid_dec_q) RETURN(STATUS_FAIL);
	
	vid_dec_ack_m = OSMboxCreate(NULL);
	if(!vid_dec_ack_m) RETURN(STATUS_FAIL);
	
	err = OSTaskCreate(video_decode_task_entry, (void *)NULL, &video_decode_stack[C_VIDEO_DECODE_STACK_SIZE - 1], proi);	
	if(err != OS_NO_ERR) RETURN(STATUS_FAIL);

	nRet = STATUS_OK;	
Return:	
	return nRet;		
}

INT32S video_decode_task_del(INT8U proi)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;
	SEND_MESSAGE(vid_dec_q, MSG_VID_DEC_EXIT, vid_dec_ack_m, 0, msg, err);
Return: 	
	OSQFlush(vid_dec_q);
	OSQDel(vid_dec_q, OS_DEL_ALWAYS, &err);
	OSMboxDel(vid_dec_ack_m, OS_DEL_ALWAYS, &err);	
	return STATUS_OK;
}

static void video_decode_task_end(void)
{
}

INT32U deocde_cnt = 0;
void video_decode_task_entry(void *parm)
{
	INT8U  success_flag, init_flag, nth_flag;
	INT8U  err;
	INT32S status, error_id;
	INT32U raw_data_addr, display_addr;
	INT32U scaler_out_addr;
	INT32U msg_id;
	INT64S delta_Tv, temp;
	long Duration, size;
	OS_CPU_SR cpu_sr;
	SCALER_MAS scaler_mas;
	INT8U curr_disp_dev;

	gp_memset((INT8S *) &scaler_mas, 0, sizeof(SCALER_MAS));
	scaler_mas.mas_0 = MAS_EN_READ|MAS_EN_WRITE;
	scaler_mas_set(SCALER_1, &scaler_mas);	
	nth_flag = 0;

	while(1)
	{
		msg_id = (INT32U) OSQPend(vid_dec_q, 0, &err);
		if((err != OS_NO_ERR)||	!msg_id)
		{
			continue;
		}
			
		switch(msg_id)
		{
		case MSG_VID_DEC_ONE_FRAME:
			success_flag = display_addr = 0;
			if(!raw_data_addr || size <= 0) // read video data speed too slow
			{
				if(MultiMediaParser_IsEOV(p_vid_dec_para->media_handle))
				{
					DEBUG_MSG(DBG_PRINT("VidDecEnd.\r\n"));
					video_decode_task_end();
					vid_dec_clear_status(C_VIDEO_DECODE_PLAYING);
					vid_dec_end_callback();
					p_vid_dec_para->pend_cnt = p_vid_dec_para->post_cnt;
					OSQFlush(vid_dec_q);
					continue;
				}
				goto EndOf_MSG_VID_DEC_ONE_FRAME;
			}

			OS_ENTER_CRITICAL();
			temp = (INT64S)(p_vid_dec_para->tv - p_vid_dec_para->Tv);
			OS_EXIT_CRITICAL();
			if(temp > p_vid_dec_para->time_range) // check sync
			{
				if(p_vid_dec_para->fail_cnt < 2)
				{
					p_vid_dec_para->fail_cnt++;
					MultiMediaParser_SetFrameDropLevel(p_vid_dec_para->media_handle, p_vid_dec_para->fail_cnt);
					DEBUG_MSG(DBG_PRINT("FailCnt = %d\r\n", p_vid_dec_para->fail_cnt));
				}							
			}
			else
			{
				if(p_vid_dec_para->fail_cnt > 0)
				{
					p_vid_dec_para->fail_cnt--;
					MultiMediaParser_SetFrameDropLevel(p_vid_dec_para->media_handle, p_vid_dec_para->fail_cnt);	
					DEBUG_MSG(DBG_PRINT("FailCnt = %d\r\n", p_vid_dec_para->fail_cnt));
				}
			}

			curr_disp_dev = ap_display_get_device();
			display_addr = ap_display_queue_get(display_isr_queue);
			if (!display_addr) {
				goto EndOf_MSG_VID_DEC_ONE_FRAME_doNothing;
			}

			if(curr_disp_dev==DISP_DEV_HDMI) { //HDMI
#ifdef HDMI_JPG_DECODE_AS_GP420			
				status = video_decode_jpeg_as_gp420((INT8U *) raw_data_addr, size, 1280, 720, display_addr);
#elif defined(HDMI_JPG_DECODE_AS_YUV422)
				status = video_decode_jpeg_as_gp422((INT8U *) raw_data_addr, size, 1280, 720, display_addr);
#endif				
			} else if(curr_disp_dev==DISP_DEV_TFT) { //TFT
				status = video_decode_jpeg_as_rgb565((INT8U *) raw_data_addr, size, TFT_WIDTH, TFT_HEIGHT, display_addr);
			} else { //TV
				status = video_decode_jpeg_as_rgb565((INT8U *) raw_data_addr, size, TV_WIDTH, TV_HEIGHT, display_addr);
			}

			if(status < 0) {
				DEBUG_MSG(DBG_PRINT("JpegDecFail = %d\r\n", status));
				success_flag = 1;
				
				ap_display_queue_put(display_isr_queue, display_addr);
				display_addr = 0;
				
				goto EndOf_MSG_VID_DEC_ONE_FRAME;
			}

			success_flag = 1;
			if(display_addr && !nth_flag)
			{
				#if DUAL_STREAM_FUNC_ENABLE
					if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
					{
						Disp_MJpeg_To_Wifi(display_addr);
					}
					else
				#endif
					{
						if ((curr_disp_dev==DISP_DEV_TFT) || (curr_disp_dev==DISP_DEV_TV)) {
							OSQPost(DisplayTaskQ, (void *) (display_addr|MSG_DISPLAY_TASK_MJPEG_DRAW));	
						} else if (curr_disp_dev==DISP_DEV_HDMI) {
							OSQPost(DisplayTaskQ, (void *) (display_addr|MSG_DISPLAY_TASK_HDMI_MJPEG_DRAW));
							//DBG_PRINT("x");
						}
					}
			}
			
EndOf_MSG_VID_DEC_ONE_FRAME:
			if(success_flag)
			{
				#if DUAL_STREAM_FUNC_ENABLE
				if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
				{
					if(display_addr)
					{
						ap_display_queue_put(display_isr_queue, display_addr);	
					}
				}
				#endif

				OS_ENTER_CRITICAL();
				p_vid_dec_para->Tv += delta_Tv;
				OS_EXIT_CRITICAL();
			}
			
			// get video raw data
			raw_data_addr = (INT32U)MultiMediaParser_GetVidBuf(p_vid_dec_para->media_handle, &size, &Duration, &error_id);
			size += raw_data_addr & 3;	//align and size increase
			raw_data_addr -= raw_data_addr & 3;	//align addr	
			while(error_id<0);
			
			if(p_wave_info)
			{
           		delta_Tv = (INT64S)Duration * p_wave_info->nSamplesPerSec*2;
       		}
       		else
       		{
       			p_vid_dec_para->n = 1;       			
       			delta_Tv = (INT64S)Duration * TIME_BASE_TICK_RATE;
       		}

EndOf_MSG_VID_DEC_ONE_FRAME_doNothing:
			// allow next frame input
			p_vid_dec_para->pend_cnt++;
			break;
		
		case MSG_VID_DEC_START:
			scaler_out_addr = 0; 
		case MSG_VID_DEC_RESTART:
			init_flag = 0;
			delta_Tv = 0;
			p_vid_dec_para->fail_cnt = 0;
			// pre-latch video raw data
			while(1)
			{	
				raw_data_addr = (INT32U)MultiMediaParser_GetVidBuf(p_vid_dec_para->media_handle, &size, &Duration, &error_id);
				size += raw_data_addr & 3;	//align and size increase
				raw_data_addr -= raw_data_addr & 3;	//align addr	
				while(error_id<0);
				if(nth_flag) 
				{ 
					if(raw_data_addr && (size >= 10)) break;
				}
				else 
				{
					if(raw_data_addr) break;	
				}
				OSTimeDly(1);
			}
			
			if(p_wave_info)
			{
           		delta_Tv = (INT64S)Duration * p_wave_info->nSamplesPerSec*2;
       		}
       		else
   			{
       			p_vid_dec_para->n = 1;       			
       			delta_Tv = (INT64S)Duration * TIME_BASE_TICK_RATE;
       		}	

			switch(p_vid_dec_para->video_format)
			{
			case C_MJPG_FORMAT:
				init_flag = 1;	
				break;
				
			default:
				while(1);
			}	
			
			if(nth_flag) break;
			OSMboxPost(vid_dec_ack_m, (void*)C_ACK_SUCCESS);
			break;

		case MSG_VID_DEC_STOP:
			if(nth_flag) nth_flag = 0;
			video_decode_task_end();
			OSQFlush(vid_dec_q);
			OSMboxPost(vid_dec_ack_m, (void*)C_ACK_SUCCESS);
			break;	
			
		case MSG_VID_DEC_NTH:
			if(OSQPost(vid_dec_q, (void*)MSG_VID_DEC_START) != OS_NO_ERR)
				goto MSG_VID_DEC_NTH_Fail;
			
			if(OSQPost(vid_dec_q, (void*)MSG_VID_DEC_ONE_FRAME) != OS_NO_ERR)
				goto MSG_VID_DEC_NTH_Fail;
			
			if(OSQPost(vid_dec_q, (void*)MSG_VID_DEC_STOP) != OS_NO_ERR)
				goto MSG_VID_DEC_NTH_Fail;
			nth_flag = 1;
			continue;
MSG_VID_DEC_NTH_Fail:
			nth_flag = 0;
			OSQFlush(vid_dec_q);
			OSMboxPost(vid_dec_ack_m, (void*)C_ACK_FAIL);	
			break;
			
		case MSG_VID_DEC_EXIT:
			OSMboxPost(vid_dec_ack_m, (void*)C_ACK_SUCCESS);
			OSTaskDel(OS_PRIO_SELF);
			break;	
		}
	}
}		

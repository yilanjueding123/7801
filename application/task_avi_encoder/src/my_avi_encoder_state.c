#include "my_avi_encoder_state.h"
#include "avi_encoder_app.h"
#include "ap_state_config.h"
#include "ap_display.h"
#include "ap_state_resource.h"
#include "video_encoder.h"

#include "drv_l1_jpeg.h"
#include "jpeg_header.h"

#include "state_wifi.h"

//+++
#include "LDWs.h"
#if (Enable_Lane_Departure_Warning_System)

extern INT32U LDWS_buf_malloc_cnt; 
extern INT32U LDWS_buf_addrs;
extern INT32U LDWS_buf_size;
extern INT8U  LDWS_buf_addrs_idx;
extern INT8U  LDWS_start_fifo_idx;
extern INT8U  LDWS_end_fifo_idx;
extern INT8U  LDWS_Enable_Flag;
extern INT8U  LDWS_Key_On_flag;


INT8U LDWS_MSG_Send_flag;
LDWsParameter LDWPar = {0};
#endif

/****************************************************************************/
#if PRINTF_WIFI_SPEED_ENABLE
	INT32U Wifi_Jpeg_Count;
	INT32U Wifi_Jpeg_Data_Byte;
	INT32U Wifi_Jpeg_Start_Time;
#endif

/****************************************************************************/
#define ENABLE_DYNAMIC_TUNING_JPEG_Q	1
#define AVI_Y_Q_VALUE_VIDEO_720P		50
#define AVI_Y_Q_VALUE_VIDEO_1080FHD		40
#define AVI_UV_Q_VALUE_VIDEO_1080FHD	30

#if ENABLE_DYNAMIC_TUNING_JPEG_Q
INT32S current_Y_Q_value;
INT32S target_Y_Q_value;

INT32S current_UV_Q_value;
INT32S target_UV_Q_value;

INT32U current_VLC_size;
INT32U max_VLC_size;

static INT32U cnt = 0, full_size_cnt = 0, jpeg_size_sum = 0;
//volatile INT32U cnt = 0, full_size_cnt = 0, jpeg_size_sum = 0;
static INT32U disp_cnt = 0, disp_full_size_cnt = 0, disp_jpeg_size_sum = 0;

INT32S disp_current_Y_Q_value;
INT32S disp_current_UV_Q_value;

INT32S disp_target_Y_Q_value;
INT32S disp_target_UV_Q_value;

#endif
//---

/****************************************************************************/
#if DUAL_STREAM_FUNC_ENABLE
INT32U jpeg_disp_out_buffer_size;
#define JPEG_DISP_OUT_COUNT			3
//#define JPEG_DISP_OUT_BUFFER_SIZE_640_360	(60*1024)//(40*1024)
//#define JPEG_DISP_OUT_BUFFER_SIZE_480_272	(30*1024)
INT32U JPEG_DISP_OUT_BUFFER_SIZE;

extern INT32U JPEG_DISP_OUT_BUFFER_SIZE_640_360;
extern INT32U JPEG_DISP_OUT_BUFFER_SIZE_480_272;

typedef struct jpeg_disp_comress_args_s
{
	INT32U Disp_Fifo_Line_Cnt;
	INT32U Disp_rec_fifo_data_size;
	INT32U Disp_Frame_Buff_Addrs;
	INT32U Disp_Max_Vlc_Size;
	INT8U  Disp_Skip_WIFI_Jpeg_Flag;		
}jpeg_disp_comress_args_t;

mjpeg_write_data_t mjpegWriteData[JPEG_DISP_OUT_COUNT];

jpeg_disp_comress_args_t gDispCompressArgs = {0};

extern INT32S mjpeg_send_picture(mjpeg_write_data_t* pMjpegWData);

#else	
	#define JPEG_DISP_OUT_COUNT		0
#endif

INT32U jpeg_disp_out_buffer_cnt = 0;

/****************************************************************************/
#define PREVIEW_ZOOM_OUT 	0
#define PREVIEW_ZOOM_IN 	1

//+++ Queue
/*
						640		  *32          *1.5		
	iRAM(200K) / 30K [SENSOR_WIDTH*FIFO_LINE_LN*1.5] = 6 
*/
#define FIFO_Q_MAX_CNT	6
INT8U USE_SDRAM_SUPPORT_FIFO_CNT = 0;
INT32U USE_SDRAM_SUPPORT_FIFO_ADDRS_START = 0; 
INT32U USE_SDRAM_SUPPORT_FIFO_ADDRS_END = 0; 


INT32U fifoQArray[FIFO_Q_MAX_CNT];

//+++ Jpeg
#define JPEG_Q_MAX_CNT  32

#define JPEG_VLC_BUFFER_CNT		2
#define JPEG_VLC_BUFFER_SIZE	1024*1024

#define  JPEG_OUT_BUFFER_CNT_MAX  (JPEG_DISP_OUT_COUNT+JPEG_OUT_BUFFER_LARGE_CNT+JPEG_OUT_BUFFER_MIDDLE_CNT+JPEG_OUT_BUFFER_SMALL_CNT)
static INT32U JPEG_OUT_BUFFER_CNT = 0;

INT32U Jpeg_Vlc_Total_Size = 0;
//+++ Fifo
INT8U FIFO_LINE_LN = 0;

#define FIFO_USE_FORMAT FIFO_FORMAT_420

//CDSP
INT32S ae_gain_get(TIME_T  *tm);
extern INT8S gp_cdsp_set_exposure_time(void);

/****************************************************************************/

extern INT32U vid_buf_addr[JPEG_UVC_BUFFER_LARGE_CNT+JPEG_UVC_BUFFER_MIDDLE_CNT+JPEG_UVC_BUFFER_SMALL_CNT];
INT32S usb_webcam_buffer_id(INT32U *pBuf);

/****************************************************************************/
typedef struct jpeg_Q_args_s
{
	INT32U jpeg_buffer_addrs;
	INT32U jpeg_fifo_msg;
	INT32U jpeg_fifo_idx;
	#if DUAL_STREAM_FUNC_ENABLE	
	INT32U jpeg_fifo_source;
	#endif
}jpeg_Q_args_t;

typedef struct jpeg_comress_args_s
{
	jpeg_Q_args_t jpeg_Q_array[JPEG_Q_MAX_CNT];
	INT32U jpeg_fifo_source;
	INT32U jpeg_input_addrs;
	INT32U jpeg_input_size;
	AVIPACKER_FRAME_INFO jpeg_output_addrs[JPEG_OUT_BUFFER_CNT_MAX];
	INT32U jpeg_img_width;
	INT32U jpeg_img_height;
	INT32U jpeg_isr_status_flag;
	INT32U jpeg_vlc_size;
	INT32U jpeg_engine_status;
	void   (*jpeg_status_notify)(INT32S eventMsg, INT32U vlcCount);
	INT32U jpeg_file_handle;
	INT8U  jpeg_img_format;
	INT8U  jpeg_output_addrs_idx;
	INT8U  jpeg_send_to_target;
	INT8U  jpeg_enable_scaler_up_func_flag; 
	INT8U  jpeg_skip_this_one_flag;
	INT8U  jpeg_input_empty_count;
	INT8U  jpeg_input_empty_count_max;	
	#if DUAL_STREAM_FUNC_ENABLE	
	INT32U jpeg_video_next_fifo_addrs;
	INT32S jpeg_video_max_vlc_size;
	INT8U  jpeg_video_new_jpeg_buffer;
	INT8U  jpeg_video_out_jpeg_buf_idx;
	INT8U  jpeg_video_Header_Restart_Interval_Count;

	INT32U jpeg_disp_next_fifo_addrs;
	INT32S jpeg_disp_max_vlc_size;
	INT8U  jpeg_disp_new_jpeg_buffer;
	INT8U  jpeg_disp_out_jpeg_buf_idx;
	INT8U  jpeg_disp_Header_Restart_Interval_Count;
	INT8U  jpeg_disp_input_empty_count;
	INT8U  jpeg_disp_input_empty_max_count;
	INT8U  jpeg_disp_encode_end_flag;	
	#endif	
}jpeg_comress_args_t;

typedef enum
{
    JPEG_ENGINE_STATUS_IDLE,                     
    JPEG_ENGINE_STATUS_BUSY,
    JPEG_ENGINE_STATUS_WAIT,
    JPEG_ENGINE_STATUS_ERR_OCCUR
} JPEG_ENGINE_STATUS;

typedef enum
{
	JPEG_SEND_FOR_NOTHING,
	JPEG_SEND_FOR_RECORD,
	JPEG_SEND_FOR_PC_CAM,
	JPEG_SEND_FOR_PHOTO,    // Send to file system
	JPEG_SEND_FOR_DISPLAY
}JPEG_SEND_TARGET;

jpeg_comress_args_t gJpegCompressArgs = {0};


INT32U gAviEncoderState = AVI_ENCODER_STATE_IDLE;
/****************************************************************************/
OS_EVENT* my_AVIEncodeApQ;
OS_EVENT* my_avi_encode_ack_m;

#define MY_AVI_ENCODE_QUEUE_MAX_LEN    	60
void* my_AVIEncodeApQ_Stack[MY_AVI_ENCODE_QUEUE_MAX_LEN];

#define MY_C_AVI_ENCODE_STATE_STACK_SIZE   512
INT32U my_AVIEncodeStateStack[MY_C_AVI_ENCODE_STATE_STACK_SIZE];


/*********************** ¥­§¡±¼´Vºtºâªk ***************************************/
#define  AVERAGE_DROP_FRAME_EN  1
#define	AVI_PACKER_TH	2
static INT32U time_stamp_buffer[JPEG_OUT_BUFFER_CNT_MAX];
static INT32S time_stamp_buffer_size = 0;
static INT32S free_JPEG_buffer_size = 0;
static INT32U first_time = 0;
static INT32U last_time = 0;
/****************************************************************************/

void jpeg_compress_free_addrs_set(jpeg_comress_args_t* pJpegCompressArgs,INT8U freeAddrsIdx);
void jpeg_quality_set(INT8U Y_quality_value,INT8U UV_quality_value);
#if DUAL_STREAM_FUNC_ENABLE
void wifi_jpeg_quality_set(INT8U Y_quality_value,INT8U UV_quality_value);
#endif

/****************************************************************************/
#if ENABLE_DYNAMIC_TUNING_JPEG_Q
/****************************************************************************/
/*
 *	dynamic_tune_jpeg_Q
 */
#define Q_Y_STEP 2
#define Q_UV_STEP 2
#define Q_DISTANCE 0

// 1080P/1080FHD
#define Q_LARGE_Y_MAX 		70
#define Q_LARGE_Y_MIN		10
#define Q_LARGE_Y_EMER		15

#define Q_LARGE_UV_MAX 	(Q_LARGE_Y_MAX-Q_DISTANCE)
#define Q_LARGE_UV_MIN 	    10
#define Q_LARGE_UV_EMER 	15
// 720P/WVGA/VGA
#define Q_SMALL_Y_MAX		80
#define Q_SMALL_Y_MIN		15
#define Q_SMALL_Y_EMER		20

#define Q_SMALL_UV_MAX		(Q_SMALL_Y_MAX-Q_DISTANCE)
#define Q_SMALL_UV_MIN		15
#define Q_SMALL_UV_EMER		20

#define DISP_Q_Y_STEP 		1
#define DISP_Q_UV_STEP 		1
#define DISP_Q_DISTANCE 	5

#define DISP_Q_SMALL_Y_MAX		60
#define DISP_Q_SMALL_Y_MIN		30
#define DISP_Q_SMALL_UV_MAX		(DISP_Q_SMALL_Y_MAX-DISP_Q_DISTANCE)
#define DISP_Q_SMALL_UV_MIN		30

void system_check(void)
{
	INT32U resolution_flag = 0;

	if (*((volatile INT32U *) 0xF840001C))
	{
		resolution_flag = 1;
	}
	if (resolution_flag)
	{
		R_TFT_CTRL = 0;
		R_JPG_CTRL = 0;
		*P_SCALER0_BASE = 0;
		*P_SCALER1_BASE = 0;
		R_SDC_CTRL = 0;
		////////////////////////
		R_TV_CTRL =  0;
		R_TV_CTRL2 = 0;
		R_TV_FBI_ADDR = 0xF8500500;
		DBG_PRINT("Err\r\n");				
	}
}

#if DUAL_STREAM_FUNC_ENABLE	
static void jpeg_Q_adjust(INT32U srcFrom, INT32S val)
{
	if(srcFrom == FIFO_SOURCE_FROM_VIDEO)
	{
		if (val<0)
		{	if ((current_Y_Q_value-current_UV_Q_value)>=Q_DISTANCE) {
				current_Y_Q_value -= Q_Y_STEP;
			}
			else {
				current_UV_Q_value -= Q_UV_STEP;
			}
		}
		else if (val >0)
		{
			if ((current_Y_Q_value-current_UV_Q_value)>=Q_DISTANCE) {
				current_UV_Q_value += Q_UV_STEP;
			}
			else {
				current_Y_Q_value += Q_Y_STEP;
			}
		}
	}
	else
	{
		if (val<0)
		{	if ((disp_current_Y_Q_value-disp_current_UV_Q_value)>= DISP_Q_DISTANCE) {
				disp_current_Y_Q_value -= DISP_Q_Y_STEP;
			}
			else {
				disp_current_UV_Q_value -= DISP_Q_UV_STEP;
			}
		}
		else if (val >0)
		{
			if ((disp_current_Y_Q_value-disp_current_UV_Q_value)>= DISP_Q_DISTANCE) {
				disp_current_UV_Q_value += DISP_Q_UV_STEP;
			}
			else {
				disp_current_Y_Q_value += DISP_Q_Y_STEP;
			}
		}
	}
}

extern void Dynamic_Tune_Q(INT32U vlcSize);
extern void Display_Dynamic_Tune_Q(INT32U vlcSize,INT32U disp_fullsize_cnt);
void dynamic_tune_jpeg_Q(INT32U srcFrom, INT32U vlcSize)
{
	INT32U jpeg_avg_size;

	if(srcFrom == FIFO_SOURCE_FROM_VIDEO)
	{	
	/*	if(vlcSize >= max_VLC_size)
		{
			full_size_cnt++;
		}
		jpeg_size_sum += vlcSize;
		cnt++;

		if ( (cnt>=0x20)||(full_size_cnt) )
		{
			 jpeg_avg_size = jpeg_size_sum>>5;	// ¤@±i¥­§¡¤j¤p

	 		if ( (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||(my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P) )
			{
	 			if (full_size_cnt)
				{
					current_Y_Q_value -= Q_DISTANCE;
					current_UV_Q_value -= Q_DISTANCE;
				}
				else if (jpeg_avg_size > (max_VLC_size-(45*1024)))
				{
					jpeg_Q_adjust(FIFO_SOURCE_FROM_VIDEO,-1);
				}
				else if (jpeg_avg_size < (max_VLC_size-(60*1024)))
				{
					jpeg_Q_adjust(FIFO_SOURCE_FROM_VIDEO,1);			
				}

				if  ( current_Y_Q_value < Q_LARGE_Y_MIN ) 	{
					current_Y_Q_value = Q_LARGE_Y_MIN;
				}
				if  ( current_UV_Q_value < Q_LARGE_UV_MIN ) 	{
					current_UV_Q_value = Q_LARGE_UV_MIN;
				}
				if  (current_Y_Q_value > Q_LARGE_Y_MAX) {
					current_Y_Q_value = Q_LARGE_Y_MAX;
				}
				if  ( current_UV_Q_value > Q_LARGE_UV_MAX ) {
					current_UV_Q_value = Q_LARGE_UV_MAX;
				}
	 		}
			else	 // other size (720P, WVGA, VGA)
			{
				if (full_size_cnt)
				{
					current_Y_Q_value -= Q_DISTANCE;
					current_UV_Q_value -= Q_DISTANCE;
				}
				else if (jpeg_avg_size > (max_VLC_size-(40*1024)))
				{
					jpeg_Q_adjust(FIFO_SOURCE_FROM_VIDEO,-1);
				}
				else if (jpeg_avg_size < (max_VLC_size-(55*1024)))
				{
					jpeg_Q_adjust(FIFO_SOURCE_FROM_VIDEO,1);
				}

				if  ( current_Y_Q_value < Q_SMALL_Y_MIN ) 	{
					current_Y_Q_value = Q_SMALL_Y_MIN;
				}
				if  ( current_UV_Q_value < Q_SMALL_UV_MIN ) 	{
					current_UV_Q_value = Q_SMALL_UV_MIN;
				}
				if   (current_Y_Q_value > Q_SMALL_Y_MAX) {
					current_Y_Q_value = Q_SMALL_Y_MAX;
				}
				if  ( current_UV_Q_value > Q_SMALL_UV_MAX ) {
					current_UV_Q_value = Q_SMALL_UV_MAX;
				}			
			}

			if ( (current_Y_Q_value!=target_Y_Q_value)||(current_UV_Q_value!=target_UV_Q_value) )
			{
				//DBG_PRINT("Y=%d, UV=%d\r\n",current_Y_Q_value, current_UV_Q_value);
			}
			
			full_size_cnt = 0;
			jpeg_size_sum = 0;
			cnt = 0;
		}
	*/	
	Dynamic_Tune_Q(vlcSize);
	#if 0
	 if(vlcSize > (max_VLC_size - 10*1024)) { // Èç¹û JPEG > (max_VLC_size - 5KB) »ò ±¬µô£¬ñRÉÏ Q ÖµÕ{µÍ 
			if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) || (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P))
			{  // 1080P 
				if((current_Y_Q_value > Q_LARGE_Y_EMER) || (current_UV_Q_value > Q_LARGE_UV_EMER)) {
					current_Y_Q_value = Q_LARGE_Y_EMER;
					current_UV_Q_value = Q_LARGE_UV_EMER;
				} else {
					current_Y_Q_value = Q_LARGE_Y_MIN;
					current_UV_Q_value = Q_LARGE_UV_MIN;
				}
			} else { // 720P 
				if((current_Y_Q_value > Q_SMALL_Y_EMER) || (current_UV_Q_value > Q_SMALL_UV_EMER)) {
					current_Y_Q_value = Q_SMALL_Y_EMER;
					current_UV_Q_value = Q_SMALL_UV_EMER;
				} else {
					current_Y_Q_value = Q_SMALL_Y_MIN;
					current_UV_Q_value = Q_SMALL_UV_MIN;
				}
			}
		} else {
	 		if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||(my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)) {
				if(vlcSize > (115*1024)) {
					if((current_Y_Q_value - current_UV_Q_value) >= Q_DISTANCE) {
						current_Y_Q_value -= Q_Y_STEP;
						if(current_Y_Q_value < Q_LARGE_Y_MIN) {
							current_Y_Q_value = Q_LARGE_Y_MIN;
						}
					} else {
						if(current_UV_Q_value > Q_LARGE_UV_MIN) {
							current_UV_Q_value -= Q_UV_STEP;
							if(current_UV_Q_value < Q_LARGE_UV_MIN) {
								current_UV_Q_value = Q_LARGE_UV_MIN;
							}
						} else {
							current_Y_Q_value -= Q_Y_STEP;
							if(current_Y_Q_value < Q_LARGE_Y_MIN) {
								current_Y_Q_value = Q_LARGE_Y_MIN;
							}
						}
					}
				} else if (vlcSize < (70*1024)) {
					if ((current_Y_Q_value - current_UV_Q_value) >= Q_DISTANCE) {
						current_UV_Q_value += Q_UV_STEP;
						if(current_UV_Q_value > Q_LARGE_Y_MAX) {
							current_UV_Q_value = Q_LARGE_Y_MAX;
						}
					} else {
						if(current_Y_Q_value < Q_LARGE_Y_MAX) {
							current_Y_Q_value += Q_Y_STEP;
							if(current_Y_Q_value > Q_LARGE_Y_MAX) {
								current_Y_Q_value = Q_LARGE_Y_MAX;
							}
						} else {
							current_UV_Q_value += Q_UV_STEP;
							if(current_UV_Q_value > Q_LARGE_UV_MAX) {
								current_UV_Q_value = Q_LARGE_UV_MAX;
							}
						}
					}
				}
	 		} else { //other size (720P, WVGA, VGA)
				if(vlcSize > (100*1024)) {
					if ((current_Y_Q_value - current_UV_Q_value) >= Q_DISTANCE) {
						current_Y_Q_value -= Q_Y_STEP;
						if(current_Y_Q_value < Q_SMALL_Y_MIN) {
							current_Y_Q_value = Q_SMALL_Y_MIN;
						}
					} else {
						if(current_UV_Q_value > Q_SMALL_UV_MIN) {
							current_UV_Q_value -= Q_UV_STEP;
							if(current_UV_Q_value < Q_SMALL_UV_MIN) {
								current_UV_Q_value = Q_SMALL_UV_MIN;
							}
						} else {
							current_Y_Q_value -= Q_Y_STEP;
							if(current_Y_Q_value < Q_SMALL_Y_MIN) {
								current_Y_Q_value = Q_SMALL_Y_MIN;
							}
						}
					}
				} else if (vlcSize < (70*1024)) {
					if ((current_Y_Q_value - current_UV_Q_value) >= Q_DISTANCE) {
						current_UV_Q_value += Q_UV_STEP;
						if(current_UV_Q_value > Q_SMALL_UV_MAX) {
							current_UV_Q_value = Q_SMALL_UV_MAX;
						}
					} else {
						if(current_Y_Q_value < Q_SMALL_Y_MAX) {
							current_Y_Q_value += Q_Y_STEP;
							if(current_Y_Q_value > Q_SMALL_Y_MAX) {
								current_Y_Q_value = Q_SMALL_Y_MAX;
							}
						} else {
							current_UV_Q_value += Q_UV_STEP;
							if(current_UV_Q_value > Q_SMALL_UV_MAX) {
								current_UV_Q_value = Q_SMALL_UV_MAX;
							}
						}
					}
				}
			}
		}
	#endif
		
	       /************** set target value **************/
		target_Y_Q_value = current_Y_Q_value;
		target_UV_Q_value = current_UV_Q_value;
	}
	else //FIFO_SOURCE_FROM_DISPLAY
	{
		if(vlcSize >= JPEG_DISP_OUT_BUFFER_SIZE)
		{
			disp_full_size_cnt++;
		}
		disp_jpeg_size_sum += vlcSize;
		disp_cnt++;

		if ( (disp_cnt>=0x2)||(disp_full_size_cnt) )
		{
			#if 0
			 jpeg_avg_size = disp_jpeg_size_sum>>5;	// ¤@±i¥­§¡¤j¤p
				
			if (disp_full_size_cnt)
			{
				disp_current_Y_Q_value -= DISP_Q_DISTANCE;
				disp_current_UV_Q_value -= DISP_Q_DISTANCE;
			}
			else if (jpeg_avg_size > (JPEG_DISP_OUT_BUFFER_SIZE-(5*1024)))
			{
				jpeg_Q_adjust(FIFO_SOURCE_FROM_DISPLAY,-1);
			}
			else if (jpeg_avg_size < (JPEG_DISP_OUT_BUFFER_SIZE-(15*1024)))
			{
				jpeg_Q_adjust(FIFO_SOURCE_FROM_DISPLAY,1);
			}

			if  ( disp_current_Y_Q_value < DISP_Q_SMALL_Y_MIN ) 	{
				disp_current_Y_Q_value = DISP_Q_SMALL_Y_MIN;
			}
			if  ( disp_current_UV_Q_value < DISP_Q_SMALL_UV_MIN ) 	{
				disp_current_UV_Q_value = DISP_Q_SMALL_UV_MIN;
			}
			if   (disp_current_Y_Q_value > DISP_Q_SMALL_Y_MAX) {
				disp_current_Y_Q_value = DISP_Q_SMALL_Y_MAX;
			}
			if  ( disp_current_UV_Q_value > DISP_Q_SMALL_UV_MAX ) {
				disp_current_UV_Q_value = DISP_Q_SMALL_UV_MAX;
			}			
			
			/*
			if ( (disp_current_Y_Q_value!=disp_target_Y_Q_value)||(disp_current_UV_Q_value!=disp_target_UV_Q_value) )
			{
				DBG_PRINT("Y=%d, UV=%d\r\n",disp_current_Y_Q_value, disp_current_UV_Q_value);
			}
			*/
			#else
			jpeg_avg_size = disp_jpeg_size_sum>>1;
			///DBG_PRINT("avg=%d,vlc=%d\r\n",jpeg_avg_size,vlcSize);
			Display_Dynamic_Tune_Q(jpeg_avg_size,disp_full_size_cnt);
			/*if (disp_full_size_cnt)
			{
				disp_current_Y_Q_value -= 20;	
			}
			else if (jpeg_avg_size > (JPEG_DISP_OUT_BUFFER_SIZE-(15*1024)))
			{
				disp_current_Y_Q_value -= 5;
			}
			else if (jpeg_avg_size < (JPEG_DISP_OUT_BUFFER_SIZE-(25*1024)))
			{
				disp_current_Y_Q_value += 5;
			}

			if(disp_current_Y_Q_value < 10)
			{
				disp_current_Y_Q_value = 10;
			}

			if(disp_current_Y_Q_value >= 50)
			{
				disp_current_Y_Q_value = 50;
			}*/
			#endif

			disp_full_size_cnt = 0;
			disp_jpeg_size_sum = 0;
			disp_cnt = 0;
		}
		
	       /************** set target value **************/
		disp_target_Y_Q_value = disp_current_Y_Q_value;
		disp_target_UV_Q_value = disp_target_Y_Q_value;//disp_current_UV_Q_value;
	}
}
#else
static void jpeg_Q_adjust(INT32S val)
{
	if (val<0)
	{	if ((current_Y_Q_value-current_UV_Q_value)>=Q_DISTANCE) {
			current_Y_Q_value -= Q_Y_STEP;
		}
		else {
			current_UV_Q_value -= Q_UV_STEP;
		}
	}
	else if (val >0)
	{
		if ((current_Y_Q_value-current_UV_Q_value)>=Q_DISTANCE) {
			current_UV_Q_value += Q_UV_STEP;
		}
		else {
			current_Y_Q_value += Q_Y_STEP;
		}
	}
}

void dynamic_tune_jpeg_Q(INT32U vlcSize)
{
	static INT32U cnt = 0, full_size_cnt=0, jpeg_size_sum=0;
	
	if(vlcSize >= max_VLC_size)
	{
		full_size_cnt++;
	}
	jpeg_size_sum += vlcSize;
	cnt++;

	if ( (cnt>=0x20)||(full_size_cnt) )
	{
		INT32U jpeg_avg_size = jpeg_size_sum>>5;	// ¤@±i¥­§¡¤j¤p

 		if ( (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||(my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P) )
		{
 			if (full_size_cnt)
			{
				current_Y_Q_value -= Q_DISTANCE;
				current_UV_Q_value -= Q_DISTANCE;
			}
			else if (jpeg_avg_size > (max_VLC_size-(45*1024)))
			{
				jpeg_Q_adjust(-1);
			}
			else if (jpeg_avg_size < (max_VLC_size-(60*1024)))
			{
				jpeg_Q_adjust(+1);			
			}

			if  ( current_Y_Q_value < Q_LARGE_Y_MIN ) 	{
				current_Y_Q_value = Q_LARGE_Y_MIN;
			}
			if  ( current_UV_Q_value < Q_LARGE_UV_MIN ) 	{
				current_UV_Q_value = Q_LARGE_UV_MIN;
			}
			if  (current_Y_Q_value > Q_LARGE_Y_MAX) {
				current_Y_Q_value = Q_LARGE_Y_MAX;
			}
			if  ( current_UV_Q_value > Q_LARGE_UV_MAX ) {
				current_UV_Q_value = Q_LARGE_UV_MAX;
			}
 		}
		else	 // other size (720P, WVGA, VGA)
		{
			if (full_size_cnt)
			{
				current_Y_Q_value -= Q_DISTANCE;
				current_UV_Q_value -= Q_DISTANCE;
			}
			else if (jpeg_avg_size > (max_VLC_size-(40*1024)))
			{
				jpeg_Q_adjust(-1);
			}
			else if (jpeg_avg_size < (max_VLC_size-(55*1024)))
			{
				jpeg_Q_adjust(1);
			}

			if  ( current_Y_Q_value < Q_SMALL_Y_MIN ) 	{
				current_Y_Q_value = Q_SMALL_Y_MIN;
			}
			if  ( current_UV_Q_value < Q_SMALL_UV_MIN ) 	{
				current_UV_Q_value = Q_SMALL_UV_MIN;
			}
			if   (current_Y_Q_value > Q_SMALL_Y_MAX) {
				current_Y_Q_value = Q_SMALL_Y_MAX;
			}
			if  ( current_UV_Q_value > Q_SMALL_UV_MAX ) {
				current_UV_Q_value = Q_SMALL_UV_MAX;
			}			
		}

		if ( (current_Y_Q_value!=target_Y_Q_value)||(current_UV_Q_value!=target_UV_Q_value) )
		{
			//DBG_PRINT("Y=%d, UV=%d\r\n",current_Y_Q_value, current_UV_Q_value);
		}
		
		full_size_cnt = 0;
		jpeg_size_sum = 0;
		cnt = 0;
	}
	
       /************** set target value **************/
	target_Y_Q_value = current_Y_Q_value;
	target_UV_Q_value = current_UV_Q_value;
	jpeg_quality_set(target_Y_Q_value,target_UV_Q_value);

}	
#endif
#endif

/****************************************************************************/
/*
 *	jpeg_Q_put
 */
#if DUAL_STREAM_FUNC_ENABLE	
INT32S jpeg_Q_put(INT32U fifoSrc,INT32U fifoAddrs, INT32U fifoMsg, INT32U fifoIdx)
#else 
INT32S jpeg_Q_put(INT32U fifoAddrs, INT32U fifoMsg, INT32U fifoIdx)
#endif
{
	INT32U QIdx;
	OS_CPU_SR cpu_sr;

	OS_ENTER_CRITICAL();

	// §äªÅ¦ì¸m©ñ
	for(QIdx=0; QIdx<JPEG_Q_MAX_CNT; QIdx++) 
    { 
		if(gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_buffer_addrs == 0) 
		{ 
	        gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_buffer_addrs = fifoAddrs; 
	        gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_msg = fifoMsg; 
	        gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_idx = fifoIdx; 
	        #if DUAL_STREAM_FUNC_ENABLE         
	        gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_source = fifoSrc; 
	        #endif 
	        OS_EXIT_CRITICAL(); 
	        return STATUS_OK; 
		} 
    } 

	// ³£¨SªÅ¦ì
	OS_EXIT_CRITICAL();
	return STATUS_FAIL;
}

/****************************************************************************/
/*
 *	jpeg_Q_get
 */
#if DUAL_STREAM_FUNC_ENABLE	
INT32U jpeg_Q_get(INT32U* pRetFifoSrc, INT32U* pRetFifoAddrs, INT32U* pRetFifoMsg, INT32U* pRetFifoIdx)
#else
INT32U jpeg_Q_get(INT32U* pRetFifoAddrs, INT32U* pRetFifoMsg, INT32U* pRetFifoIdx)
#endif
{
	INT32U QIdx;
	OS_CPU_SR cpu_sr;

	OS_ENTER_CRITICAL();
	
	if(gJpegCompressArgs.jpeg_Q_array[0].jpeg_buffer_addrs != 0)
	{
		*pRetFifoAddrs = gJpegCompressArgs.jpeg_Q_array[0].jpeg_buffer_addrs;
		*pRetFifoMsg = gJpegCompressArgs.jpeg_Q_array[0].jpeg_fifo_msg;
		*pRetFifoIdx = gJpegCompressArgs.jpeg_Q_array[0].jpeg_fifo_idx;
		#if DUAL_STREAM_FUNC_ENABLE	
		*pRetFifoSrc = gJpegCompressArgs.jpeg_Q_array[0].jpeg_fifo_source;
		#endif

		// ±N«á­±©¹«e²¾¨Ã²M°£³Ì«á¤@­Ó¬°0
		for(QIdx=0; QIdx<(JPEG_Q_MAX_CNT-1); QIdx++)
		{
			gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_buffer_addrs = gJpegCompressArgs.jpeg_Q_array[QIdx+1].jpeg_buffer_addrs;
			gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_msg = gJpegCompressArgs.jpeg_Q_array[QIdx+1].jpeg_fifo_msg;
			gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_idx = gJpegCompressArgs.jpeg_Q_array[QIdx+1].jpeg_fifo_idx;
			#if DUAL_STREAM_FUNC_ENABLE	
			gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_source = gJpegCompressArgs.jpeg_Q_array[QIdx+1].jpeg_fifo_source;
			#endif
		}
		gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_buffer_addrs = 0;
		gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_msg = 0;
		gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_idx = 0;
		#if DUAL_STREAM_FUNC_ENABLE	
		gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_source = 0;
		#endif
		
		OS_EXIT_CRITICAL();
		return STATUS_OK;	
	}
	else
	{
		*pRetFifoAddrs = DUMMY_BUFFER_ADDRS;
		*pRetFifoMsg = 0;
		OS_EXIT_CRITICAL();
		return STATUS_FAIL;
	}
}

/****************************************************************************/
/*
 *	jpeg_Q_clear
 */
void jpeg_Q_clear(void)
{
	INT32U QIdx;

	for(QIdx=0; QIdx<JPEG_Q_MAX_CNT; QIdx++)
	{
		#if DUAL_STREAM_FUNC_ENABLE	
		if(gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_source == FIFO_SOURCE_FROM_DISPLAY)
		{
			if(gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_msg == MSG_VIDEO_ENCODE_FIFO_START)
			{
				ap_display_queue_put(display_isr_queue, gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_buffer_addrs);
			}
		}
		gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_source = 0;		
		#endif
	
		gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_buffer_addrs = 0;
		gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_msg = 0;
		gJpegCompressArgs.jpeg_Q_array[QIdx].jpeg_fifo_idx = 0;
	}
}

/****************************************************************************/
/*
 *	fifo_Q_put
 */
INT32S fifo_Q_put(INT32U QData)
{

	INT32U QIdx;
	OS_CPU_SR cpu_sr;

	OS_ENTER_CRITICAL();

	// §äªÅ¦ì¸m©ñ
	for(QIdx=0; QIdx<FIFO_Q_MAX_CNT; QIdx++)
	{
		if(fifoQArray[QIdx] == 0)
		{
			fifoQArray[QIdx] = QData;
			OS_EXIT_CRITICAL();
			return STATUS_OK;			
		} else if(fifoQArray[QIdx] == QData) {
			OS_EXIT_CRITICAL();
			return STATUS_FAIL;
		}
	}

	// ³£¨SªÅ¦ì
	OS_EXIT_CRITICAL();
	return STATUS_FAIL;
}

/****************************************************************************/
/*
 *	fifo_Q_get
 */
#if 0
INT32U fifo_Q_get(void)
{
	INT32U QIdx;
	INT32U QData;
	OS_CPU_SR cpu_sr;

	OS_ENTER_CRITICAL();
	
	if(fifoQArray[0] != 0)
	{
		QData = fifoQArray[0];

		// ±N«á­±©¹«e²¾¨Ã²M°£³Ì«á¤@­Ó¬°0
		for(QIdx=0; QIdx<(FIFO_Q_MAX_CNT-1); QIdx++)
		{
			fifoQArray[QIdx] = fifoQArray[QIdx+1];
		}
		fifoQArray[QIdx] = 0;
		
		OS_EXIT_CRITICAL();
		return QData;	
	}
	else
	{
		OS_EXIT_CRITICAL();
		return DUMMY_BUFFER_ADDRS;
	}
}
#else
/*
	·í¦³IRAM ¥ý¥Î,¨S¦³¤~¥ÎSDRAM,¥[³t1080P/1080FHD ¨Ï¥Î¨ìSDRAM
*/
INT32U fifo_Q_get(void)
{
	INT32U QIdx;
	INT32U i;
	INT32U QData;
	OS_CPU_SR cpu_sr;

	OS_ENTER_CRITICAL();
	
	for(QIdx=0; QIdx<FIFO_Q_MAX_CNT; QIdx++)
	{
		if(fifoQArray[QIdx] >= 0xF8000000)
		{
			QData = fifoQArray[QIdx];
			break;
		}		
	}
	
	// ¨S§ä¨ìIRAM,¦^¶Ç²Ä1­Ó	
	if(QIdx == FIFO_Q_MAX_CNT)
	{
		QData = fifoQArray[0];
		QIdx = 0;
	}
	
	// ±N«á­±©¹«e²¾¨Ã²M°£³Ì«á¤@­Ó¬°0
	for(i=QIdx; i<(FIFO_Q_MAX_CNT-1); i++)
	{
		fifoQArray[i] = fifoQArray[i+1];
	}
	fifoQArray[i] = 0;

	if(QData != 0)	
	{	
		OS_EXIT_CRITICAL();
		return QData;	
	}
	else
	{
		OS_EXIT_CRITICAL();
		return DUMMY_BUFFER_ADDRS;
	}	
}
#endif
/****************************************************************************/
/*
 *	fifo_Q_clear
 */
void fifo_Q_clear(void)
{
	INT32U QIdx;

	for(QIdx=0; QIdx<FIFO_Q_MAX_CNT; QIdx++)
	{
		fifoQArray[QIdx] = 0;
	}
}

/****************************************************************************/
/*
 *	fifo_ready_notify_encode_callback
 */
INT32U fifo_ready_notify_encode_callback(INT32U fifoMsg, INT32U fifoAddrs, INT32U fifoIdx)
{
	INT32S ret;

	if(fifoAddrs == DUMMY_BUFFER_ADDRS)
	{
		DBG_PRINT("M");
		fifoMsg = MSG_VIDEO_ENCODE_FIFO_ERR;
	}

	#if DUAL_STREAM_FUNC_ENABLE
	ret = jpeg_Q_put(FIFO_SOURCE_FROM_VIDEO,fifoAddrs,fifoMsg,fifoIdx);
	#else
	ret = jpeg_Q_put(fifoAddrs,fifoMsg,fifoIdx);
	#endif
	if(ret == STATUS_FAIL) {
		#if (Enable_Lane_Departure_Warning_System)
		if(((fifoAddrs >= 0xF8000000) && (fifoAddrs < 0xF8040000)) ||
		   ((fifoAddrs >= USE_SDRAM_SUPPORT_FIFO_ADDRS_START) && (fifoAddrs < USE_SDRAM_SUPPORT_FIFO_ADDRS_END))
		)
		#endif
		{
			if(fifoAddrs != DUMMY_BUFFER_ADDRS) {
				fifo_Q_put(fifoAddrs);
			}
		}
	} else if(ret == STATUS_OK) {
		OSQPost(my_AVIEncodeApQ, (void*)MSG_JPEG_QUEUE_NOTIFY);
	}
	return STATUS_OK;
}

/****************************************************************************/
/*
 *	fifo_ready_notify_capture_callback
 */
INT32U fifo_ready_notify_capture_callback(INT32U fifoMsg, INT32U fifoAddrs, INT32U fifoIdx)
{
	OSQPost(my_AVIEncodeApQ, (void*)MSG_JPEG_CAPTURE_FRAME_DONE);
	return STATUS_OK;
}

/****************************************************************************/
/*
 *	scaler_ready_notify_encode_callback => For capture photo
 */
INT32U scaler_ready_notify_encode_callback(INT32U fifoMsg, INT32U fifoAddrs)
{
	#if DUAL_STREAM_FUNC_ENABLE	
	jpeg_Q_put(FIFO_SOURCE_FROM_VIDEO,fifoAddrs,fifoMsg,0);
	#else
	jpeg_Q_put(fifoAddrs,fifoMsg,0);
	#endif

	// ³qª¾ scaler engine buffer ªÅ¶¡¥Î±¼¤@­Ó
	scale_up_buffer_idx_decrease();
	
	OSQPost(my_AVIEncodeApQ, (void*)MSG_JPEG_QUEUE_NOTIFY);

	return STATUS_OK;
}

/****************************************************************************/
/*
 *	display_buffer_pointer_get_func
 */
INT32U display_buffer_pointer_get_func(void)
{
	INT32U display_buffer_addrs;
	
	display_buffer_addrs = ap_display_queue_get(display_isr_queue); 

	if(display_buffer_addrs == NULL)
	{
		return DUMMY_BUFFER_ADDRS;
	}
	else
	{
		return display_buffer_addrs;
	}
}

/****************************************************************************/
/*
 *	jpeg_status_notify
 */
void jpeg_status_notify(INT32S eventMsg, INT32U vlcCount)
{
	gJpegCompressArgs.jpeg_isr_status_flag = eventMsg;

 	#if !DUAL_STREAM_FUNC_ENABLE
		#if ENABLE_DYNAMIC_TUNING_JPEG_Q 
		current_VLC_size = vlcCount;
	 	#endif	
	#endif
	
	if(gJpegCompressArgs.jpeg_isr_status_flag & C_JPG_STATUS_ENCODE_DONE)
	{		
		gJpegCompressArgs.jpeg_isr_status_flag = 0;
		#if DUAL_STREAM_FUNC_ENABLE	
		gJpegCompressArgs.jpeg_vlc_size = vlcCount;
		#else
		gJpegCompressArgs.jpeg_vlc_size = vlcCount + 624;
		#endif
		OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_ENCODE_DONE);
		return;
	}

	if(gJpegCompressArgs.jpeg_isr_status_flag  & C_JPG_STATUS_OUTPUT_FULL)
	{	
		gJpegCompressArgs.jpeg_isr_status_flag &= ~C_JPG_STATUS_OUTPUT_FULL;

		// Input Empty & Output Full ¦P®É²£¥Í
		if(gJpegCompressArgs.jpeg_isr_status_flag  & C_JPG_STATUS_INPUT_EMPTY)
		{
			gJpegCompressArgs.jpeg_isr_status_flag &= ~C_JPG_STATUS_INPUT_EMPTY;
			OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_INPUT_EMPTY_OUTPUT_FULL);
		}
		else
		{	
			OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_OUTPUT_FULL);
		}
	}	
	
	if(gJpegCompressArgs.jpeg_isr_status_flag  & C_JPG_STATUS_INPUT_EMPTY)
	{	
		gJpegCompressArgs.jpeg_isr_status_flag &= ~C_JPG_STATUS_INPUT_EMPTY;
		OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_INPUT_EMPTY);
	}
}

/****************************************************************************/
/*
 *	jpeg_compress_free_addrs_get
 */
#define NEXT_JPEG_ARRIVAL_TIME 7  // 30fps => 33.33ms + 33.33ms

#if DUAL_STREAM_FUNC_ENABLE
INT32U jpeg_compress_free_addrs_get(jpeg_comress_args_t* pJpegCompressArgs)
{
	INT32U i, i_bak;
	INT32U bufStart, bufEnd;
	INT32U free_JPEG_buffer_count;

	i_bak = 0;

	if(pJpegCompressArgs->jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
	{
		free_JPEG_buffer_size = 0;	
		
		bufStart = 0;
		bufEnd = JPEG_OUT_BUFFER_CNT;
	}
	else // FIFO_SOURCE_FROM_DISPLAY
	{
		free_JPEG_buffer_count = 0;
		
		bufStart = JPEG_OUT_BUFFER_CNT;
		bufEnd = JPEG_OUT_BUFFER_CNT+jpeg_disp_out_buffer_cnt;
	}

	for(i=bufStart; i<bufEnd; i++)
	{
		if(pJpegCompressArgs->jpeg_output_addrs[i].is_used == 0)
		{
			i_bak = i;

			if(pJpegCompressArgs->jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
			{

				free_JPEG_buffer_size++;
			}
			else
			{
				free_JPEG_buffer_count++;
			}
		}
	}

	//DBG_PRINT("%d_",free_JPEG_buffer_size);
#if AVERAGE_DROP_FRAME_EN
	if(pJpegCompressArgs->jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
	{
		// free_JPEG_buffer_size==0  ¨S¦³ªÅ buffer
		// time_stamp_buffer_size==0      ¨S¦³½w½Ä¸ê®Æ
		if (  (free_JPEG_buffer_size==0)&&(time_stamp_buffer_size)  )
		{
			INT32U i;
			INT32U min_idx = time_stamp_buffer_size;
			INT32S min_delay = 0x10000, delay;
			INT32U prev, next;

			last_time = OSTimeGet();
			// ÀY¤@´V
			next  = gJpegCompressArgs.jpeg_output_addrs[ time_stamp_buffer[1]  ].buffer_time;
			prev  = first_time;
			delay = (INT32S)next - (INT32S)prev;
			if (delay < min_delay)		// ©¹«á§ä
			{
				min_idx = 0;
				min_delay = delay;
			}
			// ¤¤¶¡´V
			for (i=1;i<time_stamp_buffer_size;i++)
			{
				if (i==time_stamp_buffer_size-1) {
					next = last_time;
				}
				else {
					next = gJpegCompressArgs.jpeg_output_addrs[ time_stamp_buffer[i+1]  ].buffer_time;
				}
				prev  = gJpegCompressArgs.jpeg_output_addrs[ time_stamp_buffer[i-1]  ].buffer_time;
				delay = (INT32S)next - (INT32S)prev;
				if (delay < min_delay)		// ©¹«á§ä
				{
					min_idx = i;
					min_delay = delay;
				}		
			}
			// ¥½¤@´V
			next = last_time+NEXT_JPEG_ARRIVAL_TIME;
			prev  = gJpegCompressArgs.jpeg_output_addrs[ time_stamp_buffer[time_stamp_buffer_size-1]  ].buffer_time;
			delay = (INT32S)next - (INT32S)prev;	
			if (delay < min_delay)		// ©¹«á§ä
			{
				min_idx = time_stamp_buffer_size;
				min_delay = delay;
			}
			
			// §PÂ_
			if (min_idx < time_stamp_buffer_size)
			{	// ¨M©w¥á´V
				jpeg_compress_free_addrs_set(&gJpegCompressArgs, time_stamp_buffer[min_idx]);
				i_bak = time_stamp_buffer[min_idx];
				time_stamp_buffer_size--;
				if(time_stamp_buffer_size > 0) {
					for (i=min_idx; i<time_stamp_buffer_size; ++i)
					{
						time_stamp_buffer[i] = time_stamp_buffer[i+1];
					}
					time_stamp_buffer[i] = 0;
				} else {
					time_stamp_buffer[0] = 0;
				}
				free_JPEG_buffer_size = 1;
			}
		}
	}
#endif

	if(pJpegCompressArgs->jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
	{
		if (free_JPEG_buffer_size != 0)
		{
			pJpegCompressArgs->jpeg_output_addrs[i_bak].is_used = 1;
			pJpegCompressArgs->jpeg_output_addrs_idx = i_bak;
			return pJpegCompressArgs->jpeg_output_addrs[i_bak].buffer_addrs;	
		}
		else
		{
			pJpegCompressArgs->jpeg_output_addrs_idx = bufEnd;
			return DUMMY_BUFFER_ADDRS;
		}
	}
	else // FIFO_SOURCE_FROM_DISPLAY
	{
		if (free_JPEG_buffer_count != 0)
		{
			pJpegCompressArgs->jpeg_output_addrs[i_bak].is_used = 1;
			pJpegCompressArgs->jpeg_output_addrs_idx = i_bak;
			return pJpegCompressArgs->jpeg_output_addrs[i_bak].buffer_addrs;	
		}
		else
		{
			pJpegCompressArgs->jpeg_output_addrs_idx = bufEnd;
			return DUMMY_BUFFER_ADDRS;
		}
	}
}
#else
INT32U jpeg_compress_free_addrs_get(jpeg_comress_args_t* pJpegCompressArgs)
{
	INT32U i, i_bak;

	free_JPEG_buffer_size = 0;
	i_bak = 0;
	for(i=0; i<JPEG_OUT_BUFFER_CNT; i++)
	{
		if(pJpegCompressArgs->jpeg_output_addrs[i].is_used == 0)
		{
			i_bak = i;
			free_JPEG_buffer_size++;
		}
	}

	//DBG_PRINT("%d_",free_JPEG_buffer_size);
#if AVERAGE_DROP_FRAME_EN
	// free_JPEG_buffer_size==0  ¨S¦³ªÅ buffer
	// time_stamp_buffer_size==0      ¨S¦³½w½Ä¸ê®Æ
	if (  (free_JPEG_buffer_size==0)&&(time_stamp_buffer_size)  )
	{
		INT32U i;
		INT32U min_idx = time_stamp_buffer_size;
		INT32S min_delay = 0x10000, delay;
		INT32U prev, next;

		last_time = OSTimeGet();
		// ÀY¤@´V
		next  = gJpegCompressArgs.jpeg_output_addrs[ time_stamp_buffer[1]  ].buffer_time;
		prev  = first_time;
		delay = (INT32S)next - (INT32S)prev;
		if (delay < min_delay)		// ©¹«á§ä
		{
			min_idx = 0;
			min_delay = delay;
		}
		// ¤¤¶¡´V
		for (i=1;i<time_stamp_buffer_size;i++)
		{
			if (i==time_stamp_buffer_size-1) {
				next = last_time;
			}
			else {
				next = gJpegCompressArgs.jpeg_output_addrs[ time_stamp_buffer[i+1]  ].buffer_time;
			}
			prev  = gJpegCompressArgs.jpeg_output_addrs[ time_stamp_buffer[i-1]  ].buffer_time;
			delay = (INT32S)next - (INT32S)prev;
			if (delay < min_delay)		// ©¹«á§ä
			{
				min_idx = i;
				min_delay = delay;
			}		
		}
		// ¥½¤@´V
		next = last_time+NEXT_JPEG_ARRIVAL_TIME;
		prev  = gJpegCompressArgs.jpeg_output_addrs[ time_stamp_buffer[time_stamp_buffer_size-1]  ].buffer_time;
		delay = (INT32S)next - (INT32S)prev;	
		if (delay < min_delay)		// ©¹«á§ä
		{
			min_idx = time_stamp_buffer_size;
			min_delay = delay;
		}
		
		// §PÂ_
		if (min_idx < time_stamp_buffer_size)
		{	// ¨M©w¥á´V
			jpeg_compress_free_addrs_set(&gJpegCompressArgs, time_stamp_buffer[min_idx]);
			i_bak = time_stamp_buffer[min_idx];
			time_stamp_buffer_size--;
			if(time_stamp_buffer_size > 0) {
				for (i=min_idx; i<time_stamp_buffer_size; ++i)
				{
					time_stamp_buffer[i] = time_stamp_buffer[i+1];
				}
				time_stamp_buffer[i] = 0;
			} else {
				time_stamp_buffer[0] = 0;
			}
			free_JPEG_buffer_size = 1;
		}
	}
#endif

	if (free_JPEG_buffer_size!=0)
	{
		pJpegCompressArgs->jpeg_output_addrs[i_bak].is_used = 1;
		pJpegCompressArgs->jpeg_output_addrs_idx = i_bak;
		return pJpegCompressArgs->jpeg_output_addrs[i_bak].buffer_addrs;	
	}
	else
	{
		pJpegCompressArgs->jpeg_output_addrs_idx = JPEG_OUT_BUFFER_CNT;
		//	DBG_PRINT("jpeg no free buf\r\n");
		return DUMMY_BUFFER_ADDRS;
	}
}
#endif

/****************************************************************************/
/*
 *	jpeg_compress_free_addrs_set
 */
void jpeg_compress_free_addrs_set(jpeg_comress_args_t* pJpegCompressArgs,INT8U freeAddrsIdx)
{
	sw_wifi_jpeg_lock();
	pJpegCompressArgs->jpeg_output_addrs[freeAddrsIdx].is_used = 0;	
	sw_wifi_jpeg_unlock();
}

#if DUAL_STREAM_FUNC_ENABLE
/****************************************************************************/
/*
 *	jpeg_compress_FIFO_next_addrs_set
 */
INT8U jpeg_compress_FIFO_next_addrs_set(jpeg_comress_args_t* pJpegCompressArgs)
{
	INT32U jpeg_FFD9_tag_addrs;
	INT32U jpeg_output_addrs;

	if(pJpegCompressArgs->jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
	{
		if(pJpegCompressArgs->jpeg_video_next_fifo_addrs != DUMMY_BUFFER_ADDRS)
		{		
			jpeg_output_addrs = gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_addrs;		
			jpeg_FFD9_tag_addrs = pJpegCompressArgs->jpeg_video_next_fifo_addrs + pJpegCompressArgs->jpeg_vlc_size;
			if(jpeg_FFD9_tag_addrs >= (jpeg_output_addrs+gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_scope))
			{
				OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_OUTPUT_FULL_SW);
				DBG_PRINT("T5");
				return 1;
			}

			jpeg_FFD9_tag_addrs -= 1;			// Replace 0xFFD9
			while ((jpeg_FFD9_tag_addrs & 0xF) != 0xF) {
				*((INT8U *) jpeg_FFD9_tag_addrs++) = 0xFF;
				pJpegCompressArgs->jpeg_vlc_size++;
			}
			*((INT8U *) jpeg_FFD9_tag_addrs++) = 0xD0|((pJpegCompressArgs->jpeg_video_Header_Restart_Interval_Count)&7);	
			pJpegCompressArgs->jpeg_video_Header_Restart_Interval_Count++;
		
			pJpegCompressArgs->jpeg_video_next_fifo_addrs = jpeg_FFD9_tag_addrs;
			pJpegCompressArgs->jpeg_video_max_vlc_size -= pJpegCompressArgs->jpeg_vlc_size;
			/*
				­pºâJPEG OUT Buffer Address ¶W¥X³Ì¤jªÅ¶¡
			*/
			if(pJpegCompressArgs->jpeg_video_max_vlc_size <= 0)
			{
				OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_OUTPUT_FULL_SW);
				DBG_PRINT("T1");
				return 1;
			}
		}
		else		
		{
			/*
				¤W¦¸¥Ñjpeg_compress_FIFO_addrs_get ¨ú¨ìdummy address
				©ñ±ó³o¦¸À£¨îJPEG
			*/
			OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_OUTPUT_FULL_SW);
			DBG_PRINT("T2");
			return 1;
		}
	}
	else //FIFO_SOURCE_FROM_DISPLAY
	{
		if(pJpegCompressArgs->jpeg_disp_next_fifo_addrs != DUMMY_BUFFER_ADDRS)
		{		
			jpeg_output_addrs = gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx].buffer_addrs;
			jpeg_FFD9_tag_addrs = pJpegCompressArgs->jpeg_disp_next_fifo_addrs + pJpegCompressArgs->jpeg_vlc_size;

			if(jpeg_FFD9_tag_addrs >= (jpeg_output_addrs+JPEG_DISP_OUT_BUFFER_SIZE))
			{
				OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_OUTPUT_FULL_SW);
				DBG_PRINT("T6");
				return 1;
			}

			jpeg_FFD9_tag_addrs -= 1;			// Replace 0xFFD9
			while ((jpeg_FFD9_tag_addrs & 0xF) != 0xF) {
				*((INT8U *) jpeg_FFD9_tag_addrs++) = 0xFF;
				pJpegCompressArgs->jpeg_vlc_size++;
			}
			*((INT8U *) jpeg_FFD9_tag_addrs++) = 0xD0|((pJpegCompressArgs->jpeg_disp_Header_Restart_Interval_Count)&7);	
			pJpegCompressArgs->jpeg_disp_Header_Restart_Interval_Count++;
		
			pJpegCompressArgs->jpeg_disp_next_fifo_addrs = jpeg_FFD9_tag_addrs;
			pJpegCompressArgs->jpeg_disp_max_vlc_size -= pJpegCompressArgs->jpeg_vlc_size;
			/*
				­pºâJPEG OUT Buffer Address ¶W¥X³Ì¤jªÅ¶¡
			*/
			if(pJpegCompressArgs->jpeg_disp_max_vlc_size <= 0)
			{
				OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_OUTPUT_FULL_SW);
				DBG_PRINT("T3");
				return 1;
			}
		}
		else
		{
			/*
				¤W¦¸¥Ñjpeg_compress_FIFO_addrs_get ¨ú¨ìdummy address
				©ñ±ó³o¦¸À£¨îJPEG
			*/
			OSQPost(my_AVIEncodeApQ, (void*)MSG_JPG_ISR_OUTPUT_FULL_SW);
			DBG_PRINT("T4");
			return 1;		
		}
	}

	return 0;	
}

/****************************************************************************/
/*
 *	jpeg_compress_FIFO_addrs_get
 */
INT32U jpeg_compress_FIFO_addrs_get(jpeg_comress_args_t* pJpegCompressArgs,INT32U* RetMaxVlcSize)
{
	INT32U RetJpegAddrs;
	INT8U*  Jpeg_Header_Addrs = 0;

	/*
		¥ý¨úªº¤@±iªÅªºJPEG ¦a§}
	*/
	if(pJpegCompressArgs->jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
	{
		if(pJpegCompressArgs->jpeg_video_new_jpeg_buffer == 0)
		{
			pJpegCompressArgs->jpeg_video_new_jpeg_buffer = 1;
			
			RetJpegAddrs = jpeg_compress_free_addrs_get(pJpegCompressArgs);
			/*
				¸õ±¼JPEG Header ¦ì¸m
			*/
			pJpegCompressArgs->jpeg_video_next_fifo_addrs = RetJpegAddrs;
			pJpegCompressArgs->jpeg_video_out_jpeg_buf_idx = pJpegCompressArgs->jpeg_output_addrs_idx;

			if(RetJpegAddrs == DUMMY_BUFFER_ADDRS)
			{
				//DBG_PRINT("[FD]\r\n");
				pJpegCompressArgs->jpeg_video_max_vlc_size = max_VLC_size;
			}
			else
			{
				#if ENABLE_DYNAMIC_TUNING_JPEG_Q		
					jpeg_quality_set(target_Y_Q_value,target_UV_Q_value);

					if(pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_video_out_jpeg_buf_idx].jpeg_Y_Q_value != target_Y_Q_value)
					{
						gp_memcpy((INT8S*)(RetJpegAddrs+16+0x27),(INT8S*)(jpeg_picture_header+0x27),64);
						pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_video_out_jpeg_buf_idx].jpeg_Y_Q_value = target_Y_Q_value;
					}
					// Change UV value
					if(pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_video_out_jpeg_buf_idx].jpeg_UV_Q_value != target_UV_Q_value)
					{
						gp_memcpy((INT8S*)(RetJpegAddrs+16+0x6C),(INT8S*)(jpeg_picture_header+0x6C),64);
						pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_video_out_jpeg_buf_idx].jpeg_UV_Q_value = target_UV_Q_value;
					}
				#endif
			
				/*
					Add Jpeg Header
				*/
				pJpegCompressArgs->jpeg_video_next_fifo_addrs += 640;
				/*
					Find Jpeg Header
				*/
				Jpeg_Header_Addrs = (INT8U*)(RetJpegAddrs+16);

				Jpeg_Header_Addrs[7] = ((my_pAviEncVidPara->encode_height & 0xFF00)>>8);		
				Jpeg_Header_Addrs[8] = (my_pAviEncVidPara->encode_height & 0x00FF);
				
				Jpeg_Header_Addrs[9] = ((my_pAviEncVidPara->encode_width & 0xFF00)>>8);		
				Jpeg_Header_Addrs[10] = (my_pAviEncVidPara->encode_width & 0x00FF);

				if(gJpegCompressArgs.jpeg_img_format == FIFO_FORMAT_422)
				{	
					Jpeg_Header_Addrs[13] = JPEG_IMG_FORMAT_422;		

					if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||
				   	   (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)
				  	  )
					{	
						// ­×§ï interval ¼Æ­È => jpeg image (width*height) / 16*8 (yuyv)
						/*
						¦]¬°¬O§Q¥ÎJPEG ©ñ¤j1.5­¿,©Ò¥HFIFO_LINE_LN­n1.5³B²z
						*/
						Jpeg_Header_Addrs[176] = (((((my_pAviEncVidPara->encode_width*FIFO_LINE_LN*3)>>1)>>7) & 0xFF00)>>8);	
						Jpeg_Header_Addrs[177] = (((((my_pAviEncVidPara->encode_width*FIFO_LINE_LN*3)>>1)>>7) & 0x00FF));			
					}
					else
					{
						// ­×§ï interval ¼Æ­È => jpeg image (width*height) / 16*8 (yuyv)
						Jpeg_Header_Addrs[176] = ((((gJpegCompressArgs.jpeg_img_width*FIFO_LINE_LN)>>7) & 0xFF00)>>8);	
						Jpeg_Header_Addrs[177] = ((((gJpegCompressArgs.jpeg_img_width*FIFO_LINE_LN)>>7) & 0x00FF));			
					}
				}
				else
				{
					Jpeg_Header_Addrs[13] = JPEG_IMG_FORMAT_420;		

					if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||
				   	   (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)
				  	  )
					{	
						// ­×§ï interval ¼Æ­È => jpeg image (width*height) / 16*8 (yuyv)
						/*
						¦]¬°¬O§Q¥ÎJPEG ©ñ¤j1.5­¿,©Ò¥HFIFO_LINE_LN­n1.5³B²z
						*/
						Jpeg_Header_Addrs[176] = (((((my_pAviEncVidPara->encode_width*FIFO_LINE_LN*3)>>1)>>8) & 0xFF00)>>8);	
						Jpeg_Header_Addrs[177] = (((((my_pAviEncVidPara->encode_width*FIFO_LINE_LN*3)>>1)>>8) & 0x00FF));			
					}
					else
					{
						// ­×§ï interval ¼Æ­È => jpeg image (width*height) / 16*8 (yuyv)
						Jpeg_Header_Addrs[176] = ((((gJpegCompressArgs.jpeg_img_width*FIFO_LINE_LN)>>8) & 0xFF00)>>8);	
						Jpeg_Header_Addrs[177] = ((((gJpegCompressArgs.jpeg_img_width*FIFO_LINE_LN)>>8) & 0x00FF));			
					}
				}	
				
				/*
					¦©±¼JPEG Header size + AVI Packer
				*/
				pJpegCompressArgs->jpeg_video_max_vlc_size = pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_video_out_jpeg_buf_idx].buffer_scope-656; 
			}

			*RetMaxVlcSize = pJpegCompressArgs->jpeg_video_max_vlc_size;
		}
		else
		{
			#if ENABLE_DYNAMIC_TUNING_JPEG_Q	
			jpeg_header_quantization_table_calculate(ENUM_JPEG_LUMINANCE_QTABLE, target_Y_Q_value, NULL);
			jpeg_header_quantization_table_calculate(ENUM_JPEG_CHROMINANCE_QTABLE, target_UV_Q_value, NULL);			
			#endif
			
			*RetMaxVlcSize = pJpegCompressArgs->jpeg_video_max_vlc_size;
		}

		return (pJpegCompressArgs->jpeg_video_next_fifo_addrs);
	}
	else // FIFO_SOURCE_FROM_DISPLAY
	{
		if(pJpegCompressArgs->jpeg_disp_new_jpeg_buffer == 0)
		{
			pJpegCompressArgs->jpeg_disp_new_jpeg_buffer = 1;
			
			RetJpegAddrs = jpeg_compress_free_addrs_get(pJpegCompressArgs);
			/*
				¸õ±¼JPEG Header ¦ì¸m
			*/
			pJpegCompressArgs->jpeg_disp_next_fifo_addrs = RetJpegAddrs;
			pJpegCompressArgs->jpeg_disp_out_jpeg_buf_idx = pJpegCompressArgs->jpeg_output_addrs_idx;

			if(RetJpegAddrs == DUMMY_BUFFER_ADDRS)
			{
				//DBG_PRINT("[BD]\r\n");
				pJpegCompressArgs->jpeg_disp_max_vlc_size = JPEG_DISP_OUT_BUFFER_SIZE;
			}
			else
			{
				#if ENABLE_DYNAMIC_TUNING_JPEG_Q		
					wifi_jpeg_quality_set(disp_target_Y_Q_value,disp_target_UV_Q_value);

					if(pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_disp_out_jpeg_buf_idx].jpeg_Y_Q_value != disp_target_Y_Q_value)
					{
						gp_memcpy((INT8S*)(RetJpegAddrs+0x27),(INT8S*)(wifi_jpeg_picture_header+0x27),64);
						pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_disp_out_jpeg_buf_idx].jpeg_Y_Q_value = disp_target_Y_Q_value;
					}
					// Change UV value
					if(pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_disp_out_jpeg_buf_idx].jpeg_UV_Q_value != disp_target_UV_Q_value)
					{
						gp_memcpy((INT8S*)(RetJpegAddrs+0x6C),(INT8S*)(wifi_jpeg_picture_header+0x6C),64);
						pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_disp_out_jpeg_buf_idx].jpeg_UV_Q_value = disp_target_UV_Q_value;
					}
				#endif

				/*
					Add Jpeg Header
				*/
				pJpegCompressArgs->jpeg_disp_next_fifo_addrs += 624;

				/*
					Find Jpeg Header
				*/
				Jpeg_Header_Addrs = (INT8U*)(RetJpegAddrs);

				Jpeg_Header_Addrs[7] = ((my_pAviEncVidPara->display_height & 0xFF00)>>8);		
				Jpeg_Header_Addrs[8] = (my_pAviEncVidPara->display_height & 0x00FF);
				
				Jpeg_Header_Addrs[9] = ((my_pAviEncVidPara->display_width & 0xFF00)>>8);		
				Jpeg_Header_Addrs[10] = (my_pAviEncVidPara->display_width & 0x00FF);

				Jpeg_Header_Addrs[13] = JPEG_IMG_FORMAT_422;		

				if(gDispCompressArgs.Disp_Fifo_Line_Cnt != my_pAviEncVidPara->display_height)
				{				
					// ­×§ï interval ¼Æ­È => jpeg image (width*height) / 16*8 (yuyv)
					Jpeg_Header_Addrs[176] = ((((my_pAviEncVidPara->display_width*gDispCompressArgs.Disp_Fifo_Line_Cnt)>>7) & 0xFF00)>>8);	
					Jpeg_Header_Addrs[177] = ((((my_pAviEncVidPara->display_width*gDispCompressArgs.Disp_Fifo_Line_Cnt)>>7) & 0x00FF));			
				}
				else
				{
					Jpeg_Header_Addrs[176] = 0;
					Jpeg_Header_Addrs[177] = 0;
				}
			
				/*
					¦©±¼JPEG Header size
				*/
				pJpegCompressArgs->jpeg_disp_max_vlc_size = pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_disp_out_jpeg_buf_idx].buffer_scope-624; 
			}

			*RetMaxVlcSize = pJpegCompressArgs->jpeg_disp_max_vlc_size;
		}
		else
		{
		
			#if ENABLE_DYNAMIC_TUNING_JPEG_Q		
			jpeg_header_quantization_table_calculate(ENUM_JPEG_LUMINANCE_QTABLE, disp_target_Y_Q_value, NULL);
			jpeg_header_quantization_table_calculate(ENUM_JPEG_CHROMINANCE_QTABLE, disp_target_UV_Q_value, NULL);			
			#endif

			*RetMaxVlcSize = pJpegCompressArgs->jpeg_disp_max_vlc_size;
		}

		return (pJpegCompressArgs->jpeg_disp_next_fifo_addrs);
	}
}
#endif

/****************************************************************************/
/*
 *	jpeg_compress_fifo_start
 */
#if DUAL_STREAM_FUNC_ENABLE	
void jpeg_compress_fifo_start(jpeg_comress_args_t* pJpegCompressArgs)
{
	INT32U jpeg_output_addrs;
	INT32U jpeg_size;
	
	jpeg_init();

	if(pJpegCompressArgs->jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
	{
		if(pJpegCompressArgs->jpeg_img_format == FIFO_FORMAT_422)
		{
			jpeg_yuv_sampling_mode_set(C_JPG_CTRL_YUV422);	
		}
		else
		{
			jpeg_yuv_sampling_mode_set(C_JPG_CTRL_YUV420 | C_JPG_CTRL_GP420);
		}

		// For 1080P / 1080FHD
		if(pJpegCompressArgs->jpeg_enable_scaler_up_func_flag)
		{
			jpeg_scaler_up_x1_5();
		}
		
		jpeg_image_size_set(pJpegCompressArgs->jpeg_img_width, pJpegCompressArgs->jpeg_img_height);

		jpeg_yuv_addr_set(pJpegCompressArgs->jpeg_input_addrs, 0, 0);	// Input addresses(32-byte alignment)
		jpeg_multi_yuv_input_init(pJpegCompressArgs->jpeg_input_size);
	}	
	else // FIFO_SOURCE_FROM_DISPLAY
	{
		jpeg_yuv_sampling_mode_set(C_JPG_CTRL_YUV422);	
		jpeg_image_size_set(my_pAviEncVidPara->display_width, gDispCompressArgs.Disp_Fifo_Line_Cnt);
		jpeg_yuv_addr_set(pJpegCompressArgs->jpeg_input_addrs, 0, 0);	// Input addresses(32-byte alignment)
		if(gDispCompressArgs.Disp_Fifo_Line_Cnt != my_pAviEncVidPara->display_height)
		{
			jpeg_multi_yuv_input_init(gDispCompressArgs.Disp_rec_fifo_data_size);
		}
	}

	if(pJpegCompressArgs->jpeg_send_to_target == JPEG_SEND_FOR_PHOTO) // Capture Photo	
	{
		jpeg_vlc_addr_set(pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_output_addrs_idx].buffer_addrs);	// Output address(16-byte alignment)
		jpeg_vlc_maximum_length_set(JPEG_VLC_BUFFER_SIZE);
	}
	else
	{
		if((pJpegCompressArgs->jpeg_send_to_target == JPEG_SEND_FOR_RECORD)||
			(pJpegCompressArgs->jpeg_send_to_target == JPEG_SEND_FOR_DISPLAY)
		)
		{			
			jpeg_output_addrs = jpeg_compress_FIFO_addrs_get(pJpegCompressArgs,&jpeg_size);
			jpeg_vlc_addr_set(jpeg_output_addrs);	// Output address(16-byte alignment)
			jpeg_vlc_maximum_length_set(jpeg_size);
		}
		else // For PC Cam
		{
			jpeg_output_addrs = jpeg_compress_free_addrs_get(pJpegCompressArgs);
			jpeg_vlc_addr_set(jpeg_output_addrs + 624);	// +624 jpeg header

			if (pJpegCompressArgs->jpeg_output_addrs_idx<JPEG_OUT_BUFFER_CNT)
			{
				jpeg_size = pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_output_addrs_idx].buffer_scope-624;
			}
			else
			{	// dummy address
				jpeg_vlc_addr_set(DUMMY_BUFFER_ADDRS);	// ¿ý¼v(32¶ô)¤Á¨ì UVC(4¶ô)¡A«Ü¥i¯à index >= JPEG_OUT_BUFFER_CNT¡A¦ý¬O jpeg_output_addrs ¤£¬O DUMMY
				jpeg_size = 0x800000; 		// 8MB  (¤@©w­nÀ£§¹¡A¤Ï¥¿¤]¤£¦ûÀW¼e)
			}
			jpeg_vlc_maximum_length_set(jpeg_size);
		}
	}

	jpeg_compression_start(pJpegCompressArgs->jpeg_status_notify);	
}
#else
void jpeg_compress_fifo_start(jpeg_comress_args_t* pJpegCompressArgs)
{
	INT32U jpeg_output_addrs;
	
	jpeg_init();

	if(pJpegCompressArgs->jpeg_img_format == FIFO_FORMAT_422)
	{
		jpeg_yuv_sampling_mode_set(C_JPG_CTRL_YUV422);	
	}
	else
	{
		jpeg_yuv_sampling_mode_set(C_JPG_CTRL_YUV420 | C_JPG_CTRL_GP420);
	}

	// For 1080P / 1080FHD
	if(pJpegCompressArgs->jpeg_enable_scaler_up_func_flag)
	{
		jpeg_scaler_up_x1_5();
	}
	
	jpeg_image_size_set(pJpegCompressArgs->jpeg_img_width, pJpegCompressArgs->jpeg_img_height);

	jpeg_yuv_addr_set(pJpegCompressArgs->jpeg_input_addrs, 0, 0);	// Input addresses(32-byte alignment)
	jpeg_multi_yuv_input_init(pJpegCompressArgs->jpeg_input_size);


	if(pJpegCompressArgs->jpeg_send_to_target == JPEG_SEND_FOR_PHOTO) // Capture Photo	
	{
		jpeg_vlc_addr_set(pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_output_addrs_idx].buffer_addrs);	// Output address(16-byte alignment)
		jpeg_vlc_maximum_length_set(JPEG_VLC_BUFFER_SIZE);
	}
	else
	{
		INT32U jpeg_size;
		
		if(pJpegCompressArgs->jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
		{			
			jpeg_output_addrs = jpeg_compress_free_addrs_get(pJpegCompressArgs);	
			
			//+++
			#if ENABLE_DYNAMIC_TUNING_JPEG_Q	
			if(jpeg_output_addrs != DUMMY_BUFFER_ADDRS)
			{
				// ¦h¥[16¬Oµ¹avipacker ¥Î¦bheader
				// Change Y value
				if(pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_output_addrs_idx].jpeg_Y_Q_value != target_Y_Q_value)
				{
					gp_memcpy((INT8S*)(jpeg_output_addrs+16+0x27),(INT8S*)(jpeg_picture_header+0x27),64);
					pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_output_addrs_idx].jpeg_Y_Q_value = target_Y_Q_value;
				}
				// Change UV value
				if(pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_output_addrs_idx].jpeg_UV_Q_value != target_UV_Q_value)
				{
					gp_memcpy((INT8S*)(jpeg_output_addrs+16+0x6C),(INT8S*)(jpeg_picture_header+0x6C),64);
					pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_output_addrs_idx].jpeg_UV_Q_value = target_UV_Q_value;
				}
			}
			#endif
			//---
			
			jpeg_vlc_addr_set(jpeg_output_addrs + 640);	// Output address(16-byte alignment)
		}
		else // For PC Cam
		{
			jpeg_output_addrs = jpeg_compress_free_addrs_get(pJpegCompressArgs);
			jpeg_vlc_addr_set(jpeg_output_addrs + 624);	// +624 jpeg header
		}

		if (pJpegCompressArgs->jpeg_output_addrs_idx<JPEG_OUT_BUFFER_CNT) 	{
			if(pJpegCompressArgs->jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
			{
				jpeg_size = pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_output_addrs_idx].buffer_scope-656;
			}
			else // For PC Cam
			{
				jpeg_size = pJpegCompressArgs->jpeg_output_addrs[pJpegCompressArgs->jpeg_output_addrs_idx].buffer_scope-624;
			}
		}
		else {	// dummy address
			jpeg_vlc_addr_set(DUMMY_BUFFER_ADDRS);	// ¿ý¼v(32¶ô)¤Á¨ì UVC(4¶ô)¡A«Ü¥i¯à index >= JPEG_OUT_BUFFER_CNT¡A¦ý¬O jpeg_output_addrs ¤£¬O DUMMY
			jpeg_size = 0x800000; 		// 8MB  (¤@©w­nÀ£§¹¡A¤Ï¥¿¤]¤£¦ûÀW¼e)
		}
		jpeg_vlc_maximum_length_set(jpeg_size);
	}

	jpeg_compression_start(pJpegCompressArgs->jpeg_status_notify);	
}
#endif

/****************************************************************************/
/*
 *	jpeg_compress_fifo_continue
 */
void jpeg_compress_fifo_continue(jpeg_comress_args_t* pJpegCompressArgs)
{
	jpeg_multi_yuv_input_restart(pJpegCompressArgs->jpeg_input_addrs, 0, 0, pJpegCompressArgs->jpeg_input_size);					
}

/****************************************************************************/
/*
 *	display_frame_ready_notify_callback
 */
#if DUAL_STREAM_FUNC_ENABLE
void display_frame_ready_notify_callback(INT32U fifoSrc,INT32U fifoMsg, INT32U fifoAddrs, INT32U fifoIdx)
{
	INT32S ret;

	if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT) 
	{         
        ret = jpeg_Q_put(fifoSrc,fifoAddrs,fifoMsg,fifoIdx); 
        if(ret == STATUS_OK) 
        { 
        	OSQPost(my_AVIEncodeApQ, (void*)MSG_JPEG_QUEUE_NOTIFY); 
        } 
        else 
        { 
            if(fifoAddrs != DUMMY_BUFFER_ADDRS) 
            { 
				ap_display_queue_put(display_isr_queue, fifoAddrs); 
            } 
        }                 
	} 
	else 
	{                 
		OSQPost(DisplayTaskQ, (void *)fifoAddrs); 
	} 
}
#else 
void display_frame_ready_notify_callback(INT32U display_buffer_addrs)
{
	OSQPost(DisplayTaskQ, (void *) display_buffer_addrs);
}
#endif

/****************************************************************************/
/*
 *	jpeg_quality_set: 
 */
void jpeg_quality_set(INT8U Y_quality_value,INT8U UV_quality_value)
{
	// Y
	jpeg_header_quantization_table_calculate(0, Y_quality_value, jpeg_picture_header + 0x27);
	// UV
	jpeg_header_quantization_table_calculate(1, UV_quality_value, jpeg_picture_header + 0x6C);
}

#if DUAL_STREAM_FUNC_ENABLE
/****************************************************************************/
/*
 *	wifi_jpeg_quality_set: 
 */
void wifi_jpeg_quality_set(INT8U Y_quality_value,INT8U UV_quality_value)
{
	// Y
	jpeg_header_quantization_table_calculate(0, Y_quality_value, wifi_jpeg_picture_header + 0x27);
	// UV
	jpeg_header_quantization_table_calculate(1, UV_quality_value, wifi_jpeg_picture_header + 0x6C);
}
#endif

/****************************************************************************/
/*
 *	jpeg_quality_set_photo: 
 */
void jpeg_quality_set_photo(INT8U Y_quality_value,INT8U UV_quality_value)
{
	// Y
	jpeg_header_quantization_table_calculate(0, Y_quality_value, jpeg_picture_header_photo + 0x27+6);
	// UV
	jpeg_header_quantization_table_calculate(1, UV_quality_value, jpeg_picture_header_photo + 0x6C+6);
}

/****************************************************************************/
/*
 *	avi_encode_start
 */
void avi_encode_start(INT8U videoRec_webCam)
{
	vid_rec_args_t vidRecArgs = {0};
	INT32U buffer_addrs;
	INT32U buffer_large_cnt, buffer_middle_cnt, buffer_small_cnt;
	INT32U i;

	#if ENABLE_DYNAMIC_TUNING_JPEG_Q
	cnt = 0;
	full_size_cnt = 0;
	jpeg_size_sum = 0;
	#endif

	/*
		³]¸m M14(JPEG) > M4(CONV422TO420)
		¦]¬°JPEG ·|¥Î¨ìSDRAM ¤£¯à¤ÓºC³B²z
		¤£µM·|¨SBUFFERÁÙ¦^¥hµ¹FIFO
	*/
#if TV_DET_ENABLE
	if(ap_display_get_device() == DISP_DEV_TV)
	{
		R_MEM_M2_BUS_PRIORITY = 0x0C;
		R_MEM_M3_BUS_PRIORITY = 0x40;
		R_MEM_M4_BUS_PRIORITY = 0x10;
		R_MEM_M11_BUS_PRIORITY = 0x40;
		R_MEM_M14_BUS_PRIORITY = 0x10;
	}
	else
#endif	
	{
		R_MEM_M2_BUS_PRIORITY = 0x03;
		R_MEM_M3_BUS_PRIORITY = 0x04;
		R_MEM_M4_BUS_PRIORITY = 0x18;
		R_MEM_M11_BUS_PRIORITY = 0x18;
		R_MEM_M14_BUS_PRIORITY = 0x08;
	}

	fifo_Q_clear();
	#if DUAL_STREAM_FUNC_ENABLE
		if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
		{
			jpeg_Q_clear();
		}
	#endif

	if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||
	   (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)
	  )
	{		
		gJpegCompressArgs.jpeg_img_width = my_pAviEncVidPara->sensor_width;
		gJpegCompressArgs.jpeg_img_height = 736;
		gJpegCompressArgs.jpeg_enable_scaler_up_func_flag = 1;
		USE_SDRAM_SUPPORT_FIFO_CNT = 3;
		FIFO_LINE_LN = 32;
	}
	else
	{
		gJpegCompressArgs.jpeg_img_width = my_pAviEncVidPara->encode_width;
		gJpegCompressArgs.jpeg_img_height = my_pAviEncVidPara->encode_height;
		gJpegCompressArgs.jpeg_enable_scaler_up_func_flag = 0;
		USE_SDRAM_SUPPORT_FIFO_CNT = 0;
		FIFO_LINE_LN = 16;
	}

	gJpegCompressArgs.jpeg_input_empty_count_max = (my_pAviEncVidPara->sensor_height/FIFO_LINE_LN);
	if((my_pAviEncVidPara->sensor_height % FIFO_LINE_LN) != 0)
	{
		gJpegCompressArgs.jpeg_input_empty_count_max++;
	}

	vidRecArgs.vid_rec_fifo_path = FIFO_PATH_TO_VIDEO_RECORD;
	vidRecArgs.vid_rec_fifo_total_count = FIFO_Q_MAX_CNT;
	vidRecArgs.vid_rec_fifo_line_len = FIFO_LINE_LN;
	vidRecArgs.vid_rec_fifo_output_format = FIFO_USE_FORMAT;

	if(vidRecArgs.vid_rec_fifo_output_format == FIFO_FORMAT_422)
	{
		vidRecArgs.vid_rec_fifo_data_size = (my_pAviEncVidPara->sensor_width*vidRecArgs.vid_rec_fifo_line_len*2);
	}
	else
	{
		vidRecArgs.vid_rec_fifo_data_size = (my_pAviEncVidPara->sensor_width*vidRecArgs.vid_rec_fifo_line_len*3)>>1;
	}
	
	vidRecArgs.fifo_ready_notify = &fifo_ready_notify_encode_callback;
	vidRecArgs.fifo_buffer_addrs_get = &fifo_Q_get;
	vidRecArgs.fifo_buffer_addrs_put = &fifo_Q_put;

	gJpegCompressArgs.jpeg_img_format = vidRecArgs.vid_rec_fifo_output_format;
	gJpegCompressArgs.jpeg_input_size = vidRecArgs.vid_rec_fifo_data_size;

	if(videoRec_webCam)
	{
		gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_PC_CAM;
		buffer_large_cnt = JPEG_UVC_BUFFER_LARGE_CNT;
		buffer_middle_cnt = JPEG_UVC_BUFFER_MIDDLE_CNT;
		buffer_small_cnt = JPEG_UVC_BUFFER_SMALL_CNT;
		JPEG_OUT_BUFFER_CNT = buffer_large_cnt+buffer_middle_cnt+buffer_small_cnt;
	}
	else
	{
		gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_RECORD;
 		if ( (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)) )
		{
			buffer_large_cnt = jpeg_out_1080p_large_cnt;
			buffer_middle_cnt = jpeg_out_1080p_middle_cnt;
			buffer_small_cnt = jpeg_out_1080p_small_cnt;
		
 		}
		else
		{
			buffer_large_cnt = jpeg_out_buffer_large_cnt;
			buffer_middle_cnt = jpeg_out_buffer_middle_cnt;
			buffer_small_cnt = jpeg_out_buffer_small_cnt;
		}
		#if DUAL_STREAM_FUNC_ENABLE
			if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
			{
				JPEG_OUT_BUFFER_CNT = buffer_large_cnt+buffer_middle_cnt+buffer_small_cnt;
			}
		#endif
	}

	/*
		JPEG Header + AVI Packer
		624+16+16= 656
	*/	
	if (buffer_large_cnt>buffer_middle_cnt)
	{
		if (buffer_large_cnt>buffer_small_cnt)
		{
			max_VLC_size = jpeg_out_buffer_large_size-656;
		}
		else
		{
			max_VLC_size = jpeg_out_buffer_small_size-656;
		}
	}
	else
	{
		if (buffer_middle_cnt>buffer_small_cnt)
		{
			max_VLC_size = jpeg_out_buffer_middle_size-656;
		}
		else
		{
			max_VLC_size = jpeg_out_buffer_small_size-656;
		}
	}

	#if DUAL_STREAM_FUNC_ENABLE
		if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
		{
			gJpegCompressArgs.jpeg_img_height = FIFO_LINE_LN;
		}
	#endif

	//+++ Jpeg Header
	jpeg_picture_header[7] = ((my_pAviEncVidPara->encode_height & 0xFF00)>>8);		
	jpeg_picture_header[8] = (my_pAviEncVidPara->encode_height & 0x00FF);
	
	jpeg_picture_header[9] = ((my_pAviEncVidPara->encode_width & 0xFF00)>>8);		
	jpeg_picture_header[10] = (my_pAviEncVidPara->encode_width & 0x00FF);
	if(gJpegCompressArgs.jpeg_img_format == FIFO_FORMAT_422)
	{	
		jpeg_picture_header[13] = JPEG_IMG_FORMAT_422;	
		#if DUAL_STREAM_FUNC_ENABLE
			if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
			{
				if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||
			   	   (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)
			  	  )
				{	
					// ­×§ï interval ¼Æ­È => jpeg image (width*height) / 16*8 (yuyv)
					jpeg_picture_header[176] = (((((my_pAviEncVidPara->encode_width*FIFO_LINE_LN*3)>>1)>>7) & 0xFF00)>>8);	
					jpeg_picture_header[177] = (((((my_pAviEncVidPara->encode_width*FIFO_LINE_LN*3)>>1)>>7) & 0x00FF));			
				}
				else
				{
					// ­×§ï interval ¼Æ­È => jpeg image (width*height) / 16*8 (yuyv)
					jpeg_picture_header[176] = ((((gJpegCompressArgs.jpeg_img_width*FIFO_LINE_LN)>>7) & 0xFF00)>>8);	
					jpeg_picture_header[177] = ((((gJpegCompressArgs.jpeg_img_width*FIFO_LINE_LN)>>7) & 0x00FF));			
				}
			}		
		#endif
	}
	else
	{
		jpeg_picture_header[13] = JPEG_IMG_FORMAT_420;		
		#if DUAL_STREAM_FUNC_ENABLE
			if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
			{
				if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||
			   	   (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)
			  	  )
				{	
					// ­×§ï interval ¼Æ­È => jpeg image (width*height) / 16*8 (yuyv)
					jpeg_picture_header[176] = (((((my_pAviEncVidPara->encode_width*FIFO_LINE_LN*3)>>1)>>8) & 0xFF00)>>8);	
					jpeg_picture_header[177] = (((((my_pAviEncVidPara->encode_width*FIFO_LINE_LN*3)>>1)>>8) & 0x00FF));			
				}
				else
				{
					// ­×§ï interval ¼Æ­È => jpeg image (width*height) / 16*8 (yuyv)
					jpeg_picture_header[176] = ((((gJpegCompressArgs.jpeg_img_width*FIFO_LINE_LN)>>8) & 0xFF00)>>8);	
					jpeg_picture_header[177] = ((((gJpegCompressArgs.jpeg_img_width*FIFO_LINE_LN)>>8) & 0x00FF));			
				}
			}
		#endif
	}

	#if DUAL_STREAM_FUNC_ENABLE
	if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_PC_CAM)
	{
		jpeg_picture_header[176] = 0;
		jpeg_picture_header[177] = 0;
	}
	#endif		

	#if DUAL_STREAM_FUNC_ENABLE		
		#if ENABLE_DYNAMIC_TUNING_JPEG_Q
		jpeg_quality_set(target_Y_Q_value,target_UV_Q_value);
		#endif
	#endif
	
	{
		INT32U use_buffer_size = 0;
		INT32U alloc_buffer_size = 	(buffer_large_cnt*jpeg_out_buffer_large_size)
									+(buffer_middle_cnt*jpeg_out_buffer_middle_size)
									+(buffer_small_cnt*jpeg_out_buffer_small_size);
		buffer_addrs = (INT32U)gp_malloc_align(alloc_buffer_size, 32);

		#if DUAL_STREAM_FUNC_ENABLE
		if(buffer_addrs!=NULL)
		#else
		if ( (buffer_addrs!=NULL)&&(JPEG_OUT_BUFFER_CNT_MAX>=JPEG_OUT_BUFFER_CNT) )
		#endif
		{
			int begin,end;

			begin = 0;
			end = buffer_large_cnt;	
			for(i=begin; i<end; i++)
			{	
				INT32U buffer_size = jpeg_out_buffer_large_size;
				gJpegCompressArgs.jpeg_output_addrs[i].is_used = 0;
				gJpegCompressArgs.jpeg_output_addrs[i].buffer_scope= buffer_size;
				gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs = buffer_addrs+use_buffer_size;
				#if ENABLE_DYNAMIC_TUNING_JPEG_Q
				gJpegCompressArgs.jpeg_output_addrs[i].jpeg_Y_Q_value = target_Y_Q_value;
				gJpegCompressArgs.jpeg_output_addrs[i].jpeg_UV_Q_value = target_UV_Q_value;
				#endif
				
				use_buffer_size += buffer_size;

				if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
				{
					gp_memcpy((INT8S*)gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs + 16, (INT8S*) &jpeg_picture_header[0], 624);
				}
				else // To USB
				{
					gp_memcpy((INT8S*)gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs, (INT8S*) &jpeg_picture_header[0], 624);
				}	
			}
			begin = buffer_large_cnt;
			end = buffer_large_cnt+buffer_middle_cnt;
			for(i=begin; i<end; i++)
			{
				INT32U buffer_size = jpeg_out_buffer_middle_size;
				gJpegCompressArgs.jpeg_output_addrs[i].is_used = 0;
				gJpegCompressArgs.jpeg_output_addrs[i].buffer_scope= buffer_size;		
				gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs = buffer_addrs+use_buffer_size;
				#if ENABLE_DYNAMIC_TUNING_JPEG_Q
				gJpegCompressArgs.jpeg_output_addrs[i].jpeg_Y_Q_value = target_Y_Q_value;
				gJpegCompressArgs.jpeg_output_addrs[i].jpeg_UV_Q_value = target_UV_Q_value;
				#endif
				
				use_buffer_size += buffer_size;

				if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
				{
					gp_memcpy((INT8S*)gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs + 16, (INT8S*) &jpeg_picture_header[0], 624);
				}
				else // To USB
				{
					gp_memcpy((INT8S*)gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs, (INT8S*) &jpeg_picture_header[0], 624);
				}
			}
			begin = buffer_large_cnt+buffer_middle_cnt;
			end = buffer_large_cnt+buffer_middle_cnt+buffer_small_cnt;
			for(i=begin; i<end; i++)
			{	
				INT32U buffer_size = jpeg_out_buffer_small_size;
				gJpegCompressArgs.jpeg_output_addrs[i].is_used = 0;
				gJpegCompressArgs.jpeg_output_addrs[i].buffer_scope= buffer_size;		
				gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs = buffer_addrs+use_buffer_size;
				#if ENABLE_DYNAMIC_TUNING_JPEG_Q
				gJpegCompressArgs.jpeg_output_addrs[i].jpeg_Y_Q_value = target_Y_Q_value;
				gJpegCompressArgs.jpeg_output_addrs[i].jpeg_UV_Q_value = target_UV_Q_value;
				#endif
				use_buffer_size += buffer_size;

				if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
				{
					gp_memcpy((INT8S*)gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs + 16, (INT8S*) &jpeg_picture_header[0], 624);
				}
				else // To USB
				{
					gp_memcpy((INT8S*)gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs, (INT8S*) &jpeg_picture_header[0], 624);
				}	
			}
		}
		else
		{
			DBG_PRINT("Video Record Allocate Memory Error !!\r\n");
			for(i=0; i<JPEG_OUT_BUFFER_CNT; i++)
			{
				gJpegCompressArgs.jpeg_output_addrs[i].is_used = 1;
				gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs	= DUMMY_BUFFER_ADDRS;
			}
		}
	}
	
	time_stamp_buffer_size = 0;

	#if DUAL_STREAM_FUNC_ENABLE
		if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
		{
			gJpegCompressArgs.jpeg_video_new_jpeg_buffer = 0;
			gJpegCompressArgs.jpeg_video_Header_Restart_Interval_Count = 0;
		}
		else
	#endif
		{
			gJpegCompressArgs.jpeg_video_new_jpeg_buffer = 0;
			gJpegCompressArgs.jpeg_video_Header_Restart_Interval_Count = 0;

			gJpegCompressArgs.jpeg_status_notify = &jpeg_status_notify;
			gJpegCompressArgs.jpeg_isr_status_flag = 0;
			gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_IDLE;		
		}

	gJpegCompressArgs.jpeg_output_addrs_idx = 0;	
	gJpegCompressArgs.jpeg_input_empty_count = 0;
	gJpegCompressArgs.jpeg_input_addrs = 0;	// add by josejphhsieh@20150422
	
	//---

	buffer_addrs = 0xF8000000;	// iRAM address
	USE_SDRAM_SUPPORT_FIFO_ADDRS_START = 0;
	USE_SDRAM_SUPPORT_FIFO_ADDRS_END = 0;

	if(USE_SDRAM_SUPPORT_FIFO_CNT > 0)
	{
		USE_SDRAM_SUPPORT_FIFO_ADDRS_START = (INT32U)gp_malloc_align((USE_SDRAM_SUPPORT_FIFO_CNT*(vidRecArgs.vid_rec_fifo_data_size+64)), 32);
		USE_SDRAM_SUPPORT_FIFO_ADDRS_END = USE_SDRAM_SUPPORT_FIFO_ADDRS_START+(USE_SDRAM_SUPPORT_FIFO_CNT*vidRecArgs.vid_rec_fifo_data_size);
	}

	for(i=0; i<vidRecArgs.vid_rec_fifo_total_count; i++)
	{	
		if(USE_SDRAM_SUPPORT_FIFO_CNT > 0)
		{
			#if 0
			if(i & 0x01)
			{
				fifo_Q_put(USE_SDRAM_SUPPORT_FIFO_ADDRS_START+((i>>1)*vidRecArgs.vid_rec_fifo_data_size));
			}
			else
			{
				fifo_Q_put(buffer_addrs+((i>>1)*vidRecArgs.vid_rec_fifo_data_size));
			}
			#else
			if(i >= (FIFO_Q_MAX_CNT-USE_SDRAM_SUPPORT_FIFO_CNT))
			{
				fifo_Q_put(USE_SDRAM_SUPPORT_FIFO_ADDRS_START+((i-(FIFO_Q_MAX_CNT-USE_SDRAM_SUPPORT_FIFO_CNT))*(vidRecArgs.vid_rec_fifo_data_size+64)));
			}
			else
			{
				fifo_Q_put(buffer_addrs+(i*vidRecArgs.vid_rec_fifo_data_size));
			}			
			#endif
		}
		else
		{
			fifo_Q_put(buffer_addrs+(i*vidRecArgs.vid_rec_fifo_data_size));
		}
	}

	#if (Enable_Lane_Departure_Warning_System)
	if(LDWS_Enable_Flag)
	{
		LDWS_MSG_Send_flag = 0;
		LDWS_buf_addrs_idx = 0;
		LDWS_Key_On_flag = 0;
		LDWPar.LDW_keyFlg = 0;
		LDWPar.LDWsAlarmFlg = 0;
		
		if(my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD)
		{
			LDWS_start_fifo_idx = 13; 
			LDWS_end_fifo_idx = 19;
		}
		else // 720P
		{
			LDWS_start_fifo_idx = 27; 
			LDWS_end_fifo_idx = 39;
		}
		LDWS_buf_malloc_cnt = LDWS_NEED_BUF_SIZE/FIFO_LINE_LN;

		if(vidRecArgs.vid_rec_fifo_output_format == FIFO_FORMAT_422)
		{
			LDWS_buf_size = ((my_pAviEncVidPara->sensor_width*FIFO_LINE_LN)<<1);
		}
		else
		{
			LDWS_buf_size = ((my_pAviEncVidPara->sensor_width*FIFO_LINE_LN*3)>>1);
		}
		LDWS_buf_addrs = (INT32U)gp_malloc_align((LDWS_buf_size*LDWS_buf_malloc_cnt), 32);
	}
	#endif
	
	video_recording_start(&vidRecArgs);
} 

/****************************************************************************/
/*
 *	avi_encode_stop
 */
void avi_enc_buffer_free(void)
{
	#if DUAL_STREAM_FUNC_ENABLE
	if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
	#endif
	{
		gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_NOTHING;
	}
	
	time_stamp_buffer_size = 0;

	if(gJpegCompressArgs.jpeg_output_addrs[0].buffer_addrs != 0)
	{
		gp_free((void *) gJpegCompressArgs.jpeg_output_addrs[0].buffer_addrs);
		gJpegCompressArgs.jpeg_output_addrs[0].buffer_addrs = 0;
	}

	if(USE_SDRAM_SUPPORT_FIFO_ADDRS_START != 0)
	{
		gp_free((void *)USE_SDRAM_SUPPORT_FIFO_ADDRS_START);
		USE_SDRAM_SUPPORT_FIFO_ADDRS_START = 0;
	}

	#if (Enable_Lane_Departure_Warning_System)
	if(LDWS_Enable_Flag)
	{
		if(LDWS_buf_addrs != 0)
		{
			gp_free((void *)LDWS_buf_addrs);
			LDWS_buf_addrs = 0;
		}
	}
	#endif	
	
}

/****************************************************************************/
/*
 *	avi_capture_start
 */
void avi_capture_start(void)
{
	vid_rec_args_t vidRecArgs = {0};
	INT32U buffer_size;
	INT32U buffer_addrs;
	INT32U i;

	fifo_Q_clear();

	/*
		§Q¥ÎFifo(Sensor W*H*2 Frame Mode])
	*/
	vidRecArgs.vid_rec_fifo_path = FIFO_PATH_TO_CAPTURE_PHOTO;
	vidRecArgs.vid_rec_fifo_total_count = 1;
	vidRecArgs.vid_rec_fifo_line_len = my_pAviEncVidPara->sensor_height;

	#if 1 //TV_DET_ENABLE
	vidRecArgs.vid_rec_fifo_output_format = FIFO_FORMAT_422;
	#else
	vidRecArgs.vid_rec_fifo_output_format = FIFO_USE_FORMAT;
	#endif

	if(vidRecArgs.vid_rec_fifo_output_format == FIFO_FORMAT_422)
	{
		vidRecArgs.vid_rec_fifo_data_size = (my_pAviEncVidPara->sensor_width*vidRecArgs.vid_rec_fifo_line_len*2);
	}
	else
	{
		vidRecArgs.vid_rec_fifo_data_size = (my_pAviEncVidPara->sensor_width*vidRecArgs.vid_rec_fifo_line_len*3)>>1;
	}
	
	vidRecArgs.fifo_ready_notify = &fifo_ready_notify_capture_callback;
	vidRecArgs.fifo_buffer_addrs_get = &fifo_Q_get;

	buffer_size = vidRecArgs.vid_rec_fifo_data_size; // 422: W*H*2 420:W*H*1.5
	buffer_addrs = (INT32U)gp_malloc_align(buffer_size, 32);

	// Sensor Frame
	my_pAviEncVidPara->sensor_frame_addrs = buffer_addrs;
	for(i=0; i<vidRecArgs.vid_rec_fifo_total_count; i++)
	{	
		fifo_Q_put(buffer_addrs+(i*vidRecArgs.vid_rec_fifo_data_size));
	}
	
	video_recording_start(&vidRecArgs);	
}

/****************************************************************************/
/*
 *	avi_scale_up_start
 */
extern void take_a_picture_init(INT8U* picture_header_addr);

void avi_scale_up_start(void)
{
	scaler_args_t scalerArgs = {0};
	INT32U buffer_size;
	INT32U buffer_addrs;
 	INT8U photo_size_setting;
	INT8U photo_quality_setting;
	INT32U i;

	Jpeg_Vlc_Total_Size = 0;

	jpeg_Q_clear();

	scalerArgs.scaler_in_width = my_pAviEncVidPara->sensor_width;
	scalerArgs.scaler_in_height = my_pAviEncVidPara->sensor_height; 

	//+++ Photo Size
	photo_size_setting = ap_state_config_pic_size_get();
	if(photo_size_setting == 0)
	{ 	//12M
		scalerArgs.scaler_out_width = 4032;
		scalerArgs.scaler_out_height = 3024;
	} 
	else if(photo_size_setting == 1)
	{ 	//10M
		scalerArgs.scaler_out_width = 3648;
		scalerArgs.scaler_out_height = 2736;
	}
	else if(photo_size_setting == 2)
	{ 	//8M
		scalerArgs.scaler_out_width = 3264;
		scalerArgs.scaler_out_height = 2448;
	}
	else if(photo_size_setting == 3)
	{ 	//5M
		scalerArgs.scaler_out_width = 2592;
		scalerArgs.scaler_out_height = 1944;
	}
	else if(photo_size_setting == 4)
	{ 	//3M
		scalerArgs.scaler_out_width = 2048;
		scalerArgs.scaler_out_height = 1536;
	}
	else if(photo_size_setting == 5)
	{ 	//2M
		scalerArgs.scaler_out_width = 1920;
		scalerArgs.scaler_out_height = 1080;
	}
	else if(photo_size_setting == 6)
	{ 	//1.3M
		scalerArgs.scaler_out_width = 1280;
		scalerArgs.scaler_out_height = 960;
	}
	else
	{ 	//1M
		scalerArgs.scaler_out_width = 640;
		scalerArgs.scaler_out_height = 480;
	}

	/*
		0	¸û¨Î
		1	¤@¯ë
		2	¸gÀÙ
	*/

	photo_quality_setting = ap_state_config_quality_get();
	if(ap_state_handling_storage_id_get() == NO_STORAGE) {
		if((photo_size_setting == 0) || (photo_size_setting == 1)) { //12M & 10M pixels
			scalerArgs.scaler_out_width = 3648;
			scalerArgs.scaler_out_height = 2736;

			jpeg_quality_set_photo(30, 30);
		} else if(photo_size_setting == 2) { //8M pixels
			if(photo_quality_setting == 0)
			{
				jpeg_quality_set_photo(60, 60);
			}
			else if(photo_quality_setting == 1)
			{
				jpeg_quality_set_photo(50, 50);
			}
			else
			{
				jpeg_quality_set_photo(40, 40);
			}
		} else {
			if(photo_quality_setting == 0)
			{
				jpeg_quality_set_photo(AVI_Q_VALUE_CAPTURE_NORNAL, AVI_Q_VALUE_CAPTURE_NORNAL);
			}
			else if(photo_quality_setting == 1)
			{
				jpeg_quality_set_photo(AVI_Q_VALUE_CAPTURE_LOW, AVI_Q_VALUE_CAPTURE_LOW);
			}
			else
			{
				jpeg_quality_set_photo(AVI_Q_VALUE_CAPTURE_LOW-10, AVI_Q_VALUE_CAPTURE_LOW-10);
			}
		}
	} else {
		if(photo_quality_setting == 0)
		{
			jpeg_quality_set_photo(AVI_Q_VALUE_CAPTURE_BEST,AVI_Q_VALUE_CAPTURE_BEST);
		}
		else if(photo_quality_setting == 1)
		{
			jpeg_quality_set_photo(AVI_Q_VALUE_CAPTURE_NORNAL,AVI_Q_VALUE_CAPTURE_NORNAL);
		}
		else
		{
			jpeg_quality_set_photo(AVI_Q_VALUE_CAPTURE_LOW,AVI_Q_VALUE_CAPTURE_LOW);
		}
	}

	#if 1 //TV_DET_ENABLE
	scalerArgs.scaler_input_format = FIFO_FORMAT_422;
	#else
	scalerArgs.scaler_input_format = FIFO_USE_FORMAT;
	#endif

	scalerArgs.scaler_in_buffer_addrs = my_pAviEncVidPara->sensor_frame_addrs;
	buffer_size = scalerArgs.scaler_out_width*SCALER_CTRL_OUT_FIFO_16LINE*2; 
	buffer_addrs = (INT32U)gp_malloc_align(buffer_size*2, 32); // Scaler A/B Output Buffer
	if(buffer_addrs == NULL)
	{
		DBG_PRINT("[Warning] Scale up is NULL \r\n");
	}

	// Scaler A/B Output Buffer
	scalerArgs.scaler_out_buffer_addrs = buffer_addrs;
	scalerArgs.scaler_ready_notify = &scaler_ready_notify_encode_callback;

	//+++ Jpeg Setting
	gJpegCompressArgs.jpeg_file_handle = pAviEncPara->AviPackerCur->file_handle;

	gJpegCompressArgs.jpeg_img_width = scalerArgs.scaler_out_width;
	gJpegCompressArgs.jpeg_img_height = scalerArgs.scaler_out_height;
	gJpegCompressArgs.jpeg_img_format = FIFO_FORMAT_422;
	gJpegCompressArgs.jpeg_input_size = buffer_size;

	// Jpeg output buffer
	buffer_addrs = (INT32U)gp_malloc_align((JPEG_VLC_BUFFER_SIZE*JPEG_VLC_BUFFER_CNT), 32);
	if(buffer_addrs == NULL)
	{
		DBG_PRINT("[Warning] JPEG out is NULL \r\n");
	}

	for(i=0; i<JPEG_VLC_BUFFER_CNT; i++)
	{
		gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs = buffer_addrs+(i*JPEG_VLC_BUFFER_SIZE);
		gJpegCompressArgs.jpeg_output_addrs[i].is_used = 0;
	}
 		
	gJpegCompressArgs.jpeg_status_notify = &jpeg_status_notify;
	gJpegCompressArgs.jpeg_output_addrs_idx = 0;
	gJpegCompressArgs.jpeg_isr_status_flag = 0;
	gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_IDLE;		
	gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_PHOTO;
	gJpegCompressArgs.jpeg_skip_this_one_flag = 0;
	gJpegCompressArgs.jpeg_enable_scaler_up_func_flag = 0;
	
	// Write JPEG file header
	jpeg_picture_header_photo[7] = ((gJpegCompressArgs.jpeg_img_height & 0xFF00)>>8);		
	jpeg_picture_header_photo[8] = (gJpegCompressArgs.jpeg_img_height & 0x00FF);
	
	jpeg_picture_header_photo[9] = ((gJpegCompressArgs.jpeg_img_width & 0xFF00)>>8);		
	jpeg_picture_header_photo[10] = (gJpegCompressArgs.jpeg_img_width & 0x00FF);
	if(gJpegCompressArgs.jpeg_img_format == FIFO_FORMAT_422)
	{	
		jpeg_picture_header_photo[13] = JPEG_IMG_FORMAT_422;		
	}
	else
	{
		jpeg_picture_header_photo[13] = JPEG_IMG_FORMAT_420;		
	}
	take_a_picture_init(jpeg_picture_header_photo);
	write(gJpegCompressArgs.jpeg_file_handle, (INT32U) jpeg_picture_header_photo, jpeg_header_photo_lenth);
	//---
	
	do_scale_up_start(&scalerArgs);
}

/****************************************************************************/
/*
 *	avi_scale_up_stop
 */
void avi_scale_up_stop(void)
{
	gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_NOTHING;

	// Free jpeg output buffer
	if(gJpegCompressArgs.jpeg_output_addrs[0].buffer_addrs != 0)
	{
		gp_free((void *)gJpegCompressArgs.jpeg_output_addrs[0].buffer_addrs);
		gJpegCompressArgs.jpeg_output_addrs[0].buffer_addrs = 0;
	}

	// Free senosr frame buffer
	if(my_pAviEncVidPara->sensor_frame_addrs != 0)
	{
		gp_free((void *)my_pAviEncVidPara->sensor_frame_addrs);
		my_pAviEncVidPara->sensor_frame_addrs = 0;
	}

	// Free Scaler A/B Output Buffer
	do_scale_up_stop();
}

/****************************************************************************/
/*
 * 	my_video_encode_sensor_start
 */
INT32S my_video_encode_sensor_start(void)
{
	preview_args_t previewArgs = {0};
	INT32S  nRet;

	#if DUAL_STREAM_FUNC_ENABLE
	if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
	{
		jpeg_disp_out_buffer_cnt = 0;
	}

	gDispCompressArgs.Disp_Skip_WIFI_Jpeg_Flag = 0;
	#endif


	previewArgs.clip_width = my_pAviEncVidPara->clip_width;
	previewArgs.clip_height = my_pAviEncVidPara->clip_height;
	previewArgs.sensor_width = my_pAviEncVidPara->sensor_width;
	previewArgs.sensor_height = my_pAviEncVidPara->sensor_height;
	previewArgs.display_width = my_pAviEncVidPara->display_width;
	previewArgs.display_height = my_pAviEncVidPara->display_height;
	previewArgs.sensor_do_init = my_pAviEncVidPara->sensor_do_init;
	previewArgs.display_buffer_addrs = display_buffer_pointer_get_func();
	previewArgs.run_ap_mode = my_pAviEncVidPara->enter_ap_mode;
	previewArgs.display_frame_ready_notify = &display_frame_ready_notify_callback;
	previewArgs.display_buffer_pointer_get = &display_buffer_pointer_get_func;
	previewArgs.display_buffer_pointer_put = &ap_display_queue_put;

	nRet = video_preview_start(&previewArgs);
	return nRet;
}

#if DUAL_STREAM_FUNC_ENABLE
/****************************************************************************/
/*
 *	Wifi_Preview_Start
 */
INT32S Wifi_Preview_Start(void)
{
	INT32U buffer_addrs;
	INT32U buffer_size;
	INT32U buffer_large_cnt, buffer_middle_cnt, buffer_small_cnt;
	INT32U i;

	#if PRINTF_WIFI_SPEED_ENABLE
	Wifi_Jpeg_Count = 0;
	Wifi_Jpeg_Data_Byte = 0;
	Wifi_Jpeg_Start_Time = OSTimeGet();
	#endif
	
	
	disp_current_Y_Q_value = 50;
	disp_current_UV_Q_value = 50;
	disp_target_Y_Q_value = 50;
	disp_target_UV_Q_value = 50;

	#if ENABLE_DYNAMIC_TUNING_JPEG_Q
	disp_cnt = 0;
	disp_full_size_cnt = 0;
	disp_jpeg_size_sum = 0;
	wifi_jpeg_quality_set(disp_target_Y_Q_value,disp_target_UV_Q_value);
	#endif
	
	jpeg_Q_clear();

	if(my_pAviEncVidPara->display_width== 640)
	{
		JPEG_DISP_OUT_BUFFER_SIZE = JPEG_DISP_OUT_BUFFER_SIZE_640_360;
		DBG_PRINT("Wi-Fi APP 640x360\r\n");
	}
	else
	{
		JPEG_DISP_OUT_BUFFER_SIZE = JPEG_DISP_OUT_BUFFER_SIZE_480_272;
		DBG_PRINT("Wi-Fi APP 480x272\r\n");		
	}

	gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_IDLE;		
	gJpegCompressArgs.jpeg_status_notify = &jpeg_status_notify;
	gJpegCompressArgs.jpeg_isr_status_flag = 0;
	
	gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_DISPLAY;
	gJpegCompressArgs.jpeg_disp_next_fifo_addrs = DUMMY_BUFFER_ADDRS;
	gJpegCompressArgs.jpeg_disp_Header_Restart_Interval_Count = 0;
	gJpegCompressArgs.jpeg_disp_input_empty_count = 0;
	gJpegCompressArgs.jpeg_disp_encode_end_flag = 0;
	gDispCompressArgs.Disp_Frame_Buff_Addrs = 0;
	
	gDispCompressArgs.Disp_Fifo_Line_Cnt = 16;

	gJpegCompressArgs.jpeg_disp_input_empty_max_count = (my_pAviEncVidPara->display_height/gDispCompressArgs.Disp_Fifo_Line_Cnt);
	if((my_pAviEncVidPara->display_height % gDispCompressArgs.Disp_Fifo_Line_Cnt ) != 0)
	{
		gJpegCompressArgs.jpeg_disp_input_empty_max_count++;
	}

	gDispCompressArgs.Disp_rec_fifo_data_size = (my_pAviEncVidPara->display_width*gDispCompressArgs.Disp_Fifo_Line_Cnt*2);

	jpeg_disp_out_buffer_cnt = JPEG_DISP_OUT_COUNT;
	buffer_size = JPEG_DISP_OUT_BUFFER_SIZE;

	gDispCompressArgs.Disp_Max_Vlc_Size = JPEG_DISP_OUT_BUFFER_SIZE;

	buffer_addrs = (INT32U)gp_malloc_align((jpeg_disp_out_buffer_cnt*buffer_size), 32);
	if(buffer_addrs == NULL)
	{
		DBG_PRINT("Display Buffer is Null!\r\n");
	}

		if ( (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)) )
	{
		buffer_large_cnt = jpeg_out_1080p_large_cnt;
		buffer_middle_cnt = jpeg_out_1080p_middle_cnt;
		buffer_small_cnt = jpeg_out_1080p_small_cnt;
	
		}
	else
	{
		buffer_large_cnt = jpeg_out_buffer_large_cnt;
		buffer_middle_cnt = jpeg_out_buffer_middle_cnt;
		buffer_small_cnt = jpeg_out_buffer_small_cnt;
	}

	JPEG_OUT_BUFFER_CNT = buffer_large_cnt+buffer_middle_cnt+buffer_small_cnt;
	
	for(i=JPEG_OUT_BUFFER_CNT; i<(jpeg_disp_out_buffer_cnt+JPEG_OUT_BUFFER_CNT); i++)
	{	
		gJpegCompressArgs.jpeg_output_addrs[i].is_used = 0;
		gJpegCompressArgs.jpeg_output_addrs[i].buffer_scope= buffer_size;
		gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs = buffer_addrs+((i-JPEG_OUT_BUFFER_CNT)*buffer_size);
		#if ENABLE_DYNAMIC_TUNING_JPEG_Q
		gJpegCompressArgs.jpeg_output_addrs[i].jpeg_Y_Q_value = disp_target_Y_Q_value;
		gJpegCompressArgs.jpeg_output_addrs[i].jpeg_UV_Q_value = disp_target_UV_Q_value;
		#endif
		
		gp_memcpy((INT8S*)gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs, (INT8S*) &wifi_jpeg_picture_header[0], 624);
	}			

	return my_video_encode_sensor_start();
}
#endif


void video_preview_close(int flag)
{
	if(flag==0)
	{
		video_preview_stop(0); // ¤£Ãösensor
	}
	else
	{
	     	video_preview_stop(1); // Ãösensor
	}

	#if DUAL_STREAM_FUNC_ENABLE
	// Free Display Buffer
	if(gJpegCompressArgs.jpeg_output_addrs[JPEG_OUT_BUFFER_CNT].buffer_addrs != 0)
	{
		gp_free((void *) gJpegCompressArgs.jpeg_output_addrs[JPEG_OUT_BUFFER_CNT].buffer_addrs);
		gJpegCompressArgs.jpeg_output_addrs[JPEG_OUT_BUFFER_CNT].buffer_addrs = 0;
	}
	#endif
}

#if SUPORT_GET_JPG_BUF == CUSTOM_ON


INT32U *get_jpeg_buf;
INT32U get_jpeg_buf_flag;
INT32U get_jpeg_buf_lenth;

OS_EVENT* get_jpeg_ack_m;

INT32S ap_jpeg_get_from_buf(AVIPACKER_FRAME_INFO frame_info)
{
	INT32U i;
	INT32U *BUF;
	if((get_jpeg_buf_flag == 1)&&(get_jpeg_buf!=0)&&(frame_info.buffer_addrs !=0)){
		BUF = (INT32U*)(frame_info.buffer_addrs)+4;
		for(i=0;i<(frame_info.buffer_len/4);i++){
			*get_jpeg_buf++ = *BUF++;
		}
        get_jpeg_buf_lenth = frame_info.buffer_len;
        get_jpeg_ack_m->OSEventPtr = &get_jpeg_buf_lenth;
        OSMboxPost(get_jpeg_ack_m, (void*)C_ACK_SUCCESS);
		get_jpeg_buf = 0;
		get_jpeg_buf_flag = 0;
	}
}

INT32S ap_jpeg_get_one_frame_on_recording(INT32U addr,INT32U* lenth)
{
	INT32S msg;
	INT8U err;
	
	get_jpeg_buf = (INT32U*)addr;
	get_jpeg_buf_flag = 1;
	
	get_jpeg_ack_m = OSMboxCreate(NULL);
	msg = (INT32S) OSMboxPend(get_jpeg_ack_m, 10, &err);
	if(err != OS_NO_ERR || msg == C_ACK_FAIL)
	{
		*lenth = 0;
		DEBUG_MSG(DBG_PRINT("OSMbox ack Fail!!!\r\n"));
		OSMboxDel(get_jpeg_ack_m,0,&err);
		return (STATUS_FAIL);
	}
	*lenth = *(INT32U*)get_jpeg_ack_m->OSEventPtr;
	OSMboxDel(get_jpeg_ack_m,0,&err);
	return (STATUS_OK);
}


#endif

/****************************************************************************/
/*
 *	my_avi_encode_state_task_entry
 */
void my_avi_encode_state_task_entry(void *para)
{
	static INT8S ae_flag = -1;

	INT8U   err;
    INT32U  msg_id, encode_size;
	INT32S  nRet;
	INT32U  fifoMsg;
	INT32U  fifoIndex;
	INT8U   AddDateStampFlag = 0;
	TIME_T	g_osd_time = {0};
	INT32U  writeBufIdx;
	INT32U  Remain_Vlc_Size = 0;
	INT8U   WebCamFlag = 0;
    INT8U   RawEnableFlag=0;
	#if DUAL_STREAM_FUNC_ENABLE
    INT32U i;
	#endif

	#if PRINTF_WIFI_SPEED_ENABLE
	INT32U date_stamp_flag;
	INT32U date_stamp_msg;
	#endif

	sw_wifi_jpeg_sem_init();
	
	//+++ Jpeg Setting
	{
		gplib_jpeg_default_huffman_table_load();
		#if ENABLE_DYNAMIC_TUNING_JPEG_Q
		current_Y_Q_value = 50;
		current_UV_Q_value = 50;
		target_Y_Q_value = current_Y_Q_value;
		target_UV_Q_value = current_UV_Q_value;
		#endif
		jpeg_header_quantization_table_calculate(ENUM_JPEG_LUMINANCE_QTABLE, 50, NULL);
		jpeg_header_quantization_table_calculate(ENUM_JPEG_CHROMINANCE_QTABLE, 50, NULL);
	}

	
	while(1)
	{
        msg_id = (INT32U) OSQPend(my_AVIEncodeApQ, 0, &err);
        if((!msg_id) || (err != OS_NO_ERR))
        {
        	continue;
        }

        switch(msg_id & 0xFFFF0000)
        {
        	//+++ Preview
        	case MSG_PREVIEW_BUF_TO_DUMMY:
        		video_preview_buf_to_dummy(1);

        		
	           	OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
        	break;
        	case MSG_PREVIEW_BUF_TO_DISPLAY:

				#if DUAL_STREAM_FUNC_ENABLE
        		if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
        		{
					jpeg_Q_clear();

					gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_IDLE;		
					gJpegCompressArgs.jpeg_isr_status_flag = 0;	
					gJpegCompressArgs.jpeg_disp_next_fifo_addrs = DUMMY_BUFFER_ADDRS;
					gJpegCompressArgs.jpeg_disp_Header_Restart_Interval_Count = 0;
					gJpegCompressArgs.jpeg_disp_input_empty_count = 0;
					gJpegCompressArgs.jpeg_disp_encode_end_flag = 0;
					gDispCompressArgs.Disp_Frame_Buff_Addrs = 0;
        		}

				for(i=JPEG_OUT_BUFFER_CNT; i<(jpeg_disp_out_buffer_cnt+JPEG_OUT_BUFFER_CNT); i++)
				{	
					gJpegCompressArgs.jpeg_output_addrs[i].is_used = 0;
				}
				#endif
        	
        		video_preview_buf_to_dummy(0);
            	OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
        	break;
        	
	        case MSG_AVI_START_SENSOR: // Start preview
        		if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
        		{
	        		nRet = Wifi_Preview_Start();	        		
        		}
        		else
        		{
	        		nRet = my_video_encode_sensor_start(); 
	        	}
	        	
		        if(nRet >= 0)
		        {
	            	OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
	            }
	        	else
	        	{
	            	OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_FAIL);
	            }
	        break;

	        case MSG_AVI_DISABLE_SENSOR_CLOCK:
	        case MSG_AVI_STOP_SENSOR:
				if(ae_flag != -1) { //added by wwj, wait I2C idle
					OSQPost(my_AVIEncodeApQ, (void *)msg_id);
					break;
				}
				
	        	if((msg_id & 0xFFFF0000) == MSG_AVI_DISABLE_SENSOR_CLOCK)
	        	{
		        	video_preview_close(0); // ¤£Ãösensor
		        }
			else
			{
				video_preview_close(1);  // Ãösensor
			}
				
           		OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
	        break;
	        //---

			//+++ Zoom 
	        case MSG_AVI_ZOOM_IN:
	        	video_preview_zoom_in_out(PREVIEW_ZOOM_IN,1);
            	OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
	        break;	        
	        case MSG_AVI_ZOOM_OUT:
	        	video_preview_zoom_in_out(PREVIEW_ZOOM_OUT,0);
            	OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
	        break;
			//---

			//+++ Video Recording & Pc Cam
			case MSG_AVI_START_USB_WEBCAM:
			case MSG_AVI_START_ENCODE:
				if((msg_id & 0xFFFF0000) == MSG_AVI_START_USB_WEBCAM)
				{
					WebCamFlag = 1;
					jpeg_quality_set(AVI_Y_Q_VALUE_PC_CAM,AVI_Y_Q_VALUE_PC_CAM);
				}
				else
				{
					AddDateStampFlag = ap_state_config_date_stamp_get();
					WebCamFlag = 0;
					if(my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD)
					{
						jpeg_quality_set(AVI_Y_Q_VALUE_VIDEO_1080FHD,AVI_UV_Q_VALUE_VIDEO_1080FHD);
					}
					else
					{
						jpeg_quality_set(AVI_Y_Q_VALUE_VIDEO_720P,AVI_Y_Q_VALUE_VIDEO_720P);
					}
				}

				avi_encode_start(WebCamFlag);
				if (WebCamFlag)  {
					usb_webcam_buffer_id(vid_buf_addr);
				}
	           	//OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);  //wwj modify, wait scaler interrupt to switch path
			break;

			case MSG_AVI_STOP_USB_WEBCAM:
			case MSG_AVI_STOP_ENCODE:
				video_recording_stop();
			break;
			//---

			//+++ Capture Photo
			case MSG_AVI_CAPTURE_PICTURE:	
				#if DUAL_STREAM_FUNC_ENABLE
        		if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
				{
        			video_preview_buf_to_dummy(1);
        		}
        		#endif

				RawEnableFlag =cdsp_raw_data_save_user_config();
				if(RawEnableFlag)
				{
					video_capture_save_raw_data();
				}
				else
				{
					AddDateStampFlag = ap_state_config_capture_date_stamp_get();
					avi_capture_start();
				}
			break;
			case MSG_JPEG_CAPTURE_FRAME_DONE:
				sensor_2_dummy_addrs_wait(); // Get one frame
				avi_encoder_state_clear(AVI_ENCODER_STATE_SENSOR);

				#if DUAL_STREAM_FUNC_ENABLE
				// Free Display Buffer
				if(gJpegCompressArgs.jpeg_output_addrs[JPEG_OUT_BUFFER_CNT].buffer_addrs != 0)
				{
					gp_free((void *) gJpegCompressArgs.jpeg_output_addrs[JPEG_OUT_BUFFER_CNT].buffer_addrs);
					gJpegCompressArgs.jpeg_output_addrs[JPEG_OUT_BUFFER_CNT].buffer_addrs = 0;
				}
				#endif
				
				// Date stamp
				if(AddDateStampFlag)
				{
					//cal_time_get(&g_osd_time);
					DateTimeBufUpdate();
					#if 1 //TV_DET_ENABLE	
						#if USE_BYPASS_SCALER1_PATH 
						cpu_draw_time_osd(g_osd_time,my_pAviEncVidPara->sensor_frame_addrs, UYVY_DRAW|((AddDateStampFlag==1)?DRAW_DATE_ONLY:DRAW_DATE_TIME), (STATE_VIDEO_PREVIEW & 0xF),my_pAviEncVidPara->sensor_width,my_pAviEncVidPara->sensor_height);
						#else
						cpu_draw_time_osd(g_osd_time,my_pAviEncVidPara->sensor_frame_addrs, YUYV_DRAW|((AddDateStampFlag==1)?DRAW_DATE_ONLY:DRAW_DATE_TIME), (STATE_VIDEO_PREVIEW & 0xF),my_pAviEncVidPara->sensor_width,my_pAviEncVidPara->sensor_height);
						#endif
					#else
					cpu_draw_time_osd(g_osd_time,my_pAviEncVidPara->sensor_frame_addrs, YUV420_DRAW|((AddDateStampFlag==1)?DRAW_DATE_ONLY:DRAW_DATE_TIME), (STATE_VIDEO_PREVIEW & 0xF));
					#endif					
				}
				
				avi_scale_up_start(); // Scale up the frame
			break;

			//+++ Jpeg
			case MSG_JPG_ISR_OUTPUT_FULL_SW:
			case MSG_JPG_ISR_OUTPUT_FULL:
				if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_PHOTO)
				{
					// Jpeg Engine Restart
					gJpegCompressArgs.jpeg_output_addrs_idx++;
					if(gJpegCompressArgs.jpeg_output_addrs_idx >= JPEG_VLC_BUFFER_CNT)
					{
						gJpegCompressArgs.jpeg_output_addrs_idx = 0;
					}
					
					jpeg_vlc_output_restart(gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_addrs, JPEG_VLC_BUFFER_SIZE);

					// Write to a file
					if(gJpegCompressArgs.jpeg_output_addrs_idx == 0)
					{
						writeBufIdx = JPEG_VLC_BUFFER_CNT - 1;
					}
					else
					{
						writeBufIdx = gJpegCompressArgs.jpeg_output_addrs_idx - 1;
					}
						
					cache_invalid_range(gJpegCompressArgs.jpeg_output_addrs[writeBufIdx].buffer_addrs, JPEG_VLC_BUFFER_SIZE);
					write(gJpegCompressArgs.jpeg_file_handle, gJpegCompressArgs.jpeg_output_addrs[writeBufIdx].buffer_addrs, JPEG_VLC_BUFFER_SIZE);										
					Jpeg_Vlc_Total_Size += JPEG_VLC_BUFFER_SIZE;													
				}
				else // Check jpeg output size when in Video Recording & UVC mode
				{
					#if DUAL_STREAM_FUNC_ENABLE
						/*
							FIFO¤À¬qÀ£¨îªºJPEG ¶W¥XJPEG OUT Buffer
						*/
						if(gJpegCompressArgs.jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
						{
							gJpegCompressArgs.jpeg_skip_this_one_flag = 1;

							#if ENABLE_DYNAMIC_TUNING_JPEG_Q
							if(gJpegCompressArgs.jpeg_video_next_fifo_addrs != DUMMY_BUFFER_ADDRS)
							{
								if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
								{
									current_VLC_size = max_VLC_size;
									dynamic_tune_jpeg_Q(FIFO_SOURCE_FROM_VIDEO,current_VLC_size);
								}
							}
							#endif

							if((msg_id & 0xFFFF0000) == MSG_JPG_ISR_OUTPUT_FULL) // By HW
							{
								jpeg_vlc_output_restart(DUMMY_BUFFER_ADDRS, 0x800000); // dummy address
								break;
							}
						}
						else // FIFO_SOURCE_FROM_DISPLAY
						{
							if((msg_id & 0xFFFF0000) == MSG_JPG_ISR_OUTPUT_FULL) // By HW
							{
								gDispCompressArgs.Disp_Skip_WIFI_Jpeg_Flag = 1;
								jpeg_vlc_output_restart(DUMMY_BUFFER_ADDRS, 0x800000); // dummy address
								break;
							}
							else
							{						
								gJpegCompressArgs.jpeg_disp_new_jpeg_buffer = 0;
								gJpegCompressArgs.jpeg_disp_input_empty_count = 0;
								gJpegCompressArgs.jpeg_disp_Header_Restart_Interval_Count = 0;
								gJpegCompressArgs.jpeg_disp_encode_end_flag = 1;
								ap_display_queue_put(display_isr_queue, gDispCompressArgs.Disp_Frame_Buff_Addrs);

								/*
									±N­n¨ìJPEG OUT Buffer ÁÙ¦^¥h
								*/
								if(gJpegCompressArgs.jpeg_disp_next_fifo_addrs != DUMMY_BUFFER_ADDRS)
								{
									jpeg_compress_free_addrs_set(&gJpegCompressArgs, gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx);							
									gJpegCompressArgs.jpeg_disp_next_fifo_addrs = DUMMY_BUFFER_ADDRS;
									dynamic_tune_jpeg_Q(FIFO_SOURCE_FROM_DISPLAY,JPEG_DISP_OUT_BUFFER_SIZE);
								}
							}
						}					
					#else
						gJpegCompressArgs.jpeg_skip_this_one_flag = 1;
						
						//+++
						#if ENABLE_DYNAMIC_TUNING_JPEG_Q
						if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
						{
							dynamic_tune_jpeg_Q(current_VLC_size);
						}
						#endif
						//---
					#endif
					gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_IDLE;
				    goto GOTO_MSG_JPEG_QUEUE_NOTIFY;
				}
			break;
			case MSG_JPG_ISR_ENCODE_DONE:
				//+++ ±N jpeg °e¥X¥h 
				if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_PC_CAM)
				{
					gJpegCompressArgs.jpeg_input_empty_count = 0;

					//+++ °e¨ì USB Task
					if  (gJpegCompressArgs.jpeg_output_addrs_idx!=JPEG_OUT_BUFFER_CNT)
					{
						INT32U FrameSize;
						INT32U AddrFrame;
						#if DUAL_STREAM_FUNC_ENABLE	
							FrameSize = gJpegCompressArgs.jpeg_vlc_size+624;
						#else						
							FrameSize = gJpegCompressArgs.jpeg_vlc_size;
						#endif
						AddrFrame = gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_addrs;
						usb_send_video((INT8U*)(AddrFrame), FrameSize);
					}		

					// »Ý­nµ¥¨ìJPEG Done ¤~¥i¦^UI ACK
					if(video_recording_stop_get())
					{
						gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_NOTHING;
						OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
			           	break;
					}					
				}
				#if DUAL_STREAM_FUNC_ENABLE
				else if((gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)||
						(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_DISPLAY)
					   )
				{
					if(gJpegCompressArgs.jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
					{
						gJpegCompressArgs.jpeg_input_empty_count++;
						if((gJpegCompressArgs.jpeg_input_empty_count < gJpegCompressArgs.jpeg_input_empty_count_max) && (gJpegCompressArgs.jpeg_skip_this_one_flag == 0))
						{						
							if(jpeg_compress_FIFO_next_addrs_set(&gJpegCompressArgs))
							{
								//µ{¦¡¶]¨ìMSG_JPG_ISR_OUTPUT_FULL
								break;
							}
						}
						else
						{						
							// °e¨ìAVIPacker
							gJpegCompressArgs.jpeg_video_new_jpeg_buffer = 0;
							gJpegCompressArgs.jpeg_input_empty_count = 0;
							gJpegCompressArgs.jpeg_video_Header_Restart_Interval_Count = 0;

							if(gJpegCompressArgs.jpeg_video_next_fifo_addrs != DUMMY_BUFFER_ADDRS)
							{
								if(gJpegCompressArgs.jpeg_skip_this_one_flag == 0)
								{							
									if (free_JPEG_buffer_size > JPEG_OUT_BUFFER_CNT - AVI_PACKER_TH)
									{
										if (time_stamp_buffer_size!=0)
										{
											INT32U i;
											//DBG_PRINT("W0(%d)",time_stamp_buffer_size);  // ³q±`¥u¦³¤@¶ô
											for (i=0;i<time_stamp_buffer_size;++i)
											{
												if(pfn_avi_encode_put_data) {
													pfn_avi_encode_put_data(pAviEncPara->AviPackerCur->avi_workmem, &gJpegCompressArgs.jpeg_output_addrs[time_stamp_buffer[i]]);
												}
												time_stamp_buffer[i] = 0;
											}
											time_stamp_buffer_size = 0;	// ¤W¦¸buffer Àþ¶¡³£ÁÙ§¹¤F¡A¦ý free_JPEG_buffer_size ¨S§ó·s¡A©Ò¥H¶i time_stamp_buffer¡Aµ²ªG³o¦¸´N¶i¨Ó³oùØ
										}
													
										/*
											jpeg_video_next_fifo_addrs­n¥[³Ì«á¤@¦¸jpeg_vlc_size
										*/
										encode_size = ((gJpegCompressArgs.jpeg_video_next_fifo_addrs+gJpegCompressArgs.jpeg_vlc_size)-gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_addrs);

										#if ENABLE_DYNAMIC_TUNING_JPEG_Q
										  current_VLC_size = encode_size;
										  dynamic_tune_jpeg_Q(FIFO_SOURCE_FROM_VIDEO,current_VLC_size);
										#endif

										/*
											¦©¦^JPEG Header ¦ì¸m
										*/
										first_time = OSTimeGet();
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_time = first_time;
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].msg_id = AVIPACKER_MSG_VIDEO_WRITE;
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_len = encode_size;
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_idx = gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx;
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].src_from = FIFO_SOURCE_FROM_VIDEO;
										
										if(pfn_avi_encode_put_data) {
											pfn_avi_encode_put_data(pAviEncPara->AviPackerCur->avi_workmem, &gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx]);
											#if SUPORT_GET_JPG_BUF == CUSTOM_ON
											ap_jpeg_get_from_buf(gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx]);
											#endif
										}
									}
									else
									{
										encode_size = ((gJpegCompressArgs.jpeg_video_next_fifo_addrs+gJpegCompressArgs.jpeg_vlc_size)-gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_addrs);

										//+++
										#if ENABLE_DYNAMIC_TUNING_JPEG_Q
										  current_VLC_size = encode_size;
										  dynamic_tune_jpeg_Q(FIFO_SOURCE_FROM_VIDEO,current_VLC_size);
										#endif
										//---
								
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_time = OSTimeGet();
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].msg_id = AVIPACKER_MSG_VIDEO_WRITE;
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_len = encode_size;
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].buffer_idx = gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx;
										gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx].src_from = FIFO_SOURCE_FROM_VIDEO;
										// ¦s°_¨Ó
										time_stamp_buffer[time_stamp_buffer_size] = gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx;
										time_stamp_buffer_size++;
										//DBG_PRINT("S");
									}
								}
								else
								{
									jpeg_compress_free_addrs_set(&gJpegCompressArgs, gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx);
								}
							}

							gJpegCompressArgs.jpeg_video_next_fifo_addrs = DUMMY_BUFFER_ADDRS;
						}

						// »Ý­nµ¥¨ìJPEG Done ¤~¥i¦^UI ACK
						if(video_recording_stop_get())
						{			
							OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
							if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
							{							
								gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_DISPLAY;
							}
							else
							{
								break;
							}
				        }											
					}
					else // FIFO_SOURCE_FROM_DISPLAY
					{												
						gJpegCompressArgs.jpeg_disp_input_empty_count++;
						if((gJpegCompressArgs.jpeg_disp_input_empty_count < gJpegCompressArgs.jpeg_disp_input_empty_max_count) && (gDispCompressArgs.Disp_Skip_WIFI_Jpeg_Flag == 0))
						{
							/*
							 JPEG OUT Buffer ¨S¦³¶W¥X,Ä~Äò°µ¤U¥h
							*/
							if(jpeg_compress_FIFO_next_addrs_set(&gJpegCompressArgs) == 0)
							{
								#if PRINTF_WIFI_SPEED_ENABLE
								if((gJpegCompressArgs.jpeg_disp_input_empty_count >= (gJpegCompressArgs.jpeg_disp_input_empty_max_count-3)) &&
								   (gJpegCompressArgs.jpeg_disp_input_empty_count < (gJpegCompressArgs.jpeg_disp_input_empty_max_count))
								)
								{
									date_stamp_flag = gJpegCompressArgs.jpeg_disp_input_empty_count-(gJpegCompressArgs.jpeg_disp_input_empty_max_count-3);
									date_stamp_msg = MSG_VIDEO_ENCODE_FIFO_1ST_DATE+(date_stamp_flag*0x00100000);
									
									display_frame_ready_notify_callback(FIFO_SOURCE_FROM_DISPLAY,date_stamp_msg,(gDispCompressArgs.Disp_Frame_Buff_Addrs+(gJpegCompressArgs.jpeg_disp_input_empty_count*gDispCompressArgs.Disp_rec_fifo_data_size)),gJpegCompressArgs.jpeg_disp_input_empty_count);
								}
								else
								#endif
								{																		
									display_frame_ready_notify_callback(FIFO_SOURCE_FROM_DISPLAY,MSG_VIDEO_ENCODE_FIFO_CONTINUE,(gDispCompressArgs.Disp_Frame_Buff_Addrs+(gJpegCompressArgs.jpeg_disp_input_empty_count*gDispCompressArgs.Disp_rec_fifo_data_size)),gJpegCompressArgs.jpeg_disp_input_empty_count);
								}
							}
							else
							{	
								//µ{¦¡¶]¨ìMSG_JPG_ISR_OUTPUT_FULL
								break;
							}
						}
						else
						{
							gJpegCompressArgs.jpeg_disp_new_jpeg_buffer = 0;
							gJpegCompressArgs.jpeg_disp_input_empty_count = 0;
							gJpegCompressArgs.jpeg_disp_Header_Restart_Interval_Count = 0;
							gJpegCompressArgs.jpeg_disp_encode_end_flag = 1;
							
							ap_display_queue_put(display_isr_queue, gDispCompressArgs.Disp_Frame_Buff_Addrs);
							
							if(gJpegCompressArgs.jpeg_disp_next_fifo_addrs != DUMMY_BUFFER_ADDRS)
							{		
								if(gDispCompressArgs.Disp_Skip_WIFI_Jpeg_Flag == 0)
								{
									encode_size = ((gJpegCompressArgs.jpeg_disp_next_fifo_addrs+gJpegCompressArgs.jpeg_vlc_size)-gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx].buffer_addrs);							

								  	dynamic_tune_jpeg_Q(FIFO_SOURCE_FROM_DISPLAY,encode_size);
									
									gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx].src_from = FIFO_SOURCE_FROM_DISPLAY;							
									mjpegWriteData[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx-JPEG_OUT_BUFFER_CNT].msg_id = MJPEG_SEND_EVENT;
									mjpegWriteData[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx-JPEG_OUT_BUFFER_CNT].mjpeg_addr = gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx].buffer_addrs;
									mjpegWriteData[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx-JPEG_OUT_BUFFER_CNT].mjpeg_addr_idx = gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx;
									mjpegWriteData[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx-JPEG_OUT_BUFFER_CNT].mjpeg_size = encode_size;
									mjpegWriteData[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx-JPEG_OUT_BUFFER_CNT].running_app = STATE_VIDEO_RECORD;

									nRet = mjpeg_send_picture(&(mjpegWriteData[gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx-JPEG_OUT_BUFFER_CNT]));
									if(nRet)
									{
										jpeg_compress_free_addrs_set(&gJpegCompressArgs, gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx);	
									}
									#if PRINTF_WIFI_SPEED_ENABLE
									else
									{
										Wifi_Jpeg_Count++;
										Wifi_Jpeg_Data_Byte += encode_size;
									}
									#endif
								}
								else
								{
									jpeg_compress_free_addrs_set(&gJpegCompressArgs, gJpegCompressArgs.jpeg_disp_out_jpeg_buf_idx);							
								  	dynamic_tune_jpeg_Q(FIFO_SOURCE_FROM_DISPLAY,JPEG_DISP_OUT_BUFFER_SIZE);
								}								
							}						
							// ¥Î§¹²M¦¨ DUMMY_BUFFER_ADDRS
							gJpegCompressArgs.jpeg_disp_next_fifo_addrs = DUMMY_BUFFER_ADDRS; 
						}
					}
				}
				#else				
				else if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
				{
					//+++ °e¨ì AVI Packer
					if  (gJpegCompressArgs.jpeg_output_addrs_idx==JPEG_OUT_BUFFER_CNT)
					{		
						// do nothing
						#if ENABLE_DYNAMIC_TUNING_JPEG_Q
							// SD ¥d¦s¨ú³t«×ÅÜºC¡AJPEG ¤p¤@ÂI¹ï¨t²Î¤ñ¸û¦n
							// dynamic_tune_jpeg_Q(1024*1024);   // §RÀÉ®É­° Q->5¡Aµ²ªG¦³°¨ÁÉ§J
						#endif
					}
					else if (free_JPEG_buffer_size > JPEG_OUT_BUFFER_CNT - AVI_PACKER_TH)
					{
						//DBG_PRINT("P");
						if (time_stamp_buffer_size!=0)
						{
							INT32U i;
							//DBG_PRINT("W0(%d)",time_stamp_buffer_size);  // ³q±`¥u¦³¤@¶ô
							for (i=0;i<time_stamp_buffer_size;++i)
							{
								if(pfn_avi_encode_put_data) {
									pfn_avi_encode_put_data(pAviEncPara->AviPackerCur->avi_workmem, &gJpegCompressArgs.jpeg_output_addrs[time_stamp_buffer[i]]);
								}
								time_stamp_buffer[i] = 0;
							}
							time_stamp_buffer_size = 0;	// ¤W¦¸buffer Àþ¶¡³£ÁÙ§¹¤F¡A¦ý free_JPEG_buffer_size ¨S§ó·s¡A©Ò¥H¶i time_stamp_buffer¡Aµ²ªG³o¦¸´N¶i¨Ó³oùØ
						}

						//+++
						#if ENABLE_DYNAMIC_TUNING_JPEG_Q
							dynamic_tune_jpeg_Q(current_VLC_size);
						#endif
						//---

						encode_size = (gJpegCompressArgs.jpeg_vlc_size + 0xF)& ~0x3; //4-byte alignment
							
						first_time = OSTimeGet();
						gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_time = first_time;
						gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].msg_id = AVIPACKER_MSG_VIDEO_WRITE;
						gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_len = encode_size;
						gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_idx = gJpegCompressArgs.jpeg_output_addrs_idx;
						if(pfn_avi_encode_put_data) {
							pfn_avi_encode_put_data(pAviEncPara->AviPackerCur->avi_workmem, &gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx]);
						}
					}
					else
					{
						//+++
						#if ENABLE_DYNAMIC_TUNING_JPEG_Q
							dynamic_tune_jpeg_Q(current_VLC_size);
						#endif
						//---
				
						encode_size = (gJpegCompressArgs.jpeg_vlc_size + 0xF)& ~0x3; //4-byte alignment

						gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_time = OSTimeGet();
						gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].msg_id = AVIPACKER_MSG_VIDEO_WRITE;
						gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_len = encode_size;
						gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_idx = gJpegCompressArgs.jpeg_output_addrs_idx;
						// ¦s°_¨Ó
						time_stamp_buffer[time_stamp_buffer_size] = gJpegCompressArgs.jpeg_output_addrs_idx;
						time_stamp_buffer_size++;
						//DBG_PRINT("S");
					}

					// »Ý­nµ¥¨ìJPEG Done ¤~¥i¦^UI ACK
					if(video_recording_stop_get())
					{
						gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_NOTHING;
						OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
			           	break;
					}
				}
				#endif
				else if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_PHOTO)
				{
					gJpegCompressArgs.jpeg_input_empty_count = 0;

					// ³Ì¤@µ§­n¼g¶i¥h
					Remain_Vlc_Size = gJpegCompressArgs.jpeg_vlc_size - Jpeg_Vlc_Total_Size;
					cache_invalid_range(gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_addrs, Remain_Vlc_Size);
					write(gJpegCompressArgs.jpeg_file_handle, gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_addrs, Remain_Vlc_Size);					
					close(gJpegCompressArgs.jpeg_file_handle);
			    	avi_scale_up_stop();
   					#if DUAL_STREAM_FUNC_ENABLE
	        		if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
					{
	        			video_preview_buf_to_dummy(0);
	        		}
	        		#endif

			    	// ³qª¾¤W¼h
			    	err = 0;
					msgQSend(ApQ, MSG_STORAGE_SERVICE_PIC_DONE, &err, sizeof(INT8S), MSG_PRI_NORMAL);
					break;
				}

				gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_IDLE;	
		    goto GOTO_MSG_JPEG_QUEUE_NOTIFY;

			/*
				Input Empty & Output Full occur at the same time
			*/
			case AVIPACKER_MSG_VIDEO_WRITE_DONE:
				//DBG_PRINT(">%d",(msg_id & 0x3F));

				jpeg_compress_free_addrs_set(&gJpegCompressArgs, (msg_id & 0x3F));

				#if DUAL_STREAM_FUNC_ENABLE
				if ((time_stamp_buffer_size>0) && (gJpegCompressArgs.jpeg_output_addrs[msg_id & 0x3F].src_from == FIFO_SOURCE_FROM_VIDEO))
				#else				
				if (time_stamp_buffer_size>0)
				#endif
				{
					first_time = gJpegCompressArgs.jpeg_output_addrs[time_stamp_buffer[0]].buffer_time;
					if(pfn_avi_encode_put_data) {
						pfn_avi_encode_put_data(pAviEncPara->AviPackerCur->avi_workmem, &gJpegCompressArgs.jpeg_output_addrs[time_stamp_buffer[0]]);
					}
					time_stamp_buffer_size--;
					if (time_stamp_buffer_size>0)
					{
						INT32S i;

						for (i=0; i<time_stamp_buffer_size; ++i)
						{
							time_stamp_buffer[i] = time_stamp_buffer[i+1];
						}
						time_stamp_buffer[i] = 0;
					} else {
						time_stamp_buffer[0] = 0;
					}
				}
				//DBG_PRINT("%d ",time_stamp_buffer_size);
				break;
	
			case MSG_JPG_ISR_INPUT_EMPTY_OUTPUT_FULL: 
				if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_PHOTO)
				{
					// Jpeg Engine Restart
					gJpegCompressArgs.jpeg_output_addrs_idx++;
					if(gJpegCompressArgs.jpeg_output_addrs_idx >= JPEG_VLC_BUFFER_CNT)
					{
						gJpegCompressArgs.jpeg_output_addrs_idx = 0;
					}
					
					jpeg_vlc_output_restart(gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_addrs, JPEG_VLC_BUFFER_SIZE);

					// Write to a file
					if(gJpegCompressArgs.jpeg_output_addrs_idx == 0)
					{
						writeBufIdx = JPEG_VLC_BUFFER_CNT - 1;
					}
					else
					{
						writeBufIdx = gJpegCompressArgs.jpeg_output_addrs_idx - 1;
					}
						
					cache_invalid_range(gJpegCompressArgs.jpeg_output_addrs[writeBufIdx].buffer_addrs, JPEG_VLC_BUFFER_SIZE);
					write(gJpegCompressArgs.jpeg_file_handle, gJpegCompressArgs.jpeg_output_addrs[writeBufIdx].buffer_addrs, JPEG_VLC_BUFFER_SIZE);									
				}
				else // Check jpeg output size when in Video Recording & UVC mode
				{
					//+++
					#if (ENABLE_DYNAMIC_TUNING_JPEG_Q & !DUAL_STREAM_FUNC_ENABLE)
					if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD)
					{
						current_VLC_size = max_VLC_size;
						dynamic_tune_jpeg_Q(current_VLC_size);
					}
					#endif
					//---
				
					gJpegCompressArgs.jpeg_skip_this_one_flag = 1;
				}
				
			case MSG_JPG_ISR_INPUT_EMPTY:				
				
				// Notify scaler to do the next block
				if(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_PHOTO)
				{
					scale_up_buffer_idx_add();
					if(scaler_engine_status_get() == SCALER_ENGINE_STATUS_IDLE)
					{
						do_scale_up_next_block();
					}

					// Jpeg Engine Restart
					if(msg_id == MSG_JPG_ISR_INPUT_EMPTY_OUTPUT_FULL)
					{
						jpeg_vlc_output_restart(gJpegCompressArgs.jpeg_output_addrs[gJpegCompressArgs.jpeg_output_addrs_idx].buffer_addrs,JPEG_VLC_BUFFER_SIZE);
					}

					gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_WAIT;
				}
				else // For video recording & web cam
				{
					#if DUAL_STREAM_FUNC_ENABLE	
						if((gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD) ||
							(gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_DISPLAY)
						   )
						{
							break;
						}
					#endif
						gJpegCompressArgs.jpeg_input_empty_count++;

						/*
							¨¾¤î¥´¦L®É¶¡¹L¤[,¤U¤@frameªºfifo ¤w¥á¨ìQ¤º,»~§ì¨ì¥H¦Ü©óµLªkjpeg start
						*/
						if(gJpegCompressArgs.jpeg_input_empty_count >= gJpegCompressArgs.jpeg_input_empty_count_max)
						{
							break;
						}
						else
						{					
							gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_WAIT;
						}
				}
				// Don't add "break;" , Continue to do jpeg compress after jpeg input empty			
			case MSG_JPEG_QUEUE_NOTIFY:
GOTO_MSG_JPEG_QUEUE_NOTIFY:
				if(gJpegCompressArgs.jpeg_engine_status == JPEG_ENGINE_STATUS_BUSY)
				{
					// Jpeg engine is busy, try again later
					break;
				}

				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				// To fix printing "MMMMM...."
				#if (Enable_Lane_Departure_Warning_System)
				if(((gJpegCompressArgs.jpeg_input_addrs >= 0xF8000000) && (gJpegCompressArgs.jpeg_input_addrs < 0xF8040000)) ||
				   ((gJpegCompressArgs.jpeg_input_addrs >= USE_SDRAM_SUPPORT_FIFO_ADDRS_START) && (gJpegCompressArgs.jpeg_input_addrs < USE_SDRAM_SUPPORT_FIFO_ADDRS_END))
				)
				#endif
				{
					#if DUAL_STREAM_FUNC_ENABLE	
					if(gJpegCompressArgs.jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
					#endif
					{
						if(gJpegCompressArgs.jpeg_input_addrs != 0)
						{
							fifo_Q_put(gJpegCompressArgs.jpeg_input_addrs);
							gJpegCompressArgs.jpeg_input_addrs = 0;
						}
					}
				}
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

				//+++ ¨úfifo address
				fifoMsg = 0;
				fifoIndex = 0;
				#if DUAL_STREAM_FUNC_ENABLE	
				jpeg_Q_get(&(gJpegCompressArgs.jpeg_fifo_source),&(gJpegCompressArgs.jpeg_input_addrs),&fifoMsg,&fifoIndex);
				#else
				jpeg_Q_get(&(gJpegCompressArgs.jpeg_input_addrs),&fifoMsg,&fifoIndex);
				#endif

				//+++ fifo address ¬Odummy ©Î cdsp overflow ¿ù»~³B²z
				#if DUAL_STREAM_FUNC_ENABLE	
				if((fifoMsg == MSG_VIDEO_ENCODE_FIFO_ERR) && (gJpegCompressArgs.jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO))
				#else
				if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_ERR)
				#endif
				{
					gJpegCompressArgs.jpeg_skip_this_one_flag = 1;
				}

				/*
					conv422to420 ¶Çdummy address ©Î jpeg °µ¸û§Ö conv422to420 ©|¥¼¶ÇFIFO data
				*/
				if(gJpegCompressArgs.jpeg_input_addrs == DUMMY_BUFFER_ADDRS ||
				   gJpegCompressArgs.jpeg_input_addrs == 0
				   )
				{
					gJpegCompressArgs.jpeg_input_addrs = 0;
					break;
				}				

				#if DUAL_STREAM_FUNC_ENABLE	
					if(gJpegCompressArgs.jpeg_fifo_source == FIFO_SOURCE_FROM_DISPLAY) 
                    { 
                    	//DBG_PRINT("fifoMsg=0x%x\r\n",fifoMsg);
                            if(gDispCompressArgs.Disp_Skip_WIFI_Jpeg_Flag == 1) 
                            { 
                                    if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_START) 
                                    {                         
                                            gDispCompressArgs.Disp_Skip_WIFI_Jpeg_Flag = 0; 

                                            // Disp_Frame_Buff_Addrs = 0 ÊÇÒ»é_Ê¼Preview 
                                            if(gJpegCompressArgs.jpeg_disp_encode_end_flag || (gDispCompressArgs.Disp_Frame_Buff_Addrs == 0)) 
                                            {                                                 
                                                    gJpegCompressArgs.jpeg_disp_encode_end_flag = 0; 
                                                    gDispCompressArgs.Disp_Frame_Buff_Addrs = gJpegCompressArgs.jpeg_input_addrs; 
                                            } 
                                            else 
                                            { 
                                                    ap_display_queue_put(display_isr_queue, gJpegCompressArgs.jpeg_input_addrs); 
                                                    break; 
                                            } 
                                    } 
                                    else 
                                    { 
                                            /* 
                                                    ®”Œ¢Ç°´Î·Å—‰,ÓÐÄÜÄjpeg_Q_get È¡³öÇ°´ÎµÄjpeg_input_addrs 
                                                    Ò²Òª·Å—‰ 
                                            */ 
                                            if(fifoIndex > gJpegCompressArgs.jpeg_disp_input_empty_count) 
                                            { 
                                                    break; 
                                            } 
                                    } 
                            } 
                            else 
                            { 
                                    if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_START) 
                                    {                         
                                            gDispCompressArgs.Disp_Skip_WIFI_Jpeg_Flag = 0; 

                                            // Disp_Frame_Buff_Addrs = 0 ÊÇÒ»é_Ê¼Preview 
                                            if(gJpegCompressArgs.jpeg_disp_encode_end_flag || (gDispCompressArgs.Disp_Frame_Buff_Addrs == 0)) 
                                            {                                                 
                                                    gJpegCompressArgs.jpeg_disp_encode_end_flag = 0; 
                                                    gDispCompressArgs.Disp_Frame_Buff_Addrs = gJpegCompressArgs.jpeg_input_addrs; 
                                            } 
                                            else 
                                            { 
                                                    ap_display_queue_put(display_isr_queue, gJpegCompressArgs.jpeg_input_addrs); 
                                                    break; 
                                            } 
                                    } 
                            } 
                    
                    } 

				#endif

				/*
					¦]¬°CONV422TO420 ¥ý§ì2­ÓBUFFER,©Ò¥H­n©µ«á2­ÓÁÙ¦^¥h
				*/
				#if (Enable_Lane_Departure_Warning_System)
				if(LDWS_Enable_Flag)
				{
					if((fifoIndex >= LDWS_start_fifo_idx) &&
					   (fifoIndex < LDWS_end_fifo_idx)
					)
					{
						if(LDWS_MSG_Send_flag == 0)
						{
		        			msgQSend(PeripheralTaskQ, MSG_LDWS_DO, NULL, NULL, MSG_PRI_NORMAL);
		        			LDWS_MSG_Send_flag = 1;
		        		}
					}
					else
					{
						LDWS_MSG_Send_flag = 0;
					}
				}
				#endif
				
				/*
					JPEG is over size or fifo is dummy
				*/	
				#if DUAL_STREAM_FUNC_ENABLE	
				if((gJpegCompressArgs.jpeg_skip_this_one_flag) &&
				   (gJpegCompressArgs.jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO)
				  )
				#else
				if(gJpegCompressArgs.jpeg_skip_this_one_flag)
				#endif
				{
					/*
						·í¬O¶}ÀYªºfifo index,¤~©ñ¦æ¶i¦æjpeg compress
						¦ý¬O²Ä¤@¶ôFIFO¥X²{cdsp overflow¤]¤£±Ò°Êjpeg engine
						ª½±µÁÙ¦^¥h
					*/

					if((fifoIndex == 1) && (fifoMsg == MSG_VIDEO_ENCODE_FIFO_START)) 
					{
						#if DUAL_STREAM_FUNC_ENABLE
							if(gJpegCompressArgs.jpeg_video_next_fifo_addrs != DUMMY_BUFFER_ADDRS)
							{
								jpeg_compress_free_addrs_set(&gJpegCompressArgs, gJpegCompressArgs.jpeg_video_out_jpeg_buf_idx);
								gJpegCompressArgs.jpeg_video_next_fifo_addrs = DUMMY_BUFFER_ADDRS;
							}
						#else
							// Stop jpeg engine
							jpeg_stop();
							if(gJpegCompressArgs.jpeg_output_addrs_idx != JPEG_OUT_BUFFER_CNT)
							{
								jpeg_compress_free_addrs_set(&gJpegCompressArgs, gJpegCompressArgs.jpeg_output_addrs_idx);
							}
						#endif
						
						gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_IDLE;
						gJpegCompressArgs.jpeg_skip_this_one_flag = 0;
						gJpegCompressArgs.jpeg_input_empty_count = 0;
						
						#if DUAL_STREAM_FUNC_ENABLE
							gJpegCompressArgs.jpeg_video_Header_Restart_Interval_Count = 0;
							gJpegCompressArgs.jpeg_video_new_jpeg_buffer = 0;
						#endif

					}
					else
					{
						// ±N¨Ï¥Î¹Lªºfifo buffer ÁÙ¦^¥h
						#if (Enable_Lane_Departure_Warning_System)
						if(((gJpegCompressArgs.jpeg_input_addrs >= 0xF8000000) && (gJpegCompressArgs.jpeg_input_addrs < 0xF8040000)) ||
						   ((gJpegCompressArgs.jpeg_input_addrs >= USE_SDRAM_SUPPORT_FIFO_ADDRS_START) && (gJpegCompressArgs.jpeg_input_addrs < USE_SDRAM_SUPPORT_FIFO_ADDRS_END))
						)
						#endif
						{							
							if(gJpegCompressArgs.jpeg_input_addrs != 0)
							{
								fifo_Q_put(gJpegCompressArgs.jpeg_input_addrs);
								gJpegCompressArgs.jpeg_input_addrs = 0;
							}
						}
						if(video_recording_stop_get())
						{
							if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
							{							
								gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_DISPLAY;
							}
							else
							{
								gJpegCompressArgs.jpeg_send_to_target = JPEG_SEND_FOR_NOTHING;
							}
							OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS);
						}						
						break;
					}
				}
				
				#if DUAL_STREAM_FUNC_ENABLE	
				if(gJpegCompressArgs.jpeg_engine_status == JPEG_ENGINE_STATUS_IDLE)
				#else
				if(gJpegCompressArgs.jpeg_engine_status == JPEG_ENGINE_STATUS_IDLE &&
				   fifoMsg == MSG_VIDEO_ENCODE_FIFO_START 
				   )
				#endif   
				{					
					//+++ ¶}ÀY¨úªº®É¶¡
					#if DUAL_STREAM_FUNC_ENABLE	
						if((gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD) &&
						   (gJpegCompressArgs.jpeg_fifo_source == FIFO_SOURCE_FROM_VIDEO) &&
							AddDateStampFlag 
						)
						{
							if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_1ST_DATE)
							{				
								DateTimeBufUpdate();
								#if PRINTF_WIFI_SPEED_ENABLE
								g_osd_time.tm_sec = FIFO_SOURCE_FROM_VIDEO;
								#endif
								cpu_draw_time_osd(g_osd_time, gJpegCompressArgs.jpeg_input_addrs, YUV420_DRAW|DRAW_DATE_TIME, (STATE_VIDEO_RECORD & 0xF),my_pAviEncVidPara->sensor_width,FIFO_LINE_LN);
							}
							else if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_2ND_DATE)
							{							
								#if PRINTF_WIFI_SPEED_ENABLE
								g_osd_time.tm_sec = FIFO_SOURCE_FROM_VIDEO;
								#endif
								cpu_draw_time_osd(g_osd_time, gJpegCompressArgs.jpeg_input_addrs, YUV420_DRAW|DRAW_DATE_TIME, (STATE_VIDEO_RECORD & 0xF)|0x40,my_pAviEncVidPara->sensor_width,FIFO_LINE_LN);
							}
							else if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_LAST_DATE)
							{							
								#if PRINTF_WIFI_SPEED_ENABLE
								g_osd_time.tm_sec = FIFO_SOURCE_FROM_VIDEO;
								#endif
								cpu_draw_time_osd(g_osd_time, gJpegCompressArgs.jpeg_input_addrs, YUV420_DRAW|DRAW_DATE_TIME, (STATE_VIDEO_RECORD & 0xF)|0x80,my_pAviEncVidPara->sensor_width,FIFO_LINE_LN);
							}						
						}

						#if PRINTF_WIFI_SPEED_ENABLE
						if(gJpegCompressArgs.jpeg_fifo_source == FIFO_SOURCE_FROM_DISPLAY)
						{
							if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_1ST_DATE)
							{											
								g_osd_time.tm_sec = FIFO_SOURCE_FROM_DISPLAY;
								cpu_draw_time_osd(g_osd_time, gJpegCompressArgs.jpeg_input_addrs, YUYV_DRAW|DRAW_DATE_TIME, (STATE_VIDEO_RECORD & 0xF),my_pAviEncVidPara->display_width,gDispCompressArgs.Disp_Fifo_Line_Cnt);
							}
							else if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_2ND_DATE)
							{							
								g_osd_time.tm_sec = FIFO_SOURCE_FROM_DISPLAY;
								cpu_draw_time_osd(g_osd_time, gJpegCompressArgs.jpeg_input_addrs, YUYV_DRAW|DRAW_DATE_TIME, (STATE_VIDEO_RECORD & 0xF)|0x40,my_pAviEncVidPara->display_width,gDispCompressArgs.Disp_Fifo_Line_Cnt);
							}
							else if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_LAST_DATE)
							{							
								g_osd_time.tm_sec = FIFO_SOURCE_FROM_DISPLAY;
								cpu_draw_time_osd(g_osd_time, gJpegCompressArgs.jpeg_input_addrs, YUYV_DRAW|DRAW_DATE_TIME, (STATE_VIDEO_RECORD & 0xF)|0x80,my_pAviEncVidPara->display_width,gDispCompressArgs.Disp_Fifo_Line_Cnt);
							}													
						}
						#endif

						
					#else						
						if((gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD) &&
							AddDateStampFlag 
						)
						{
							#if (JPEG_TIME_STAMP_SHOW == JPEG_REC_TIME)
							//cal_time_get(&g_osd_time);
							DateTimeBufUpdate();
							#elif (JPEG_TIME_STAMP_SHOW == JPEG_AE_GAIN)
							ae_gain_get(&g_osd_time);
							#endif
						}
					#endif

					gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_BUSY;
					jpeg_compress_fifo_start(&gJpegCompressArgs);
				}
				else if (gJpegCompressArgs.jpeg_engine_status == JPEG_ENGINE_STATUS_WAIT)
				{				
					// ¿ý¼v¥B­n¥[¤W®É¶¡
					if((gJpegCompressArgs.jpeg_send_to_target == JPEG_SEND_FOR_RECORD) &&
						AddDateStampFlag 
					)
					{
						if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_1ST_DATE)
						{				
							cpu_draw_time_osd(g_osd_time, gJpegCompressArgs.jpeg_input_addrs, YUV420_DRAW|DRAW_DATE_TIME, (STATE_VIDEO_RECORD & 0xF),my_pAviEncVidPara->sensor_width,FIFO_LINE_LN);
						}
						else if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_2ND_DATE)
						{							
							cpu_draw_time_osd(g_osd_time, gJpegCompressArgs.jpeg_input_addrs, YUV420_DRAW|DRAW_DATE_TIME, (STATE_VIDEO_RECORD & 0xF)|0x40,my_pAviEncVidPara->sensor_width,FIFO_LINE_LN);
						}
						else if(fifoMsg == MSG_VIDEO_ENCODE_FIFO_LAST_DATE)
						{							
							cpu_draw_time_osd(g_osd_time, gJpegCompressArgs.jpeg_input_addrs, YUV420_DRAW|DRAW_DATE_TIME, (STATE_VIDEO_RECORD & 0xF)|0x80,my_pAviEncVidPara->sensor_width,FIFO_LINE_LN);
						}						
					}
					gJpegCompressArgs.jpeg_engine_status = JPEG_ENGINE_STATUS_BUSY;
					jpeg_compress_fifo_continue(&gJpegCompressArgs);
				}				
			break;
			case MSG_UVC_BUF_DMA_DOWN:
				{
					INT32U i = msg_id&0xFFFF;
							gJpegCompressArgs.jpeg_output_addrs[i].is_used = 0;
				}

			break;				
			case MSG_UVC_BUF_FLUSH:
				{
					INT32U i = 0;
					for (i=0; i<JPEG_OUT_BUFFER_CNT; ++i)
					{					
						gJpegCompressArgs.jpeg_output_addrs[i].is_used = 0;
					}
				}
				//DBG_PRINT("UVC Buffer flush\r\n");
			break;

			//+++ Exit 
			case MSG_AVI_ENCODE_STATE_EXIT:
	       		OSMboxPost(my_avi_encode_ack_m, (void*)C_ACK_SUCCESS); 
	   			OSTaskDel(OS_PRIO_SELF);
			break;
			
			//Set Sensor
			case MSG_CDSP_AE_SHUTTER:
				ae_flag = gp_cdsp_set_exposure_time();
			break;

			default:				
			break;	        
	    }		
	}
}

/****************************************************************************/
/*
 *	my_avi_encode_state_task_create
 */
INT32S my_avi_encode_state_task_create(INT8U pori)
{
	INT8U err;
	
	my_AVIEncodeApQ = OSQCreate(my_AVIEncodeApQ_Stack, MY_AVI_ENCODE_QUEUE_MAX_LEN);
    if(!my_AVIEncodeApQ)
	{
		return STATUS_FAIL;
	}	

    my_avi_encode_ack_m = OSMboxCreate(NULL);
	if(!my_avi_encode_ack_m)
	{
		return STATUS_FAIL;
	}	
	
	err = OSTaskCreate(my_avi_encode_state_task_entry, (void*) NULL, &my_AVIEncodeStateStack[MY_C_AVI_ENCODE_STATE_STACK_SIZE - 1], pori);
	if(err != OS_NO_ERR)
	{
		return STATUS_FAIL;
	}	
	
    return STATUS_OK;
}

/****************************************************************************/
/*
 *	my_avi_encode_state_task_del
 */
INT32S my_avi_encode_state_task_del(void)
{
    INT8U   err;
    INT32S  nRet, msg;
   
    nRet = STATUS_OK; 
 	POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_ENCODE_STATE_EXIT, my_avi_encode_ack_m, 5000, msg, err);	

 Return:	
   	OSQFlush(my_AVIEncodeApQ);

    if(my_AVIEncodeApQ)
    {
  		OSQDel(my_AVIEncodeApQ, OS_DEL_ALWAYS, &err);
  	}

  	if(my_avi_encode_ack_m)
  	{
		OSMboxDel(my_avi_encode_ack_m, OS_DEL_ALWAYS, &err);
	}

    return nRet;
}

/****************************************************************************/
/*
 * avi_encoder_state_set
 */
void avi_encoder_state_set(INT32U stateSetting)
{
	OSSchedLock();
	gAviEncoderState |= stateSetting;
	OSSchedUnlock();
}

/****************************************************************************/
/*
 *	avi_encoder_state_get
 */
INT32U avi_encoder_state_get(void)
{
	return gAviEncoderState;
}

/****************************************************************************/
/*
 *	avi_encoder_state_clear
 */
void avi_encoder_state_clear(INT32U stateSetting)
{
	OSSchedLock();
	gAviEncoderState &= ~(stateSetting);
	OSSchedUnlock();
}

INT32S avi_enc_storage_full(void)
{
	DEBUG_MSG(DBG_PRINT("avi encode storage full!!!\r\n"));
	//avi_encode_video_timer_stop();
	if(OSQPost(my_AVIEncodeApQ, (void*)MSG_AVI_STORAGE_FULL) != OS_NO_ERR)
		return STATUS_FAIL;

	return STATUS_OK;
} 

INT32S avi_packer_err_handle(INT32S ErrCode)
{
	DEBUG_MSG(DBG_PRINT("AviPacker-ErrID = 0x%x!!!\r\n", ErrCode));	
	//avi_encode_video_timer_stop();
	if (ErrCode == AVIPACKER_RESULT_FILE_WRITE_ERR || ErrCode == AVIPACKER_RESULT_IDX_FILE_WRITE_ERR) {
		msgQSend(ApQ, MSG_APQ_AVI_PACKER_ERROR, NULL, NULL, MSG_PRI_NORMAL);
	}
	return 1; 	//return 1 to close task
}

#if C_UVC == CUSTOM_ON
INT32S usb_webcam_buffer_id(INT32U *pBuf)
{
	INT32U i;
	INT32U jpeg_buf_cnt = (JPEG_UVC_BUFFER_LARGE_CNT+JPEG_UVC_BUFFER_MIDDLE_CNT+JPEG_UVC_BUFFER_SMALL_CNT);

	for (i=0; i<jpeg_buf_cnt; ++i)
	{
		pBuf[i] = gJpegCompressArgs.jpeg_output_addrs[i].buffer_addrs;
		DBG_PRINT("b[%d]=0x%x\r\n",i,pBuf[i]);
		
	}
	return jpeg_buf_cnt;
}

INT32S usb_webcam_start(void)
{

	INT8U  err;
	INT32S nRet, msg;

	#if 0
	INT32S nTemp;
	
	if(avi_encode_get_status()&C_AVI_ENCODE_START)
	{
		if(avi_encode_get_status() & C_AVI_ENCODE_PAUSE)
		 	nRet = avi_enc_resume();
		 
    	//stop avi encode	
    	nRet = avi_enc_stop();
   		 //stop avi packer
    	nTemp = avi_enc_packer_stop(pAviEncPara->AviPackerCur);
    
    	if(nRet < 0 || nTemp < 0)
    		return CODEC_START_STATUS_ERROR_MAX;
	}
	#endif

	nRet = STATUS_OK;
	if((avi_encode_get_status()&C_AVI_ENCODE_USB_WEBCAM) == 0)
	{
		POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_START_USB_WEBCAM, my_avi_encode_ack_m, 5000, msg, err);
		avi_encode_set_status(C_AVI_ENCODE_USB_WEBCAM);
	}

#if C_USB_AUDIO == CUSTOM_ON
	avi_audio_record_start();
	//avi_encode_audio_timer_start();  // remove by josephhsieh@140430
#endif
Return:
	return nRet;
}

INT32S usb_webcam_stop(void)
{
	INT8U  err;
	INT32S nRet, msg;

	nRet = STATUS_OK;
	if(avi_encode_get_status()&C_AVI_ENCODE_USB_WEBCAM)
	{
		POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_STOP_USB_WEBCAM, my_avi_encode_ack_m, 5000, msg, err);
Return:
		jpeg_stop();
		avi_enc_stop_flush();
		jpeg_Q_clear();
		fifo_Q_clear();

		avi_enc_buffer_free();

	#if C_USB_AUDIO == CUSTOM_ON
		avi_audio_record_stop();
		//avi_encode_audio_timer_stop();	// remove by josephhsieh@140430
	#endif
		avi_encode_clear_status(C_AVI_ENCODE_USB_WEBCAM);
	}
	return nRet;
}
#endif




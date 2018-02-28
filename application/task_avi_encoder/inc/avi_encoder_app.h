#ifndef __AVI_ENCODER_APP_H__
#define __AVI_ENCODER_APP_H__

#include "application.h"

//=====================================================================================================
/* avi encode configure set */
#define AVI_ENCODE_PCM_BUFFER_NO		 3 	//audio record pcm use buffer number

//audio format	
#define AVI_ENCODE_AUDIO_FORMAT			WAVE_FORMAT_PCM //0: no audio, WAVE_FORMAT_PCM, WAVE_FORMAT_IMA_ADPCM and WAVE_FORMAT_ADPCM 

#define AVI_AUDIO_RECORD_TIMER		ADC_AS_TIMER_F  //adc use timer, C ~ F

//avi file max size
#define AVI_ENCODE_CAL_DISK_SIZE_EN		0				//0: disable, 1: enable	

//video format
#define C_XVID_FORMAT					0x44495658
#define	C_MJPG_FORMAT					0x47504A4D
#define AVI_ENCODE_VIDEO_FORMAT			C_MJPG_FORMAT //only support mjpeg 
//=====================================================================================================
//=====================================================================================================
/* avi encode status */
#define C_AVI_ENCODE_IDLE       0x00000000
#define C_AVI_ENCODE_PACKER0	0x00000001
#define C_AVI_ENCODE_AUDIO      0x00000002
//#define C_AVI_ENCODE_VIDEO      0x00000004
//#define C_AVI_ENCODE_SCALER     0x00000008
#define C_AVI_ENCODE_SENSOR     0x00000010
//#define C_AVI_ENCODE_PACKER1	0x00000020
//#define C_AVI_ENCODE_MD			0x00000040
#define C_AVI_ENCODE_USB_WEBCAM 0x00000080

#define C_AVI_ENCODE_START      0x00000100

/****************************************************************************/

typedef struct my_AviEncVidPara_s
{
	// Sensor
	INT32U 	sensor_frame_addrs;
    INT16U  sensor_width;
    INT16U  sensor_height; 	
    INT16U  clip_width;
    INT16U  clip_height; 	

    INT8U   sensor_do_init;
    // Display 
    INT16U  display_width;          
    INT16U  display_height;   

    INT32U  video_format;			// video encode format
    INT8U   dwScale;				// dwScale
    INT8U   dwRate;					// dwRate 
    INT16U  quality_value;			// video encode quality
    INT16U  encode_width;           // video encode width
    INT16U  encode_height;          // videoe ncode height
    INT16U  enter_ap_mode;
    
}my_AviEncVidPara_t;

//+++ AVI_ENCODER_STATE.C
extern my_AviEncVidPara_t my_AviEncVidPara, *my_pAviEncVidPara;
extern OS_EVENT* my_AVIEncodeApQ;
extern OS_EVENT* my_avi_encode_ack_m;

/****************************************************************************/
extern void my_avi_encode_init(void);

/****************************************************************************/

typedef struct AviEncAudPara_s
{
	//audio encode
	INT32U  audio_format;			// audio encode format
	INT16U  audio_sample_rate;		// sample rate
	INT16U  channel_no;				// channel no, 1:mono, 2:stereo
	INT8U   *work_mem;				// wave encode work memory 
	INT32U  pack_size;				// wave encode package size in byte
	INT32U  pack_buffer_addr;
	INT32U  pcm_input_size;			// wave encode pcm input size in short
	INT32U  pcm_input_addr[AVI_ENCODE_PCM_BUFFER_NO];
}AviEncAudPara_t;

typedef struct AviEncPacker_s
{
	void 	*avi_workmem;
	INT16S  file_handle;
    INT16S  index_handle;
    INT8U   index_path[16];
    
    //avi video info
    GP_AVI_AVISTREAMHEADER *p_avi_vid_stream_header;
    INT32U  bitmap_info_cblen;		// bitmap header length
    GP_AVI_BITMAPINFO *p_avi_bitmap_info;
    
    //avi audio info
    GP_AVI_AVISTREAMHEADER *p_avi_aud_stream_header;
    INT32U  wave_info_cblen;		// wave header length
    GP_AVI_PCMWAVEFORMAT *p_avi_wave_info;
    
    //avi packer 
	INT32U  task_prio;
	void    *index_write_buffer;
	INT32U	index_buffer_size;		// AviPacker index buffer size in byte

	INT16S txt_handle;
}AviEncPacker_t;

typedef struct AviEncPara_s
{   
	INT8U  source_type;				// SOURCE_TYPE_FS or SOURCE_TYPE_USER_DEFINE
   	
    //avi file info
    INT32U  avi_encode_status;      // 0:IDLE
    AviEncPacker_t *AviPackerCur;
    
	//allow record size
	INT64U  disk_free_size;			// disk free size
	INT32U  record_total_size;		// AviPacker storage to disk total size
}AviEncPara_t;

//extern os-event
extern OS_EVENT *AVIEncodeApQ;
extern OS_EVENT *scaler_task_q;
extern OS_EVENT *cmos_frame_q;
extern OS_EVENT *vid_enc_task_q;
extern OS_EVENT *scaler_hw_sem;

extern AviEncPara_t *pAviEncPara;
extern AviEncAudPara_t *pAviEncAudPara;
extern AviEncPacker_t *pAviEncPacker0;

#if C_UVC == CUSTOM_ON
//USB Webcam 
extern OS_EVENT *usbwebcam_frame_q;
extern OS_EVENT  *USBCamApQ;
extern OS_EVENT *USBAudioApQ;

#define USB_AUDIO_PCM_SAMPLES (2*1024)
#define AVI_AUDIO_PCM_SAMPLES (32000)

typedef struct   
{
    INT32U FrameSize;
    INT32U  AddrFrame;
    INT8U IsoSendState;
}ISOTaskMsg;
#endif

//jpeg header
extern INT8U jpeg_422_q90_header[];

//avi encode state
extern INT32U avi_enc_packer_start(AviEncPacker_t *pAviEncPacker);
extern INT32U avi_enc_packer_init(AviEncPacker_t *pAviEncPacker);
extern INT32U avi_enc_packer_stop(AviEncPacker_t *pAviEncPacker); 
extern INT32U avi_enc_packer_switch_file(AviEncPacker_t *pAviEncPacker, INT16S fd_new, INT16S fd_txt_new);
extern INT32S vid_enc_preview_start(void);
extern INT32S vid_enc_preview_stop(void);
extern INT32S vid_enc_disable_sensor_clock(void);
extern INT32S avi_enc_start(void);
extern INT32S avi_enc_stop(void);
extern INT32S avi_enc_save_jpeg(void);
extern INT32S avi_enc_storage_full(void);
extern INT32S avi_packer_err_handle(INT32S ErrCode);
extern void  (*pfn_avi_encode_put_data)(void *WorkMem, AVIPACKER_FRAME_INFO* pFrameInfo);

//audio task
extern INT32S avi_adc_gps_register(void);
extern void avi_adc_gsensor_data_register(void **msgq_id, INT32U *msg_id);
extern INT32S avi_adc_record_task_create(INT8U pori);
extern INT32S avi_adc_record_task_del(void);
extern INT32S avi_audio_record_start(void);
extern INT32S avi_audio_record_restart(void);
extern INT32S avi_audio_record_stop(void);
extern void avi_audio_record_entry(void *parm);

extern void avi_encode_audio_timer_start(void);
extern void avi_encode_audio_timer_stop(void);

extern INT32S avi_encode_set_file_handle_and_caculate_free_size(AviEncPacker_t *pAviEncPacker, INT16S FileHandle, INT16S txt_FileHandle);
extern INT16S avi_encode_close_file(AviEncPacker_t *pAviEncPacker);
extern INT32S avi_encode_set_avi_header(AviEncPacker_t *pAviEncPacker);
extern void avi_encode_set_curworkmem(void *pAviEncPacker);
//status
extern void   avi_encode_set_status(INT32U bit);
extern void   avi_encode_clear_status(INT32U bit);
extern INT32S avi_encode_get_status(void);
extern INT32S avi_encode_packer_memory_alloc(void);

extern INT32S avi_encode_disk_size_is_enough(INT32S cb_write_size);

extern INT32S avi_audio_memory_allocate(INT32U	cblen);
extern void avi_audio_memory_free(void);
extern INT32U avi_audio_get_next_buffer(void);
extern INT8U avi_audio_get_buffer_idx(void);

extern INT32S avi_adc_double_buffer_put(INT16U *data,INT32U cwlen, OS_EVENT *os_q);
extern INT32U avi_adc_double_buffer_set(INT16U *data, INT32U cwlen);
extern INT32S avi_adc_dma_status_get(void);
extern void avi_adc_double_buffer_free(void);
extern void avi_adc_hw_start(INT16U sampling_rate);
extern void avi_adc_hw_stop(void);

extern void mic_fifo_clear(void);
extern void mic_fifo_level_set(INT8U level);
extern INT32S mic_auto_sample_start(void);
extern void mic_auto_sample_stop(void);
extern INT32S mic_sample_rate_set(INT8U timer_id, INT32U hz);
extern INT32S mic_timer_stop(INT8U timer_id);
extern INT8U ap_state_config_date_stamp_get(void);

/****************************************************************************/
extern INT32S vid_enc_preview_buf_to_dummy(void);
extern INT32S vid_enc_preview_buf_to_display(void);



#endif // __AVI_ENCODER_APP_H__

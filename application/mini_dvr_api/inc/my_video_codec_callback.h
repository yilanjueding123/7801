#ifndef __MY_VIDEO_CODEC_CALLBACK_H__
#define __MY_VIDEO_CODEC_CALLBACK_H__

#include "application.h"

/****************************************************************************/
#define FIFO_SOURCE_FROM_VIDEO		1
#define FIFO_SOURCE_FROM_DISPLAY	2

#define JPEG_IMG_FORMAT_422 0x21
#define JPEG_IMG_FORMAT_420 0x22

/****************************************************************************/
#define LDWS_NEED_BUF_SIZE		192

#define DUMMY_BUFFER_ADDRS	0xF8500000

//+++ Fifo Format
#define FIFO_FORMAT_422		0
#define FIFO_FORMAT_420		1

#define FIFO_PATH_TO_VIDEO_RECORD	0
#define FIFO_PATH_TO_CAPTURE_PHOTO	1

//+++ Scaler 
#define SCALER_CTRL_OUT_FIFO_16LINE  16
/****************************************************************************/
typedef struct preview_args_s
{
	INT16U clip_width;
	INT16U clip_height;
	INT16U sensor_width;
	INT16U sensor_height;
	INT16U display_width;
	INT16U display_height;
	INT32U display_buffer_addrs;
	INT32U (*display_buffer_pointer_get)(void);
	INT32S (*display_buffer_pointer_put)(INT32U *queue, INT32U addr);
	#if DUAL_STREAM_FUNC_ENABLE
	void (*display_frame_ready_notify)(INT32U fifoSrc,INT32U fifoMsg, INT32U fifoAddrs, INT32U fifoIdx);
	#else
	void (*display_frame_ready_notify)(INT32U display_buffer_addrs);
	#endif
	INT8U  sensor_do_init;
	INT16U  run_ap_mode;
}preview_args_t;

typedef struct vid_rec_args_s
{
	INT32U vid_rec_fifo_total_count; // total count 
	INT32U vid_rec_fifo_line_len;	
	INT32U vid_rec_fifo_data_size;	
	INT32U (*fifo_ready_notify)(INT32U fifoMsg, INT32U fifoAddrs, INT32U fifoIdx);
	INT32U (*fifo_buffer_addrs_get)(void);
	INT32S (*fifo_buffer_addrs_put)(INT32U addr);
	INT8U  vid_rec_fifo_output_format;
	INT8U  vid_rec_fifo_path;
}vid_rec_args_t;

typedef struct scaler_args_s
{
	INT32U scaler_in_width; 
	INT32U scaler_in_height; 
	INT32U scaler_out_width; 
	INT32U scaler_out_height; 
	INT32U scaler_in_buffer_addrs; 
	INT32U scaler_out_buffer_addrs; 
	INT32U (*scaler_ready_notify)(INT32U fifoMsg, INT32U fifoAddrs);
	INT8U  scaler_input_format;
}scaler_args_t;

typedef enum
{
    SCALER_ENGINE_STATUS_IDLE,                     
    SCALER_ENGINE_STATUS_BUSY,
    SCALER_ENGINE_STATUS_FINISH,
    SCALER_ENGINE_STATUS_ERR_OCCUR
} SCALER_ENGINE_STATUS;


/****************************************************************************/
extern INT32S video_preview_start(preview_args_t* pPreviewArgs);
extern INT32S video_preview_stop(INT8U closeSensor);
extern INT32S sensor_2_dummy_addrs_wait(void);
extern INT32S video_preview_zoom_in_out(INT8U zoomInOut, INT8U zoomIn_zoomOut);
extern INT32S video_recording_start(vid_rec_args_t* pVidRecArgs);
extern void sensor_crop_W_H_get(INT32U* pCropWValue,INT32U* pCropHValue);
extern void video_recording_stop(void);
extern INT32U video_recording_stop_get(void);
extern void avi_enc_stop_flush(void);
extern void do_scale_up_start(scaler_args_t* pScalerArgs);
extern void do_scale_up_stop(void);
extern void do_scale_up_next_block(void);
extern void scale_up_buffer_idx_add(void);
extern void scale_up_buffer_idx_decrease(void);
extern INT32U scaler_engine_status_get(void);
extern void video_preview_buf_to_dummy(INT8U enableValue);
extern INT32U enable_black_edge_get(void);
extern void video_preview_zoom_to_zero(void);
extern INT32U video_preview_zoom_setp_get(void);

extern void DateTimeBufUpdate(void);


extern void sensor_ov7670_init(void);
extern void sensor_SOi_h22_init(INT32U WIDTH, INT32U HEIGHT);
extern void gp_cdsp_enable(INT32U SNR_WIDTH, INT32U SNR_HEIGHT, INT32U sensor_module);
extern void SensorIF_Setting(unsigned char front_type);
extern void CDSP_SensorIF_CLK(unsigned char front_type);
extern void hwFront_SetFrameSize(INT16U hoffset,INT16U voffset,INT16U hsize,INT16U vsize);
extern void gp_cdsp_start(INT32U cdsp_buffer);
/****************************************************************************/
#endif		// __MY_VIDEO_CODEC_CALLBACK_H__
/****************************************************************************/

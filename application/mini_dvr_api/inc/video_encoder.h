#ifndef __VIDEO_ENCODER_H__
#define __VIDEO_ENCODER_H__

#include "avi_encoder_app.h"

#include "my_avi_encoder_state.h"

/****************************************************************************/
#define JPEG_OUT_BUFFER_LARGE_SIZE	(230*1024)//(240*1024)//250	// It will influence UVC
#define JPEG_OUT_BUFFER_MIDDLE_SIZE	(140*1024)//(145*1024)//150
#define JPEG_OUT_BUFFER_SMALL_SIZE	(120*1024)

// Video Record buffer alloc
#if SUPORT_GET_JPG_BUF==CUSTOM_OFF

#define  JPEG_OUT_BUFFER_LARGE_CNT	0
#define JPEG_OUT_BUFFER_MIDDLE_CNT	0
#if (USE_PANEL_NAME == PANEL_T43)
#define JPEG_OUT_BUFFER_SMALL_CNT	29
#else
#define JPEG_OUT_BUFFER_SMALL_CNT	32
#endif
#define JPEG_OUT_1080P_LARGE_CNT		0
#define JPEG_OUT_1080P_MIDDLE_CNT		28
#define JPEG_OUT_1080P_SMALL_CNT		0

#else
//USED
#define  JPEG_OUT_BUFFER_LARGE_CNT	0
#define JPEG_OUT_BUFFER_MIDDLE_CNT	28//0
#define JPEG_OUT_BUFFER_SMALL_CNT	0 //(32-2)

#define JPEG_OUT_1080P_LARGE_CNT		16  //0
#define JPEG_OUT_1080P_MIDDLE_CNT		0	//(28-2)
#define JPEG_OUT_1080P_SMALL_CNT		0

#endif

void video_encode_entrance(void);
void video_encode_exit(void);
extern INT8U ap_state_config_voice_record_switch_get(void);
extern void ap_peripheral_gsensor_data_register(void );

CODEC_START_STATUS video_encode_preview_start(VIDEO_ARGUMENT arg);
CODEC_START_STATUS video_encode_preview_stop(void);
CODEC_START_STATUS video_encode_start(MEDIA_SOURCE src, INT16S txt_handle);
CODEC_START_STATUS video_encode_fast_stop_and_start(MEDIA_SOURCE src, INT16S next_txt_handle);
CODEC_START_STATUS video_encode_stop(void);
CODEC_START_STATUS video_encode_auto_switch_csi_frame(void);
CODEC_START_STATUS video_encode_auto_switch_csi_fifo_end(INT8U flag);
CODEC_START_STATUS video_encode_auto_switch_csi_frame_end(INT8U flag);
CODEC_START_STATUS video_encode_set_zoom_scaler(FP32 zoom_ratio);
CODEC_START_STATUS video_encode_capture_picture(MEDIA_SOURCE src);
CODEC_START_STATUS video_encode_fast_switch_stop_and_start(MEDIA_SOURCE src);

extern void video_capture_save_raw_data(void);
extern INT8U cdsp_raw_data_save_user_config(void);

#endif

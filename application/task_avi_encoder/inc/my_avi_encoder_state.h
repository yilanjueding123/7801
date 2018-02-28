#ifndef __MY_AVI_ENCODER_STATE_H__
#define __MY_AVI_ENCODER_STATE_H__

#include "application.h"
#include "avi_encoder_app.h"

#include "my_video_codec_callback.h"
#include "my_encoder_jpeg.h"

/****************************************************************************/
#define AVI_ENCODER_STATE_IDLE       0x00000000
#define AVI_ENCODER_STATE_VIDEO      0x00000001
#define AVI_ENCODER_STATE_SENSOR     0x00000002
#define AVI_ENCODER_STATE_PACKER0	 0x00000004

extern INT32U jpeg_out_buffer_large_size;
extern INT32U jpeg_out_buffer_middle_size;
extern INT32U jpeg_out_buffer_small_size;

// Video Record buffer alloc
extern INT32U jpeg_out_buffer_large_cnt;
extern INT32U jpeg_out_buffer_middle_cnt;
extern INT32U jpeg_out_buffer_small_cnt;

extern INT32U jpeg_out_1080p_large_cnt;
extern INT32U jpeg_out_1080p_middle_cnt;
extern INT32U jpeg_out_1080p_small_cnt;

// WebCam buffer allc
#define JPEG_UVC_BUFFER_LARGE_CNT 	 	4
#define JPEG_UVC_BUFFER_MIDDLE_CNT 	 	0
#define JPEG_UVC_BUFFER_SMALL_CNT			0

/****************************************************************************/
extern INT32S my_avi_encode_state_task_create(INT8U pori);
extern INT32S my_avi_encode_state_task_del(void);

extern void avi_encoder_state_set(INT32U stateSetting);
extern INT32U avi_encoder_state_get(void);
extern void avi_encoder_state_clear(INT32U stateSetting);
extern void avi_enc_buffer_free(void);

extern void video_preview_close(int flag);

/****************************************************************************/
#endif		// __MY_AVI_ENCODER_STATE_H__
/****************************************************************************/

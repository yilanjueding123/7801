#ifndef __VIDEO_CODEC_CALLBACK_H__
#define __VIDEO_CODEC_CALLBACK_H__
#include "application.h"
#include "drv_l1_sfr.h"

//video decode
extern void video_decode_end(void);
extern void video_decode_FrameReady(INT8U *FrameBufPtr);

//video encode
extern INT32U video_encode_sensor_start(INT32U csi_frame1, INT32U csi_frame2);
extern INT32U video_encode_sensor_stop(void);
extern INT32S video_encode_display_frame_ready(INT32U frame_buffer);
extern void  video_encode_frame_ready(void *WorkMem, AVIPACKER_FRAME_INFO* pFrameInfo);
extern void video_encode_end(void *workmem);

//display 
extern void user_defined_video_codec_entrance(void);
extern void video_codec_show_image(INT8U TV_TFT, INT32U BUF,INT32U DISPLAY_MODE ,INT32U SHOW_TYPE);
extern void tft_vblank_isr_register(void (*user_isr)(void));
extern void tv_vblank_isr_register(void (*user_isr)(void));
#if C_MOTION_DETECTION == CUSTOM_ON
	extern void motion_detect_isr_register(void (*user_isr)(void));
#endif
#endif

#ifndef __STATE_USBD_H__
#define __STATE_USBD_H__

#include "application.h"
#include "Customer.h"

/*****  MSDC *****/
extern void USBD_MSDC_Init(void);

/*****  UVC *****/
extern void iso_video_enable(void);
extern int usb_send_video(INT8U *jpeg_addr, INT32U jpeg_size);
extern int usb_send_audio(INT8U *aud_addr, INT32U aud_size);
extern void usb_uvc_start(void);

#endif  /*__STATE_CALENDAR_H__*/

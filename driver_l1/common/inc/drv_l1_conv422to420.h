#ifndef __DRV_L1_CONV422TO420_H__
#define __DRV_L1_CONV422TO420_H__

#include "driver_l1.h"
#include "drv_l1_sfr.h"

//--------------------------------
#define CONV422_INPUT_IMG_FORMAT_YUYV		0
#define CONV422_INPUT_IMG_FORMAT_UYVY		2
//--------------------------------

typedef struct 
{										// Offset
	volatile INT32U	BUF_A_ADDR;       	// 0x0008
	volatile INT32U	BUF_B_ADDR;       	// 0x000C
	volatile INT32U	IMG_WIDTH;       	// 0x0010
	volatile INT32U	IMG_HEIGHT;       	// 0x0014
	volatile INT32U	CTRL;       		// 0x0018
	volatile INT32U	BUF_DEPTH;       	// 0x001C
	volatile INT32U	FIFO_LINE;       	// 0x0020
	volatile INT32U	FIFO_A_LINE;       	// 0x0024
	volatile INT32U	FIFO_B_LINE;       	// 0x0028	
}CONV422_SFR;

typedef enum
{
    FIFO_MODE,                 
    FRAME_MODE
} CONV422_MODE;

typedef enum
{
    FORMAT_420,                 
    FORMAT_422
} CONV422_FORMAT;

typedef enum
{
    CONV422_IDLE_BUF_A,                 
    CONV422_IDLE_BUF_B,
    CONV422_IDLE_BUF_NON
} CONV422_IDLE_BUF;


// Extern APIs
extern INT32S conv422_init(void);
extern INT32S conv422_input_pixels_set(INT32U inWidth, INT32U inHeight);
extern INT32S conv422_fifo_line_set(INT32U lineCount);
extern INT32S conv422_output_A_addr_set(INT32U addr);
extern INT32U conv422_output_A_addr_get(void);
extern INT32S conv422_output_B_addr_set(INT32U addr);
extern INT32U conv422_output_B_addr_get(void);
extern INT32S conv422_mode_set(INT8U mode);
extern INT32S conv422_wait_idle(void);
extern INT32S conv422_idle_check(void);
extern INT32S conv422_bypass_enable(INT8U bypassEnable);
extern INT32S conv422_output_format_set(INT8U mode);
extern INT32S conv422_fifo_interrupt_enable(INT8U enableValue);
extern INT32S conv422_idle_buf_get(INT8U waitValue);
extern INT32S conv422_clear_set(void);
extern INT32S conv422_frame_end_check(void);
extern INT32S conv422_input_img_format_set(INT32U inImgFormat);

#endif		// __DRV_L1_CONV422TO420_H__


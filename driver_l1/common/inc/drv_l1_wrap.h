#ifndef __drv_l1_WRAP_H__
#define __drv_l1_WRAP_H__

#include "driver_l1.h"
#include "drv_l1_sfr.h"


typedef struct 
{										// Offset
	volatile INT32U	CTRL;           	// 0x0000
	volatile INT32U	WRAP_SADDR;        	// 0x0004
	volatile INT32U	FILTER_SADDR;      	// 0x0008
	volatile INT32U	FILTER_SIZE;      	// 0x000C
	volatile INT32U	FRAME_WIDTH;      	// 0x0010
	volatile INT32U	FRAME_HEIGHT;      	// 0x0014
	volatile INT32U	CLIP_SRC_WIDTH;    	// 0x0018
	volatile INT32U	CLIP_SRC_HEIGHT;   	// 0x001C
	volatile INT32U	CLIP_START_X;    	// 0x0020
	volatile INT32U	CLIP_START_Y;   	// 0x0024
}WRAP_SFR;

typedef enum
{
    WRAP_SCA2TFT,                 
    WRAP_CSIMUX,                 
    WRAP_CSI2SCA,
    WRAP_MAX
} WRAP_NUM;


// Wrap extern APIs
extern INT32S wrap_filter_enable(INT8U wrap_num,INT8U enableFilter);
extern INT32S wrap_path_set(INT8U wrap_num, INT8U pathSrEnable, INT8U pathOEnable);
extern INT32S wrap_addr_set(INT8U wrap_num, INT32U addr);
extern INT32S wrap_filter_addr_set(INT8U wrap_num, INT32U addr, INT32U addrSize);
extern INT32S wrap_filter_flush(INT8U wrap_num);
extern INT32S wrap_busy_wait(INT8U wrap_num);
extern INT32S wrap_idle_check(INT8U wrap_num);
extern INT32S wrap_protect_enable(INT8U wrap_num, INT8U enableProtect);
extern INT32S wrap_protect_pixels_set(INT8U wrap_num, INT32U inWidth, INT32U inHeight);
extern INT32S wrap_clip_mode_enable(INT8U wrap_num, INT8U enableClip);
extern INT32S wrap_clip_source_pixels_set(INT8U wrap_num, INT32U inWidth, INT32U inHeight);
extern INT32S wrap_clip_start_pixels_set(INT8U wrap_num, INT32U inStartX, INT32U inStartY);
extern INT32S wrap_protect_status_get(INT8U wrap_num);

#endif		// __drv_l1_WRAP_H__


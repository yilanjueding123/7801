#ifndef __DRV_L2_SENSOR_H__
#define __DRV_L2_SENSOR_H__

/****************************************************************************/
#include "project.h"
#include "driver_l2_cfg.h"
#include "ap_peripheral_handling.h"
/****************************************************************************/
//#define 	MD_SENS			10
//#define 	MD_NORMAL		32
//#define	MD_SLOW			100
//#define	CDSP_MD_THR		MD_NORMAL
/****************************************************************************/
typedef struct {
	INT32U	point_x; 
	INT32U 	point_y;
	INT32U  frame_w;
	INT32U	frame_h; 
	INT32U  clip_w;
	INT32U	clip_h; 
	INT32U  scaledown_w;
	INT32U	scaledown_h; 
	INT8U 	docropflag;
	INT8U   doscaledownflag;
}sensor_frame_range_t;

typedef struct {
	void   (*init)(void);
	void   (*start)(INT32U frameAddrs, INT32U motionAddrs);
	void   (*stop)(void);
	void   (*wait4FrameEnd)(void);
	void   (*frameRangeClip)(sensor_frame_range_t* frameRange);
	void   (*set_fps)(INT32U fpsValue);
	INT32U (*get_fps)(void); 
}sensor_apis_ops;

/****************************************************************************/
/*
 *	Sensor extern APIs
 */
/****************************************************************************/
/*
 *	sensor_attach 	
 */
extern  sensor_apis_ops* sensor_attach(void);


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct sensor_calibration_s
{
	const unsigned short *lenscmp;
	const unsigned short (*r_b_gain)[2];
	const unsigned int *gamma;
	const short *color_matrix;
	const short *awb_thr;
	
} sensor_calibration_t;

typedef struct callbackfunc_s {
	int (*powerctl)(int ctl);
	int (*standby)(int ctl);
	int (*reset)(int ctl);
	int (*set_port)(char *port);
}callbackfunc_t;

/** @brief A structure of sensor config */
typedef struct sensor_timing_s 
{
	unsigned char *desc;
	unsigned int pixelformat;
	unsigned int bpp;
	unsigned int mclk_src;
	unsigned int mclk;
	unsigned int hpixel;
	unsigned int hoffset;
	unsigned int vline;
	unsigned int voffset;

	sensor_calibration_t *cdsp_calibration;
}sensor_fmt_t;

/****************************************************************************/
#endif	// __DRV_L2_SENSOR_H__
/****************************************************************************/

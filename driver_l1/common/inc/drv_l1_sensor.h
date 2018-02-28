#ifndef __drv_l1_SENSOR_H__
#define __drv_l1_SENSOR_H__

#include "driver_l1.h"
#include "drv_l1_sfr.h"


#define CSI_10FPS			0
#define CSI_15FPS			1
#define CSI_27FPS			2	
#define CSI_30FPS			3
#define CSI_07FPS			4
#define CSI_FPS				CSI_10FPS

#define CSI_CLOCK_SYS_CLK_DIV2	0
#define CSI_CLOCK_27MHZ			1
#define CSI_CLOCK_SYS_CLK_DIV4	2
#define CSI_CLOCK_13_5MHz		3

#define CSI_CLOCK				CSI_CLOCK_27MHZ	

#define CSI_PPU_FRAME_MODE		0
#define CSI_TG_FRAME_MODE		1
#define CSI_FIFO_8_MODE			2
#define CSI_FIFO_16_MODE		3
#define CSI_FIFO_32_MODE		4
#if VIDEO_ENCODE_USE_MODE == SENSOR_BUF_FIFO_MODE
	#define CSI_MODE			CSI_FIFO_32_MODE
#else
	#define CSI_MODE			CSI_TG_FRAME_MODE
#endif
// Sensor Interface Control Register Definitions
/*
R_CSI_TG_CTRL0			-	Timing Generator Control 1
*/
#define CSIEN					(1<<0)			// Sensor controller enable bit
#define CAP					(1<<1)			// Capture / preview mode
#define HREF					(1<<2)			// HREF / HSYNC mode
#define CSI_RGB888IN			(0<<3)
#define CSI_RGB565IN			(1<<3)			// RGB565 / RGB888 input @YUVIN=0B
#define CLKIINV				(1<<4)			// Invert input clock
#define YUVIN					(1<<5)			// YUV / RGB input
#define YUVOUT				(1<<6)			// YUV / RGB output
#define BSEN					(1<<7)			// Enable blue screen effect
#define CCIR656				(1<<8)			// CCIR656 / CCIR601 Interface
#define FGET_RISE				(1<<9)			// Field latch timing at the rising edge of VSYNC
#define HRST_FALL				(0<<10)			// Horizontal counter reset at the rising edge of HSYNC
#define HRST_RISE				(1<<10)			// Horizontal counter reset at the rising edge of HSYNC
#define VADD_FALL				(0<<11)			// Vertical counter increase at the rising edge of HSYNC
#define VADD_RISE				(1<<11)			// Vertical counter increase at the rising edge of HSYNC
#define VRST_FALL				(0<<12)			// Vertical counter reset at the rising edge of VSYNC
#define VRST_RISE				(1<<12)			// Vertical counter reset at the rising edge of VSYNC
#define YUV_YUYV				(1<<13)			// YUYV(GBGR) / UYVY (BGRG) selection
#define FIELDINV   	 			(1<<14)			// Invert field input
#define CSI_INTERLACE  			(1<<15)			// Interlace / non-interlace mode
/*
R_CSI_TG_CTRL1			-	Timing Generator Control 2
*/
#define D_TYPE0         		(0<<0)			// Data latch delay 1 clock
#define D_TYPE1         		(1<<0)			// Data latch delay 2 clock
#define D_TYPE2         		(2<<0)			// Data latch delay 3 clock
#define D_TYPE3         		(3<<0)			// Data latch delay 4 clock
#define CLKOINV			(1<<3)			// Invert output clock
#define CSI_RGB565			(0<<4)
#define CSI_RGB1555			(1<<4)			// RGB1555 / RGB565 mode output
#define INVYUVO          		(1<<6)			// Invert output UV's bit 7
#define CLKOEN			(1<<7)			// CSI output clock enable
#define CUTEN				(1<<8)			// Screen CUT enable
#define INVYUVI			(1<<9)			// Invert input UV's bit 7
#define YONLY				(1<<10)			// Only Y output enable
#define CLK_SEL27M			(0<<11)			// 27MHz
#define CLK_SEL48M			(1<<11)			// 48MHz
#define CSI_CELL			(1<<12)
#define CSI_CELL32X32		(1<<13)
#define CSI_NOSTOP			(1<<14)
#define CSI_HIGHPRI		(1<<15)
/*
P_MD_CTRL			-	Motion Detection Control Register
*/
#define MD_EN					(1<<1)			// Enable H/W motion detection (MD)
#define MD_FRAME_1			(0<<2)			// MD detects every frame
#define MD_FRAME_2			(1<<2)			// MD detects every 2 frame
#define MD_FRAME_4			(2<<2)			// MD detects every 4 frame
#define MD_FRAME_8			(3<<2)			// MD detects every 8 frame
#define MD_VGA				(1<<4)			// VGA / QVGA size
#define MD_YUV				(1<<5)			// YUV / RGB data type
#define MD_MODE_AVE			(0<<6)			// Average mode
#define MD_MODE_SINGLE		(1<<6)			// Single mode
#define MD_MODE_AVEIIR		(2<<6)			// Average IIR mode
#define MD_MODE_SINGLEIIR		(3<<6)			// Single IIR mode
#define MD_THRESHOLD        0x1200




extern void sccb_delay (INT16U i);
extern void sccb_start (void);
extern void sccb_stop (void);
extern void sccb_w_phase (INT16U value);
extern INT16U sccb_r_phase (void);
extern void sccb_init (INT32U nSCL, INT32U nSDA);
extern void sccb_write (INT16U id, INT16U addr, INT16U data);
extern INT16U sccb_read (INT16U id, INT16U addr);
extern void CSI_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag, INT32U uFrmBuf0, INT32U uFrmBuf1);

#ifdef	__OV6680_DRV_C__
extern void OV6680_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV7680_DRV_C__
extern void OV7680_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV7670_DRV_C__
extern void OV7670_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV7725_DRV_C__
extern void OV7725_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV9655_DRV_C__
extern void OV9655_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV2655_DRV_C__
extern void OV2655_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV3640_DRV_C__
extern void OV3640_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV5642_DRV_C__
extern void OV5642_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif

extern void Sensor_Bluescreen_Enable(void);
extern void Sensor_Cut_Enable(void);
#if C_MOTION_DETECTION == CUSTOM_ON
	extern void Sensor_MotionDection_Inital(INT32U buff);
	extern void Sensor_MotionDection_stop(void);
	extern void Sensor_MotionDection_start(void);
#endif

#if 0
//sensor define
#define DRV_OV7670		0x01
#define DRV_OV3640		0x02
#define DRV_OV9712		0x03
#define DRV_SOI_H22		0x04
#define DRV_BF3703		0x05
#define DRV_GC308		0x06
#define DRV_OV5642		0x07
#define DRV_OV7725		0x08
#define SENSOR_TYPE		DRV_OV7670
#endif

#endif	/*__drv_l1_SENSOR_H__*/
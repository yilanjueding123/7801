#include "project.h"

#define CDSP_S511_BASE				0xD0800000	//GPCV1248_CDSP_BASE		//0x93001000//GP12B_CDSP_BASE		
//front control
#define	R_CDSP_FRONT_CTRL0			(*(volatile INT32U *)(CDSP_S511_BASE+0x600))
#define	R_CDSP_FLASH_WIDTH			(*(volatile INT32U *)(CDSP_S511_BASE+0x604))
#define	R_CDSP_FLASH_TRIG_NUM		(*(volatile INT32U *)(CDSP_S511_BASE+0x608))
#define	R_CDSP_SNAP_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x60C))

#define	R_CDSP_FRONT_CTRL1			(*(volatile INT32U *)(CDSP_S511_BASE+0x610))
#define	R_CDSP_HSYNC_FREDGE			(*(volatile INT32U *)(CDSP_S511_BASE+0x614))
#define	R_CDSP_VSYNC_FREDGE			(*(volatile INT32U *)(CDSP_S511_BASE+0x618))
#define	R_CDSP_FRONT_CTRL2			(*(volatile INT32U *)(CDSP_S511_BASE+0x61C))

#define	R_CDSP_FRAME_H_SETTING		(*(volatile INT32U *)(CDSP_S511_BASE+0x620))	//*1
#define	R_CDSP_FRAME_V_SETTING		(*(volatile INT32U *)(CDSP_S511_BASE+0x624))	//*1
#define	R_CDSP_TG_LINE_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x628))
#define	R_CDSP_FRAME_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x62C))

#define	R_CDSP_EXTH_SEPA			(*(volatile INT32U *)(CDSP_S511_BASE+0x630))
#define	R_CDSP_TG_LS_LINE_NUM		(*(volatile INT32U *)(CDSP_S511_BASE+0x634))
#define	R_CDSP_FRONT_CTRL3			(*(volatile INT32U *)(CDSP_S511_BASE+0x638))	//*1
#define	R_CDSP_TG_ZERO				(*(volatile INT32U *)(CDSP_S511_BASE+0x63C))	//*1   ,rFRONT_SOURCE

#define	R_CDSP_TG_GPIO_SEL_OEN		(*(volatile INT32U *)(CDSP_S511_BASE+0x640))
#define	R_CDSP_TG_GPIO_OUT_IN		(*(volatile INT32U *)(CDSP_S511_BASE+0x644))
#define	R_CDSP_TG_GPIO_REVENT		(*(volatile INT32U *)(CDSP_S511_BASE+0x648))
#define	R_CDSP_TG_GPIO_FEVENT		(*(volatile INT32U *)(CDSP_S511_BASE+0x64C))

#define	R_CDSP_MIPI_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x650))	//*1 
#define	R_CDSP_MIPI_HVOFFSET		(*(volatile INT32U *)(CDSP_S511_BASE+0x654))	//*1 
#define	R_CDSP_MIPI_HVSIZE			(*(volatile INT32U *)(CDSP_S511_BASE+0x658))	//*1 
#define	R_CDSP_FRONT_CTRL4			(*(volatile INT32U *)(CDSP_S511_BASE+0x65C))

#define	R_CDSP_SONY_SEN_DATA		(*(volatile INT32U *)(CDSP_S511_BASE+0x660))
#define	R_CDSP_FRONT_GCLK			(*(volatile INT32U *)(CDSP_S511_BASE+0x664))	//*1 
#define	R_CDSP_SEN_CTRL_SIG			(*(volatile INT32U *)(CDSP_S511_BASE+0x668))
#define	R_CDSP_FRONT_INT			(*(volatile INT32U *)(CDSP_S511_BASE+0x66C))

#define	R_CDSP_VD_RFOCC_INT			(*(volatile INT32U *)(CDSP_S511_BASE+0x670))
#define	R_CDSP_INTH_NUM				(*(volatile INT32U *)(CDSP_S511_BASE+0x674))
#define	R_CDSP_FRONT_INTEN			(*(volatile INT32U *)(CDSP_S511_BASE+0x678))
#define	R_CDSP_FRONT_VDRF_INT		(*(volatile INT32U *)(CDSP_S511_BASE+0x67C))

#define	R_CDSP_FRONT_VDRF_INTEN		(*(volatile INT32U *)(CDSP_S511_BASE+0x680))
#define	R_CDSP_SIG_GEN				(*(volatile INT32U *)(CDSP_S511_BASE+0x684))
#define	R_CDSP_FRONT_PROBE_CTRL		(*(volatile INT32U *)(CDSP_S511_BASE+0x688))
#define	R_CDSP_FRONT_DUMMY			(*(volatile INT32U *)(CDSP_S511_BASE+0x68C))

#define	R_CDSP_FPICNT				(*(volatile INT32U *)(CDSP_S511_BASE+0x690))
#define	R_CDSP_EXTRGB				(*(volatile INT32U *)(CDSP_S511_BASE+0x694))
#define	R_CDSP_MACRO_SDRAM_RW		(*(volatile INT32U *)(CDSP_S511_BASE+0x800))	//GPCV1248 Added

//front input
#define SENSOR_RAW10	0
#define SENSOR_RAW8		1
#define SENSOR_UYVY		2
#define SENSOR_YVYU		3
#define SENSOR_VYUY		4
#define SENSOR_YUYV		5
#define MIPI_RAW8		6
#define MIPI_RAW10		7
#define MIPI_UYVY		8

//YUV mode
//#define 8Y4U4V_FMT		2
#define YUYV_FMT		1

//GPIO sel oen
#define SEN_BIT			(1 << 0)
#define FLASH_CTRL_BIT	(1 << 1)
#define SCK_BIT			(1 << 2)
#define SDO_BIT			(1 << 3)
#define HD_BIT			(1 << 4)
#define VD_BIT			(1 << 5)
#define PIX_CLK_BIT		(1 << 6)
#define MCLK_BIT		(1 << 7)
#define SEN1_BIT		(1 << 8)
#define SCK1_BIT		(1 << 9)
#define SD_BIT			(1 << 10)

//TG input signal gating
#define EXT_HDI_BIT		(1 << 0)
#define EXT_VDI_BIT		(1 << 1)
#define TV_VVALIDI_BIT	(1 << 2)
#define TV_HVALIDI_BIT	(1 << 3)
#define TV_DVALIDI_BIT		(1 << 4)
#define SDI_BIT		(1 << 5)
#define SDI2_BIT		(1 << 6)
#define SCLKI_BIT		(1 << 7)
#define SCLKI2_BIT		(1 << 8)


//front control
void hwFront_Reset(
	void
);

void hwFront_SetInputFormat(
	INT8U format 
);
void hwFront_InputYuvSet(
	INT8U yuv_format,
	INT8U yuv_sub
);
void hwFront_SetFrameSize(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
);
void hwFront_SetMipiFrameSize(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
);
void 
hwFront_TGSigGatingToZero(
	INT16U tg_in_gate_zero
);

//front interrupt
void hwFront_VdRFIntEnable(
	INT8U bit
);

void hwFront_IntEnable(
	INT16U bit
);

void hwFront_SetSize(
	INT32U hsize,
	INT32U vsize
);
void hwMipi_SetSize(
	INT32U hsize,
	INT32U vsize
);


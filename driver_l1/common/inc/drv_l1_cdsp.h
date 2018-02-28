#ifndef __drv_l1_CDSP_H__
#define __drv_l1_CDSP_H__

#include "project.h"
//+++
// Motion Detect
#define MOTION_DETECT_STATUS_IDLE		0
#define MOTION_DETECT_STATUS_START		1
#define MOTION_DETECT_STATUS_STOP		2

#define MD_SENS			10
#define MD_NORMAL		32
#define	MD_SLOW			80
#define	CDSP_MD_THR		MD_NORMAL

//---
/*********************************************
MCLK IO POS
*IO_C9 is CSI_CLKO_#0 ,defaulr setting
*IO_D7 is CSI_CLKO_#1
*IO_D12 is CSI_CLKO_#2
*IO_B9 is PWN generat

MCLK Rate 24MHz
**********************************************/
#define	MCLK_IO_D7		300
#define MCLK_IO_D12		310
#define MCLK_IO_B9		320

#define	MCLK_IO_POS		MCLK_IO_B9


//TV format
#define NTSC						0x0
#define PAL							0x1

#define C_RAW_FMT					0
#define C_YUV_FMT					1

//Input Scal path
#define CDSP_PATH					0
#define	CSI_PATH					1

#define CDSP_S511_BASE				0xD0800000//GPCV1248_CDSP_BASE		//0x93001000//GP12B_CDSP_BASE
//register 0
#define R_CDSP_MACRO_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x000))	//*1
#define R_CDSP_BADPIX_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x004))
#define R_CDSP_BADPIX_CTHR			(*(volatile INT32U *)(CDSP_S511_BASE+0x008))
#define R_CDSP_YUVSP_EFFECT_OFFSET	(*(volatile INT32U *)(CDSP_S511_BASE+0x00C))

#define R_CDSP_IMG_TYPE				(*(volatile INT32U *)(CDSP_S511_BASE+0x010))	//*1
#define R_CDSP_YUVSP_EFFECT_SCALER  (*(volatile INT32U *)(CDSP_S511_BASE+0x014)) 
#define	R_CDSP_OPB_CTRL				(*(volatile INT32U *)(CDSP_S511_BASE+0x018))
#define	R_CDSP_YUVSP_EFFECT_BTHR	(*(volatile INT32U *)(CDSP_S511_BASE+0x01C))

#define	R_CDSP_OPB_TYPE				(*(volatile INT32U *)(CDSP_S511_BASE+0x020))
#define	R_CDSP_OPB_HOFFSET			(*(volatile INT32U *)(CDSP_S511_BASE+0x024))
#define	R_CDSP_OPB_VOFFSET			(*(volatile INT32U *)(CDSP_S511_BASE+0x028))

#define	R_CDSP_OPB_RAVG				(*(volatile INT32U *)(CDSP_S511_BASE+0x040))
#define	R_CDSP_OPB_GRAVG			(*(volatile INT32U *)(CDSP_S511_BASE+0x044))
#define	R_CDSP_OPB_BAVG				(*(volatile INT32U *)(CDSP_S511_BASE+0x048))
#define	R_CDSP_OPB_GBAVG			(*(volatile INT32U *)(CDSP_S511_BASE+0x04C))
#define	R_CDSP_DUMMY				(*(volatile INT32U *)(CDSP_S511_BASE+0x050))

#define	R_CDSP_LENS_CMP_CTRL		(*(volatile INT32U *)(CDSP_S511_BASE+0x080))	//*1
#define	R_CDSP_CDSP_VVALID			(*(volatile INT32U *)(CDSP_S511_BASE+0x084))
#define	R_CDSP_LENS_CMP_XOFFSET_MAP (*(volatile INT32U *)(CDSP_S511_BASE+0x088))	//*1
#define	R_CDSP_LENS_CMP_YOFFSET_MAP	(*(volatile INT32U *)(CDSP_S511_BASE+0x08C))	//*1

#define	R_CDSP_LENS_CMP_YINSTEP_MAP	(*(volatile INT32U *)(CDSP_S511_BASE+0x090))	//*1
#define	R_CDSP_HSCALE_EVEN_PVAL		(*(volatile INT32U *)(CDSP_S511_BASE+0x094))
#define	R_CDSP_IM_XCENT				(*(volatile INT32U *)(CDSP_S511_BASE+0x098))	//*1
#define	R_CDSP_IM_YCENT				(*(volatile INT32U *)(CDSP_S511_BASE+0x09C))	//*1

#define	R_CDSP_HRAW_SCLDW_FACTOR	(*(volatile INT32U *)(CDSP_S511_BASE+0x0A0))
#define	R_CDSP_HSCALE_ODD_PVAL		(*(volatile INT32U *)(CDSP_S511_BASE+0x0A4))
#define	R_CDSP_VYUV_SCLDW_FACTOR	(*(volatile INT32U *)(CDSP_S511_BASE+0x0A8))
#define	R_CDSP_VSCALE_ACC_INIT		(*(volatile INT32U *)(CDSP_S511_BASE+0x0AC))

#define	R_CDSP_HYUV_SCLDW_FACTOR	(*(volatile INT32U *)(CDSP_S511_BASE+0x0B0))
#define	R_CDSP_HSCALE_ACC_INIT		(*(volatile INT32U *)(CDSP_S511_BASE+0x0B4))
#define	R_CDSP_SCLDW_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x0B8))
#define	R_CDSP_SCALE_FACTOR_CTRL	(*(volatile INT32U *)(CDSP_S511_BASE+0x0BC))

#define	R_CDSP_WHBAL_RSETTING		(*(volatile INT32U *)(CDSP_S511_BASE+0x0C0))	//0x0C0
#define	R_CDSP_WHBAL_GRSETTING		(*(volatile INT32U *)(CDSP_S511_BASE+0x0C4))
#define	R_CDSP_WHBAL_BSETTING		(*(volatile INT32U *)(CDSP_S511_BASE+0x0C8))	//0c0C8
#define	R_CDSP_WHBAL_GBSETTING		(*(volatile INT32U *)(CDSP_S511_BASE+0x0CC))

#define	R_CDSP_YUVSPEC_MODE			(*(volatile INT32U *)(CDSP_S511_BASE+0x0D0))
#define	R_CDSP_GLOBAL_GAIN			(*(volatile INT32U *)(CDSP_S511_BASE+0x0D4))
#define	R_CDSP_IMCROP_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x0D8))
#define	R_CDSP_IMCROP_HOFFSET		(*(volatile INT32U *)(CDSP_S511_BASE+0x0DC))
#define	R_CDSP_IMCROP_HSIZE			(*(volatile INT32U *)(CDSP_S511_BASE+0x0E0))
#define	R_CDSP_IMCROP_VOFFSET		(*(volatile INT32U *)(CDSP_S511_BASE+0x0E4))
#define	R_CDSP_IMCROP_VSIZE			(*(volatile INT32U *)(CDSP_S511_BASE+0x0E8))

#define	R_CDSP_INP_DENOISE_THR		(*(volatile INT32U *)(CDSP_S511_BASE+0x0EC))

#define	R_CDSP_INP_MIRROR_CTRL		(*(volatile INT32U *)(CDSP_S511_BASE+0x0F0))
#define	R_CDSP_RGB_SPEC_ROT_MODE	(*(volatile INT32U *)(CDSP_S511_BASE+0x0F4))

#define	R_CDSP_HPF_LCOEF0			(*(volatile INT32U *)(CDSP_S511_BASE+0x100))
#define	R_CDSP_HPF_LCOEF1			(*(volatile INT32U *)(CDSP_S511_BASE+0x104))
#define	R_CDSP_HPF_LCOEF2			(*(volatile INT32U *)(CDSP_S511_BASE+0x108))
#define	R_CDSP_LH_DIV_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x10C))

#define	R_CDSP_INP_EDGE_CTRL		(*(volatile INT32U *)(CDSP_S511_BASE+0x110))
#define	R_CDSP_INP_QTHR				(*(volatile INT32U *)(CDSP_S511_BASE+0x114))
#define	R_CDSP_INP_QCNT				(*(volatile INT32U *)(CDSP_S511_BASE+0x118))
#define	R_CDSP_CC_COF0				(*(volatile INT32U *)(CDSP_S511_BASE+0x11C))

#define	R_CDSP_CC_COF1				(*(volatile INT32U *)(CDSP_S511_BASE+0x120))
#define	R_CDSP_CC_COF2				(*(volatile INT32U *)(CDSP_S511_BASE+0x124))
#define	R_CDSP_CC_COF3				(*(volatile INT32U *)(CDSP_S511_BASE+0x128))
#define	R_CDSP_CC_COF4				(*(volatile INT32U *)(CDSP_S511_BASE+0x12C))

#define	R_CDSP_RB_CLAMP_CTRL		(*(volatile INT32U *)(CDSP_S511_BASE+0x134))
#define	R_CDSP_UVSCALE_COND0		(*(volatile INT32U *)(CDSP_S511_BASE+0x138))
#define	R_CDSP_UVSCALE_COND1		(*(volatile INT32U *)(CDSP_S511_BASE+0x13C))

#define	R_CDSP_YUV_CTRL				(*(volatile INT32U *)(CDSP_S511_BASE+0x140))	//*1			
#define	R_CDSP_BIST_EN				(*(volatile INT32U *)(CDSP_S511_BASE+0x144))
#define	R_CDSP_DENOISE_SETTING		(*(volatile INT32U *)(CDSP_S511_BASE+0x148))
#define	R_CDSP_HUE_ROT_U			(*(volatile INT32U *)(CDSP_S511_BASE+0x14C))

#define	R_CDSP_HUE_ROT_V			(*(volatile INT32U *)(CDSP_S511_BASE+0x150))
#define	R_CDSP_YUV_RANGE			(*(volatile INT32U *)(CDSP_S511_BASE+0x154))	//*1
#define	R_CDSP_INTEN				(*(volatile INT32U *)(CDSP_S511_BASE+0x158))
#define	R_CDSP_GATING_CLK_CTRL		(*(volatile INT32U *)(CDSP_S511_BASE+0x15C))

#define	R_CDSP_YUV_CORING_SETTING	(*(volatile INT32U *)(CDSP_S511_BASE+0x160))	//*1
#define	R_CDSP_INT					(*(volatile INT32U *)(CDSP_S511_BASE+0x164))	//*1
#define	R_CDSP_BIST_FAIL			(*(volatile INT32U *)(CDSP_S511_BASE+0x168))
#define	R_CDSP_PROBE_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x16C))

#define	R_CDSP_EXT_BANK_SIZE		(*(volatile INT32U *)(CDSP_S511_BASE+0x170))
#define	R_CDSP_YUV_AVG_LPF_TYPE		(*(volatile INT32U *)(CDSP_S511_BASE+0x174))	//*1
#define	R_CDSP_EXT_LINE_SIZE		(*(volatile INT32U *)(CDSP_S511_BASE+0x178))
#define	R_CDSP_RST					(*(volatile INT32U *)(CDSP_S511_BASE+0x17C))	//*1

// add GPCV1248 NEW ISP
#define R_ISP_LI_HR_CTRL				(*(volatile INT32U *)(CDSP_S511_BASE+0x180))
#define R_ISP_HR_DEPIXCANCEL_THOLD		(*(volatile INT32U *)(CDSP_S511_BASE+0x184))
#define R_ISP_HR_DENOISE_THOLD			(*(volatile INT32U *)(CDSP_S511_BASE+0x188))
#define R_ISP_HR_CROSSTALK_THOLD		(*(volatile INT32U *)(CDSP_S511_BASE+0x18C))
#define R_ISP_HRR_DENOISE_CROSSTALK_WEIGHT		(*(volatile INT32U *)(CDSP_S511_BASE+0x190))
#define R_ISP_HR_LUT_CTRL				(*(volatile INT32U *)(CDSP_S511_BASE+0x194))

#define R_ISP_IMSIZE_CROSSTALK_WEIGHT	(*(volatile INT32U *)(CDSP_S511_BASE+0x198))	//from sambai new isp
#define P_CDSP_SRAM_RW					(*(volatile INT32U *)(CDSP_S511_BASE+0x800))	//*1

//register 1
#define	R_CDSP_RAW_SUBSAMPLE_SETTING	(*(volatile INT32U *)(CDSP_S511_BASE+0x200))
#define	R_CDSP_RAW_MIRROR_SETTING	(*(volatile INT32U *)(CDSP_S511_BASE+0x204))	
#define	R_CDSP_CLAMP_SETTING		(*(volatile INT32U *)(CDSP_S511_BASE+0x208))	//*1
#define	R_CDSP_RB_HSIZE				(*(volatile INT32U *)(CDSP_S511_BASE+0x20C))	//*1
#define	R_CDSP_RB_HOFFSET			(*(volatile INT32U *)(CDSP_S511_BASE+0x210))	//*1
#define	R_CDSP_RB_VSIZE				(*(volatile INT32U *)(CDSP_S511_BASE+0x214))	//*1
#define	R_CDSP_RB_VOFFSET			(*(volatile INT32U *)(CDSP_S511_BASE+0x218))	//*1
#define	R_CDSP_WDRAM_HOFFSET		(*(volatile INT32U *)(CDSP_S511_BASE+0x21C))	//*1
#define	R_CDSP_WDRAM_VOFFSET		(*(volatile INT32U *)(CDSP_S511_BASE+0x220))	//*1

#define	R_CDSP_LINE_INTERVAL		(*(volatile INT32U *)(CDSP_S511_BASE+0x224))	//*1
#define	R_CDSP_DO					(*(volatile INT32U *)(CDSP_S511_BASE+0x228))	//*1
#define	R_CDSP_TV_MODE				(*(volatile INT32U *)(CDSP_S511_BASE+0x22C))	
#define	R_CDSP_WSRAM_THR			(*(volatile INT32U *)(CDSP_S511_BASE+0x230))	//*1

#define	R_CDSP_DATA_FORMAT			(*(volatile INT32U *)(CDSP_S511_BASE+0x240))	//*1 /*YUV output format test code 
//#define	R_CDSP_TSRX_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x244))	//GPCV1248 delete
#define	R_CDSP_GINT					(*(volatile INT32U *)(CDSP_S511_BASE+0x248))
//GPCV1248 added new denoise 0x250~0x278
#define	R_Ndenoise_CTRL				(*(volatile INT32U *)(CDSP_S511_BASE+0x250))	//GPCV1248 New denoise function Control
#define	R_Ndenoise_Ledge_Set		(*(volatile INT32U *)(CDSP_S511_BASE+0x254))	//GPCV1248 New denoise function L edge Setting
#define	R_Ndenoise_Ledge_Cof0		(*(volatile INT32U *)(CDSP_S511_BASE+0x258))	//GPCV1248 New denoise function L edge cofficent
#define R_Ndenoise_Ledge_Cof1		(*(volatile INT32U *)(CDSP_S511_BASE+0x25C))	//GPCV1248 New denoise function L edge cofficent
#define	R_Ndenoise_Ledge_Cof2		(*(volatile INT32U *)(CDSP_S511_BASE+0x260))	//GPCV1248 New denoise function L edge cofficent

#define	R_CDSP_MD_CTRL				(*(volatile INT32U *)(CDSP_S511_BASE+0x270))	//GPCV1248 Motion Detection & thr
#define	R_CDSP_MD_HSIZE				(*(volatile INT32U *)(CDSP_S511_BASE+0x274))	//GPCV1248 Motion Detection HSize
#define	R_CDSP_MD_DMA_SADDR			(*(volatile INT32U *)(CDSP_S511_BASE+0x278))	//GPCV1248 Motion Detection Start Address
#define R_CDSP_MD_DIFF_DMA_SADDR	(*(volatile INT32U *)(CDSP_S511_BASE+0x27C))	//GPCV1248 Motion Detection Different Count Address

//#define	R_CDSP_YCOR_CTRL3			(*(volatile INT32U *)(CDSP_S511_BASE+0x28C))	//GPCV1248 delete
//#define	R_CDSP_YCOR_CTRL4			(*(volatile INT32U *)(CDSP_S511_BASE+0x290))	//GPCV1248 delete
//#define	R_CDSP_YCOR_CTRL5			(*(volatile INT32U *)(CDSP_S511_BASE+0x294))	//GPCV1248 delete
//#define	R_CDSP_YCOR_CTRL6			(*(volatile INT32U *)(CDSP_S511_BASE+0x298))	//GPCV1248 delete
//#define	R_CDSP_YCOR_CTRL7			(*(volatile INT32U *)(CDSP_S511_BASE+0x29C))	//GPCV1248 delete
//#define	R_CDSP_YCOR_EN				(*(volatile INT32U *)(CDSP_S511_BASE+0x2A0))	//GPCV1248 delete
//register 2	
#define	R_CDSP_DMA_YUVABUF_SADDR	(*(volatile INT32U *)(CDSP_S511_BASE+0x300))	//*1
#define	R_CDSP_DMA_YUVABUF_HVSIZE	(*(volatile INT32U *)(CDSP_S511_BASE+0x304))	//*1
#define	R_CDSP_DMA_YUVBBUF_SADDR	(*(volatile INT32U *)(CDSP_S511_BASE+0x308))	//*1
#define	R_CDSP_DMA_YUVBBUF_HVSIZE	(*(volatile INT32U *)(CDSP_S511_BASE+0x30C))	//*1

#define	R_CDSP_DMA_RAWBUF_SADDR		(*(volatile INT32U *)(CDSP_S511_BASE+0x310))	//*1	
#define	R_CDSP_DMA_RAWBUF_HVSIZE	(*(volatile INT32U *)(CDSP_S511_BASE+0x314))	//*1
#define	R_CDSP_DMA_RAWBUF_HOFFSET	(*(volatile INT32U *)(CDSP_S511_BASE+0x318))
//#define	R_CDSP_DMA_DF_HOFFSET		(*(volatile INT32U *)(CDSP_S511_BASE+0x31C))	//GPCV1248 delete
//#define	R_CDSP_DMA_DF_SADDR			(*(volatile INT32U *)(CDSP_S511_BASE+0x320))	//GPCV1248 delete
#define	R_CDSP_DMA_COFG				(*(volatile INT32U *)(CDSP_S511_BASE+0x324))	//*1

//windows setting
#define	R_CDSP_AWB_WIN_RGGAIN2		(*(volatile INT32U *)(CDSP_S511_BASE+0x400))
#define	R_CDSP_AWB_WIN_BGAIN2		(*(volatile INT32U *)(CDSP_S511_BASE+0x404))
#define	R_CDSP_AE_AWB_WIN_CTRL		(*(volatile INT32U *)(CDSP_S511_BASE+0x408))	//*1
#define	R_CDSP_AE_WIN_SIZE			(*(volatile INT32U *)(CDSP_S511_BASE+0x40C))

#define	R_CDSP_RGB_WINH_CTRL		(*(volatile INT32U *)(CDSP_S511_BASE+0x410))
#define	R_CDSP_RGB_WINV_CTRL		(*(volatile INT32U *)(CDSP_S511_BASE+0x414))
#define	R_CDSP_AE_WIN_ABUFADDR		(*(volatile INT32U *)(CDSP_S511_BASE+0x418))
#define	R_CDSP_AE_WIN_BBUFADDR		(*(volatile INT32U *)(CDSP_S511_BASE+0x41C))

#define	R_CDSP_HISTGM_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x420))	//*1
#define	R_CDSP_HISTGM_LCNT			(*(volatile INT32U *)(CDSP_S511_BASE+0x424))
#define	R_CDSP_HISTGM_HCNT			(*(volatile INT32U *)(CDSP_S511_BASE+0x428))

#define	R_CDSP_AF_WIN1_HVOFFSET		(*(volatile INT32U *)(CDSP_S511_BASE+0x42C))
#define	R_CDSP_AF_WIN1_HVSIZE		(*(volatile INT32U *)(CDSP_S511_BASE+0x430))
#define	R_CDSP_AF_WIN_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x434))	//*1
#define	R_CDSP_AF_WIN2_HVOFFSET		(*(volatile INT32U *)(CDSP_S511_BASE+0x438))
#define	R_CDSP_AF_WIN3_HVOFFSET		(*(volatile INT32U *)(CDSP_S511_BASE+0x43C))

#define	R_CDSP_AWB_WIN_CTRL			(*(volatile INT32U *)(CDSP_S511_BASE+0x454))
#define	R_CDSP_AWB_SPECWIN_Y_THR	(*(volatile INT32U *)(CDSP_S511_BASE+0x458))
#define	R_CDSP_AWB_SPECWIN_UV_THR1	(*(volatile INT32U *)(CDSP_S511_BASE+0x45C))
#define	R_CDSP_AWB_SPECWIN_UV_THR2	(*(volatile INT32U *)(CDSP_S511_BASE+0x460))
#define	R_CDSP_AWB_SPECWIN_UV_THR3	(*(volatile INT32U *)(CDSP_S511_BASE+0x464))	

#define	R_CDSP_SUM_CNT1				(*(volatile INT32U *)(CDSP_S511_BASE+0x468))
#define	R_CDSP_SUM_G1_L				(*(volatile INT32U *)(CDSP_S511_BASE+0x46C))
#define	R_CDSP_SUM_G1_H				(*(volatile INT32U *)(CDSP_S511_BASE+0x470))
#define	R_CDSP_SUM_RG1_L			(*(volatile INT32U *)(CDSP_S511_BASE+0x474))
#define	R_CDSP_SUM_RG1_H			(*(volatile INT32U *)(CDSP_S511_BASE+0x478))
#define	R_CDSP_SUM_BG1_L			(*(volatile INT32U *)(CDSP_S511_BASE+0x47C))
#define	R_CDSP_SUM_BG1_H			(*(volatile INT32U *)(CDSP_S511_BASE+0x480))

#define	R_CDSP_SUM_CNT2				(*(volatile INT32U *)(CDSP_S511_BASE+0x484))
#define	R_CDSP_SUM_G2_L				(*(volatile INT32U *)(CDSP_S511_BASE+0x488))
#define	R_CDSP_SUM_G2_H				(*(volatile INT32U *)(CDSP_S511_BASE+0x48C))
#define	R_CDSP_SUM_RG2_L			(*(volatile INT32U *)(CDSP_S511_BASE+0x490))
#define	R_CDSP_SUM_RG2_H			(*(volatile INT32U *)(CDSP_S511_BASE+0x494))
#define	R_CDSP_SUM_BG2_L			(*(volatile INT32U *)(CDSP_S511_BASE+0x498))
#define	R_CDSP_SUM_BG2_H			(*(volatile INT32U *)(CDSP_S511_BASE+0x49C))

#define	R_CDSP_SUM_CNT3				(*(volatile INT32U *)(CDSP_S511_BASE+0x4A0))
#define	R_CDSP_SUM_G3_L				(*(volatile INT32U *)(CDSP_S511_BASE+0x4A4))
#define	R_CDSP_SUM_G3_H				(*(volatile INT32U *)(CDSP_S511_BASE+0x4A8))
#define	R_CDSP_SUM_RG3_L			(*(volatile INT32U *)(CDSP_S511_BASE+0x4AC))
#define	R_CDSP_SUM_RG3_H			(*(volatile INT32U *)(CDSP_S511_BASE+0x4B0))
#define	R_CDSP_SUM_BG3_L			(*(volatile INT32U *)(CDSP_S511_BASE+0x4B4))
#define	R_CDSP_SUM_BG3_H			(*(volatile INT32U *)(CDSP_S511_BASE+0x4B8))

#define	R_CDSP_AF_WIN1_HVALUE_L		(*(volatile INT32U *)(CDSP_S511_BASE+0x4C0))
#define	R_CDSP_AF_WIN1_HVALUE_H		(*(volatile INT32U *)(CDSP_S511_BASE+0x4C4))
#define	R_CDSP_AF_WIN1_VVALUE_L		(*(volatile INT32U *)(CDSP_S511_BASE+0x4C8))
#define	R_CDSP_AF_WIN1_VVALUE_H		(*(volatile INT32U *)(CDSP_S511_BASE+0x4CC))

#define	R_CDSP_AF_WIN2_HVALUE_L		(*(volatile INT32U *)(CDSP_S511_BASE+0x4D0))
#define	R_CDSP_AF_WIN2_HVALUE_H		(*(volatile INT32U *)(CDSP_S511_BASE+0x4D4))
#define	R_CDSP_AF_WIN2_VVALUE_L		(*(volatile INT32U *)(CDSP_S511_BASE+0x4D8))
#define	R_CDSP_AF_WIN2_VVALUE_H		(*(volatile INT32U *)(CDSP_S511_BASE+0x4DC))

#define	R_CDSP_AF_WIN3_HVALUE_L		(*(volatile INT32U *)(CDSP_S511_BASE+0x4E0))
#define	R_CDSP_AF_WIN3_HVALUE_H		(*(volatile INT32U *)(CDSP_S511_BASE+0x4E4))
#define	R_CDSP_AF_WIN3_VVALUE_L		(*(volatile INT32U *)(CDSP_S511_BASE+0x4E8))
#define	R_CDSP_AF_WIN3_VVALUE_H		(*(volatile INT32U *)(CDSP_S511_BASE+0x4EC))
#define	R_CDSP_AEF_WIN_TEST			(*(volatile INT32U *)(CDSP_S511_BASE+0x4FC)) //AE/AF wintest
	
#define CDSP_TABLE_BASEADDR		0xD0800800


#define C_RAW_FMT					0
#define C_YUV_FMT					1

typedef struct cdspLensData_s 
{
	volatile INT32U  LensTable[256];     /* 0x00~0xFF */        
} cdspLensData_t;

typedef struct cdspGammaData_s 
{
	volatile INT32U  GammaTable[128];   /* 0x000~0x3FF */        
} cdspGammaData_t;

typedef struct cdspEdgeLutData_s 
{
	volatile INT32U  EdgeTable[256];   /* 0x000~0x3FF */        
} cdspEdgeLutData_t;

typedef struct cdspNewDEdgeLutData_s 
{
	volatile INT32U  NewDEdgeTable[256];   /* 0x000~0x3FF */        
} cdspNewDEdgeLutData_t;

/* New ISP */
typedef struct ispLiCorData_s 
{
	volatile INT32U  LiCorTable[48]; 
} ispLiCorData_t;

typedef struct ispHr0Data_s 
{
	volatile INT32U  Hr0Table[48]; 
} ispHr0Data_t;

#if 0
typedef struct ispMaxTan8Data_s 
{
	volatile INT32U MaxTan8[32]; 
} ispMaxTan8Data_t;

typedef struct ispslop4Data_s 
{
	volatile INT32U Slope4[16]; 
} ispslop4Data_t;

typedef struct ispCLPointData_s 
{
	volatile INT32U  CLPoint[8]; 
} ispCLPointData_t;
#else
typedef struct ispLucData_s 
{
	volatile INT32U MaxTan8[32]; 
	volatile INT32U Slope4[16]; 
	volatile INT32U CLPoint[8];
} ispLucData_t;
#endif

typedef struct ispRadiusFile0_s 
{
	volatile INT32U Radius_File_0[512]; 
} ispRadiusFile0_t;

typedef struct ispRadiusFile1_s 
{
	volatile INT32U Radius_File_1[512]; 
} ispRadiusFile1_t;

typedef struct gpCdspCorMatrix_s
{
	/* color matrix */
	unsigned char	colcorren;		/* 0:disable, 1:enable */
	unsigned char	reserved0;
	
	signed short	a11;			/* 1 sign + 3.6 bit, -512/64~511/64 */
	signed short	a12;			/* 1 sign + 3.6 bit, -512/64~511/64 */
	signed short	a13;			/* 1 sign + 3.6 bit, -512/64~511/64 */
	signed short	a21;			/* 1 sign + 3.6 bit, -512/64~511/64 */
	signed short	a22;			/* 1 sign + 3.6 bit, -512/64~511/64 */
	signed short	a23;			/* 1 sign + 3.6 bit, -512/64~511/64 */
	signed short	a31;			/* 1 sign + 3.6 bit, -512/64~511/64 */
	signed short	a32;			/* 1 sign + 3.6 bit, -512/64~511/64 */
	signed short	a33;			/* 1 sign + 3.6 bit, -512/64~511/64 */
}gpCdspCorMatrix_t;

/* define */
#ifndef VIC_CDSP
 #define VIC_CDSP		12	//GPCV1248
#endif

// cdsp int type 
#define CDSP_INT_BIT		0x1
#define FRONT_VD_INT_BIT	0x2
#define FRONT_INT_BIT		0x4

// cdsp interrupt bit
#define CDSP_AFWIN_UPDATE	(1<<0)
#define CDSP_AWBWIN_UPDATE	(1<<1)
#define CDSP_AEWIN_SEND		(1<<2)
#define CDSP_OVERFOLW		(1<<3)
#define CDSP_EOF			(1<<4)
#define CDSP_FACWR			(1<<5)
#define CDSP_FIFO			(1<<6)
#define CDSP_INT_ALL		0x37 
#define CDSP_INT_ALL_WFIFO	0x7F

// front vd interrupt bit
#define FRONT_VD_RISE		0x1
#define FRONT_VD_FALL		0x2

// front interrupt bit
#define VDR_EQU_VDRINT_NO	(1<<0)
#define VDF_EQU_VDFINT_NO	(1<<1)
#define SERIAL_DONE			(1<<2)
#define SNAP_DONE			(1<<3)
#define CLR_DO_CDSP			(1<<4)
#define EHDI_FS_EQU_INTH_NO	(1<<5)
#define FRONT_VVALID_RISE	(1<<6)
#define FRONT_VVALID_FALL	(1<<7)
#define OVERFLOW_RISE		(1<<8)
#define OVERFLOW_FALL		(1<<9)

// raw data format

// image source
#define FRONT_INPUT		0
#define SDRAM_INPUT		1
#define MIPI_INPUT		2

//yuv special mode
#define SP_YUV_NONE			0
#define SP_YUV_NEGATIVE		1
#define SP_YUV_BINARIZE		2
#define SP_YUV_YbYcSatHue	3
#define SP_YUV_EMBOSSMENT	4
#define SP_YUV_EMBOSSMENTx2	5
#define SP_YUV_EMBOSSMENTx4	6
#define SP_YUV_EMBOSSMENTx8	7

//raw spec mode
#define SP_RAW_NONE			0
#define SP_RAW_NEGATIVE		1
#define SP_RAW_SOL_ARISE	2
#define SP_RAW_EMBOSSMENT	3
#define SP_RAW_BINARIZE		4
#define SP_RAW_SEPIA		5
#define SP_RAW_BLACK_WHITE	6

//dma set
#define RD_A_WR_A		0
#define RD_A_WR_B		1
#define RD_B_WR_B		2
#define RD_B_WR_A		3
#define AUTO_SWITCH		4	

//fifo line set
#define LINE_8			0
#define LINE_16			1
#define LINE_32			2
#define LINE_64			3
#define LINE_NONE		4

//rgb path
#define RGB_NONE		0
#define RGB_PATH1		1	//From Lenscmp
#define RGB_PATH2		3	//From WBGain
#define RGB_PATH3		5	//From Lutgamma

//AF window size
#define C_AF256			0x0
#define C_AF512			0x1
#define C_AF1024		0x2
#define C_AF64			0x3
#define C_AF2048		0x4

//AE windows size
#define C_AE4			0x0
#define C_AE8			0x1
#define C_AE16			0x2
#define C_AE32			0x3
#define C_AE64			0x4
#define C_AE128			0x5

//Output Format
#define YUYV		0x0
#define UYVY 		0x1
#define YVYU 		0x2
#define	VYUY		0x3

//Algorithm Select
#define mastumoto		0x0
#define missing_pixel	0x1

//DPC RCV Mode
#define default_mode	0x1
#define second_mode		0x0

//ISP_Crosstalk removal
#define Gb			0x1
#define Gr			0x2
#define GbGr		0x3

//cdsp system
void hwCdsp_Reset(
	void
);
void hwCsdp_EnableDynGClk(
	INT8U enable
);
void hwCdsp_DataSource(
	INT8U image_source
);
void hwCdsp_RedoTriger(
	INT8U cdspdo_mode
);
void hwCdsp_SetGatingValid(
	INT8U gating_gamma_vld,
	INT8U gating_badpixob_vld
);
void hwCdsp_SetDarkSub(
	INT8U raw_sub_en,
	INT8U sony_jpg_en
);
void hwCdsp_SetRawDataFormat(
	INT8U format
);
void 
hwCdsp_SetYuvRange(
	INT8U yuv_range
);
void
hwCdsp_SetSRAM(
	INT8U overflowen,
	INT16U sramthd
);
void
hwCdsp_SetReadBackSize(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
);

void hwCdsp_SetReadBackSize10(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
);

//cdsp interrupt
void hwCdsp_SetInt(
	INT8U inten,
	INT8U bit
);
// other 
void
hwCdsp_SetLineInterval(
	INT16U line_interval
);
void
hwCdsp_SetExtLine(
	INT8U extinen,
	INT16U linesize,
	INT16U lineblank
);
//path set
void
hwCdsp_DataSource(
	INT8U image_source
);
void
hwCdsp_SetRawPath(
	INT8U raw_mode,
	INT8U cap_mode,
	INT8U yuv_mode
);
void
hwCdsp_SetYuvMuxPath(
	INT8U redoedge
);
void
hwCdsp_SetLensCmpPath(
	INT8U yuvlens
);
void
hwCdsp_SetExtLinePath(
	INT8U path
);
void
hwCdsp_SetLineCtrl(
	INT8U ybufen	//0: raw data, 1: YUV data
);

//clamp set
void
hwCdsp_SetRBClamp(
	INT8U rbclampen,
	INT16U rbclamp
);
void 
hwCdsp_EnableClamp(
	INT8U clamphsizeen,
	INT16U Clamphsize
);
void
hwCdsp_SetUVClamp(
	INT8U uvDiven,
	INT8U yfrcuvdiv1_8,
	INT8U yfrcuvdiv2_8,
	INT8U yfrcuvdiv3_8,
	INT8U yfrcuvdiv4_8,
	INT8U yfrcuvdiv5_8,
	INT8U yfrcuvdiv6_8
);
void 
hwCdsp_QcntThr(
	INT16U Qthr,
	INT16U PreRBclamp
);

//buffer set
void
hwCdsp_SetYuvBuffA(
	INT16U width,
	INT16U height,
	INT32U buffer_a_addr
);
void
hwCdsp_SetYuvBuffB(
	INT16U width,
	INT16U height,
	INT32U buffer_b_addr
);
void hwCdsp_SetRawBuff(
	INT32U width,
	INT32U height,
	INT16U hoffset,
	INT32U buffer_addr
);
void hwCdsp_SetRaw10Buff(
	INT32U width,
	INT32U height,
	INT16U hoffset,
	INT32U buffer_addr
);
void hwCdsp_SetDmaBuff(
	INT8U buffer_mode
);
//bad pixel set
void hwCdsp_EnableBadPixel(
	INT8U badpixen, 
	INT8U badpixmiren, 
	INT8U badpixmirlen
);
void hwCdsp_SetBadPixel(
	INT8U bprthr,
	INT8U bpgthr,
	INT8U bpbthr
);
//optical black set
void
hwCdsp_SetManuOB(
	INT8U manuoben,
	INT16U manuob
);
void
hwCdsp_SetAutoOB(
	INT8U autooben,
	INT8U obtype,
	INT8U curfob
);
void
hwCdsp_OBRead(
	INT32U *prvalue,
	INT32U *pgrvalue,
	INT32U *pbvalue,
	INT32U *pgbvalue
);
//lens compensation
void
hwCdsp_InitLensCmp(
	INT16U *plensdata
);
void
hwCdsp_SetLensCmp(
	INT16U centx,
	INT16U centy,
	INT16U xmoffset,
	INT8U  xminc,
	INT16U ymoffset,
	INT8U  ymoinc,
	INT8U  ymeinc
);
void
hwCdsp_EnableLensCmp(
	INT8U lcen,
	INT8U stepfactor
);
//raw h scaler
void
hwCdsp_EnableRawHScale(
	INT8U hscale_en,
	INT8U hscale_mode
);
void
hwCdsp_SetRawHScale(
	INT32U src_hsize,
	INT32U dst_hsize
);
void
hwCdsp_RgbScaleModeSet(
	INT8U Raw_H_ScaleHMode
);
//white balance set
void 
hwCdsp_EnableWbGain(
	//INT8U wboffseten,
	INT8U wbgainen
);
void hwCdsp_SetWbOffset(
	INT8U wboffseten,
	INT8U roffset,
	INT8U groffset,
	INT8U boffset,
	INT8U gboffset
);
void hwCdsp_GetWbOffset(
	INT8U *wboffseten,
	INT8S *proffset,
	INT8S *pgroffset,
	INT8S *pboffset,
	INT8S *pgboffset
);
void hwCdsp_SetWbGain(
	INT16U rgain,
	INT16U grgain,
	INT16U bgain,
	INT16U gbgain
);
void hwCdsp_GetWbGain(
	INT8U *wbgainen,
	INT16U *prgain,
	INT16U *pgrgain,
	INT16U *pbgain,
	INT16U *pgbgain
);

void hwCdsp_SetWb_R_B_Gain(
	INT16U r_gain,
	INT16U b_gain
);
//global gain
void hwCdsp_SetGlobalGain(INT8U global_gain);

void hwCdsp_GlobalGainRead(INT8U *global_gain);

//wb gain2
void 
hwCdsp_EnableWbGain2(
	INT8U wbgain2en
);
void
hwCdsp_WbGain2Set(
	INT16U rgain2,
	INT16U ggain2,
	INT16U bgain2
);
void
hwCdsp_WbGain2Read(
	INT16U *prgain,
	INT16U *pggain,
	INT16U *pbgain
);
//gamma set
void hwCdsp_InitGamma(
	INT32U *pGammaTable
);
void hwCdsp_EnableLutGamma(
	INT8U lut_gamma_en
);
//interpolation set
void
hwCdsp_IntplThrSet(
	INT8U int_low_thr,
	INT8U int_hi_thr
);
void
hwCdsp_EnableIntplMir(
	INT8U intplmiren,
	INT8U intplmirvsel,
	INT8U intplcnt2sel
);
//edge in intpolation set
void
hwCdsp_EnableEdge(
	INT8U edgeen
);
void hwCdsp_SetEdgeFilter(
	INT8U lf00,
	INT8U lf01,
	INT8U lf02,
	INT8U lf10,
	INT8U lf11,
	INT8U lf12,
	INT8U lf20,
	INT8U lf21,
	INT8U lf22 
);
void
hwCdsp_SetEdgeLCoring(
	INT8U lhdiv,
	INT8U lhtdiv,
	INT8U lhcoring,
	INT8U lhmode
);

void
hwCdsp_SetEdgeAmpga(
	INT8U ampga,
	INT8U edgedomain
);
//edge lut table set
void
hwCdsp_EnableEdgeLutTable(
	INT8U eluten
);
void
hwCdsp_InitEdgeLut(
	INT8U *pLutEdgeTable
);

void hwCdsp_NewDEdgeLut(
	INT8U *pLutNewDEdgeTable
);

//color matrix set
void
hwCdsp_EnableColorMatrix(
	INT8U colcorren
);
void
hwCdsp_SetColorMatrix(
	INT32S a11,
	INT32S a12,
	INT32S a13,
	INT32S a21,
	INT32S a22,
	INT32S a23,
	INT32S a31,
	INT32S a32,
	INT32S a33
);
//YUV insert & coring set
void hwCdsp_EnableYuv444Insert(
	INT8U yuvinserten
);
void hwCdsp_SetYuvCoring(
	INT8U y_corval_coring,
	INT8U u_corval_coring,
	INT8U v_corval_coring 
);
//crop set
void hwCdsp_SetCrop(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
);
void
hwCdsp_EnableCrop(
	INT8U hvEnable
);
void
hwCdsp_CropGet(
	INT32U *hoffset,
	INT32U *voffset,
	INT32U *hsize,
	INT32U *vsize
);
//YUV avg set
void
hwCdsp_SetYuvHAvg(
	INT8U yuvhavgmiren,
	INT8U ytype,
	INT8U utype,
	INT8U vtype
);
//YUV spec mode set
void
hwCdsp_SetYuvSpecMode(
	INT8U yuvspecmode
);
void
hwCdsp_SetYuvSpecModeBinThr(
	INT8U binarthr
);
//Y brightness & contrast set
void
hwCdsp_BriContSet(
	INT32U yb,
	INT32U yc
);
void
hwCdsp_BriContGet(
	INT32U *yb,
	INT32U *yc
);
void 
hwCdsp_EnableBriCont(
	INT8U YbYcEn
);
void 
hwCdsp_SetYuvSPEffOffset(
	INT8S y_offset,
	INT8S u_offset,
	INT8S v_offset
);
void 
hwCdsp_SetYuvSPEffScale(
	INT8U y_scale,
	INT8U u_scale,
	INT8U v_scale
);
//UV saturation & hue set
void
hwCdsp_SatHueSet(
	INT32U uvsat_scale,
	INT32U uoffset,
	INT32U voffset,
	INT32U u_huesindata,
	INT32U u_huecosdata,
	INT32U v_huesindata,
	INT32U v_huecosdata
);

void 
hwCdsp_SetYuvSPHue(
	INT16U u_huesindata,
	INT16U u_huecosdata,
	INT16U v_huesindata,
	INT16U v_huecosdata
);
//raw special mode
void
hwCdsp_SetRawSpecMode(
	INT8U rawspecmode
);
//rotate mode set
void
hwCdsp_RotateModeSet(
	INT8U rotmode
);
//yuv h & v scale down
void
hwCdsp_EnableYuvHScale(
	INT8U yuvhscale_en,
	INT8U yuvhscale_mode //0: drop, 1: filter
);
void
hwCdsp_EnableYuvVScale(
	INT8U vscale_en,
	INT8U vscale_mode //0: drop, 1: filter
);
void
hwCdsp_SetYuvHScale(
	INT16U hscaleaccinit,
	INT16U yuvhscalefactor
);
void
hwCdsp_SetYuvVScale(
	INT16U vscaleaccinit,
	INT16U yuvvscalefactor
);
//suppression
void
hwCdsp_EnableUvSuppr(
	INT8U suppressen
);
void
hwCdsp_SetUvSuppr(
	INT8U yuvsupmirvsel,
	INT8U fstextsolen,
	INT8U yuvsupmiren
);
//Y edge in uv suppression
void
hwCdsp_EnableYEdgeUvSuppr(
	INT8U posyedgeen
);

//Y denoise in suppression set
void 
hwCdsp_EnableYDenoise(
	INT8U denoisen
);
void
hwCdsp_SetYDenoise(
	INT8U denoisethrl,
	INT8U denoisethrwth,
	INT8U yhtdiv
);
//Y LPF in suppression set
void 
hwCdsp_EnableYLPF(
	INT8U lowyen
);
//af
void 
hwCdsp_EnableAF(
	INT8U af_en,
	INT8U af_win_hold
);
void
hwCdsp_SetAfWin1(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
);
void
hwCdsp_SetAfWin2(
	INT16U hoffset,	
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
);
void
hwCdsp_SetAfWin3(
	INT16U hoffset,	
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
);
//awb
void
hwCdsp_EnableAWB(
	INT8U awb_win_en,
	INT8U awb_win_hold
);
void
hwCdsp_SetAWBThr(
	INT32U Ythr,
	INT32U UVL1N1,
	INT32U UVL1N2,
	INT32U UVL1N3	
);
void
hwCdsp_SetAWB(
	INT8U awbclamp_en,
	INT8U sindata,	
	INT8U cosdata,
	INT8U awbwinthr
);

void hwCdsp_SetIntEn(INT8U enable, INT8U bit);

//ae
void
hwCdsp_SetAEAWB(
	INT8U raw_en, //0:raw, 1:rgb ae/awb window
	INT8U subample //2: 1/2, 4: 1/4 subsample
);
void
hwCdsp_EnableAE(
	INT8U ae_win_en,
	INT8U ae_win_hold
);
void 
hwCdsp_Enable3ATestWin(
	INT8U AeWinTest,
	INT8U AfWinTest
);
void
hwCdsp_SetAEWin(
	INT8U phaccfactor,
	INT8U pvaccfactor
);
void
hwCdsp_SetRGBWin(
	INT16U hwdoffset,
	INT16U vwdoffset,
	INT16U hwdsize,
	INT16U vwdsize
);
void
hwCdsp_SetAEBuffAddr(
	INT32U winaddra,
	INT32U winaddrb
);
//histgm set
void 
hwCdsp_EnableHistgm(
	INT8U his_en,
	INT8U his_hold_en
);
void
hwCdsp_SetHistgm(
	INT8U hislowthr,
	INT8U hishighthr 
);
void
hwCdsp_GetHistgm(
	INT32U *hislowcnt,
	INT32U *hishighcnt 
);


void hwCdsp_LinCorr_ReadTable(
	INT8U *rgb_tbl, 
	INT32U len
);

void hwIsp_LinCorr_Enable(
	INT8U lincorr_en
);

void hwCdsp_SetAverageOB(
	INT8U wboffseten,
	INT16U ob_avg_r,
	INT16U ob_avg_gr,
	INT16U ob_avg_b,
	INT16U ob_avg_gb
);


void hwCdsp_SetOB_HVoffset(
	INT16U obHOffset,
	INT16U obVOffset
);

void hwCdsp_SetOutYUVFmt(			
	INT8U yuv_format 
);

void hwCdsp_SetUVScale(
	INT8U uvDiven,
	INT8U yfrcuvdiv1_8,
	INT8U yfrcuvdiv2_8,
	INT8U yfrcuvdiv3_8,
	INT8U yfrcuvdiv4_8,
	INT8U yfrcuvdiv5_8,
	INT8U yfrcuvdiv6_8
);

/***************************
Motion Detection set	//by u2
****************************/

void hwCdsp_MD_set(INT8U enable, INT8U threshold, INT16U width, INT32U working_buf);
INT32U hwCdsp_MD_get_result(void);
void hwCdsp_md_enable(INT8U enable);

/***************************
New ISP Function	//by u2
****************************/
void hwIsp_SetImageSize( INT16U SNR_WIDTH, INT16U SNR_HEIGHT);
void hwIsp_dpc_en(INT8U dpc_en,INT8U algorithm_sel);	//bpc=dectect bad pixel
void hwIsp_dpc_rcv_mode_sel_thr(INT8U sel_mode, INT16U DPCth1, INT16U DPCth2,INT16U DPCth3);
void hwIsp_smooth_factor(INT16U DPCn);	//factor 64:smooth; 192:sharpen
void hwIsp_EnableCrostlkGbGr(INT8U ctk_en, INT8U ctk_gbgr);
void hwIsp_EnableCrostlk(INT8U ctk_en);
void hwIsp_CrostlkThold(INT16U ctk_thr1,INT16U ctk_thr2,INT16U ctk_thr3,INT16U ctk_thr4);
void hwIsp_CrostlkWeight(INT16U ctkw1, INT16U ctkw2, INT16U ctkw3);
void hwIsp_DenoiseThold(INT16U rdn_thr1,INT16U rdn_thr2,INT16U rdn_thr3,INT16U rdn_thr4);
void hwIsp_DenoiseWeight(INT16U rdnw1, INT16U rdnw2, INT16U rdnw3);
void hwIsp_DenoiseWeight_3(INT16U rdnw3);
void hwIsp_InitLiCor(INT8U *plicordata);
void hwIsp_Hr0(INT32U value);
void hwIsp_Hr1(INT32U value);
void hwIsp_Hr2(INT32U value);
void hwIsp_Hr3(INT32U value);
void hwIsp_luc_MaxTan8_Slop_CLP(INT16U *pmaxtan8data, INT16U *pslopdata, INT16U *pclpdata);
void hwIsp_RadusF0(INT16U *pradusf0data);
void hwIsp_RadusF1(INT16U *pradusf1data);
void hwIsp_EnableDenoise(INT8U denoise_en);
void hwCdsp_SetFIFO(INT8U frcen,INT8U fifo_en);
void hwCdsp_SetFIFO_Line(INT8U line_mode);
void hwIsp_LenCmp(INT32U lcen,INT32U stepfactor);
void hwCdsp_SetColorMatrix_Str(const INT16S *MatrixTable);

/**************************************
New Denoise From GP12B
***************************************/
void hwCdsp_EnableNewDenoise(INT8U newdenoiseen);
void hwCdsp_SetNewDenoise_Sel_Mirror(INT8U ndmirvsel, INT8U ndmiren);
void hwCdsp_EnableNdEdge(INT8U ndedgeen,INT8U ndeluten);
void hwCdsp_SetNdEdgeFilter(INT8U index,INT8U L0_SignBit,INT8U L0,INT8U L1_SignBit,INT8U L1,INT8U L2_SignBit,INT8U L2);
void hwCdsp_SetNdEdgeLCoring(INT8U ndlhdiv,	INT8U ndlhtdiv,	INT8U ndlhcoring);
void hwCdsp_SetNdEdgeAmpga(INT8U ndampga);
/**************************************
New Added AWB Config
***************************************/
void hwCdsp_SetAWBYThr(	INT8U Ythr0,	INT8U Ythr1,	INT8U Ythr2,	INT8U Ythr3);
void hwCdsp_SetAWBUVThr1(INT16S UL1N1,	INT16S UL1P1,	INT16S VL1N1,	INT16S VL1P1);
void hwCdsp_SetAWBUVThr2(INT16S UL1N2,	INT16S UL1P2,	INT16S VL1N2,	INT16S VL1P2);
void hwCdsp_SetAWBUVThr3(INT16S UL1N3,	INT16S UL1P3,	INT16S VL1N3,	INT16S VL1P3);
void hwCdsp_Scal_Source(INT32U sen_path);

void CDSPFB_CLK_Setting(void);
void SdramIF_Setting(unsigned char front_type);
void SensorIF_Setting(unsigned char front_type);
void CDSP_SensorIF_CLK(unsigned char front_type);
void MipiIF_Setting(unsigned char mipi_type, unsigned int width, unsigned int height);

void drvl1_cdsp_init(INT32U SNR_WIDTH, INT32U SNR_HEIGHT);
void drvl1_cdsp_stop(void);

void hwCdsp_EnableWbGain2_Read(INT8U *pgain2en);

void cdsp_isr(void);
void cdsp_eof_isr_register(void (*user_isr)(void));
void cdsp_ae_isr_register(void (*user_isr)(void));
void cdsp_awb_isr_register(void (*user_isr)(void));
INT32S gpHalCdspGetAwbSumCnt(
	INT8U section,
	INT32U *sumcnt 
);

INT32S gpHalCdspGetAwbSumG(
	INT8U section,
	INT32U *sumgl,
	INT32U *sumgh
);

INT32S gpHalCdspGetAwbSumRG(
	INT8U section,
	INT32U *sumrgl,
	INT32S *sumrgh
);

INT32S gpHalCdspGetAwbSumBG(
	INT8U section,
	INT32U *sumbgl,
	INT32S *sumbgh
);

INT32U gpHalCdspGetAEActBuff(void);

void gpHalCdspSetNdEdgeFilter(
	INT8U index,
	INT8U L0,
	INT8U L1,
	INT8U L2
);
void gpHalCdspSetNdEdgeFilter(
	INT8U index,
	INT8U L0,
	INT8U L1,
	INT8U L2
);
INT8U gpHalCdspGetNewDenoiseEn(void);
void gpHalCdspGetNewDenoise(
	INT8U *ndmirvsel, 
	INT8U *ndmiren
);
void gpHalCdspGetNdEdgeEn(
	INT8U *ndedgeen,
	INT8U *ndeluten
);
void gpHalCdspGetNdEdgeLCoring(
	INT8U *ndlhdiv, 
	INT8U *ndlhtdiv, 
	INT8U *ndlhcoring 
);
INT8U gpHalCdspGetNdEdgeAmpga(void);
void gpHalCdspGetNdEdgeFilter(
	INT8U index,
	INT8U *L0,
	INT8U *L1,
	INT8U *L2
);
void gpHalCdspSetEdgeQthr(INT8U Qthr);
void gpHalCdspSetEdgeFilter(
	INT8U index,
	INT8U L0,
	INT8U L1,
	INT8U L2
);
INT8U gpHalCdspGetEdgeLutTableEn(void);
INT8U gpHalCdspGetEdgeEn(void);
void gpHalCdspGetEdgeLCoring(
	INT8U *lhdiv, 
	INT8U *lhtdiv, 
	INT8U *lhcoring, 
	INT8U *lhmode
);
INT8U gpHalCdspGetEdgeAmpga(void);
INT8U gpHalCdspGetEdgeDomain(void);
INT8U gpHalCdspGetEdgeDomain(void);
INT32U gpHalCdspGetEdgeQCnt(void);

INT32U gpHalCdspGetGlbIntStatus(void);
INT32U gpHalCdspGetIntStatus(void);
void gpHalCdspGetEdgeFilter(INT8U index,INT8U *L0,INT8U *L1,INT8U *L2);
void gpHalCdspGetYuvSPHue(INT8S *u_huesindata, INT8S *u_huecosdata,	INT8S *v_huesindata, INT8S *v_huecosdata);
void gpHalCdspGetYuvSPEffOffset(INT8S *y_offset, INT8S *u_offset, INT8S *v_offset);
void gpHalCdspGetYuvSPEffScale(INT8U *y_scale, INT8U *u_scale, INT8U *v_scale);
INT8U gpHalCdspGetBriContEn(void);
INT8U hwCdsp_GetGlobalGain(void);

//+++
extern void cdsp_overflow_isr_register(void (*user_isr)(void));
//---

//+++
extern void motion_detect_status_set(INT8U statusVal);
//---
#endif //__drv_l1_CDSP_H__

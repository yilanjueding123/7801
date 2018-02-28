#ifndef __drv_l2_CDSP_H__
#define __drv_l2_CDSP_H__


//#include "drv_l1_cdsp.h"
#include "drv_l1_front.h"
#include "project.h"
#include "gp_aeawb.h"

#define OLD_EDGE	1
#define NEW_EDGE	2
#define STRONG		3
#define	NORMAL		4
#define	SOFT		5

#define CDSP_SCAL	1	//Enable = 1, Disable = 0
#define CDSP_WIDTH	1280
#define CDSP_HEIGHT 720

#define LENSCMP		0x10
#define WBGAIN		0x11
#define LUTGAMMA	0x12

extern unsigned short LensCampensation_Table[];
extern unsigned char EdgeLut_Table[];

extern INT32U OV7725_gammatable[];
extern INT8U OV7670_Gamma_TBLValue[];
extern INT8U OV7670_Edge_TBLValue[];
extern INT16U Ov7670_LensCmp_TblValue[];
extern void ov9712_group_off(void);
extern void ov9712_sensor_calibration(void);
extern int gc1004_set_exposure_time(sensor_exposure_t *si);

extern INT8U LEElut[];
extern INT8U g_nd_edge_table[];
extern INT8U LiTable_rgb[];
extern INT16U MaxTan8[];
extern INT16U Slope4[];
extern INT16U CLPoint[];
extern INT16U Radius_File_0[];
extern INT16U Radius_File_1[];
extern INT32U gammatable[];

extern const INT32S g_H22_color_matrix4gamma045_220[];

//system init
extern void CDSP_CLK_Init(void);
extern void CDSPFB_CLK_Setting(void);
extern void SensorIF_Setting(unsigned char front_type);
extern void MipiIF_Setting(	unsigned char mipi_type, unsigned int width, unsigned int height);
extern void sensor_ov9712_init(INT32U format, INT32U width, INT32U height);

void gp_isp_start(INT32U dummy_addr,INT32U gpSENSOR_WIDTH, INT32U gpSENSOR_HEIGHT);
void gp_cdsp_sharpness(INT32U edge_en, INT32U edge_mode, INT32U sel_fun);
void gp_cdsp_sensor_start(void);
void SOi_h22_sensor_calibration(void);
void gp_cdsp_drop_frame(INT8U drop_frame);
void gp_Cdsp_SetAWBYUV(const INT16S *AwbTable);
INT8U hwCdspGetAeAwbSrc(void);
INT8U hwCdspGetAeAwbSubSample(void);
void hwCdspGet3ATestWinEn(INT8U *AeWinTest, INT8U *AfWinTest);
void hwCdspGetRGBWin(INT16U *hwdoffset, INT16U *vwdoffset, INT16U *hwdsize, INT16U *vwdsize);

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef enum
{	
	MSG_CDSP_POSTPROCESS = 0x30000000,
	MSG_CDSP_SCALE_CROP,
	
	MSG_CDSP_BADPIX_OB,
	MSG_CDSP_LENS_CMP,
	MSG_CDSP_WBGAIN,
	MSG_CDSP_LUT_GAMMA,
	MSG_CDSP_INTERPOLATION,
	MSG_CDSP_RAW_SPEF,
	MSG_CDSP_EDGE,
	MSG_CDSP_COLOR_MATRIX,
	MSG_CDSP_POSWB_RGB2YUV,

	MSG_CDSP_YUV_INSERT,
	MSG_CDSP_YUV_HAVG,
	MSG_CDSP_SPEC_MODE,
	MSG_CDSP_SAT_HUE,
	MSG_CDSP_SUPPRESSION,
	MSG_CDSP_NEWDENOISE,

	MSG_CDSP_RAW_WIN,
	MSG_CDSP_AE_WIN,
	MSG_CDSP_AF_WIN,
	MSG_CDSP_AWB_WIN,
	MSG_CDSP_WBGAIN2,
	MSG_CDSP_HISTGM,

	
	MSG_CDSP_TARGET_AE,
	MSG_CDSP_WB_MODE,
	MSG_CDSP_EV,
	MSG_CDSP_ISO,
	
	MSG_CDSP_3A_STATISTIC,
	MSG_CDSP_MAX
}CDSP_MSG_CTRL_ID;

typedef struct gpCdspPostProcess_s 
{
	unsigned int SetFlag;
	unsigned int inFmt;
	unsigned int yuvRange;
	unsigned int width;
	unsigned int height;
	unsigned int inAddr;
	unsigned int outAddr;
}gpCdspPostProcess_t;

typedef struct gpCdspScalePara_s 
{
	/* raw hscale */
	unsigned char 	hscale_en;		/* 0:disable, 1:enable */ 		
	unsigned char 	hscale_mode;	/* 0:drop, 1:filter */ 
	unsigned short	dst_hsize;		/* 0~0xFFF */

	/* crop function */ 
	unsigned int	crop_en;		/* 0:disable, 1:enable */ 
	unsigned short	crop_hoffset;	/* 1~0xFFF */
	unsigned short	crop_voffset;	/* 1~0xFFF */
	unsigned short	crop_hsize;		/* 1~0xFFF */	
	unsigned short	crop_vsize;		/* 1~0xFFF */
	
	/* yuv h/v scale down */
	unsigned char	yuvhscale_en;	/* 0:disable, 1:enable */ 
	unsigned char	yuvvscale_en;	/* 0:disable, 1:enable */ 
	unsigned char	yuvhscale_mode; /* 0:drop, 1:filter */ 
	unsigned char	yuvvscale_mode; /* 0:drop, 1:filter */ 
	unsigned short	yuv_dst_hsize;	/* 0~0xFFF */
	unsigned short	yuv_dst_vsize;	/* 0~0xFFF */

	/* read back size */
	unsigned short	img_rb_h_size;		/* 0~0xFFF */
	unsigned short	img_rb_v_size;		/* 0~0xFFF */
}gpCdspScalePara_t;

typedef struct gpCdspBadPixOB_s
{
	/* bad pixel */
	unsigned char   badpixen; 		/* 0:disable, 1:enable */
	unsigned char   badpixmirr;        /* badpixmirlen[0]: Enable badpix mirror 2 pixels, badpixmirren[1]: Enable badpix right mirror 2 pixels*/
	unsigned short	bprthr;			/* 0~1023 */
	unsigned short	bpgthr;			/* 0~1023 */
	unsigned short	bpbthr;			/* 0~1023 */
	
	/* optical black */
	unsigned char	manuoben;		/* 0:disable, 1:enable */
	unsigned char   reserved1;
	signed short	manuob;			/* -1024~1023 */
	
	unsigned char   reserved2;
	unsigned char   reserved3;
	unsigned char	autooben;		/* 0:disable, 1:enable */
	unsigned char	obtype;			/* 0:2x256,1:4x256,2:8x256,3:16x256,4:256x2,5:256x4,6:256x8,7:256x16 */
	
	unsigned short	obHOffset;		/* 0~0xFFF */
	unsigned short	obVOffset;		/* 0~0xFFF */
	unsigned short	Ravg;			/* read back auto ob r avg */ 
	unsigned short	GRavg;			/* read back auto ob gr avg */
	unsigned short	Bavg;			/* read back auto ob b avg */
	unsigned short	GBavg;			/* read back auto ob gb avg */
	
	/* white balance */
	unsigned char	wboffseten;		/* 0:disable, 1:enable */
	signed char		roffset;		/* -128 ~ 127 */
	signed char		groffset;		/* -128 ~ 127 */
	signed char		boffset;		/* -128 ~ 127 */
	signed char		gboffset;		/* -128 ~ 127 */
}gpCdspBadPixOB_t;

typedef struct gpCdspLenCmp_s
{
	/* lens compensation */
	unsigned int	lcen;			/* 0:disable, 1:enable */
	unsigned short  *lenscmp_table; /* lenscmp_table, size 512 byte */
	
	unsigned char	stepfactor;		/* 0:2^6,1:2^7,2:2^8,3:2^9,4:2^10,5:2^11,6:2^12,7:2^13 */
	unsigned char	xminc;			/* 0~0xF */
	unsigned char  	ymoinc;			/* 0~0xF */
	unsigned char  	ymeinc;			/* 0~0xF */

	unsigned short	centx;			/* 0~0x1FFF */	
	unsigned short	centy;			/* 0~0x1FFF */
	unsigned short	xmoffset;		/* 0~0xFFF */
	unsigned short	ymoffset;		/* 0~0xFFF */
	unsigned short	seg_R;			/* 0~0xFFF */
}gpCdspLenCmp_t;

typedef struct gpCdspWhtBal_s
{
	/* white balance */
	unsigned char	wboffseten;		/* 0:disable, 1:enable */
	signed char		roffset;		/* -128 ~ 127 */
	signed char		groffset;		/* -128 ~ 127 */
	signed char		boffset;		/* -128 ~ 127 */
	signed char		gboffset;		/* -128 ~ 127 */
	
	unsigned char	wbgainen;		/* 0:disable, 1:enable */
	unsigned char	global_gain;	/* 0~0xFF */
	unsigned char 	reserved;
	
	unsigned short	rgain;			/* 3.6 bit, 0/64~511/64 */
	unsigned short	grgain;			/* 3.6 bit, 0/64~511/64 */
	unsigned short	bgain;			/* 3.6 bit, 0/64~511/64 */
	unsigned short	gbgain;			/* 3.6 bit, 0/64~511/64 */
}gpCdspWhtBal_t;

typedef struct gpCdspIntpl_s
{
	/* interpolation */
	unsigned char	int_low_thr;	/* 0~0xFF */	
	unsigned char	int_hi_thr;		/* 0~0xFF */
	/* raw special mode */
	unsigned char	rawspecmode;	/* 0~6 */
	unsigned char	reserved;
}gpCdspIntpl_t;

typedef struct gpCdspEdge_s
{
	/* edge lut table */
	unsigned char	eluten;			/* 0:disable, 1:enable */
	unsigned char 	*edge_table;	/* edge table, 256byte */

	/* edge enhance */
	unsigned char	edgeen;			/* 0:disable, 1:enable */
	unsigned char	lhdiv;			/* 1/lhdiv,  1, 2, 4, 8, 16, 32, 64, 128 */
	unsigned char	lhtdiv;			/* 1/lhtdiv,  1, 2, 4, 8, 16, 32, 64, 128 */
	
	unsigned char	lhcoring;		/* 0~0xFF */
	unsigned char	ampga;			/* 1, 2, 3, 4 */
	unsigned char	edgedomain;		/* 0:add y value, 1:add RGB value */
	unsigned char	lhmode;			/* 0:USE HPF LF00 matrix, 1:default matrix */ 

	unsigned char	Qthr;			/* edge thr set */
	/* edge filter */
	unsigned char	lf00;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	lf01;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	lf02;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */

	unsigned char	lf10;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	lf11;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	lf12;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	lf20;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */

	unsigned char	lf21;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	lf22;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	reserved0;
	unsigned char	reserved1;

	unsigned int 	Qcnt;			/* read back edge thr count */
}gpCdspEdge_t;

#if 0
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
#endif

typedef struct gpCdspRgb2Yuv_s
{
	/* pre rb clamp */
	unsigned char  	pre_rbclamp;
	/* rb clamp */
	unsigned char  	rbclampen;
	unsigned short 	rbclamp;
	
	/* Uv divider */
	unsigned char	uvdiven;		/* 0:disable, 1:enable */
	unsigned char 	reserved;
	unsigned char	Yvalue_T1;		/* 0~0xFF, when Y=T1, UV divide 1/8 */
	unsigned char	Yvalue_T2;		/* 0~0xFF, when Y=T2, UV divide 2/8 */
	unsigned char	Yvalue_T3;		/* 0~0xFF, when Y=T3, UV divide 3/8 */
	unsigned char	Yvalue_T4;		/* 0~0xFF, when Y=T4, UV divide 4/8 */
	unsigned char	Yvalue_T5;		/* 0~0xFF, when Y=T5, UV divide 5/8 */
	unsigned char	Yvalue_T6;		/* 0~0xFF, when Y=T6, UV divide 6/8 */
}gpCdspRgb2Yuv_t;

typedef struct gpCdspGamma_s
{
	unsigned int	lut_gamma_en;	/* 0:disable, 1:enable */
	unsigned int 	*gamma_table;	/* gamma table, 512 byte */ 
}gpCdspGamma_t;

typedef struct gpCdspSpecMod_s
{
	/* yuv special mode */
	unsigned char	yuvspecmode;	/* 0~7 */
	unsigned char	binarthr;		/* 0~0xFF */
	unsigned char	reserved0;
	unsigned char	reserved1;
}gpCdspSpecMod_t;

typedef struct gpCdspYuvInsert_s
{
	/* yuv 422 to 444 */
	unsigned char	yuv444_insert;	/* 0:disable, 1:enable */
	/* yuv coring threshold */
	unsigned char	y_coring;		/* 0~0xF */
	unsigned char	u_coring;		/* 0~0xF */
	unsigned char	v_coring;		/* 0~0xF */
	/* yuv coring subtraction */
	unsigned char	y_corval;		/* 0~0xF */
	unsigned char	u_corval;		/* 0~0xF */
	unsigned char	v_corval;		/* 0~0xF */
	unsigned char	reserved;
}gpCdspYuvInsert_t;

typedef struct gpCdspYuvHAvg_s
{
	/* h average */
	unsigned char	ytype;			/* 0:disable, 1:3tap, 2:5tap */
	unsigned char	utype;			/* 0:disable, 1:3tap, 2:5tap */
	unsigned char	vtype;			/* 0:disable, 1:3tap, 2:5tap */
	unsigned char   reserved;
}gpCdspYuvHAvg_t;

typedef struct gpCdspSatHue_s
{
	unsigned char	YbYcEn;			/* 0:disable, 1:enable */
	/* brightness & constract */
	signed char		y_offset;		/* -128 ~ 127 */
	signed char		u_offset;		/* -128 ~ 127 */
	signed char		v_offset;		/* -128 ~ 127 */
	
	unsigned char	y_scale;		/* 3.5 bit, 0/32~255/32 */	
	unsigned char	u_scale;		/* 3.5 bit, 0/32~255/32 */
	unsigned char	v_scale;		/* 3.5 bit, 0/32~255/32 */
	unsigned char	reserved;
	
	/* saturation, hue */
	signed char		u_huesindata;	/* 1 sign + 1.6 bit , -128/64~127/64 */
	signed char		u_huecosdata;	/* 1 sign + 1.6 bit , -128/64~127/64 */
	signed char		v_huesindata;	/* 1 sign + 1.6 bit , -128/64~127/64 */
	signed char		v_huecosdata;	/* 1 sign + 1.6 bit , -128/64~127/64 */
}gpCdspSatHue_t;

typedef struct gpCdspSuppression_s
{
	/* suppression */
	unsigned char	suppressen;		/* 0:disable, 1:enable */
	unsigned char   suppr_mode;		/* 0:edge, 1:denoise, 2:YLPF */
	
	/* y denoise */
	unsigned char 	denoisen;		/* 0:disable, 1:enable */
	unsigned char	denoisethrl;	/* 0~0xFF */
	unsigned char	denoisethrwth;	/* 1, 2, 4, 8, 16, 32, 64, 128 */
	unsigned char	yhtdiv;			/* 1, 2, 4, 8, 16, 32, 64, 128 */

	/* y lpf */
	unsigned char	lowyen;			/* 0:disable, 1:enable */
	unsigned char   reserved;
}gpCdspSuppression_t;

typedef struct gpCdspRawWin_s
{
	unsigned char	aeawb_src;		/* 0:from poswb, 1:form awb line ctrl */
	unsigned char	subsample; 		/* 0:disable, 1:1/2, 2:1/4 subsample */
	unsigned char 	AeWinTest;		/* 0:disable, 1:enable */
	unsigned char 	AfWinTest;		/* 0:disable, 1:enable */
	/* rgb window set */
	unsigned short	hwdoffset;		/* 0~0x3FFF */
	unsigned short	vwdoffset;		/* 0~0x3FFF */
	unsigned short	hwdsize;		/* 0~0x1FF, h cell size */
	unsigned short	vwdsize;		/* 0~0x1FF, v cell size */
}gpCdspRawWin_t;

typedef struct gpCdspAF_s
{
	/* auto focuse */
	unsigned char	af_win_en;		/* 0:disable, 1:enable */
	unsigned char	af_win_hold;	/* 0:disable, 1:enable */
	unsigned char	reserved00;
	unsigned char	reserved01;
	/* af window 1 */
	unsigned short	af1_hoffset;	/* 0~0xFFF */
	unsigned short 	af1_voffset;	/* 0~0xFFF */
	unsigned short 	af1_hsize;		/* 0~0xFFF */
	unsigned short	af1_vsize;		/* 0~0xFFF */
	/* af window 2 */
	unsigned short	af2_hoffset;	/* 0~0xFFF */
	unsigned short 	af2_voffset;	/* 0~0xFFF */
	unsigned short 	af2_hsize;		/* 64, 256, 512, 1024, 2048 */
	unsigned short	af2_vsize;		/* 64, 256, 512, 1024, 2048 */
	/* af window 3 */
	unsigned short	af3_hoffset;	/* 0~0xFFF */
	unsigned short 	af3_voffset;	/* 0~0xFFF */
	unsigned short 	af3_hsize;		/* 64, 256, 512, 1024, 2048 */
	unsigned short	af3_vsize;		/* 64, 256, 512, 1024, 2048 */
}gpCdspAF_t;

typedef struct gpCdspAE_s
{
	/* auto expore */
	unsigned char	ae_win_en;		/* 0:disable, 1:enable */
	unsigned char	ae_win_hold;	/* 0:disable, 1:enable */
	unsigned char	phaccfactor;		/* 4, 8, 16, 32, 64, 128 */
	unsigned char	pvaccfactor;		/* 4, 8, 16, 32, 64, 128 */
	int ae_meter;
}gpCdspAE_t;

typedef struct gpCdspAWB_s
{
	/* auto white balance */
	unsigned char	awb_win_en;		/* 0:disable, 1:enable */
	unsigned char	awb_win_hold;	/* 0:disable, 1:enable */
	unsigned char	awbclamp_en;	/* 0:disable, 1:enable */
	unsigned char	awbwinthr;		/* 0~0xFF */
	unsigned char	sindata;		/* 1 sign + 1.6bits, -128/64~127/64 */
	unsigned char	cosdata;		/* 1 sign + 1.6bits, -128/64~127/64 */
	unsigned char	reserved0;
	unsigned char	reserved1;
	
	unsigned char	Ythr0;			/* 0~0xFF */
	unsigned char	Ythr1;			/* 0~0xFF */
	unsigned char	Ythr2;			/* 0~0xFF */
	unsigned char	Ythr3;			/* 0~0xFF */
	
	signed short	UL1N1;			/* -256 ~ 255 */
	signed short	UL1P1;			/* -256 ~ 255 */
	signed short	VL1N1;			/* -256 ~ 255 */
	signed short	VL1P1;			/* -256 ~ 255 */
	
	signed short	UL1N2;			/* -256 ~ 255 */
	signed short	UL1P2;			/* -256 ~ 255 */
	signed short	VL1N2;			/* -256 ~ 255 */
	signed short	VL1P2;			/* -256 ~ 255 */

	signed short	UL1N3;			/* -256 ~ 255 */
	signed short	UL1P3;			/* -256 ~ 255 */
	signed short	VL1N3;			/* -256 ~ 255 */
	signed short	VL1P3;			/* -256 ~ 255 */

}gpCdspAWB_t;

typedef struct gpCdspWbGain2_s
{
	/* wb gain2 */
	unsigned char	wbgain2en;		/* 0:disable, 1:enable */
	unsigned char	reserved;
	unsigned short	rgain2;			/* 0~0x1FF */
	unsigned short	ggain2;			/* 0~0x1FF */
	unsigned short	bgain2;			/* 0~0x1FF */
}gpCdspWbGain2_t;

typedef struct gpCdspHistgm_s
{
	/* histgm */
	unsigned char	his_en;			/* 0:disable, 1:enable */
	unsigned char	his_hold_en;	/* 0:disable, 1:enable */
	unsigned char	hislowthr;		/* 0~0xFF */
	unsigned char	hishighthr;		/* 0~0xFF */

	unsigned int 	hislowcnt;		/* read back result low count */
	unsigned int 	hishicnt;		/* read back result hight count */
}gpCdspHistgm_t;

typedef struct gpCdsp3aResult_s
{
	/* ae */
	unsigned char ae_win[64];

	/* af */
	unsigned long long 	af1_h_value;
	unsigned long long 	af1_v_value;
	unsigned long long 	af2_h_value;
	unsigned long long 	af2_v_value;
	unsigned long long 	af3_h_value;
	unsigned long long 	af3_v_value;

	/* awb */
	AWB_RESULT_t awb_result;

	/* histgm */
	unsigned int hislowcnt;	/* histgm low count */
	unsigned int hishicnt; /* histgm hight count */	
}gpCdsp3aResult_t;

typedef struct gpCdspNewDenoise_s
{
	/* new denoise enhance */
	unsigned char	newdenoiseen;	/* 0:disable, 1:enable */
	unsigned char	ndmiren;		/* 0~0xF */
	unsigned char	ndmirvsel;		/* 0:cnt3eq1, 1:cnt3eq2 */
	/* edge enhance */
	unsigned char	ndedgeen;		/* 0:disable, 1:enable */
	/* edge lut table */
	unsigned char	ndeluten;		/* 0:disable, 1:enable */	
	unsigned char	ndlhdiv;			/* 1/lhdiv,  1, 2, 4, 8, 16, 32, 64, 128 */
	unsigned char	ndlhtdiv;		/* 1/lhtdiv,  1, 2, 4, 8, 16, 32, 64, 128 */	
	unsigned char	ndlhcoring;		/* 0~0xFF */
	unsigned char	ndampga;		/* 1, 2, 3, 4 */
	unsigned char	ndlhmode;		/* 0:USE HPF LF00 matrix, 1:default matrix */ 
	
	/* edge filter */
	unsigned char	ndlf00;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	ndlf01;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	ndlf02;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	ndlf10;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	ndlf11;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	ndlf12;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	ndlf20;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	ndlf21;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	ndlf22;			/* [3]:0:positive, 1:negtive + [2:0]:0:0,1:1,2:2,3:3,4:4,5:8,6:16 */
	unsigned char	reserved0;
}gpCdspNewDenoise_t;

typedef struct gpIspHybridRaw_s                                                                      
{                                                                                                    
    unsigned int IspHeight;
    unsigned int ISPWidth;
    
    /* new denoise enhance */                                                                                                                 
	unsigned char DefectPixelSel;
	unsigned char DefectPixelEnable;
	unsigned char DenoiseEnable; 
   	unsigned char CrosstalkEnable; 
    	
   	unsigned char DPCth1; 
   	unsigned char DPCth2; 
   	unsigned char DPCth3; 
    	
   	unsigned char DPCDefaultMode; 
   	unsigned char DPCn; 
    	
   	unsigned char DenoiseT1; 
   	unsigned char DenoiseT2; 
   	unsigned char DenoiseT3; 
   	unsigned char DenoiseT4; 
   	unsigned char DenoiseW1; 
   	unsigned char DenoiseW2; 
   	unsigned char DenoiseW3; 
    	
   	unsigned char CrosstalkT1; 
   	unsigned char CrosstalkT2; 
   	unsigned char CrosstalkT3; 
   	unsigned char CrosstalkT4; 
   	unsigned char CrosstalkW1;
   	unsigned char CrosstalkW2; 
   	unsigned char CrosstalkW3; 
	unsigned char CrosstalkGbGr;                            
}gpIspHybridRaw_t;                                                                                   
     

/**
 * enum irqreturn
 * @IRQ_NONE		interrupt was not from this device
 * @IRQ_HANDLED		interrupt was handled by this device
 * @IRQ_WAKE_THREAD	handler requests to wake the handler thread
 */
enum irqreturn {
	IRQ_NONE,
	IRQ_HANDLED,
	IRQ_WAKE_THREAD
};

typedef enum irqreturn irqreturn_t;

void gpCdspSetNewDenoise(gpCdspNewDenoise_t *argp);
void gpCdspGetNewDenoise(gpCdspNewDenoise_t *argp);
void gpCdspSetEdge(unsigned char raw_flag,gpCdspEdge_t *argp);
void gpCdspGetEdge(gpCdspEdge_t *argp);
void gpCdspSetSatHue(gpCdspSatHue_t *argp);
void gpCdspGetSatHue(gpCdspSatHue_t *argp);
void gpCdspSetSpecialMode(gpCdspSpecMod_t *argp);
void gp_cdsp_crop(INT8U crop_en, INT32U crop_width, INT32U crop_height, INT32U frame_width, INT32U frame_height);
void gp_cdsp_yuv_scaler (INT8U scaler_en, INT32U scaler_width, INT32U scaler_height, INT32U frame_width, INT32U frame_height);
void gp_cdsp_set_ev_val(INT8U ev);
void gp_cdsp_set_edge(INT32U sharpness);
//void gp_isp_set_hybridraw(gpIspHybridRaw_t *argp);
void gc1004_sensor_calibration(void);

#endif	/*__drv_l2_CDSP_H__*/

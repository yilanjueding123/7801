#include "drv_l1_cdsp.h"
#include "drv_l1_front.h"
#include "drv_l2_cdsp.h"
#include "drv_l1_i2c.h"
#include "drv_l1_csi.h"
#include "drv_l2_sensor.h"
#include "gplib.h"
#include "gp_aeawb.h"

#include "ap_state_handling.h"
/*
#if CDSP_IQ == 3 || CDSP_IQ == 4
#include "cdsp_cfg_iqp.h"
#elif CDSP_IQ == 5
#include "cdsp_cfg_iqw.h"
#elif CDSP_IQ == 2
#include "cdsp_cfg_iqj.h"
#else
#include "cdsp_cfg.h"
#endif 
*/
#include "cdsp_cfg_iqj.h"

//#include "my_video_codec_callback.h"

extern volatile INT8U cdsp_eof_occur_flag;


extern void CDSP_SensorIF_CLK(unsigned char front_type);
extern void gpCdspSetBadPixOb(gpCdspBadPixOB_t *argp);

extern void sccb_delay(INT16U i);
extern INT8U ap_state_config_sharpness_get(void);
extern INT8U ap_state_config_white_balance_get(void);
extern INT8U ap_state_config_ev_get(void);
extern INT8U ap_state_config_iso_get(void);
extern INT8U ap_state_config_light_freq_get(void);
extern void hwFront_SetSize(INT32U hsize,INT32U vsize);

extern OS_EVENT* my_AVIEncodeApQ;

#if  (USE_SENSOR_NAME == SENSOR_SOI_H22)	
	extern sensor_exposure_t *jxh22_get_senInfo(void);
	//extern void jxh22_get_senInfo2(sensor_exposure_t *pSensorInfo) ;
	extern sensor_calibration_t *jxh22_get_calibration(void);
	extern int H22_set_exposure_time(sensor_exposure_t *si);
	extern void H22_set_exposure_gain(void);
	extern void H22_get_exposure_time(sensor_exposure_t *se);
	extern void sensor_SOi_h22_init(INT32U WIDTH, INT32U HEIGHT);
	
	extern int H22_get_night_ev_idx(void);
	extern int H22_get_max_ev_idx(void);
	extern void H22_set_exp_freq(int freq);		
#elif (USE_SENSOR_NAME == SENSOR_GC1004)
	extern sensor_exposure_t *gc1004_get_senInfo(void);
	//extern void gc1004_get_senInfo2(sensor_exposure_t *pSensorInfo) ;
	extern sensor_calibration_t *gc1004_get_calibration(void);
	extern int gc1004_set_exposure_time(sensor_exposure_t *si);
	extern void gc1004_set_exposure_gain(void);
	extern void gc1004_get_exposure_time(sensor_exposure_t *se);
	extern void sensor_gc1004_init(INT32U WIDTH, INT32U HEIGHT);
	
	extern int gc1004_get_night_ev_idx(void);
	extern int gc1004_get_max_ev_idx(void);
	extern void gc1004_set_exp_freq(int freq);
#else	//SENSOR_OV9712
	extern sensor_exposure_t *ov9712_get_senInfo(void);
	//extern void ov9712_get_senInfo2(sensor_exposure_t *pSensorInfo) ;
	extern sensor_calibration_t *ov9712_get_calibration(void);
	extern int ov9712_set_exposure_time(sensor_exposure_t *si);
	extern void ov9712_set_exposure_gain(void);
	extern void ov9712_get_exposure_time(sensor_exposure_t *se);
	extern void sensor_ov9712_init(INT32U format, INT32U width, INT32U height);
	
	extern int ov9712_get_night_ev_idx(void);
	extern int ov9712_get_max_ev_idx(void);
	extern void ov9712_set_exp_freq(int freq);
#endif
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

typedef enum {
	DENOISE_DISABLE = 0,
	NEW_DENOISE_ONLY,
	HYBRID_DENOISE_ONLY, 
	BOTH_DENOISE	
} DENOISE_LEVLE;




#define C_BUFFER_MAX		5

#define CDSP_AE_CTRL_EN		0x01
#define CDSP_AWB_CTRL_EN	0x02
#define CDSP_AF_CTRL_EN		0x04
#define CDSP_HIST_CTRL_EN	0x08
#define CDSP_LUM_STATUS		0x10
#define CDSP_LOW_LUM		0x20
#define CDSP_HIGH_LUM		0x40
#define CDSP_AWB_SET_GAIN	0x80
#define CDSP_SAT_SWITCH		0x100

#define	WEAK_DENOISE		0x03
#define MEDIUM_DENOISE		0x09
#define STRONG_DENOISE		0x11

#define HYBRID_DENOISE_LEVEL	WEAK_DENOISE

#define CDSP_DEBUG  0
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct gpCdspDev_s
{
	unsigned int cdsp_feint_flag;
	
	unsigned int OpenCnt;	/* open count */
	unsigned int SyncFlag;	/* Cdsp Sync Flag */
	unsigned int MclkEn;	/* mclk enable flag */
	unsigned int CapCnt;	/* capture count */
	
	/*Sub device*/
	unsigned int sdidx;	 
	//struct v4l2_subdev *sd;
	//callbackfunc_t *cb_func;
	char *port;

	/*buffer control*/
	unsigned int bfidx;
	unsigned int bfaddr[C_BUFFER_MAX];
	unsigned char in_que[C_BUFFER_MAX];
	unsigned char out_que[C_BUFFER_MAX];
	
	/* AE buffer */
	unsigned char *aeAddr[2];

	/* format set */	
	unsigned int imgSrc; /* image input source, sdram, front, mipi */
	unsigned int inFmt; /* input format */
	unsigned int rawFmtFlag; /* raw Format Flag */
	unsigned int rawFmt; /* raw Format Flag */
	unsigned short imgWidth; /* image/csi h size */
	unsigned short imgHeight; /* image/csi v size */
	unsigned short imgRbWidth; /* output buffer h size */
	unsigned short imgRbHeight; /* output buffer v size */

	/* cdsp module set */	
	gpCdspScalePara_t scale; /* scale down set */
	gpCdspSuppression_t suppression; /* suppression mode */	
	gpCdsp3aResult_t result;

	sensor_calibration_t sensor_cdsp;

	/* cdsp 2A */
	//wait_queue_head_t ae_awb_wait_queue;
	//struct task_struct *ae_awb_task;
	unsigned int ae_awb_flag;
	int low_lum_switch_cnt, high_lum_switch_cnt;

	int getSensorInfo;
	int sensor_gain_thr, sensor_time_thr;
	int night_gain_thr;
	//int y_offset;
	
	unsigned char *ae_workmem, *awb_workmem;
	unsigned short ae_win_tmp[64];

	AWB_MODE awbmode;

	INT8S ae_ret;
	INT8S awb_ret_flag;
	
	int awb_low_lum_cnt;
	
	
	int y_offset, u_offset, v_offset;
	int y_scale, u_scale, v_scale;
	int sat_contr_idx;
	
}gpCdspDev_t;

static gpCdspDev_t gp_CdspDev;
static gpCdspDev_t *CdspDev = NULL;
sensor_exposure_t *p_seInfo, seInfo, seInfo2;
static gpIspHybridRaw_t IspHybridRaw;

gpCdspWhtBal_t wb_gain;
gpCdspWbGain2_t wb_gain2;

static INT32U denoise_level = NEW_DENOISE_ONLY;//DENOISE_DISABLE;
static INT32U ae_frame=0;
static INT32U ae_frame_thr = 3;
static INT32U ae_switch_flag = 0;
static INT32U awb_frame=0;
unsigned char *awb = NULL;
unsigned char *ae = NULL;

static int ae_exp_set_cnt = 0;

static INT32U ae_ev = 6;

INT8U cdsp_ae_addr_a[64], cdsp_ae_addr_b[64];
INT8U ae_workszie[1024], awb_worksize[1024];

// add by Comi 20140924
static int sat_yuv_level[4][6]; // [6] ==> y_offset, u_offset, v_offset, y_scale, u_scale, v_scale
static int sat_yuv_thr[4];

static int uv_div[4][6] = 
{
/*	{16, 24, 32, 40, 48, 56},
	{24, 32, 40, 48, 56, 64},
	{32, 40, 48, 56, 64, 72},
	{40, 48, 56, 64, 72, 80}*/
	
	{ 4,  8, 12, 16, 20, 24},
	{ 8, 12, 16, 20, 24, 32},
	{12, 16, 20, 24, 32, 36},
	{16, 20, 24, 32, 36, 40},	
	//{20, 24, 32, 36, 40, 48},
	//{24, 32, 40, 48, 56, 64}
};



/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/

/**
* @brief	cdsp old & new denoise edge enable
* @ex		hwCdsp_SetEdgeFilter(lf00,lf01,lf02,lf10,lf11,lf12,lf20,lf21,lf22);	
* @ex		0x9,0x0,0x9,0x0,0x4,0x0,0x9,0x0,0x9*ftr_ration = 0xA,0x0,0xA,0x0,0x8,0xA,0x0,0xA
* @param	ftr_ratio=2 is mean default value*2
* @param	sel_fun: select old edage or new edage
* @return	none
*/
void gpCdspSetAWB(gpCdspAWB_t *argp)
{
	if(argp->awb_win_en) {
	hwCdsp_SetAWB(argp->awbclamp_en, argp->sindata, argp->cosdata, argp->awbwinthr);	
	hwCdsp_SetAWBYThr(argp->Ythr0, argp->Ythr1, argp->Ythr2, argp->Ythr3);	//lum0clamp....
	hwCdsp_SetAWBUVThr1(argp->UL1N1, argp->UL1P1, argp->VL1N1, argp->VL1P1);
	hwCdsp_SetAWBUVThr2(argp->UL1N2, argp->UL1P2, argp->VL1N2, argp->VL1P2);
	hwCdsp_SetAWBUVThr3(argp->UL1N3, argp->UL1P3, argp->VL1N3, argp->VL1P3);
	hwCdsp_EnableAWB(argp->awb_win_en, argp->awb_win_hold);
	hwCdsp_SetIntEn(ENABLE, CDSP_AWBWIN_UPDATE);
	} 
	else{
		hwCdsp_EnableAWB(DISABLE, DISABLE);
		hwCdsp_SetIntEn(DISABLE, CDSP_AWBWIN_UPDATE);
	}
}

void gp_Cdsp_SetAWBYUV(const INT16S *AwbTable)
{
	gpCdspAWB_t awb;
	
	awb.awb_win_en = ENABLE;
	awb.awb_win_hold = DISABLE;
	awb.awbclamp_en = DISABLE;//ENABLE;
	
	awb.awbwinthr = AwbTable[0];
		
	awb.sindata = AwbTable[1];
	awb.cosdata = AwbTable[2];
       
	awb.Ythr0 = AwbTable[3];
	awb.Ythr1 = AwbTable[4];
	awb.Ythr2 = AwbTable[5];
	awb.Ythr3 = AwbTable[6];
       
	awb.UL1N1 = AwbTable[7]; 
	awb.UL1P1 = AwbTable[8]; 
	awb.VL1N1 = AwbTable[9]; 
	awb.VL1P1 = AwbTable[10]; 
       
	awb.UL1N2 = AwbTable[11]; 
	awb.UL1P2 = AwbTable[12]; 
	awb.VL1N2 = AwbTable[13]; 
	awb.VL1P2 = AwbTable[14]; 
       
	awb.UL1N3 = AwbTable[15]; 
	awb.UL1P3 = AwbTable[16]; 
	awb.VL1N3 = AwbTable[17]; 
	awb.VL1P3 = AwbTable[18]; 
	
	gpCdspSetAWB(&awb);
}

void gpCdspSetAE(gpCdspAE_t *argp)
{
	if(argp->ae_win_en) {
		unsigned int ae0, ae1;
		
		ae0 = 0x80000000 | (unsigned int)CdspDev->aeAddr[0];
		ae1 = 0x80000000 |  (unsigned int)CdspDev->aeAddr[1];

		//DBG_PRINT( "\r\nAeWinAddr0 = 0x%x.\r\n", ae0);
		//DBG_PRINT( "AeWinAddr1 = 0x%x.\r\n", ae1);

		hwCdsp_SetAEWin(argp->phaccfactor, argp->pvaccfactor);
		hwCdsp_SetAEBuffAddr(ae0, ae1);
		hwCdsp_SetInt(ENABLE, CDSP_AEWIN_SEND);
		hwCdsp_EnableAE(argp->ae_win_en, argp->ae_win_hold);
	} else {
		hwCdsp_EnableAE(DISABLE, DISABLE);
		hwCdsp_SetInt(DISABLE, CDSP_AEWIN_SEND);
	}
}

void gpCdspSetWhiteBalance(gpCdspWhtBal_t *argp)
{
	hwCdsp_EnableWbGain(argp->wbgainen);
	hwCdsp_SetWbGain(argp->rgain, argp->grgain, argp->bgain, argp->gbgain);	
	
	#if(USE_SENSOR_NAME == SENSOR_GC1004)
		hwCdsp_SetGlobalGain(argp->global_gain);
	#endif
}

void gpCdspGetWhiteBalance(gpCdspWhtBal_t *argp)
{
	//hwCdsp_GetWbOffset(&argp->wboffseten, &argp->roffset, &argp->groffset, &argp->boffset, &argp->gboffset);
	hwCdsp_GetWbGain(&argp->wbgainen, &argp->rgain, &argp->grgain, &argp->bgain, &argp->gbgain);
	hwCdsp_GlobalGainRead(&argp->global_gain);
}

void gpCdspSetWBGain2(gpCdspWbGain2_t *argp)
{
	if(argp->wbgain2en) {
		hwCdsp_WbGain2Set(argp->rgain2, argp->ggain2, argp->bgain2);
		hwCdsp_EnableWbGain2(ENABLE);
	} else {
		hwCdsp_EnableWbGain2(DISABLE);
	}
}

void gpCdspGetWBGain2(gpCdspWbGain2_t *argp)
{
	hwCdsp_WbGain2Read(&argp->rgain2, &argp->ggain2, &argp->bgain2);
	hwCdsp_EnableWbGain2_Read(&argp->wbgain2en);
}

void gpCdspSetRawWin(
	unsigned short width,
	unsigned short height,
	gpCdspRawWin_t *argp
)
{
	unsigned int x, y;

	//width -= 8;
	//height -= 8;
	if(argp->hwdoffset == 0) {
		argp->hwdoffset = 1;
	}
	
	if(argp->vwdoffset == 0) {
		argp->vwdoffset = 1;
	}
		
	if(argp->aeawb_src == 0) {
		x = argp->hwdoffset + argp->hwdsize * 8;
		y = argp->vwdoffset + argp->vwdsize * 8;
		if(x >= width) {
			x = width - argp->hwdoffset;
			argp->hwdsize = x / 8;
		}

		if(y >= height) {
			y = height - argp->vwdoffset;
			argp->vwdsize = y / 8;
		}
	} else {
		x = argp->hwdoffset*2 + argp->hwdsize*2 * 8;
		y = argp->vwdoffset*2 + argp->vwdsize*2 * 8;
		if(x >= width) {
			x = width - argp->hwdoffset*2;
			argp->hwdsize = x / 8;
			argp->hwdsize >>= 1;
		}

		if(y >= height) {
			y = height - argp->vwdoffset*2;
			argp->vwdsize = y / 8;
			argp->vwdsize >>= 1;
		}
	}

	//DEBUG("AeWinTest = %d\n", argp->AeWinTest);
	//DEBUG(KERN_WARNING "RawWinOffset[%d,%d]\n", argp->hwdoffset, argp->vwdoffset);
	//DEBUG(KERN_WARNING "RawWinCellSize[%d,%d]\n", argp->hwdsize, argp->vwdsize);
	
	hwCdsp_SetAEAWB(argp->aeawb_src, argp->subsample);

	hwCdsp_SetRGBWin(argp->hwdoffset, argp->vwdoffset, argp->hwdsize, argp->vwdsize);
}

void gpCdspGetRawWin(gpCdspRawWin_t *argp)
{
	argp->aeawb_src = hwCdspGetAeAwbSrc();
	argp->subsample = hwCdspGetAeAwbSubSample();
	hwCdspGet3ATestWinEn(&argp->AeWinTest, &argp->AfWinTest);
	hwCdspGetRGBWin(&argp->hwdoffset, &argp->vwdoffset, &argp->hwdsize, &argp->vwdsize);
}

/****************************************************************************************************
AP Function
*****************************************************************************************************/


void gp_cdsp_set_wb_gain(INT8U gp_wbgainen, INT8U gp_wbgain2en)
{
	gpCdspWhtBal_t wht_bal;   
	gpCdspWbGain2_t wbgain2;   

	// set AWB enable   
	wht_bal.wbgainen = gp_wbgainen;	   
#if (USE_SENSOR_NAME == SENSOR_SOI_H22)
	#if 0	//3600K
	wht_bal.rgain = 64;
	wht_bal.bgain = 121;
	#elif 0 //4000K
	wht_bal.rgain = 72;//67;
	wht_bal.bgain =  120;//110;
	#else
	wht_bal.rgain = 92;
	wht_bal.bgain = 82;
	#endif
#elif (USE_SENSOR_NAME == SENSOR_GC1004)	
	#if 0 //3600K
	wht_bal.rgain = 58;	   
	wht_bal.bgain = 90;  
	#elif 0 //4000K
	wht_bal.rgain = 71; 
	wht_bal.bgain = 82;
	#else	//6400K
	wht_bal.rgain = 88; 
	wht_bal.bgain = 65;   
	#endif
#elif (USE_SENSOR_NAME == SENSOR_OV9712)	
	#if 0 //3600K
	wht_bal.rgain = 58;	   
	wht_bal.bgain = 90;  
	#elif 0 //4000K
	wht_bal.rgain = 64; 
	wht_bal.bgain = 64;
	#else	//6400K
	wht_bal.rgain = 91; 
	wht_bal.bgain = 67;   
	#endif
#endif	
	wht_bal.gbgain = 1.0*64;   
	wht_bal.grgain = 1.0*64;   
	wht_bal.global_gain = 1*32;   
	   
	gpCdspSetWhiteBalance(&wht_bal);   

	// set WB gain2 disable   
	wbgain2.wbgain2en =gp_wbgain2en;   
	wbgain2.bgain2 = 64;   
	wbgain2.rgain2 = 64;   
	wbgain2.ggain2 = 64;   
	   
	gpCdspSetWBGain2(&wbgain2);   
	   
	//return 0;   
}



int gp_cdsp_set_aeawb_window(int width, int height, int tagetY)
{
	int t1;
	
	//gpCdspAWB_t awb_win;
	gpCdspAE_t ae;
	gpCdspRawWin_t raw_win;
	
	
	// set RAW Win
	raw_win.aeawb_src = 1;
	raw_win.subsample = 0;	// 0:disable, 1:1/2, 2:1/4 subsample
	t1 = width;
	while(t1 > 1024)
	{
		raw_win.subsample++;
		t1 >>= 1;
	}

	raw_win.hwdoffset = 32*2; //org 32*2
	raw_win.vwdoffset = 2*2;

	raw_win.hwdsize = (t1 - (raw_win.hwdoffset << 1)) >> 3;
	raw_win.vwdsize = (height - (raw_win.vwdoffset<<1)) >> 3;
	
	raw_win.hwdsize >>= 1;
	raw_win.vwdsize >>= 1;
	raw_win.hwdoffset >>= 1;
	raw_win.vwdoffset >>= 1;
	
	DBG_PRINT("RAW_Win Set: subsample = %d, size[%d, %d],  offset[%d, %d]\n", raw_win.subsample, raw_win.hwdsize, raw_win.vwdsize, raw_win.hwdoffset, raw_win.vwdoffset);
	
	gpCdspSetRawWin(width, height, &raw_win);
	gpCdspGetRawWin(&raw_win);
	
	gp_cdsp_awb_set_cnt_thr(awb, width, height, raw_win.subsample, 1200); //2000-->1200
	
	// set AE
	ae.ae_win_en = ENABLE;
	ae.ae_win_hold = 0;
	ae.phaccfactor = raw_win.hwdsize;
	ae.pvaccfactor = raw_win.vwdsize;
	ae.ae_meter = CDSP_AE_METER_CENTER_WEIGHTED_AVG_CVR;// CDSP_AE_METER_SPOT; //CDSP_AE_METER_CENTER_WEIGHTED_AVG;
	//ae.ae_meter = CDSP_AE_METER_AVG;
	
	gp_cdsp_ae_set_meter(CDSP_AE_METER_CENTER_WEIGHTED_AVG_CVR, CdspDev->ae_workmem);
	//gp_cdsp_ae_set_meter(CDSP_AE_METER_AVG, CdspDev->ae_workmem);
		
	gp_cdsp_ae_set_lum_bound(CdspDev->ae_workmem, width, height, raw_win.hwdsize, raw_win.vwdsize);
	gp_cdse_ae_set_backlight_detect(CdspDev->ae_workmem, ENABLE);		//Backlight function
	gpCdspSetAE(&ae);

	// set Target Lum		
	gp_cdsp_ae_set_target_lum(CdspDev->ae_workmem, TARGET_Y);
	//gp_cdsp_ae_set_target_lum_night(CdspDev->ae_workmem, 0x40);
	
	return 0;
}

void gp_cdsp_set_histgm(INT8U hi_en, INT8U hi_thr, INT8U low_thr)
{
	gpCdspHistgm_t histgm;

	histgm.his_en = hi_en;
	histgm.hishighthr = hi_thr;
	histgm.hislowthr = low_thr;
	histgm.his_hold_en = 0;

	if(histgm.his_en) {
		hwCdsp_SetHistgm(histgm.hislowthr, histgm.hishighthr);
		hwCdsp_EnableHistgm(ENABLE, histgm.his_hold_en);
	} else {
		hwCdsp_EnableHistgm(DISABLE, DISABLE);
	}
}

void gp_cdsp_set_new_denoise(INT32U sharpness)
{
	gpCdspNewDenoise_t NewDenoise;

	NewDenoise.newdenoiseen = DISABLE;
	NewDenoise.ndmirvsel = 0;
	NewDenoise.ndmiren = 0xD;
	
	NewDenoise.ndedgeen = ENABLE;
	NewDenoise.ndeluten = ENABLE;
	NewDenoise.ndampga = sharpness;
	NewDenoise.ndlf00 = 0x9;
	NewDenoise.ndlf01 = 0;
	NewDenoise.ndlf02 = 0x9;
	NewDenoise.ndlf10 = 0;
	NewDenoise.ndlf11 = 0x4;
	NewDenoise.ndlf12 = 0;
	NewDenoise.ndlf20 = 0x9;
	NewDenoise.ndlf21 = 0;
	NewDenoise.ndlf22 = 0x9;
	NewDenoise.ndlhdiv = 1;
	NewDenoise.ndlhtdiv = 0;
	NewDenoise.ndlhcoring = 8;
	NewDenoise.ndlhmode = 0;

	gpCdspSetNewDenoise(&NewDenoise);
}

void gp_cdsp_set_saturation(void)
{
	gpCdspSatHue_t sat_hue;
	gpCdspSpecMod_t spec_mode;
	// set  Saturation/Constrast enhancement
	spec_mode.yuvspecmode = SP_YUV_YbYcSatHue;
	spec_mode.binarthr = 0x1F;
	
	sat_hue.YbYcEn = ENABLE;
	sat_hue.u_huecosdata = 0x40;
	sat_hue.u_huesindata = 0x00;
	sat_hue.v_huecosdata = 0x40;
	sat_hue.v_huesindata = 0x00;
		
	// daylight
	sat_hue.y_offset = DAYLIGHT_HUE_Y_OFFSET; //-8,-4 // -128 ~ +127	
	sat_hue.u_offset = DAYLIGHT_HUE_U_OFFSET;//0;//8;//-4; // -128 ~ +127,   +: more blue,  -: more yellow/green	
	sat_hue.v_offset = DAYLIGHT_HUE_V_OFFSET;//-8; //-128 ~ +127,   +: more red,  -: more blue/green
	
	sat_hue.y_scale = DAYLIGHT_SAT_Y_SCALE;//0x20;//0x21; // contrast
	sat_hue.u_scale = DAYLIGHT_SAT_U_SCALE;//0x20;//0x24; // blud
	sat_hue.v_scale = DAYLIGHT_SAT_V_SCALE;//0x1E;//0x20;//0x24; // red

	gpCdspSetSpecialMode(&spec_mode);
	gpCdspSetSatHue(&sat_hue);
}

void gp_cdsp_set_edge(INT32U sharpness)
{
	gpCdspEdge_t edge;

	CdspDev->rawFmtFlag = 1;	//raw:1, yuv:0
	
	edge.edgeen = ENABLE;//DISABLE;
	edge.eluten = ENABLE;
#if (USE_SENSOR_NAME == SENSOR_SOI_H22)
	edge.edge_table = 0;
	edge.ampga = sharpness;
	edge.edgedomain = 0x1;
	edge.lhmode = 0;
	edge.lhdiv = 1; //1-->0
	edge.lhtdiv = 0;
	edge.lhcoring = 4;
	edge.Qcnt = 0x0;
	edge.Qthr = 255;
	edge.lf00 = 0x9;
	edge.lf01 = 0;
	edge.lf02 = 0x9;
	edge.lf10 = 0;
	edge.lf11 = 0x4;
	edge.lf12 = 0;
	edge.lf20 = 0x9;
	edge.lf21 = 0;
	edge.lf22 = 0x9;
#elif (USE_SENSOR_NAME == SENSOR_GC1004)
	edge.edge_table = 0;
	edge.ampga = sharpness;
	edge.edgedomain = 0x0;
	edge.lhmode = 0;
	edge.lhdiv = 1;
	edge.lhtdiv = 0;
	edge.lhcoring = 1;
	edge.Qcnt = 0;
	edge.Qthr = 255;
	edge.lf00 = 0x9;
	edge.lf01 = 0;
	edge.lf02 = 0x9;
	edge.lf10 = 0;
	edge.lf11 = 0x4;
	edge.lf12 = 0;
	edge.lf20 = 0x9;
	edge.lf21 = 0;
	edge.lf22 = 0x9;
#elif (USE_SENSOR_NAME == SENSOR_OV9712)
	edge.edge_table = 0;
	edge.ampga = sharpness;
	edge.edgedomain = 0x0;
	edge.lhmode = 0;
	edge.lhdiv = 0; //1-->0
	edge.lhtdiv = 1;
	edge.lhcoring = 0x1;
	edge.Qcnt = 0x0;
	edge.Qthr = 0x00;
	edge.lf00 = 0x9;
	edge.lf01 = 0;
	edge.lf02 = 0x9;
	edge.lf10 = 0;
	edge.lf11 = 0x4;
	edge.lf12 = 0;
	edge.lf20 = 0x9;
	edge.lf21 = 0;
	edge.lf22 = 0x9;	
#endif
	DBG_PRINT("Edge Enable\r\n");
	gpCdspSetEdge(CdspDev->rawFmtFlag, &edge);
}

void gp_cdsp_set_awb_mode(INT32U mode)
{
	AWB_MODE wbmode = mode;

	gp_cdsp_awb_set_mode(CdspDev->awb_workmem, wbmode);
}

// 0:+2, 1:+5/3, 2:+4/3, 3:+1.0, 4:+2/3, 5:+1/3, 6:+0.0, 7:-1/3, 8:-2/3, 9:-1.0, 10:-4/3, 11:-5/3, 12:-2.0 
void gp_cdsp_set_ev_val(INT8U ev)
{
	INT8U ev_val = ev;

	gp_cdsp_ae_set_ev(CdspDev->ae_workmem, ev_val);	
}

void ap_iso_set(INT32U iso)
{
	INT32U iso_val;
	
	switch(iso)
	{
		case 100: 
			iso_val = 1;
			break;
		case 200: 
			iso_val = 2;
			break;
		case 400: 
			iso_val = 4;
			break;
		case 800: 
			iso_val = 8;
			break;

		default:
			iso_val = ISO_AUTO;
			break;
	}

	if (iso_val != ISO_AUTO) 
	{
		DBG_PRINT("\r\nSet ISO = %d00\r\n", iso_val);
	}
	else 
	{
		DBG_PRINT("\r\nSet AUTO ISO\r\n");
	}

	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_ISP_ISO_SET, &iso_val, sizeof(INT32U), MSG_PRI_NORMAL);
}

void gp_isp_iso_set(INT32U iso)
{
	//sensor_exposure_t iso_seinfo;

	if (iso == 0x81){
		iso = 0x1881;
	}
	
	#if (USE_SENSOR_NAME == SENSOR_SOI_H22)	
		H22_get_exposure_time(&seInfo);
	#elif (USE_SENSOR_NAME == SENSOR_GC1004)
		gc1004_get_exposure_time(&seInfo);
	#elif (USE_SENSOR_NAME == SENSOR_OV9712)
		ov9712_get_exposure_time(&seInfo);	
	#endif
	
	seInfo.userISO = iso;
	//p_seInfo->userISO = iso;
		
 	//memcpy(&seInfo, p_seInfo, sizeof(sensor_exposure_t));
	gp_cdsp_ae_set_iso(CdspDev->ae_workmem, iso, &seInfo);
}

void gp_isp_set_hybridraw(gpIspHybridRaw_t *argp)
{
	/*ISP image size*/
	hwIsp_SetImageSize(argp->ISPWidth, argp->IspHeight); //must be setting for hybridRaw function
	
#if 1		//Bad pixel isp function
	hwIsp_dpc_rcv_mode_sel_thr(argp->DPCDefaultMode, argp->DPCth1, argp->DPCth2, argp->DPCth3);	//DPC_rcv_mode,DPCth1,DPCth2,DPCth3
	hwIsp_dpc_en(argp->DefectPixelEnable, argp->DefectPixelSel);
	hwIsp_smooth_factor(argp->DPCn);	//for bad pixel

	//Crosstalk Enable/Disable
	hwIsp_EnableCrostlkGbGr(argp->CrosstalkEnable, argp->CrosstalkGbGr);	//Gb&Gr
	hwIsp_CrostlkThold(argp->CrosstalkT1, argp->CrosstalkT2, argp->CrosstalkT3, argp->CrosstalkT4);
	hwIsp_CrostlkWeight(argp->CrosstalkW1, argp->CrosstalkW2, argp->CrosstalkW3);
	
	hwIsp_EnableDenoise(argp->DenoiseEnable);
	hwIsp_DenoiseThold(argp->DenoiseT1, argp->DenoiseT2, argp->DenoiseT3, argp->DenoiseT4);
	hwIsp_DenoiseWeight(argp->DenoiseW1, argp->DenoiseW2, argp->DenoiseW3);
#endif
	
#if 1	//New ISP Linearity Correction
	hwIsp_LinCorr_Enable(DISABLE);
#endif
}

void gp_cdsp_set_exp_freq(INT32U freq_num)
{
	INT32U exp_freq;
	switch(freq_num)
	{
		case 0: 
			exp_freq = 50;
			break;
		case 1: 
			exp_freq = 60;
			break;
	}		
	#if (USE_SENSOR_NAME == SENSOR_SOI_H22)	
		H22_set_exp_freq(exp_freq);
	#elif (USE_SENSOR_NAME == SENSOR_GC1004)
		gc1004_set_exp_freq(exp_freq);
	#elif (USE_SENSOR_NAME == SENSOR_OV9712)
		ov9712_set_exp_freq(exp_freq);	
	#endif
	
	DBG_PRINT("\r\nFREQ = %d\r\n", exp_freq);	
}
/*****************************************************************************************************
AP End
******************************************************************************************************/

/*******************
IRQ Handle
********************/
static void gp_cdsp_handle_awb(void)
{
	signed int tempsh;
	unsigned int cnt, temph, templ, tempsl;		
	__int64 tt;
	AWB_RESULT_t *awb_result = &CdspDev->result.awb_result;
	
	//DBG_PRINT("1 AWB ISR, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
	
	if(CdspDev == NULL) {
		return;
	}
	
	if((CdspDev->ae_awb_flag & CDSP_AWB_CTRL_EN) != 0)
		return ;
		
	gpHalCdspGetAwbSumCnt(1, &cnt);
	awb_result->sumcnt[0] = cnt;
	gpHalCdspGetAwbSumCnt(2, &cnt);
	awb_result->sumcnt[1] = cnt;
	gpHalCdspGetAwbSumCnt(3, &cnt);
	awb_result->sumcnt[2] = cnt;

	gpHalCdspGetAwbSumG(1, &templ, &temph);
	awb_result->sumg[0] = temph;
	awb_result->sumg[0] <<= 16;
	awb_result->sumg[0] |= templ;
	gpHalCdspGetAwbSumG(2, &templ, &temph);
	awb_result->sumg[1] = temph;
	awb_result->sumg[1] <<= 16;
	awb_result->sumg[1] |= templ;
	gpHalCdspGetAwbSumG(3, &templ, &temph);
	awb_result->sumg[2] = temph;
	awb_result->sumg[2] <<= 16;
	awb_result->sumg[2] |= templ;

	gpHalCdspGetAwbSumRG(1, &tempsl, &tempsh);
	//DBG_PRINT("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	tt = tempsh;
	tt <<= 16;
	tt |= tempsl;	
	awb_result->sumrg[0] = tt;
	//DBG_PRINT("awb_result->sumrg[0] = 0x%010lx\r\n", awb_result->sumrg[0]);
	
	gpHalCdspGetAwbSumRG(2, &tempsl, &tempsh);
	//DBG_PRINT("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumrg[1] = tempsh;
	awb_result->sumrg[1] <<= 16;
	awb_result->sumrg[1] |= tempsl;
	//DBG_PRINT("awb_result->sumrg[1] = 0x%010lx\r\n", awb_result->sumrg[1]);	
	
	gpHalCdspGetAwbSumRG(3, &tempsl, &tempsh);
	//DBG_PRINT("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumrg[2] = tempsh;
	awb_result->sumrg[2] <<= 16;
	awb_result->sumrg[2] |= tempsl;
	//DBG_PRINT("awb_result->sumrg[2] = 0x%010lx\r\n", awb_result->sumrg[2]);	
	 

	gpHalCdspGetAwbSumBG(1, &tempsl, &tempsh);
	//DBG_PRINT("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumbg[0] = tempsh;
	awb_result->sumbg[0] <<= 16;
	awb_result->sumbg[0] |= tempsl;
	//DBG_PRINT("awb_result->sumbg[0] = 0x%llx\r\n", awb_result->sumbg[0]);
	
	gpHalCdspGetAwbSumBG(2, &tempsl, &tempsh);
	//DBG_PRINT("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumbg[1] = tempsh;
	awb_result->sumbg[1] <<= 16;	
	awb_result->sumbg[1] |= tempsl;
	
	//DBG_PRINT("awb_result->sumbg[1] = 0x%010lx\r\n", awb_result->sumbg[1]);
	
	gpHalCdspGetAwbSumBG(3, &tempsl, &tempsh);
	//DBG_PRINT("tempsh = 0x%x, tempsl = 0x%x\r\n", tempsh, tempsl);	
	awb_result->sumbg[2] = tempsh;
	awb_result->sumbg[2] <<= 16;
	awb_result->sumbg[2] |= tempsl;
	
	//DBG_PRINT("awb_result->sumbg[2] = 0x%010lx\r\n", awb_result->sumbg[2]);

	CdspDev->ae_awb_flag |= CDSP_AWB_CTRL_EN;
	//wake_up_interruptible(&CdspDev->ae_awb_wait_queue);	//?
	
	//DBG_PRINT("2 AWB ISR, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
}


static void gp_cdsp_handle_ae(void)
{
	gpCdsp3aResult_t *argp = &CdspDev->result;
	//unsigned int *ptr;
		
	//DBG_PRINT("1 AE ISR, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
	if (CdspDev == NULL){
		return;
	}
	if((CdspDev->ae_awb_flag & CDSP_AE_CTRL_EN) != 0)
		return;
	
	if(gpHalCdspGetAEActBuff()) {
		gp_memcpy((INT8S *) argp->ae_win, (INT8S *)CdspDev->aeAddr[1], 64);
		//ptr = (unsigned int *)CdspDev->aeAddr[1];
	} else {
		gp_memcpy((INT8S *) argp->ae_win, (INT8S *)CdspDev->aeAddr[0], 64);
		//ptr = (unsigned int *)CdspDev->aeAddr[0];
	}
	CdspDev->ae_awb_flag |= CDSP_AE_CTRL_EN;
/*	
	if(ae_switch_flag == 0)
		hwCdsp_SetRGBWin(2, 2, 32, 44);
	else if(ae_switch_flag == 2)
		hwCdsp_SetRGBWin(62, 2, 32, 44);
*/	

	//wake_up_interruptible(&CdspDev->ae_awb_wait_queue);	//??
	//DBG_PRINT("2 AE ISR, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
}

#if 0
static int gp_cdsp_handle_of(void)
{
	//DBG_PRINT("CDSP: overflow\n");
	return 0;
}

static int gp_cdsp_handle_facwr(void)
{
	//DBG_PRINT("CDSP: facwr\n");
	return 0;
}

static int gp_cdsp_handle_postprocess(void)
{
	if(CdspDev->cdsp_feint_flag == 0) {
		CdspDev->cdsp_feint_flag = 1;
		//wake_up_interruptible(&CdspDev->cdsp_wait_queue);	//??
	} else {
		return -1;
	}
	return 0;
}

static void gp_cdsp_handle_capture(void)
{
	//DBG_PRINT("%d\n", CdspDev->CapCnt);
#if 0	//by uii off
	CdspDev->CapCnt--;
	if(CdspDev->CapCnt == 0) {
		if(CdspDev->cdsp_feint_flag == 0) {
			CdspDev->cdsp_feint_flag = 1;
			//wake_up_interruptible(&CdspDev->cdsp_wait_queue);		//??
		} else {
			CdspDev->CapCnt++;
		}
	}
#endif
}
#endif

extern void hwMipi_SetSize(INT32U hsize,INT32U vsize);
static void gp_cdsp_handle_eof(void)
{
	
	//INT32U htmp;
	//CdspDev->rawFmtFlag = 1;
	if (CdspDev == NULL) {
		return;
	}
	
	if (CdspDev-> ae_ret == AE_CHANGE) 
	{
	
	#if 0
		//OSQPost(my_AVIEncodeApQ, (void *) MSG_CDSP_AE_SHUTTER);
	#else
		#if (USE_SENSOR_NAME == SENSOR_SOI_H22)	
		if(ae_exp_set_cnt == 0)
		{
			H22_set_exposure_time(&seInfo2);
			ae_exp_set_cnt = 1;
		}
		else
		{
			H22_set_exposure_gain();
			ae_exp_set_cnt = 0;
		}			
		#elif (USE_SENSOR_NAME == SENSOR_GC1004)
			gc1004_set_exposure_time(&seInfo2);
			gc1004_set_exposure_gain();
		#elif (USE_SENSOR_NAME == SENSOR_OV9712)
		if(ae_exp_set_cnt == 0)
		{
			ov9712_set_exposure_time(&seInfo2);
			ov9712_set_exposure_gain();
			ae_exp_set_cnt = 1;
		}
		else
		{
			ov9712_group_off();
			ae_exp_set_cnt = 0;
		}			
		#endif
	#endif
		if(ae_exp_set_cnt == 0)	CdspDev->ae_ret = -1;
	}
	
	//if(CdspDev -> awb_ret_flag != AWB_FAIL || CdspDev -> awb_ret_flag != AWB_RET)
	if(CdspDev->ae_awb_flag & CDSP_AWB_SET_GAIN	!= 0)
	{
		hwCdsp_SetWb_R_B_Gain(wb_gain.rgain, wb_gain.bgain);
		CdspDev->ae_awb_flag &= (~CDSP_AWB_SET_GAIN);
	}
#if 1	
	if(ae_switch_flag == 0)
	{
		hwCdsp_SetRGBWin(0, 2, 32, 44);
		//hwCdsp_SetRGBWin(4, 80, 32, 32);
		//hwCdsp_SetAEWin(32, 32);
	}
	else if(ae_switch_flag == 2)
	{
		hwCdsp_SetRGBWin(64, 2, 32, 44);
		//hwCdsp_SetRGBWin(60, 80, 32, 32);
	}
	else if(ae_switch_flag == 3)
	{
		hwCdsp_SetRGBWin(32, 2, 32, 44);
		//hwCdsp_SetAEWin(32, 44);
		ae_switch_flag = 1; // not change AE window
	}
#endif
	
	if((CdspDev->ae_awb_flag & CDSP_HIST_CTRL_EN) == 0)
	{
		hwCdsp_GetHistgm(&CdspDev->result.hislowcnt, &CdspDev->result.hishicnt);
		CdspDev->ae_awb_flag |= CDSP_HIST_CTRL_EN;
	}

#if 1		
	// add by Comi for changing the CDSP setting
	if((CdspDev->ae_awb_flag & CDSP_LUM_STATUS) != 0)
	{	
		if (CdspDev->ae_awb_flag & CDSP_HIGH_LUM)
		{
			//for enable new denoise
			#if ((sensor_format == GC1004_MIPI) || (sensor_format == SOI_H22_MIPI))
			hwMipi_SetSize(1280, 720);
			#endif	
			hwFront_SetSize(1280, 720);
			hwIsp_SetImageSize(1280, 720);
			hwCdsp_EnableNewDenoise(DISABLE);
			hwIsp_EnableDenoise(DISABLE);
			hwIsp_EnableCrostlk(DISABLE);
			
			hwCdsp_SetBadPixel(160, 160, 160);
			hwCdsp_SetEdgeAmpga(DAYLIGHT_EDGE, 0);

			//gp_cdsp_set_ev_val(6);
			//DBG_PRINT("hdf");
			
			//DBG_PRINT("\r\n\r\n ============== nd DISABLE ================\r\n\r\n");
		}
		else if (CdspDev->ae_awb_flag & CDSP_LOW_LUM)
		{
			hwCdsp_SetBadPixel(80, 80, 80);
			
			#if 1
			if(denoise_level == HYBRID_DENOISE_ONLY)
			{
				#if ((sensor_format == GC1004_MIPI) || (sensor_format == SOI_H22_MIPI))
				hwMipi_SetSize(1280, 720);
				#else
				hwFront_SetSize(1280, 720);
				#endif
				hwIsp_SetImageSize(1280, 720);
				hwCdsp_EnableNewDenoise(DISABLE);
				hwIsp_EnableDenoise(ENABLE);
				
				//gp_cdsp_set_ev_val(4);
				//DBG_PRINT("hd-");
			}
			else if(denoise_level == NEW_DENOISE_ONLY)
			{
				#if ((sensor_format == GC1004_MIPI) || (sensor_format == SOI_H22_MIPI))
				hwMipi_SetSize(1280, 720+2);
				#else
				hwFront_SetSize(1280, 720+2);
				#endif
				hwIsp_SetImageSize(1280, 720+2);		
				hwCdsp_EnableNewDenoise(ENABLE);	
				hwIsp_EnableDenoise(ENABLE);
				hwIsp_EnableCrostlk(ENABLE);
				
				//gp_cdsp_set_ev_val(ae_ev);
				//DBG_PRINT("nd-");
			}
			#else			
			hwCdsp_EnableNewDenoise(DISABLE);	
			hwIsp_EnableDenoise(DISABLE);
			
			//gp_cdsp_set_ev_val(ae_ev);
			//DBG_PRINT("ldf");
			#endif
						
			hwCdsp_SetEdgeAmpga(NIGHT_EDGE, 0);
			//DBG_PRINT("\r\n\r\n ============== nd ENABLE ================\r\n\r\n");
		}

		CdspDev->ae_awb_flag &= (~CDSP_LUM_STATUS);		
	}
	
	if((CdspDev->ae_awb_flag & CDSP_SAT_SWITCH) != 0)
	{						
		if(CdspDev->sat_contr_idx != -1)		
		{	
			int *p_yuv, *p_uvdiv;	
			p_yuv = &sat_yuv_level[CdspDev->sat_contr_idx][0];
			p_uvdiv = &uv_div[CdspDev->sat_contr_idx][0];
			hwCdsp_SetYuvSPEffOffset(p_yuv[0], p_yuv[1], p_yuv[2]);
			hwCdsp_SetYuvSPEffScale(p_yuv[3], p_yuv[4], p_yuv[5]);
			hwCdsp_SetUVScale(1, p_uvdiv[0], p_uvdiv[1], p_uvdiv[2], p_uvdiv[3], p_uvdiv[4], p_uvdiv[5]);
			CdspDev->ae_awb_flag &= (~CDSP_SAT_SWITCH);
			
			//gp_cdsp_set_ev_val(ae_ev);
			//DBG_PRINT("YUV[%d] = %d, %d, %d, %d, %d, %d\r\n", CdspDev->sat_contr_idx,	p_yuv[0], p_yuv[1], p_yuv[2], p_yuv[3], p_yuv[4], p_yuv[5]);
		}
		else
		{
			hwCdsp_SetUVScale(0,0,0,0,0,0,0);
			CdspDev->ae_awb_flag &= (~CDSP_SAT_SWITCH);
		}
		
	}	
#endif		
}


/**************************************************************************
Drv_L2 Function
**************************************************************************/
void gp_cdsp_crop(INT8U crop_en, INT32U crop_width, INT32U crop_height, INT32U frame_width, INT32U frame_height)
{
	//INT32U clamp_width;
	INT32U HOffset, VOffset;
	
	HOffset = (frame_width - crop_width) >> 1;
	VOffset = (frame_height - crop_height) >> 1;
	//Enable Image crop function
	hwCdsp_SetCrop(HOffset, VOffset, crop_width, crop_height);
	hwCdsp_EnableCrop(crop_en); 	//1:Enable, 0:Disable
	//clamp_width = crop_width; //(200 - 8) is for not enable other function
	//hwCdsp_EnableClamp(1, clamp_width);
	//hwCdsp_SetYuvBuffA(crop_width, crop_height, dummy_addrs);	//cdsp_buffer = dummy_addrs
}

void gp_cdsp_yuv_scaler (INT8U scaler_en, INT32U scaler_width, INT32U scaler_height, INT32U frame_width, INT32U frame_height)
{
	INT32U hfactor, vfactor;
	
	hfactor = (scaler_width << 16)/frame_width + 1;
	vfactor = (scaler_height << 16)/frame_height + 1;
	hwCdsp_SetYuvHScale(hfactor, hfactor);
	hwCdsp_SetYuvVScale(vfactor, vfactor);
	hwCdsp_EnableYuvHScale(scaler_en, 1);
	hwCdsp_EnableYuvVScale(scaler_en, 1);
		//clamp_en = 1;
		//clamp_width = scaler_width;	//(scaler_width - 4) is for not enable other function
		//read_width = scaler_width;
		//read_height = scaler_height;
}

void gpCdspSetBadPixOb(gpCdspBadPixOB_t *argp)
{
	/* bad pixel */
	
	if(argp->badpixen) {
		hwCdsp_SetBadPixel(argp->bprthr, argp->bpgthr, argp->bpbthr);
		hwCdsp_EnableBadPixel(argp->badpixen, 0x1, 0x1); /* enable mirror */
	} else {
		hwCdsp_EnableBadPixel(DISABLE, 0x1,0x1);
	}

	/* optical black */
	if(argp->manuoben || argp->autooben) {
		/* enable white balance offset when use ob */
		if(argp->wboffseten == 0) {
			argp->wboffseten = 1;
		}
		
		hwCdsp_SetWbOffset(argp->wboffseten, argp->roffset, argp->groffset, argp->boffset, argp->gboffset);
		//hwCdsp_SetAverageOB(argp->wboffseten, argp->roffset, argp->groffset, argp->boffset, argp->gboffset);
		hwCdsp_SetManuOB(argp->manuoben, argp->manuob);
		hwCdsp_SetAutoOB(argp->autooben, argp->obtype, 0);
	} else {
		hwCdsp_SetManuOB(DISABLE, argp->manuob);
		hwCdsp_SetAutoOB(DISABLE, argp->obtype, 0);
	}
}

void gp_Cdsp_SetBadPixOb(const INT16U *OBTable)
{
	gpCdspBadPixOB_t bad_pixel;

	gp_memset((INT8S*) &bad_pixel, 0, sizeof(gpCdspBadPixOB_t));
	
	DBG_PRINT("\r\n gp_Cdsp_SetBadPixOb \r\n ");
	
	bad_pixel.autooben = OBTable[0];				
	bad_pixel.manuoben =  OBTable[1];
	bad_pixel.manuob =  OBTable[2];
	bad_pixel.wboffseten = OBTable[3];
	bad_pixel.roffset = OBTable[4];
	bad_pixel.groffset = OBTable[5];
	bad_pixel.gboffset = OBTable[6];
	bad_pixel.boffset = OBTable[7];
	
	bad_pixel.badpixen = OBTable[8];
	bad_pixel.bprthr = OBTable[9];
	bad_pixel.bpgthr = OBTable[10];
	bad_pixel.bpbthr = OBTable[11];
	
	bad_pixel.badpixmirr = 0x3;

/*
	// add by Comi 20140806
	bad_pixel.badpixen = ENABLE;

	bad_pixel.badpixmirr = 0x3;
	bad_pixel.bprthr = 160;
	bad_pixel.bpgthr = 160;
	bad_pixel.bpbthr = 160;
*/	
	gpCdspSetBadPixOb(&bad_pixel);
}


void gpCdspSetNewDenoise(
	gpCdspNewDenoise_t *argp
)
{
	//DEBUG("%s: newdenoiseen = %d\n", __FUNCTION__, argp->newdenoiseen);
	
		hwCdsp_EnableNewDenoise(argp->newdenoiseen);
		hwCdsp_SetNewDenoise_Sel_Mirror(argp->ndmirvsel, argp->ndmiren);
		hwCdsp_EnableNdEdge(argp->ndedgeen, argp->ndeluten);
		//gpHalCdspSetNdEdgeLCoring(argp->ndlhdiv, argp->ndlhtdiv, argp->ndlhcoring, argp->ndlhmode);
		hwCdsp_SetNdEdgeLCoring(argp->ndlhdiv, argp->ndlhtdiv, argp->ndlhcoring);
		hwCdsp_SetNdEdgeAmpga(argp->ndampga);
		if(argp->ndeluten) {
		hwCdsp_NewDEdgeLut((INT8U *)g_nd_edge_table);
	}

		/*3x3 programing matrix */
		gpHalCdspSetNdEdgeFilter(0, argp->ndlf00, argp->ndlf01, argp->ndlf02);
		gpHalCdspSetNdEdgeFilter(1, argp->ndlf10, argp->ndlf11, argp->ndlf12);
		gpHalCdspSetNdEdgeFilter(2, argp->ndlf20, argp->ndlf21, argp->ndlf22);
}

void gpCdspGetNewDenoise(gpCdspNewDenoise_t *argp)
{
	argp->newdenoiseen = gpHalCdspGetNewDenoiseEn();
	gpHalCdspGetNewDenoise(&argp->ndmirvsel, &argp->ndmiren);
	gpHalCdspGetNdEdgeEn(&argp->ndedgeen, &argp->ndeluten);
	//DBG_PRINT("%s: ndedgeen = %d, ndeluten = %d\n", __FUNCTION__, argp->ndedgeen, argp->ndeluten);
	
	gpHalCdspGetNdEdgeLCoring(&argp->ndlhdiv, &argp->ndlhtdiv, &argp->ndlhcoring);
	argp->ndampga =	gpHalCdspGetNdEdgeAmpga();
		
	gpHalCdspGetNdEdgeFilter(0, &argp->ndlf00, &argp->ndlf01, &argp->ndlf02);
	gpHalCdspGetNdEdgeFilter(1, &argp->ndlf10, &argp->ndlf11, &argp->ndlf12);
	gpHalCdspGetNdEdgeFilter(2, &argp->ndlf20, &argp->ndlf21, &argp->ndlf22);
}

void gpCdspSetEdge(	unsigned char raw_flag,	gpCdspEdge_t *argp)
{
	//DBG_PRINT("%s = %d\n", __FUNCTION__, argp->edgeen);
	if(argp->edgeen) {
		if(argp->edge_table) {
			hwCdsp_InitEdgeLut(argp->edge_table);
		} else {
			hwCdsp_InitEdgeLut((INT8U *)LEElut);
		}
		hwCdsp_EnableEdgeLutTable(argp->eluten);
		hwCdsp_EnableEdge(argp->edgeen);
		hwCdsp_SetEdgeLCoring(argp->lhdiv, argp->lhtdiv, argp->lhcoring, argp->lhmode);
		hwCdsp_SetEdgeAmpga(argp->ampga, argp->edgedomain);
		gpHalCdspSetEdgeQthr(argp->Qthr);

		if(raw_flag) {
			hwCdsp_EnableYEdgeUvSuppr(0);
		} else {
			hwCdsp_EnableYEdgeUvSuppr(1);
		}
		
		/*3x3 programing matrix */
		if(argp->lhmode == 0) {
			gpHalCdspSetEdgeFilter(0, argp->lf00, argp->lf01, argp->lf02);
			gpHalCdspSetEdgeFilter(1, argp->lf10, argp->lf11, argp->lf12);
			gpHalCdspSetEdgeFilter(2, argp->lf20, argp->lf21, argp->lf22);
		}
	} else {
		hwCdsp_EnableEdge(DISABLE);
		hwCdsp_EnableEdgeLutTable(DISABLE);
	}
}

void gpCdspGetEdge(gpCdspEdge_t *argp)
{
	argp->eluten = gpHalCdspGetEdgeLutTableEn();
	argp->edge_table = NULL;
	argp->edgeen = gpHalCdspGetEdgeEn();
	gpHalCdspGetEdgeLCoring(&argp->lhdiv, &argp->lhtdiv, &argp->lhcoring, &argp->lhmode);
	argp->ampga = gpHalCdspGetEdgeAmpga();
	argp->edgedomain = gpHalCdspGetEdgeDomain();
	argp->Qthr = gpHalCdspGetEdgeQCnt();

	gpHalCdspGetEdgeFilter(0, &argp->lf00, &argp->lf01, &argp->lf02);
	gpHalCdspGetEdgeFilter(1, &argp->lf10, &argp->lf11, &argp->lf12);
	gpHalCdspGetEdgeFilter(2, &argp->lf20, &argp->lf21, &argp->lf22);
}

void gpCdspSetSpecialMode(gpCdspSpecMod_t *argp)
{
	//DBG_PRINT("%s = %d\n", __FUNCTION__, argp->yuvspecmode);
	hwCdsp_SetYuvSpecModeBinThr(argp->binarthr);
	hwCdsp_SetYuvSpecMode(argp->yuvspecmode);
}

void gpCdspSetSatHue(gpCdspSatHue_t *argp)
{
	//DBG_PRINT("%s = %d\n", __FUNCTION__, argp->YbYcEn);
	hwCdsp_SetYuvSPHue(argp->u_huesindata, argp->u_huecosdata, argp->v_huesindata, argp->v_huecosdata);
	hwCdsp_SetYuvSPEffOffset(argp->y_offset, argp->u_offset, argp->v_offset);
	hwCdsp_SetYuvSPEffScale(argp->y_scale, argp->u_scale, argp->v_scale);
	hwCdsp_EnableBriCont(argp->YbYcEn);
}

void gpCdspGetSatHue(gpCdspSatHue_t *argp)
{
	gpHalCdspGetYuvSPHue(&argp->u_huesindata, &argp->u_huecosdata, &argp->v_huesindata, &argp->v_huecosdata);
	gpHalCdspGetYuvSPEffOffset(&argp->y_offset, &argp->u_offset, &argp->v_offset);
	gpHalCdspGetYuvSPEffScale(&argp->y_scale, &argp->u_scale, &argp->v_scale);
	argp->YbYcEn = gpHalCdspGetBriContEn();
}


/*************************************************************
CDSP Function
*************************************************************/
void gp_cdsp_drop_frame(INT8U drop_frame)
{
	INT8U i=0;

	cdsp_eof_occur_flag = 0;

	while (i < drop_frame)
	{
		#if 0
		if ((R_CDSP_INT & 0x10) != 0){
			R_CDSP_INT = R_CDSP_INT;
			i++;
		}
		#else
		if(cdsp_eof_occur_flag)
		{
			cdsp_eof_occur_flag = 0;
			i++;
		}
		#endif
	}
	cdsp_eof_occur_flag = 0;
}

//========================================================================================
void cdsp_ae_awb_stop(void)
{
	if (!CdspDev) {
		return;
	}
	
	if (CdspDev->ae_workmem){
		gp_free(CdspDev->ae_workmem);
		CdspDev->ae_workmem = NULL;
		ae = NULL;
		DBG_PRINT("\r\n ae workmen free! \r\n");
	}

	if (CdspDev->awb_workmem){
		gp_free(CdspDev->awb_workmem);
		CdspDev->awb_workmem = NULL;
		awb = NULL;
		DBG_PRINT(" aw workmen free! \r\n");
	}
}


void cdsp_ae_awb_init(void)
{
	//sensor_exposure_t *p_seInfo, seInfo, seInfo2;
	
	//INT32U ae_workmem_size, awb_workmem_size;
	
	sensor_calibration_t *p_seCalibration;
	
	//jxh22_get_senInfo2(&seInfo2);	//seInfo2 memcpy use

	if (CdspDev == NULL){
		return;
	}

#if  (USE_SENSOR_NAME == SENSOR_SOI_H22)
	p_seInfo = jxh22_get_senInfo();
	p_seCalibration	= jxh22_get_calibration();
#elif  (USE_SENSOR_NAME == SENSOR_GC1004)
	p_seInfo = gc1004_get_senInfo();
	p_seCalibration = gc1004_get_calibration();
#elif  (USE_SENSOR_NAME == SENSOR_OV9712)
	p_seInfo = ov9712_get_senInfo();
	p_seCalibration = ov9712_get_calibration();	
#endif
	
	CdspDev->getSensorInfo = 0;
	CdspDev->awb_ret_flag = -1;
		
	/* create kernel thread for AE/AWB */
#if 0
	CdspDev->ae_workmem = (unsigned char *)gp_malloc(gp_cdsp_ae_get_workmem_size());		
	
	if (CdspDev->ae_workmem == 0)
	{
		DBG_PRINT("ae_workmem alloc Err!\r\n");
	}
	
	CdspDev->awb_workmem = (unsigned char *) gp_malloc(gp_cdsp_awb_get_workmem_size());
	
	if (CdspDev->awb_workmem == 0)
	{
		DBG_PRINT("awb_workmem alloc Err!\r\n");
	}
#endif	
	// aeawb initial
	CdspDev->awbmode = AWB_AUTO_CVR;
	gp_cdsp_aeawb_init(CdspDev->ae_workmem, CdspDev->awb_workmem, AWB_AUTO_CVR);
	
	
	CdspDev->ae_awb_flag = 0;
	CdspDev->low_lum_switch_cnt = 0;
	CdspDev->high_lum_switch_cnt = 0;	
	
	CdspDev->y_offset = DAYLIGHT_HUE_Y_OFFSET;
	CdspDev->u_offset = DAYLIGHT_HUE_U_OFFSET;
	CdspDev->v_offset = DAYLIGHT_HUE_V_OFFSET;
	CdspDev->y_scale = DAYLIGHT_SAT_Y_SCALE;
	CdspDev->u_scale = DAYLIGHT_SAT_U_SCALE;
	CdspDev->v_scale = DAYLIGHT_SAT_V_SCALE;
	
	CdspDev->sat_contr_idx = -1;
	
	// for IQ turning
	CdspDev->sensor_cdsp.r_b_gain= p_seCalibration->r_b_gain;
	/* not use
	CdspDev->sensor_cdsp.gamma  = p_seCalibration->gamma;
	CdspDev->sensor_cdsp.color_matrix = p_seCalibration->color_matrix;
	CdspDev->sensor_cdsp.awb_thr = p_seCalibration->awb_thr;
	*/
	
	wb_gain.gbgain = (1.0*64);
	wb_gain.grgain = (1.0*64);	
	wb_gain.global_gain = 32;	
			
#if  (USE_SENSOR_NAME == SENSOR_SOI_H22)
	gp_cdsp_awb_reset_wb_gain(awb, 55, CdspDev->sensor_cdsp.r_b_gain); //50/36:start color tmp
#elif (USE_SENSOR_NAME == SENSOR_GC1004)
	gp_cdsp_awb_reset_wb_gain(awb, 55, CdspDev->sensor_cdsp.r_b_gain); //50/36:start color tmp
#elif  (USE_SENSOR_NAME == SENSOR_OV9712)
	gp_cdsp_awb_reset_wb_gain(awb, 55, CdspDev->sensor_cdsp.r_b_gain); //50/36:start color tmp	
#endif	
	
	wb_gain.bgain = gp_cdsp_awb_get_b_gain(awb);
	wb_gain.rgain = gp_cdsp_awb_get_r_gain(awb);
	
	gp_cdsp_awb_set_r_b_gain_boundary(awb, 95, 120, 60, 60);
	
	//H22_get_exposure_time(&seInfo);
	
	if(CdspDev->getSensorInfo == 0)
	{
		gp_cdsp_ae_set_sensor_exp_time(CdspDev->ae_workmem, p_seInfo);					
		CdspDev->sensor_gain_thr = p_seInfo->max_analog_gain >> 2;

		if(CdspDev->sensor_gain_thr < (256)){
			CdspDev->sensor_gain_thr = 256;
		 }

		CdspDev->sensor_time_thr = p_seInfo->max_time >> 2;
		if(CdspDev->sensor_time_thr < (8)){
			CdspDev->sensor_time_thr = 8;
		}
		DBG_PRINT("Sensor: Gain thr = 0x%x, time thr = 0x%x\r\n", CdspDev->sensor_gain_thr, CdspDev->sensor_time_thr);
	} 
	
	CdspDev->getSensorInfo = 1;	
}

INT32U gp_ae_awb_process(void)
{
	int ae_stable = 1;		
	//unsigned char *awb;
	//unsigned char *ae;

	gpCdsp3aResult_t *p_3a_result = &CdspDev->result;	
	
	if (CdspDev == NULL){
		return -1;
	}

	if(CdspDev->getSensorInfo == 0x321)
	{
		//DBG_PRINT("\r\n\r\n=========== AE & AWB Process Init =========\r\n\r\n");
		
		//awb_frame = 0;
		//DBG_PRINT("Get Sensor Info: 0x%x\r\n", CdspDev->getSensorInfo);
			
		if (awb == NULL) {
			awb = CdspDev->awb_workmem;
		}
		
		if (ae == NULL) {
			ae = CdspDev->ae_workmem;
		}
	
		CdspDev->ae_awb_flag = 0;
		CdspDev->ae_awb_flag |= (CDSP_LUM_STATUS);
		ae_switch_flag = 3;
		
#if (USE_SENSOR_NAME == SENSOR_SOI_H22)
		CdspDev->sensor_gain_thr = (1*256);
		CdspDev->sensor_time_thr = 0x100;

		p_seInfo = jxh22_get_senInfo();
		
#elif (USE_SENSOR_NAME == SENSOR_GC1004)
		CdspDev->sensor_gain_thr = (1*256);
		CdspDev->sensor_time_thr = 0x100;

		p_seInfo = gc1004_get_senInfo();
#elif (USE_SENSOR_NAME == SENSOR_OV9712)
		CdspDev->sensor_gain_thr = (1*256);
		CdspDev->sensor_time_thr = 0x100;

		p_seInfo = ov9712_get_senInfo();
#endif
		
		gp_cdsp_ae_set_sensor_exp_time(CdspDev->ae_workmem, p_seInfo);				
		CdspDev->sensor_gain_thr = p_seInfo->max_analog_gain >> 2;
		if(CdspDev->sensor_gain_thr < (1*256)) CdspDev->sensor_gain_thr = (1*256);
		
		CdspDev->night_gain_thr = CdspDev->sensor_gain_thr << 2;

		CdspDev->sensor_time_thr = p_seInfo->max_time>>2;

		if(CdspDev->sensor_time_thr < (8)) {
			CdspDev->sensor_time_thr = 8;			
		}
				
		//DBG_PRINT("Sensor: Gain thr = 0x%x, time thr = 0x%x\r\n", CdspDev->sensor_gain_thr, CdspDev->sensor_time_thr);
		
		gp_memcpy((INT8S *)&seInfo, (INT8S *)p_seInfo, sizeof(sensor_exposure_t));
		
		CdspDev->getSensorInfo &= ~0x300;
		
		CdspDev->awb_low_lum_cnt = 0;
		
		ae_frame=0;
 		ae_switch_flag = 0;
		awb_frame=0;
		ae_frame_thr = 3;

		return 0;
	}
	else if(CdspDev->getSensorInfo == 0x21)
	{
		//DBGPRINT("\r\n\r\n=========== AE & AWB Process Start =========\r\n\r\n");
	
		//DBG_PRINT("0: AE/AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);

			//+++ AE process start
			if((CdspDev->ae_awb_flag & CDSP_AE_CTRL_EN) != 0)
			{			
				ae_frame++;
				if(ae_frame >= ae_frame_thr)
				{
					int ret;
					int i;
					unsigned int hist_lo, hist_hi;
				
					ae_frame = 0;	
					//DBG_PRINT("1: AE/AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);				
#if 1								
					//DBG_PRINT("\r\n\r\n====== AE Process Start =========\r\n\r\n");
					#if (USE_SENSOR_NAME == SENSOR_SOI_H22)	
						H22_get_exposure_time(&seInfo);
					#elif (USE_SENSOR_NAME == SENSOR_GC1004)
						gc1004_get_exposure_time(&seInfo);
					#elif (USE_SENSOR_NAME == SENSOR_OV9712)
						ov9712_get_exposure_time(&seInfo);
					#endif
				
					//memcpy(&seInfo, p_seInfo, sizeof(sensor_exposure_t));
					//DBG_PRINT("\r\n0 Time:%d, A Gain:%d, D Gain:%d\r\n", seInfo.time, seInfo.analog_gain, seInfo.digital_gain);
					//DBG_PRINT("\r\nG_Digital Gain: %x, %x, %x\r\n", seInfo.digital_gain, seInfo.max_digital_gain, seInfo.min_digital_gain);
					//DBG_PRINT("G_Analog Gain: %x, %x, %x\r\n", seInfo.analog_gain, seInfo.max_analog_gain, seInfo.min_analog_gain);
					//DBG_PRINT("Get_Time: %x, %x, %x\r\n", seInfo.time, seInfo.max_time, seInfo.min_time);
					//DBG_PRINT("Get EV idx: sensor idx = %d, night idx = %d\r\n", seInfo.sensor_ev_idx, seInfo.night_ev_idx);
					hist_lo = 0;
					hist_hi = 0;
					if((CdspDev->ae_awb_flag & CDSP_HIST_CTRL_EN) != 0)
					{
						hist_lo = CdspDev->result.hislowcnt;
						hist_hi = CdspDev->result.hishicnt;
						CdspDev->ae_awb_flag &= (~CDSP_HIST_CTRL_EN);
						//DBG_PRINT("hist_hi = %d, hist_lo = %d\n", hist_hi, hist_lo);
					}
					
					
					/*DBG_PRINT("\r\n\r\nAE win input:");
					for(i = 0 ; i < 64 ; i++)
					{
						if(i%8 == 0)DBG_PRINT("\r\n");
						DBG_PRINT(" %d,",p_3a_result->ae_win[i]);
					}*/
					
					if(ae_switch_flag == 0)
					{						
				/*	DBG_PRINT("\r\n\r\n\r\n1 AE win output:");						
						for(i = 0 ; i < 64 ; i++)
						{
							if(i%8 == 0)DBG_PRINT("\r\n");
							DBG_PRINT(" %d,",p_3a_result->ae_win[i]);
						}
						DBG_PRINT("\r\n");	*/		
						
						gp_cdsp_ae_comb_win(ae, CdspDev->ae_win_tmp, p_3a_result->ae_win, ae_switch_flag);						
						ae_switch_flag = 2;
						ret = ~AE_CHANGE;
						
						seInfo.ae_ev_idx = 0;
					}
					else if(ae_switch_flag == 2)
					{
						unsigned char ae_win_t[64], *ptr;
						unsigned short *p_ae_win_tmp;	
												
					/*	DBG_PRINT("\r\n\r\n\r\n2 AE win output:");						
						for(i = 0 ; i < 64 ; i++)
						{
							if(i%8 == 0)DBG_PRINT("\r\n");
							DBG_PRINT(" %d,",p_3a_result->ae_win[i]);
						}
						DBG_PRINT("\r\n");	*/		
						
						gp_cdsp_ae_comb_win(ae, CdspDev->ae_win_tmp, p_3a_result->ae_win, ae_switch_flag);
						ae_switch_flag= 0;						
						
					#if SENSOR_FLIP//flip
						ptr = ae_win_t;
						p_ae_win_tmp = &CdspDev->ae_win_tmp[56];
						i = 8;
						do {
							int j;
							unsigned short t;
							j = 8;
							do {
								t = *p_ae_win_tmp++;
								*ptr++ = t;
								j--;
							} while(j != 0);
							
							p_ae_win_tmp -= 16;
														
							i--;
						} while(i != 0);
					#else 	// normal		
						ptr = ae_win_t;
						p_ae_win_tmp = CdspDev->ae_win_tmp;
						i = 64;
						do {
							*ptr++ = *p_ae_win_tmp++;
							
							i--;
						} while(i != 0);
					
					#endif
						
				/*		DBG_PRINT("\r\n\r\n\r\nAE win output:");
						for(i = 0 ; i < 64 ; i++)
						{
							if(i%8 == 0)DBG_PRINT("\r\n");
							DBG_PRINT(" %d,",ae_win_t[i]);
						}
						DBG_PRINT("\r\n");*/
						
						
						ret = gp_cdsp_ae_calc_exp(ae,ae_win_t, &seInfo, hist_hi, hist_lo);		
						seInfo.ae_ev_idx = gp_cdsp_ae_get_result_ev(ae);				
					}
					else
					{
						//DBG_PRINT("\r\n\r\n\r\nAE win output:");
						/*
						for(i = 0 ; i < 64 ; i++)
						{
							if(i%8 == 0)DBG_PRINT("\r\n");
							DBG_PRINT(" %d,",p_3a_result->ae_win[i]);
						}
						DBG_PRINT("\r\n");
						*/
						ret = gp_cdsp_ae_calc_exp(ae, p_3a_result->ae_win, &seInfo, hist_hi, hist_lo);
						seInfo.ae_ev_idx = gp_cdsp_ae_get_result_ev(ae);
					}
					
					if(seInfo.ae_ev_idx == 0)ae_frame_thr = 2;
					else ae_frame_thr = 3;
					
									
					//DBG_PRINT("\r\nGet lum = 0x%x\r\n", gp_cdsp_ae_get_result_lum(ae));
					if(seInfo.ae_ev_idx != 0)
					{
						int current_ev_idx = seInfo.sensor_ev_idx;
						int night_ev_idx = seInfo.night_ev_idx;
						
						gp_memcpy((INT8S *)&seInfo2, (INT8S *)&seInfo, sizeof(sensor_exposure_t));
						CdspDev->ae_ret = ret;
						
																		
										
						if(current_ev_idx >= sat_yuv_thr[3])
						{ 	// low light
							ae_ev = 3;
							
							//DBG_PRINT("ae_awb_flag = 0x%x, low_lum_switch_cnt = %d\r\n", CdspDev->ae_awb_flag, CdspDev->low_lum_switch_cnt);
							if(denoise_level != NEW_DENOISE_ONLY)
							//if(denoise_level != HYBRID_DENOISE_ONLY)
							{
								CdspDev->low_lum_switch_cnt++;							
								if(CdspDev->low_lum_switch_cnt >= 10)
								{
									ae_frame_thr = 3;
									ae_switch_flag = 0;	// for night
									CdspDev->low_lum_switch_cnt = 0;
									CdspDev->high_lum_switch_cnt = 0;
									denoise_level = NEW_DENOISE_ONLY;//HYBRID_DENOISE_ONLY;
									
									CdspDev->ae_awb_flag &=( ~(CDSP_LUM_STATUS | CDSP_HIGH_LUM | CDSP_LOW_LUM));
									CdspDev->ae_awb_flag |= (CDSP_LUM_STATUS | CDSP_LOW_LUM);
									
									//DBG_PRINT("low lum\r\n");		
								}
							}
							else
								CdspDev->low_lum_switch_cnt = 0;
						}
						else if(current_ev_idx > sat_yuv_thr[0] && current_ev_idx < sat_yuv_thr[3])
						{							
							ae_ev = 4;
							if(denoise_level != NEW_DENOISE_ONLY)
							//if(denoise_level != HYBRID_DENOISE_ONLY)							
							{
								CdspDev->low_lum_switch_cnt++;							
								if(CdspDev->low_lum_switch_cnt >= 8)
								{
									ae_frame_thr = 3;
									ae_switch_flag = 0;	// for night
									CdspDev->low_lum_switch_cnt = 0;
									CdspDev->high_lum_switch_cnt = 0;
									denoise_level = NEW_DENOISE_ONLY;//HYBRID_DENOISE_ONLY;
									
									CdspDev->ae_awb_flag &=( ~(CDSP_LUM_STATUS | CDSP_HIGH_LUM | CDSP_LOW_LUM));
									CdspDev->ae_awb_flag |= (CDSP_LUM_STATUS | CDSP_LOW_LUM);
								}
							}
							else
								CdspDev->low_lum_switch_cnt = 0;
						}
						else if(current_ev_idx <= (night_ev_idx - 15))					
						{ 	// Daylight
							ae_ev = 5;
							//DBG_PRINT("ae_awb_flag = 0x%x, high_lum_switch_cnt = %d\r\n", CdspDev->ae_awb_flag, CdspDev->high_lum_switch_cnt);
							if((CdspDev->ae_awb_flag & CDSP_HIGH_LUM) == 0)
							{
								CdspDev->high_lum_switch_cnt++;
							
								if(CdspDev->high_lum_switch_cnt >= 5) 
								{
									ae_frame_thr = 3;
									ae_switch_flag = 3; // for daylight
									denoise_level = DENOISE_DISABLE;
									CdspDev->high_lum_switch_cnt = 0;
									CdspDev->low_lum_switch_cnt = 0;
									
									CdspDev->ae_awb_flag &= (~(CDSP_LUM_STATUS | CDSP_HIGH_LUM | CDSP_LOW_LUM));
									CdspDev->ae_awb_flag |= (CDSP_LUM_STATUS | CDSP_HIGH_LUM);									
									//DBG_PRINT("high lum\n\r");								
								}	
							}
						}
						else
						{
							CdspDev->low_lum_switch_cnt--;
							if(CdspDev->low_lum_switch_cnt < 0) CdspDev->low_lum_switch_cnt = 0;
							CdspDev->high_lum_switch_cnt--;
							if(CdspDev->high_lum_switch_cnt < 0) CdspDev->high_lum_switch_cnt = 0;
						}
					
					
					
						if(current_ev_idx >= sat_yuv_thr[3] && CdspDev->sat_contr_idx != 3)
						{							
							CdspDev->sat_contr_idx = 3;
							CdspDev->ae_awb_flag |= CDSP_SAT_SWITCH;
							//DBG_PRINT("sat_contr_idx = %d\r\n", CdspDev->sat_contr_idx);
						}
						else if(current_ev_idx < sat_yuv_thr[3] && current_ev_idx >= sat_yuv_thr[2] && CdspDev->sat_contr_idx != 2)
						{							
							CdspDev->sat_contr_idx = 2;
							CdspDev->ae_awb_flag |= CDSP_SAT_SWITCH;
							//DBG_PRINT("sat_contr_idx = %d\r\n", CdspDev->sat_contr_idx);
						}
						else if(current_ev_idx < sat_yuv_thr[2] && current_ev_idx >= sat_yuv_thr[1] && CdspDev->sat_contr_idx != 1)
						{							
							CdspDev->sat_contr_idx = 1;
							CdspDev->ae_awb_flag |= CDSP_SAT_SWITCH;
							//DBG_PRINT("sat_contr_idx = %d\r\n", CdspDev->sat_contr_idx);
						}
						else if(current_ev_idx < sat_yuv_thr[1] && CdspDev->sat_contr_idx != 0)
						{							
							CdspDev->sat_contr_idx = 0;
							CdspDev->ae_awb_flag |= CDSP_SAT_SWITCH;
							//DBG_PRINT("sat_contr_idx = %d\r\n", CdspDev->sat_contr_idx);
						}
						else if(current_ev_idx < sat_yuv_thr[0] && CdspDev->sat_contr_idx != -1)
						{
							CdspDev->sat_contr_idx = -1;
							CdspDev->ae_awb_flag |= CDSP_SAT_SWITCH;
						}
					
						//DBG_PRINT("\r\n1 Time:%d, A Gain:%d, D Gain:%d\r\n", seInfo2.time, seInfo2.analog_gain, seInfo2.digital_gain);
						//DBG_PRINT("0S_analog_gain = 0x%x, S_sensor_gain_thr = 0x%x\r\n", seInfo.analog_gain,  CdspDev->sensor_gain_thr);
					
						ae_stable = 0;
					}
					else
						ae_stable = 1;	
#endif
					//DBG_PRINT("ae_awb_flag = 0x%x\r\n", CdspDev->ae_awb_flag);
				}

				//ae_stable = 1;

				CdspDev->ae_awb_flag &= (~CDSP_AE_CTRL_EN);
				//DBG_PRINT("AE process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
			}	
			//--- AE process end

			//+++ AWB process start
			if( (CdspDev->ae_awb_flag & CDSP_AWB_CTRL_EN) != 0) 
			{		
				awb_frame++;
				//if(ae_stable == 1 && awb_frame >= 2)
				if(awb_frame >= 2)
				{
					//gpCdspWbGain2_t wb_gain2;
					int ret, lum;
									
					awb_frame = 0;
					
					//DBG_PRINT("2: AE/AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
	#if 1
					//DBG_PRINT("\r\n\r\n====== AWB Process Start =========\r\n\r\n");
					/*
					DBG_PRINT("======\r\n");
					DBG_PRINT("sumcnt[0] = 0x%010lx\r\n", p_3a_result->awb_result.sumcnt[0]);
					DBG_PRINT("sumcnt[1] = 0x%010lx\r\n", p_3a_result->awb_result.sumcnt[1]);
					DBG_PRINT("sumcnt[2] = 0x%010lx\r\n", p_3a_result->awb_result.sumcnt[2]);
					DBG_PRINT("******\r\n");
					DBG_PRINT("sumg[0] = 0x%010lx\r\n", p_3a_result->awb_result.sumg[0]);
					DBG_PRINT("sumg[1] = 0x%010lx\r\n", p_3a_result->awb_result.sumg[1]);
					DBG_PRINT("sumg[2] = 0x%010lx\r\n", p_3a_result->awb_result.sumg[2]);
					DBG_PRINT("sumrg[0] = 0x%010lx\r\n", p_3a_result->awb_result.sumrg[0]);
					DBG_PRINT("sumrg[1] = 0x%010lx\r\n", p_3a_result->awb_result.sumrg[1]);
					DBG_PRINT("sumrg[2] = 0x%010lx\r\n", p_3a_result->awb_result.sumrg[2]);
					DBG_PRINT("sumbg[0] = 0x%010lx\r\n", p_3a_result->awb_result.sumbg[0]);
					DBG_PRINT("sumbg[1] = 0x%010lx\r\n", p_3a_result->awb_result.sumbg[1]);
					DBG_PRINT("sumbg[2] = 0x%010lx\r\n", p_3a_result->awb_result.sumbg[2]);
					*/
					
					//DBG_PRINT("\nawb sum cnt = %d\r\n",  p_3a_result->awb_result.sumcnt[0] +  p_3a_result->awb_result.sumcnt[1] +  p_3a_result->awb_result.sumcnt[2]);
					
					ret = gp_cdsp_awb_calc_gain(awb, &p_3a_result->awb_result, CdspDev->sensor_cdsp.r_b_gain); 
					CdspDev->awb_ret_flag = ret;
					
					lum = gp_cdsp_ae_get_result_lum(ae);
					if((ret == AWB_FAIL) && ((CdspDev->ae_awb_flag & CDSP_LOW_LUM) != 0) && (lum <= TARGET_Y_LOW))
					{
						CdspDev->awb_low_lum_cnt++;
						
						if(CdspDev->awb_low_lum_cnt >= 3)
						{
							// could be night)
							gp_cdsp_awb_reset_wb_gain(awb, 40, CdspDev->sensor_cdsp.r_b_gain);
							
							
							wb_gain.bgain = gp_cdsp_awb_get_b_gain(awb);
							wb_gain.rgain = gp_cdsp_awb_get_r_gain(awb);
							
							CdspDev->ae_awb_flag |= CDSP_AWB_SET_GAIN;
							
							//DBG_PRINT("AWB low light\r\n");
							CdspDev->awb_low_lum_cnt = -512;
						}
					}
					else if(ret != AWB_FAIL && ret != AWB_RET && CdspDev->awb_low_lum_cnt > 0)
					{
						CdspDev->awb_low_lum_cnt = 0;
					}
					
					//DBG_PRINT("\nawb sum cnt = %d\r\n",  p_3a_result->awb_result.sumcnt[0] +  p_3a_result->awb_result.sumcnt[1] +  p_3a_result->awb_result.sumcnt[2]);
					if(ret != AWB_FAIL || ret != AWB_RET)
					{
						wb_gain.bgain = gp_cdsp_awb_get_b_gain(awb);
						wb_gain.rgain = gp_cdsp_awb_get_r_gain(awb);
						
						CdspDev->ae_awb_flag |= CDSP_AWB_SET_GAIN;
						//CdspDev -> awb_ret_flag = -1;
					}						
					
											
					#if 1
					if(ret != AWB_RET)
					{
						int awbmode;
						awbmode = gp_cdsp_awb_get_mode(awb);
						//DBG_PRINT("awb mode = 0x%x\r\n", awbmode);
						//if(ret != AWB_SUCCESS_CVR)
						if(awbmode == AWB_AUTO_CVR || awbmode == AWB_AUTO_CVR_DAYLIGHT || awbmode == AWB_AUTO_CVR_NIGHT)
						{
							//DBG_PRINT("SensorExposure: time = 0x%x, analogGain = 0x%x, digitalGain = 0x%x\r\n", p_seInfo->time, p_seInfo->analog_gain, p_seInfo->digital_gain);
							//DBG_PRINT("SensorExposure: time = 0x%x, analogGain = 0x%x, digitalGain = 0x%x\r\n", seInfo.time, seInfo.analog_gain, seInfo.digital_gain);
							//DBG_PRINT("Thr: gain = 0x%x, time = 0x%x\r\n", CdspDev->night_gain_thr, CdspDev->sensor_time_thr);
						
							// Auto mode for CVR
							//if(p_seInfo->analog_gain > CdspDev->night_gain_thr) 
								
							if(gp_cdsp_ae_is_night(ae) == 1)  
							{
								//DBG_PRINT("AWB Night\r\n");
								awbmode = AWB_AUTO_CVR_NIGHT;
						
								gp_cdsp_awb_set_ct_offset(awb, NIGHT_WB_OFFSET); // +: warm,   -: cold , +10~-10	
							}
							//else if(p_seInfo->time < CdspDev->sensor_time_thr)
							else if(seInfo.time < CdspDev->sensor_time_thr)
							{
								//DBG_PRINT("AWB Daylight\r\n");
								awbmode = AWB_AUTO_CVR_DAYLIGHT;
					
								gp_cdsp_awb_set_ct_offset(awb, DAYLIGHT_WB_OFFSET); // +: warm,   -: cold , +10~-10		
							}
							else 				
							{
								//DBG_PRINT("AWB Auto\r\n");
								awbmode = AWB_AUTO_CVR;
								
							/*	#if (USE_SENSOR_NAME == SENSOR_GC1004)
								gp_cdsp_awb_set_r_gain_factor(awb, 1.11);
								gp_cdsp_awb_set_b_gain_factor(awb, 0.95);
								#endif*/
								
								gp_cdsp_awb_set_ct_offset(awb, AUTO_WB_OFFSET); // +: warm,   -: cold , +10~-10
							}
							awbmode = AWB_AUTO_CVR;
							
							gp_cdsp_awb_set_mode(awb, awbmode);
							gp_cdsp_awb_calc_gain(awb, &p_3a_result->awb_result, CdspDev->sensor_cdsp.r_b_gain);
							//DBG_PRINT("awb color tmp = %d\r\n", gp_cdsp_awb_get_color_temperature(awb));
							//DBG_PRINT("RG = %d, BG = %d\r\n", wb_gain.rgain, wb_gain.bgain);
							//DBG_PRINT("calc color tmp = %d\r\n", gp_cdsp_awb_get_calc_color_temperature(awb));
						}
					}
					#endif
					wb_gain2.wbgain2en = ENABLE;
					wb_gain2.rgain2 = gp_cdsp_awb_get_r_gain2(awb);
					wb_gain2.bgain2 = gp_cdsp_awb_get_b_gain2(awb);
					wb_gain2.ggain2 = 64;					
					gpCdspSetWBGain2(&wb_gain2);
					
					//color tmp
					seInfo2.set_color_tmp = gp_cdsp_awb_get_color_temperature(awb);	
					//DBG_PRINT("rgain2 = 0x%x, bgain2 = 0x%x\r\n", wb_gain2.rgain2 , wb_gain2.bgain2);
					
	#endif				
				}

				CdspDev->ae_awb_flag &= (~CDSP_AWB_CTRL_EN);
				//DBG_PRINT("AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
			}
			//DBG_PRINT("3: AE/AWB process, flag = 0x%x\r\n", CdspDev->ae_awb_flag);
	
		//DBG_PRINT("\r\n\r\n=========== AE & AWB Process Stop =========\r\n\r\n");

		return 0;
	}
	else 
	{
		//DBG_PRINT("Err: Not Get Sensor Info: 0x%x\r\n", CdspDev->getSensorInfo);
		DBG_PRINT("Er.");
		return -1;
	}
}


void gp_cdsp_set_awb_by_config(INT32U awb_value)
{
	INT32U temp;
	temp = awb_value;
	if(temp == 0){
		temp = AWB_AUTO_CVR;
	}else if(temp == 1){
		temp = AWB_AUTO_CVR_DAYLIGHT;
	}else if(temp == 2){
		temp = AWB_AUTO_CVR_CLOUDY;
	}else if(temp == 3){
		temp = AWB_LAMP;
	}else if(temp == 4){
		temp = AWB_FLASH;
	}else{
		temp = AWB_AUTO_CVR;
	}
	gp_cdsp_set_awb_mode(temp); //AWB_AUTO_CVR,AWB_AUTO_CVR_DAYLIGHT,AWB_AUTO_CVR_CLOUDY,AWB_LAMP,AWB_FLASH


}

void ap_iso_set_by_config(INT32U iso)
{
	INT32U temp;
	temp = iso;
	if(temp == 0){
		temp = ISO_AUTO;
	}else if(temp == 1){
		temp = 100;
	}else if(temp == 2){
		temp = 200;
	}else if(temp == 3){
		temp = 400;
	}else if(temp == 4){
		temp = 800;
	}else{
		temp = ISO_AUTO;
	}
	ap_iso_set(temp); //iso init, 100,200,400,800,ISO_AUTO	
	
	
}

void ap_edge_set_by_config(INT8U edge)
{
	INT8U temp;
	if(edge == 0){
		temp = 3;
	}else if(edge == 1){
		temp = 2;
	}else if(edge == 2){
		temp = 1;
	}else if(edge == 3){
		temp = 1;
	}else{
		temp = 2;
	}
	gp_cdsp_set_edge(temp);	//tmp: 0: soft, 1:standard, 2: sharp, 3: more sharp
}
/*****************************************************
Save RAW Data Function
*****************************************************/
#if ENABLE_SAVE_SENSOR_RAW_DATA

void save_raw10(INT32U g_frame_addr,INT32U capture_mode)
{

	capture_mode = LENSCMP;//0x10;//0x11//0x12
	
	hwCdsp_DataSource(SDRAM_INPUT);	//Sensor data input
	
	hwCdsp_SetInt(0,CDSP_INT_ALL); //off all Interupt
	/* switch capture mode*/
	//hwCdsp_SetRawPath(raw_mode, cap_mode, yuv_mode);   

	//Set RAW Buffer Address
	hwCdsp_SetRaw10Buff(1280, 720, 0, g_frame_addr);
	//hwCdsp_SetRaw10Buff(width, height, 0, g_frame_addr[1]);
	hwIsp_LenCmp(DISABLE, 0);
	hwCdsp_SetSRAM(ENABLE, 0xA0);
	hwCdsp_EnableClamp(ENABLE, 1280);
	
	//read back 
	hwCdsp_SetReadBackSize10(0x0, 0x0, 1280, 720);
	//hwCdsp_SetReadBackSize10(0x08, 0x0, width, height);
	//hwCdsp_SetReadBackSize10(hoffset, voffset, hsize, vsize);
	
	switch (capture_mode)
	{
		case LENSCMP:
	 		hwCdsp_SetRawPath(0x01, 0x0, 0x0);   //0x240 = 0x01;	//10 bit to 16bit, From LensCmp
		break;
 	
		case WBGAIN:
			hwCdsp_SetRawPath(0x03, 0x0, 0x0);   //0x240 = 0x03;	//10 bit to 16bit, From WBGain
		break;

		case LUTGAMMA:
			hwCdsp_SetRawPath(0x05, 0x0, 0x0);   //0x240 = 0x05	//10 bit to 16bit, From LutGamma
		break;

		default:
			DBG_PRINT("RAW PATH not selsect! \r\n");
		break;
	}
	
	DBG_PRINT("Getting Sensor Data\r\n");		//imgsorce from front

	hwCdsp_DataSource(FRONT_INPUT);

}

void cdsp_yuyv_restart(INT32U g_frame_addr)
{
	hwCdsp_SetRawPath(0, 0, 1);
	
	//Set YUV Buffer Addr
	hwCdsp_SetYuvBuffA(1280, 720, g_frame_addr);

	hwCdsp_SetSRAM(ENABLE, 0xA0);
	hwCdsp_EnableClamp(ENABLE, 1280);
	
	hwCdsp_SetOutYUVFmt(YVYU);
}
#endif


void gp_cdsp_sat_contrast_init(void)
{
	int step;
	
	// y offset
	step = (abs(DAYLIGHT_HUE_Y_OFFSET - NIGHT_HUE_Y_OFFSET) + 2) >> 2;
	if(DAYLIGHT_HUE_Y_OFFSET > NIGHT_HUE_Y_OFFSET) step = -step;
	
	sat_yuv_level[0][0] = DAYLIGHT_HUE_Y_OFFSET;
	sat_yuv_level[1][0] = DAYLIGHT_HUE_Y_OFFSET + step;
	sat_yuv_level[2][0] = NIGHT_HUE_Y_OFFSET - step;
	sat_yuv_level[3][0] = NIGHT_HUE_Y_OFFSET;
	
	// u offset
	step = (abs(DAYLIGHT_HUE_U_OFFSET - NIGHT_HUE_U_OFFSET) + 2) >> 2;
	if(DAYLIGHT_HUE_U_OFFSET > NIGHT_HUE_U_OFFSET) step = -step;
	
	sat_yuv_level[0][1] = DAYLIGHT_HUE_U_OFFSET;
	sat_yuv_level[1][1] = DAYLIGHT_HUE_U_OFFSET + step;
	sat_yuv_level[2][1] = NIGHT_HUE_U_OFFSET - step;
	sat_yuv_level[3][1] = NIGHT_HUE_U_OFFSET;
	
	// v offset
	step = (abs(DAYLIGHT_HUE_V_OFFSET - NIGHT_HUE_V_OFFSET) + 2) >> 2;
	if(DAYLIGHT_HUE_V_OFFSET > NIGHT_HUE_V_OFFSET) step = -step;
	
	sat_yuv_level[0][2] = DAYLIGHT_HUE_V_OFFSET;
	sat_yuv_level[1][2] = DAYLIGHT_HUE_V_OFFSET + step;
	sat_yuv_level[2][2] = NIGHT_HUE_V_OFFSET - step;
	sat_yuv_level[3][2] = NIGHT_HUE_V_OFFSET;
	
	
	// y scale
	step = (abs(DAYLIGHT_SAT_Y_SCALE - NIGHT_SAT_Y_SCALE) + 2) >> 2;
	if(DAYLIGHT_SAT_Y_SCALE > NIGHT_SAT_Y_SCALE) step = -step;
	
	sat_yuv_level[0][3] = DAYLIGHT_SAT_Y_SCALE;
	sat_yuv_level[1][3] = DAYLIGHT_SAT_Y_SCALE + step;
	sat_yuv_level[2][3] = NIGHT_SAT_Y_SCALE - step;
	sat_yuv_level[3][3] = NIGHT_SAT_Y_SCALE;
	
	// u scale
	step = (abs(DAYLIGHT_SAT_U_SCALE - NIGHT_SAT_U_SCALE)+ 2) >> 2;
	if(DAYLIGHT_SAT_U_SCALE > NIGHT_SAT_U_SCALE) step = -step;
	
	sat_yuv_level[0][4] = DAYLIGHT_SAT_U_SCALE;
	sat_yuv_level[1][4] = DAYLIGHT_SAT_U_SCALE + step;
	sat_yuv_level[2][4] = NIGHT_SAT_U_SCALE - step;
	sat_yuv_level[3][4] = NIGHT_SAT_U_SCALE;
	
	// v scale
	step = (abs(DAYLIGHT_SAT_V_SCALE - NIGHT_SAT_V_SCALE) + 2) >> 2;
	if(DAYLIGHT_SAT_V_SCALE > NIGHT_SAT_V_SCALE) step = -step;
	
	sat_yuv_level[0][5] = DAYLIGHT_SAT_V_SCALE;
	sat_yuv_level[1][5] = DAYLIGHT_SAT_V_SCALE + step;
	sat_yuv_level[2][5] = NIGHT_SAT_V_SCALE - step;
	sat_yuv_level[3][5] = NIGHT_SAT_V_SCALE;	
	
	
	// thr
#if (USE_SENSOR_NAME == SENSOR_SOI_H22)		
	step = sat_yuv_thr[3] = H22_get_max_ev_idx() - 4;
	sat_yuv_thr[0] = H22_get_night_ev_idx();	
#elif (USE_SENSOR_NAME == SENSOR_OV9712)	
	step = sat_yuv_thr[3] = ov9712_get_max_ev_idx() - 4;
	sat_yuv_thr[0] = ov9712_get_night_ev_idx();	
#elif (USE_SENSOR_NAME == SENSOR_GC1004)	
	step = sat_yuv_thr[3] = gc1004_get_max_ev_idx() - 4;
	sat_yuv_thr[0] = gc1004_get_night_ev_idx();	
#endif
	step -= sat_yuv_thr[0];
	step = (step + 2) >> 2;	
	sat_yuv_thr[1] = sat_yuv_thr[0] + step;
	sat_yuv_thr[2] = sat_yuv_thr[3] - step;
	
	{
		int i, j;
		DBG_PRINT("\r\n\r\nsat constrast level:\r\n");
		for(i = 0;i<4;i++)
		{
			DBG_PRINT("Thr[%d]: YUV = ", sat_yuv_thr[i]);
			for(j = 0;j<6;j++)	DBG_PRINT(" %d,", sat_yuv_level[i][j]);
			
			DBG_PRINT("\r\n");
		}
	}
	
}

void gp_isp_start(INT32U dummy_addr,INT32U gpSENSOR_WIDTH, INT32U gpSENSOR_HEIGHT)	
{
	INT32U read_width, read_height;
	INT32U clamp_width, clamp_en;
	//INT8U clip_tmp;

	CdspDev = &gp_CdspDev;
	gp_memset((INT8S*) CdspDev, 0, sizeof(gpCdspDev_t));
	
	//hwCdsp_Enable3ATestWin(1, 1);
	
	
	ae_exp_set_cnt = 0;
	CdspDev->awb_ret_flag = -1;
 	CdspDev->ae_ret = -1;
	CdspDev->ae_workmem = ae_workszie;
	CdspDev->awb_workmem = awb_worksize;
	if (awb == NULL) {
		awb = CdspDev->awb_workmem;
	}

	if (ae == NULL) {
		ae = CdspDev->ae_workmem;
	}
	
		
	CdspDev->aeAddr[0] = &cdsp_ae_addr_a[0];
	CdspDev->aeAddr[1] = &cdsp_ae_addr_b[0];
	DBG_PRINT("\r\nae_work_buffer = 0x%x, awb_work_buffer = 0x%x\r\n",CdspDev->ae_workmem, CdspDev->awb_workmem);
	DBG_PRINT("\r\nae_addr[0] = 0x%x, ae_addr[1] = 0x%x\r\n",CdspDev->aeAddr[0], CdspDev->aeAddr[1]);

	hwCdsp_Reset();
	hwFront_Reset();
	
	hwIsp_Hr0(0);
	hwIsp_Hr1(0);
	hwIsp_Hr2(0);
	hwIsp_Hr3(0);
	
#if 1
	hwCdsp_SetAEBuffAddr(0xF8500000, 0xF8500000);
	R_CDSP_DMA_YUVABUF_SADDR = 0xF8500000;
	R_CDSP_DMA_YUVBBUF_SADDR = 0xF8500000;
	R_CDSP_MD_DMA_SADDR = 0xF8500000;
#endif
	hwCdsp_DataSource(FRONT_INPUT);
	//init sensor
	hwCdsp_SetYuvRange(0x0); 				//unsigned
	hwFront_SetInputFormat(SENSOR_RAW10);	//set cdsp input format
	SensorIF_Setting(C_RAW_FMT);			//set sensor input foramt clk

#if CDSP_DEBUG == 0
	/* --- Init SRAM Data ---*/
	#if (USE_SENSOR_NAME == SENSOR_SOI_H22)
	SOi_h22_sensor_calibration();
	#elif (USE_SENSOR_NAME == SENSOR_GC1004)
	gc1004_sensor_calibration();
	#elif (USE_SENSOR_NAME == SENSOR_OV9712)
	ov9712_sensor_calibration();
	#else
	DBG_PRINT("Non calibration sensor\r\n");
	#endif
#endif	
	hwCdsp_EnableAF(0, 0);
	hwCdsp_EnableAE(0, 0);
	hwCdsp_EnableAWB(1, 0);
	
	read_width = gpSENSOR_WIDTH - 8;
	read_height = gpSENSOR_HEIGHT - 4;
	clamp_width = gpSENSOR_WIDTH - 8;
	clamp_en = 0;	

	/*--- Bad Pixel ---*/
	//hwCdsp_SetBadPixel(160,160,160);
	hwCdsp_EnableBadPixel(ENABLE, 1, 1);

	/*--- YUV Lens compesation ---*/
	hwCdsp_EnableLensCmp(DISABLE, 0);	
	//disable when use YUV data input
	hwCdsp_SetLensCmpPath(0);
#if CDSP_DEBUG == 1
	/*--- ISP LSC ---*/
	hwIsp_LenCmp(DISABLE, 0);
	/*--- wb gain config ---*/
	gp_cdsp_set_wb_gain(DISABLE, DISABLE);

	/*-- Gamma --*/
	hwCdsp_EnableLutGamma(DISABLE);

	/*--Color matrix --*/	
	hwCdsp_EnableColorMatrix(DISABLE);	
#else
	/*--- ISP LSC ---*/
	//hwIsp_LenCmp(ENABLE, 16);
	hwIsp_LenCmp(ENABLE, 19);
	/*--- wb gain config ---*/
	gp_cdsp_set_wb_gain(ENABLE, ENABLE);

	/*-- Gamma --*/
	hwCdsp_EnableLutGamma(ENABLE);

	/*--Color matrix --*/	
	hwCdsp_EnableColorMatrix(ENABLE);	
#endif

	/*--Edge--*/
	/*new denoise config*/
	gp_cdsp_set_new_denoise(2);
	/*set Hybrid Denoise*/
	IspHybridRaw.IspHeight = gpSENSOR_HEIGHT;
	IspHybridRaw.ISPWidth = gpSENSOR_WIDTH;
	IspHybridRaw.DenoiseEnable = ENABLE;//DISABLE;
	IspHybridRaw.DPCDefaultMode = 1;
	IspHybridRaw.DPCn = 1;			
	IspHybridRaw.DefectPixelSel = 0;
	IspHybridRaw.DefectPixelEnable = DISABLE;
	IspHybridRaw.CrosstalkEnable =  DISABLE; 

	IspHybridRaw.DPCth1 = 55; //55 -> 66, weak -> strong
   	IspHybridRaw.DPCth2 = 76; //76 -> 55 -> 43, weak -> medium -> strong
   	IspHybridRaw.DPCth3 = 4; //if 3 line be filter

#if (HYBRID_DENOISE_LEVEL == WEAK_DENOISE)		
	IspHybridRaw.DenoiseT1 = 0; 
	IspHybridRaw.DenoiseT2 = 8; 
	IspHybridRaw.DenoiseT3 = 16; 
	IspHybridRaw.DenoiseT4 = 32; 
	IspHybridRaw.DenoiseW1 = 1; 
	IspHybridRaw.DenoiseW2 = 2; 
	IspHybridRaw.DenoiseW3 = 3;
#elif (HYBRID_DENOISE_LEVEL == MEDIUM_DENOISE)
	IspHybridRaw.DenoiseT1 = 8; 
	IspHybridRaw.DenoiseT2 = 16; 
	IspHybridRaw.DenoiseT3 = 32; 
	IspHybridRaw.DenoiseT4 = 48; 
	IspHybridRaw.DenoiseW1 = 2; 
	IspHybridRaw.DenoiseW2 = 4; 
	IspHybridRaw.DenoiseW3 = 6;
#elif (HYBRID_DENOISE_LEVEL == STRONG_DENOISE)
	IspHybridRaw.DenoiseT1 = 16; 
	IspHybridRaw.DenoiseT2 = 24; 
	IspHybridRaw.DenoiseT3 = 48; 
	IspHybridRaw.DenoiseT4 = 64; 
	IspHybridRaw.DenoiseW1 = 1; 
	IspHybridRaw.DenoiseW2 = 2; 
	IspHybridRaw.DenoiseW3 = 3; 
#endif

	IspHybridRaw.CrosstalkT1 = 0; 
	IspHybridRaw.CrosstalkT2 = 0; 
	IspHybridRaw.CrosstalkT3 = 0; 
	IspHybridRaw.CrosstalkT4 = 16; 
	IspHybridRaw.CrosstalkW1 = 0;
	IspHybridRaw.CrosstalkW2 = 0; 
	IspHybridRaw.CrosstalkW3 = 0; 
	IspHybridRaw.CrosstalkGbGr = 2;
	
	gp_isp_set_hybridraw(&IspHybridRaw);
#if 1
	//Enable raw interpolution
	hwCdsp_SetExtLine(ENABLE, gpSENSOR_WIDTH, 0x28);
	hwCdsp_IntplThrSet(24, 240);
	hwCdsp_EnableIntplMir(0x0F, 1, 0);	//intplmirvsel,	//suggest set 1
	read_width += 4;
	read_height += 4;
	clamp_width += 4;
#endif
#if 1
	//For YUV Horizontal Average method and LP Filiter temp
	hwCdsp_SetYuvHAvg(3, 0, 0, 0);	
	read_width += 4;
	clamp_width += 4;	
#endif
#if CDSP_DEBUG == 1
	//For h and v scale down, must be 2 align. 
	//dummy_addr = 0x600000;
	{
		INT32U hfactor, vfactor;
		hfactor = (320 << 16)/gpSENSOR_WIDTH + 1;
		vfactor = (240 << 16)/gpSENSOR_HEIGHT + 1;
		clamp_en = 1;
		clamp_width = 320;	//(320 - 4) is for not enable other function
		hwCdsp_SetYuvHScale(hfactor, hfactor);
		hwCdsp_SetYuvVScale(vfactor, vfactor);
		hwCdsp_EnableYuvHScale(ENABLE, 1);
		hwCdsp_EnableYuvVScale(ENABLE, 1);
		read_width = 320;
		read_height = 240;
	}
#endif

	hwCdsp_SetSRAM(ENABLE, 0xA0);
	hwCdsp_EnableClamp(clamp_en, clamp_width);
	
	hwCdsp_SetYuvBuffA(read_width, read_height,dummy_addr);
	hwCdsp_SetYuvBuffB(read_width, read_height,dummy_addr);		
#if 0 
	hwCdsp_SetDmaBuff(AUTO_SWITCH);
#else
	hwCdsp_SetDmaBuff(RD_A_WR_A);
#endif
	
	//cdsp irq
#if CDSP_DEBUG == 0
	vic_irq_register(VIC_CDSP, cdsp_isr);
	vic_irq_enable(VIC_CDSP);

	cdsp_eof_isr_register(gp_cdsp_handle_eof);
	cdsp_ae_isr_register(gp_cdsp_handle_ae);
	cdsp_awb_isr_register(gp_cdsp_handle_awb);

	hwCdsp_SetInt(1, CDSP_EOF);
#endif	

#if (USE_SENSOR_NAME == SENSOR_SOI_H22)
	#if SENSOR_FLIP
	hwCdsp_SetRawDataFormat(1);   
	#else
	hwCdsp_SetRawDataFormat(2);   //BGGR
	#endif
	hwFront_SetFrameSize(0xD8, 0x02, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);	//pll 38.4,BGGR
	
	sensor_SOi_h22_init(gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
#elif (USE_SENSOR_NAME == SENSOR_GC1004)
	hwCdsp_SetRawDataFormat(2);			//2:BGGR, 1:RGGB
	#if SENSOR_FLIP
	//hwFront_SetFrameSize(0x08, 0x01, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);	//BGGR
	hwFront_SetFrameSize(0x07, 0x02, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);	//BGGR    Liuxi modified in 2015-01-21
	#else
	//hwFront_SetFrameSize(0x07, 0x02, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);	//BGGR
	//hwFront_SetFrameSize(0x08, 0x03, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);	//BGGR
	hwFront_SetFrameSize(0x08, 0x01, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);	//BGGR     Liuxi modified in 2015-01-21
	#endif	
	sensor_gc1004_init(gpSENSOR_WIDTH, gpSENSOR_HEIGHT);	
#elif(USE_SENSOR_NAME == SENSOR_OV9712)
	hwCdsp_SetRawDataFormat(2); 		//BGGR
	hwFront_SetFrameSize(0x190, 0x02, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);	//RGGB
	sensor_ov9712_init(OV9712_RAW, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);	
#else
	DBG_PRINT("\r\n= Non Sensor Init =\r\n");
#endif

	//cdsp output image format
	hwCdsp_SetOutYUVFmt(YVYU);
#if CDSP_DEBUG == 0
	/*AE, AWB Init & Enable*/
	cdsp_ae_awb_init();
	gp_cdsp_set_aeawb_window(gpSENSOR_WIDTH, gpSENSOR_HEIGHT,TARGET_Y);

	gp_cdsp_set_histgm(ENABLE, 200, 25);	
	gp_cdsp_set_saturation();
	hwCdsp_SetUVScale(0, uv_div[0][0], uv_div[0][1], uv_div[0][2], uv_div[0][3], uv_div[0][4], uv_div[0][5]);
	
	gp_cdsp_sat_contrast_init();
	
{
	INT32U temp;
	/*-- Edage --*/
	temp = 	ap_state_config_sharpness_get();
	ap_edge_set_by_config(temp);	//// 0: soft, 3:standard, 2: sharp, 1: more sharp
	temp = ap_state_config_white_balance_get();
	gp_cdsp_set_awb_by_config(temp);
	temp = ap_state_config_ev_get();
	gp_cdsp_set_ev_val(temp);	//0:+2, 1:+5/3, 2:+4/3, 3:+1.0, 4:+2/3, 5:+1/3, 6:+0.0, 7:-1/3, 8:-2/3, 9:-1.0, 10:-4/3, 11:-5/3, 12:-2.0 
	temp = ap_state_config_iso_get();
	ap_iso_set_by_config(temp);
	/*-- AC Frequence--*/
	temp = ap_state_config_light_freq_get(); //0: 50Hz,  1: 60Hz
	gp_cdsp_set_exp_freq(temp);
}
#if 1
	hwFront_SetSize(1280, 720+2);
	hwIsp_SetImageSize(1280, 720);
	hwCdsp_EnableNewDenoise(ENABLE);
	hwIsp_EnableDenoise(DISABLE);
	//gp_cdsp_set_ev_val(4);
	//hwCdsp_SetYuvHAvg(3,1,2,2);
#endif
#endif
	//hwCdsp_DataSource(FRONT_INPUT);
	CdspDev->getSensorInfo |= 0x320;
	DBG_PRINT("\r\n%s\r\n", gp_cdsp_aeawb_get_version());
	DBG_PRINT("\r\n CDSP START ! \r\n");
	
#if CDSP_DEBUG == 1// Initiate TFT controller for test	
	tft_init();					 
	R_TFT_FBI_ADDR =0x80500000;//(INT32U) dummy_addr;
	R_PPU_ENABLE = 0x00200580; //for CDSP ouput YVYU
	//R_PPU_ENABLE = 0x00300580; //for CDSP output UYVY
		 
	R_TFT_CTRL |= 0x1; 	 // TFT ON

	//hwCdsp_SetYuvBuffA(read_width, read_height,dummy_addr);
	
	//save_raw10(0x80400000, LENSCMP);
	while(1);
#endif	
}

void gp_mipi_isp_start(INT32U dummy_addr, INT32U gpSENSOR_WIDTH, INT32U gpSENSOR_HEIGHT)	
{
	INT32U read_width, read_height;
	INT32U clamp_width, clamp_en;
	//INT8U clip_tmp;

	CdspDev = &gp_CdspDev;
	gp_memset((INT8S*) CdspDev, 0, sizeof(gpCdspDev_t));
	
	//hwCdsp_Enable3ATestWin(1, 1);
	
	
	ae_exp_set_cnt = 0;
	CdspDev->awb_ret_flag = -1;
 	CdspDev->ae_ret = -1;
	CdspDev->ae_workmem = ae_workszie;
	CdspDev->awb_workmem = awb_worksize;
	if (awb == NULL) {
		awb = CdspDev->awb_workmem;
	}

	if (ae == NULL) {
		ae = CdspDev->ae_workmem;
	}
	
		
	CdspDev->aeAddr[0] = cdsp_ae_addr_a;
	CdspDev->aeAddr[1] = cdsp_ae_addr_b;
	DBG_PRINT("\r\nae_work_buffer = 0x%x, awb_work_buffer = 0x%x\r\n",CdspDev->ae_workmem, CdspDev->awb_workmem);
	DBG_PRINT("\r\nae_addr[0] = 0x%x, ae_addr[1] = 0x%x\r\n",CdspDev->aeAddr[0], CdspDev->aeAddr[1]);

	hwCdsp_Reset();
	hwFront_Reset();
	
	
	hwIsp_Hr0(0);
	hwIsp_Hr1(0);
	hwIsp_Hr2(0);
	hwIsp_Hr3(0);
#if 1
	hwCdsp_SetAEBuffAddr(0xF8500000, 0xF8500000);
	R_CDSP_DMA_YUVABUF_SADDR = 0xF8500000;
	R_CDSP_DMA_YUVBBUF_SADDR = 0xF8500000;
	R_CDSP_MD_DMA_SADDR = 0xF8500000;
#endif
	
	//init sensor
	hwCdsp_SetYuvRange(0x0); 				//unsigned

#if 0
	hwCdsp_DataSource(FRONT_INPUT);
	hwFront_SetInputFormat(SENSOR_RAW10);	//set cdsp input format
	SensorIF_Setting(C_RAW_FMT);			//set sensor input foramt clk
#else
	hwCdsp_DataSource(MIPI_INPUT);
	hwFront_SetInputFormat(MIPI_RAW10);
	//hwFront_SetMipiFrameSize(0, 0, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
		//init sensor
	MipiIF_Setting(C_RAW_FMT, gpSENSOR_WIDTH, gpSENSOR_HEIGHT); //raw 	//for h&v scaler
#endif


#if CDSP_DEBUG == 0
	/* --- Init SRAM Data ---*/
	#if (USE_SENSOR_NAME == SENSOR_SOI_H22)
	SOi_h22_sensor_calibration();
	#elif (USE_SENSOR_NAME == SENSOR_GC1004)
	gc1004_sensor_calibration();
	#else
	DBG_PRINT("Non calibration sensor\r\n");
	#endif
#endif	
	hwCdsp_EnableAF(0, 0);
	hwCdsp_EnableAE(0, 0);
	hwCdsp_EnableAWB(1, 0);
	
	read_width = gpSENSOR_WIDTH - 8;
	read_height = gpSENSOR_HEIGHT - 4;
	clamp_width = gpSENSOR_WIDTH - 8;
	clamp_en = 0;	

	/*--- Bad Pixel ---*/
	//hwCdsp_SetBadPixel(160,160,160);
	hwCdsp_EnableBadPixel(ENABLE, 1, 1);

	/*--- Lens compesation ---*/
	hwCdsp_EnableLensCmp(DISABLE, 0x0);
	//disable when use YUV data input
	hwCdsp_SetLensCmpPath(0);
#if CDSP_DEBUG == 1
	/*--- wb gain config ---*/
	gp_cdsp_set_wb_gain(DISABLE, DISABLE);

	/*-- Gamma --*/
	hwCdsp_EnableLutGamma(DISABLE);

	/*--Color matrix --*/	
	hwCdsp_EnableColorMatrix(DISABLE);	
#else
	/*--- ISP LSC ---*/
	hwIsp_LenCmp(ENABLE, 19); //lx add
	/*--- wb gain config ---*/
	gp_cdsp_set_wb_gain(ENABLE, ENABLE);

	/*-- Gamma --*/
	hwCdsp_EnableLutGamma(ENABLE);

	/*--Color matrix --*/	
	hwCdsp_EnableColorMatrix(ENABLE);	
#endif

	/*--Edge--*/
	/*new denoise config*/
	gp_cdsp_set_new_denoise(2);
	/*set Hybrid Denoise*/
	IspHybridRaw.IspHeight = gpSENSOR_HEIGHT;
	IspHybridRaw.ISPWidth = gpSENSOR_WIDTH;
	IspHybridRaw.DenoiseEnable =ENABLE;//DISABLE; //lx change
	IspHybridRaw.DPCDefaultMode = 1;
	IspHybridRaw.DPCn = 1;			
	IspHybridRaw.DefectPixelSel = 0;
	IspHybridRaw.DefectPixelEnable = DISABLE;
	IspHybridRaw.CrosstalkEnable = DISABLE; 

	IspHybridRaw.DPCth1 = 55; //55 -> 66, weak -> strong
   	IspHybridRaw.DPCth2 = 76; //76 -> 55 -> 43, weak -> medium -> strong
   	IspHybridRaw.DPCth3 = 4; //if 3 line be filter
/*
#if (CDSP_DENOISE_LEVEL == BOTH_DENOISE)		
	IspHybridRaw.DenoiseT1 = 0; 
	IspHybridRaw.DenoiseT2 = 0; 
	IspHybridRaw.DenoiseT3 = 0; 
	IspHybridRaw.DenoiseT4 = 64;//56; 
	IspHybridRaw.DenoiseW1 = 0; 
	IspHybridRaw.DenoiseW2 = 0; 
	IspHybridRaw.DenoiseW3 = 4;//2; 
#endif*/

//#if (CDSP_DENOISE_LEVEL == HYBRID_DENOISE_ONLY)
	IspHybridRaw.DenoiseT1 = 0; 
	IspHybridRaw.DenoiseT2 = 8; 
	IspHybridRaw.DenoiseT3 = 16; 
	IspHybridRaw.DenoiseT4 = 32; 
	IspHybridRaw.DenoiseW1 = 1; 
	IspHybridRaw.DenoiseW2 = 2; 
	IspHybridRaw.DenoiseW3 = 3; 
//#endif

	IspHybridRaw.CrosstalkT1 = 0; 
	IspHybridRaw.CrosstalkT2 = 0; 
	IspHybridRaw.CrosstalkT3 = 0; 
	IspHybridRaw.CrosstalkT4 = 16; 
	IspHybridRaw.CrosstalkW1 = 0;
	IspHybridRaw.CrosstalkW2 = 0; 
	IspHybridRaw.CrosstalkW3 = 0; 
	IspHybridRaw.CrosstalkGbGr = 2;
	
	gp_isp_set_hybridraw(&IspHybridRaw);
#if 1
	//Enable raw interpolution
	hwCdsp_SetExtLine(ENABLE, gpSENSOR_WIDTH, 0x28);
	//hwCdsp_IntplThrSet(0x10, 0xF0);
	//hwCdsp_IntplThrSet(16, 240); // same as GP12B
	hwCdsp_IntplThrSet(24, 240); //lx change
	hwCdsp_EnableIntplMir(0x0F, 1, 0);	//intplmirvsel,	//suggest set 1
	read_width += 4;
	read_height += 4;
	clamp_width += 4;
#endif
#if 1
	//For YUV Horizontal Average method and LP Filiter temp
	hwCdsp_SetYuvHAvg(0x03, 0, 0, 0);	
	read_width += 4;
	clamp_width += 4;	
#endif
#if CDSP_DEBUG == 1
	//For h and v scale down, must be 2 align. 
	//dummy_addr = 0x600000;
	{
		INT32U hfactor, vfactor;
		hfactor = (320 << 16)/gpSENSOR_WIDTH + 1;
		vfactor = (240 << 16)/gpSENSOR_HEIGHT + 1;
		clamp_en = 1;
		clamp_width = 320;	//(320 - 4) is for not enable other function
		hwCdsp_SetYuvHScale(hfactor, hfactor);
		hwCdsp_SetYuvVScale(vfactor, vfactor);
		hwCdsp_EnableYuvHScale(ENABLE, 1);
		hwCdsp_EnableYuvVScale(ENABLE, 1);
		read_width = 320;
		read_height = 240;
	}
#endif

	hwCdsp_SetSRAM(ENABLE, 0xA0);
	hwCdsp_EnableClamp(clamp_en, clamp_width);
	
	hwCdsp_SetYuvBuffA(read_width, read_height,dummy_addr);
	hwCdsp_SetYuvBuffB(read_width, read_height,dummy_addr);		
#if 0 
	hwCdsp_SetDmaBuff(AUTO_SWITCH);
#else
	hwCdsp_SetDmaBuff(RD_A_WR_A);
#endif
	
	//cdsp irq
#if CDSP_DEBUG == 0
	vic_irq_register(VIC_CDSP, cdsp_isr);
	vic_irq_enable(VIC_CDSP);

	cdsp_eof_isr_register(gp_cdsp_handle_eof);
	cdsp_ae_isr_register(gp_cdsp_handle_ae);
	cdsp_awb_isr_register(gp_cdsp_handle_awb);

	hwCdsp_SetInt(1, CDSP_EOF);
#endif	

#if (USE_SENSOR_NAME == SENSOR_SOI_H22)
	hwCdsp_SetRawDataFormat(2); 	//RAW:0x1:RGrGbB, 0x2:BGGR;
	hwFront_SetMipiFrameSize(4, 0, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
	//hwFront_SetFrameSize(0x8, 0x0E, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
	sensor_SOi_h22_init(gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
#elif (USE_SENSOR_NAME == SENSOR_OV3640)
	hwCdsp_SetRawDataFormat(1);		//RAW:0x1:RGrGbB;
	hwFront_SetMipiFrameSize(0, 0, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
	Sensor_Start(DRV_OV3640, OV3640_BGGR);
#elif (USE_SENSOR_NAME == SENSOR_OV5642)
	hwCdsp_SetRawDataFormat(1);		//RAW:0x1:RGrGbB;
	hwFront_SetMipiFrameSize(0, 0, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
	Sensor_Start(DRV_OV5642, OV5642_MIPI_GRBG);
#elif (USE_SENSOR_NAME == SENSOR_GC1004)
	hwCdsp_SetRawDataFormat(2);			//BGGR
	#if SENSOR_FLIP
	hwFront_SetMipiFrameSize(0xB, 0, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
	#else
	hwFront_SetMipiFrameSize(0xC, 0x2, gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
	#endif
	sensor_gc1004_init(gpSENSOR_WIDTH, gpSENSOR_HEIGHT);
#else	
	DBG_PRINT("NO Sensor Init !\r\n");
#endif
	//cdsp output image format
	hwCdsp_SetOutYUVFmt(YVYU);
#if CDSP_DEBUG == 0
	/*AE, AWB Init & Enable*/
	cdsp_ae_awb_init();
	gp_cdsp_set_aeawb_window(gpSENSOR_WIDTH, gpSENSOR_HEIGHT,TARGET_Y);

	gp_cdsp_set_histgm(ENABLE, 200, 25);	
	gp_cdsp_set_saturation();
	hwCdsp_SetUVScale(0, uv_div[0][0], uv_div[0][1], uv_div[0][2], uv_div[0][3], uv_div[0][4], uv_div[0][5]);
	
	gp_cdsp_sat_contrast_init();
	
{
	INT32U temp;
	/*-- Edage --*/
	temp = 	ap_state_config_sharpness_get();
	ap_edge_set_by_config(temp);	//// 0: soft, 3:standard, 2: sharp, 1: more sharp
	temp = ap_state_config_white_balance_get();
	gp_cdsp_set_awb_by_config(temp);
	temp = ap_state_config_ev_get();
	gp_cdsp_set_ev_val(temp);	//0:+2, 1:+5/3, 2:+4/3, 3:+1.0, 4:+2/3, 5:+1/3, 6:+0.0, 7:-1/3, 8:-2/3, 9:-1.0, 10:-4/3, 11:-5/3, 12:-2.0 
	temp = ap_state_config_iso_get();
	ap_iso_set_by_config(temp);
	/*-- AC Frequence--*/
	temp = ap_state_config_light_freq_get(); //0: 50Hz,  1: 60Hz
	gp_cdsp_set_exp_freq(temp);
}
#endif
	//hwCdsp_DataSource(FRONT_INPUT);

	DBG_PRINT("\r\n%s\r\n", gp_cdsp_aeawb_get_version());
	DBG_PRINT("\r\n CDSP mipi START ! \r\n");
	CdspDev->getSensorInfo |= 0x320;
		
#if CDSP_DEBUG == 1// Initiate TFT controller for test	
	tft_init();					 
	R_TFT_FBI_ADDR =0x80500000;//(INT32U) dummy_addr;
	R_PPU_ENABLE = 0x00200580; //for CDSP ouput YVYU
	//R_PPU_ENABLE = 0x00300580; //for CDSP output UYVY
		 
	R_TFT_CTRL |= 0x1; 	 // TFT ON

	//save_raw10(0x80400000, LENSCMP);
	while(1);
#endif	
}

void gp_cdsp_set_exposure_time(void)
{
	#if (USE_SENSOR_NAME == SENSOR_SOI_H22)	
		H22_set_exposure_time(&seInfo2);
	#elif (USE_SENSOR_NAME == SENSOR_GC1004)
		gc1004_set_exposure_time(&seInfo2);
	#endif
	//DBG_PRINT("2      S_Digital Gain: %x, %x, %x\r\n", seInfo2.digital_gain, seInfo2.max_digital_gain, seInfo2.min_digital_gain);
	//DBG_PRINT("2      S_Analog Gain: %x, %x, %x\r\n", seInfo2.analog_gain, seInfo2.max_analog_gain, seInfo2.min_analog_gain);
	//DBG_PRINT("2	  S_Time: %x, %x, %x\r\n", seInfo2.time, seInfo2.max_time, seInfo2.min_time);

}

INT32S ae_gain_get(TIME_T  *tm)
{
	#if (USE_SENSOR_NAME == SENSOR_SOI_H22)	
		p_seInfo = jxh22_get_senInfo();
	#elif (USE_SENSOR_NAME == SENSOR_GC1004)	
		p_seInfo = gc1004_get_senInfo();
	#elif (USE_SENSOR_NAME == SENSOR_OV9712)	
		p_seInfo = ov9712_get_senInfo();
	#endif

	tm->tm_year = p_seInfo->time;

	tm->tm_mon = p_seInfo->analog_gain/100;
	tm->tm_mday = p_seInfo->analog_gain%100;

	tm->tm_hour = seInfo2.set_color_tmp; //u2 added

	tm->tm_min = p_seInfo->digital_gain/100;
	tm->tm_sec = p_seInfo->digital_gain%100;

	//DBG_PRINT("\r\n2.set_color_tmp = %d\n",seInfo2.set_color_tmp);

	return 0;
}

void gp_cdsp_target_lum_set(INT16S target_y){ 
        // set Target Lum                 
        gp_cdsp_ae_set_target_lum(CdspDev->ae_workmem, target_y); 
} 
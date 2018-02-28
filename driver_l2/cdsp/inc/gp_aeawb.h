#ifndef __AEAWB_H__
#define __AEAWB_H__


#if 1//BODY_GP15B
// for GP15B

#ifndef sensor_exposure_t
typedef struct {
	// real setting for sensor
	int time, analog_gain, digital_gain;

	// set limitation
	int max_time, min_time;
	int max_analog_gain, min_analog_gain;
	int max_digital_gain, min_digital_gain;


	int daylight_ev_idx;	// daylight time
	int night_ev_idx;		// night time
	int max_ev_idx;

	// ev index
	int sensor_ev_idx;
	int ae_ev_idx;
	
	int set_color_tmp;

	int userISO;
} sensor_exposure_t;
#endif


#ifndef ISO_AUTO
#define ISO_AUTO						0x1881
#endif

#ifndef int64_t
typedef long long int64_t;
#endif



#else

// for GP12B
//#include "gp_cdsp.h"
#include <mach/sensor_mgr.h>

#endif



// ***********************************************************************************************
// Constant value definition
// ***********************************************************************************************
#define AEGAIN_P05EV				1060    // + 0.05 EV
#define AEGAIN_P01EV				1031    // +0.01 EV
#define AEGAIN_0EV					1024    // 0 EV 
#define AEGAIN_N01EV				1017    // -0.01 EV
#define AEGAIN_N05EV				989     // -0.05 EV


#define AE_EV0_LUM					0x30    // default target luminance
#define AE_EV0_LUM_TOR				0x06    // default tolerance for target luminance


#ifndef ENABLE
#define ENABLE						1
#endif

#ifndef DISABLE
#define DISABLE						0
#endif




// ***********************************************************************************************
// Type definition
// ***********************************************************************************************
typedef enum {
	AE_STABLE = -1,  // AE satble
	AE_CHANGE = 0 	// AE should be modified and to set sensor exposure time
} AE_RESULT;



typedef enum {
	AWB_FAIL = -1,		// AWB fail
	AWB_RET = 0,		// AWB processing but has no result for setting wb gain
	AWB_SUCCESS_CVR,	// AWB success in CVR mode
	AWB_SUCCESS_DC,	// AWB success in DC mode
	AWB_USERDEF		// AWB success in user define mode
} AWB_RESULT;


typedef enum {
	AWB_AUTO_DC = 0x00001000,	// AUTO WB for DC
		
	AWB_AUTO_CVR,				// AUTO WB for CVR
	AWB_AUTO_CVR_DAYLIGHT,	// AUTO WB for CVR in daylight
	AWB_AUTO_CVR_NIGHT,		// AUTO WB for CVR in night
	AWB_AUTO_CVR_CLOUDY,	// AUTO WB for CVR between day and night or cloudy/raining day

	AWB_LAMP, 					// Color temperature = 3200k
	AWB_FLUORESCENT, 			// Color temperature = 4000k
	AWB_DAYLIGHT,				// Color temperature = 5200k	
	AWB_FLASH, 				// Color temperature = 6000k	
	AWB_CLOULDY, 				// Color temperature =  6000k
	AWB_SHADE 				// Color temperature =  7000k
} AWB_MODE;



typedef enum {
	CDSP_AE_METER_SPOT = 0,						// AE METER: Spot
	CDSP_AE_METER_CENTER_WEIGHTED_AVG,		// AE METER: Center weighted average
	CDSP_AE_METER_CENTER_WEIGHTED_AVG_CVR,	// AE METER: Center weighted average for CVR
	CDSP_AE_METER_AVG,							// AE METER: Averate
	CDSP_AE_METER_USER_WEIGHT					// AE METER: 64 window weight is defined by user
} CDSP_AE_METER_MODE;


typedef struct {
	unsigned int 		sumcnt[3];	// count for each region (different luminance)
	unsigned long long sumg[3];	// sum of green vale in different region
	int64_t			sumrg[3];	// sum of red vale in different region
	int64_t			sumbg[3];	// sum of blue vale in different region
} AWB_RESULT_t;



// ***********************************************************************************************
//                                                       Function definition
// ***********************************************************************************************


//=========================================================================
// Function Name :  gp_cdsp_aeawb_get_version
// Purpose :  Get library version
// Parameters : void
// Return : version string
//=========================================================================
const char *gp_cdsp_aeawb_get_version(void);




//=========================================================================
// Function Name :  gp_cdsp_aeawb_init
// Purpose :  for AE & AWB initialization
// Parameters : unsigned char *_ae: working memory for AE
//                    unsigned char *_awb: working memory for AWB
//                    AWB_MODE mode: AWB mode 
// Return : void
//=========================================================================
void gp_cdsp_aeawb_init(unsigned char *_ae, unsigned char *_awb, AWB_MODE mode);



//=========================================================================
// Function Name :  gp_cdsp_aeawb_reset
// Purpose :  for AE & AWB reset some memory
// Parameters : unsigned char *_ae: working memory for AE
//                    unsigned char *_awb: working memory for AWB
// Return : void
//=========================================================================
void gp_cdsp_aeawb_reset(unsigned char *_ae, unsigned char *_awb);



//=========================================================================
// Function Name :  gp_cdsp_ae_get_workmem_size
// Purpose :  Get AE working memory size
// Parameters : void
// Return: size of working memory for AE
//=========================================================================
unsigned int gp_cdsp_ae_get_workmem_size(void);


//=========================================================================
// Function Name :  gp_cdsp_awb_get_workmem_size
// Purpose :  Get AWB working memory size
// Parameters : void
// Return: size of working memory for AWB
//=========================================================================
unsigned int gp_cdsp_awb_get_workmem_size(void);





// ***********************************************************************************************
// ****************************************   AE   *************************************************
// ***********************************************************************************************


//=========================================================================
// Function Name: gp_cdsp_ae_set_fps
// Purpose: to make AE algorithm to know the frame rate
// Parameters: unsigned char *_ae: working memory for AE
//                  int fps: sensor frame rate, ex: 30 or 25 or 20 ...
// Return: none
//=========================================================================
void gp_cdsp_ae_set_fps(unsigned char *_ae, int fps);



//=========================================================================
// Function Name: gp_cdsp_ae_calc_exp
// Purpose: to calculate the sensor exposure time
// Parameters: unsigned char *_ae: working memory for AE
//                  unsigned char *ae_win: pointer to AE window (8x8 = 64 values)
//                  sensor_exposure_t *seinfo: information of sensor exposure time/gain
//                  int hist_hi: count of histogram in high luminanace threshold
//                  int hist_low: count of histogram in low luminanace threshold
// Return: AE_STABLE: AE satble
//     	  AE_CHANGE: AE should be modified and to set sensor exposure time
//=========================================================================
int gp_cdsp_ae_calc_exp(unsigned char *_ae, unsigned char *ae_win, sensor_exposure_t *seinfo, int hist_hi, int hist_low);



//=========================================================================
// Function Name: gp_cdsp_ae_set_meter
// Purpose : set AE meter
// Parameters : CDSP_AE_METER_MODE meter: type of AE meter
//			   unsigned char *_ae: working memory for AE
// Return: void
//=========================================================================
void gp_cdsp_ae_set_meter(CDSP_AE_METER_MODE meter, unsigned char *_ae);



//=========================================================================
// Function Name : gp_cdsp_ae_set_target_lum
// Purpose : gp_cdsp_ae_set_target_lum
// Parameters : unsigned char *_ae: working memory for AE
//                    int targetLum: target luminance
// Return: void
//=========================================================================
void gp_cdsp_ae_set_target_lum(unsigned char *_ae, int targetLum);


//=========================================================================
// Function Name : gp_cdsp_ae_set_ev
// Purpose : se EV (form -2 ~ 0 ~ +2)
// Parameters : unsigned char *_ae: working memory for AE
//                    int ev_idx: index of EV table => 0:+2, 1:+5/3, 2:+4/3, 3:+1.0, 4:+2/3, 5:+1/3, 6:+0.0, 7:-1/3, 8:-2/3, 9:-1.0, 10:-4/3, 11:-5/3, 12:-2.0 
// Return: void
//=========================================================================
void gp_cdsp_ae_set_ev(unsigned char *_ae, int ev_idx);



//=========================================================================
// Function Name : gp_cdsp_ae_set_iso
// Purpose : set ISO
// Parameters: unsigned char *_ae: working memory for AE
//                   int iso: ISO value (1: 100, 2: 200, 4: 400, ISO_AUTO)
//                   sensor_exposure_t *seinfo: information of sensor exposure time/gain
// Return: void
//=========================================================================
void gp_cdsp_ae_set_iso(unsigned char *_ae, int iso, sensor_exposure_t *seinfo);


//=========================================================================
// Function Name: gp_cdsp_ae_set_lum_bound
// Purpose : set the luminance bound by different size of image & ae window
// Parameters : unsigned char *_ae: working memory for AE
//                    int img_w: width of image
//                    int img_h: height of image
//                    int win_w: width of ae window
//                    int win_h: height of ae window
// Return: void
//=========================================================================
void gp_cdsp_ae_set_lum_bound(unsigned char *_ae, int img_w, int img_h, int win_w, int win_h);


//=========================================================================
// Function Name: gp_cdsp_ae_set_sensor_exp_time
// Purpose : AE algorithm must save the information of sensor exposure time/gain
// Parameters: unsigned char *_ae: working memory for AE
//                   sensor_exposure_t *seinfo: information of sensor exposure time/gain
// Return: void
//=========================================================================
void gp_cdsp_ae_set_sensor_exp_time(unsigned char *_ae, sensor_exposure_t *seinfo);




//=========================================================================
// Function Name: gp_cdsp_ae_is_night
// Purpose : get the night status
// Parameters: unsigned char *_ae: working memory for AE
// Return: 1: night,  0: not night
//=========================================================================
int gp_cdsp_ae_is_night(void *_ae);



//=========================================================================
// Function Name: gp_cdsp_ae_get_result_lum
// Purpose : get the current luminance
// Parameters: unsigned char *_ae: working memory for AE
// Return: current luminance
//=========================================================================
int gp_cdsp_ae_get_result_lum(unsigned char *_ae);




//=========================================================================
// Function Name: gp_cdsp_ae_get_result_ev
// Purpose : get the EV index, Ex: 0ev = 0,  +0.2ev = +2,  -0.3ev = -3, ...
// Parameters: unsigned char *_ae: working memory for AE
// Return: EV index, step = 0.1ev
//=========================================================================
int gp_cdsp_ae_get_result_ev(unsigned char *_ae);




//=========================================================================
// Function Name: gp_cdsp_ae_set_user_center_weight
// Purpose : set ae weight for each window (8x8 = 64), be careful: Sum(weight) = 1
// Parameters: float *weight: total 64 weighting values, Ex: 0.1, 0.2 ...
//                  unsigned char *_ae: working memory for AE
// Return: void
//=========================================================================
void gp_cdsp_ae_set_user_weight(float *weight, unsigned char *_ae);




//=========================================================================
// Function Name: gp_cdse_ae_set_backlight_detect
// Purpose : Enable/Disable backlight detect
// Parameters:  unsigned char *_ae: working memory for AE
//                    int enable: enable = 1, disable = 0
// Return: void
//=========================================================================
void gp_cdse_ae_set_backlight_detect(unsigned char *_ae, int enable);



//=========================================================================
// Function Name: gp_cdsp_ae_comb_win
// Purpose : combine two ae window for dynamic set window 
// Parameters:  unsigned char *_ae: working memory for AE
//              unsigned char *ae_win_org: destination of AE window
//              unsigned char *ae_win_src: source of AE window
//              int flag: 0: 1st,   2: 2nd
// Return: void
//=========================================================================
void gp_cdsp_ae_comb_win(unsigned char *_ae, unsigned short *ae_win_dst, unsigned char *ae_win_src, int flag);



// ***********************************************************************************************
// ****************************************   AWB   ************************************************
// ***********************************************************************************************




//=========================================================================
// Function Name : gp_cdsp_awb_set_mode
// Purpose : set WB mode
// Parameters : unsigned char *_awb: working memory for AWB
//                    AWB_MODE mode: white balance mode
// Return: void
//=========================================================================
void gp_cdsp_awb_set_mode(unsigned char *_awb, AWB_MODE mode);




//=========================================================================
// Function Name : gp_cdsp_awb_set_cnt_thr
// Purpose : set 
// Parameters : unsigned char *_awb: working memory for AWB
//                    int img_w: width of image
//                    int img_h: height of image
//                    int subsample: subsample for wb window
//                    int cnt_thr: user set thr, otherwise cnt_thr = 0;
// Return: void
//=========================================================================
void gp_cdsp_awb_set_cnt_thr(unsigned char *_awb, int img_w, int img_h, int subsample, int cnt_thr);



//=========================================================================
// Function Name : gp_cdsp_awb_get_mode
// Purpose : get wb mode
// Parameters : unsigned char *_awb: working memory for AWB
// Return: white balance mode
//=========================================================================
int gp_cdsp_awb_get_mode(unsigned char *_awb);


//=========================================================================
// Function Name : gp_cdsp_awb_get_color_temperature
// Purpose : get color temperature
// Parameters : unsigned char *_awb: working memory for AWB
// Return: color temperature
//=========================================================================
int gp_cdsp_awb_get_color_temperature(unsigned char *_awb);


//=========================================================================
// Function Name : gp_cdsp_awb_get_calc_color_temperature
// Purpose : get calcucated color temperature
// Parameters : unsigned char *_awb: working memory for AWB
// Return: color temperature
//=========================================================================
int gp_cdsp_awb_get_calc_color_temperature(unsigned char *_awb);


//=========================================================================
// Function Name : gp_cdsp_awb_get_r_gain
// Purpose : get Red gain
// Parameters : unsigned char *_awb: working memory for AWB
// Return: red gain for wb gain
//=========================================================================
int gp_cdsp_awb_get_r_gain(unsigned char *_awb);

//=========================================================================
// Function Name : gp_cdsp_awb_get_b_gain
// Purpose :  get blue gain
// Parameters : unsigned char *_awb: working memory for AWB
// Return: blue gain for wb gain
//=========================================================================
int gp_cdsp_awb_get_b_gain(unsigned char *_awb);


//=========================================================================
// Function Name : gp_cdsp_awb_get_r_gain2
// Purpose : get Red gain
// Parameters : unsigned char *_awb: working memory for AWB
// Return: red gain for wb gain2
//=========================================================================
int gp_cdsp_awb_get_r_gain2(unsigned char *_awb);



//=========================================================================
// Function Name : gp_cdsp_awb_get_b_gain2
// Purpose :  get blue gain
// Parameters : unsigned char *_awb: working memory for AWB
// Return: blue gain for wb gain2
//=========================================================================
int gp_cdsp_awb_get_b_gain2(unsigned char *_awb);



//=========================================================================
// Function Name:  gp_cdsp_awb_calc_gain
// Purpose :  calculate the white balance gain
// Parameters : unsigned char *_awb: working memory for AWB
//                    AWB_RESULT_t *awb_result: result of wb window
//                    const unsigned short (*awb_gain_table)[2]: pointer to red/blue gain of sensor (from sensor calibration)
// Return: AWB_FAIL: AWB fail
//	         AWB_RET: AWB processing but has no result for setting wb gain
//	         AWB_SUCCESS_CVR: AWB success in CVR mode
//	         AWB_SUCCESS_DC: AWB success in DC mode
//	         AWB_USERDEF: AWB success in user define mode
//=========================================================================
int gp_cdsp_awb_calc_gain(unsigned char *_awb, AWB_RESULT_t *awb_result, const unsigned short (*awb_gain_table)[2]);




//=========================================================================
// Function Name:  gp_cdsp_awb_reset_wb_gain
// Purpose :  reset the white balance gain
// Parameters : unsigned char *_awb: working memory for AWB
//                    int color_temperature: Ex: 5000k --> color_temperature = 50, 6500k --> color_temperature = 65, ...
//                    const unsigned short (*awb_gain_table)[2]: pointer to red/blue gain of sensor (from sensor calibration)
// Return: void
//=========================================================================
void gp_cdsp_awb_reset_wb_gain(unsigned char *_awb, int color_temperature, const unsigned short (*awb_gain_table)[2]);


//=========================================================================
// Function Name:  gp_cdsp_awb_set_r_gain_factor
// Purpose :  adjust the r gain by factor
// Parameters : unsigned char *_awb: working memory for AWB
//                    float factor: the scale factor of r gain
// Return: void
//=========================================================================
void gp_cdsp_awb_set_r_gain_factor(unsigned char *_awb, float factor);



//=========================================================================
// Function Name:  gp_cdsp_awb_set_b_gain_factor
// Purpose :  adjust the r gain by factor
// Parameters : unsigned char *_awb: working memory for AWB
//                    float factor: the scale factor of b gain
// Return: void
//=========================================================================
void gp_cdsp_awb_set_b_gain_factor(unsigned char *_awb, float factor);



//=========================================================================
// Function Name:  gp_cdsp_awb_set_ct_offset
// Purpose :  set the white balance offset
// Parameters : unsigned char *_awb: working memory for AWB
//                    int offset: the offset of the color temperature
// Return: void
//=========================================================================
void gp_cdsp_awb_set_ct_offset(unsigned char *_awb, int offset);



//=========================================================================
// Function Name:  gp_cdsp_awb_set_calc_ct_offset
// Purpose :  set the white balance offset when calucated
// Parameters : unsigned char *_awb: working memory for AWB
//                    int offset: the offset of the color temperature
// Return: void
//=========================================================================
void gp_cdsp_awb_set_calc_ct_offset(unsigned char *_awb, int offset);


//=========================================================================
// Function Name:  gp_cdsp_awb_set_r_b_gain_boundary
// Purpose :  limit the r/b gain for AWB2
// Parameters : unsigned char *_awb: working memory for AWB
//                    int max_rgain: max r gain
//                    int max_bgain: max b gain
//                    int min_rgain: min r gain
//                    int min_bgain: min b gain
// Return: void
//=========================================================================
void gp_cdsp_awb_set_r_b_gain_boundary(unsigned char *_awb, int max_rgain, int max_bgain, int min_rgain, int min_bgain);

#endif



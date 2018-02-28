#include "drv_l1_i2c.h"
#include "drv_l2_sensor.h"
#include "drv_l1_cdsp.h"
#include "gp_aeawb.h"

#include "LDWs.h"

#include "sensor_soi_h42.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (USE_SENSOR_NAME == SENSOR_SOI_H42)
//================================================================//
static sensor_exposure_t jxh42_seInfo;
static sensor_calibration_t h42_cdsp_calibration;
static int *p_expTime_table;

extern INT8U LDWS_Enable_Flag;
extern void sensor_set_fps(INT32U fpsValue);
extern INT32U sensor_get_fps(void);

static gpCdspWBGain_t soi_h42_wbgain;

#if C_DISPLAY_REVERSE
static INT32U h42_flip_mode = SENSOR_FLIP;
#endif

INT8U	black_sun_flag = 0;
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
#if C_DISPLAY_REVERSE
void h42_set_flip_mode(int mode)
{
	h42_flip_mode = mode;
}
#endif
sensor_exposure_t *jxh42_get_senInfo(void)
{
	return &jxh42_seInfo;
}

void sensor_calibration_str(void)
{
	//sensor_calibration_t h42_cdsp_calibration;

	//H42_cdsp_calibration.lenscmp = g_H42_lenscmp_table;	//GPCV1248 not support
	h42_cdsp_calibration.r_b_gain = g_H42_r_b_gain;
/*
	h42_cdsp_calibration.gamma = g_H42_gamma_045_table;
	h42_cdsp_calibration.color_matrix = g_H42_color_matrix4gamma045;
	h42_cdsp_calibration.awb_thr = g_H42_awb_thr;
*/
}

sensor_calibration_t *jxh42_get_calibration(void)
{
	return &h42_cdsp_calibration;
}
void SOi_h42_sensor_calibration(void)
{
	//OB
	gp_Cdsp_SetBadPixOb((INT16U *)g_h42_badpix_ob_table);
	//Gamma
	hwCdsp_InitGamma((INT32U *)g_H42_gamma_045_table);
	//Color Correction
	hwCdsp_SetColorMatrix_Str((INT16S *)g_H42_color_matrix4gamma045);
	//AWB
	gp_Cdsp_SetAWBYUV((INT16S *)g_H42_awb_thr);
	
	//Lenscmp
	/*
	hwIsp_luc_MaxTan8_Slop_CLP((INT16U *)g_H42_MaxTan8 ,(INT16U *)g_H42_Slope4 ,(INT16U *)g_H42_CLPoint);
	hwIsp_RadusF0((INT16U *)g_H42_Radius_File_0);
	hwIsp_RadusF1((INT16U *)g_H42_Radius_File_1);
	*/
	//R,G,B Linearity correction
	//hwIsp_InitLiCor((INT8U *)LiTable_rgb);		
}

gpCdspWBGain_t *soi_h42_awb_r_b_gain_boundary(void)
{
	
	soi_h42_wbgain.max_rgain = g_H42_r_b_gain[49][0];
	soi_h42_wbgain.max_bgain = g_H42_r_b_gain[11][1];
	soi_h42_wbgain.min_rgain = g_H42_r_b_gain[11][0];
	soi_h42_wbgain.min_bgain = g_H42_r_b_gain[49][1];
	
	return &soi_h42_wbgain;
}

static int H42_analog_gain_table[65] = 
{
	// coarse gain = 0
	(int)(1.00*256+0.5), (int)(1.06*256+0.5), (int)(1.13*256+0.5), (int)(1.19*256+0.5), 
	(int)(1.25*256+0.5), (int)(1.31*256+0.5), (int)(1.38*256+0.5), (int)(1.44*256+0.5), 
	(int)(1.50*256+0.5), (int)(1.56*256+0.5), (int)(1.63*256+0.5), (int)(1.69*256+0.5), 
	(int)(1.75*256+0.5), (int)(1.81*256+0.5), (int)(1.88*256+0.5), (int)(1.94*256+0.5),
	
	// coarse gain = 1
	(int)(2.00*256+0.5), (int)(2.13*256+0.5), (int)(2.25*256+0.5), (int)(2.38*256+0.5), 
	(int)(2.50*256+0.5), (int)(2.63*256+0.5), (int)(2.75*256+0.5), (int)(2.88*256+0.5),
	(int)(3.00*256+0.5), (int)(3.13*256+0.5), (int)(3.25*256+0.5), (int)(3.38*256+0.5),
	(int)(3.50*256+0.5), (int)(3.63*256+0.5), (int)(3.75*256+0.5), (int)(3.88*256+0.5),

	// coarse gain = 2
	(int)(4.00*256+0.5), (int)(4.25*256+0.5), (int)(4.50*256+0.5), (int)(4.75*256+0.5), 
	(int)(5.00*256+0.5), (int)(5.25*256+0.5), (int)(5.50*256+0.5), (int)(5.75*256+0.5), 
	(int)(6.00*256+0.5), (int)(6.25*256+0.5), (int)(6.50*256+0.5), (int)(6.75*256+0.5), 
	(int)(7.00*256+0.5), (int)(7.25*256+0.5), (int)(7.50*256+0.5), (int)(7.75*256+0.5), 
	
	// coarse gain = 3
	(int)(8.00*256+0.5), (int)(8.50*256+0.5), (int)(9.00*256+0.5), (int)(9.50*256+0.5), 
	(int)(10.00*256+0.5), (int)(10.50*256+0.5), (int)(11.00*256+0.5), (int)(11.50*256+0.5), 
	(int)(12.00*256+0.5), (int)(12.50*256+0.5), (int)(13.00*256+0.5), (int)(13.50*256+0.5), 
	(int)(14.00*256+0.5), (int)(14.50*256+0.5), (int)(15.00*256+0.5), (int)(15.50*256+0.5), 
	
	// coarse gain = 4
	(int)(16.00*256+0.5)
};


/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
static int H42_cvt_analog_gain(int analog_gain)
{
	int i;
	int coarse_gain, fine_gain;
	int *p_H42_analog_gain_table = H42_analog_gain_table;

	for( i = 0 ; i < 65 ; i++)
	{
		if(analog_gain >= p_H42_analog_gain_table[i] && analog_gain < p_H42_analog_gain_table[i+1])
			break;
	} 
	
	

	if( i < 16 )
	{
		coarse_gain = 0;
		fine_gain = i;
	}
	else if(i < (16 + 16))
	{
		coarse_gain = 1;
		fine_gain = (i - 16);
	}
	else if(i < (16 + 16 + 16))
	{
		coarse_gain = 2;
		fine_gain = (i - 16 - 16);
	}
	else if(i < (16 + 16 + 16 + 16))
	{
		coarse_gain = 3;
		fine_gain = (i - 16 - 16 -16);
	}
	else
	{
		coarse_gain = 4;
		fine_gain = 0;
	}

	//DBG_PRINT("%d: coarse_gain = 0x%x, fine_gain = 0x%x\n\r", i, coarse_gain, fine_gain);
	return ((coarse_gain << 4) | fine_gain);
}

int H42_get_night_ev_idx(void)
{
	return jxh42_seInfo.night_ev_idx;
}

int H42_get_max_ev_idx(void)
{
	return jxh42_seInfo.max_ev_idx;
}



int H42_set_exposure_time(sensor_exposure_t *si)
{
	//int ret=0;
	//unsigned short tmp;
	//int analog_gain, digital_gain;
	int lsb_time, msb_time;
	int idx;
	//unsigned char cvt_digital_gain;
		
#if 1	// From agoritham calc new data update to jxh42_seInfo.
	jxh42_seInfo.sensor_ev_idx += si->ae_ev_idx;
	if(jxh42_seInfo.sensor_ev_idx >= jxh42_seInfo.max_ev_idx) jxh42_seInfo.sensor_ev_idx = jxh42_seInfo.max_ev_idx;
	if(jxh42_seInfo.sensor_ev_idx < 0) jxh42_seInfo.sensor_ev_idx = 0;
	
	idx = jxh42_seInfo.sensor_ev_idx * 3;
	jxh42_seInfo.time = p_expTime_table[idx];
	jxh42_seInfo.analog_gain = p_expTime_table[idx+1];
	jxh42_seInfo.digital_gain = p_expTime_table[idx+2];
	
	jxh42_seInfo.userISO = si->userISO;
#endif	
	//DBG_PRINT("Time = %d, a gain = %d, d gain = %d, ev idx = %d\r\n", jxh42_seInfo.time, jxh42_seInfo.analog_gain, jxh42_seInfo.digital_gain, jxh42_seInfo.sensor_ev_idx);

	// set exposure time
	if (( 0x2EE < jxh42_seInfo.time)&&(jxh42_seInfo.time <= 0x384))
	//if (( 0x354 < jxh42_seInfo.time)&&(jxh42_seInfo.time <= 0x400))
	{
		if(sensor_get_fps() != 25)
		{
			sensor_set_fps(25);	
			DBG_PRINT("f25");		
		}
	}
	else if (jxh42_seInfo.time > 0x384)
	//else if (jxh42_seInfo.time > 0x400)
	{
		if(sensor_get_fps() != 20)
		{
			sensor_set_fps(20);
			DBG_PRINT("f20");
		}
	}
	else
	{
		if(sensor_get_fps() != 30)
		{
			sensor_set_fps(30);
			DBG_PRINT("f30");
		}
	}


	lsb_time = (jxh42_seInfo.time & 0xFF);
	msb_time = ((jxh42_seInfo.time >>8 )& 0xFF);
	sccb_write(SOI_H42_ID, 0x01 , lsb_time );
	sccb_write(SOI_H42_ID, 0x02 , msb_time );

	if (jxh42_seInfo.sensor_ev_idx <= BLACK_SUN_IDX)
	{
		if (black_sun_flag == 0)
		{	
			sccb_write(SOI_H42_ID, 0x0C , 0x00);		//enable black sun
			black_sun_flag = 1;	
			DBG_PRINT("s");
		}
	}
	else if (jxh42_seInfo.sensor_ev_idx > BLACK_SUN_IDX)
	{
		if (black_sun_flag == 1)
		{	
			sccb_write(SOI_H42_ID, 0x0C , 0x40);		//disble black sun
			black_sun_flag = 0;	
			DBG_PRINT("y");
		}	
	}
	//DBG_PRINT("\r\n<H42_set_exposure_time> time = %d, Lsb_time: 0x%x, Msb_time: 0x%x\r\n",jxh42_seInfo.time,lsb_time,msb_time);
	
	return 0;
}


void H42_set_exposure_gain(void)
{
	int analog_gain, digital_gain;
	
	analog_gain = H42_cvt_analog_gain(jxh42_seInfo.analog_gain);
	//DBG_PRINT("analog_gain = 0x%x\r\n", analog_gain);
	sccb_write(SOI_H42_ID, 0x00, analog_gain );
	
	digital_gain =jxh42_seInfo.digital_gain >> 3;
	hwCdsp_SetGlobalGain(digital_gain);	
}


void H42_get_exposure_time(sensor_exposure_t *se)
{
	gp_memcpy((INT8S *)se, (INT8S *)&jxh42_seInfo, sizeof(sensor_exposure_t));
}


void H42_set_exp_freq(int freq)
{
	if(freq == 50)
	{
			jxh42_seInfo.sensor_ev_idx = H42_50HZ_INIT_EV_IDX;
			jxh42_seInfo.ae_ev_idx = 0;
			jxh42_seInfo.daylight_ev_idx= H42_50HZ_DAY_EV_IDX;
			jxh42_seInfo.night_ev_idx= H42_50HZ_NIGHT_EV_IDX;			
			jxh42_seInfo.max_ev_idx = H42_50HZ_MAX_EXP_IDX - 1;
			p_expTime_table = (int *)g_h42_exp_time_gain_50Hz;
	}
	else if(freq == 60)
	{
		jxh42_seInfo.sensor_ev_idx = H42_60HZ_INIT_EV_IDX;
		jxh42_seInfo.ae_ev_idx = 0;
		jxh42_seInfo.daylight_ev_idx= H42_60HZ_DAY_EV_IDX;
		jxh42_seInfo.night_ev_idx= H42_60HZ_NIGHT_EV_IDX;
		jxh42_seInfo.max_ev_idx = H42_60HZ_MAX_EXP_IDX - 1;
		p_expTime_table = (int *)g_h42_exp_time_gain_60Hz;
	}
}

static int H42_init(void)
{
	jxh42_seInfo.max_time = H42_MAX_EXPOSURE_TIME;
	jxh42_seInfo.min_time = H42_MIN_EXPOSURE_TIME;

	jxh42_seInfo.max_digital_gain = H42_MAX_DIGITAL_GAIN ;
	jxh42_seInfo.min_digital_gain = H42_MIN_DIGITAL_GAIN ;

	jxh42_seInfo.max_analog_gain = H42_MAX_ANALOG_GAIN;
	jxh42_seInfo.min_analog_gain = H42_MIN_ANALOG_GAIN;

	jxh42_seInfo.analog_gain = jxh42_seInfo.min_analog_gain;
	jxh42_seInfo.digital_gain = jxh42_seInfo.min_digital_gain;
	jxh42_seInfo.time = jxh42_seInfo.max_time;// >> 1;
	
	H42_set_exp_freq(50);
	
	DBG_PRINT("H42_init\r\n");
	return 0;
}

INT8U ver_check(void)
{
	INT8U h42_version;
	
	h42_version = sccb_read(SOI_H42_ID, 0x09);
	DBG_PRINT("\r\nH42_version = 0x%x\r\n", h42_version);
	
	return h42_version;
}

void reg_chang1(void)
{
	sccb_write(SOI_H42_ID, 0x27, 0x49);
	sccb_write(SOI_H42_ID, 0x2C, 0x00);
	sccb_write(SOI_H42_ID, 0x63, 0x19);		
}

void reg_chang2(void)
{
	sccb_write(SOI_H42_ID, 0x27, 0x3B);
	sccb_write(SOI_H42_ID, 0x2C, 0x04);
	sccb_write(SOI_H42_ID, 0x63, 0x59);		
}


void sensor_SOi_h42_init(INT32U WIDTH, INT32U HEIGHT)
{
	INT32U i;

	H42_init();
	sensor_calibration_str();
	if(sensor_format == SOI_H42_RAW){
		if(WIDTH == 1280 && HEIGHT == 720)
		{
			for (i=0; i<sizeof(JXH42_1280x720x30_DVP_10b)/2; i++) 
			{
				sccb_write(SOI_H42_ID,JXH42_1280x720x30_DVP_10b[i][0], JXH42_1280x720x30_DVP_10b[i][1]);	
			}
#if C_DISPLAY_REVERSE
			if (h42_flip_mode==1) {
				sccb_write(SOI_H42_ID, 0x12, 0x30);	// [0]mirror [1]flip
			}
			else {
				sccb_write(SOI_H42_ID, 0x12, 0x00);	// [0]mirror [1]flip
			}
#endif
		}			
		else 
		{
			while(1);
		}
	}else if(sensor_format == SOI_H42_MIPI){
		if(WIDTH == 1280 && HEIGHT == 800)
		{
			for (i=0; i<sizeof(H42_MIPI_1280_800_30)/2; i++) 
			{
				sccb_write(SOI_H42_ID,H42_MIPI_1280_800_30[i][0], H42_MIPI_1280_800_30[i][1]);
			}
		}	
		else if	(WIDTH == 1280 && HEIGHT == 720)
		{
			for (i=0; i<sizeof(JXH42_1280x720x30_Mipi_1L_10b)/2; i++) 
			{
				sccb_write(SOI_H42_ID,JXH42_1280x720x30_Mipi_1L_10b[i][0], JXH42_1280x720x30_Mipi_1L_10b[i][1]);
			}
#if C_DISPLAY_REVERSE
			if (h42_flip_mode==1) {
				sccb_write(SOI_H42_ID, 0x12, 0x30);	// [0]mirror [1]flip
			}
			else {
				sccb_write(SOI_H42_ID, 0x12, 0x00);	// [0]mirror [1]flip
			}
#endif
			
			if((ver_check() == 0)||(ver_check() == 0x80))	//SOI suggetion do it.
			{
				reg_chang1();
			}
			else if (ver_check() == 0x81)
			{
				reg_chang2();
			}
		}	
		else if(WIDTH == 640 && HEIGHT == 480)
		{
			for (i=0; i<sizeof(SOI_H42_MIPI_VGA_f60)/2; i++) 
			{
				sccb_write(SOI_H42_ID,SOI_H42_MIPI_VGA_f60[i][0], SOI_H42_MIPI_VGA_f60[i][1]);
			}			
		}		
		else 
		{
			while(1);
		}
	}
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(USE_SENSOR_NAME == SENSOR_SOI_H42)     //
//================================================================//

#include "drv_l1_i2c.h"
#include "drv_l2_sensor.h"
#include "drv_l1_cdsp.h"
#include "gp_aeawb.h"

#include "LDWs.h"

#include "sensor_soi_h22.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (USE_SENSOR_NAME == SENSOR_SOI_H22)
//================================================================//
static sensor_exposure_t jxh22_seInfo;
//static sensor_exposure_t seInfo;
static sensor_calibration_t h22_cdsp_calibration;
static int *p_expTime_table;
//static int sensor_max_ev_idx;
extern INT8U LDWS_Enable_Flag;
extern void sensor_set_fps(INT32U fpsValue);
extern INT32U sensor_get_fps(void);

static gpCdspWBGain_t soi_h22_wbgain;
#if C_DISPLAY_REVERSE
static INT32U h22_flip_mode = SENSOR_FLIP;
#endif
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
 #if C_DISPLAY_REVERSE
void h22_set_flip_mode(int mode)
{
	h22_flip_mode = mode;
}
#endif
sensor_exposure_t *jxh22_get_senInfo(void)
{
	return &jxh22_seInfo;
}

void sensor_calibration_str(void)
{
	//sensor_calibration_t h22_cdsp_calibration;

	//H22_cdsp_calibration.lenscmp = g_H22_lenscmp_table;	//GPCV1248 not support
	h22_cdsp_calibration.r_b_gain = g_H22_r_b_gain;
/*
	h22_cdsp_calibration.gamma = g_H22_gamma_045_table;
	h22_cdsp_calibration.color_matrix = g_H22_color_matrix4gamma045;
	h22_cdsp_calibration.awb_thr = g_H22_awb_thr;
*/
}

sensor_calibration_t *jxh22_get_calibration(void)
{
	return &h22_cdsp_calibration;
}
void SOi_h22_sensor_calibration(void)
{
	//OB
	gp_Cdsp_SetBadPixOb((INT16U *)g_h22_badpix_ob_table);
	//Gamma
	hwCdsp_InitGamma((INT32U *)g_H22_gamma_045_table);
	//Color Correction
	hwCdsp_SetColorMatrix_Str((INT16S *)g_H22_color_matrix4gamma045);
	//AWB
	gp_Cdsp_SetAWBYUV((INT16S *)g_H22_awb_thr);
	
	//Lenscmp
	/*
	hwIsp_luc_MaxTan8_Slop_CLP((INT16U *)g_H22_MaxTan8 ,(INT16U *)g_H22_Slope4 ,(INT16U *)g_H22_CLPoint);
	hwIsp_RadusF0((INT16U *)g_H22_Radius_File_0);
	hwIsp_RadusF1((INT16U *)g_H22_Radius_File_1);
	*/
	//R,G,B Linearity correction
	//hwIsp_InitLiCor((INT8U *)LiTable_rgb);		
}

gpCdspWBGain_t *soi_h22_awb_r_b_gain_boundary(void)
{
	int i;
	int max_r_gain, max_b_gain, min_r_gain, min_b_gain;
	
	max_r_gain = max_b_gain = 0;
	min_r_gain = min_b_gain = 255;
	
	for(i = 10 ; i < 55 ; i++)
	{
		if(max_r_gain < g_H22_r_b_gain[i][0]) max_r_gain = g_H22_r_b_gain[i][0];
		if(max_b_gain < g_H22_r_b_gain[i][1]) max_b_gain = g_H22_r_b_gain[i][1];
		if(min_r_gain > g_H22_r_b_gain[i][0]) min_r_gain = g_H22_r_b_gain[i][0];
		if(min_b_gain > g_H22_r_b_gain[i][1]) min_b_gain = g_H22_r_b_gain[i][1];
	}
	
	soi_h22_wbgain.max_rgain = max_r_gain;
	soi_h22_wbgain.max_bgain = max_b_gain;
	soi_h22_wbgain.min_rgain = min_r_gain;
	soi_h22_wbgain.min_bgain = min_b_gain;

	return &soi_h22_wbgain;
}

static int H22_analog_gain_table[65] = 
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

	// coarse gain = 3
	(int)(4.00*256+0.5), (int)(4.25*256+0.5), (int)(4.50*256+0.5), (int)(4.75*256+0.5), 
	(int)(5.00*256+0.5), (int)(5.25*256+0.5), (int)(5.50*256+0.5), (int)(5.75*256+0.5), 
	(int)(6.00*256+0.5), (int)(6.25*256+0.5), (int)(6.50*256+0.5), (int)(6.75*256+0.5), 
	(int)(7.00*256+0.5), (int)(7.25*256+0.5), (int)(7.50*256+0.5), (int)(7.75*256+0.5), 
	
	// coarse gain = 7
	(int)(8.00*256+0.5), (int)(8.50*256+0.5), (int)(9.00*256+0.5), (int)(9.50*256+0.5), 
	(int)(10.00*256+0.5), (int)(10.50*256+0.5), (int)(11.00*256+0.5), (int)(11.50*256+0.5), 
	(int)(12.00*256+0.5), (int)(12.50*256+0.5), (int)(13.00*256+0.5), (int)(13.50*256+0.5), 
	(int)(14.00*256+0.5), (int)(14.50*256+0.5), (int)(15.00*256+0.5), (int)(15.50*256+0.5), 
	
	// coarse gain = 15
	(int)(16.00*256+0.5)
};


/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
#if 1
static int H22_cvt_analog_gain(int analog_gain)
{
	int i;
	int coarse_gain, fine_gain;
	int *p_H22_analog_gain_table = H22_analog_gain_table;

	for( i = 0 ; i < 65 ; i++)
	{
		if(analog_gain >= p_H22_analog_gain_table[i] && analog_gain < p_H22_analog_gain_table[i+1])
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
		coarse_gain = 3;
		fine_gain = (i - 16 - 16);
	}
	else if(i < (16 + 16 + 16 + 16))
	{
		coarse_gain = 7;
		fine_gain = (i - 16 - 16 -16);
	}
	else
	{
		coarse_gain = 15;
		fine_gain = 0;
	}

	//DBG_PRINT("%d: coarse_gain = 0x%x, fine_gain = 0x%x\n\r", i, coarse_gain, fine_gain);
	return ((coarse_gain << 4) | fine_gain);
}


#if 0
static int H22_get_real_analog_gain(int analog_gain)
{
	int real_analog_gain;
	int coarse_gain, fine_gain;

	coarse_gain = (analog_gain >> 4) & 0x7;
	fine_gain = (analog_gain & 0xf);

	if(coarse_gain == 0)
	{
		real_analog_gain = H22_analog_gain_table[fine_gain];
	}
	else	if(coarse_gain == 1)
	{
		real_analog_gain = H22_analog_gain_table[16 + fine_gain];
	}
	else	if(coarse_gain == 3)
	{
		real_analog_gain = H22_analog_gain_table[16 + 16 + fine_gain];
	}
	else	if(coarse_gain == 7)
	{
		real_analog_gain = H22_analog_gain_table[16 + 16 + 16 + fine_gain];
	}
	else	if(coarse_gain == 15)
	{
		real_analog_gain = H22_analog_gain_table[16 + 16 + 16 + 16];
	}
	else
	{
		DBG_PRINT("H22 analog gain Err!\r\n");
	}
	
	
	return real_analog_gain;
}
#endif


int H22_get_night_ev_idx(void)
{
	return jxh22_seInfo.night_ev_idx;
}

int H22_get_max_ev_idx(void)
{
	return jxh22_seInfo.max_ev_idx;
}



int H22_set_exposure_time(sensor_exposure_t *si)
{
	//int ret=0;
	//unsigned short tmp;
	//int analog_gain, digital_gain;
	int lsb_time, msb_time;
	int idx;
	//unsigned char cvt_digital_gain;
		
#if 1	// From agoritham calc new data update to jxh22_seInfo.
	jxh22_seInfo.sensor_ev_idx += si->ae_ev_idx;
	if(jxh22_seInfo.sensor_ev_idx >= jxh22_seInfo.max_ev_idx) jxh22_seInfo.sensor_ev_idx = jxh22_seInfo.max_ev_idx;
	if(jxh22_seInfo.sensor_ev_idx < 0) jxh22_seInfo.sensor_ev_idx = 0;
	
	idx = jxh22_seInfo.sensor_ev_idx * 3;
	jxh22_seInfo.time = p_expTime_table[idx];
	jxh22_seInfo.analog_gain = p_expTime_table[idx+1];
	jxh22_seInfo.digital_gain = p_expTime_table[idx+2];
	
	jxh22_seInfo.userISO = si->userISO;
#endif	
	//DBG_PRINT("Time = %d, a gain = %d, d gain = %d, ev idx = %d\r\n", jxh22_seInfo.time, jxh22_seInfo.analog_gain, jxh22_seInfo.digital_gain, jxh22_seInfo.sensor_ev_idx);

	//analog_gain = H22_cvt_analog_gain(jxh22_seInfo.analog_gain);
	
	// set exposure time

	if (( 0x354 < jxh22_seInfo.time)&&(jxh22_seInfo.time <= 0x400))
	{
		if(sensor_get_fps() != 25)
		{
			sensor_set_fps(25);			
		}
	}
	else if (jxh22_seInfo.time > 0x400)
	{
		if(sensor_get_fps() != 20)
		{
			sensor_set_fps(20);
		}
	}
	else
	{
		if(sensor_get_fps() != 30)
		{
			sensor_set_fps(30);
		}
	}


	lsb_time = (jxh22_seInfo.time & 0xFF);
	msb_time = ((jxh22_seInfo.time >>8 )& 0xFF);
	sccb_write(SOI_H22_ID, 0x01 , lsb_time );
	sccb_write(SOI_H22_ID, 0x02 , msb_time );
	// set Analog gain	
	//sccb_write(SOI_H22_ID, 0x00, analog_gain ); 
	//DBG_PRINT("<H22_set_exposure_time> Lib_analog_gain = 0x%x, cvt_analog_gain = 0x%x\r\n",jxh22_seInfo.analog_gain, analog_gain);
		
	// set Digital gain
	//with cdsp global gain
	//digital_gain =jxh22_seInfo.digital_gain >> 3;
	//hwCdsp_SetGlobalGain(digital_gain);	
	//DBG_PRINT("<H22_set_exposure_time> digital gain = 0x%x, cvt_digital_gain: 0x%x\r\n",jxh22_seInfo.digital_gain, digital_gain);


	//DBG_PRINT("\r\n<H22_set_exposure_time> time = %d, Lsb_time: 0x%x, Msb_time: 0x%x\r\n",jxh22_seInfo.time,lsb_time,msb_time);
	
	return 0;
}


void H22_set_exposure_gain(void)
{
	int analog_gain, digital_gain;
	
	analog_gain = H22_cvt_analog_gain(jxh22_seInfo.analog_gain);
	//DBG_PRINT("analog_gain = 0x%x\r\n", analog_gain);
	sccb_write(SOI_H22_ID, 0x00, analog_gain );
	
	digital_gain =jxh22_seInfo.digital_gain >> 3;
	hwCdsp_SetGlobalGain(digital_gain);	
}


void H22_get_exposure_time(sensor_exposure_t *se)
{
#if 0
	unsigned char tmp1,tmp2;
	unsigned char tmp;
	int g_time;

	// Analog gain
	//sccb_delay(100);
	tmp =sccb_read(SOI_H22_ID,0x00);  
	se->analog_gain= H22_get_real_analog_gain(tmp);
	//DBG_PRINT("<H22_get_exposure_time>se->analog_gain:0x%x, g_an_gain:0x%x\r\n",se->analog_gain,tmp);

	// coase time
	//sccb_delay(100);
	tmp1 =sccb_read(SOI_H22_ID,0x01); 
	//DBG_PRINT("<0H22_get_exposure_time>lsb_time:0x%x, msb_time:0x%x\r\n",tmp1,tmp2);

	//sccb_delay(100);
	tmp2 =sccb_read(SOI_H22_ID,0x02);
	//DBG_PRINT("<1H22_get_exposure_time>lsb_time:0x%x, msb_time:0x%x\r\n",tmp1,tmp2);

	g_time = tmp1 | (tmp2 << 8);
	//DBG_PRINT("<2H22_get_exposure_time>time = 0x%x, lsb_time:0x%x, msb_time:0x%x\r\n",g_time,tmp1,tmp2);
	se->time = g_time;
		
	// digital gain
	//sccb_delay(100);
	#if 0
	tmp =sccb_read(SOI_H22_ID,0x0D);
	se->digital_gain =  H22_get_real_digital_gain(tmp);
	//se->digital_gain = ((tmp & 0x3) << 1);	//	seInfo.digital_gain = seInfo.digital_gain << 1;
	#else
	tmp = hwCdsp_GetGlobalGain();
	se->digital_gain =  tmp << 3;
	#endif
	//DBG_PRINT("<H22_get_exposure_time>se->digital_gain = 0x%x\r\n",se->digital_gain);

	se->max_time = jxh22_seInfo.max_time;                    
	se->min_time = jxh22_seInfo.min_time;                    
                                                         
	se->max_digital_gain = jxh22_seInfo.max_digital_gain;    
	se->min_digital_gain = jxh22_seInfo.min_digital_gain;    
                                                         
	se->max_analog_gain = jxh22_seInfo.max_analog_gain;      
	se->min_analog_gain = jxh22_seInfo.min_analog_gain;      
	
	se->sensor_ev_idx = jxh22_seInfo.sensor_ev_idx;   
	se->night_ev_idx = jxh22_seInfo.night_ev_idx;   
	se->max_ev_idx = jxh22_seInfo.max_ev_idx;   
	se->daylight_ev_idx = jxh22_seInfo.daylight_ev_idx;   
	                                                         
	//se->analog_gain = jxh22_seInfo.analog_gain;              
	//se->digital_gain = jxh22_seInfo.digital_gain;            
	//se->time = jxh22_seInfo.time;// >> 1;                               
	//return ret;
#else
	gp_memcpy((INT8S *)se, (INT8S *)&jxh22_seInfo, sizeof(sensor_exposure_t));
#endif
}
#endif


void H22_set_exp_freq(int freq)
{
	if(freq == 50)
	{
			jxh22_seInfo.sensor_ev_idx = H22_50HZ_INIT_EV_IDX;
			jxh22_seInfo.ae_ev_idx = 0;
			jxh22_seInfo.daylight_ev_idx= H22_50HZ_DAY_EV_IDX;
			jxh22_seInfo.night_ev_idx= H22_50HZ_NIGHT_EV_IDX;			
			jxh22_seInfo.max_ev_idx = H22_50HZ_MAX_EXP_IDX - 1;
			p_expTime_table = (int *)g_h22_exp_time_gain_50Hz;
	}
	else if(freq == 60)
	{
		jxh22_seInfo.sensor_ev_idx = H22_60HZ_INIT_EV_IDX;
		jxh22_seInfo.ae_ev_idx = 0;
		jxh22_seInfo.daylight_ev_idx= H22_60HZ_DAY_EV_IDX;
		jxh22_seInfo.night_ev_idx= H22_60HZ_NIGHT_EV_IDX;
		jxh22_seInfo.max_ev_idx = H22_60HZ_MAX_EXP_IDX - 1;
		p_expTime_table = (int *)g_h22_exp_time_gain_60Hz;
	}
}



#if 1
static int H22_init(void)
{
	jxh22_seInfo.max_time = H22_MAX_EXPOSURE_TIME;
	jxh22_seInfo.min_time = H22_MIN_EXPOSURE_TIME;

	jxh22_seInfo.max_digital_gain = H22_MAX_DIGITAL_GAIN ;
	jxh22_seInfo.min_digital_gain = H22_MIN_DIGITAL_GAIN ;

	jxh22_seInfo.max_analog_gain = H22_MAX_ANALOG_GAIN;
	jxh22_seInfo.min_analog_gain = H22_MIN_ANALOG_GAIN;

	jxh22_seInfo.analog_gain = jxh22_seInfo.min_analog_gain;
	jxh22_seInfo.digital_gain = jxh22_seInfo.min_digital_gain;
	jxh22_seInfo.time = jxh22_seInfo.max_time;// >> 1;
	
	H22_set_exp_freq(50);
	
	DBG_PRINT("H22_init\r\n");
	return 0;
}
#endif



void sensor_SOi_h22_init(INT32U WIDTH, INT32U HEIGHT)
{
	//i2c_bus_handle_t i2c_handle; 
	INT32U i;
	/*
	  °Ñ¼Æ³]­È
	*/
	//i2c_handle.slaveAddr = 0x60;
	//i2c_handle.clkRate = 100;
	
	H22_init();
	sensor_calibration_str();
	if(sensor_format == SOI_H22_RAW){
		if(WIDTH == 1280 && HEIGHT == 720)
		{
			for (i=0; i<sizeof(H22_Para_720P_30_10b_384)/2; i++) //sizeof(H22_Para_720P_30)
			{
				//sccb_write(SOI_H22_ID, H22_Para_720P_30[i][0], H22_Para_720P_30[i][1]);
				sccb_write(SOI_H22_ID, H22_Para_720P_30_10b_384[i][0], H22_Para_720P_30_10b_384[i][1]);
				//sccb_write(SOI_H22_ID, H22_Para_720P_30_10b_432[i][0], H22_Para_720P_30_10b_432[i][1]);
			}
#if C_DISPLAY_REVERSE
			if (h22_flip_mode==1) {
				sccb_write(SOI_H22_ID, 0x12, 0x30);	// [0]mirror [1]flip
			}
			else {
				sccb_write(SOI_H22_ID, 0x12, 0x00);	// [0]mirror [1]flip
			}
#endif
		}			
		else 
		{
			while(1);
		}
	}else if(sensor_format == SOI_H22_MIPI){
		if(WIDTH == 1280 && HEIGHT == 800)
		{
			for (i=0; i<sizeof(H22_MIPI_1280_800_30)/2; i++) 
			{
				//reg_1byte_data_1byte_write(&i2c_handle,H22_MIPI_1280_800_30[i][0], H22_MIPI_1280_800_30[i][1]);
				sccb_write(SOI_H22_ID,H22_MIPI_1280_800_30[i][0], H22_MIPI_1280_800_30[i][1]);
			}
		}	
		else if	(WIDTH == 1280 && HEIGHT == 720)
		{
			for (i=0; i<sizeof(SOI_H22_MIPI_720P)/2; i++) 
			{
				//reg_1byte_data_1byte_write(&i2c_handle,SOI_H22_MIPI_720P[i][0], SOI_H22_MIPI_720P[i][1]);
				sccb_write(SOI_H22_ID,SOI_H22_MIPI_720P[i][0], SOI_H22_MIPI_720P[i][1]);
			}
#if C_DISPLAY_REVERSE
			if (h22_flip_mode==1) {
				sccb_write(SOI_H22_ID, 0x12, 0x30);	// [0]mirror [1]flip
			}
			else {
				sccb_write(SOI_H22_ID, 0x12, 0x00);	// [0]mirror [1]flip
			}
#endif
		}	
		else if(WIDTH == 640 && HEIGHT == 480)
		{
			for (i=0; i<sizeof(SOI_H22_MIPI_VGA_f60)/2; i++) 
			{
				//reg_1byte_data_1byte_write(&i2c_handle,SOI_H22_MIPI_VGA_f60[i][0], SOI_H22_MIPI_VGA_f60[i][1]);
				sccb_write(SOI_H22_ID,SOI_H22_MIPI_VGA_f60[i][0], SOI_H22_MIPI_VGA_f60[i][1]);
			}			
		}		
		else 
		{
			while(1);
		}
	}
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(USE_SENSOR_NAME == SENSOR_SOI_H22)     //
//================================================================//

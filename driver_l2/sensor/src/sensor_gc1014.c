#include "drv_l1_i2c.h"
#include "drv_l2_sensor.h"
#include "drv_l1_cdsp.h"
#include "gp_aeawb.h"

#include "sensor_gc1014.h"


//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (USE_SENSOR_NAME == SENSOR_GC1014)
//================================================================//
//Set Gain
#if 0//(!GC1014_GC_AE)
#define ANALOG_GAIN_1 64 		// 1.00x
#define ANALOG_GAIN_2 90 		// 1.4x
#define ANALOG_GAIN_3 118 		// 1.8x
#define ANALOG_GAIN_4 163 		// 2.56x
#define ANALOG_GAIN_5 218 		// 3.40x
#define ANALOG_GAIN_6 304 		// 4.7x
#define ANALOG_GAIN_7 438 		// 6.84x
#define ANALOG_GAIN_8 602 		// 9.4x
#define ANALOG_GAIN_9 851 		// 13.2x
#endif


INT32U GC_CLK	= GC_42MHZ;

static sensor_exposure_t gc1014_seInfo;
static sensor_calibration_t gc1014_cdsp_calibration;
//static INT32U gc1014_digital_gain = 0x180; //SENSOR_GC1014
static INT32U gc1014_analog_gain = 0x100;
static int *p_expTime_table;
//static int sensor_max_ev_idx;
static gpCdspWBGain_t gc1014_wbgain;
#if C_DISPLAY_REVERSE
static INT32U gc1014_flip_mode = SENSOR_FLIP;
#endif
/**************************************************************************
 *                         SENSEOR FUNCTION                          *
 **************************************************************************/
#if C_DISPLAY_REVERSE
void gc1014_set_flip_mode(int mode)
{
	gc1014_flip_mode = mode;
}
#endif
sensor_exposure_t *gc1014_get_senInfo(void)
{
	return &gc1014_seInfo;
}

void gc1014_sensor_calibration_str(void)
{
	gc1014_cdsp_calibration.r_b_gain = g_GC1014_r_b_gain;
	/*
	gc1014_cdsp_calibration.gamma = g_GC1014_gamma_045_table_5;
	gc1014_cdsp_calibration.color_matrix = g_GC1014_color_matrix4gamma045_7;
	gc1014_cdsp_calibration.awb_thr = g_GC1014_awb_thr_2;
	*/
}

sensor_calibration_t *gc1014_get_calibration(void)
{
	return &gc1014_cdsp_calibration;
}

void gc1014_sensor_calibration(void)
{
	//OB
	gp_Cdsp_SetBadPixOb((INT16U *)g_GC1014_badpix_ob_table);
	//Linearity
	/*
	hwIsp_LinCorr_Table((INT8U *)g_GC1014_LiTable_rgb);
	//Lenscmp
	hwIsp_luc_MaxTan8_Slop_CLP((INT16U *)g_GC1014_MaxTan8 ,(INT16U *)g_GC1014_Slope4 ,(INT16U *)g_GC1014_CLPoint);
	hwIsp_RadusF0((INT16U *)g_GC1014_Radius_File_0);
	hwIsp_RadusF1((INT16U *)g_GC1014_Radius_File_1);
	*/		
	//Gamma
	hwCdsp_InitGamma((INT32U *)g_GC1014_gamma_045_table);
	//Color Correction
	hwCdsp_SetColorMatrix_Str((INT16S *)g_GC1014_color_matrix4gamma045);
	//AWB
	gp_Cdsp_SetAWBYUV((INT16S *)g_GC1014_awb_thr);
}

int gc1014_get_night_ev_idx(void)
{
	return gc1014_seInfo.night_ev_idx;
}

int gc1014_get_max_ev_idx(void)
{
	return gc1014_seInfo.max_ev_idx;
}

gpCdspWBGain_t *gc1014_awb_r_b_gain_boundary(void)
{
	int i;
	int max_r_gain, max_b_gain, min_r_gain, min_b_gain;
	
	max_r_gain = max_b_gain = 0;
	min_r_gain = min_b_gain = 255;
	
	for(i = 10 ; i < 55 ; i++)
	{
		if(max_r_gain < g_GC1014_r_b_gain[i][0]) max_r_gain = g_GC1014_r_b_gain[i][0];
		if(max_b_gain < g_GC1014_r_b_gain[i][1]) max_b_gain = g_GC1014_r_b_gain[i][1];
		if(min_r_gain > g_GC1014_r_b_gain[i][0]) min_r_gain = g_GC1014_r_b_gain[i][0];
		if(min_b_gain > g_GC1014_r_b_gain[i][1]) min_b_gain = g_GC1014_r_b_gain[i][1];
	}
	
	gc1014_wbgain.max_rgain = max_r_gain;
	gc1014_wbgain.max_bgain = max_b_gain;
	gc1014_wbgain.min_rgain = min_r_gain;
	gc1014_wbgain.min_bgain = min_b_gain;
	
	return &gc1014_wbgain;
}
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
/*
static int gc1014_analog_gain_table[9] = 
{
	(int)(1.00*256+0.5), (int)(1.40*256+0.5), (int)(1.80*256+0.5), (int)(2.60*256+0.5), 
	(int)(3.40*256+0.5), (int)(4.70*256+0.5), (int)(6.84*256+0.5), (int)(9.40*256+0.5), 
	(int)(13.2*256+0.5)
};


static int gc1014_analog_gain_table[49] = 
{
	//digital gain x 1
	(int)(1.00*256+0.5), (int)(1.06*256+0.5), (int)(1.13*256+0.5), (int)(1.19*256+0.5),
	(int)(1.25*256+0.5), (int)(1.31*256+0.5), (int)(1.38*256+0.5), (int)(1.44*256+0.5),
	(int)(1.50*256+0.5), (int)(1.56*256+0.5), (int)(1.63*256+0.5), (int)(1.69*256+0.5), 
	(int)(1.75*256+0.5), (int)(1.81*256+0.5), (int)(1.88*256+0.5), (int)(1.94*256+0.5),
	
	//digital gain x 2
	(int)(2.00*256+0.5), (int)(2.06*256+0.5), (int)(2.13*256+0.5), (int)(2.19*256+0.5),
	(int)(2.25*256+0.5), (int)(2.31*256+0.5), (int)(2.38*256+0.5), (int)(2.44*256+0.5),
	(int)(2.50*256+0.5), (int)(2.56*256+0.5), (int)(2.63*256+0.5), (int)(2.69*256+0.5),
	(int)(2.75*256+0.5), (int)(2.81*256+0.5), (int)(2.88*256+0.5), (int)(2.94*256+0.5),

	//digital gain x 3
	(int)(3.00*256+0.5), (int)(3.06*256+0.5), (int)(3.13*256+0.5), (int)(3.19*256+0.5),           
	(int)(3.25*256+0.5), (int)(3.31*256+0.5), (int)(3.38*256+0.5), (int)(3.44*256+0.5),           
	(int)(3.50*256+0.5), (int)(3.56*256+0.5), (int)(3.63*256+0.5), (int)(3.69*256+0.5),           
	(int)(3.75*256+0.5), (int)(3.81*256+0.5), (int)(3.88*256+0.5), (int)(3.94*256+0.5),           


	//digital gain x 4
	(int)(4.00*256+0.5),
};
*/

#if 0//(!GC1014_GC_AE)
static void gc1014_cvt_analog_gain(INT32U analog_gain)
{
	INT32U iReg, temp;
	//INT32U coarse_gain, fine_gain;

	gc1014_analog_gain = analog_gain;                                                          

	iReg = analog_gain >> 2;
	
	sccb_write(GC1014_SLAVE_ID, 0xb1, 0x01);   

	if(iReg < 0x40) {
		iReg = 0x40; 
		sccb_write(GC1014_SLAVE_ID, 0xb1, 0x01);                               
		sccb_write(GC1014_SLAVE_ID, 0xb2, 0x00);                                                                                  
	}                                                        
	else if((ANALOG_GAIN_1<= iReg)&&(iReg < ANALOG_GAIN_2))                
	{                                                                      
		//analog gain                                                          
		temp = iReg;                                                           
		//sccb_write(GC1014_SLAVE_ID, 0xb1, temp>>6);                               
		sccb_write(GC1014_SLAVE_ID, 0xb2, (temp<<2)&0xfc);                     

		sccb_write(GC1014_SLAVE_ID, 0xb6, 0x00);//                                   
		//DBG_PRINT("GC1014 analogic gain 1x , GC1014 add pregain = %d\r\n",temp);  
		//DBG_PRINT("a:1x,p:%d\r\n",(temp<<2)&0xfc);  
	}                                                                      
	else if((ANALOG_GAIN_2<= iReg)&&(iReg < ANALOG_GAIN_3))                
	{                                                                      
		temp = 64*iReg/ANALOG_GAIN_2;                                          
		//sccb_write(GC1014_SLAVE_ID, 0xb1, temp>>6);                               
		sccb_write(GC1014_SLAVE_ID, 0xb2, (temp<<2)&0xfc);                 
		
		sccb_write(GC1014_SLAVE_ID, 0xb6, 0x01);//  
		//DBG_PRINT("a:1.4x,p:%d\r\n",(temp<<2)&0xfc);                                       
		//DBG_PRINT("GC1014 analogic gain 1.4x , GC1014 add pregain = %d\r\n",temp);
	}                                                                      
	else if((ANALOG_GAIN_3<= iReg)&&(iReg < ANALOG_GAIN_4))                
	{                                                                                       
		temp = 64*iReg/ANALOG_GAIN_3;                                          
		//sccb_write(GC1014_SLAVE_ID, 0xb1, temp>>6);                               
		sccb_write(GC1014_SLAVE_ID, 0xb2, (temp<<2)&0xfc);                 
		
		sccb_write(GC1014_SLAVE_ID, 0xb6, 0x02);//                      
		//DBG_PRINT("a:1.8x,p:%d\r\n",(temp<<2)&0xfc);  
		//DBG_PRINT("GC1014 analogic gain 1.8x , GC1014 add pregain = %d\r\n",temp);
	}                                                                      
	else if((ANALOG_GAIN_4<= iReg)&&(iReg < ANALOG_GAIN_5))                
	{                                                                      
		temp = 64*iReg/ANALOG_GAIN_4;       
		//sccb_write(GC1014_SLAVE_ID, 0xb1, temp>>6);                                                                   
		sccb_write(GC1014_SLAVE_ID, 0xb2, (temp<<2)&0xfc);                 
		
		sccb_write(GC1014_SLAVE_ID, 0xb6, 0x03);
		//DBG_PRINT("a:2.56x,p:%d\r\n",(temp<<2)&0xfc);                                            
		//DBG_PRINT("GC1014 analogic gain 2.56x , GC1014 add pregain = %d\r\n",temp);
	}                                                                       
	else if((ANALOG_GAIN_5<= iReg)&&(iReg < ANALOG_GAIN_6))                 
	{                                                                       
		temp = 64*iReg/ANALOG_GAIN_5;                                           
		//sccb_write(GC1014_SLAVE_ID, 0xb1, temp>>6);                                
		sccb_write(GC1014_SLAVE_ID, 0xb2, (temp<<2)&0xfc);                  

		sccb_write(GC1014_SLAVE_ID, 0xb6, 0x04);
		//DBG_PRINT("a:3.4x,p:%d\r\n",(temp<<2)&0xfc);                                   		       
		//DBG_PRINT("GC1014 analogic gain 3.4x , GC1014 add pregain = %d\r\n",temp); 
	}                                                                       
	else if((ANALOG_GAIN_6<= iReg)&&(iReg < ANALOG_GAIN_7))                 
	{                                                                                                     
		temp = 64*iReg/ANALOG_GAIN_6;                                           
		//sccb_write(GC1014_SLAVE_ID, 0xb1, temp>>6);                                
		sccb_write(GC1014_SLAVE_ID, 0xb2, (temp<<2)&0xfc);                  

		sccb_write(GC1014_SLAVE_ID, 0xb6, 0x05);
		//DBG_PRINT("a:4.7x,p:%d\r\n",(temp<<2)&0xfc);   		       
		//DBG_PRINT("GC1014 analogic gain 4.7x , GC1014 add pregain = %d\r\n",temp); 
	}                                                                       
	else if((ANALOG_GAIN_7<= iReg)&&(iReg < ANALOG_GAIN_8))                 
	{                                                                                                   
		temp = 64*iReg/ANALOG_GAIN_7;                                           
		//sccb_write(GC1014_SLAVE_ID, 0xb1, temp>>6);                                
		sccb_write(GC1014_SLAVE_ID, 0xb2, (temp<<2)&0xfc);                  

		sccb_write(GC1014_SLAVE_ID, 0xb6, 0x06);//     		       
		//DBG_PRINT("a:6.84x,p:%d\r\n",(temp<<2)&0xfc);
		//DBG_PRINT("GC1014 analogic gain 6.84x, GC1014 add pregain = %d\r\n",temp); 
	}                                                                       
	else if((ANALOG_GAIN_8<= iReg)&&(iReg < ANALOG_GAIN_9))                 
	{                                                                                                 
		temp = 64*iReg/ANALOG_GAIN_8;                                           
		//sccb_write(GC1014_SLAVE_ID, 0xb1, temp>>6);                                
		sccb_write(GC1014_SLAVE_ID, 0xb2, (temp<<2)&0xfc);                  

		sccb_write(GC1014_SLAVE_ID, 0xb6, 0x07);       		       
		//DBG_PRINT("a:9.4x,p:%d\r\n",(temp<<2)&0xfc);
		//DBG_PRINT("GC1014 analogic gain 9.4x,GC1014 add pregain = %d\r\n",temp);   
	}                                                                                                                  
	else //if((ANALOG_GAIN_9<= iReg)&&(iReg < ANALOG_GAIN_10))                
	{                                                                                                 
		temp = 64*iReg/ANALOG_GAIN_9;                                           
		//sccb_write(GC1014_SLAVE_ID, 0xb1, temp>>6);                                
		sccb_write(GC1014_SLAVE_ID, 0xb2, (temp<<2)&0xfc);                  

		sccb_write(GC1014_SLAVE_ID, 0xb6, 0x08);       		       
		//DBG_PRINT("GC1014 analogic gain 13.2x ,GC1014 add pregain = %d\r\n",temp); 
	} 
}
#else
static void gc1014_cvt_analog_gain(INT32U analog_gain)
{
    INT32U Analog_Multiple[9]={1000, 1706, 2000, 3414, 4026,6870,8127,13869,19610}; //GC1014-TEST-GAIN

	INT32U Analog_Index;
	INT32U Digital_Gain;
	INT32U Decimal;

	gc1014_analog_gain = analog_gain*10;	 //  1800											


#if 1   //Abner-test-20150727
	    if(gc1014_analog_gain>8800)  gc1014_analog_gain=8800; //limit max gain , added by  simon 20141208 
#endif


	Analog_Index=0;
	
	while(Analog_Index<9)
	{
	  if(gc1014_analog_gain<Analog_Multiple[Analog_Index]) 
	  {
		break;
	  }
	  else
	  {
		Analog_Index++; 
	  }
	}

	
	Digital_Gain = gc1014_analog_gain*1000/Analog_Multiple[Analog_Index-1];
	Decimal=(Digital_Gain*64)/1000;

	sccb_write(GC1014_SLAVE_ID, 0xb1,  Decimal>>6); 
	sccb_write(GC1014_SLAVE_ID, 0xb2,  (Decimal<<2)&0xfc);
	sccb_write(GC1014_SLAVE_ID, 0xb6,   Analog_Index-1);

}
#endif

int gc1014_set_exposure_time(sensor_exposure_t *si)
{
	//int ret=0;
	//unsigned short tmp;
	//int analog_gain;
	//unsigned char cvt_digital_gain;
	//int digital_gain;
	int lsb_time, msb_time;
	int idx;

	// From agoritham calc new data update to gc1014_seInfo.
	gc1014_seInfo.sensor_ev_idx += si->ae_ev_idx;
	if(gc1014_seInfo.sensor_ev_idx >= gc1014_seInfo.max_ev_idx) gc1014_seInfo.sensor_ev_idx = gc1014_seInfo.max_ev_idx;
	if(gc1014_seInfo.sensor_ev_idx < 0) gc1014_seInfo.sensor_ev_idx = 0;
	
	idx = gc1014_seInfo.sensor_ev_idx * 3;
	gc1014_seInfo.time = p_expTime_table[idx];
	gc1014_seInfo.analog_gain = p_expTime_table[idx+1];
	gc1014_seInfo.digital_gain = p_expTime_table[idx+2];
	
	gc1014_seInfo.userISO = si->userISO;

	//DBG_PRINT("T %d, ag %d, ev %d.\r\n", gc1014_seInfo.time, gc1014_seInfo.analog_gain, gc1014_seInfo.sensor_ev_idx );
	//DBG_PRINT("Time = %d, a gain = %d, d gain = %d, ev idx = %d [%d]\r\n", gc1014_seInfo.time, gc1014_seInfo.analog_gain, gc1014_seInfo.digital_gain, gc1014_seInfo.sensor_ev_idx, si->ae_ev_idx );
	//digital_gain = ((si->digital_gain >> 2) & 0xFF);	//0x40:1x, 0xff:4x
	// set exposure time

     if (gc1014_seInfo.time<0x0A) gc1014_seInfo.time=0x0A;//added by gc fae abner
  
	lsb_time = (gc1014_seInfo.time & 0xFF);
	msb_time = ((gc1014_seInfo.time >>8 )& 0xFF);

	sccb_write(GC1014_SLAVE_ID, 0x04 , lsb_time );
	sccb_write(GC1014_SLAVE_ID, 0x03 , msb_time );
	return 0;
}

void gc1014_set_exposure_gain(void)
{
	//int digital_gain_tmp;
	/*digital_gain_tmp =  0x60;((gc1014_seInfo.digital_gain  >> 2) & 0xFF);	//0x40:1x, 0xff:4x
	sccb_write(GC1014_SLAVE_ID, 0xb0, digital_gain_tmp);*/
	gc1014_cvt_analog_gain(gc1014_seInfo.analog_gain);
	//DBG_PRINT("G");
}


void gc1014_get_exposure_time(sensor_exposure_t *se)
{
	//int ret=0;
	gp_memcpy((INT8S *)se, (INT8S *)&gc1014_seInfo, sizeof(sensor_exposure_t));

}

void gc1014_set_exp_freq(int freq)
{
	if(freq == 50)
	{
		if(GC_CLK == GC_48MHZ) {
			gc1014_seInfo.sensor_ev_idx = GC1014_50HZ_INIT_EV_IDX_48MHz;
			gc1014_seInfo.ae_ev_idx = 0;
			gc1014_seInfo.daylight_ev_idx= GC1014_50HZ_DAY_EV_IDX_48MHz;
			gc1014_seInfo.night_ev_idx= GC1014_50HZ_NIGHT_EV_IDX_48MHz;
			gc1014_seInfo.max_ev_idx = GC1014_50HZ_MAX_EXP_IDX_48MHz - 1;
		} else {
			gc1014_seInfo.sensor_ev_idx = GC1014_50HZ_INIT_EV_IDX_42MHz;
			gc1014_seInfo.ae_ev_idx = 0;
			gc1014_seInfo.daylight_ev_idx= GC1014_50HZ_DAY_EV_IDX_42MHz;
			gc1014_seInfo.night_ev_idx= GC1014_50HZ_NIGHT_EV_IDX_42MHz;
			gc1014_seInfo.max_ev_idx = GC1014_50HZ_MAX_EXP_IDX_42MHz - 1;
		}

		#if 0//(!GC1014_GC_AE)
			p_expTime_table = (int *)g_GC1014_exp_time_gain_50Hz;
		#else
			if(GC_CLK == GC_48MHZ) {
				p_expTime_table = (int *)g_GC1014_exp_time_gain_50Hz_48MHz;
			} else {
				p_expTime_table = (int *)g_GC1014_exp_time_gain_50Hz_42MHz;
			}
		#endif
	}
	else if(freq == 60)
	{
		if(GC_CLK == GC_48MHZ) {
			gc1014_seInfo.sensor_ev_idx = GC1014_60HZ_INIT_EV_IDX_48MHz;
			gc1014_seInfo.ae_ev_idx = 0;
			gc1014_seInfo.daylight_ev_idx= GC1014_60HZ_DAY_EV_IDX_48MHz;
			gc1014_seInfo.night_ev_idx= GC1014_60HZ_NIGHT_EV_IDX_48MHz;
			gc1014_seInfo.max_ev_idx = GC1014_60HZ_MAX_EXP_IDX_48MHz - 1;
		} else {
			gc1014_seInfo.sensor_ev_idx = GC1014_60HZ_INIT_EV_IDX_42MHz;
			gc1014_seInfo.ae_ev_idx = 0;
			gc1014_seInfo.daylight_ev_idx= GC1014_60HZ_DAY_EV_IDX_42MHz;
			gc1014_seInfo.night_ev_idx= GC1014_60HZ_NIGHT_EV_IDX_42MHz;
			gc1014_seInfo.max_ev_idx = GC1014_60HZ_MAX_EXP_IDX_42MHz - 1;
		}
		#if 0//(!GC1014_GC_AE)
			p_expTime_table = (int *)g_GC1014_exp_time_gain_60Hz;
		#else
			if(GC_CLK == GC_48MHZ) {
				p_expTime_table = (int *)g_GC1014_exp_time_gain_60Hz_48MHz;
			} else {
				p_expTime_table = (int *)g_GC1014_exp_time_gain_60Hz_42MHz;
			}
		#endif
	}
}

static int gc1014_init(void)
{
	gc1014_seInfo.max_time = GC1014_MAX_EXPOSURE_TIME;
	gc1014_seInfo.min_time = GC1014_MIN_EXPOSURE_TIME;

	gc1014_seInfo.max_digital_gain = GC1014_MAX_DIGITAL_GAIN ;
	gc1014_seInfo.min_digital_gain = GC1014_MIN_DIGITAL_GAIN ;

	gc1014_seInfo.max_analog_gain = GC1014_MAX_ANALOG_GAIN;
	gc1014_seInfo.min_analog_gain = GC1014_MIN_ANALOG_GAIN;

	gc1014_seInfo.analog_gain = gc1014_seInfo.min_analog_gain;
	gc1014_seInfo.digital_gain = gc1014_seInfo.min_digital_gain;
	gc1014_seInfo.time = gc1014_seInfo.max_time;// >> 1;
	gc1014_seInfo.userISO = ISO_AUTO;

	gc1014_set_exp_freq(50);
	
	DBG_PRINT("gc1014_init\r\n");
	return 0;
}

void sensor_gc1014_init(INT32U WIDTH, INT32U HEIGHT)
{
	//i2c_bus_handle_t i2c_handle; 
	INT32U i;
	INT8U reg_tmp, data_tmp;


	gc1014_init();
	gc1014_sensor_calibration_str();
	
	if(sensor_format == GC1014_RAW){
		if(WIDTH == 1280 && HEIGHT == 720)
		{
			if(GC_CLK == GC_48MHZ) {
				for (i=0; i<sizeof(GC1014_Para_720P_30)/2; i++) 
				{
					sccb_write(GC1014_SLAVE_ID, GC1014_Para_720P_30[i][0], GC1014_Para_720P_30[i][1]);
				}
			} else {
				for (i=0; i<sizeof(GC1014_Para_720P_42m_28)/2; i++) 
				{
					sccb_write(GC1014_SLAVE_ID, GC1014_Para_720P_42m_28[i][0], GC1014_Para_720P_42m_28[i][1]);
				}
			}
#if C_DISPLAY_REVERSE
			if (gc1014_flip_mode==1) {
				sccb_write(GC1014_SLAVE_ID, 0x17, 0x17);	// [0]mirror [1]flip
			}
			else {
				sccb_write(GC1014_SLAVE_ID, 0x17, 0x14);	// [0]mirror [1]flip
			}
#endif
		}			
		else 
		{
			while(1);
		}
	}else if(sensor_format == GC1014_MIPI){
		if(WIDTH == 1280 && HEIGHT == 800)
		{
			for (i=0; i<sizeof(GC1014_MIPI_1280_800_30)/2; i++) 
			{
				sccb_write(GC1014_SLAVE_ID,GC1014_MIPI_1280_800_30[i][0], GC1014_MIPI_1280_800_30[i][1]);
			}
		}	
		else if	(WIDTH == 1280 && HEIGHT == 720)
		{
		
		if(GC_CLK == GC_48MHZ)
		  {			
			for (i=0; i<sizeof(GC1014_MIPI_720P)/2; i++) 
			{
				reg_tmp = GC1014_MIPI_720P[i][0];
				data_tmp =  GC1014_MIPI_720P[i][1];
				
				sccb_write(GC1014_SLAVE_ID,GC1014_MIPI_720P[i][0], GC1014_MIPI_720P[i][1]);
#if 1
				if (reg_tmp == 0x10 && data_tmp == 0x80)
				{					
					//gpio_write_io(IO_A14, 1);	//pull Low					
					//sccb_delay(200);//120us
					drv_msec_wait(200);//90); //wait 1 is 1ms for CLKO stable
					//gpio_write_io(IO_A14, 0);	//pull high
				}
#endif				
			}
		
		 }else	//42mhz
			{
				DBG_PRINT("GC1014 PLCK=42MHZ\r\n");
				for (i=0; i<sizeof(GC1014_MIPI_720P_42mhz)/2; i++) 
				{
					reg_tmp = GC1014_MIPI_720P_42mhz[i][0];
					data_tmp =  GC1014_MIPI_720P_42mhz[i][1];
					
					//DBG_PRINT("0x%02x, 0x%02x\r\n", GC1014_MIPI_720P_42mhz[i][0],  GC1014_MIPI_720P_42mhz[i][1]);
					sccb_write(GC1014_SLAVE_ID,GC1014_MIPI_720P_42mhz[i][0], GC1014_MIPI_720P_42mhz[i][1]);
					
#if 1
					if (reg_tmp == 0x10 && data_tmp == 0x80)
					{
						//gpio_write_io(IO_A14, 1);	//pull Low					
						drv_msec_wait(200);//90); //wait 1 is 1ms for CLKO stable
						//gpio_write_io(IO_A14, 0);	//pull high
					}				
#endif				
				}
			}
#if C_DISPLAY_REVERSE
			if (gc1014_flip_mode==1) {
				sccb_write(GC1014_SLAVE_ID, 0x17, 0x17);	// [0]mirror [1]flip
				sccb_write(GC1014_SLAVE_ID, 0x92, 0x10);
			}
			else {
				sccb_write(GC1014_SLAVE_ID, 0x17, 0x14);	// [0]mirror [1]flip
				sccb_write(GC1014_SLAVE_ID, 0x92, 0x01);
			}
#endif					
			
		}	
		else if(WIDTH == 640 && HEIGHT == 480)
		{
			for (i=0; i<sizeof(GC1014_MIPI_VGA_f60)/2; i++) 
			{
				sccb_write(GC1014_SLAVE_ID,GC1014_MIPI_VGA_f60[i][0], GC1014_MIPI_VGA_f60[i][1]);
			}			
		}		
		else 
		{
			while(1);
		}
	}
}


INT32S gc1014_switch_pclk(INT8U flag)
{
	if(flag) {
		GC_CLK = GC_42MHZ;
	} else {
		GC_CLK = GC_42MHZ;//42
	}
	return 0;
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(USE_SENSOR_NAME == SENSOR_GC1014)     //
//================================================================//

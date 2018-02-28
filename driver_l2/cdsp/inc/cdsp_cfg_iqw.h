#ifndef _CDSP_CONFIG_H_
#define _CDSP_CONFIG_H_

#include "drv_l1_front.h"
#include "project.h"
#include "drv_l1_cdsp.h"
#include "drv_l2_cdsp.h"
#include "gp_aeawb.h"

//gpCdspSatHue_t daylight_sat_hue, night_sat_hue;
/**************************************************************************
 *                          USER CONFIG				                            *
 **************************************************************************/

 /**************************************************************************
 **************************************************************************
 *							SOI H22											
 **************************************************************************							*
 **************************************************************************/
#if (USE_SENSOR_NAME == SENSOR_SOI_H22)
	#define	TARGET_Y  				0x42	//Luminace 
	#define TARGET_Y_LOW			(TARGET_Y >> 2)	//TARGET_Y_LOW = TARGET_Y/4   
	#define MAX_ANALOG_GAIN			2.5		//3 ~ 5
	
/**************************************************************************
*	JXH22 Daylight Mode																														*
 **************************************************************************/	
	//=========================================================================
	// Purpose :  set the color, Hue is one of the main properties of a color
	// Parameters : -128 ~ +127
	// DAYLIGHT_HUE_U_OFFSET:	+: more blue,  	-: more yellow/green	
	// DAYLIGHT_HUE_V_OFFSET:	+: more red,  	-: more blue/green
	//=========================================================================
	#define	DAYLIGHT_HUE_U_OFFSET  	0		//-128 ~ +127,   +: more blue,  -: more yellow/green	
	#define	DAYLIGHT_HUE_V_OFFSET  	0 		//-128 ~ +127,   +: more red,  -: more blue/green
		
	//=========================================================================
	// Purpose :  set the contrast, Y_offset relationship with Y_scale
	// Parameters : Default 0x20
	// Ex: 	Y_SCALE = 0x21, Y_OFFSET range = 0~-7, 
	//		Y_SCALE = 0x22, Y_OFFSET range = 0~-15,  
	//========================================================================= 
	#define	DAYLIGHT_HUE_Y_OFFSET  	-4		//-128 ~ +127	
	#define	DAYLIGHT_SAT_Y_SCALE  	0x26	//Default:0x20
	
	//=========================================================================
	// Purpose :  set the sturatin for single color
	// Parameters : Default 0x20
	// DAYLIGHT_SAT_U_SCALE: Blue, upper 0x20: deep blue, less 0x20: light blue
	// DAYLIGHT_SAT_V_SCALE: Red,  upper 0x20: deep red,  less 0x20: light red
	//========================================================================= 
	#define	DAYLIGHT_SAT_U_SCALE  	0x26	//Default:0x20 // blud
	#define	DAYLIGHT_SAT_V_SCALE  	0x26	//Default:0x20 // red
	
	//=========================================================================
	// Purpose :  set the sharpness
	// Parameters : 0:soft, 1:smooth, 2:normal, 3:strong
	//========================================================================= 
	#define DAYLIGHT_EDGE			3 		//0:soft, 1:smooth, 2:normal, 3:strong
	
	//=========================================================================
	// Purpose :  set the white balance offset
	// Parameters : +: warm,   -: cold , +10~-10
	//=========================================================================
	#define DAYLIGHT_WB_OFFSET		0
	#define AUTO_WB_OFFSET				0
	
/**************************************************************************
*	JXH22 Night Mode																												*
**************************************************************************/	
	//=========================================================================
	// Purpose :  set the color, Hue is one of the main properties of a color
	// Parameters : -128 ~ +127
	// DAYLIGHT_HUE_U_OFFSET:	+: more blue,  	-: more yellow/green	
	// DAYLIGHT_HUE_V_OFFSET:	+: more red,  	-: more blue/green
	//=========================================================================
	#define	NIGHT_HUE_U_OFFSET  	0		//-128 ~ +127,   +: more blue,  -: more yellow/green	
	#define	NIGHT_HUE_V_OFFSET  	0 		//-128 ~ +127,   +: more red,  -: more blue/green
	
	//=========================================================================
	// Purpose :  set the contrast, Y_offset relationship with Y_scale
	// Parameters : Default 0x20
	// Ex: 	Y_SCALE = 0x21, Y_OFFSET range = 0~-7, 
	//		Y_SCALE = 0x22, Y_OFFSET range = 0~-15,  
	//=========================================================================	
	#define	NIGHT_HUE_Y_OFFSET  	4		//-128 ~ +127	
	#define	NIGHT_SAT_Y_SCALE  		0x20	//Default:0x20
	
	//=========================================================================
	// Purpose :  set the sturatin
	// Parameters : Default 0x20
	// DAYLIGHT_SAT_U_SCALE: Blue, upper 0x20: deep blue, less 0x20: light blue
	// DAYLIGHT_SAT_V_SCALE: Red,  upper 0x20: deep red,  less 0x20: light red
	//========================================================================= 
	#define	NIGHT_SAT_U_SCALE  		0x18	//Default:0x20 // blud
	#define	NIGHT_SAT_V_SCALE  		0x18	//Default:0x20 // red

	//=========================================================================
	// Purpose :  set the sharpness
	// Parameters : 0:soft, 1:smooth, 2:normal, 3:strong
	//=========================================================================
	#define NIGHT_EDGE				3	

	//=========================================================================
	// Purpose :  set the white balance offset
	// Parameters : +: warm,   -: cold , +10~-10
	//=========================================================================
	#define NIGHT_WB_OFFSET 		0	

 /**************************************************************************
 **************************************************************************
 *							GC1004											
 ***************************************************************************
 **************************************************************************/	
#elif (USE_SENSOR_NAME == SENSOR_GC1004)
	#define	TARGET_Y  				0x38	//Luminace
	#define TARGET_Y_LOW			(TARGET_Y/4)	//TARGET_Y_LOW = TARGET_Y/4  
	#define MAX_ANALOG_GAIN			4		//3 ~ 5
		
/**************************************************************************
*	GC1004 Daylight Mode																														*
**************************************************************************/	
	//=========================================================================
	// Purpose :  set the color, Hue is one of the main properties of a color
	// Parameters : -128 ~ +127
	// DAYLIGHT_HUE_U_OFFSET:	+: more blue,  	-: more yellow/green	
	// DAYLIGHT_HUE_V_OFFSET:	+: more red,  	-: more blue/green
	//=========================================================================
	#define	DAYLIGHT_HUE_U_OFFSET  	0		//-128 ~ +127,   +: more blue,  -: more yellow/green	
	#define	DAYLIGHT_HUE_V_OFFSET  	1 		//-128 ~ +127,   +: more red,  -: more blue/green
		
	//=========================================================================
	// Purpose :  set the contrast, Y_offset relationship with Y_scale
	// Parameters : Default 0x20
	// Ex: 	Y_SCALE = 0x21, Y_OFFSET range = 0~-7, 
	//		Y_SCALE = 0x22, Y_OFFSET range = 0~-15,  
	//=========================================================================
	#define	DAYLIGHT_HUE_Y_OFFSET  	-4		//-128 ~ +127	
	#define	DAYLIGHT_SAT_Y_SCALE  	0x22	//Default:0x20
	
	//=========================================================================
	// Purpose :  set the sturatin
	// Parameters : Default 0x20
	// DAYLIGHT_SAT_U_SCALE: Blue, upper 0x20: deep blue, less 0x20: light blue
	// DAYLIGHT_SAT_V_SCALE: Red,  upper 0x20: deep red,  less 0x20: light red
	//========================================================================= 
	#define	DAYLIGHT_SAT_U_SCALE  	0x26	//Default:0x20 	// blud
	#define	DAYLIGHT_SAT_V_SCALE  	0x26	//Default:0x20	// red
	
	//=========================================================================
	// Purpose :  set the sharpness
	// Parameters : 0:soft, 1:smooth, 2:normal, 3:strong
	//========================================================================= 
	#define DAYLIGHT_EDGE			2
	
	//=========================================================================
	// Purpose :  set the white balance offset
	// Parameters : +: warm,   -: cold , +10~-10
	//=========================================================================
	#define DAYLIGHT_WB_OFFSET		0
	#define AUTO_WB_OFFSET			0
	 
/**************************************************************************
*****	GC1004 Night Mode			
***  Contarst: NIGHT_SAT_Y_SCALE +	NIGHT_HUE_Y_OFFSET, Strong: 0x22 + (-15 ~ 0)
***											*
**************************************************************************/	
	//=========================================================================
	// Purpose :  set the color, Hue is one of the main properties of a color
	// Parameters : -128 ~ +127
	// DAYLIGHT_HUE_U_OFFSET:	+: more blue,  	-: more yellow/green	
	// DAYLIGHT_HUE_V_OFFSET:	+: more red,  	-: more blue/green
	//=========================================================================                 	    	
	#define	NIGHT_HUE_U_OFFSET  	0		//-128 ~ +127,   +: more blue,  -: more yellow/green	
	#define	NIGHT_HUE_V_OFFSET  	-4		//-128 ~ +127,   +: more red,  -: more blue/green
		
	//=========================================================================
	// Purpose :  set the contrast, Y_offset relationship with Y_scale
	// Parameters : Default 0x20
	// Ex: 	Y_SCALE = 0x21, Y_OFFSET range = 0~-7, 
	//		Y_SCALE = 0x22, Y_OFFSET range = 0~-15,  
	//=========================================================================
	#define	NIGHT_HUE_Y_OFFSET  	-8		//-128 ~ +127	
	#define	NIGHT_SAT_Y_SCALE  		0x22	//Default:0x20
	
	//=========================================================================
	// Purpose :  set the sturatin
	// Parameters : Default 0x20
	// DAYLIGHT_SAT_U_SCALE: Blue, upper 0x20: deep blue, less 0x20: light blue
	// DAYLIGHT_SAT_V_SCALE: Red,  upper 0x20: deep red,  less 0x20: light red
	//=========================================================================            		
	#define	NIGHT_SAT_U_SCALE  		0x1B	//Default:0x20	// blud
	#define	NIGHT_SAT_V_SCALE  		0x1B	//Default:0x20	// red
	
	//=========================================================================
	// Purpose :  set the sharpness
	// Parameters : 0:soft, 1:smooth, 2:normal, 3:strong
	//========================================================================= 
	#define NIGHT_EDGE				2	
	
	//=========================================================================
	// Purpose :  set the white balance offset
	// Parameters : +: warm,   -: cold , +10~-10
	//=========================================================================
	#define NIGHT_WB_OFFSET			0
	
 /**************************************************************************
 **************************************************************************
 *							OV9712											
 **************************************************************************							*
 **************************************************************************/	
#elif (USE_SENSOR_NAME == SENSOR_OV9712)
	#define	TARGET_Y  				0x38	//Luminace
	#define TARGET_Y_LOW			(TARGET_Y/4)	//TARGET_Y_LOW = TARGET_Y/4  
	#define MAX_ANALOG_GAIN			4		//3 ~ 5
	
/**************************************************************************
*	OV9712 Daylight Mode																														*
**************************************************************************/	
	//=========================================================================
	// Purpose :  set the color, Hue is one of the main properties of a color
	// Parameters : -128 ~ +127
	// DAYLIGHT_HUE_U_OFFSET:	+: more blue,  	-: more yellow/green	
	// DAYLIGHT_HUE_V_OFFSET:	+: more red,  	-: more blue/green
	//=========================================================================
	#define	DAYLIGHT_HUE_U_OFFSET  	0		//-128 ~ +127,   +: more blue,  -: more yellow/green	
	#define	DAYLIGHT_HUE_V_OFFSET  	0 		//-128 ~ +127,   +: more red,  -: more blue/green
		
	//=========================================================================
	// Purpose :  set the contrast, Y_offset relationship with Y_scale
	// Parameters : Default 0x20
	// Ex: 	Y_SCALE = 0x21, Y_OFFSET range = 0~-7, 
	//		Y_SCALE = 0x22, Y_OFFSET range = 0~-15,  
	//=========================================================================
	#define	DAYLIGHT_HUE_Y_OFFSET  	0		//-128 ~ +127	
	#define	DAYLIGHT_SAT_Y_SCALE	0x20	//Default:0x20
	
	//=========================================================================
	// Purpose :  set the sturatin
	// Parameters : Default 0x20
	// DAYLIGHT_SAT_U_SCALE: Blue, upper 0x20: deep blue, less 0x20: light blue
	// DAYLIGHT_SAT_V_SCALE: Red,  upper 0x20: deep red,  less 0x20: light red
	//========================================================================= 
	#define	DAYLIGHT_SAT_U_SCALE	0x20	//Default:0x20 	// blud
	#define	DAYLIGHT_SAT_V_SCALE	0x20	//Default:0x20	// red
	
	//=========================================================================
	// Purpose :  set the sharpness
	// Parameters : 0:soft, 1:smooth, 2:normal, 3:strong
	//========================================================================= 
	#define DAYLIGHT_EDGE			2
	
	//=========================================================================
	// Purpose :  set the white balance offset
	// Parameters : +: warm,   -: cold , +10~-10
	//=========================================================================
	#define DAYLIGHT_WB_OFFSET		0
	#define AUTO_WB_OFFSET			0
	 
/**************************************************************************
*****	OV9712 Night Mode			
***  Contarst: NIGHT_SAT_Y_SCALE +	NIGHT_HUE_Y_OFFSET, Strong: 0x22 + (-15 ~ 0)
***											*
**************************************************************************/		
	//=========================================================================
	// Purpose :  set the color, Hue is one of the main properties of a color
	// Parameters : -128 ~ +127
	// DAYLIGHT_HUE_U_OFFSET:	+: more blue,  	-: more yellow/green	
	// DAYLIGHT_HUE_V_OFFSET:	+: more red,  	-: more blue/green
	//=========================================================================                 	
	#define	NIGHT_HUE_U_OFFSET  	0		//-128 ~ +127,   +: more blue,  -: more yellow/green	
	#define	NIGHT_HUE_V_OFFSET		0		//-128 ~ +127,   +: more red,  -: more blue/green
		                        	
	//=========================================================================
	// Purpose :  set the contrast, Y_offset relationship with Y_scale
	// Parameters : Default 0x20
	// Ex: 	Y_SCALE = 0x21, Y_OFFSET range = 0~-7, 
	//		Y_SCALE = 0x22, Y_OFFSET range = 0~-15,  
	//=========================================================================     
	#define	NIGHT_HUE_Y_OFFSET		0		//-128 ~ +127        	
	#define	NIGHT_SAT_Y_SCALE  		0x20	//Default:0x20
	
	//=========================================================================
	// Purpose :  set the sturatin
	// Parameters : Default 0x20
	// DAYLIGHT_SAT_U_SCALE: Blue, upper 0x20: deep blue, less 0x20: light blue
	// DAYLIGHT_SAT_V_SCALE: Red,  upper 0x20: deep red,  less 0x20: light red
	//=========================================================================             	
	#define	NIGHT_SAT_U_SCALE  		0x20	//Default:0x20	// blud
	#define	NIGHT_SAT_V_SCALE  		0x20	//Default:0x20	// red
	
	//=========================================================================
	// Purpose :  set the sharpness
	// Parameters : 0:soft, 1:smooth, 2:normal, 3:strong
	//=========================================================================                             	
	#define NIGHT_EDGE				2	
	
	//=========================================================================
	// Purpose :  set the white balance offset
	// Parameters : +: warm,   -: cold , +10~-10
	//=========================================================================                            	
	#define NIGHT_WB_OFFSET			0
	
 /**************************************************************************
 **************************************************************************
 *							Other Sensor											
 **************************************************************************							*
 **************************************************************************/			
#else 
	#define	TARGET_Y  				40		//Luminace
	#define TARGET_Y_LOW			(TARGET_Y/4)	//TARGET_Y_LOW = TARGET_Y/4  
	#define MAX_ANALOG_GAIN			4		//3 ~ 5  	
	
/**************************************************************************
*	Other Sensor Daylight Mode																														*
**************************************************************************/	
	//=========================================================================
	// Purpose :  set the color, Hue is one of the main properties of a color
	// Parameters : -128 ~ +127
	// DAYLIGHT_HUE_U_OFFSET:	+: more blue,  	-: more yellow/green	
	// DAYLIGHT_HUE_V_OFFSET:	+: more red,  	-: more blue/green
	//=========================================================================                    	    	
	#define	DAYLIGHT_HUE_U_OFFSET  	0		//-128 ~ +127,   +: more blue,  -: more yellow/green	
	#define	DAYLIGHT_HUE_V_OFFSET  	0 		//-128 ~ +127,   +: more red,  -: more blue/green
		
	//=========================================================================
	// Purpose :  set the contrast, Y_offset relationship with Y_scale
	// Parameters : Default 0x20
	// Ex: 	Y_SCALE = 0x21, Y_OFFSET range = 0~-7, 
	//		Y_SCALE = 0x22, Y_OFFSET range = 0~-15,  
	//=========================================================================
	#define	DAYLIGHT_HUE_Y_OFFSET  	0		//-128 ~ +127	
	#define	DAYLIGHT_SAT_Y_SCALE  	0x20	//Default:0x20
	
	//=========================================================================
	// Purpose :  set the sturatin
	// Parameters : Default 0x20
	// DAYLIGHT_SAT_U_SCALE: Blue, upper 0x20: deep blue, less 0x20: light blue
	// DAYLIGHT_SAT_V_SCALE: Red,  upper 0x20: deep red,  less 0x20: light red
	//=========================================================================               	
	#define	DAYLIGHT_SAT_U_SCALE  	0x20	//Default:0x20 	// blud
	#define	DAYLIGHT_SAT_V_SCALE  	0x20	//Default:0x20	// red
	
	//=========================================================================
	// Purpose :  set the sharpness
	// Parameters : 0:soft, 1:smooth, 2:normal, 3:strong
	//========================================================================= 
	#define DAYLIGHT_EDGE			2
	
	//=========================================================================
	// Purpose :  set the white balance offset
	// Parameters : +: warm,   -: cold , +10~-10
	//=========================================================================
	#define DAYLIGHT_WB_OFFSET		0
	#define AUTO_WB_OFFSET			0
	 
/**************************************************************************
*****	Other Sensor Night Mode			
***  Contarst: NIGHT_SAT_Y_SCALE +	NIGHT_HUE_Y_OFFSET, Strong: 0x22 + (-15 ~ 0)
***											*
**************************************************************************/		
	//=========================================================================
	// Purpose :  set the color, Hue is one of the main properties of a color
	// Parameters : -128 ~ +127
	// DAYLIGHT_HUE_U_OFFSET:	+: more blue,  	-: more yellow/green	
	// DAYLIGHT_HUE_V_OFFSET:	+: more red,  	-: more blue/green
	//=========================================================================                	    	
	#define	NIGHT_HUE_U_OFFSET  	0		//-128 ~ +127,   +: more blue,  -: more yellow/green	
	#define	NIGHT_HUE_V_OFFSET  	0		//-128 ~ +127,   +: more red,  -: more blue/green
		
	//=========================================================================
	// Purpose :  set the contrast, Y_offset relationship with Y_scale
	// Parameters : Default 0x20
	// Ex: 	Y_SCALE = 0x21, Y_OFFSET range = 0~-7, 
	//		Y_SCALE = 0x22, Y_OFFSET range = 0~-15,  
	//=========================================================================
	#define	NIGHT_HUE_Y_OFFSET  	0		//-128 ~ +127
	#define	NIGHT_SAT_Y_SCALE  		0x20	//Default:0x20
	
	//=========================================================================
	// Purpose :  set the sturatin
	// Parameters : Default 0x20
	// DAYLIGHT_SAT_U_SCALE: Blue, upper 0x20: deep blue, less 0x20: light blue
	// DAYLIGHT_SAT_V_SCALE: Red,  upper 0x20: deep red,  less 0x20: light red
	//=========================================================================            		
	#define	NIGHT_SAT_U_SCALE  		0x20	//Default:0x20	// blud
	#define	NIGHT_SAT_V_SCALE  		0x20	//Default:0x20	// red
	
	//=========================================================================
	// Purpose :  set the sharpness
	// Parameters : 0:soft, 1:smooth, 2:normal, 3:strong
	//========================================================================= 
	#define NIGHT_EDGE				2	
	
	//=========================================================================
	// Purpose :  set the white balance offset
	// Parameters : +: warm,   -: cold , +10~-10
	//=========================================================================
	#define NIGHT_WB_OFFSET			0
	
	 /**************************************************************************/		
#endif

#endif	/*_CDSP_CONFIG_H_*/

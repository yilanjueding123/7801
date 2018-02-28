#include "drv_l2_sensor.h"
#include "drv_l2_cdsp.h"
#include "drv_l1_sfr.h"
#include "driver_l1.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L2_SENSOR) && (_DRV_L2_SENSOR == 1)           	  //
//================================================================//

#include "drv_l1_cdsp.h"

/****************************************************************************/
#if (USE_SENSOR_NAME == SENSOR_OV7670)
#include "sensor_ov7670.h"
#elif (USE_SENSOR_NAME == SENSOR_OV9712)
//#include "sensor_ov9712.h"
#elif (USE_SENSOR_NAME == SENSOR_GC1004)
//#include "sensor_gc1004.h"
//#define GC1004_SLAVE_ID						0x78
#endif

/****************************************************************************/
extern INT32U frame_width_after_clip;
extern volatile INT8U cdsp_eof_occur_flag;

/****************************************************************************/

sensor_apis_ops Sensor_apis_ops; 
INT32U sensor_frame_rate;

#define SENSOR_STATUS_INIT		0x00000001
#define SENSOR_STATUS_START		0x00000002

typedef struct
{
	INT32U frameAddrs;
	INT32U frameWidth;
	INT32U frameHeight;
	INT32U motionAddrs;
	INT32U sensorStatus;	
}sensor_args_t;

sensor_args_t gSensorArgs = {0};
/****************************************************************************/
/*
 *	sensor_init
 */
void sensor_init(void)
{
	sw_i2c_lock();		//added by wwj to protect I2C timing

	#if (USE_SENSOR_NAME == SENSOR_OV7670)
		//sensor_ov7670_init();
	#elif (USE_SENSOR_NAME == SENSOR_OV9712)
		//sensor_ov9712_init();
	#endif

	sw_i2c_unlock();	//added by wwj to protect I2C timing

}

/****************************************************************************/
/*
 *	sensor_start:  motionAddrs: 0: disable motion detect non-zero is enable motion detect
 */
extern void gp_mipi_isp_start(INT32U dummy_addr, INT32U gpSENSOR_WIDTH, INT32U gpSENSOR_HEIGHT);
void sensor_start(INT32U frameAddrs,INT32U motionAddrs)
{
	//motionAddrs = 0x600000;	//for test

	sensor_frame_rate = 20; // defatue sensor frame rate

	gSensorArgs.frameAddrs = frameAddrs;
	gSensorArgs.motionAddrs = motionAddrs;

	sw_i2c_lock();		//added by wwj to protect I2C timing

	hwCdsp_EnableCrop(0);
	hwCdsp_EnableClamp(0,frameAddrs);

	#if ( (sensor_format != SOI_H22_MIPI) | (sensor_format != GC1004_MIPI))
		gp_isp_start(frameAddrs, SENSOR_WIDTH, SENSOR_HEIGHT);
	#else
		 gp_mipi_isp_start(frameAddrs, SENSOR_WIDTH, SENSOR_HEIGHT);
	#endif	

	if (motionAddrs)		// motion detction working buffer address(3600 Bytes)
	{
		// Enable motion detection
		hwCdsp_MD_set(0, CDSP_MD_THR, SENSOR_WIDTH, motionAddrs);
		hwCdsp_md_enable(1);
	}
	else
	{
		// Disable motion detection
		hwCdsp_MD_set(0, CDSP_MD_THR, SENSOR_WIDTH, 0xF8500000);
	}


	sw_i2c_unlock();	//added by wwj to protect I2C timing

	gp_cdsp_drop_frame(6);
	//hwCdsp_SetYuvBuffA(SENSOR_WIDTH,SENSOR_HEIGHT,dummy_addrs);
	//hwCdsp_Scal_Source(CDSP_PATH);		//Input scal from csi switch to cdsp
}

/****************************************************************************/
/*
 *	sensor_stop
 */
void sensor_stop(void)
{
  	INT32U temp;

  	// close sensor
	R_TGR_IRQ_EN &= ~0x1;


	R_CSI_TG_CTRL0 &= ~0x0001;	//disable sensor controller
	R_CSI_TG_CTRL1 &= ~0x0080;	//disable sensor clock out

  	temp = R_TGR_IRQ_STATUS;
  	R_TGR_IRQ_STATUS = temp;
  	
  	//cdsp_ae_awb_stop();
}

/****************************************************************************/
/*
 *	sensor_frame_end_ready
 */
void sensor_frame_end_ready(void)
{
       INT32U time_begin, t;
	   
	#if 0
	while(1){
		//if(R_CDSP_INT & R_CDSP_INTEN & CDSP_EOF) {
		if((R_CDSP_INT & CDSP_EOF)!=0) {
			R_CDSP_INT |= CDSP_EOF;
			
			break;	
		}
	}
	#else
	cdsp_eof_occur_flag = 0;
	time_begin = OSTimeGet();
	while(cdsp_eof_occur_flag == 0) {
		t = OSTimeGet();
		if((t - time_begin) > 50) { //500ms
		       DBG_PRINT("frame end wait error!\r\n");
			break;
		}
	}
	#endif

	cdsp_eof_occur_flag = 0;
}

/****************************************************************************/
/*
 *	sensor_frame_range_clip:
 */
void sensor_frame_range_clip(sensor_frame_range_t* frameRange)
{
	INT32U Wfactor, Hfactor;
	
	#if (sensor_format == GC1004_MIPI) 
	{	
		if (SENSOR_FLIP) {
			//sccb_write(GC1004_SLAVE_ID, 0x17, 0x17);	// [0]mirror [1]flip
			//sccb_write(GC1004_SLAVE_ID, 0x92, 0x10);
			R_CDSP_MIPI_HVOFFSET = 0xB | (INT32U)(0) << 12;								
		}
		else {
			//sccb_write(GC1004_SLAVE_ID, 0x17, 0x14);	// [0]mirror [1]flip
			//sccb_write(GC1004_SLAVE_ID, 0x92, 0x01);
			R_CDSP_MIPI_HVOFFSET = 0xC | (INT32U)(0) << 12;
		}
	}
	#endif

	frame_width_after_clip = frameRange->clip_w;
	// Crop
	if(frameRange->docropflag)
	{
		hwCdsp_SetCrop(frameRange->point_x, frameRange->point_y, frameRange->clip_w, frameRange->clip_h);
		hwCdsp_EnableCrop(frameRange->docropflag); 	//1:Enable, 0:Disable
	}
	else
	{
		hwCdsp_EnableCrop(frameRange->docropflag); 	//1:Enable, 0:Disable
	}

	hwCdsp_SetExtLine(ENABLE, frameRange->clip_w, 0x28);

	// Scale Down 
	if(frameRange->doscaledownflag)
	{		

		Wfactor = ((frameRange->scaledown_w+1)<<16)/frameRange->frame_w + 1;
		Hfactor = ((frameRange->scaledown_h+1)<<16)/frameRange->frame_h + 1;
		hwCdsp_SetYuvHScale(Wfactor, Wfactor);
		hwCdsp_SetYuvVScale(Hfactor, Hfactor);
		hwCdsp_EnableYuvHScale(frameRange->doscaledownflag, 1);
		hwCdsp_EnableYuvVScale(frameRange->doscaledownflag, 1);
	}
	else
	{
		hwCdsp_EnableYuvHScale(frameRange->doscaledownflag, 1);
		hwCdsp_EnableYuvVScale(frameRange->doscaledownflag, 1);
	}

	hwCdsp_SetSRAM(ENABLE, 0xA0);
	hwCdsp_EnableClamp(1,frameRange->scaledown_w);

	hwCdsp_SetYuvBuffA(frameRange->scaledown_w, frameRange->scaledown_h, gSensorArgs.frameAddrs);
	hwCdsp_SetYuvBuffB(frameRange->scaledown_w, frameRange->scaledown_h, gSensorArgs.frameAddrs);	
	
	hwCdsp_RedoTriger(1);
}

/****************************************************************************/
/*
 *	sensor_frame_range_clip:
 */
void sensor_set_fps(INT32U fpsValue)
{
	sensor_frame_rate = fpsValue;

	if(fpsValue == 25)
	{
		#if ((USE_SENSOR_NAME == SENSOR_SOI_H22) && (sensor_format == SOI_H22_RAW))
		sccb_write(0x60, 0x22, 0x00);
		sccb_write(0x60, 0x23, 0x04);
		#elif (USE_SENSOR_NAME == SENSOR_GC1004)
		sccb_write(0x78, 0xFE, 0x00);
		sccb_write(0x78, 0x08, 0x9B);
		#elif ((USE_SENSOR_NAME == SENSOR_OV9712) && (sensor_format == OV9712_RAW))
		sccb_write(0x60, 0x3d, 0x08);
		sccb_write(0x60, 0x3e, 0x04);
		//hwFront_SetFrameSize(0x196, 0x135, 1280, 720);
		#endif
	}
	else if (fpsValue == 22)
	{
		#if ((USE_SENSOR_NAME == SENSOR_SOI_H22) && (sensor_format == SOI_H22_RAW))
		sccb_write(0x60, 0x22, 0x00);
		sccb_write(0x60, 0x23, 0x05);
		#elif (USE_SENSOR_NAME == SENSOR_GC1004)
		sccb_write(0x78, 0xFE, 0x00);
		sccb_write(0x78, 0x08, 0xff);
		#elif ((USE_SENSOR_NAME == SENSOR_OV9712) && (sensor_format == OV9712_RAW))
		sccb_write(0x60, 0x3d, 0x08);
		sccb_write(0x60, 0x3e, 0x05);
		//hwFront_SetFrameSize(0x196, 0x235, 1280, 720);
		#endif
	}
	else if (fpsValue == 20)
	{
		//0x0702, 0x08b0: Ö¡ÂÊ: 15FPS
		//0x0702, 0x0891: Ö¡ÂÊ: 20FPS
		#if (USE_SENSOR_NAME == SENSOR_GC1004)
		//sccb_write(0x78, 0xFE, 0x00);
		sccb_write(0x78, 0x07, 0x01);
		sccb_write(0x78, 0x08, 0x91);
		//hwFront_SetFrameSize(0x196, 0x235, 1280, 720);
		#endif
	}
	else
	{
		#if ((USE_SENSOR_NAME == SENSOR_SOI_H22) && (sensor_format == SOI_H22_RAW))
		sccb_write(0x60, 0x22, 0x56);
		sccb_write(0x60, 0x23, 0x03);
		#elif (USE_SENSOR_NAME == SENSOR_GC1004)
		sccb_write(0x78, 0xFE, 0x00);
		sccb_write(0x78, 0x08, 0x08);
		#elif ((USE_SENSOR_NAME == SENSOR_OV9712) && (sensor_format == OV9712_RAW))
		sccb_write(0x60, 0x3d, 0x3c);
		sccb_write(0x60, 0x3e, 0x03);		
		//hwFront_SetFrameSize(0x196, 0x6B, 1280, 720);
		#endif
	}
}

/****************************************************************************/
/*
 *	sensor_get_fps:
 */
INT32U sensor_get_fps(void)
{
	return sensor_frame_rate;
}
/****************************************************************************/
/*
 *	sensor_attach: Get sensor apis
 */
sensor_apis_ops* sensor_attach(void)
{
	Sensor_apis_ops.init = sensor_init;
	Sensor_apis_ops.start = sensor_start;
	Sensor_apis_ops.stop = sensor_stop;
	Sensor_apis_ops.wait4FrameEnd = sensor_frame_end_ready;
	Sensor_apis_ops.frameRangeClip = sensor_frame_range_clip;
	Sensor_apis_ops.set_fps = sensor_set_fps;
	Sensor_apis_ops.get_fps = sensor_get_fps;
	
	return ((sensor_apis_ops*)&Sensor_apis_ops);
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L2_SENSOR) && (_DRV_L2_SENSOR == 1)      //
//================================================================//


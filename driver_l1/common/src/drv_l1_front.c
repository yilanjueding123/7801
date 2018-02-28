#include "drv_l1_cdsp.h"
#include "drv_l1_front.h"

/**************
front control
***************/
void 
hwFront_Reset(
	void
)
{
	R_CDSP_FRONT_GCLK |= 0x100;
	R_CDSP_FRONT_GCLK &= ~0x100;
}

void hwFront_SetInputFormat(
	INT8U format 
)
{
	R_CDSP_DATA_FORMAT |= 0x320;
	
	//DBG_PRINT("<LutGamma>gatlutvld:%d, \r\n",(R_CDSP_DATA_FORMAT>>8));
	//DBG_PRINT("<BadPixelOB>gatbadvld:%d, \r\n",(R_CDSP_DATA_FORMAT>>9));
	
	R_CDSP_YUV_CTRL &= ~0x80;
	
	switch(format)
	{
		// Sensor raw
		case SENSOR_RAW8:
			R_CDSP_TG_ZERO = 0x1FC;
			R_CDSP_FRONT_CTRL3 &= ~0x800;
			R_CDSP_DATA_FORMAT |= 0x10;
			break;
		case SENSOR_RAW10:
			R_CDSP_TG_ZERO = 0x1FC;
			R_CDSP_FRONT_CTRL3 &= ~0x800;
			R_CDSP_DATA_FORMAT &= ~0x10;
			break;
		// Sensor YUV	
		case SENSOR_UYVY:
			R_CDSP_TG_ZERO = 0x1E0;
			R_CDSP_FRONT_CTRL3 &= ~0x03;
			R_CDSP_FRONT_CTRL3 |= 0x860;
			break;
		case SENSOR_YVYU:
			R_CDSP_TG_ZERO = 0x1E0;
			R_CDSP_FRONT_CTRL3 &= ~0x03;
			R_CDSP_FRONT_CTRL3 |= 0x861;
			break;
		case SENSOR_VYUY:
			R_CDSP_TG_ZERO = 0x1E0;
			R_CDSP_FRONT_CTRL3 &= ~0x03;
			R_CDSP_FRONT_CTRL3 |= 0x862;
			break;
		case SENSOR_YUYV:
			R_CDSP_TG_ZERO = 0x1E0;
			R_CDSP_FRONT_CTRL3 |= 0x863;
			break;
		// MIPI	
		case MIPI_RAW8:
			R_CDSP_TG_ZERO = 0x1FF;
			R_CDSP_FRONT_GCLK |= 0x3F;
			R_CDSP_MIPI_CTRL = 0x0002;
			R_CDSP_DATA_FORMAT |= 0x10;
			break;
		case MIPI_RAW10:
			R_CDSP_TG_ZERO = 0x1FF;
			R_CDSP_FRONT_GCLK |= 0x3F;
			R_CDSP_MIPI_CTRL = 0x0002;
			R_CDSP_DATA_FORMAT &= ~0x10;
			break;
		case MIPI_UYVY:
			R_CDSP_TG_ZERO = 0x1FF;
			R_CDSP_FRONT_GCLK |= 0x3F;
			R_CDSP_FRONT_CTRL3 &= ~0x03;
			R_CDSP_FRONT_CTRL3 |= 0x860;
			R_CDSP_MIPI_CTRL = 0x0001;
			break;
			
		default:
			while(1);

		//DBG_PRINT("<RawFormat>R_CDSP_DATA_FORMAT:%d, \r\n",((R_CDSP_DATA_FORMAT>>4)&0x1));
		//DBG_PRINT("<mipiType>R_CDSP_MIPI_CTRL:0x%x, \r\n",((R_CDSP_MIPI_CTRL>>2)&0x7));
	}
	
}

void 
hwFront_InputYuvIfSet(
	INT8U bt656en,
	INT8U tvresh_vh
)
{

	
}

void hwFront_SetFrameSize(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
)
{
	R_CDSP_FRAME_H_SETTING = hsize & 0xFFF | (INT32U)hoffset << 16;
	R_CDSP_FRAME_V_SETTING = vsize & 0xFFF | (INT32U)voffset << 16;
}

void hwFront_SetVFrameSize(
	INT16U voffset,
	INT16U vsize
)
{
	R_CDSP_FRAME_V_SETTING = vsize & 0xFFF | (INT32U)voffset << 16;
}

void 
hwFront_SetMipiFrameSize(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
)
{
	R_CDSP_MIPI_HVOFFSET = hoffset & 0xFFF | (INT32U)(voffset& 0xFFF ) << 12;
	R_CDSP_MIPI_HVSIZE = hsize & 0xFFF | (INT32U)(vsize & 0xFFF) << 12;
}

void 
hwFront_TGSigGatingToZero(
	INT16U tg_in_gate_zero
)
{	
	// 1: enable, 0:disbale gating to zero
	R_CDSP_TG_ZERO = tg_in_gate_zero & 0x1FF;
}

void 
hwFront_SyncWithHVSize(
	INT8U HVsvden
)
{
	R_CDSP_FRONT_CTRL2 &= ~(0x03 << 4);
	R_CDSP_FRONT_CTRL2 |= (HVsvden & 0x03) << 4;
}

void hwFront_SetSize(
	INT32U hsize,
	INT32U vsize
)
{
	R_CDSP_FRAME_H_SETTING &= ~0xFFF;
	R_CDSP_FRAME_V_SETTING &= ~0xFFF;
	R_CDSP_FRAME_H_SETTING = R_CDSP_FRAME_H_SETTING | (hsize & 0xFFF);
	R_CDSP_FRAME_V_SETTING = R_CDSP_FRAME_V_SETTING | (vsize & 0xFFF);
}

void hwMipi_SetSize(
	INT32U hsize,
	INT32U vsize
)
{
	R_CDSP_MIPI_HVSIZE = hsize & 0xFFF | (INT32U)(vsize & 0xFFF) << 12;
}

/**************
front interrupt
***************/
void 
hwFront_VdRFIntEnable(
	INT8U vd_int
)
{
	R_CDSP_FRONT_VDRF_INTEN = vd_int & 0x03;
}

void 
hwFront_IntEnable(
	INT16U bit
)
{	
	R_CDSP_FRONT_INTEN = bit & 0x3FF;
}

void
hwFront_VdUpdateEn(
	INT8U VdUpdate
)
{
	R_CDSP_FRONT_CTRL0 &= ~0x10;
	R_CDSP_FRONT_CTRL0 |= (VdUpdate & 0x1) << 4;
} 

void
hwFront_SiggenSet(
	INT8U siggen_en,
	INT8U sig_clr_bar,
	INT32U sigmode
)
{
	R_CDSP_SIG_GEN = sigmode & 0x0F |
				(sig_clr_bar & 0x03) << 5 |
				(siggen_en & 0x1) << 7;
} 

INT32U
hwFront_FrameRateGet(
	void
)
{
	INT32U ltotal, ftotal, pixclk, framerate;
	
	ltotal = R_CDSP_TG_LINE_CTRL & 0xFFFF;       /* Read line total */
	ftotal = R_CDSP_FRAME_CTRL & 0xFFFF;        /* Read frame total */    	
	//pixclk = hwFrontPixClkRd();           /* Read pixel clock */

	framerate = pixclk / (ltotal * ftotal);
	return framerate;
}

/**************
flash set
***************/
void
hwFront_FlashSet(
	INT16U width,
	INT16U linenum,
	INT8U mode 
)
{
	R_CDSP_TG_GPIO_SEL_OEN &= ~(FLASH_CTRL_BIT); 		//sel
	R_CDSP_TG_GPIO_SEL_OEN |= (FLASH_CTRL_BIT) << 12;	//en
	
	R_CDSP_FLASH_WIDTH = width;
	R_CDSP_FLASH_TRIG_NUM = linenum ;
	
	R_CDSP_FRONT_CTRL0 &= ~(0x0D << 8);
	R_CDSP_FRONT_CTRL0 |= ((INT32U)mode & 0xD) << 8;
	//hwFrontVValidWait(0, 2);

	R_CDSP_TG_GPIO_SEL_OEN |= FLASH_CTRL_BIT; 			//sel
	R_CDSP_TG_GPIO_SEL_OEN &= ~((FLASH_CTRL_BIT) << 12);	//en
} 

/**************
wait v sync
***************/
void
hwFront_VdWait(
	INT32U mode,
	INT32U number
)
{
	INT32U i;
	INT32U timeOutCnt = 0x000FFFFF,tmp0;


	if(mode) {
		for(i=0;i<number;i++) {
			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = R_CDSP_SEN_CTRL_SIG & 0x02;  //vsync
				timeOutCnt--;
			} while(tmp0 == 0x02 && timeOutCnt > 0);

			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = R_CDSP_SEN_CTRL_SIG & 0x02; //vsync 
				timeOutCnt--;
			} while(tmp0 == 0x00 && timeOutCnt > 0);
		}
	}
	else {
		for(i=0;i<number;i++) {
    	
			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = R_CDSP_SEN_CTRL_SIG & 0x02; 
				timeOutCnt--;
			} while(tmp0 == 0x00 && timeOutCnt > 0);

			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = R_CDSP_SEN_CTRL_SIG & 0x02; 
				timeOutCnt--;
			} while(tmp0 == 0x02 && timeOutCnt > 0);
		}
	} 
} 

INT32U
hwFront_VValidWait(
	INT32U mode,
	INT32U number
)
{
	INT32U i;
	INT32U timeOutCnt = 0x000FFFFF,tmp0;
	
	if(mode) {
		for(i=0;i<number;i++) {    	
			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = R_CDSP_SEN_CTRL_SIG & 0x08; //v vaild sync
				timeOutCnt--;
			} while(tmp0 == 0x08 && timeOutCnt > 0);

			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = R_CDSP_SEN_CTRL_SIG & 0x08; 		//v vaild sync
				timeOutCnt--;
			} while(tmp0 == 0x00 && timeOutCnt > 0);
		}
	}
	else {
		for(i=0;i<number;i++) { 	
			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = R_CDSP_SEN_CTRL_SIG & 0x08; 
				timeOutCnt--;
			} while(tmp0 == 0x00 && timeOutCnt > 0);

			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = R_CDSP_SEN_CTRL_SIG & 0x08; 
				timeOutCnt--;
			} while(tmp0 == 0x08 && timeOutCnt > 0);

		}
	}
	return 0;
} 

/**************
gpio set
***************/
void 
hwFront_GpioSelEn(
	INT32U bit
)
{
	R_CDSP_TG_GPIO_SEL_OEN = bit;
}

void 
hwFront_GpioOutInSet(
	INT32U bit
)
{
	R_CDSP_TG_GPIO_OUT_IN = bit;
}

/**************
gpio int set
***************/
void 
hwFront_GpioRiseIntEn(
	INT16U bit
)
{
	R_CDSP_TG_GPIO_REVENT &= ~0x7FF;
	R_CDSP_TG_GPIO_REVENT |= bit; 		
}

INT32U 
hwFront_GpioRiseIntStatus(
	void
)
{
	return R_CDSP_TG_GPIO_REVENT >> 11; 		
}

void 
hwFront_GpioRiseIntClr(
	INT16U bit
)
{
	
}

void 
hwFront_GpioFallIntEn(
	INT16U bit
)
{
	R_CDSP_TG_GPIO_FEVENT &= ~0x7FF;
	R_CDSP_TG_GPIO_FEVENT |= bit; 		
}

INT32U 
hwFront_GpioFallIntStatus(
	void
)
{
	return R_CDSP_TG_GPIO_FEVENT >> 11;	
}

void 
hwFront_GpioFallIntClr(
	INT16U bit
)
{
	
}


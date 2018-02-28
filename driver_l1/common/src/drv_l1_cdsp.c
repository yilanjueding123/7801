#include "project.h"
#include "driver_l1.h"
#include "drv_l1_sfr.h"
#include "drv_l1_cdsp.h"
#include "drv_l1_front.h"

#if (defined _DRV_L1_CDSP) && (_DRV_L1_CDSP == 1)                   //
//================================================================//
//extern INT8U LEElut[];
extern INT8U LiTable_rgb[];
extern INT16U MaxTan8[];
extern INT16U Slope4[];
extern INT16U CLPoint[];
extern INT16U Radius_File_0[];
extern INT16U Radius_File_1[];
extern INT32U gammatable[];

//system init
extern void CDSP_CLK_Init(void);
extern void CDSPFB_CLK_Setting(void);
extern void SensorIF_Setting(unsigned char front_type);
extern void MipiIF_Setting(	unsigned char mipi_type, unsigned int width, unsigned int height);
extern void print_string(CHAR *fmt, ...);

void 	(*cdsp_eof_callback)(void);
void 	(*cdsp_ae_callback)(void);
void 	(*cdsp_awb_callback)(void);

volatile INT8U cdsp_eof_occur_flag = 0;

//+++ cdsp overflow 
volatile INT8U cdsp_overflow_occur_flag = 0;
void (*cdsp_overflow_callback)(void);

//---

INT32U frame_width_after_clip;

//+++
#if C_MOTION_DETECTION == CUSTOM_ON
__align(16) INT32U md_work_memory_addr[3616/4];

INT8U motion_detect_status_flag = MOTION_DETECT_STATUS_IDLE;

void motion_detect_status_set(INT8U statusVal)
{
	motion_detect_status_flag = statusVal;
}
#endif
//---

/*******************
CDSP CLK INIT.
********************/
#if 0
void CDSP_CLK_Init(void)
{
	//CDSP CLK Enable Setting
	//[29] : CDSP AHB CLK EN
	rSCUA_PERI_CLKEN = 0xFFFFFFFF;
	
	//[8]:Enable CDSP Clk,[7:0] Ratio
	rSCUA_CDSP_CLK_RATIO = 0x101;
	
	rSCUA_PERI_CLKEN1 |= (1<<5);	//PPU 27MHz clock enable
	rSCUA_VDAC_CFG = 0;				//TV dac enable
	
	rSCUA_PERI_CLKEN1 |= (1<<8);	//cdsp_clk_en, CDSP clock enable
	rSCUA_PERI_CLKEN1 &= ~(1<<9);	//0: Run raw mode
	rSCUA_PERI_CLKEN1 |= (1<<10); 	//1: Clock From System Clock ==> For Ram mode use
	rSCUA_PERI_CLKEN1 &= ~(1<<11);	//2 CSI source switch. 1: From csi2 ,0: From csi
 	rSCUA_PERI_CLKEN1 &= ~(1<<12);	//1: Clock from Mipi, 0:clock from csi
}
#endif
#if 1
void CDSPFB_CLK_Setting(void)
{
	(*(volatile unsigned *)(0xD00000E0)) |= (1<<8);	//cdsp_clk_en, CDSP clock enable
	(*(volatile unsigned *)(0xD00000E0)) &= ~(1<<9);	//0: Run raw mode
	(*(volatile unsigned *)(0xD00000E0)) |= (1<<10); 	//1: Clock From System Clock ==> For Ram mode use
	(*(volatile unsigned *)(0xD00000E0)) &= ~(1<<11);	//2 CSI source switch. 1: From csi2 ,0: From csi
 	(*(volatile unsigned *)(0xD00000E0)) &= ~(1<<12);	//1: Clock from Mipi, 0:clock from csi
}
#endif

void SdramIF_Setting(unsigned char front_type)/* 0: Raw 1:YUV */
{
	//For Enable Sensel GPIO Function
	//Fun_POS = 0x0;
	R_SYSTEM_CLK_CTRL |= 0x08;	//0xD000001C,enable clock
	
#if 1
	R_CSI_TG_CTRL1 &= ~0x00000800;		////0xD0500244,select Crystal option:6/12/27MHz
	/*-----------From Main clock div--------------------------------------------*/	
#elif 1		// Div 2	 
	R_CSI_TG_CTRL1 |= 0x00000800;		//0xD0500244
	R_SYSTEM_CTRL &= ~0x00004000;		//0xD000000C,C_SYSTEM_CTRL_CSI_CLOCK_DIV
#else		// Div 4	
	R_CSI_TG_CTRL1 |= 0x00000800;
	R_SYSTEM_CTRL |= 0x00004000;		// C_SYSTEM_CTRL_CSI_CLOCK_DIV: When set, CMOS sensor clock is divided by 2
#endif
	/*-----------Enable MCLK --------------------------------------------*/
	R_CSI_TG_CTRL1 |= 0x00000080;		//0xD0500244, Enable CSI MCLK from osc (48/2),(48/4)MHz,or crystal 6/12/27MHz

	if(front_type == C_YUV_FMT) {		//0xD0000008
 		R_CDSP_YUV_MODE  |= (1<<1) ;		//1: Run YUV mode //cdsp clock sel, yuyv 2pixel clock
 	}
	else{
		R_CDSP_YUV_MODE  &= ~(1<<1);		//0: Run raw mode, raw 1pixel clock
	}
	//D000_0008 [3:2]	0:From DRAM 1:Sen :2 MIPI
	R_CDSP_YUV_MODE &= ~(1<<2);  //sensor	 
}

//sensor
void SensorIF_Setting(unsigned char front_type)/* 0: Raw 1:YUV */
{
	//For Enable Sensel GPIO Function
	//Fun_POS = 0x0;
	R_SYSTEM_CLK_CTRL |= 0x08;	//0xD000001C,enable clock
	
#if 0
	R_CSI_TG_CTRL1 &= ~0x00000800;		////0xD0500244,select Crystal option:6/12/27MHz
	/*-----------From SYSTEM CLK DIV (OSC)--------------------------------------------*/	
#elif 0		// Div 2	 				
	R_CSI_TG_CTRL1 |= 0x00000800;		//0xD0500244
	R_SYSTEM_CTRL &= ~0x00004000;		//0xD000000C,C_SYSTEM_CTRL_CSI_CLOCK_DIV
#else		// Div 4	
	R_CSI_TG_CTRL1 |= 0x00000800;
	R_SYSTEM_CTRL |= 0x00004000;		// C_SYSTEM_CTRL_CSI_CLOCK_DIV: When set, CMOS sensor clock is divided by 2
#endif
	/*-----------Enable MCLK --------------------------------------------*/
	R_CSI_TG_CTRL1 |= 0x00000080;		//0xD0500244, Enable CSI MCLK from osc (48/2),(48/4)MHz,or crystal 6/12/27MHz

	if(front_type == C_YUV_FMT) {		//0xD0000008
 		R_CDSP_YUV_MODE  |= (1<<1) ;		//1: Run YUV mode //cdsp clock sel, yuyv 2pixel clock
 	}
	else{
		R_CDSP_YUV_MODE  &= ~(1<<1);		//0: Run raw mode, raw 1pixel clock
	}
	//D000_0008 [3:2]	0:From DRAM 1:Sen :2 MIPI
	R_CDSP_YUV_MODE |= (1<<2);  //sensor	 
}

void CDSP_SensorIF_CLK(unsigned char front_type)/* 0: Raw 1:YUV */
{
	//For Enable Sensel GPIO Function
	//Fun_POS = 0x0;
	R_SYSTEM_CLK_CTRL |= 0x08;			//0xD000001C,enable clock

	/*-----------Enable MCLK --------------------------------------------*/
	//R_CSI_TG_CTRL1 |= 0x00000080;		//0xD0500244, Enable CSI MCLK from osc (48/2),(48/4)MHz,or crystal 6/12/27MHz

	if(front_type == C_YUV_FMT) 		//cdsp clock sel(0xD0000008)
	{
 		R_CDSP_YUV_MODE  |= (1<<1) ;	//1: Run YUV mode, yuyv 2pixel clock
 	}
	else
	{
		R_CDSP_YUV_MODE  &= ~(1<<1);	//0: Run raw mode, raw 1pixel clock
	}
	
	R_CDSP_YUV_MODE |= (1<<2);  		//D000_0008 [3:2]	0:From DRAM 1:Parallel Sensor :2 MIPI Sensor	 
}


void MipiIF_Setting( unsigned char mipi_type, unsigned int width, unsigned int height)	/* 0: Raw 1:YUV */
{
	INT32U intpl_heigh;
	INT32U intpl_width;
	
	intpl_heigh = height+2; //Enabel new denoise need 2line
	intpl_width = width+4; //Left need shift 4pixel column 
	/*-----------program Mclk IO -------------------*/
	#if (MCLK_IO_POS == MCLK_IO_B9)
	R_FUNPOS0 |= 0<<7;
	#elif (MCLK_IO_POS == MCLK_IO_D12)
	R_FUNPOS1 |= 0x42;
	#elif (MCLK_IO_POS == MCLK_IO_D7)
	R_FUNPOS1 |= 0x21;		
	#endif

	/*-----------program Mclk frquence 24Mhz--------*/
	#if (MCLK_IO_POS == MCLK_IO_B9)
	timer_pwm_setup(TIMER_B, (24000000), 50, PWM_NRO_OUTPUT);
	#else
	R_SYSTEM_CTRL |= (1<< 11); //0x800;				//SEN_48M_SEL (0xD000000C[11])
	R_CSI_TG_CTRL1 |= ((1<<11) | (1<<7)); 			//0x0000880/*0xD0500244,//System clock+CLKOEN //Enable CSI MCLK from "SystemCLK/4 = 96/4"*/
	R_SYSTEM_CTRL |= (1<<14); //0x4000;				//MCLK/2(0xD000000C[14])
	#endif

	/*-----------program cdsp---------------------------------------------------------*/
	if(mipi_type == C_YUV_FMT) 				/*D00000008[1] sel YUV/ RAW mode*/
	{
		R_CDSP_YUV_MODE |= (1<<1) ;		 	//1: Run YUV mode //cdsp clock sel, yuyv 2pixel clock
 	}
	else 
	{
		R_CDSP_YUV_MODE &= ~(1<<1);			//0: Run raw mode, raw 1pixel clock
	}	 
	R_CDSP_YUV_MODE |= (2<<2);  			//sel source path	/*D00000008[3:2]	0:From DRAM 1:Sen :2 MIPI*/
	/*-----------program cdsp end---------------------------------------------------------*/
	
	/*-----------program mipi---------------------------------------------------------*/
	//enable power down n 0xD0F00000[0]
	(*(volatile unsigned *)0xD0F00000) |= 0x00000021; //P_GLB_CSR ,Enable MIPI
	
	#if (sensor_format == GC1004_MIPI)
	(*(volatile unsigned *)0xD0F0000c)  = 0x00011B07;	//0x00011B07; //0x00010807; //P_ECC_ORDER  //栦馱 0x00012007
	#else
	(*(volatile unsigned *)0xD0F0000c)  =  0x00010807; //P_ECC_ORDER for H22_mipi
	#endif
	
	if(mipi_type == C_YUV_FMT) 
	{      
		(*(volatile unsigned *)0xD0F00014) |= (intpl_heigh & 0xFFFF) << 16 | ((intpl_width << 1) & 0xFFFF);//0x01e00500; //P_IMAGE_SIZE
	}
	else 
	{
		(*(volatile unsigned *)0xD0F00014) |= (intpl_heigh & 0xFFFF) << 16 | (intpl_width & 0xFFFF);//0x01e00280;
	}
	#if (sensor_format == GC1004_MIPI)
	(*(volatile unsigned *)0xD0F00020) =0xc1;//D1; //C1;//Data Format auto switch
	#else
	(*(volatile unsigned *)0xD0F00020) =0x80; //Data Format auto switch for H22_mipi
	#endif
	//front&back porch
	(*(volatile unsigned *) 0xD0F00010) |= 0x00010404;

	(*(volatile unsigned *) 0xD0F00000) &= ~(1<<8);
		
	(*(volatile unsigned *) 0xD0F00004) |=0x3; //SOF_RST_DA & CK
	(*(volatile unsigned *) 0xD0F00080) |=0x3F; // Clear interrupt
	(*(volatile unsigned *) 0xD0F00040) |=0x3F; //Enable mipi interrupt
	/*-----------program mipi end---------------------------------------------------------*/
}

/***************************
CDSP ISR & INT Handle
****************************/
/**
* @brief	cdsp get int status
* @param	none
* @return	int status
*/
INT32U gpHalCdspGetIntStatus(void)
{
	INT32U irq = 0;
	
	if(R_CDSP_INT & R_CDSP_INTEN & CDSP_FACWR)
	{
		R_CDSP_INT |= CDSP_FACWR;
		irq |= CDSP_FACWR;
	}
		
	if(R_CDSP_INT & R_CDSP_INTEN & CDSP_OVERFOLW)
	{
		R_CDSP_INT |= CDSP_OVERFOLW;
		irq |= CDSP_OVERFOLW;
	}
	
	if(R_CDSP_INT & R_CDSP_INTEN & CDSP_EOF)
	{
		R_CDSP_INT |= CDSP_EOF;
		irq |= CDSP_EOF;
	}
		
	if(R_CDSP_INT & R_CDSP_INTEN & CDSP_AEWIN_SEND)
	{
		R_CDSP_INT |= CDSP_AEWIN_SEND;
		irq |= CDSP_AEWIN_SEND;
	}
		
	if(R_CDSP_INT & R_CDSP_INTEN & CDSP_AWBWIN_UPDATE)
	{
		R_CDSP_INT |= CDSP_AWBWIN_UPDATE;
		irq |= CDSP_AWBWIN_UPDATE;
	}
		
	if(R_CDSP_INT & R_CDSP_INTEN & CDSP_AFWIN_UPDATE)
	{
		R_CDSP_INT |= CDSP_AFWIN_UPDATE;
		irq |= CDSP_AFWIN_UPDATE;
	}
	return irq;
}

/**
* @brief	cdsp get front vd int status
* @param	none
* @return	int status
*/
INT32U gpHalCdspGetFrontVdIntStatus(void)
{
	INT32U irq = 0;

	if(R_CDSP_FRONT_VDRF_INT & R_CDSP_FRONT_VDRF_INTEN & FRONT_VD_RISE)
	{
		R_CDSP_FRONT_VDRF_INT |= FRONT_VD_RISE;
		irq |= FRONT_VD_RISE;
	} 
		
	if(R_CDSP_FRONT_VDRF_INT & R_CDSP_FRONT_VDRF_INTEN & FRONT_VD_FALL) 
	{
		R_CDSP_FRONT_VDRF_INT |= FRONT_VD_FALL;
		irq |= FRONT_VD_FALL;
	}	
/*
	if(pCdspReg1->cdspGInt & CDSP_INT_BIT)
		irq |= CDSP_INT_BIT;

	if(pCdspReg1->cdspGInt & FRONT_VD_INT_BIT) 
		irq |= FRONT_VD_INT_BIT;

	if(pCdspReg1->cdspGInt & FRONT_INT_BIT)
		irq |= FRONT_INT_BIT;
*/	
	return irq;
}

/**
* @brief	cdsp get front int status
* @param	none
* @return	int status
*/
INT32U gpHalCdspGetFrontIntStatus(void)
{
	INT32U irq = 0;

#if 0	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & VDR_EQU_VDRINT_NO) 
	{
		R_CDSP_FRONT_INT |= VDR_EQU_VDRINT_NO;
		irq |= VDR_EQU_VDRINT_NO;
	} 
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & VDF_EQU_VDFINT_NO) 
	{
		R_CDSP_FRONT_INT |= VDF_EQU_VDFINT_NO;
		irq |= VDR_EQU_VDRINT_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & SERIAL_DONE) 
	{
		R_CDSP_FRONT_INT |= SERIAL_DONE;
		irq |= VDR_EQU_VDRINT_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & SNAP_DONE) 
	{
		R_CDSP_FRONT_INT |= SNAP_DONE;
		irq |= VDR_EQU_VDRINT_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & CLR_DO_CDSP) 
	{
		R_CDSP_FRONT_INT |= CLR_DO_CDSP;
		irq |= VDR_EQU_VDRINT_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & EHDI_FS_EQU_INTH_NO) 
	{
		R_CDSP_FRONT_INT |= EHDI_FS_EQU_INTH_NO;
		irq |= VDR_EQU_VDRINT_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & FRONT_VVALID_RISE) 
	{
		R_CDSP_FRONT_INT |= FRONT_VVALID_RISE;
		irq |= VDR_EQU_VDRINT_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & FRONT_VVALID_FALL) 
	{
		R_CDSP_FRONT_INT |= FRONT_VVALID_FALL;
		irq |= VDR_EQU_VDRINT_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & OVERFLOW_RISE) 
	{
		R_CDSP_FRONT_INT |= OVERFLOW_RISE;
		irq |= VDR_EQU_VDRINT_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & OVERFLOW_FALL) 
	{
		R_CDSP_FRONT_INT |= OVERFLOW_FALL;
		irq |= VDR_EQU_VDRINT_NO;
	}
#endif
	return irq;
}

/***************************
CDSP ISR & Calback Function
****************************/

void cdsp_eof_isr_register(void (*user_isr)(void))
{
	cdsp_eof_callback = user_isr;
}

void cdsp_ae_isr_register(void (*user_isr)(void))
{
	cdsp_ae_callback = user_isr;
}

void cdsp_awb_isr_register(void (*user_isr)(void))
{
	cdsp_awb_callback = user_isr;
}

//+++
void cdsp_overflow_isr_register(void (*user_isr)(void))
{
	cdsp_overflow_callback = user_isr;
}
//---

/*
static void front_vd_int_handle(void)
{
	if(R_CDSP_FRONT_VDRF_INT & R_CDSP_FRONT_VDRF_INTEN & FRONT_VD_RISE)
	{
		R_CDSP_FRONT_VDRF_INT |= FRONT_VD_RISE;
	} 
		
	if(R_CDSP_FRONT_VDRF_INT & R_CDSP_FRONT_VDRF_INTEN & FRONT_VD_FALL) 
	{
		R_CDSP_FRONT_VDRF_INT |= FRONT_VD_FALL;
	}	
}

static void front_int_handle(void)
{
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & VDR_EQU_VDRINT_NO) 
	{
		R_CDSP_FRONT_INT |= VDR_EQU_VDRINT_NO;
	} 
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & VDF_EQU_VDFINT_NO) 
	{
		R_CDSP_FRONT_INT |= VDF_EQU_VDFINT_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & SERIAL_DONE) 
	{
		R_CDSP_FRONT_INT |= SERIAL_DONE;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & SNAP_DONE) 
	{
		R_CDSP_FRONT_INT |= SNAP_DONE;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & CLR_DO_CDSP) 
	{
		R_CDSP_FRONT_INT |= CLR_DO_CDSP;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & EHDI_FS_EQU_INTH_NO) 
	{
		R_CDSP_FRONT_INT |= EHDI_FS_EQU_INTH_NO;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & FRONT_VVALID_RISE) 
	{
		R_CDSP_FRONT_INT |= FRONT_VVALID_RISE;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & FRONT_VVALID_FALL) 
	{
		R_CDSP_FRONT_INT |= FRONT_VVALID_FALL;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & OVERFLOW_RISE) 
	{
		R_CDSP_FRONT_INT |= OVERFLOW_RISE;
	}
	
	if(R_CDSP_FRONT_INT & R_CDSP_FRONT_INTEN & OVERFLOW_FALL) 
	{
		R_CDSP_FRONT_INT |= OVERFLOW_FALL;
	}
}
*/

void cdsp_isr(void)
{
	if(R_CDSP_GINT & CDSP_INT_BIT) //cdsp int
	{
		//+++
		if(R_CDSP_INT & CDSP_OVERFOLW)
		{
			DBG_PRINT("L");
			cdsp_overflow_occur_flag = 1;
			R_CDSP_INT |= CDSP_OVERFOLW;
		}
		//---

		if(R_CDSP_INT & R_CDSP_INTEN & CDSP_FACWR)
		{
			R_CDSP_INT |= CDSP_FACWR;
			//while(1);
		}

		if(R_CDSP_INT & R_CDSP_INTEN & CDSP_AEWIN_SEND)
		{
			R_CDSP_INT |= CDSP_AEWIN_SEND;

			if (cdsp_ae_callback != 0)
	    	{
				(*cdsp_ae_callback)();
			}

		  #if 0
			if(R_CDSP_AE_AWB_WIN_CTRL & 0x8000)
				g_ready_ae_win = g_ae_window[1];//R_AE_WIN_BBUFADDR;
			else
				g_ready_ae_win = g_ae_window[0];//R_AE_WIN_ABUFADDR;
		  #endif
		}

		if(R_CDSP_INT & R_CDSP_INTEN & CDSP_AWBWIN_UPDATE)
		{
			R_CDSP_INT |= CDSP_AWBWIN_UPDATE;
			if (cdsp_awb_callback != 0)
	    	{
				(*cdsp_awb_callback)();
			}
		}
#if 0
		if(R_CDSP_INT & R_CDSP_INTEN & CDSP_AFWIN_UPDATE)
		{
			R_CDSP_INT |= CDSP_AFWIN_UPDATE;
		}
#endif

		if(R_CDSP_INT & R_CDSP_INTEN & CDSP_EOF)
		{
			R_CDSP_INT |= CDSP_EOF;

			cdsp_eof_occur_flag = 1;

			//DBG_PRINT("x"); //wwj test

			//+++ 發生cdsp overflow
			if(cdsp_overflow_occur_flag)
			{
				cdsp_overflow_occur_flag = 0;
				hwCdsp_Reset();

				DBG_PRINT("C");
				DBG_PRINT("%1x",(R_PROTECT_STATUS));

				if (cdsp_overflow_callback != 0)
	    		{
					(*cdsp_overflow_callback)();
				}
			}
			//---

			//+++
			#if C_MOTION_DETECTION == CUSTOM_ON
			if(motion_detect_status_flag != MOTION_DETECT_STATUS_IDLE)
			{
				if(motion_detect_status_flag == MOTION_DETECT_STATUS_START)
				{
					hwCdsp_MD_set(1,CDSP_MD_THR,frame_width_after_clip, (INT32U) &md_work_memory_addr[0]);
				}
				if(motion_detect_status_flag == MOTION_DETECT_STATUS_STOP)
				{
					hwCdsp_MD_set(0,CDSP_MD_THR,frame_width_after_clip, (INT32U) &md_work_memory_addr[0]);
				}
				motion_detect_status_flag = MOTION_DETECT_STATUS_IDLE;
			}
			#endif
			//---

			if (cdsp_eof_callback != 0)
	    	{
				(*cdsp_eof_callback)();
			}
		}				
	}

	if(R_CDSP_GINT & FRONT_VD_INT_BIT) //vd int
	{
		//front_vd_int_handle();
	}
	if(R_CDSP_GINT & FRONT_INT_BIT) //front int
	{
		//front_int_handle();
	}
}



/**
* @brief	cdsp get global int status
* @param	none
* @return	int status
*/
INT32U gpHalCdspGetGlbIntStatus(void)
{
	INT32U irq = 0;
	
	if(R_CDSP_GINT & CDSP_INT_BIT)
		irq |= CDSP_INT_BIT;

	if(R_CDSP_GINT & FRONT_VD_INT_BIT) 
		irq |= FRONT_VD_INT_BIT;

	if(R_CDSP_GINT & FRONT_INT_BIT)
		irq |= FRONT_INT_BIT;
	
	return irq;
}

/***************
cdsp system
****************/
void hwCdsp_Reset(
	void
)
{
	R_CDSP_RST = 0x01;
	R_CDSP_RST = 0x00;
}

void hwCsdp_EnableDynGClk(
	INT8U enable
)
{
	if (enable==1) 
		R_CDSP_GATING_CLK_CTRL = 0x00;
	else if (enable == 0)
		R_CDSP_GATING_CLK_CTRL = 0xFFFFFFFF;
}

void hwCdsp_RedoTriger(
	INT8U docdsp
)
{
	if(docdsp)
		R_CDSP_DO |= 0x01;
	else
		R_CDSP_DO &= ~0x01;
}

void hwCdsp_SetGatingValid(
	INT8U gating_gamma_vld,
	INT8U gating_badpixob_vld
)
{
	R_CDSP_DATA_FORMAT &= ~(0x03 << 8);
	R_CDSP_DATA_FORMAT |= ((INT32U)gating_gamma_vld & 0x1) << 8 |
					 ((INT32U)gating_badpixob_vld & 0x1) << 9;
}

void hwCdsp_SetDarkSub(
	INT8U raw_sub_en,
	INT8U sony_jpg_en
)
{
	R_CDSP_DATA_FORMAT &= ~0x0880;
	R_CDSP_DATA_FORMAT |= (raw_sub_en & 0x1) << 7 |
					((INT32U)sony_jpg_en & 0x1) << 11; 
}

/*U2 Added*/
void hwCdsp_SetOutYUVFmt(			
	INT8U yuv_format //out sequence, 0:YUYV, 1:UYVY, 2:YVYU, 3:VYUY
)
{
	R_CDSP_DATA_FORMAT &= ~0xC000;						/*0x240*/
	R_CDSP_DATA_FORMAT |= yuv_format  << 14; 
	
	//DBG_PRINT("\r\n",yuv_format);
}


void hwCdsp_SetRawDataFormat(
	INT8U format
)
{
	R_CDSP_IMG_TYPE = format & 0x03;
	
	//DBG_PRINT("<ImageInput>linesw|pixsw: 0x%x\r\n", format);
}

void hwCdsp_SetYuvRange(
	INT8U signed_yuv_en
)
{
	R_CDSP_YUV_RANGE &= ~0x07;
	R_CDSP_YUV_RANGE |= signed_yuv_en;
}	

void hwCdsp_SetSRAM(
	INT8U overflowen,
	INT16U sramthd
)
{
	if(sramthd > 0x1FF)
		sramthd = 0x100;
	
	R_CDSP_WSRAM_THR = (sramthd & 0x1FF) |
					((INT32U)overflowen & 0x1) << 11;
}

/*******************
interrupt
********************/
void hwCdsp_SetInt(
	INT8U inten,
	INT8U bit
)
{
	if (inten)
	{
		R_CDSP_INT |= bit;	//clear
		R_CDSP_INTEN |= bit;	//enable
	}
	else
	{
		R_CDSP_INTEN &= ~bit;	//disable
		R_CDSP_INT |= bit;	//clear
		
	}
}

/*******************
other
********************/
void hwCdsp_SetLineInterval(
	INT16U line_interval
)
{
	R_CDSP_LINE_INTERVAL = line_interval;
}

void hwCdsp_SetExtLine(	
	INT8U extinen,
	INT16U linesize,
	INT16U lineblank
)
{
	//DBG_PRINT("<ExtLine>\r\n");
	if(extinen){
		R_CDSP_INP_MIRROR_CTRL |= 0x30;		//B[4]:Suggest it set 1
		
		//DBG_PRINT("extlinen = 1, intplmirvsel = 1\r\n");
	}
	else
	{
		R_CDSP_INP_MIRROR_CTRL &= ~0x20;
		
		//DBG_PRINT("extlinen = 0\r\n");
	}
	R_CDSP_EXT_LINE_SIZE = linesize;
	R_CDSP_EXT_BANK_SIZE = lineblank;

	//DBG_PRINT("linesize = %d, lineblank = %d\r\n",linesize,lineblank);
}

/*******************
path set
********************/
void hwCdsp_DataSource(
	INT8U image_source
)
{
	// 0:front, 1:sdram, 2:mipi
	R_CDSP_DO &= ~(0x03 << 4);
	R_CDSP_DO |= (image_source << 4);
}

/**
* @brief	cdsp raw data path set
* @param	raw_mode [in]: raw data path set, 0:YUV out, 1:Raw data out, 3:WBgain out, 5:LutGamma out
* @param	cap_mode [in]: set 0:raw10, 1:raw8 
* @param	yuv_mode [in]: set 0:8y4u4v, 1:yuyv set
* @param	yuv_format[in]: set out sequence, 0:YUYV, 1:UYVY, 2:YVYU, 3:VYUY
* @return	none
*/
void hwCdsp_SetRawPath(
	INT8U raw_mode, //rgb path
	INT8U cap_mode, //0: package 10 to 16, 1:package 8 to 16  
	INT8U yuv_mode	//0:8y4u4v, 1: yuyv
)   
{
	R_CDSP_DATA_FORMAT &= ~0x3F;				/*0x240*/
	R_CDSP_DATA_FORMAT |= (raw_mode & 0x07) |
					(cap_mode & 0x1) << 4 |
					(yuv_mode & 0x1) << 5;					
}

void hwCdsp_SetYuvMuxPath(
	INT8U redoedge
)
{
	if(redoedge)
		R_CDSP_DO |= 0x02;	//YUV path	
	else
		R_CDSP_DO &= ~0x02;	//YUV path6
}

void hwCdsp_SetLensCmpPath(
	INT8U yuvlens
)
{
	//if yuv path, imgcrop, yuvhavg
	//else yuvhavg
	if(yuvlens)
		R_CDSP_LENS_CMP_CTRL |= 0x20;	//YUV path2	
	else
		R_CDSP_LENS_CMP_CTRL &= ~0x20;	//YUV path5
}

void hwCdsp_SetExtLinePath(
	INT8U path
)
{
	if(path)
		R_CDSP_INP_MIRROR_CTRL |= 0x40;
	else
		R_CDSP_INP_MIRROR_CTRL &= ~0x40;
}

void hwCdsp_SetLineCtrl(
	INT8U ybufen	//0: raw data, 1: YUV data	
)
{
	if(ybufen)
		R_CDSP_YUV_CTRL |= 0x80;	//sol1, YUV data	
	else
		R_CDSP_YUV_CTRL &= ~0x80;	//sol2, raw data
}

/********************
cdsp dma buffer set
*********************/
void hwCdsp_SetYuvBuffA(
	INT16U width,
	INT16U height,
	INT32U buffer_addr
)
{
	R_CDSP_DMA_YUVABUF_HVSIZE =  (width & 0xFFF) | (((INT32U)height&0xFFF) << 12);
	R_CDSP_DMA_YUVABUF_SADDR = buffer_addr;
}

void hwCdsp_SetYuvBuffB(
	INT16U width,
	INT16U height,
	INT32U buffer_addr
)
{
	R_CDSP_DMA_YUVBBUF_HVSIZE =  (width & 0xFFF) | (((INT32U)height&0xFFF) << 12);
	R_CDSP_DMA_YUVBBUF_SADDR = buffer_addr;
}

void hwCdsp_SetRawBuff(	//width, height must be (x8/16) for 0x240 B[4]
	INT32U width,
	INT32U height,
	INT16U hoffset,
	INT32U buffer_addr
)
{
		R_CDSP_DMA_RAWBUF_HVSIZE =  (((INT32U)width*8/16) & 0xFFF) | 
									((((INT32U)height*8/16) & 0xFFF) << 12);
		R_CDSP_DMA_RAWBUF_HOFFSET = hoffset;
		R_CDSP_DMA_RAWBUF_SADDR = buffer_addr;	
}

void hwCdsp_SetRaw10Buff(	//width, height must be (x10/16) for 0x240 B[4]
	INT32U width,
	INT32U height,
	INT16U hoffset,
	INT32U buffer_addr
)
{
		R_CDSP_DMA_RAWBUF_HVSIZE = (((INT32U)(width*10/16))& 0xFFF) | 
									(((INT32U)((height*10/16)) & 0xFFF) << 12);
		R_CDSP_DMA_RAWBUF_HOFFSET = hoffset;
		R_CDSP_DMA_RAWBUF_SADDR = buffer_addr;	
}


void hwCdsp_SetDmaBuff(
	INT8U buffer_mode
)
{
	if(buffer_mode == RD_A_WR_A)
		R_CDSP_DMA_COFG = 0x00;
	else if(buffer_mode == RD_A_WR_B)
		R_CDSP_DMA_COFG = 0x01;
	else if(buffer_mode == RD_B_WR_B)
		R_CDSP_DMA_COFG = 0x03;
	else if(buffer_mode == RD_B_WR_A)
		R_CDSP_DMA_COFG = 0x02;
	else
		R_CDSP_DMA_COFG = 0x10;
}

void hwCdsp_SetFIFO(		//0x324
	INT8U frcen,
	INT8U fifo_en
)
{
	if(fifo_en){
		R_CDSP_DMA_COFG |= 0x01 << 5 |
						(frcen & 0x1) << 4;
	}	
}

void hwCdsp_SetFIFO_Line(		//0x324
	INT8U line_mode
)
{
#if 0	
	if(line_mode == 8_LINE)
		R_CDSP_DMA_COFG |= 0x00 << 6;
	else if(line_mode == 16_LINE)
		R_CDSP_DMA_COFG |= 0x01 << 6;
	else if(line_mode == 32_LINE)
		R_CDSP_DMA_COFG |= 0x02 << 6;
	else //(line_mode == 64_LINE)
		R_CDSP_DMA_COFG |= 0x03 << 6;
#else
	R_CDSP_DMA_COFG |= (line_mode & 0x3) << 6;
#endif
}

void hwCdsp_SetReadBackSize(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
)
{
	R_CDSP_WDRAM_HOFFSET = 0;//hoffset;
	R_CDSP_WDRAM_VOFFSET = 0;//voffset;
	
	R_CDSP_RB_HOFFSET = hoffset;
	R_CDSP_RB_VOFFSET = voffset;
	
	R_CDSP_RB_HSIZE = hsize;
	R_CDSP_RB_VSIZE = vsize;
}

void hwCdsp_SetReadBackSize10(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
)
{
	R_CDSP_WDRAM_HOFFSET = hoffset;
	R_CDSP_WDRAM_VOFFSET = voffset;
	
	R_CDSP_RB_HOFFSET = hoffset;
	R_CDSP_RB_VOFFSET = voffset;
	
	R_CDSP_RB_HSIZE = ((hsize*10)/8);
	R_CDSP_RB_VSIZE = ((vsize*10)/8);
}
/***************
clamp set
***************/
void hwCdsp_SetRBClamp(
	INT8U rbclampen,
	INT16U rbclamp
)
{
	R_CDSP_RB_CLAMP_CTRL = ((INT32U)rbclampen << 8) | rbclamp;
}

void hwCdsp_EnableClamp(
	INT8U clamphsizeen,
	INT16U Clamphsize
)
{
	/* Enable clamping */
	R_CDSP_CLAMP_SETTING = ((INT32U)clamphsizeen & 0x1) << 12 | Clamphsize; 
	
//	DBG_PRINT("<ImageOutput> clamphsizeen: %d, clamphsize: %d\r\n",clamphsizeen,Clamphsize);
}

void hwCdsp_SetUVScale(
	INT8U uvDiven,
	INT8U yfrcuvdiv1_8,
	INT8U yfrcuvdiv2_8,
	INT8U yfrcuvdiv3_8,
	INT8U yfrcuvdiv4_8,
	INT8U yfrcuvdiv5_8,
	INT8U yfrcuvdiv6_8
)
{
	if(uvDiven == 0)
	{
		R_CDSP_YUV_CTRL &= (~0x01);
	}
	else
	{
		R_CDSP_UVSCALE_COND1 = ((INT32U)yfrcuvdiv3_8 << 16) | 
								((INT32U)yfrcuvdiv2_8 << 8) | yfrcuvdiv1_8;
		R_CDSP_UVSCALE_COND0 = ((INT32U)yfrcuvdiv6_8 << 16) |
								((INT32U)yfrcuvdiv5_8 << 8) | yfrcuvdiv4_8;
		R_CDSP_YUV_CTRL |=  uvDiven & 0x01;		
	}
}

void 
hwCdsp_QcntThr(
	INT16U Qthr,
	INT16U PreRBclamp
)
{
	R_CDSP_INP_QTHR =  (PreRBclamp & 0xFF) << 8 | Qthr & 0xFF;
}

/********************
bad pixel set
*********************/
void 
hwCdsp_EnableBadPixel(
	INT8U badpixen, 
	INT8U badpixmiren, 
	INT8U badpixmirlen
)
{
	R_CDSP_BADPIX_CTRL = (badpixmirlen & 0x1) | ((badpixmiren & 0x01) << 1) | (badpixen & 0x01) << 3;
}

void hwCdsp_SetBadPixel(
	INT8U bprthr,
	INT8U bpgthr,
	INT8U bpbthr
)
{	
	R_CDSP_BADPIX_CTHR = bprthr & 0xFF |
					((INT32U)bpgthr & 0xFF) << 8 |
					((INT32U)bpbthr & 0xFF) << 16;
}

/********************
optical black set
*********************/
void
hwCdsp_SetManuOB(
	INT8U manuoben,
	INT16U manuob
)
{
	R_CDSP_OPB_CTRL = manuob & 0x7FF | 
				((INT32U)manuoben & 0x1) << 15;	
}

void
hwCdsp_SetAutoOB(
	INT8U autooben,
	INT8U obtype,
	INT8U curfob
)
{
	R_CDSP_OPB_TYPE = obtype & 0x7 | ((autooben & 0x1) << 3) | ((curfob & 0x1) << 4); 	
}


void hwCdsp_SetOB_HVoffset(
	INT16U obHOffset,
	INT16U obVOffset
)
{
	R_CDSP_OPB_HOFFSET = obHOffset & 0xFFF;
	R_CDSP_OPB_VOFFSET = obVOffset & 0xFFF;
}


void hwCdsp_SetAverageOB(
	INT8U wboffseten,
	INT16U ob_avg_r,
	INT16U ob_avg_gr,
	INT16U ob_avg_b,
	INT16U ob_avg_gb
)
{
	R_CDSP_OPB_RAVG = ob_avg_r & 0x3ff;
	R_CDSP_OPB_GRAVG = ob_avg_gr & 0x3ff;
	R_CDSP_OPB_BAVG = ob_avg_b & 0x3ff;
	R_CDSP_OPB_GBAVG = ob_avg_gb & 0x3ff;
	
	if (wboffseten){
		R_CDSP_YUVSPEC_MODE |= (wboffseten & 0x1) << 6;
	}else{
		R_CDSP_YUVSPEC_MODE &= ~(wboffseten & 0x1) << 6;
	}
}


void
hwCdsp_OBRead(
	INT32U *prvalue,
	INT32U *pgrvalue,
	INT32U *pbvalue,
	INT32U *pgbvalue
)
{
}

/********************
lens compensation
*********************/
void hwCdsp_InitLensCmp(INT16U *plensdata)
{
	INT32U i;
	cdspLensData_t *pLensData = (cdspLensData_t *)(CDSP_TABLE_BASEADDR);

	R_CDSP_MACRO_CTRL = 0x0101;	
	for(i=0; i<256; i++)
		pLensData->LensTable[i] = *plensdata++;

	R_CDSP_MACRO_CTRL = 0x00; /* Select None RAM */
}

void
hwCdsp_EnableLensCmp(
	INT8U lcen,
	INT8U stepfactor
)
{
	R_CDSP_LENS_CMP_CTRL &= ~0x17;
	R_CDSP_LENS_CMP_CTRL = (stepfactor & 0x07) | (lcen & 0x1) << 4; 				
}

void
hwCdsp_SetLensCmp(
	INT16U centx,
	INT16U centy,
	INT16U xmoffset,
	INT8U  xminc,
	INT16U ymoffset,
	INT8U  ymoinc,
	INT8U  ymeinc
)
{
	R_CDSP_IM_XCENT = centx;
	R_CDSP_IM_YCENT = centy;	
	R_CDSP_LENS_CMP_XOFFSET_MAP = (((INT16U)xminc & 0x0F) << 12) | xmoffset;	
	R_CDSP_LENS_CMP_YOFFSET_MAP = ymoffset;
	R_CDSP_LENS_CMP_YINSTEP_MAP = (((INT16U)ymeinc & 0x0F) << 4) | (ymoinc & 0x0F);
}


/*********************************************
New ISP Macro Table 
**********************************************/
/*--- GP_ISP_LI  ---*/
void hwIsp_InitLiCor(INT8U *plicordata)
{
	INT32U i;
	ispLiCorData_t *pLicordata = (ispLiCorData_t *)(CDSP_TABLE_BASEADDR);

	R_CDSP_MACRO_CTRL = 0x1414;	
	for(i=0; i<48; i++)
		pLicordata->LiCorTable[i] = *plicordata++;

	R_CDSP_MACRO_CTRL = 0x00; /* Select None RAM */
}

/*--- GP_ISP_hr_Table ---*/
void hwIsp_Hr0(INT32U value)	//1280*24
{
	INT32S i;

	R_CDSP_MACRO_CTRL = 0x1515;
	for(i=0; i<512; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x5515;
	for(i=0; i<512; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x9515;
	for(i=0; i<256; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x00;
}

void hwIsp_Hr1(INT32U value)	//1280*24
{
	INT32S i;

	R_CDSP_MACRO_CTRL = 0x1616;
	for(i=0; i<512; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x5616;
	for(i=0; i<512; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x9616;
	for(i=0; i<256; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x00;
}

void hwIsp_Hr2(INT32U value)	//1280*24
{
	INT32S i;

	R_CDSP_MACRO_CTRL = 0x1717;
	for(i=0; i<512; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x5717;
	for(i=0; i<512; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x9717;
	for(i=0; i<256; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x00;
}

void hwIsp_Hr3(INT32U value)	//1280*24
{
	INT32S i;

	R_CDSP_MACRO_CTRL = 0x1818;
	for(i=0; i<512; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x5818;
	for(i=0; i<512; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x9818;
	for(i=0; i<256; i++)
	{
		WRITE32(CDSP_TABLE_BASEADDR+i*4) = value;
	}	
	
	R_CDSP_MACRO_CTRL = 0x00;
}


/*--- GP_ISP_luc_MaxTan8_Slop_CLP_Table ---*/

//("../model/MaxTan8_File.txt","../model/slope4_File.txt","../model/CLPoint_File.txt");
void hwIsp_luc_MaxTan8_Slop_CLP(INT16U *pmaxtan8data, INT16U *pslopdata, INT16U *pclpdata)
{
	INT32U i;
	ispLucData_t *pLucdata = (ispLucData_t *)(CDSP_TABLE_BASEADDR);

	R_CDSP_MACRO_CTRL = 0x1C1C;	
	for(i=0; i<32; i++)
		pLucdata->MaxTan8[i] = *pmaxtan8data++;

	for(i=0; i<16; i++)
		pLucdata->Slope4[i] = *pslopdata++;

	for(i=0; i<8; i++)
		pLucdata->CLPoint[i] = *pclpdata++;


	R_CDSP_MACRO_CTRL = 0x00; /* Select None RAM */
}

/*---- GP_ISP_Luc_Radius_Table ---*/
void hwIsp_RadusF0(INT16U *pradusf0data)
{
	INT32U i;
	ispRadiusFile0_t *pRadusF0data = (ispRadiusFile0_t *)(CDSP_TABLE_BASEADDR);

	R_CDSP_MACRO_CTRL = 0x1919;	
	for(i=0; i<512; i++)
		pRadusF0data->Radius_File_0[i] = *pradusf0data++;

	R_CDSP_MACRO_CTRL = 0x00; /* Select None RAM */
}

void hwIsp_RadusF1(INT16U *pradusf1data)
{
	INT32U i;
	ispRadiusFile1_t *pRadusF1data = (ispRadiusFile1_t *)(CDSP_TABLE_BASEADDR);

	R_CDSP_MACRO_CTRL = 0x1A1A;	
	for(i=0; i<512; i++)
		pRadusF1data->Radius_File_1[i] = *pradusf1data++;

	R_CDSP_MACRO_CTRL = 0x00; /* Select None RAM */
}

void hwIsp_LenCmp(INT32U lcen,INT32U stepfactor)
{
	R_ISP_HR_LUT_CTRL &= ~0xFF;
	R_ISP_HR_LUT_CTRL = (stepfactor & 0x1F) | (lcen & 0x1) << 7; 				

}

/***************
raw h scaler
***************/
void
hwCdsp_EnableRawHScale(
	INT8U hscale_en,
	INT8U hscale_mode //0: drop, 1: filter
)
{
	R_CDSP_SCLDW_CTRL &= ~0x09;
	R_CDSP_SCLDW_CTRL |= hscale_mode & 0x1 | (hscale_en & 0x1) << 3;
}

void
hwCdsp_SetRawHScale(
	INT32U src_hsize,
	INT32U dst_hsize
)
{
	INT32U factor;

	if (dst_hsize >= src_hsize) 
	{
		R_CDSP_HRAW_SCLDW_FACTOR = 0x0000;
	}
	else 
	{
		factor = (dst_hsize << 16) / (src_hsize) + 1;
		R_CDSP_HSCALE_EVEN_PVAL = (factor/2)+0x8000;
		R_CDSP_HSCALE_ODD_PVAL = factor;
		R_CDSP_HRAW_SCLDW_FACTOR = factor;
	}
	//reflected at next vaild vd edge
	//R_SCL_FACTOR_CTRL |= 0x01; 	
}

/***************************
white balance set
****************************/
void hwCdsp_EnableWbGain(
	//INT8U wboffseten,
	INT8U wbgainen
)
{
	R_CDSP_YUVSPEC_MODE &= ~0x80;
	R_CDSP_YUVSPEC_MODE |= (wbgainen & 0x1) << 7;	//(wboffseten & 0x1) << 6 |
					
	//reflected at vd update
	//R_YUVSPEC_MODE |= 0x10; 
}

void hwCdsp_SetWbOffset(
	INT8U wboffseten,	
	INT8U roffset,
	INT8U groffset,
	INT8U boffset,
	INT8U gboffset
)
{
	R_CDSP_WHBAL_RSETTING &= ~(0xFF << 12);
	R_CDSP_WHBAL_RSETTING |= ((INT32U)roffset & 0xFF) << 12;
	R_CDSP_WHBAL_GRSETTING &= ~(0xFF << 12);
	R_CDSP_WHBAL_GRSETTING |= ((INT32U)groffset & 0xFF) << 12;
	R_CDSP_WHBAL_BSETTING &= ~(0xFF << 12);
	R_CDSP_WHBAL_BSETTING |= ((INT32U)boffset & 0xFF) << 12;
	R_CDSP_WHBAL_GBSETTING &= ~(0xFF << 12);
	R_CDSP_WHBAL_GBSETTING |= ((INT32U)gboffset & 0xFF) << 12;
	
	if (wboffseten){
		R_CDSP_YUVSPEC_MODE |= (wboffseten & 0x1) << 6;
	}else{
		R_CDSP_YUVSPEC_MODE &= ~(wboffseten & 0x1) << 6;
	}
}

void hwCdsp_GetWbOffset(
	INT8U *pwboffseten,
	INT8S *proffset,
	INT8S *pgroffset,
	INT8S *pboffset,
	INT8S *pgboffset
)
{
	*pwboffseten = (R_CDSP_YUVSPEC_MODE >> 6) & 0x1;
	*proffset = (R_CDSP_WHBAL_RSETTING >> 12) & 0xFF;
	*pgroffset =(R_CDSP_WHBAL_GRSETTING >> 12) & 0xFF;
	*pboffset = (R_CDSP_WHBAL_BSETTING >> 12) & 0xFF;
	*pgboffset = (R_CDSP_WHBAL_GBSETTING >> 12) & 0xFF;
}

void hwCdsp_SetWbGain(
	INT16U r_gain,
	INT16U gr_gain,
	INT16U b_gain,
	INT16U gb_gain
)
{
	R_CDSP_WHBAL_RSETTING &= ~0x1FF;
	R_CDSP_WHBAL_RSETTING |= r_gain & 0x1FF;
	R_CDSP_WHBAL_GRSETTING &= ~0x1FF;
	R_CDSP_WHBAL_GRSETTING |= gr_gain & 0x1FF;
	R_CDSP_WHBAL_BSETTING &= ~0x1FF;
	R_CDSP_WHBAL_BSETTING |= b_gain & 0x1FF;
	R_CDSP_WHBAL_GBSETTING &= ~0x1FF;
	R_CDSP_WHBAL_GBSETTING |= gb_gain & 0x1FF;
}

void hwCdsp_GetWbGain(
	INT8U *pwbgainen,
	INT16U *prgain,
	INT16U *pgrgain,
	INT16U *pbgain,
	INT16U *pgbgain
)
{
	*pwbgainen = (R_CDSP_YUVSPEC_MODE >> 7) & 0x01;
	*prgain = R_CDSP_WHBAL_RSETTING & 0x1FF;
	*pgrgain = R_CDSP_WHBAL_GRSETTING & 0x1FF;
	*pbgain = R_CDSP_WHBAL_BSETTING & 0x1FF;
	*pgbgain = R_CDSP_WHBAL_GBSETTING & 0x1FF;
}

void hwCdsp_SetWb_R_B_Gain(
	INT16U r_gain,
	INT16U b_gain
)
{
	R_CDSP_WHBAL_RSETTING &= ~0x1FF;
	R_CDSP_WHBAL_RSETTING |= r_gain & 0x1FF;
	R_CDSP_WHBAL_BSETTING &= ~0x1FF;
	R_CDSP_WHBAL_BSETTING |= b_gain & 0x1FF;
}

/**********
global gain
***********/
void hwCdsp_SetGlobalGain(INT8U global_gain)
{
	R_CDSP_GLOBAL_GAIN = global_gain & 0xFF;
}

void hwCdsp_GlobalGainRead(INT8U *pglobal_gain)
{
	*pglobal_gain = R_CDSP_GLOBAL_GAIN;
}

INT8U hwCdsp_GetGlobalGain(void)
{
	return (R_CDSP_GLOBAL_GAIN & 0xFF);
}

/****************
gain2
*****************/
void hwCdsp_EnableWbGain2(
	INT8U wbgain2en
)
{
	//vaild when input format is raw.
	R_CDSP_AWB_WIN_BGAIN2 &= ~0x800;
	R_CDSP_AWB_WIN_BGAIN2 |= ((INT32U)wbgain2en & 0x1) << 11;
}

void hwCdsp_WbGain2Set(
	INT16U rgain2,
	INT16U ggain2,
	INT16U bgain2
)
{
	//vaild when input format is raw.
	R_CDSP_AWB_WIN_RGGAIN2 = rgain2 & 0x1FF | ((INT32U)ggain2 & 0x1FF) << 12;	
	R_CDSP_AWB_WIN_BGAIN2 &= ~0x1FF;
	R_CDSP_AWB_WIN_BGAIN2 |= bgain2 & 0x1FF;
}

void hwCdsp_WbGain2Read(
	INT16U *prgain,
	INT16U *pggain,
	INT16U *pbgain
)
{
	*prgain = R_CDSP_AWB_WIN_RGGAIN2 & 0x1FF;
	*pggain = (R_CDSP_AWB_WIN_RGGAIN2>>12) & 0x1FF;
	*prgain = R_CDSP_AWB_WIN_BGAIN2 & 0x1FF;
}

void hwCdsp_EnableWbGain2_Read(INT8U *pgain2en)
{
	*pgain2en = R_CDSP_AWB_WIN_RGGAIN2 >>11 & 0x1;
}
/*********
gamma set
**********/
void hwCdsp_InitGamma(INT32U *pGammaTable)
{
	INT16U i;
	cdspGammaData_t *pGammaData = (cdspGammaData_t *)(CDSP_TABLE_BASEADDR);
	
	R_CDSP_MACRO_CTRL = 0x0202;
	for(i=0; i<128; i++) 
		pGammaData->GammaTable[i] = *pGammaTable++;
	
	R_CDSP_MACRO_CTRL = 0;
}

void R_hwCdsp_InitGamma( INT32U *pGammaTable)
{
	INT16U i;
	cdspGammaData_t *pGammaData = (cdspGammaData_t *)(CDSP_TABLE_BASEADDR);
	
	(*(volatile INT32U *)(CDSP_S511_BASE+0x000)) = 0x0202;
	for(i=0; i<128; i++) 
		pGammaData->GammaTable[i] = *pGammaTable++;
	
	R_CDSP_MACRO_CTRL = 0;
}

void hwCdsp_EnableLutGamma(INT8U lut_gamma_en)
{
	if(lut_gamma_en)
		R_CDSP_YUVSPEC_MODE |= 0x100;
	else
		R_CDSP_YUVSPEC_MODE &= ~0x100;
}

/***************
interpolation set
***************/
void hwCdsp_EnableIntplMir(
	INT8U intplmirhlen,
	INT8U intplmirvsel,	//suggest set 1
	INT8U intplcnt2sel
)
{
	R_CDSP_INP_MIRROR_CTRL &= ~0x9F;
	R_CDSP_INP_MIRROR_CTRL |= (intplcnt2sel & 0x1) << 7 |
						(intplmirvsel & 0x1) << 4 |
						intplmirhlen & 0x0F;
						
	//DBG_PRINT("<Intpl> intpllowthr|intplmirhren|intplmirvten|intplmirvden = 0x%x\r\n",intplmirhlen);	
	//DBG_PRINT("<Intpl> intplmirvsel = %d, intplcnt2sel = %d\r\n",intplmirvsel,intplcnt2sel);	
}

void hwCdsp_IntplThrSet(
	INT8U int_low_thr,
	INT8U int_hi_thr
)
{
	R_CDSP_INP_DENOISE_THR = ((INT16U)int_hi_thr << 8) | int_low_thr;

	//DBG_PRINT("<Intpl> intplmirhlen = %d, intplhithr = %d\r\n",int_low_thr,int_hi_thr);
}

/**********************
edge in intpolation set
**********************/
void hwCdsp_EnableEdge(INT8U edgeen)
{
	if(edgeen)
		R_CDSP_INP_EDGE_CTRL |= 0x01;
	else
		R_CDSP_INP_EDGE_CTRL &= ~0x01;
}

INT8U gpHalCdspGetEdgeEn(void)
{
	return (R_CDSP_INP_EDGE_CTRL & 0x01);
}


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
)
{
	//vaild when R_INP_EDGE_CTRL.bit0 = 1
	R_CDSP_HPF_LCOEF0 = (lf02 & 0x0F) << 8 | (lf01 & 0x0F) << 4 | (lf00 & 0x0F);
	R_CDSP_HPF_LCOEF1 = (lf12 & 0x0F) << 8 | (lf11 & 0x0F) << 4 | (lf10 & 0x0F);
	R_CDSP_HPF_LCOEF2 = (lf22 & 0x0F) << 8 | (lf21 & 0x0F) << 4 | (lf20 & 0x0F);
}

void gpHalCdspSetEdgeFilter(
	INT8U index,
	INT8U L0,
	INT8U L1,
	INT8U L2
)
{
	//vaild when R_INP_EDGE_CTRL.bit0 = 1
	if (index == 0) {
		R_CDSP_HPF_LCOEF0 = (L2 & 0x0F) << 8 | (L1 & 0x0F) << 4 | (L0 & 0x0F);
	}else if (index == 1){
		R_CDSP_HPF_LCOEF1 = (L2 & 0x0F) << 8 | (L1 & 0x0F) << 4 | (L0 & 0x0F);
	}else {
		R_CDSP_HPF_LCOEF2 = (L2 & 0x0F) << 8 | (L1 & 0x0F) << 4 | (L0 & 0x0F);
	}
	//R_CDSP_HPF_LCOEF1 = (lf12 & 0x0F) << 8 | (lf11 & 0x0F) << 4 | (lf10 & 0x0F);
	//R_CDSP_HPF_LCOEF2 = (lf22 & 0x0F) << 8 | (lf21 & 0x0F) << 4 | (lf20 & 0x0F);
}


void gpHalCdspGetEdgeFilter(
	INT8U index,
	INT8U *L0,
	INT8U *L1,
	INT8U *L2
)
{
	if(index == 0) {
		*L0 = R_CDSP_HPF_LCOEF0 & 0x0F;
		*L1 = (R_CDSP_HPF_LCOEF0 >> 4) & 0x0F;
		*L2 = (R_CDSP_HPF_LCOEF0 >> 8) & 0x0F;
	} else if(index == 1) {
		*L0 = R_CDSP_HPF_LCOEF1 & 0x0F;
		*L1 = (R_CDSP_HPF_LCOEF1 >> 4) & 0x0F;
		*L2 = (R_CDSP_HPF_LCOEF1 >> 8) & 0x0F;
	} else {
		*L0 = R_CDSP_HPF_LCOEF2 & 0x0F;
		*L1 = (R_CDSP_HPF_LCOEF2 >> 4) & 0x0F;
		*L2 = (R_CDSP_HPF_LCOEF2 >> 8) & 0x0F;
	}
}


void hwCdsp_SetEdgeLCoring(
	INT8U lhdiv,
	INT8U lhtdiv,
	INT8U lhcoring,
	INT8U lhmode
)
{
	R_CDSP_LH_DIV_CTRL = lhdiv & 0x07 | 
					(lhtdiv& 0x07) << 4 |
					((INT32U)lhcoring & 0x1FF) << 8 |
					((INT32U)lhmode & 0x1) << 16; 
}

void gpHalCdspGetEdgeLCoring(
	INT8U *lhdiv, 
	INT8U *lhtdiv, 
	INT8U *lhcoring, 
	INT8U *lhmode
)
{
	*lhdiv = R_CDSP_LH_DIV_CTRL & 0x07;
	*lhtdiv = (R_CDSP_LH_DIV_CTRL >> 4) & 0x07; 
	*lhcoring = (R_CDSP_LH_DIV_CTRL >> 8) & 0x1FF;
	*lhmode = (R_CDSP_LH_DIV_CTRL >> 16) & 0x01;
}


void hwCdsp_SetEdgeAmpga(
	INT8U ampga,
	INT8U edgedomain
)
{
	R_CDSP_INP_EDGE_CTRL &= ~0xC4;
	R_CDSP_INP_EDGE_CTRL |= (edgedomain & 0x1) << 2 | (ampga & 0x03) << 6 ;
}

INT8U gpHalCdspGetEdgeAmpga(void)
{
	return (R_CDSP_INP_EDGE_CTRL >>6) & 0x3;
}

INT8U gpHalCdspGetEdgeDomain(void)
{
	return (R_CDSP_INP_EDGE_CTRL >> 2) & 0x1;
}

/**
* @brief	cdsp edge Q threshold set
* @param	Qthr [in]: edge threshold 
* @return	none
*/
void gpHalCdspSetEdgeQthr(INT8U Qthr)
{
	R_CDSP_INP_QTHR &= ~0xFF;
	R_CDSP_INP_QTHR |= (Qthr & 0xFF);
}

INT32U gpHalCdspGetEdgeQCnt(void)
{
	return R_CDSP_INP_QCNT;
}
/**********************
edge lut table set
**********************/
void hwCdsp_InitEdgeLut(
	INT8U *pLutEdgeTable
)
{
	INT32S i;
	cdspEdgeLutData_t *pEdgeLutData = (cdspEdgeLutData_t *)(CDSP_TABLE_BASEADDR);
	R_CDSP_MACRO_CTRL = 0x0404;
	for(i = 0; i < 256; i++) 
		pEdgeLutData->EdgeTable[i] = *pLutEdgeTable++;
		
	R_CDSP_MACRO_CTRL = 0x00;
}

void hwCdsp_EnableEdgeLutTable(
	INT8U eluten
)
{
	if(eluten)
		R_CDSP_INP_EDGE_CTRL |= 0x10;
	else
		R_CDSP_INP_EDGE_CTRL &= ~0x10;
}

INT8U gpHalCdspGetEdgeLutTableEn(void)
{
	return (R_CDSP_INP_EDGE_CTRL >> 4) & 0x01;
}

/***************
color matrix set
***************/
void hwCdsp_EnableColorMatrix(
	INT8U colcorren
)
{
	if(colcorren){
		R_CDSP_CC_COF4 |= 0x800;
	}	
	else{	
		R_CDSP_CC_COF4 &= ~0x800;
	}
	//DBG_PRINT("<ColCorr> colcorren:%d\r\n",colcorren);
}

void hwCdsp_SetColorMatrix(
	INT32S a11,
	INT32S a12,
	INT32S a13,
	INT32S a21,
	INT32S a22,
	INT32S a23,
	INT32S a31,
	INT32S a32,
	INT32S a33
)
{
	a11 &= 0x3FF;
	a12 &= 0x3FF;
	a13 &= 0x3FF;
	a21 &= 0x3FF;
	a22 &= 0x3FF;
	a23 &= 0x3FF;
	a31 &= 0x3FF;
	a32 &= 0x3FF;
	a33 &= 0x3FF;
	
	R_CDSP_CC_COF0 = (a12 << 12) | a11;
	R_CDSP_CC_COF1 = (a21 << 12) | a13;
	R_CDSP_CC_COF2 = (a23 << 12) | a22;
	R_CDSP_CC_COF3 = (a32 << 12) | a31;
	R_CDSP_CC_COF4 &= ~0x3FF;
	R_CDSP_CC_COF4 |= a33;
}

void hwCdsp_SetColorMatrix_Str(const INT16S *MatrixTable)
{   
	INT16S a11,a12,a13,a21,a22,a23,a31,a32,a33;
	//INT32U *p_MatrixTable;
	
	//p_MatrixTable = MatrixTable;
	//tmp = MatrixTable[0];
	a11 = MatrixTable[0] & 0x3FF;
	a12 = MatrixTable[1] & 0x3FF;
	a13 = MatrixTable[2] & 0x3FF;
	a21 = MatrixTable[3] & 0x3FF;
	a22 = MatrixTable[4] & 0x3FF;
	a23 = MatrixTable[5] & 0x3FF;
	a31 = MatrixTable[6] & 0x3FF;
	a32 = MatrixTable[7] & 0x3FF;
	a33 = MatrixTable[8] & 0x3FF;

	R_CDSP_CC_COF0 = (a12 << 12) | a11;
	R_CDSP_CC_COF1 = (a21 << 12) | a13;
	R_CDSP_CC_COF2 = (a23 << 12) | a22;
	R_CDSP_CC_COF3 = (a32 << 12) | a31;
	R_CDSP_CC_COF4 &= ~0x3FF;
	R_CDSP_CC_COF4 |= a33;
}

void gpHalCdspGetColorMatrix(INT8U index, INT16S *A1, INT16S *A2, INT16S *A3)
{	
	INT32U reg;
	
	if(index == 0)
	{
		reg = R_CDSP_CC_COF0;
		*A1 = reg & 0x3FF;
		*A2 = (reg >> 12) & 0x3FF;
		reg = R_CDSP_CC_COF1;
		*A3 = reg & 0x3FF;		
	}
	else if(index == 1)
	{
		reg = R_CDSP_CC_COF1;
		*A1 = (reg >> 12) & 0x3FF;
		reg = R_CDSP_CC_COF2;
		*A2 = reg & 0x3FF;
		*A3 = (reg >> 12) & 0x3FF;
	}
	else 
	{
		reg = R_CDSP_CC_COF3;
		*A1 = reg & 0x3FF;
		*A2 = (reg >> 12) & 0x3FF;
		reg = R_CDSP_CC_COF4; 
		*A3 = reg & 0x3FF;
	}
}

void gpCdspGetColorMatrix(gpCdspCorMatrix_t *argp)
{
	gpHalCdspGetColorMatrix(0, &argp->a11, &argp->a12, &argp->a13);
	gpHalCdspGetColorMatrix(1, &argp->a21, &argp->a22, &argp->a23);
	gpHalCdspGetColorMatrix(2, &argp->a31, &argp->a32, &argp->a33);
	argp->colcorren = (R_CDSP_CC_COF4 >> 11 & 0x1);
}

/*******************
raw special mode set
*******************/
void hwCdsp_SetRawSpecMode(
	INT8U rawspecmode
)
{
	if(rawspecmode > 6)
		return;
	
	R_CDSP_RGB_SPEC_ROT_MODE &= ~0x0F;
	R_CDSP_RGB_SPEC_ROT_MODE |= rawspecmode & 0x07;	
	//reflected at vd ypdate 
	//R_RGB_SPEC_ROT_MODE |= 0x08;
}

/****************
ratate mode set //GPCV1248 remove 
*****************/
void hwCdsp_RotateModeSet(
	INT8U rotmode
)
{	//not support
	//R_RGB_SPEC_ROT_MODE &= ~0xF0;
	//R_RGB_SPEC_ROT_MODE |= (rotmode & 0x07) << 4;;
	//reflected at vd ypdate 
	//R_RGB_SPEC_ROT_MODE |= 0x80;
}

/**********************
YUV insert & coring set
**********************/
void hwCdsp_EnableYuv444Insert(
	INT8U yuvinserten
)
{
	if(yuvinserten)
		R_CDSP_YUV_CTRL |= 0x08;
	else
		R_CDSP_YUV_CTRL &= ~0x08;
}

void hwCdsp_SetYuvCoring(
	INT8U y_corval_coring,
	INT8U u_corval_coring,
	INT8U v_corval_coring 
)
{
	R_CDSP_YUV_CORING_SETTING = y_corval_coring & 0xFF |
						(u_corval_coring & 0xFF) << 8 | 
						(v_corval_coring & 0xFF) << 16;	 							
}

/********
crop set
*********/
void hwCdsp_SetCrop(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
)
{
	R_CDSP_IMCROP_HOFFSET = hoffset;
	R_CDSP_IMCROP_VOFFSET = voffset;
	R_CDSP_IMCROP_HSIZE = hsize;
	R_CDSP_IMCROP_VSIZE = vsize;
}

void hwCdsp_EnableCrop(
	INT8U hvEnable
)
{
	if(hvEnable) hvEnable = 0x3;

	if(R_CDSP_IMCROP_HOFFSET == 0)
		hvEnable &= ~0x01;
	
	if(R_CDSP_IMCROP_VOFFSET == 0)
		hvEnable &= ~0x02;

	R_CDSP_IMCROP_CTRL = hvEnable; 
	//reflected at vd update
	//R_IMCROP_CTRL |= 0x10;
	//crop sync zoomfactor
	//R_IMCROP_CTRL |= 0x20;
}

void hwCdsp_CropGet(
	INT32U *hoffset,
	INT32U *voffset,
	INT32U *hsize,
	INT32U *vsize
)
{   
	*hoffset = R_CDSP_IMCROP_HOFFSET;
	*voffset = R_CDSP_IMCROP_VOFFSET;
	*hsize = R_CDSP_IMCROP_HSIZE;
	*vsize = R_CDSP_IMCROP_VSIZE;
}

/***************
YUV havg set
***************/
void hwCdsp_SetYuvHAvg(
	INT8U yuvhavgmiren,
	INT8U ytype,
	INT8U utype,
	INT8U vtype
)
{
	R_CDSP_YUV_AVG_LPF_TYPE = ytype & 0x03 |
						(utype & 0x03) << 2 |
						(vtype & 0x03) << 4 |	
						((INT16U)yuvhavgmiren & 0x03) << 8;
	
	//DBG_PRINT("<YUVhavg>yavgtype = %d, uavgtype = %d, vavgtype = %d, yuvhavgmirlen|yuvhavgmirren\r\n",ytype,utype,vtype,yuvhavgmiren);
}

/*******************
YUV special mode set
*******************/
void hwCdsp_SetYuvSpecMode(
	INT8U yuvspecmode
)
{
	R_CDSP_YUVSPEC_MODE &= ~0x07;
	R_CDSP_YUVSPEC_MODE |= yuvspecmode & 0x07;	
}

void hwCdsp_SetYuvSpecModeBinThr(
	INT8U binarthr
)
{
	//vaild when special mode = 2, (binarize)
	R_CDSP_YUVSP_EFFECT_BTHR = binarthr;	
}

/****************************
Y brightness & contrast set
*****************************/
void hwCdsp_BriContSet(
	INT32U yb,
	INT32U yc
)
{
	R_CDSP_YUVSP_EFFECT_SCALER &= ~0xFF;
	R_CDSP_YUVSP_EFFECT_SCALER |= yc & 0xFF;
	R_CDSP_YUVSP_EFFECT_OFFSET &= ~0xFF;
	R_CDSP_YUVSP_EFFECT_OFFSET |= yb & 0xFF;
}

void hwCdsp_EnableBriCont(
	INT8U YbYcEn
)
{
	// vaild when yuv special mode = 0x3
	if(YbYcEn) 
		R_CDSP_YUV_RANGE |= 0x10; //enable y brightness and contrast adj
	else
		R_CDSP_YUV_RANGE &= ~0x10;	
}


INT8U gpHalCdspGetBriContEn(void)
{
	return (R_CDSP_YUV_RANGE >> 4) & 0x01;
}

void hwCdsp_SetYuvSPEffOffset(
	INT8S y_offset,
	INT8S u_offset,
	INT8S v_offset
)
{
	R_CDSP_YUVSP_EFFECT_OFFSET = ((INT32U)y_offset & 0xFF) | 
							((INT32U)u_offset & 0xFF) << 8 |
							((INT32U)v_offset & 0xFF) << 16;
}

void gpHalCdspGetYuvSPEffOffset(
	INT8S *y_offset, 
	INT8S *u_offset, 
	INT8S *v_offset
)
{
	*y_offset = R_CDSP_YUVSP_EFFECT_OFFSET & 0xFF;
	*u_offset = (R_CDSP_YUVSP_EFFECT_OFFSET >> 8) & 0xFF;
	*v_offset = (R_CDSP_YUVSP_EFFECT_OFFSET >> 16) & 0xFF;
}

void hwCdsp_SetYuvSPEffScale(
	INT8U y_scale,
	INT8U u_scale,
	INT8U v_scale
)
{
	
	R_CDSP_YUVSP_EFFECT_SCALER =((INT32U) y_scale & 0xFF) |
							((INT32U)u_scale & 0xFF) << 8 |
							((INT32U)v_scale & 0xFF) << 16;
}


void gpHalCdspGetYuvSPEffScale(
	INT8U *y_scale, 
	INT8U *u_scale, 
	INT8U *v_scale
)
{
	*y_scale = R_CDSP_YUVSP_EFFECT_SCALER;
	*u_scale = (R_CDSP_YUVSP_EFFECT_SCALER >> 8) & 0xFF;
	*v_scale = (R_CDSP_YUVSP_EFFECT_SCALER >> 16) & 0xFF;
}
/***************************
UV saturation & hue set
*****************************/
void hwCdsp_SatHueSet(
	INT32U uv_sat_scale,
	INT32U uoffset,
	INT32U voffset,
	INT32U u_huesindata,
	INT32U u_huecosdata,
	INT32U v_huesindata,
	INT32U v_huecosdata
)
{	
	R_CDSP_HUE_ROT_U = (u_huecosdata << 8) | u_huesindata;
	R_CDSP_HUE_ROT_V = (v_huecosdata << 8) | v_huesindata;
	R_CDSP_YUVSP_EFFECT_SCALER &= ~0xFFFF00;
	R_CDSP_YUVSP_EFFECT_SCALER |= (uv_sat_scale << 16) | (uv_sat_scale << 8);
	R_CDSP_YUVSP_EFFECT_OFFSET &= ~0xFFFF00;
	R_CDSP_YUVSP_EFFECT_OFFSET |= (voffset << 16) | (uoffset << 8);
}

void hwCdsp_SetYuvSPHue(
	INT16U u_huesindata,
	INT16U u_huecosdata,
	INT16U v_huesindata,
	INT16U v_huecosdata
)
{
	R_CDSP_HUE_ROT_U = (u_huecosdata << 8) | u_huesindata;
	R_CDSP_HUE_ROT_V = (v_huecosdata << 8) | v_huesindata;
}

void gpHalCdspGetYuvSPHue(
	INT8S *u_huesindata, 
	INT8S *u_huecosdata,	
	INT8S *v_huesindata, 
	INT8S *v_huecosdata
)
{
	*u_huesindata = R_CDSP_HUE_ROT_U & 0xFF;
	*u_huecosdata = (R_CDSP_HUE_ROT_U >> 8) & 0xFF;
	*v_huesindata = R_CDSP_HUE_ROT_V & 0xFF;
	*v_huecosdata = (R_CDSP_HUE_ROT_V >> 8) & 0xFF;
}


/***************
yuv h & v scale
***************/
void hwCdsp_EnableYuvHScale(
	INT8U yuvhscale_en,
	INT8U yuvhscale_mode //0: drop, 1: filter
)
{
	R_CDSP_SCLDW_CTRL &= ~0x900;
	R_CDSP_SCLDW_CTRL |= ((INT32U)yuvhscale_mode & 0x1) << 8 |
					((INT32U)yuvhscale_en & 0x1) << 11;			
}

void hwCdsp_EnableYuvVScale(
	INT8U vscale_en,
	INT8U vscale_mode //0: drop, 1: filter
)
{
	R_CDSP_SCLDW_CTRL &= ~0x90;
	R_CDSP_SCLDW_CTRL |= ((INT32U)vscale_mode & 0x1) << 4 |
					((INT32U)vscale_en & 0x1) << 7;			
	//reflected at next vaild vd edge
	//R_SCL_FACTOR_CTRL = 0x01;      	 
}

void hwCdsp_SetYuvHScale(
	INT16U hscaleaccinit,
	INT16U yuvhscalefactor
)
{
	R_CDSP_HSCALE_ACC_INIT = hscaleaccinit;
	R_CDSP_HYUV_SCLDW_FACTOR = yuvhscalefactor;
}

void hwCdsp_SetYuvVScale(
	INT16U vscaleaccinit,
	INT16U yuvvscalefactor
)
{
	R_CDSP_VSCALE_ACC_INIT = vscaleaccinit;
	R_CDSP_VYUV_SCLDW_FACTOR = yuvvscalefactor;
}

/***************
Suppression set
***************/
void hwCdsp_EnableUvSuppr(
	INT8U suppressen
)
{
	if(suppressen)
		R_CDSP_YUV_CTRL |= 0x10;
	else
		R_CDSP_YUV_CTRL &= ~0x10;
}

void hwCdsp_SetUvSuppr(
	INT8U yuvsupmirvsel,
	INT8U fstextsolen,
	INT8U yuvsupmiren
)
{	
	R_CDSP_YUV_RANGE &= 0x20FF;
	R_CDSP_YUV_RANGE |= (yuvsupmiren & 0x0F) << 8 | 
				((INT16U)yuvsupmirvsel & 0x1) << 12 | 
				((INT16U)fstextsolen & 0x1) << 13;
}

/******************************
Y edge in uv suppression set
******************************/
/**
* @brief	cdsp edge source set
* @param	posyedgeen [in]: 0:raw, 1: YUV
* @return	none
*/
void hwCdsp_EnableYEdgeUvSuppr(
	INT8U posyedgeen
)
{
	if(posyedgeen)
		R_CDSP_INP_EDGE_CTRL |= 0x02;
	else
		R_CDSP_INP_EDGE_CTRL &= ~0x02;
}

INT32U gpHalCdspGetEdgeSrc(void)
{
	return ((R_CDSP_INP_EDGE_CTRL >> 1) & 0x01);
}


/******************************
Y denoise in suppression set
******************************/
void 
hwCdsp_EnableYDenoise(
	INT8U denoisen
)
{
	if(denoisen)
		R_CDSP_YUV_CTRL |= 0x20;
	else
		R_CDSP_YUV_CTRL &= ~0x20;
}

void
hwCdsp_SetYDenoise(
	INT8U denoisethrl,
	INT8U denoisethrwth,
	INT8U yhtdiv	
)
{
	R_CDSP_DENOISE_SETTING = ((INT16U)yhtdiv & 0x07) << 12 |
						((INT16U)denoisethrwth & 0x07) << 8 | 
						denoisethrl & 0xFF;
}

/***********************
Y LPF in suppression set
************************/
void 
hwCdsp_EnableYLPF(
	INT8U lowyen
)
{
	if(lowyen)
		R_CDSP_YUV_CTRL |= 0x40;
	else
		R_CDSP_YUV_CTRL &= ~0x40;
}

/***************************
auto focuse / af set
****************************/
void 
hwCdsp_EnableAF(
	INT8U af_en,
	INT8U af_win_hold
)
{
	R_CDSP_AF_WIN_CTRL &= ~0x90000;
	R_CDSP_AF_WIN_CTRL |= ((INT32U)af_win_hold & 0x1) << 16 |
					((INT32U)af_en & 0x1) << 19; 	
}

void
hwCdsp_SetAfWin1(
	INT16U hoffset,
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
)
{
	R_CDSP_AF_WIN1_HVOFFSET = (hoffset & 0xFFF) | ((INT32U)voffset & 0xFFF) << 12; 
	R_CDSP_AF_WIN1_HVSIZE = (hsize & 0xFFF) | ((INT32U)vsize & 0xFFF) << 12;
}

void
hwCdsp_SetAfWin2(
	INT16U hoffset,	
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
)
{
	// size 0: 256, 1:512, 2:1024, 3:64, 4:2048
	hoffset >>= 2;	// offset unit is 4 pixel
	voffset >>= 2;
	R_CDSP_AF_WIN2_HVOFFSET = (hoffset & 0x3FF) | ((INT32U)voffset & 0x3FF) << 12; 
	R_CDSP_AF_WIN_CTRL &= ~0xFF;
	R_CDSP_AF_WIN_CTRL |=  hsize & 0xF | (vsize & 0xF) << 4;
}

void
hwCdsp_SetAfWin3(
	INT16U hoffset,	
	INT16U voffset,
	INT16U hsize,
	INT16U vsize
)
{
	// size 0: 256, 1:512, 2:1024, 3:64, 4:2048
	hoffset >>= 2;	// offset unit is 4 pixel
	voffset >>= 2;
	R_CDSP_AF_WIN3_HVOFFSET = (hoffset & 0x3FF) | ((INT32U)voffset & 0x3FF) << 12; 
	R_CDSP_AF_WIN_CTRL &= ~0xFF00;
	R_CDSP_AF_WIN_CTRL |=  (hsize & 0xF) << 8 | (vsize & 0xF) << 12;
}

/***************************
auto white balance / awb set
****************************/
void
hwCdsp_EnableAWB(
	INT8U awb_win_en,
	INT8U awb_win_hold
)
{
	R_CDSP_AE_AWB_WIN_CTRL &= ~0x0804;
	R_CDSP_AE_AWB_WIN_CTRL |= (awb_win_hold & 0x1) << 2 | 
						((INT16U)awb_win_en& 0x1) << 11 ; 
}

void
hwCdsp_SetAWB(
	INT8U awbclamp_en,
	INT8U sindata,	
	INT8U cosdata,
	INT8U awbwinthr
)
{
	R_CDSP_AWB_WIN_CTRL = (sindata & 0xFF) |
					((INT32U)cosdata  & 0xFF)<< 8 |
					((INT32U)awbwinthr & 0xFF) << 16 |
					((INT32U)awbclamp_en & 0x1) << 24;
}

void hwCdsp_SetAWBThr(
	INT32U Ythr,
	INT32U UVL1N1,
	INT32U UVL1N2,
	INT32U UVL1N3	
)
{
	R_CDSP_AWB_SPECWIN_Y_THR = Ythr;
	R_CDSP_AWB_SPECWIN_UV_THR1 = UVL1N1;
	R_CDSP_AWB_SPECWIN_UV_THR2 = UVL1N2;
	R_CDSP_AWB_SPECWIN_UV_THR3 = UVL1N3;	
}

void hwCdsp_SetAWBYThr(	//lum0clamp,lum1clamp,lum2clamp,lum3clamp
	INT8U Ythr0,
	INT8U Ythr1,
	INT8U Ythr2,
	INT8U Ythr3	
)
{
	R_CDSP_AWB_SPECWIN_Y_THR = Ythr0 | 
							(Ythr1 << 8) | 
							(Ythr2 << 16)| 
							(Ythr3 << 24);
	}

void hwCdsp_SetAWBUVThr1(	//brl1n1, brl1p1, mgl1n1, mgl1p1
	INT16S UL1N1,
	INT16S UL1P1,
	INT16S VL1N1,	
	INT16S VL1P1	
)
{
	//UL1N1 >>= 1;		//填入時已經先除2
	//UL1P1 >>= 1;
	//VL1N1 >>= 1;
	//VL1P1 >>= 1;	
	
	R_CDSP_AWB_SPECWIN_UV_THR1 = (UL1N1 & 0xFF) | 
						((INT32S)UL1P1 & 0xFF)	<< 8 | 
						((INT32S)VL1N1 & 0xFF) << 16| 
						((INT32S)VL1P1 & 0xFF) << 24;
	
}

void hwCdsp_SetAWBUVThr2(	//brl1n2, brl1p2, mgl1n2, mgl1p2
	INT16S UL1N2,
	INT16S UL1P2,
	INT16S VL1N2,	
	INT16S VL1P2	
)
{
	//UL1N2 >>= 1;
	//UL1P2 >>= 1;
	//VL1N2 >>= 1;
	//VL1P2 >>= 1;
	
	R_CDSP_AWB_SPECWIN_UV_THR2 = (UL1N2 & 0xFF) | 
						((INT32S)UL1P2 & 0xFF)	<< 8 | 
						((INT32S)VL1N2 & 0xFF) << 16| 
						((INT32S)VL1P2 & 0xFF) << 24;
}

void hwCdsp_SetAWBUVThr3(	//brl1n3, brl1p3, mgl1n3, mgl1p3
	INT16S UL1N3,
	INT16S UL1P3,
	INT16S VL1N3,	
	INT16S VL1P3	
)
{
	//UL1N3 >>= 1;
	//UL1P3 >>= 1;
	//VL1N3 >>= 1;
	//VL1P3 >>= 1;
		
	R_CDSP_AWB_SPECWIN_UV_THR3 = (UL1N3 & 0xFF) | 
						((INT32S)UL1P3 & 0xFF)	<< 8 | 
						((INT32S)VL1N3 & 0xFF) << 16| 
						((INT32S)VL1P3 & 0xFF) << 24;
}

void hwCdsp_SetIntEn(INT8U enable, INT8U bit)
{
	if(enable)
		R_CDSP_INTEN |= bit;
	else
		R_CDSP_INTEN &= ~bit;
}

/***************************
auto expore / ae set
****************************/
void hwCdsp_SetAEAWB(
	INT8U raw_en,	//1:raw(from awblinectr), 0:rgb(poswb) ae/awb window
	INT8U subample 	//0, disable, 1: 1/2, 2: 1/4 subsample
)
{
	/*
	if (raw_en == 1){
		if(subample == 2)
			subample = 1;
		else if(subample == 4)
			subample = 2;
		else if(subample == 8)
			subample = 3;
		else
			subample = 0;
	} else {
		subample = 0;
	}*/
	R_CDSP_AE_AWB_WIN_CTRL &= ~0x7000;
	R_CDSP_AE_AWB_WIN_CTRL |= ((INT32U)raw_en & 0x1) << 12 |
						 ((INT32U)subample &0x3) << 13;  
}

INT8U hwCdspGetAeAwbSrc(void)
{
	return ((R_CDSP_AE_AWB_WIN_CTRL >> 12) & 0x01);
}

INT8U hwCdspGetAeAwbSubSample(void)
{
	//return (1 << ((R_CDSP_AE_AWB_WIN_CTRL >>13) & 0x03)); //why need x2
	return (((R_CDSP_AE_AWB_WIN_CTRL >>13) & 0x03));
}

void hwCdspGet3ATestWinEn(
	INT8U *AeWinTest, 
	INT8U *AfWinTest
)
{
	*AeWinTest = R_CDSP_AEF_WIN_TEST & 0x01;
	*AfWinTest = (R_CDSP_AEF_WIN_TEST >> 3) & 0x01;
}

void hwCdsp_EnableAE(
	INT8U ae_win_en,
	INT8U ae_win_hold
)
{
	R_CDSP_AE_AWB_WIN_CTRL &= ~0x0111;
	R_CDSP_AE_AWB_WIN_CTRL |= ae_win_hold & 0x1 | 
						((INT16U)ae_win_en& 0x1) << 8;  
	//reflected at next vaild vd edge 
	//R_CDSP_AE_AWB_WIN_CTRL |= 0x10;
	/* vdupdate */
	//R_CDSP_FRONT_CTRL0 |= (1<<4);
}


/**
* @brief	cdsp ae buffer address sett 
* @return	0: ae a buffer ready, 1: ae b buffer ready
*/
INT32U gpHalCdspGetAEActBuff(void)
{
	if((R_CDSP_AE_AWB_WIN_CTRL >>15) & 0x01)
		return 1;	/* buffer b active */
	else
		return 0;	/* buffer a active */
}

void hwCdsp_SetAEWin(
	INT8U phaccfactor,
	INT8U pvaccfactor
)
{
	INT8U h_factor, v_factor;
	
	if(phaccfactor <= 4) h_factor = 0;
	else if(phaccfactor <= 8) h_factor = 1;
	else if(phaccfactor <= 16) h_factor = 2;
	else if(phaccfactor <= 32) h_factor = 3;
	else if(phaccfactor <= 64) h_factor = 4;
	else if(phaccfactor <= 128) h_factor = 5;
	else h_factor = 5;

	if(pvaccfactor <= 4) v_factor = 0;
	else if(pvaccfactor <= 8) v_factor = 1;
	else if(pvaccfactor <= 16) v_factor = 2;
	else if(pvaccfactor <= 32) v_factor = 3;
	else if(pvaccfactor <= 64) v_factor = 4;
	else if(pvaccfactor <= 128) v_factor = 5;
	else v_factor = 5;

	DBG_PRINT( "\r\nfactor = [0x%x, 0x%x]\r\n", h_factor, v_factor);		
	R_CDSP_AE_WIN_SIZE = (v_factor << 4)|h_factor;

	//R_CDSP_AE_WIN_SIZE = phaccfactor & 0x07 | (pvaccfactor & 0x07) << 4;
	
	//reflected at next vaild vd edge
	//R_AEW_SIZE |= 0x100;			
}

void hwCdsp_SetAEBuffAddr(
	INT32U winaddra,
	INT32U winaddrb
)
{
	//R_CDSP_AE_WIN_ABUFADDR = winaddra >> 1;
	//R_CDSP_AE_WIN_BBUFADDR = winaddrb >> 1;
	R_CDSP_AE_WIN_ABUFADDR = winaddra;			//GPCV1248 After_2014-01-27
	R_CDSP_AE_WIN_BBUFADDR = winaddrb;

}


void hwCdsp_SetRGBWin(
	INT16U hwdoffset,
	INT16U vwdoffset,
	INT16U hwdsize,
	INT16U vwdsize
)
{
	R_CDSP_RGB_WINH_CTRL = hwdoffset << 12 | hwdsize;
	R_CDSP_RGB_WINV_CTRL = vwdoffset << 12 | vwdsize;
}

void hwCdspGetRGBWin(
	INT16U *hwdoffset, 
	INT16U *vwdoffset, 
	INT16U *hwdsize, 
	INT16U *vwdsize
)
{
	*hwdoffset = (R_CDSP_RGB_WINH_CTRL >> 12) & 0xFFF;
	*hwdsize = R_CDSP_RGB_WINH_CTRL & 0x3FF;
	*vwdoffset = (R_CDSP_RGB_WINV_CTRL >> 12) & 0xFFF;
	*vwdsize = R_CDSP_RGB_WINV_CTRL & 0x3FF;
}

void 
hwCdsp_Enable3ATestWin(
	INT8U AeWinTest,
	INT8U AfWinTest
)
{
	R_CDSP_AEF_WIN_TEST = (AfWinTest & 0x1) << 3 | (AeWinTest & 0x1);
}

/***************************
histgm set
****************************/
void hwCdsp_EnableHistgm(
	INT8U his_en,
	INT8U his_hold_en
)
{
	R_CDSP_HISTGM_CTRL &= ~0x10000;
	R_CDSP_HISTGM_CTRL |= (INT32U)(his_en & 0x1) << 16;
	R_CDSP_AE_AWB_WIN_CTRL &= ~0x2;
	R_CDSP_AE_AWB_WIN_CTRL |= (his_hold_en & 0x1) << 1;		
}

void hwCdsp_SetHistgm(
	INT8U hislowthr,
	INT8U hishighthr 
)
{
	R_CDSP_HISTGM_CTRL &= ~0xFFFF;
	R_CDSP_HISTGM_CTRL |= hislowthr | (INT16U)hishighthr << 8;
}

void hwCdsp_GetHistgm(
	INT32U *hislowcnt,
	INT32U *hishighcnt 
)
{
	*hislowcnt = R_CDSP_HISTGM_LCNT;
	*hishighcnt = R_CDSP_HISTGM_HCNT;
}


/**
* @brief	cdsp get awb cnt
* @param	section [in]: index = 1, 2, 3
* @param	sumcnt [out]: count get
* @return	SUCCESS/ERROR
*/
INT32S gpHalCdspGetAwbSumCnt(
	INT8U section,
	INT32U *sumcnt 
)
{
	//volatile INT32U *ptr;
	
	if(section == 1) {		
		*sumcnt = R_CDSP_SUM_CNT1;
		//*sumcnt = ((UINT32)(pCdspReg2->cdspAwbSumCnt1[3]&0x03)<<24)|((UINT32)pCdspReg2->cdspAwbSumCnt1[2]<<16)|((UINT32)pCdspReg2->cdspAwbSumCnt1[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumCnt1[0];
	} else if(section == 2) {
		//*sumcnt = ((UINT32)(pCdspReg2->cdspAwbSumCnt2[3]&0x03)<<24)|((UINT32)pCdspReg2->cdspAwbSumCnt2[2]<<16)|((UINT32)pCdspReg2->cdspAwbSumCnt2[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumCnt2[0];
		*sumcnt = R_CDSP_SUM_CNT2;
	} else if(section == 3) {
		//*sumcnt = ((UINT32)(pCdspReg2->cdspAwbSumCnt3[3]&0x03)<<24)|((UINT32)pCdspReg2->cdspAwbSumCnt3[2]<<16)|((UINT32)pCdspReg2->cdspAwbSumCnt3[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumCnt3[0];
		*sumcnt = R_CDSP_SUM_CNT3;
	} else {
		return -1;
	}
	return 0;
}

/**
* @brief	cdsp get awb g
* @param	section [in]: index = 1, 2, 3
* @param	sumgl [out]: sum g1 low 
* @param	sumgl [out]: sum g1 high 
* @return	SUCCESS/ERROR
*/
INT32S gpHalCdspGetAwbSumG(
	INT8U section,
	INT32U *sumgl,
	INT32U *sumgh
)
{
	if(section == 1) {
		*sumgl = R_CDSP_SUM_G1_L & 0x0000ffff;
		*sumgh = R_CDSP_SUM_G1_H;
	} else if(section == 2) {
		*sumgl = R_CDSP_SUM_G2_L & 0x0000ffff;
		*sumgh = R_CDSP_SUM_G2_H;
	} else if(section == 3) {
		*sumgl = R_CDSP_SUM_G3_L & 0x0000ffff;
		*sumgh = R_CDSP_SUM_G3_H;
	} else {
		return -1;
	}
	return 0;
}

/**
* @brief	cdsp get awb rg
* @param	section [in]: section = 1, 2, 3
* @param	sumrgl [out]: sum rg low 
* @param	sumrgl [out]: sum rg high 
* @return	SUCCESS/ERROR
*/
INT32S gpHalCdspGetAwbSumRG(
	INT8U section,
	INT32U *sumrgl,
	INT32S *sumrgh
)
{
	//volatile INT32U *ptr;
	
	if(section == 1) {		
		//*sumrgl = ((UINT32)pCdspReg2->cdspAwbSumRg1L[3]<<24)|((UINT32)pCdspReg2->cdspAwbSumRg1L[2] << 16)
		//	| ((UINT32)pCdspReg2->cdspAwbSumRg1L[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumRg1L[0];
		//*sumrgh = (SINT32)(pCdspReg2->cdspAwbSumRg1H&0x07);//<<16)|((UINT32)pCdspReg2->cdspAwbSumRg1L[3]<<8)|((UINT32)pCdspReg2->cdspAwbSumRg1L[2]);
		*sumrgl = R_CDSP_SUM_RG1_L;
		*sumrgh = R_CDSP_SUM_RG1_H;
	} else if(section == 2) {
		//*sumrgl = ((UINT32)pCdspReg2->cdspAwbSumRg2L[3]<<24)|((UINT32)pCdspReg2->cdspAwbSumRg2L[2]<<16)
		//	| ((UINT32)pCdspReg2->cdspAwbSumRg2L[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumRg2L[0];
		//*sumrgh = (SINT32)(pCdspReg2->cdspAwbSumRg2H&0x07);//<<16)|((UINT32)pCdspReg2->cdspAwbSumRg2L[3]<<8)|((UINT32)pCdspReg2->cdspAwbSumRg2L[2]);
		*sumrgl = R_CDSP_SUM_RG2_L;
		*sumrgh = R_CDSP_SUM_RG2_H;
	} else if(section == 3) {
		//*sumrgl = ((UINT32)pCdspReg2->cdspAwbSumRg3L[3]<<24)|((UINT32)pCdspReg2->cdspAwbSumRg3L[2] << 16) 
		//	|((UINT32)pCdspReg2->cdspAwbSumRg3L[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumRg3L[0];
		//*sumrgh = (SINT32)(pCdspReg2->cdspAwbSumRg3H&0x07);//<<16)|((UINT32)pCdspReg2->cdspAwbSumRg3L[3]<<8)|((UINT32)pCdspReg2->cdspAwbSumRg3L[2]);
		*sumrgl = R_CDSP_SUM_RG3_L;
		*sumrgh = R_CDSP_SUM_RG3_H;
	} else {
		return -1;
	}

	
	return 0;
}

/**
* @brief	cdsp get awb bg
* @param	section [in]: section = 1, 2, 3
* @param	sumbgl [out]: sum bg low 
* @param	sumbgl [out]: sum bg high 
* @return	SUCCESS/ERROR
*/
INT32S gpHalCdspGetAwbSumBG(
	INT8U section,
	INT32U *sumbgl,
	INT32S *sumbgh
)
{
	//volatile INT32U *ptr;
	
	if(section == 1) {
		//*sumbgl = ((UINT32)pCdspReg2->cdspAwbSumBg1L[3]<<24)|((UINT32)pCdspReg2->cdspAwbSumBg1L[2] << 16) 
		//	| ((UINT32)pCdspReg2->cdspAwbSumBg1L[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumBg1L[0];
		//*sumbgh = (SINT32)(pCdspReg2->cdspAwbSumBg1H&0x07);//<<16)|;
		*sumbgl = R_CDSP_SUM_BG1_L;
		*sumbgh = R_CDSP_SUM_BG1_H;
	} else if(section == 2) {
		//*sumbgl = ((UINT32)pCdspReg2->cdspAwbSumBg2L[3]<<24)|((UINT32)pCdspReg2->cdspAwbSumBg2L[2] << 16)
		//	| ((UINT32)pCdspReg2->cdspAwbSumBg2L[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumBg2L[0];
		//*sumbgh = (SINT32)(pCdspReg2->cdspAwbSumBg2H&0x07);//<<16)|
		*sumbgl = R_CDSP_SUM_BG2_L;
		*sumbgh = R_CDSP_SUM_BG2_H;
	} else if(section == 3) {
		//*sumbgl = ((UINT32)pCdspReg2->cdspAwbSumBg3L[3]<<24)|((UINT32)pCdspReg2->cdspAwbSumBg3L[2] << 16) 
		//	| ((UINT32)pCdspReg2->cdspAwbSumBg3L[1]<<8)|(UINT32)pCdspReg2->cdspAwbSumBg3L[0];
		//*sumbgh = (SINT32)(pCdspReg2->cdspAwbSumBg3H&0x07);//<<16)|
		*sumbgl = R_CDSP_SUM_BG3_L;
		*sumbgh = R_CDSP_SUM_BG3_H;
	} else {
		return -1;
	}
	
	return 0;
}



/***************************
Motion Detection set	//by u2
****************************/
void hwCdsp_MD_set(INT8U enable, INT8U threshold, INT16U width, INT32U working_buf)
{
	INT32U value;
	
	value = R_CDSP_MD_CTRL & ~0x7F01;
	if (enable && (width>16) && working_buf) {
		value |= 0x1;
	}	
	value |= (threshold & 0x7F) << 8;
		
	R_CDSP_MD_DMA_SADDR = working_buf;
	R_CDSP_MD_HSIZE = width;
	R_CDSP_MD_CTRL = value;
}

void hwCdsp_md_enable(INT8U enable)
{
	INT32U threshold;
	INT16U width; 
	INT32U working_buf;
	
	working_buf = R_CDSP_MD_DMA_SADDR;
	width = R_CDSP_MD_HSIZE;
	threshold = R_CDSP_MD_CTRL;
	
	if (threshold && (width>16) && working_buf) 
	{
		R_CDSP_MD_CTRL |= enable;
	}
}

INT32U hwCdsp_MD_get_result(void)
{
	return R_CDSP_MD_DIFF_DMA_SADDR;
}

/***************************
New ISP from sambai	//by u2
****************************/
void hwIsp_SetImageSize(INT16U hsize,INT16U vsize)
{
	R_ISP_IMSIZE_CROSSTALK_WEIGHT = hsize & 0xFFF | (INT32U)vsize << 16;
}



/***************************
linearity correction set
****************************/
void hwCdsp_LinCorr_ReadTable(
	INT8U *rgb_tbl, 
	INT32U len
)
{
	int i;
	INT32U reg_tmp;
	INT8U *p_rgb_tbl;
	volatile INT32U *p_sram_hw;
	
	
	reg_tmp = R_CDSP_MACRO_CTRL;
	
	R_CDSP_MACRO_CTRL = 0x1414;
		
	p_rgb_tbl = rgb_tbl;
	p_sram_hw = &P_CDSP_SRAM_RW;
	for(i = 0 ; i < len ; i++)
	{
		int val;
		val = *p_rgb_tbl++;
		*p_sram_hw = val;
	}
	
	R_CDSP_MACRO_CTRL = 0;
	
	
	R_CDSP_MACRO_CTRL = reg_tmp;
}



void hwIsp_LinCorr_Enable(INT8U lincorr_en)
{
	R_ISP_LI_HR_CTRL |= lincorr_en;
}

void hwIsp_dbpc_sel(INT8U algorithm_sel)
{
	if (algorithm_sel == 0){
		R_ISP_LI_HR_CTRL &= ~(1<<4);
	} else {
		//R_ISP_LI_HR_CTRL |= (1<<4);	//2014-01-21  only DPC default mode.(ISP_hr_dpc_sel = 0) has been tested.
	}
}

void hwIsp_dpc_en(INT8U dpc_en,INT8U algorithm_sel)	//bpc=dectect bad pixel
{
	if (dpc_en == 1){		//Open bad pixel algorithm should be enable matsumoto algorithm
		R_ISP_LI_HR_CTRL |= (1<<5);
		if (algorithm_sel == 0){
			R_ISP_LI_HR_CTRL &= ~(1<<4);	//matsumoto san perposal
		} else {
			R_ISP_LI_HR_CTRL |= (1<<4);		//Missing bad pixel
		}
	} else {
		R_ISP_LI_HR_CTRL &= ~(1<<5);
	}	
}

void hwIsp_dpc_rcv_mode_sel_thr(INT8U sel_mode, INT16U DPCth1, INT16U DPCth2, INT16U DPCth3)
{
	R_ISP_LI_HR_CTRL &= ~0x80000;
	R_ISP_LI_HR_CTRL |= sel_mode << 19;	//second mode non function
	
	R_ISP_HR_DEPIXCANCEL_THOLD = (DPCth1 & 0x1FF)  |
									(DPCth2 & 0x1FF) << 12;
	
	if (DPCth3 >4){
		R_ISP_HR_DEPIXCANCEL_THOLD |= 0x4 << 24;	//max is 4
	} else {
		R_ISP_HR_DEPIXCANCEL_THOLD |= (DPCth3 & 0x7) << 24;
	}

}

void hwIsp_smooth_factor(INT16U DPCn)	//factor 64:smooth; 192:sharpen
{
	R_ISP_LI_HR_CTRL |= (((INT32U)DPCn & 0x1FF)<<8);
	
	R_ISP_LI_HR_CTRL &= ~0x80000;	//set second mode,but 
}

void hwIsp_EnableCrostlkGbGr(INT8U ctk_en, INT8U ctk_gbgr)
{
	if (ctk_en == 1){
		R_ISP_LI_HR_CTRL |= 1 << 7;
	}else {
		R_ISP_LI_HR_CTRL &= ~(1 << 7);
	}
	
	R_ISP_LI_HR_CTRL |= ctk_gbgr <<20;
		
}


void hwIsp_EnableCrostlk(INT8U ctk_en)
{
	if (ctk_en == 1){
		R_ISP_LI_HR_CTRL |= 1 << 7;
	}else {
		R_ISP_LI_HR_CTRL &= ~(1 << 7);
	}
}

void hwIsp_CrostlkThold(INT16U ctk_thr1,INT16U ctk_thr2,INT16U ctk_thr3,INT16U ctk_thr4)
{
	R_ISP_HR_CROSSTALK_THOLD = (ctk_thr1 & 0xFFFF) |
								((INT32U)ctk_thr2 & 0xFFFF) << 8|
								((INT32U)ctk_thr3 & 0xFFFF) << 16|
								((INT32U)ctk_thr4 & 0xFFFF) << 24;
}

void hwIsp_CrostlkWeight(INT16U ctkw1, INT16U ctkw2, INT16U ctkw3)
{
	R_ISP_HRR_DENOISE_CROSSTALK_WEIGHT &= (~0x0fff0000);
	R_ISP_HRR_DENOISE_CROSSTALK_WEIGHT |= ((INT32U)ctkw1 & 0xF) << 16 |
											((INT32U)ctkw2 & 0xF) << 20 |
											((INT32U)ctkw3 & 0xF) << 24;
}

void hwIsp_EnableDenoise(INT8U denoise_en)
{
	if (denoise_en == 1){
		R_ISP_LI_HR_CTRL |= 1 << 6;
	}else {
		R_ISP_LI_HR_CTRL &= ~(1 << 6);
	}
		
}
void hwIsp_DenoiseThold(INT16U rdn_thr1,INT16U rdn_thr2,INT16U rdn_thr3,INT16U rdn_thr4)
{
	R_ISP_HR_DENOISE_THOLD = (rdn_thr1 & 0xFFFF) |
								((INT32U)rdn_thr2 & 0xFFFF) << 8|
								((INT32U)rdn_thr3 & 0xFFFF) << 16|
								((INT32U)rdn_thr4 & 0xFFFF) << 24;
}

void hwIsp_DenoiseWeight(INT16U rdnw1, INT16U rdnw2, INT16U rdnw3)
{
	R_ISP_HRR_DENOISE_CROSSTALK_WEIGHT &= (~0x00000fff);
	R_ISP_HRR_DENOISE_CROSSTALK_WEIGHT |= ((INT32U)rdnw1 & 0xF) |
											((INT32U)rdnw2 & 0xF) << 4 |
											((INT32U)rdnw3 & 0xF) << 8;
}

void hwIsp_DenoiseWeight_3(INT16U rdnw3)
{
	R_ISP_HRR_DENOISE_CROSSTALK_WEIGHT &= (~0x00000f00);
	R_ISP_HRR_DENOISE_CROSSTALK_WEIGHT |= ((INT32U)rdnw3 & 0xF) << 8;
}

/**********************
New Denoise edge lut table set
**********************/
void hwCdsp_NewDEdgeLut(
	INT8U *pLutNewDEdgeTable
)
{
	INT32S i;
	cdspNewDEdgeLutData_t *pNewDEdgeLutData = (cdspNewDEdgeLutData_t *)(CDSP_TABLE_BASEADDR);
	R_CDSP_MACRO_CTRL = 0x1010;
	for(i = 0; i < 256; i++) 
		pNewDEdgeLutData->NewDEdgeTable[i] = *pLutNewDEdgeTable++;
		
	R_CDSP_MACRO_CTRL = 0x00;
}

/**
* @brief	cdsp new denoise enable
* @param	newdenoiseen [in]: new denoise enable, effective when raw data input.
* @return	none
*/
void hwCdsp_EnableNewDenoise(
	INT8U newdenoiseen
)
{
	if(newdenoiseen)
		R_Ndenoise_CTRL |= 0x01;
	else
		R_Ndenoise_CTRL &= ~0x01;
}

/**
* @brief	get cdsp new denoise
* @param	
* @return	status
*/
INT8U gpHalCdspGetNewDenoiseEn(void)
{
	return (R_Ndenoise_CTRL & 0x01);
}


/**
* @brief	cdsp new denoise set
* @param	ndmirvsel [in]: 1:cnt3eq2, 0:cnt3eq1
* @param	ndmiren [in]: new denoise enable, bit0:top, bit1:down, bit2:left, bit3:right
* @return	none
*/
void hwCdsp_SetNewDenoise_Sel_Mirror(
	INT8U ndmirvsel, 
	INT8U ndmiren
)
{
	ndmirvsel &= 0x1;
	ndmiren &= 0x0F;
	R_Ndenoise_CTRL &= ~((0x0F << 4)|(0x1 << 1));
	R_Ndenoise_CTRL |= (ndmiren << 4)|(ndmirvsel << 1);
}

/**
* @brief	get cdsp new denoise
* @param	ndmirvsel [out]: 1:cnt3eq2, 0:cnt3eq1
* @param	ndmiren [out]: new denoise enable, bit0:top, bit1:down, bit2:left, bit3:right
* @return	none
*/
void gpHalCdspGetNewDenoise(
	INT8U *ndmirvsel, 
	INT8U *ndmiren
)
{
	*ndmirvsel = (R_Ndenoise_CTRL >> 1) & 0x01;
	*ndmiren = (R_Ndenoise_CTRL >> 4) & 0x0F;
}

/**
* @brief	cdsp new denoise edge enable
* @param	ndedgeen [in]: new denoise edge enable
* @param	ndeluten [in]: new denoise edge lut enable
* @return	none
*/
void hwCdsp_EnableNdEdge(
	INT8U ndedgeen,
	INT8U ndeluten
)
{
	if(ndedgeen)
		R_Ndenoise_CTRL |= 0x100;
	else
		R_Ndenoise_CTRL &= ~0x100;

	if(ndeluten)
		R_Ndenoise_CTRL |= 0x1000;
	else
		R_Ndenoise_CTRL &= ~0x1000;
}

/**
* @brief	get cdsp new denoise edge enable
* @param	ndedgeen [out]: new denoise edge enable
* @param	ndeluten [out]: new denoise edge lut enable
* @return	none
*/
void gpHalCdspGetNdEdgeEn(
	INT8U *ndedgeen,
	INT8U *ndeluten
)
{
	*ndedgeen = (R_Ndenoise_CTRL >> 8) & 0x01;
	*ndeluten = (R_Ndenoise_CTRL >> 12) & 0x01;

	//printk("%s: ndedgeen = %d, ndeluten = %d\n", __FUNCTION__, *ndedgeen, *ndeluten);
}

/**
* @brief	cdsp new denoise edge HPF matrix set
* @param	index [in]: 0 ~ 2
* @param	L0 [in]: Matrix L0
* @param	L1 [in]: Matrix L1
* @param	L2 [in]: Matrix L2
* @return	none
*/
void hwCdsp_SetNdEdgeFilter(
	INT8U index,
	INT8U L0_SignBit,
	INT8U L0,
	INT8U L1_SignBit,
	INT8U L1,
	INT8U L2_SignBit,
	INT8U L2
)
{
	L0_SignBit &= 0x01;
	L1_SignBit &= 0x01;
	L2_SignBit &= 0x01;
	L0 &= 0x07;
	L1 &= 0x07;
	L2 &= 0x07;
	
	if(index == 0)
	{
		R_Ndenoise_Ledge_Cof0 = 0;
		
		R_Ndenoise_Ledge_Cof0 |= (L0_SignBit << 3) | (L1_SignBit << 7) |(L1_SignBit << 11);
		if (L0 >= 7) {
			L0 = 6;
		}
		R_Ndenoise_Ledge_Cof0 |= L0;
		if (L1 >= 7) {
			L1 = 6;
		}
		R_Ndenoise_Ledge_Cof0 |= (L1 << 4);
		if (L2 >= 7) {
			L2 = 6;
		}
		R_Ndenoise_Ledge_Cof0 |= (L2 << 8);
	}
	else if(index == 1)
	{
		R_Ndenoise_Ledge_Cof1 = 0;
		
		R_Ndenoise_Ledge_Cof1 |= (L0_SignBit << 3) | (L1_SignBit << 7) | (L1_SignBit << 11);
		if (L0 >= 7) {
			L0 = 6;
		}
		R_Ndenoise_Ledge_Cof1 |= L0;
		if (L1 >= 7) {
			L1 = 6;
		}
		R_Ndenoise_Ledge_Cof1 |= (L1 << 4);
		if (L2 >= 7) {
			L2 = 6;
		}
		R_Ndenoise_Ledge_Cof1 |= (L2 << 8);
	}
	else
	{
		R_Ndenoise_Ledge_Cof2 = 0;
		
		R_Ndenoise_Ledge_Cof2 |= (L0_SignBit << 3) | (L1_SignBit << 7) | (L1_SignBit << 11);
		if (L0 >= 7) {
			L0 = 6;
		}
		R_Ndenoise_Ledge_Cof2 |= L0;
		if (L1 >= 7) {
			L1 = 6;
		}
		R_Ndenoise_Ledge_Cof2 |= (L1 << 4);
		if (L2 >= 7) {
			L2 = 6;
		}
		R_Ndenoise_Ledge_Cof2 |= (L2 << 8);
	}
}

/**
* @brief	cdsp new denoise edge HPF matrix set
* @param	index [in]: 0 ~ 2
* @param	L0 [in]: Matrix L0
* @param	L1 [in]: Matrix L1
* @param	L2 [in]: Matrix L2
* @return	none
*/
void gpHalCdspSetNdEdgeFilter(
	INT8U index,
	INT8U L0,
	INT8U L1,
	INT8U L2
)
{
	L0 &= 0x0F;
	L1 &= 0x0F;
	L2 &= 0x0F;
	
	if(index == 0) {
		R_Ndenoise_Ledge_Cof0 = L0 & 0x0F;
		R_Ndenoise_Ledge_Cof0 |= (L1 & 0x0F) << 4;
		R_Ndenoise_Ledge_Cof0 |= (L2 & 0x0F) << 8;
	} else if(index == 1) {
		R_Ndenoise_Ledge_Cof1 = L0 & 0x0F;
		R_Ndenoise_Ledge_Cof1 |= (L1 & 0x0F) << 4;
		R_Ndenoise_Ledge_Cof1 |= (L2 & 0x0F) << 8;
	}else {
		R_Ndenoise_Ledge_Cof2 = L0 & 0x0F;
		R_Ndenoise_Ledge_Cof2 |= (L1 & 0x0F) << 4;
		R_Ndenoise_Ledge_Cof2 |= (L2 & 0x0F) << 8;
	}
}

void gpHalCdspGetNdEdgeFilter(
	INT8U index,
	INT8U *L0,
	INT8U *L1,
	INT8U *L2
)
{
	if(index == 0) {
		*L0 = R_Ndenoise_Ledge_Cof0 & 0x0F;
		*L1 = (R_Ndenoise_Ledge_Cof0 >> 4) & 0x0F;
		*L2 = (R_Ndenoise_Ledge_Cof0 >> 8) & 0x0F;
	} else if(index == 1) {
		*L0 = (R_Ndenoise_Ledge_Cof1) & 0x0F;
		*L1 = (R_Ndenoise_Ledge_Cof1 >> 4) & 0x0F;
		*L2 = (R_Ndenoise_Ledge_Cof1 >> 8) & 0x0F;
	} else {
		*L0 = R_Ndenoise_Ledge_Cof2 & 0x0F;
		*L1 = (R_Ndenoise_Ledge_Cof2 >> 4) & 0x0F;
		*L2 = (R_Ndenoise_Ledge_Cof2 >> 8) & 0x0F;
	}
}


/**
* @brief	cdsp new denoise edge scale set
* @param	ndlhdiv [in]: L edge enhancement edge vale scale
* @param	ndlhtdiv [in]: L edge enhancement edge vale scale
* @param	ndlhcoring [in]: L core ring threshold
* @param	ndlhmode [in]: 1: default matrix, 0: enable paroramming matrix
* @return	none
*/
void hwCdsp_SetNdEdgeLCoring(
	INT8U ndlhdiv, 
	INT8U ndlhtdiv, 
	INT8U ndlhcoring 
)
{
	/*
	INT8U lh, lht;

	lh = lht = 0;
	while(1) // lhdiv=2^lh
	{
		ndlhdiv >>= 1;
		if(ndlhdiv)
			lh++;
		else 
			break;

		if(lh >= 7)
			break;
	}
	
	while(1) // lhtdiv=2^lht
	{
		ndlhtdiv >>= 1;
		if(ndlhtdiv) 
			lht++;
		else 
			break;

		if(lht >= 7)
			break;
	}
	
	if(lh > 7) lh = 7;
	if(lht> 7) lht = 7;
	R_Ndenoise_Ledge_Set |= (lht << 12)|(lh << 8);
	*/
	R_Ndenoise_Ledge_Set |= ((ndlhtdiv & 0x7) << 12 | (ndlhdiv & 0x7) << 8);
	R_Ndenoise_Ledge_Set |= (ndlhcoring & 0x1FF);
}

/**
* @brief	cdsp new denoise edge scale get
* @param	ndlhdiv [out]: L edge enhancement edge vale scale
* @param	ndlhtdiv [out]: L edge enhancement edge vale scale
* @param	ndlhcoring [out]: L core ring threshold
* @param	ndlhmode [out]: 1: default matrix, 0: enable paroramming matrix
* @return	none
*/
void gpHalCdspGetNdEdgeLCoring(
	INT8U *ndlhdiv, 
	INT8U *ndlhtdiv, 
	INT8U *ndlhcoring 
)
{
	*ndlhdiv = 1 << ((R_Ndenoise_Ledge_Set >> 8) & 0x07);
	*ndlhtdiv = 1 << ((R_Ndenoise_Ledge_Set >> 12) & 0x07);
	*ndlhcoring = R_Ndenoise_Ledge_Set & 0xFF;
	//*ndlhmode = pCdspReg0->cdspNdnLHMode;
}

/**
* @brief	cdsp new denoise edge amp set
* @param	ndampga [in]: 0:1, 1:2, 2:3, 3:4
* @return	none
*/
void hwCdsp_SetNdEdgeAmpga(INT8U ndampga)
{
	ndampga &= 0x03;
#if 1	
	switch(ndampga)
	{
	case 0:
	case 1:
		ndampga = 0;
		break;
	case 2:
		ndampga = 1;
		break;
	case 3:
		ndampga = 2;
		break;
	case 4:
		ndampga = 3;
		break;
	default:
		ndampga = 3;
		break;		
	}
#endif
	R_Ndenoise_CTRL |= (ndampga << 14);
}

/**
* @brief	cdsp new denoise edge amp get
* @param	ndampga [out]: 0:1, 1:2, 2:3, 3:4
* @return	none
*/
INT8U gpHalCdspGetNdEdgeAmpga(void)
{
	return ((R_Ndenoise_CTRL >> 14) & 0x3);
	
	/*switch(ndampga)
	{
	case 0:
	case 1:
		ndampga = 0;
		break;
	case 2:
		ndampga = 1;
		break;
	case 3:
		ndampga = 2;
		break;
	case 4:
		ndampga = 3;
		break;
	default:
		ndampga = 3;
		break;		
	}*/
}




/**
* @brief	Select Input Scaler Source: CDSP /CSI
* @param	CDSP_PATH, CSI_PATH
* @return	none
*/
void hwCdsp_Scal_Source(INT32U sen_path)
{
	if (sen_path == CDSP_PATH)
	{
		(*(volatile INT32U *)(0xD0000084)) |= (1<<4);
	}
	else
	{
		(*(volatile INT32U *)(0xD0000084)) &= ~(1<<4);
	}
}

void cdsp_init(void)
{
	hwCdsp_Reset();
	hwFront_Reset();
	
	hwCdsp_DataSource(SDRAM_INPUT);
	
	hwCdsp_EnableAE(0, 0);
	hwCdsp_EnableAWB(0, 0);
	
	hwCdsp_SetInt(ENABLE, CDSP_EOF);
	
	//hwCdsp_SetYuvRange(0x3); //unsigned
	hwCdsp_SetSRAM(1, 0xA0); 	
}


void drvl1_cdsp_init(INT32U SNR_WIDTH, INT32U SNR_HEIGHT)
{
	//DBG_PRINT("drvl1_cdsp_init!\r\n");

	frame_width_after_clip = 0;
			
	hwCdsp_Reset();
	hwFront_Reset();
	hwCdsp_DataSource(SDRAM_INPUT); //sdram
		
	hwCdsp_EnableAF(0, 0);
	hwCdsp_EnableAE(0, 0);
	hwCdsp_EnableAWB(0, 0);

#if 1	/*Enable raw interpolution*/
	hwCdsp_SetExtLine(ENABLE, SNR_WIDTH, 0x28);
	hwCdsp_IntplThrSet(0x10, 0xF0);
	hwCdsp_EnableIntplMir(0x0F, 1, 0);
#endif
#if 1 	/*For YUV Horizontal Average method and LP Filiter temp*/
	hwCdsp_SetYuvHAvg(0x03, 0, 0, 0);	
#endif

	hwCdsp_SetSRAM(ENABLE, 0xA0);
	hwCdsp_EnableClamp(0, SNR_WIDTH);
	
	/*cdsp input image format*/
	hwCdsp_SetYuvRange(0x0); 		//unsigned
	
	/*RAW8*/
	hwFront_SetInputFormat(SENSOR_RAW10);

	/*cdsp output image format*/
	hwCdsp_SetOutYUVFmt(UYVY);		//cdsp_output:YUYV == PixelView:UYVY

	/*cdsp irq*/
	hwCdsp_SetInt(DISABLE, CDSP_INT_ALL);			//not include FIFO mode
	//vic_irq_register(VIC_CDSP, cdsp_isr);	//gp_cdsp_irq_handler
	//vic_irq_enable(VIC_CDSP);
}

void drvl1_cdsp_stop(void)
{
	//hwCdsp_Reset();
	//hwFront_Reset();
	hwCdsp_DataSource(SDRAM_INPUT); //sdram
		
	hwCdsp_EnableAF(0, 0);
	hwCdsp_EnableAE(0, 0);
	hwCdsp_EnableAWB(0, 0);

	hwCdsp_RedoTriger(DISABLE);
	//cdsp irq
	hwCdsp_SetInt(DISABLE, CDSP_INT_ALL);			//not include FIFO mode
	vic_irq_unregister(VIC_CDSP);
	vic_irq_disable(VIC_CDSP);

	/*init sensor Clk*/
	//SensorIF_Setting(C_RAW_FMT);
	R_SYSTEM_CLK_CTRL &= ~(1<<7);	//0xD000001C,Disable clock
	//DBG_PRINT("cdsp_stop!\r\n");
			
}

#endif //(defined _DRV_L1_CDSP) && (_DRV_L1_CDSP == 1)                   //
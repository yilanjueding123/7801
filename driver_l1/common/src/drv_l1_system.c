/*
* Purpose: System option setting
*
* Author: wschung
*
* Date: 2008/5/9
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
* History :

     1 .Add system_bus_arbiter_init  2008/4/21 wschung
        setting all masters as 0
*/
#include "drv_l1_system.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_SYSTEM) && (_DRV_L1_SYSTEM == 1)             //
//================================================================//

#define INT_EXTAB             0x04000000
#define INT_RTC               0x80000000

INT32U  MCLK = INIT_MCLK;
INT32U  MHZ  = INIT_MHZ;

/* Prototypes */
void system_set_pll(INT32U PLL_CLK);
void system_bus_arbiter_init(void);
void system_enter_halt_mode(void);
void system_power_on_ctrl(void);
INT32U system_rtc_counter_get(void);
void system_rtc_counter_set(INT32U rtc_cnt);
static tREFI_ENUM refresh_period=tREFI_15o6u;

INT32U    ir_cnt = 1;


INT32S system_clk_init(void)
{
#if TV_DET_ENABLE
	system_clk_ext_XLAT_12M();		// 切至外部 XLAT 12MHz
#else
	R_SYSTEM_CTRL = 0x00000902;  // josephhsieh@140519 sensor 前端固定為48MHz
	R_SYSTEM_CKGEN_CTRL = 0x1F;
#endif

	{
		R_SYSTEM_POWER_CTRL0 = 0x2F1F;
		R_SYSTEM_POWER_CTRL0 = 0x271F;
	}

#if IR_PIN_TYPE == 0
	#if ( (USE_SENSOR_NAME==SENSOR_GC1004)||(USE_SENSOR_NAME==SENSOR_GC1014)||(USE_SENSOR_NAME==SENSOR_GC1004_MIPI) || (USE_SENSOR_NAME==SENSOR_OV9712))
		sys_ldo28_ctrl(1, LDO_28_3v3);	// LDO_2.8 調成 3.3v  => ON
	#else
		sys_ldo28_ctrl(1, LDO_28_2v8);	// LDO_2.8v => ON
	#endif	
#else
	sys_ldo28_ctrl(0, LDO_28_3v3);
#endif
	sys_ldo_codec_ctrl(1,LDO_CODEC_3v3);  // LDO codec ==> ON

	sys_ldo33_mode(LDO_33_3v3);		// LDO 3.3v output (default: 3.3v)
	R_SYSTEM_LVR_CTRL = 0x10;				// 設成 1.2V， CorePower 1.32V 用外灌

	R_SYSTEM_PLLEN |= 0x0100;	//use external 32K crystal for internal RTC

	return 0;
}

void system_set_pll(INT32U cpu_clock_mhz)
{
	INT32U PLL_REG = (R_SYSTEM_PLLEN & ~0x3F);
    
	// Clear reset flags
	R_SYSTEM_RESET_FLAG = 0x0000001C;

  
	while (R_SYSTEM_POWER_STATE == 0) ;
		
    if (cpu_clock_mhz <= 48) {
		PLL_REG |= 0x2;
	} else if (cpu_clock_mhz >= 144) {
		PLL_REG |= 0x1A;
	} else { 
    	PLL_REG |= (cpu_clock_mhz-40)>>2;
    }
	R_SYSTEM_PLLEN = PLL_REG;

	// Enable fast PLL
	if (cpu_clock_mhz > 48) {
		// Set USB and SPU clock to PLL/2 when system clock is faster than 48MHz
		R_SYSTEM_CLK_CTRL = 0x00008008;
	} else {
		R_SYSTEM_CLK_CTRL = 0x00008000;
	}
	
    // dennis 20140401
	//R_SYSTEM_PLLEN |= 0x00000100;		// Use 32K crystal as RTC clock and system 32K clock source

    __asm {
    	NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	    NOP
	};

    while (R_SYSTEM_POWER_STATE == 0) ; 			// Wait stable

	// Initiate watch dog
	R_SYSTEM_WATCHDOG_CTRL = 0;      
}

void system_bus_arbiter_init(void)
{
	/*
	0 ppu display
	1 sensor
	2 usb20
	3 ppu engine
	4 dma
	5 jpg
	6 scal
	7 spu
	8 nfc
	9 cpu
	a mp3
	b mp4
	*/
	R_MEM_M2_BUS_PRIORITY =0;
	R_MEM_M3_BUS_PRIORITY =0;
	R_MEM_M4_BUS_PRIORITY =2;
	R_MEM_M5_BUS_PRIORITY =0;
	R_MEM_M6_BUS_PRIORITY =0;
	R_MEM_M7_BUS_PRIORITY =0;
	R_MEM_M8_BUS_PRIORITY =0;
	R_MEM_M9_BUS_PRIORITY =0;
	R_MEM_M10_BUS_PRIORITY =0;
	R_MEM_M11_BUS_PRIORITY =0;
}

extern INT32U    day_count;
INT32U system_rtc_counter_get(void)
{
    return (INT32U) day_count;
}

void system_rtc_counter_set(INT32U rtc_cnt)
{
    day_count = rtc_cnt;
}

/* place to internal ram (432byte)*/
#ifndef __CS_COMPILER__
#pragma arm section rwdata="pwr_ctrl", code="pwr_ctrl"
#else
void system_power_on_ctrl(void) __attribute__ ((section(".pwr_ctrl")));
void system_enter_halt_mode(void) __attribute__ ((section(".pwr_ctrl")));
void sys_ir_delay(INT32U t) __attribute__ ((section(".pwr_ctrl")));
void sys_power_handler(void) __attribute__ ((section(".pwr_ctrl")));
#endif

INT32U    day_count = 0xFFFFFFFF;//2454829; /* julian date count */
INT32U    halt_data = 1;
INT32U    ir_opcode = 1;
INT32U	  Alarm_Trriger_By_Gp6 = 1;	
INT32U	  Day_Trriger_By_Gp6 = 1;

void sys_ir_delay(INT32U t)
{
	R_TIMERB_PRELOAD = (0x10000-t);

	R_TIMERB_CTRL |= 0x2000;
	while((R_TIMERB_CTRL & 0x8000) == 0);
  	R_TIMERB_CTRL &= ~0x2000;
}

void system_power_on_ctrl(void)
{
	INT32U  i;

	/* wakeup by RTC_DAY int */
	#if _DRV_L1_RTC == 1
	if (R_RTC_INT_STATUS & RTC_DAY_IEN) {
		R_RTC_INT_STATUS = 0x1f;
		day_count++; /* add 1 to julian date count */
		if (ir_opcode != 0xFFFF0000) {
			system_enter_halt_mode();
		}
	}
	#endif
	
	if (ir_opcode != 0xFFFF0000) {	
		/* wakeup by extab */
		if (R_INT_IRQFLAG & INT_EXTAB) {
		  	//R_TIMERB_PRELOAD = 0xFFF8; /* 1 msec */
			//R_TIMERB_PRELOAD = 0xF999; /* 200 msec */
			//R_TIMERB_CTRL = 0x8063;/*8192 (122us per-count)*/
			R_TIMERB_CTRL = 0x8061;
			R_INT_KECON |= 0x40; /* clear exta int */
		 
			/* default IOF5 is input with pull low */
			//R_IOF_DIR &= ~0x20; /* IOF5 */
		  	sys_ir_delay(7519); /* 10 ms in 48MHz*/
		  	//sys_ir_delay(22557);
		  	for (i=0;i<22;i++) {
				sys_ir_delay(752); /* 1 ms in 48MHz*/
			}
				
		    #if 0
  			cnt =0;
    		for (i=0; i<=200; i++) { /* press at least 200ms */
				if (cnt >= 10) {
					system_enter_halt_mode();
				}
    	
  				R_TIMERB_CTRL |= 0x2000;
  				while((R_TIMERB_CTRL & 0x8000) == 0);
  				R_TIMERB_CTRL &= ~0x2000;
    	
				if ((R_IOF_I_DATA & 0x20) == 0) {
					cnt++;
					i = 0;
				}
			}
		    #endif
		}
    }
	//R_MEM_SDRAM_CTRL0 |= 0x10; /* enable SDRAM */
	R_SYSTEM_CTRL &= ~0x20; /* strong 6M mode */
}

#define cnt_10ms_under_6M		234
#define cnt_10ms_under_96M		3750
#define cnt_10ms_under_144M		5625

#define cnt_10ms		cnt_10ms_under_6M

void sys_power_handler(void)
{
	R_TIMERB_CTRL = 0x8061;
	ir_cnt=0;

	gpio_write_io(POWER_EN_PIN, DATA_LOW);
	sys_ldo33_off();	// turn off LDO 3.3, MUST place after power key released
	system_set_pll(6);
	sys_ir_delay(cnt_10ms);	/* wait 10ms to discharge system power */

	R_SYSTEM_CLK_EN0 = /* 0x0015 */0x0017;	//wwj modify, don't turn off memory clock
	R_SYSTEM_CLK_EN1 = 0xC080;

	while(1) {

	if ( PWR_KEY_TYPE == READ_FROM_GPIO)
	{
		if (gpio_read_io(PW_KEY)) {
			ir_cnt++;
		} else {
			ir_cnt = 0;			
		}
	}
	else
	{
		
		if (PW_KEY == PWR_KEY0) {
		 	if(sys_pwr_key0_read()) {
				ir_cnt++;
		 	} else {
				ir_cnt = 0;
		 	}
		} else if (PW_KEY == PWR_KEY1) {
		 	if(sys_pwr_key1_read()) {
				ir_cnt++;
		 	} else {
				ir_cnt = 0;
		 	}
		}
	}
		

		if (ir_cnt > 20) { //power on by a short press of 200ms
			break;
		}
		sys_ir_delay(cnt_10ms); /* 10ms */	//6MHz system clock
	}
	R_SYSTEM_WATCHDOG_CTRL = 0x8003;
	while (1);
}

void system_enter_halt_mode(void)
{
	//R_IOF_ATT |= 0x0010;
	//R_IOF_DIR |= 0x0010;
	//R_IOF_O_DATA &= ~0x10;
	//R_MEM_M11_BUS_PRIORITY |= 1;
	R_SYSTEM_PLL_WAIT_CLK = 0x100; /* set pll wait clock to 8 ms when wakeup*/
	//R_MEM_SDRAM_CTRL0 &= ~0x10; /* disable SDRAM */
	R_SYSTEM_CTRL &= ~0x2; /* CPU reset when wakeup */

    R_SYSTEM_HALT = 0x500A;
    halt_data = R_CACHE_CTRL;

    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
    ASM(NOP);
}

//#ifndef __CS_COMPILER__
//#pragma arm section rwdata, code
//#endif

void sys_ir_opcode_clear(void)
{
	ir_opcode = 0;	
}

INT32U sys_ir_opcode_get(void)
{
	return ir_opcode;	
}

void system_da_ad_pll_en_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_SYSTEM_CLK_CTRL |= 0x10; /* DA/AD PLL Enable */
	}

	else {
		R_SYSTEM_CLK_CTRL &= ~0x10; /* DA/AD PLL FALSE */
	}
}

INT32S sys_sdram_MHZ_set(SDRAM_CLK_MHZ sdram_clk)
{

    INT32U cbrcyc_reg_val;
    INT32U oc=1; /* Internal test for overing clock */

    //cbrcyc_reg_val = R_MEM_SDRAM_CBRCYC;

    cbrcyc_reg_val=(refresh_period*sdram_clk/10)&0xFFFF * oc;

    R_MEM_SDRAM_CBRCYC = cbrcyc_reg_val;

    return STATUS_OK;
}

void sys_weak_6M_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_SYSTEM_CTRL |= 0x20; /* weak 6M mode */
	}
	else {
		R_SYSTEM_CTRL &= ~0x20; /* strong 6M mode */
	}
}

void sys_reg_sleep_mode_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_SYSTEM_CTRL |= 0x80; /* enable reg sleep mode */
	}
	else {
		R_SYSTEM_CTRL &= ~0x80; /* disable reg sleep mode */
	}
}

INT32S sys_ldo_codec_ctrl(int flag, int mode)
{
	// turn on/off LDO codec
	int sel = 0x100 | mode;
	int reg = R_SYSTEM_POWER_CTRL0;
	
	if (flag) {
		reg &= (~0x700);
		reg |= sel;
		R_SYSTEM_POWER_CTRL0 = reg;
	}
	else {
		R_SYSTEM_POWER_CTRL0 &= (~0x700);
	}
	return 0;
}

INT32S sys_ldo28_ctrl(int flag, int mode)
{
	// turn on/off LDO 2.8v
	int sel = 0x10 | mode;
	int reg = R_SYSTEM_POWER_CTRL0;

	if (flag) {
		reg &= (~0xF0);
		reg |= sel;
		R_SYSTEM_POWER_CTRL0 = reg;
	}
	else {
		R_SYSTEM_POWER_CTRL0 &= (~0xF0);
	}
	return 0;
}

INT32S sys_ldo33_mode(int mode)
{
	R_SYSTEM_POWER_CTRL0 &= (~0x0C);
	R_SYSTEM_POWER_CTRL0 |= mode;
	return 0;
}

INT32S sys_ldo33_off(void)
{
	// turn off LDO 3.3v  (turn on in rom code)
	R_SYSTEM_POWER_CTRL0 &= (~0x112);
	R_SYSTEM_POWER_CTRL0 &= (~0x1);	
	return 0;
}

INT32S sys_pwr_key0_read(void)
{
	return (R_SYSTEM_POWER_CTRL1&0x1);
}

INT32S sys_pwr_key1_read(void)
{
	return (R_SYSTEM_POWER_CTRL1&0x2);
}


#include "drv_l2_sensor.h"
#include "my_video_codec_callback.h"

extern void system_clk_alter(INT32U SysClk, INT32U SDramPara);

#define MSDC_WAIT_TIME 50

INT32U MSDC_CLK = 0;

INT32S sys_msdc_clk_set(INT32U clk_mhz)
{
	INT32U SysClk, SDramPara;

	switch(clk_mhz)
	{
		case 48:
			SysClk = 0x02;
			SDramPara = 0x11;
			break;
		case 72:
			SysClk = 0x08;
			SDramPara = 0x711;
			break;
		case 144:
		default:
			clk_mhz = 144;
			SysClk = 0x1A;
			SDramPara = 0x711;
	}
	MCLK = clk_mhz * 1000000;	
	MHZ	  = clk_mhz;
	system_clk_alter(SysClk, SDramPara);
	
	return 0;
}

// switch system clock
INT32S sys_msdc_clk_active(void)
{
	INT32U i;

	{
	// sensor
		#if (sensor_format == 1)		// usually symbol '1' is parallel interface, '2' is MIPI interface
	 hwCdsp_DataSource(MIPI_INPUT);
	 DBG_PRINT("Sensor Parallel Interface\r\n");
		#else
		 hwCdsp_DataSource(FRONT_INPUT);
	 DBG_PRINT("Sensor MIPI Interface\r\n");		 
		#endif
	}

	// ++++++++++++++++++++++++++
	MSDC_CLK = 48;
	if (MSDC_CLK!=0)
	{
		sys_msdc_clk_set(MSDC_CLK);
		/*if (MSDC_CLK == 48) {
			AP_TFT_ClK_48M_set();
		}	
		else {
			AP_TFT_ClK_144M_set();
		}*/

		MSDC_CLK = 0;
	}
	DBG_PRINT("SYSTEM CLOCK alter %d\r\n",MSDC_CLK);
	for (i=0;i<MSDC_WAIT_TIME;++i) {
		OSTimeDly(1);
		if (MSDC_CLK==0)
			break;
	}
	if (i==MSDC_WAIT_TIME)
	{
		hwCdsp_DataSource(FRONT_INPUT);
		DBG_PRINT("MSDC CLK setting Error !!\r\n");
		return -1;
	}
	else {
		OSTimeDly(1);
	}
	// ==========================
	
	{ // uart
		uart_buad_rate_set(UART_USED_NUM,UART0_BAUD_RATE);
		DBG_PRINT("SYS_CLK = %d\r\n",MCLK);
	}	
	{ // SD
		if (drvl2_sd_init()==0) {
			drvl2_sd_bus_clock_set(50000000); 
		}
		else {
			DBG_PRINT("SD Initial Fail\r\n");
		}
	}
	
	return 0;
}


INT32S sys_msdc_clk_restore(void)
{
	INT32U i;

	// ++++++++++++++++++++++++++
	MSDC_CLK = 144;
	if (MSDC_CLK!=0)
	{
		sys_msdc_clk_set(MSDC_CLK);
		/*if (MSDC_CLK == 48) {
			AP_TFT_ClK_48M_set();
		}	
		else {
			AP_TFT_ClK_144M_set();
		}*/

		MSDC_CLK = 0;
	}
	DBG_PRINT("SYSTEM CLOCK alter %d\r\n",MSDC_CLK);
	for (i=0;i<MSDC_WAIT_TIME;++i) {
		OSTimeDly(1);
		if (MSDC_CLK==0)
			break;
	}
	if (i==MSDC_WAIT_TIME)
	{
		DBG_PRINT("MSDC CLK setting Error !!\r\n");
		return -1;
	}
	else {
		OSTimeDly(1);
	}
	// ==========================


	{ // sensor
		#if (sensor_format == 1)		// usually symbol '1' is parallel interface, '2' is MIPI interface	
		hwCdsp_DataSource(FRONT_INPUT);
		#else
		hwCdsp_DataSource(MIPI_INPUT);
		#endif
	}	
	{ // uart
		uart_buad_rate_set(UART_USED_NUM,UART0_BAUD_RATE);
		DBG_PRINT("SYS_CLK = %d\r\n",MCLK);		
	}
	{ // SD
		if (drvl2_sd_init()==0) {
			drvl2_sd_bus_clock_set(50000000); 
		}
		else {
			DBG_PRINT("SD Initial Fail\r\n");
		}
	}

	return 0;
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_SYSTEM) && (_DRV_L1_SYSTEM == 1)        //
//================================================================//


/*
* Purpose: system initial functions after reset
*
* Author: dominant
*
* Date: 2009/12/08
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
* History :
*/
//Include files
#include "driver_l1.h"
#include "drv_l1_system.h"
#include "customer.h"

void drv_l1_init(void)
{
	// First initiate those settings that affect system, like: SDRAM access, system clock, cache, exception handler, interrupt, watchdog
	// Then initiate those hardware modules that are bus masters, like: DMA, JPEG, scaler, PPU, SPU, TFT
	// Finally, initiate other hardware modules

  #if  _DRV_L1_SYSTEM == 1
	system_clk_init();
	//system_set_pll(MHZ);				// Set PLL
	/* 請在運用Task才設置PRIORITY
	system_bus_arbiter_init();		// Initaile bus arbiter
	*/
  #endif

  #if _DRV_L1_CACHE == 1
	cache_init();						// Initiate CPU Cache
  #endif

	exception_table_init();				// Initiate CPU exception handler
  #if _DRV_L1_INTERRUPT == 1
	vic_init();							// Initiate Interrupt controller
  #endif

  #if _DRV_L1_WATCHDOG==1
	watchdog_init();					// Initiate WatchDOg
  #endif

  #if _DRV_L1_DMA == 1
	dma_init();							// Initiate DMA controller
  #endif

  #if _DRV_L1_JPEG == 1
	jpeg_init();						// Initiate JPEG engine
  #endif

  #if _DRV_L1_SCALER == 1
	scaler_init(0);						// Initiate Scaler engine0
	scaler_init(1);						// Initiate Scaler engine1
  #endif

  #if (defined SUPPORT_JTAG) && (SUPPORT_JTAG == CUSTOM_OFF)
	gpio_set_ice_en(FALSE);  
  #endif

  #if _DRV_L1_GPIO==1
	gpio_init();						// Initiate GPIO and IO PADs
  #endif 

  #if _DRV_L1_UART==1
   #if (defined DBG_MESSAGE) && (DBG_MESSAGE==CUSTOM_ON)
      uart_init(UART_USED_NUM);						// Initiate UART
      uart_buad_rate_set(UART_USED_NUM,UART0_BAUD_RATE);
      uart_tx_enable(UART_USED_NUM);
      // UART RX setting (for GPS)
      #if 0
      uart_fifo_enable(UART_USED_NUM);
      uart_fifo_ctrl(UART_USED_NUM, 0, 6);
      uart_rx_enable(UART_USED_NUM);
      #endif  
   #endif
  #endif

//DBG_PRINT("wwj: R_FUNPOS1 == 0x%x\r\n", R_FUNPOS1);
//DBG_PRINT("wwj: R_IOH_DIR == 0x%x\r\n", R_IOH_DIR);
//DBG_PRINT("wwj: R_IOH_ATT == 0x%x\r\n", R_IOH_ATT);
//DBG_PRINT("wwj: R_IOH_DATA == 0x%x\r\n", R_IOH_DATA);
//DBG_PRINT("wwj: R_IOH_BUFFER == 0x%x\r\n", R_IOH_BUFFER);

  #if _DRV_L1_SPI==1
	//spi_disable(0);						// Initiate SPI
  #endif

  //#if _DRV_L1_KEYSCAN==1
	//matre_keyscaninit();
  //#endif

  #if _DRV_L1_EXT_INT==1
	ext_int_init();
  #endif

  #if _DRV_L1_DAC==1
    // 錄音和放音用同一個 Vref
	dac_init();						// Initiate DAC
	i2s_adc_init(0);				// 錄音使用麥克風
  #endif

  #if _DRV_L1_ADC==1
	//adc_init();						// Initiate Analog to Digital Convertor
  #endif
  #if _DRV_L1_MIC==1
  	//mic_init();						// Initiate Micphone
  #endif

  #if _DRV_L1_TIMER==1
	timer_init();						// Initiate Timer
	timerD_counter_init();              // Tiny Counter Initial (1 tiny count == 2.67 us)
  #endif

 #if _DRV_L1_RTC==1
	rtc_init();							// Initiate Real Time Clock
  #endif

  #if _DRV_L1_SW_TIMER== 1
   #if _DRV_L1_RTC==1
	sw_timer_init(TIMER_RTC, 1024);
   #endif
  #endif

#if GPDV_BOARD_VERSION != GPCV4247_WIFI	
  #if _DRV_L1_TFT==1
	tft_init();							// Initiate TFT controller
  #endif
#endif  

  #if _DRV_L1_TV==1
	tv_init();
  #endif

  #if _DRV_L1_I2C == 1
       #if (GPDV_BOARD_VERSION == DVP_V1_0)  // EVB
 	  i2c_init();
	#else  // // 用IOB4, IOB5 搖 I2C，結果與 I2C 出 PIN 相衝
	  R_I2C_MISC = 0;
	#endif
  #endif

  #if _DRV_L1_CONV422TO420 == 1
 	conv422_init(); 
  #endif
 
  #if _DRV_L1_CDSP ==1
 	drvl1_cdsp_init(SENSOR_WIDTH, SENSOR_HEIGHT);
  #endif

  #if _DRV_L1_GSENSOR == 1
	G_Sensor_Init(2);
  #endif

	// clk disable for power saving
	R_SYSTEM_CLK_EN0 &= (~0x1128);
	R_SYSTEM_CLK_EN1 &= (~0x2022);
/*
	gpio_init_io(SPEAKER_EN, GPIO_OUTPUT);
	gpio_set_port_attribute(SPEAKER_EN, 1);
	gpio_write_io(SPEAKER_EN, 0);
*/
}

/*
R_SYSTEM_CLK_EN0
bit[0]	: System Bus
bit[1]	: Memory/SDRAM
bit[2]	: Interrupt
bit[3]	: HDMI
bit[4]	: TimeBase
bit[5]	: GTE
bit[6]	: DAC
bit[7]	: UART
bit[8]	: MP3
bit[9]	: SPI
bit[10]	: ADC
bit[11]	: CSI
bit[12]	: SPU
bit[13]	: TFT
bit[14]	: CDSP
bit[15]	: SD

R_SYSTEM_CLK_EN1
bit[0]	: 
bit[1]	: USB1
bit[2]	: USB2
bit[3]	: STN
bit[4]	: DMA
bit[5]	: TV
bit[6]	: PPU
bit[7]	: iRAM
bit[8]	: SPIFC
bit[9]	: MIPI
bit[10]	: JPEG
bit[11]	: PPU_REG
bit[12]	: xscatop_0
bit[13]	: Nand
bit[14]	: System Ctrl1
bit[15]	: System Ctrl2
*/


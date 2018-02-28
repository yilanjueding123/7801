#ifndef __DRIVER_L1_CFG_H__
#define __DRIVER_L1_CFG_H__

#define INIT_MCLK                   (INIT_MHZ*1000000)

#define SDRAM_START_ADDR            0x00000000
#define SDRAM_END_ADDR              (SDRAM_START_ADDR + SDRAM_SIZE - 1)

// MCU configuration
#define _DRV_L1_ADC                 1
#define _DRV_L1_CACHE               1
#define _DRV_L1_CFC                 1
#define _DRV_L1_DAC                 1
#define _DRV_L1_I2S_TX              1
#define _DRV_L1_DMA                 1
#define _DRV_L1_MIC                 1
#define _DRV_L1_EXT_INT             1
#define _DRV_L1_GPIO                1
#define _DRV_L1_GTE                 1
#define _DRV_L1_INTERRUPT           1
#define _DRV_L1_JPEG                1
//#define _DRV_L1_KEYSCAN             0
#define _DRV_L1_NAND                0
#define _DRV_L1_SPIFC				1
#define _DRV_L1_RTC                 1
#define _DRV_L1_SCALER              1
#define _DRV_L1_SDC                 1
#define _DRV_L1_SENSOR              1
#define _DRV_L1_SPI                 1
#define _DRV_L1_SW_TIMER            1
#if _DRV_L1_SW_TIMER==1
	#define DRV_L1_SW_TIMER_SOURCE  	TIMER_RTC
	#define DRV_L1_SW_TIMER_Hz      	1024
#endif
#define _DRV_L1_SYSTEM              1
#define _DRV_L1_TFT                 1
#define _DRV_L1_HDMI				1
#define _DRV_L1_TIMER               1
#define _DRV_L1_TV                  1 
#define _DRV_L1_UART                1
#define _DRV_L1_WATCHDOG            1
#define _DRV_L1_CONV420TO422		1
#define _DRV_L1_CONV422TO420		1
#define _DRV_L1_WRAP				1
#define _DRV_L1_I2C					1
#define _DRV_L1_CDSP				1
#define _DRV_L1_GSENSOR				0

// UART interface config
#define UART0_BAUD_RATE             115200
//#define UART0_BAUD_RATE             9600

#if GPDV_BOARD_VERSION == GPCV4247_WIFI
	#define TFT_WIDTH					640//480 // 640
	#define TFT_HEIGHT					360//272 // 360
#else
	#if(USE_PANEL_NAME == PANEL_T43)
	#define TFT_WIDTH					480
	#define TFT_HEIGHT					272
	#elif(USE_PANEL_NAME == PANEL_400X240_I80)
	#define TFT_WIDTH					400 //320
	#define TFT_HEIGHT					240
	#else
	#define TFT_WIDTH					320
	#define TFT_HEIGHT					240
	#endif
#endif

#define WIFI_JPEG_WIDTH				640  // 480
#define WIFI_JPEG_HEIGHT			360  // 272

#define TV_WIDTH					640
#define TV_HEIGHT					480

#define HDMI_WIDTH					1280
#define HDMI_HEIGHT					720

/* Share Pin Config*/
/*Position Config star*/
#define NAND_SHARE_MODE             NF_SHARE_MODE

#define NAND_CS_POS                 NF_CS_AS_BKCS1
#define NAND_CS_DRIVING             IO_DRIVING_4mA

#define NAND_PAGE_TYPE              NAND_LARGEBLK
#define NAND_WP_IO                  NAND_WP_PIN

#define NAND_CTRL_POS               NF_ALE_IOC6__CLE_IOC7__REB_IOC8__WEB_IOC9
#define NAND_CTRL_DRIVING           IO_DRIVING_4mA

/* only effect in nand "non-shard with SDRAM" MODE */
#define NAND_DATA5_0_POS            NAND_DATA5_0_AS_IOD5_0
#define NAND_DATA7_6_POS            NAND_DATA7_6_AS_IOD7_6
#define NAND_DATA_DRIVING           IO_DRIVING_4mA

#if DBG_MESSAGE == CUSTOM_ON
    #if (GPDV_BOARD_VERSION == DVP_V1_0) 
    #define UART_TX_RX_POS              UART1_TX_IOB7__RX_IOB6	
    #elif (GPDV_BOARD_VERSION == GPCV4247_WIFI) 
    #define UART_TX_RX_POS              UART0_TX_IOB5__RX_IOB4//UART1_TX_IOC15__RX_IOC14
    #else
    #define UART_TX_RX_POS              UART0_TX_IOB5__RX_IOB4
    #endif
#else
#define UART_TX_RX_POS              UART_TX_NONE__RX_NONE
#endif

#define UART_TX_RX_DRIVING          IO_DRIVING_4mA

#define TFT_DATA0_7_POS				TFT_DATA0_7__IOA0_7
#define TFT_DATA8_15_POS			TFT_DATA8_15__IOA8_15
#define TFT_CTRL_POS				TFT_DE_HSYNC_VSYNC_CLK_TE__IOB0_IOB1_IOB2_IOB3_IOB4
#define TFT_DATA0_7_DRIVING         IO_DRIVING_8mA
#define TFT_DATA8_15_DRIVING        IO_DRIVING_8mA
#define TFT_CTRL_DRIVING 			IO_DRIVING_8mA

#define CMOS_CLK0_POS					CMOS_CLK0__IOC9
#define CMOS_CLK1_HSYNC_VSTNC_POS	CMOS_CLK1_HSYNC_VSYNC__IOC8_IOC10_IOC11
#define CMOS_DATA2_9_POS				CMOS_DATA2_9__IOE0_7
#define CMOS_POS                    (CMOS_CLK0_POS|CMOS_CLK1_HSYNC_VSTNC_POS|CMOS_DATA2_9_POS)

#define CMOS_DRIVING                IO_DRIVING_4mA

#if (GPDV_BOARD_VERSION == GPCV1248_MINI)
#define SD_POS                      SDC1_IOD10_IOD11_IOC13_IOD13_IOC14_IOC15
#elif (GPDV_BOARD_VERSION == GPCV4247_WIFI) 
#define SD_POS                      SDC0_IOA2_IOA3_IOA4_IOA5_IOA6_IOA7
#else
#define SD_POS                      SDC1_IOD10_IOD11_IOD12_IOD13_IOD14_IOD15
#endif

#define SD_DRIVING                  IO_DRIVING_8mA /* IOC only 4mA/8mA can choice, IOD: 4mA/8mA/12mA/16mA can choice*/

#define SPI0_POS                    SPI0_RX_IOD13__CLK_IOD11__TX_IOD12 
#define SPI1_POS                    SPI1_RX_IOE3__CLK_IOE1__TX_IOE2
#define SPI1_DRIVING                IO_DRIVING_4mA  /* 4mA/8mA can choice */
#define SPI0_DRIVING                IO_DRIVING_4mA  /* 4mA/8mA can choice */


#define EXT_INT_POS                 EXT_INT_ABC_IOC13_15

/* I2C Position Define for GPCV1248 */
#define I2C_POS                     ENUM_IO_I2C_IOB4_5

#define BKCS_0_EN                   FALSE
#define BKCS_0_DRIVING              IO_DRIVING_4mA

#define BKCS_1_EN                   FALSE
#define BKCS_1_DRIVING              IO_DRIVING_4mA

#define BKCS_2_EN                   FALSE
#define BKCS_2_DRIVING              IO_DRIVING_4mA

#define BKCS_3_EN                   FALSE
#define BKCS_3_DRIVING              IO_DRIVING_4mA

#define TIMER_A_PWM_EN              FALSE
#define TIMER_A_PWM_DRIVING         IO_DRIVING_4mA /*4mA/8mA/12mA/16mA can choice*/

#define TIMER_B_PWM_EN              FALSE
#define TIMER_B_PWM_DRIVING         IO_DRIVING_4mA /*4mA/8mA/12mA/16mA can choice*/

#define TIMER_C_PWM_EN              FALSE
#define TIMER_C_PWM_DRIVING         IO_DRIVING_CANNOT_SET




//=== This is for code configuration DON'T REMOVE or MODIFY it =============================//
    // GPIO PAD
    #define NF_ALE_IOC12__CLE_IOC13__REB_IOC14__WEB_IOC15    0x00000000
    #define NF_ALE_IOC6__CLE_IOC7__REB_IOC8__WEB_IOC9        0x00000008
    #define NF_ALE_IOG5__CLE_IOG6__REB_IOG10__WEB_IOG11      0x10000008
    #define NF_CS_AS_BKCS1                  0x00000000
    #define NF_CS_AS_BKCS2                  0x00000010
    #define NF_CS_AS_BKCS3                  0x00000020

    #define NAND_LARGEBLK	 				0x00000000
    #define NAND_SMALLBLK  					0x00000001
    #define NAND_WP_PIN_NONE                0xFF
	#define NAND_WP_PIN						IO_E3//0x08//IO_A8

    #define NAND_DATA7_6_AS_IOB15_14        0x00000000
    #define NAND_DATA7_6_AS_IOD7_6          0x00000040
    #define NAND_DATA7_6_AS_IOE7_6          0x00000080
    #define NAND_DATA7_6_AS_IOC5_4          0x000000C0
    #define NAND_DATA5_0_AS_IOB13_8         0x00000000
    #define NAND_DATA5_0_AS_IOD5_0          0x00000100
    #define NAND_DATA5_0_AS_IOE5_0          0x00000200
    #define NAND_DATA7_0_AS_IOA15_8         0x000003C0               /* code-inter define */
    #define NAND_DATA7_6_AS_IOA15_14        NAND_DATA7_0_AS_IOA15_8  /* code-inter define */
    #define NAND_DATA5_0_AS_IOA13_8         NAND_DATA7_0_AS_IOA15_8  /* code-inter define */
    #define NAND_POSFUN_MASK                0x000003F8
    #define NF_SHARE_MODE  0
    #define NF_NON_SHARE   1
    #define NAND_POS_SET_VALUE  ((NAND_CS_POS|NAND_CTRL_POS|NAND_DATA5_0_POS|NAND_DATA7_6_POS)&NAND_POSFUN_MASK)

    // GPIO PAD_SETS DRIVING
    #define IO_DRIVING_CANNOT_SET  0  /* fix in 4mA */
    #define IO_DRIVING_4mA         0		// not 4mA, but 8mA
    #define IO_DRIVING_8mA         0
    #define IO_DRIVING_12mA        1	// not 12mA, but 16mA
    #define IO_DRIVING_16mA        1
	
	//SPI0
	#define SPI0_RX_IOD9__CLK_IOD7__TX_IOD8         0 
    #define SPI0_RX_IOC11__CLK_IOC9__TX_IOC10         1 
    #define SPI0_RX_IOD13__CLK_IOD11__TX_IOD12         2 
    
    //SPI1
    #define SPI1_RX_IOE3__CLK_IOE1__TX_IOE2 ~(1<<12)
    #define SPI1_RX_IOC4__CLK_IOC5__TX_IOC7  (1<<12)

    //UART
	#define UART0_TX_IOB5__RX_IOB4						0x00000001
	#define UART0_TX_IOC12__RX_IOC13					0x00000002
	#define UART0_TX_IOD5__RX_IOD4						0x00000003
	#define UART1_TX_IOB7__RX_IOB6						0x00000004
	#define UART1_TX_IOC15__RX_IOC14					0x00000005
	#define UART1_TX_IOD9__RX_IOD8						0x00000006
	#define UART1_TX_IOD15__RX_IOD14					0x00000007	
	#define UART_TX_NONE__RX_NONE						0x00000000

    /*TFT*/
	#define TFT_DATA0_7__IOA0_7							0x00000000
    #define TFT_DATA0_7__IOE0_7							0x00000001
    #define TFT_DATA8_15__IOA8_15						0x00000000
	#define TFT_DATA8_15__IOE0_7						0x00000001
    #define TFT_DE_HSYNC_VSYNC_CLK_TE__IOB0_IOB1_IOB2_IOB3_IOB4	0x00000000
    #define TFT_DE_HSYNC_VSYNC_CLK__IOC8_IOC9_IOC10_IOC11 0x00000001

    /*CMOS Pin Position Configuration*/
    #define CMOS_CLK0__IOC9				0x00000001
    #define CMOS_CLK0__IOD7				0x00000002
    #define CMOS_CLK0__IOD9				0xD0000003
    #define CMOS_CLK1_HSYNC_VSYNC__IOC8_IOC10_IOC11		0x00000100
    #define CMOS_CLK1_HSYNC_VSYNC__IOD6_IOD8_IOD9		0x00000200
    #define CMOS_DATA2_9__IOC0_7		0x00010000
    #define CMOS_DATA2_9__IOB8_15		0x00020000
    #define CMOS_DATA2_9__IOE0_7		0x00030000

    //SD Card
    /*===========================================
    SDC: 
        CMD_CLK_DATA3_DATA0_DATA1_DATA2
    ==============================================*/	
	#define SDC0_IOA2_IOA3_IOA4_IOA5_IOA6_IOA7			0x00000000
	#define SDC0_IOB14_IOB15_IOB10_IOB11_IOB12_IOB13	0x00000001	
	#define SDC0_IOC6_IOC7_IOC8_IOC9_IOC10_IOC11		0x00000002
	#define SDC1_IOD10_IOD11_IOD12_IOD13_IOD14_IOD15	0x00000003
	#define SDC1_IOB0_IOB1_IOB2_IOB3_IOB4_IOB5			0x00000004
	#define SDC1_IOD10_IOD11_IOC13_IOD13_IOC14_IOC15	0x00000005
	
	//============================================================
	//External interrupt A~C
	//============================================================

	#define EXT_INT_ABC_IOB8_10        	0x00000000				// External interrupt A~C=IOB8~IOB10(deafult)
	#define EXT_INT_ABC_IOC13_15		0x00000001				// External interrupt A~C=IOC13~IOC15
	#define EXT_INT_ABC_IOD7_9			0x00000002				// External interrupt A~C=IOD7~IOD9
	#define EXT_INT_ABC_IOD12_14_15		0x00000003				// External interrupt A=IOD12, External interrupt B~C=IOD14~IOD15

    //=============================================================
    /*Key Change Pin Position Configuration */
    #define KEYCHANGE_at_IOE1                           0x00000000
    #define KEYCHANGE_at_IOF3                           0x00001000
    #define KEYCHANGE_POSFUN_MASK                       0xFFFFEFFF
    #define KEYCHANGE_POS_SET_VALUE                     KEYCHANGE_POS
    #define KEY_CHANGE_B_AT_IOF3              0x00000000
    #define KEY_CHANGE_B_AT_IOE1              (1<<12)



#define ENABLE_CHECK_RTC				1	


typedef enum { //GPCV1248 I2C
        ENUM_IO_I2C_IOB4_5 = 0x0,                                        // I2C Clock=IOB4, DATA=IOB5(deafult) 
        ENUM_IO_I2C_IOC14_15,                                            // I2C Clock=IOC14, DATA=IOC15 
        ENUM_IO_I2C_IOD4_5 = 0x4,                                        // I2C Clock=IOD4, DATA=IOD5 
        ENUM_IO_I2C_IOD8_9 = 0x6,                                        // I2C Clock=IOD8, DATA=IOD9 
        ENUM_IO_I2C_IOD14_15 = 0x7                                       // I2C Clock=IOD14, DATA=IOD15 
} IO_I2C_PIN_ENUM; 


//==========================================================================================//


#endif      // __DRIVER_L1_CFG_H__

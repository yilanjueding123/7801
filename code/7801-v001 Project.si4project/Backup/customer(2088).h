#ifndef __CUSTOMER_H__
#define __CUSTOMER_H__

#define CUSTOM_ON       1
#define CUSTOM_OFF      0

#define DBG_MESSAGE						CUSTOM_ON	// UART debug message

//configure
#define DVP_V1_0						1
#define GPCV1248_V1_0				    2
#define GPCV1248_MINI              		3
#define GPCV4247_WIFI              		4
#define GPDV_BOARD_VERSION				GPCV4247_WIFI

// CPU and system clock
#define INIT_MHZ                    	144  // MHz

// SDRAM size: 2MB(0x00200000), 8MB(0x00800000), 16MB(0x01000000), 32MB(0x02000000)
#define SDRAM_SIZE						0x00800000

//wifi encryption method
#define WPA2                            1
#define WEP                             2
#define WIFI_ENCRYPTION_METHOD          WPA2
#define WIFI_SSID_ADD_MACADDR           1

// panel
#define PANEL_T27P05_ILI8961			0
#define PANEL_TPO_TD025THEA7			1	
#define PANEL_T43						2	
#define PANEL_400X240_I80				3	
#define PANEL_T40P00_ILI9342C	        4
#define PANEL_T20P82_ST7789V            5
#define PANEL_T15P06                    6

#define PANEL_SCREEN_PROPOTION_4_3		0
#define PANEL_SCREEN_PROPOTION_16_9		1
			
#define LDW_DIS_DYNAMIC_DRAW			0

#define G_SENSOR_DA380      0
#define G_SENSOR_DMARD07    1

/*Sensor Type*/
#define	SENSOR_OV7670		0 	
#define	SENSOR_OV9712		1
#define SENSOR_SOI_H22		2
#define SENSOR_BF3703		3
#define SENSOR_SOI_H22_MIPI	4
#define	SENSOR_OV3640		5
#define	SENSOR_OV5642		6
#define SENSOR_GC1004		7
#define SENSOR_GC1004_MIPI	8
#define SENSOR_SOI_H42		9
#define SENSOR_GC1014		10


/*Sensor Format*/
//max resolution: 640*480				
#define OV7670_YUYV					0x00
#define OV7670_BAYER_RAW			0x01
#define OV7670_RGRB					0x04
#define OV7670_PROCESS_BAYER_RAW	0x05

//max resolution: 2048*1536
#define OV3640_YUYV					0x00
#define OV3640_UYVY					0x02
#define OV3640_BGGR					0x18
#define OV3640_GBRG					0x19
#define OV3640_GRBG					0x1A
#define OV3640_RGGB					0x1B

//max resolution: 2592*1944
#define OV5642_UYVY					0x32
#define OV5642_BGGR					0x00
#define OV5642_GBRG					0x01
#define OV5642_GRBG					0x02
#define OV5642_RGGB					0x03
#define OV5642_MIPI_YUV				0x4032
#define OV5642_MIPI_GRBG			0x4002
//#define OV5642_MIPI_2LANE_YUV		0x8032
//#define OV5642_MIPI_2LANE_GRGB		0x8002

//max resolution: 1296*800	
#define OV9712_RAW					0x01

//max resolution 1296*816
#define SOI_H22_RAW					0x01
#define SOI_H22_MIPI				0x02

//max resolution 1296*816
#define SOI_H42_RAW					0x01
#define SOI_H42_MIPI				0x02

//max resolution 1296*742
#define GC1004_RAW					0x01
#define GC1004_MIPI					0x02

//max resolution 1296*742
#define GC1014_RAW					0x01
#define GC1014_MIPI					0x02

//max resolution 640*480
#define BF3703_RAW					0x01

//max resolution: 640*480				
#define OV7725_YUYV					0x00
#define OV7725_BAYER_RAW			0x01
#define OV7725_RGRB					0x04
#define OV7725_PROCESS_BAYER_RAW	0x05

#define COLOR_BAR					0x22


#if 0
	#define SENSOR_WIDTH	640
	#define SENSOR_HEIGHT	480
#elif 1
	#define SENSOR_WIDTH	1280
	#define SENSOR_HEIGHT	720
#elif 0
	#define SENSOR_WIDTH	1280
	#define SENSOR_HEIGHT	800	
#elif 0
	#define SENSOR_WIDTH	1920
	#define SENSOR_HEIGHT	1080
#elif 0 
	#define SENSOR_WIDTH	2048
	#define SENSOR_HEIGHT	1536
#else
	#define SENSOR_WIDTH	2592
	#define SENSOR_HEIGHT	1944
#endif

#define SENSOR_BUF_FRAME_MODE			1		// Frame mode
#define SENSOR_BUF_FIFO_MODE			2		// FIFO mode
#define VIDEO_ENCODE_USE_MODE			SENSOR_BUF_FRAME_MODE

// AVI recording resolution: 640x480, 720x480, 1280x720
#define AVI_WIDTH_1080FHD				1920
#define AVI_HEIGHT_1080FHD				1080

#define AVI_WIDTH_1080P					1440
#define AVI_HEIGHT_1080P				1080

#define AVI_WIDTH_720P					1280
#define AVI_HEIGHT_720P					720

#define AVI_WIDTH_WVGA					848
#define AVI_HEIGHT_WVGA					480

#define AVI_WIDTH_VGA					640
#define AVI_HEIGHT_VGA					480

#define AVI_WIDTH_QVGA					320
#define AVI_HEIGHT_QVGA					240


#define AVI_Y_Q_VALUE_PC_CAM				50

#define AVI_Q_VALUE_CAPTURE_BEST			98
#define AVI_Q_VALUE_CAPTURE_NORNAL		90
#define AVI_Q_VALUE_CAPTURE_LOW			80

#define IndexBuffer_Size				(32*1024)	// Do not modify this
#define AVI_FILE_MAX_RECORD_SIZE		1001*1024*1024	//2GB
#define AVI_ENCODE_SHOW_TIME			0

// Photo resolution and quality
//#define PIC_WIDTH						640
//#define PIC_HEIGH						480
#define PIC_Q_VALUE						70		// 30, 40, 50, 60, 70, 80, 90

// Motion detection
#define MD_STOP_TIME					10		//单位: 秒// when MD_STOP_TIME seconds detect no motion then stop video record
//motion detect sensitive
#define LOW								300
#define MEDIUM							150
#define HIGH							90	//70
#define MD_SENSITIVE					HIGH

// Storage type
#define MINI_DVR_STORAGE_TYPE			2		// T_FLASH
#define RAMDISK_TYPE					3		// SDRAM



// key detection
#define KEY_ACTIVE						DATA_LOW
#define KEY_TYPE0                       0
#define KEY_TYPE1                    	1
#define KEY_TYPE2                    	2
#define KEY_TYPE3	                    3
#define KEY_TYPE4	                    4
#define KEY_TYPE5	                    5

#define READ_FROM_PIN		            0
#define READ_FROM_GPIO		            1
#define	FUNCTION_KEY					AD_KEY_DETECT_PIN
#define NEXT_KEY						AD_KEY_DETECT_PIN
#define PREVIOUS_KEY					AD_KEY_DETECT_PIN
#define	MENU_KEY						AD_KEY_DETECT_PIN
#define	CAPTURE_KEY				    	AD_KEY_DETECT_PIN
//#define OK_KEY							AD_KEY_DETECT_PIN
#define SOS_KEY							AD_KEY_DETECT_PIN
#define C6_KEY							1
#define SAMPLE2							2
#define KEY_FUNTION_TYPE				C6_KEY

// GPIO
//========================================================================
	#define USE_PANEL_NAME					PANEL_TPO_TD025THEA7
	#define USE_PANEL_SCREEN_PROPOTION		PANEL_SCREEN_PROPOTION_4_3
	#if 0
	#define USE_SENSOR_NAME		            SENSOR_SOI_H22 
	#define sensor_format			     	SOI_H22_RAW
	#define SENSOR_INTERFACE				SENSOR_RAW10
	#elif 0
	#define USE_SENSOR_NAME		            SENSOR_GC1004 
	#define sensor_format				    GC1004_RAW
	#define SENSOR_INTERFACE				SENSOR_RAW10		
	#elif 1
	#define USE_SENSOR_NAME		            SENSOR_GC1004 
	#define sensor_format				    GC1004_MIPI
	#define SENSOR_INTERFACE				MIPI_RAW10		
	#elif 0
	#define USE_SENSOR_NAME		            SENSOR_OV9712 
	#define sensor_format				    OV9712_RAW
	#define SENSOR_INTERFACE				SENSOR_RAW10
	#elif 0
	#define USE_SENSOR_NAME		            SENSOR_GC1014 
	#define sensor_format				    GC1014_RAW
	#define SENSOR_INTERFACE				SENSOR_RAW10
	#else
	#define USE_SENSOR_NAME		            SENSOR_SOI_H42 
	#define sensor_format			     	SOI_H42_RAW
	#define SENSOR_INTERFACE				SENSOR_RAW10
	#endif
	#define SENSOR_FLIP				        0
	#define USE_G_SENSOR_NAME		        G_SENSOR_DA380 	
	#define USE_G_SENSOR_ACTIVE				1
	#define CPU_DRAW_TIME_STAMP_BLACK_EDGE	0
	#define RENAME_LOCK_FILE				1
	#define LOCK_FILE_NAME                  0 //0:LOCK+index  1:SOS0+index
	#define USE_IOKEY_NO					1
	#define USE_ADKEY_NO					1
	#define KEY_TYPE						KEY_TYPE1
	#define PWR_KEY_TYPE					READ_FROM_PIN
	#define PW_KEY						    PWR_KEY0
	#define ADP_OUT_PIN                     PWR_KEY1
	#define C_USBDEVICE_PIN   				ADP_OUT_PIN
	#define AD_KEY_DETECT_PIN			    ADC_LINE_1
	#define AD_BAT_DETECT_PIN			    ADC_LINE_0

	#define SCCB_SCL						IO_A0// Sensor I2C clock pin
	#define SCCB_SDA						IO_A1// Sensor I2C data pin	

	#define I2C_SCL     					IO_F15	// G_Sensor I2C clock pin
	#define I2C_SDA      				    IO_F15	// G_Sensor I2C data pin
    
    #define TV_OUT_MENU						1	//please change resource
	#define TV_DET_ENABLE					0
    #define	TV_DET_ACTIVE			        1      //tv_detect active level
    #define AV_IN_DET                     	IO_F15//IO_B3 
    
    #define POWER_EN_PIN					IO_B2
    #define CHARGE_DETECTION_PIN			IO_B3
    
    #define OK_KEY							IO_F15//
    #define WIFI_KEY						IO_F15
    
    #define OK_KEY_ACTIVE					0
	#define WIFI_KEY_ACTIVE					0

	#define TFT_CSN_PIN						IO_F15
	#define TFT_SDA_PIN						IO_F15
	#define TFT_SCL_PIN						IO_F15
    
    #define HDMI_IN_DET                     IO_F15
    #define TFT_BL							IO_F15	
    #define SPEAKER_EN                      IO_F15
    
    #define WIFI_LDO_EN						IO_C14	
    
    #define IR_CTRL                         IO_F15   //ir light 

	#define I80_TE_PIN						IO_F15	
	#define SDC_UPGRADE_ERASE_USER_CONFIG	1
	
	#define POWERON_BATTERY_CHECK           1
	
	#define IR_PIN_TYPE						0//1
//====================Liuxi modified in 2015-01-15==================================    
	//LED 有效电平; =0,低有效; =1,高有效	
    #define LED1							IO_F15 //blue
    #define LED2							IO_F15 //red
    #define LED3							IO_F15 //yellow
	#define WIFI_LED						IO_E1 //green
#if IR_PIN_TYPE	
    #define IR_LED1							IO_F15//IO_C14 //ir led //green
#else
    #define IR_LED1							IO_F15//IO_C14 //ir led //green
#endif
    #define LED1_ACTIVE                     1     
	#define LED2_ACTIVE						1		  
	#define LED3_ACTIVE						1		  
	#define WIFI_LED_ACTIVE					1	  
	#define IR_LED_ACTIVE 					1	  	    

	//#define IR_MK6A12P_CLK					IO_E1//PIN6
	//#define IR_MK6A12P_DATA					IO_E0//PIN7    
//========================================================================
//没有使用的IO设置为输入下拉, 并将其接到GND上, 有利于散热
//注意复用的口要设置成通用IO
    #define COOLER_PIN_NUM			5
    
    #define COOLER_PIN_IO_01	    IO_E3			
    #define COOLER_PIN_IO_02	    IO_E4		
    #define COOLER_PIN_IO_03	    IO_E5		
    #define COOLER_PIN_IO_04	    IO_E6			
    #define COOLER_PIN_IO_05	    IO_E7				
    #define COOLER_PIN_IO_06	    IO_F15			
    #define COOLER_PIN_IO_07	    IO_F15			
    #define COOLER_PIN_IO_08	    IO_F15			
    #define COOLER_PIN_IO_09	    IO_F15			
    #define COOLER_PIN_IO_10	    IO_F15			    
    #define COOLER_PIN_IO_11	    IO_F15			    
//========================================================================
#define WIFI_SSID_NAME					"JH-7801A" //8个字符

#define PRODUCT_NUM						7801
#define PRODUCT_DATA					20180110
#define PROGRAM_VERSION_NUM				1 	

//+++
#define JPEG_REC_TIME						0
#define JPEG_AE_GAIN						1
#define JPEG_TIME_STAMP_SHOW				JPEG_REC_TIME
//---

// Function enable
#define C_LOGO							CUSTOM_ON
#define C_UVC							CUSTOM_ON
#define C_USB_AUDIO						CUSTOM_ON
#define C_MOTION_DETECTION				CUSTOM_OFF
#define C_AUTO_DEL_FILE					CUSTOM_ON
#define C_CYCLIC_VIDEO_RECORD			CUSTOM_ON
#define C_SCREEN_SAVER					CUSTOM_ON
#define C_POWER_OFF_LOGO				CUSTOM_OFF
#define C_BATTERY_DETECT				CUSTOM_ON
#define C_BATTERY_LOW_POWER_OFF			CUSTOM_ON
#define C_DISPLAY_REVERSE               CUSTOM_ON
#define SUPORT_GET_JPG_BUF				CUSTOM_ON
#define SUPPORT_MODIFY_VOLUME_NAME      CUSTOM_OFF
#define SUPPORT_JTAG					CUSTOM_OFF	// ICE
#define AUD_RECORD_EN					CUSTOM_OFF //暂无此功能
#define C_IR_REMOTE						CUSTOM_OFF //红外遥控功能
#define POWER_ON_WIFI_ENABLE			CUSTOM_ON

#if DBG_MESSAGE
	#define DBG_PRINT	print_string
	//#define __msg	print_string
	#define __here__	print_string("L%d, %s\n", __LINE__, __func__)
	
	#define __msg(...)    		(print_string("MSG:L%d(%s):", __LINE__, __func__),                 \
							     print_string(__VA_ARGS__)									        )
#else
	#define DBG_PRINT	if (0) print_string
//#define __msg	if (0) print_string
#define __msg(...)
#define __here__ if (0) print_string
#endif

#if GPDV_BOARD_VERSION == GPCV1248_V1_0
	#define UART_USED_NUM                   0  // 0 or 1
#else
	#define UART_USED_NUM                   0  // 0 or 1
#endif
#if GPDV_BOARD_VERSION == GPCV4247_WIFI
#define SD_USED_NUM                     0
#else
#define SD_USED_NUM                     1  // 0 or 1
#endif

#define MJ_DECODE_SCALER                0  // SCALER_0
#define UNDEFINE_SCALER                 0  // SCALER_0  

//#define PWM_CTR_LED

//+++
#define ENABLE_SAVE_SENSOR_RAW_DATA		0
#define CDSP_DEBUG  					0

#define WIFI_FUNC_ENABLE				1
#define PRINTF_WIFI_SPEED_ENABLE		0


#endif //__CUSTOMER_H__

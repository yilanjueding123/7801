/******************************************************
* drv_l2_usbd_msdc_vendor.h
*
* Purpose: MSDC vendor command header file
*
* Author: Eugene Hsu
*
* Date: 2013/03/06
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#ifndef DRV_L2_USBD_MSDC_VENDOR_H
#define DRV_L2_USBD_MSDC_VENDOR_H

/*********************************************************************
        Structure, enumeration and definition
**********************************************************************/
typedef enum
{
	/* 0xF0xx */
	GP_READ_DRV_INFO					= 0xF000,
	GP_GET_DRV_STATUS					= 0xF001,
	GP_GET_IC_VER						= 0xF010,
	GP_SET_VENDOR_ID					= 0xF0F0,
	GP_SET_RTC_TIME						= 0xF0FF,
	
	/*0xF5xx*/
	GP_DEMO_READ_WRITE					= 0xF500,
	GP_DEMO_READ_DATA					= 0xF501,
	
	/* 0xFDxx*/
	GP_READ_MEMORY						= 0xFD28,
	GP_WRITE_MEMORY						= 0xFD2A,
	
	/* 0xF8xx */
	GP_NV_BTHDR_WRITE_DATA				= 0xF810,
	GP_NV_BTHDR_WRITE_STEP				= 0xF811,
	GP_NV_BTLDR_WRITE_DATA 				= 0xF812,
	GP_NV_BTLDR_WRITE_STEP				= 0xF813,
	GP_NV_APP_FORMAT_STEP				= 0xF814,
	GP_NV_APP_INIT_STEP					= 0xF815,
	GP_NV_DATA_FORMAT_STEP				= 0xF816,
	GP_NV_DATA_INIT_STEP				= 0xF817,
	GP_NV_CHIP_ERASE					= 0xF818,
	GP_NV_BTLDR_WRITE_RECEIVE_DATA		= 0xF820,
	GP_NV_BTLDR_WRITE_GET_STEP			= 0xF821,
	GP_NV_BTLDR_WRITE_STEP_BY_STEP		= 0xF822,
	GP_NV_APPAREA_WRITE					= 0xF830,
	GP_NV_DATAAREA_WRITE				= 0xF840,
	
	/* 0xFExx */
	GP_LOGGER_READ						= 0xFEAA,
	
	/* 0xFFxx*/
	GP_NV_ERASE							= 0xFF20,
	GP_NV_INIT_STEPS_GET				= 0xFF21,
	GP_NV_INIT							= 0xFF22,
	GP_LOW_LEVEL_FORMAT_STEPS_GET 		= 0xFF23,
	GP_DATA_AREA_LOW_LEVEL_FORMAT 		= 0xFF24,
	GP_APP_AND_DATA_AREA_TOTAL_SIZE_GET = 0xFF25,
	GP_APP_WRITE_FLUSH					= 0xFF26,
	GP_NV_READ							= 0xFF28,
	GP_NV_WRITE_FLUSH					= 0xFF29,
	GP_NV_WRITE							= 0xFF2A,
	GP_REG_VAL_GET						= 0xFF30,
	GP_APP_INIT_STEPS_GET				= 0xFF33,
	GP_APP_INIT_STEP_WORK				= 0xFF34,
	GP_APP_INIT							= 0xFF35,
	GP_CHECKSUM_ZERO_CMD				= 0xFF3A,
	GP_NV_INFO_DUMP						= 0xFF50,
	GP_NV_BOOT_HDR_PARSE				= 0xFF51,
	GP_NF_BAD_BLK_INIT					= 0xFF52,
	GP_NF_BAD_BLK_STEP					= 0xFF53,
	GP_NF_BAD_BLK_STOP					= 0xFF54,
	GP_NV_NAND_TABLE					= 0xFF55,
	GP_NV_APP_HDR_PARSE					= 0xFF56,
	GP_NV_ISP_DOWN_TYPE					= 0xFF60,
	GP_USB_JUMP_CMD						= 0xFFFF
	
} EN_GP_VENDOR_CMD;
	

typedef enum
{
	GP_NV_NONE,
	GP_NV_OTP,
	GP_NV_NAND,
	GP_NV_SPI,
	GP_NV_SD
} EN_GP_NV_STORAGE_TYPE;	

typedef struct _ISP_VENDOR_CTL_BLK
{
	INT32U nv_type;
	INT32S (*nv_init)(void);
	INT32S (*nv_read)(INT32U addr, INT32U len, void* buf, void* pri);	  	
	INT32S (*nv_write)(INT32U addr, INT32U len, void* buf, void* pri);
	INT32S (*nv_erase)(INT32U blkid);	  		  	
} ISP_VENDOR_CTL_BLK;


/*********************** Extern function declaration **********************************/


#endif  //DRV_L2_USBD_MSDC_VENDOR_H

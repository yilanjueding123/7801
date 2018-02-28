/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

#define _PORTING_C_
#include <rtos.h>
#include <porting.h>
#include <ssv_lib.h>

#define APP_INCLUDE_WIFI_FW

/*=============console==================*/
u8 hal_getchar(void)
{
    u8 data=0;
#if (HOST_PLATFORM_SEL == GP_19B)
    if(STATUS_OK == uart0_data_get(&data, 0))
#elif (HOST_PLATFORM_SEL == GP_15B)
		#if  (GPDV_BOARD_VERSION == GPCV1248_V1_0)	
    if(STATUS_OK == uart_data_get(0,&data, 0))    
	#else	// EVB
    if(STATUS_OK == drv_l1_uart1_data_get(&data, 0))
#endif
#endif
        return data;
    else
        return 0;
}

/*=============Memory==================*/
extern void * gp_malloc(u32 size);                          // SDRAM allocation
OS_APIs void *OS_MemAlloc( u32 size )
{
    /**
        *  Platform dependent code. Please rewrite
        *  this piece of code for different system.
        */
    void * ptr = NULL;
    ptr = gp_malloc(size);
    if (ptr) gp_memset(ptr,0,size);
    return ptr;
}

extern void gp_free(void *ptr);                                 // Both SDRAM and IRAM can be freed by this function
/**
 *  We do not recommend using OS_MemFree() API
 *  because we do not want to support memory
 *  management mechanism in embedded system.
 */
OS_APIs void __OS_MemFree( void *m )
{
    /**
        *  Platform depedent code. Please rewrite
        *  this piece of code for different system.
        */
    gp_free(m);
}

void OS_MemCPY(void *pdest, const void *psrc, u32 size)
{
    OS_MemCopy((void*)pdest,(void*)psrc,size);
}

void OS_MemSET(void *pdest, u8 byte, u32 size)
{
    gp_memset(pdest,byte,size);
}

//=====================Platform LDO EN ping setting=======================
#if (HOST_PLATFORM_SEL == GP_19B)
 #define SSV_LDO_EN_PIN              IO_A14
#elif (HOST_PLATFORM_SEL == GP_15B)
 #define SSV_LDO_EN_PIN              IO_F15
#elif (HOST_PLATFORM_SEL == LINUX_SIM)
 #undef SSV_LDO_EN_PIN
#endif

#include "drv_l1_gpio.h"
void platform_ldo_en_pin_init(void)
{
#ifdef SSV_LDO_EN_PIN
#if (HOST_PLATFORM_SEL == GP_19B)
    gpio_set_memcs(1,0); //cs1 set as IO
#endif
    gpio_init_io(SSV_LDO_EN_PIN,GPIO_OUTPUT);
    gpio_set_port_attribute(SSV_LDO_EN_PIN, 1);
    gpio_write_io(SSV_LDO_EN_PIN, 0);
#endif
}

void platform_ldo_en(bool en)
{
    if(en)
        wal_ldo_enable();
    else
       wal_ldo_disable();
}

//=====================find fw to download=======================
#if 1
#include <ssv_hal.h>
bool platform_download_firmware(void)
{
    return ssv6xxx_download_fw((u8 *)SSV_HAL_FW_BIN,sizeof(SSV_HAL_FW_BIN));

#else  //this option is to set fw.bin as resource to open. please check resource_wifi_fw.s
void platform_download_firmware(void)
{
    u32 fw_size;
    extern u8 *RES_WIFI_FW_START;
    extern u8 *RES_WIFI_END;

    fw_size = ((u32)&RES_WIFI_END) - ((u32)&RES_WIFI_FW_START);
    LOG_PRINTF("fw_size=%d\r\n",fw_size);
    ssv6xxx_download_fw((u8 *)&RES_WIFI_FW_START,fw_size);//??? u8* bin
#endif
}
bool ssv6xxx_drv_load_fw(void)
{
    return ssv6xxx_download_fw((u8 *)SSV_HAL_FW_BIN,sizeof(SSV_HAL_FW_BIN));
}

s32 ssv6xxx_set_rate_mask(u16 mask)
{    
    return ssv6xxx_set_rc_value(RC_RATEMASK, mask);
}

void platform_read_firmware(void *d,void *s,u32 len)
{
    OS_MemCPY(d,s,len);
}


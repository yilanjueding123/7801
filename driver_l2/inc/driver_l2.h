#ifndef __DRIVER_L2_H__
#define __DRIVER_L2_H__

#include "driver_l1.h"
#include "driver_l2_cfg.h"
#include "drv_l2_sd.h"
#include "drv_l2_cdsp.h"

// Driver layer functions
extern void drv_l2_init(void);

#if (defined _DRV_L2_EXT_RTC) && (_DRV_L2_EXT_RTC == 1)
typedef struct
{
    INT32S tm_sec;  /* 0-59 */
    INT32S tm_min;  /* 0-59 */
    INT32S tm_hour; /* 0-23 */
    INT32S tm_mday; /* 1-31 */
    INT32S tm_mon;  /* 1-12 */
    INT32S tm_year;
    INT32S tm_wday; /* 0-6 Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday */
}EXTERNAL_RTC_TIME;

typedef struct
{
    INT32S tm_min;  	/* 0-59 */
    INT32S tm_hour; 	/* 0-23 */
    INT32S tm_weekday;	/* 0- 6 */	//0 means sunday.
}EXTERNAL_RTC_ALRAM;

extern void drv_l2_external_rtc_init(void);
//==================================//
//function: drv_l2_rtc_first_power_on_check
//return: 1 means i2c first power on,
//==================================//
extern INT32S drv_l2_rtc_first_power_on_check(void);
extern INT32S drv_l2_rtc_total_set(EXTERNAL_RTC_TIME *ptr);
extern INT32S drv_l2_rtc_total_get(EXTERNAL_RTC_TIME *ptr);

#if DRV_L2_EXT_ALAEM_EN
extern INT32S drv_l2_rtc_alarm_time_set(EXTERNAL_RTC_ALRAM *ptr);
extern INT32S drv_l2_rtc_alarm_time_get(EXTERNAL_RTC_ALRAM *ptr);
#endif
#endif

// system control function
#define MAX_SYSTEM_TIMER		7
extern void sys_init_timer(void);
extern INT32S sys_set_timer(void* msg_qsend, void* msg_q_id, INT32U msg_id, INT32U timerid, INT32U interval);
extern INT32S sys_kill_timer(INT32U timerid);
extern INT32S sys_kill_all_timer(void);

extern INT32S sys_registe_timer_isr(void (*TimerIsrFunction)(void));
extern INT32S sys_release_timer_isr(void (*TimerIsrFunction)(void));
extern INT32S sys_128Hz_timer_init(void);

// spi flash driver
extern INT32S SPI_Flash_init(void);
// byte 1 is manufacturer ID,byte 2 is memory type ID ,byte 3 is device ID
extern INT32S SPI_Flash_readID(INT8U* Id);
// sector earse(4k byte)
extern INT32S SPI_Flash_erase_sector(INT32U addr);
// block erase(64k byte)
extern INT32S SPI_Flash_erase_block(INT32U addr);
// chip erase
extern INT32S SPI_Flash_erase_chip(void);
// read a page(256 byte)
extern INT32S SPI_Flash_read_page(INT32U addr, INT8U *buf);
// write a page(256 byte)
extern INT32S SPI_Flash_write_page(INT32U addr, INT8U *buf);
// read n byte
extern INT32S SPI_Flash_read(INT32U addr, INT8U *buf, INT32U nByte);

#endif		// __DRIVER_L2_H__

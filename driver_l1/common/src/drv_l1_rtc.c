/* 
* Purpose: RTC driver/interface
*
* Author: Allen Lin
*
* Date: 2008/5/9
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
* History :
*/

//Include files
#include "drv_l1_rtc.h"

extern void print_string(CHAR *fmt, ...);

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_RTC) && (_DRV_L1_RTC == 1)                   //
//================================================================//

// Constant definitions used in this file only go here

// Type definitions used in this file only go here

// Global inline functions(Marcos) used in this file only go here

// Declaration of global variables and functions defined in other files

// Variables defined in this file

// Functions defined in this file

/****************************************************
*		marco										*
****************************************************/
#define CHECK_RETURN(x) \
{\
	ret = x;\
	if(ret < 0) {\
		goto __exit;\
	}\
}\


#if _OPERATING_SYSTEM == 1	
static OS_EVENT		*sw_rtc_sem = NULL;
#endif

INT8U ext_rtc_pwr_on_flag = 0;

void (*rtc_user_isr[RTC_INT_MAX])(void);

static void rtc_isr()
{
	INT32U int_status = 0;
	INT32U int_ctrl = 0;
	
	int_status = R_RTC_INT_STATUS;
	int_ctrl = R_RTC_INT_CTRL;
		
	if (int_status & RTC_HALF_SEC_IEN) {
		R_RTC_INT_STATUS = RTC_HALF_SEC_IEN;
		if (rtc_user_isr[RTC_HSEC_INT_INDEX] && (int_ctrl & RTC_HALF_SEC_IEN)) {
			(*rtc_user_isr[RTC_HSEC_INT_INDEX])();
		}
	}	
	
	if (int_status & RTC_SEC_IEN) {
		R_RTC_INT_STATUS = RTC_SEC_IEN;
		if (rtc_user_isr[RTC_SEC_INT_INDEX]&& (int_ctrl & RTC_SEC_IEN)) {
			(*rtc_user_isr[RTC_SEC_INT_INDEX])();
		}
	}
	
	if (int_status & RTC_MIN_IEN) {
		R_RTC_INT_STATUS = RTC_MIN_IEN;
		if (rtc_user_isr[RTC_MIN_INT_INDEX]&& (int_ctrl & RTC_MIN_IEN)) {
			(*rtc_user_isr[RTC_MIN_INT_INDEX])();
		}
	}
	
	if (int_status & RTC_HR_IEN) {
		R_RTC_INT_STATUS = RTC_HR_IEN;
		if (rtc_user_isr[RTC_HR_INT_INDEX]&& (int_ctrl & RTC_HR_IEN)) {
			(*rtc_user_isr[RTC_HR_INT_INDEX])();
		}
	}
	
	if (int_status & RTC_DAY_IEN) {
		R_RTC_INT_STATUS = RTC_DAY_IEN;
		if (rtc_user_isr[RTC_DAY_INT_INDEX]&& (int_ctrl & RTC_DAY_IEN)) {
			(*rtc_user_isr[RTC_DAY_INT_INDEX])();
		}
	}
	
	if (int_status & RTC_SCH_IEN) {
		R_RTC_INT_STATUS = RTC_SCH_IEN;
		if (rtc_user_isr[RTC_SCH_INT_INDEX]) {
			(*rtc_user_isr[RTC_SCH_INT_INDEX])();
		}
	}
		
	if (int_status & RTC_ALM_IEN) {
		R_RTC_INT_STATUS = RTC_ALM_IEN;
		if (rtc_user_isr[RTC_ALM_INT_INDEX]) {
			(*rtc_user_isr[RTC_ALM_INT_INDEX])();
		}
	}	
#if 0
	INT32U int_status = 0;
	INT32U int_en = 0;
	
	int_status = gpx_rtc_read(0x9);
	int_en = gpx_rtc_read(0xA);
		
	if (int_status & GPX_RTC_HALF_SEC_IEN) {
		gpx_rtc_write(0x9,GPX_RTC_HALF_SEC_IEN);
		if (rtc_user_isr[RTC_HSEC_INT_INDEX] && (int_en & GPX_RTC_HALF_SEC_IEN)) {
			(*rtc_user_isr[RTC_HSEC_INT_INDEX])();
		}
	}	
	
	if (int_status & GPX_RTC_SEC_IEN) {
		gpx_rtc_write(0x9,GPX_RTC_SEC_IEN);
		if (rtc_user_isr[RTC_SEC_INT_INDEX]&& (int_en & GPX_RTC_SEC_IEN)) {
			(*rtc_user_isr[RTC_SEC_INT_INDEX])();
		}
	}
	
	if (int_status & GPX_RTC_MIN_IEN) {
		gpx_rtc_write(0x9,GPX_RTC_MIN_IEN);
		if (rtc_user_isr[RTC_MIN_INT_INDEX]&& (int_en & GPX_RTC_MIN_IEN)) {
			(*rtc_user_isr[RTC_MIN_INT_INDEX])();
		}
	}
	
	if (int_status & GPX_RTC_HR_IEN) {
		gpx_rtc_write(0x9,GPX_RTC_HR_IEN);
		if (rtc_user_isr[RTC_HR_INT_INDEX]&& (int_en & GPX_RTC_HR_IEN)) {
			(*rtc_user_isr[RTC_HR_INT_INDEX])();
		}
	}
	
	if (int_status & GPX_RTC_DAY_IEN) {
		gpx_rtc_write(0x9,GPX_RTC_DAY_IEN);
		if (rtc_user_isr[RTC_DAY_INT_INDEX]&& (int_en & GPX_RTC_DAY_IEN)) {
			(*rtc_user_isr[RTC_DAY_INT_INDEX])();
		}
	}
	
	if (int_status & GPX_RTC_ALM_IEN) {
		gpx_rtc_write(0x9,GPX_RTC_ALM_IEN);
		if (rtc_user_isr[RTC_ALM_INT_INDEX]) {
			(*rtc_user_isr[RTC_ALM_INT_INDEX])();
		}
	}
	
	if (R_RTC_INT_STATUS & RTC_SCH_IEN) {
		R_RTC_INT_STATUS = RTC_SCH_IEN;
		if (rtc_user_isr[RTC_SCH_INT_INDEX]) {
			(*rtc_user_isr[RTC_SCH_INT_INDEX])();
		}
	}
#endif
}

#if _OPERATING_SYSTEM == 1	
void sw_rtc_lock(void)
{
	INT8U err;

	OSSemPend(sw_rtc_sem, 0, &err);
}

void sw_rtc_unlock(void)
{
	OSSemPost(sw_rtc_sem);
}
#endif

#ifdef PWM_CTR_LED 
extern void ap_peripheral_PWM_LED_low(void);
#endif
void rtc_init(void)
{
	INT32U dwTimeOut ;
	t_rtc rtc_time = {0};
	
	R_RTC_CTRL= 0; /* disable all RTC function */
	R_RTC_CTRL |= RTC_RTCEN; /* enable RTC */
	R_RTC_CTRL |= RTC_HMSEN;

	ext_rtc_init();
#ifdef PWM_CTR_LED 
	ap_peripheral_PWM_LED_low();
#endif
	ext_rtc_time_get(&rtc_time);

	// update time to RTC
	R_RTC_MIN = (INT32U)rtc_time.rtc_min;
	dwTimeOut=0;
	while(R_RTC_BUSY & RTC_MIN_BUSY) {  /* wait until not busy */
		if (dwTimeOut++ > 100000) break ;
	}
	
	R_RTC_HOUR = (INT32U)rtc_time.rtc_hour;
	dwTimeOut=0;
	while(R_RTC_BUSY & RTC_HR_BUSY) {
		if (dwTimeOut++ > 100000) break ;
	}
	
	R_RTC_SEC = (INT32U)rtc_time.rtc_sec;
	dwTimeOut = 0 ;
	while(R_RTC_BUSY & RTC_SEC_BUSY) {
		if (dwTimeOut++ > 100000) break ;
	}

#if _OPERATING_SYSTEM == 1	
	if(sw_rtc_sem == NULL)
	{
		sw_rtc_sem = OSSemCreate(1);
	}
#endif    

	vic_irq_register(VIC_ALM_SCH_HMS, rtc_isr);	/* register RTC isr */
	vic_irq_enable(VIC_ALM_SCH_HMS);
}

void rtc_time_set(t_rtc *rtc_time)
{	
	INT32U dwTimeOut ;
	//vic_irq_disable(VIC_ALM_SCH_HMS);
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
//	sw_rtc_lock();
#endif	
	//gpx_rtc_write(0x0,rtc_time->rtc_sec); // sec
	//gpx_rtc_write(0x1,rtc_time->rtc_min); // min
	//gpx_rtc_write(0x2,rtc_time->rtc_hour); // hour
	ext_rtc_time_set(rtc_time) ;
#if _OPERATING_SYSTEM == 1	
//	sw_rtc_unlock();
#endif
	//vic_irq_enable(VIC_ALM_SCH_HMS);
	
	R_RTC_MIN = (INT32U)rtc_time->rtc_min;
	dwTimeOut = 0 ;
	while(R_RTC_BUSY & RTC_MIN_BUSY) {
		if (dwTimeOut++ > 100000) break ;
	}
	
	R_RTC_HOUR = (INT32U)rtc_time->rtc_hour;
	dwTimeOut = 0 ;
	while(R_RTC_BUSY & RTC_HR_BUSY) {
		if (dwTimeOut++ > 100000) break ;
	}

	R_RTC_SEC = (INT32U)rtc_time->rtc_sec;
	/* wait until not busy */
	dwTimeOut = 0 ;
	while(R_RTC_BUSY & RTC_SEC_BUSY) {
		if (dwTimeOut++ > 100000) break ;
	}
		
}

/*
void rtc_day_set(t_rtc *rtc_time)
{
	//vic_irq_disable(VIC_ALM_SCH_HMS);
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	sw_rtc_lock();
#endif
	gpx_rtc_write(0x3,(INT8U)(rtc_time->rtc_day & 0xff));
	gpx_rtc_write(0x4,(INT8U)((rtc_time->rtc_day >> 8) & 0xf));
#if _OPERATING_SYSTEM == 1	
	sw_rtc_unlock();
#endif
	//vic_irq_enable(VIC_ALM_SCH_HMS);
}*/

void rtc_time_get(t_rtc *rtc_time)
{
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	//sw_rtc_lock();
#endif
	rtc_time->rtc_sec = R_RTC_SEC;
	rtc_time->rtc_min = R_RTC_MIN;
	rtc_time->rtc_hour = R_RTC_HOUR;
#if _OPERATING_SYSTEM == 1	
	//sw_rtc_unlock();
#endif
}

void rtc_day_get(t_rtc *rtc_time)
{
	//vic_irq_disable(VIC_ALM_SCH_HMS);
	t_rtc ext_rtc_time ;
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	sw_rtc_lock();
#endif
	//rtc_time->rtc_day = gpx_rtc_read(0x3);	
	//rtc_time->rtc_day |= (gpx_rtc_read(0x4) & 0xf) << 8;
	ext_rtc_time_get(&ext_rtc_time);
#if _OPERATING_SYSTEM == 1	
	sw_rtc_unlock();
#endif
	rtc_time->rtc_day = ext_rtc_time.rtc_day ;
	//vic_irq_enable(VIC_ALM_SCH_HMS);
}

void rtc_alarm_set(t_rtc *rtc_time)
{
	R_RTC_ALARM_SEC = (INT32U)rtc_time->rtc_sec;
	R_RTC_ALARM_MIN = (INT32U)rtc_time->rtc_min;
	R_RTC_ALARM_HOUR = (INT32U)rtc_time->rtc_hour;
#if 0
	vic_irq_disable(VIC_ALM_SCH_HMS);
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	sw_rtc_lock();
#endif
	gpx_rtc_write(0x5,rtc_time->rtc_sec);	
	gpx_rtc_write(0x6,rtc_time->rtc_min);
	gpx_rtc_write(0x7,rtc_time->rtc_hour);
#if _OPERATING_SYSTEM == 1	
	sw_rtc_unlock();
#endif
	vic_irq_enable(VIC_ALM_SCH_HMS);
#endif
}

void rtc_alarm_get(t_rtc *rtc_time)
{
    rtc_time->rtc_sec = R_RTC_ALARM_SEC;
	rtc_time->rtc_min = R_RTC_ALARM_MIN;
	rtc_time->rtc_hour = R_RTC_ALARM_HOUR;
#if 0
	vic_irq_disable(VIC_ALM_SCH_HMS);
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	sw_rtc_lock();
#endif
	rtc_time->rtc_sec = gpx_rtc_read(0x5);
	rtc_time->rtc_min = gpx_rtc_read(0x6);
	rtc_time->rtc_hour = gpx_rtc_read(0x7);
#if _OPERATING_SYSTEM == 1	
	sw_rtc_unlock();
#endif
	vic_irq_enable(VIC_ALM_SCH_HMS);
#endif
}

/*
void rtc_function_set(INT8U mask, INT8U value)
{
	INT8U data = 0;
	
	vic_irq_disable(VIC_ALM_SCH_HMS);
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	sw_rtc_lock();
#endif
	data = gpx_rtc_read(0x08);
	data &= ~mask;
	data |= (mask & value);	
	
	gpx_rtc_write(0x08,data);
#if _OPERATING_SYSTEM == 1	
	sw_rtc_unlock();
#endif
	vic_irq_enable(VIC_ALM_SCH_HMS);
}
*/
/*
void rtc_reset_trigger_level_set(INT8U value)
{
	INT8U data = 0;
	
	vic_irq_disable(VIC_ALM_SCH_HMS);
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	sw_rtc_lock();
#endif

	data = gpx_rtc_read(0x08); // Power reset trigger level select signals (1.9v)
	data &= ~0x6;
	data |= (0x3 & value) << 1;	
	gpx_rtc_write(0x08,data);
	
#if _OPERATING_SYSTEM == 1	
	sw_rtc_unlock();
#endif
	vic_irq_enable(VIC_ALM_SCH_HMS);
}
*/

void rtc_int_set(INT8U mask, INT8U value)
{
    INT32U _mask = 0;
    INT32U _value = 0;
    
    _mask = mask & 0xF;
    _value = value & 0xF;
    
    if (mask & GPX_RTC_ALM_IEN) {
        _mask |= RTC_ALM_IEN;
    }
    
    if (mask & GPX_RTC_DAY_IEN) {
        _mask |= RTC_DAY_IEN;
    }
    
    if (value & GPX_RTC_ALM_IEN) {
        _value |= RTC_ALM_IEN;
        R_RTC_CTRL |= RTC_ALMEN;
    }
    else {
        R_RTC_CTRL &= ~RTC_ALMEN;
    }
    
    if (value & GPX_RTC_DAY_IEN) {
        _value |= RTC_DAY_IEN;
    }
    
    R_RTC_INT_CTRL &= ~_mask;
	R_RTC_INT_CTRL |= (_mask & _value);
#if 0
	INT8U data = 0;
	
	vic_irq_disable(VIC_ALM_SCH_HMS);

#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	sw_rtc_lock();
#endif
	data = gpx_rtc_read(0x0A);
	data &= ~mask;
	data |= (mask & value);	
	gpx_rtc_write(0x0A,data);
#if _OPERATING_SYSTEM == 1	
	sw_rtc_unlock();
#endif

	vic_irq_enable(VIC_ALM_SCH_HMS);
#endif
}

/*
INT8U gpx_rtc_read(INT8U addr)
{
	INT8U cmd = 0;
	INT8U data;
	
	cmd = addr << 3;
	
	R_RTC_IDPWR_ADDR = cmd;
	R_RTC_IDPWR_CTRL |= 0x1;
	
	TINY_WHILE((R_RTC_IDPWR_CTRL_FLAG & 1) == 0,375);
	//while(R_RTC_IDPWR_CTRL_FLAG & 1);
	
	data = (INT8U) R_RTC_IDPWR_RDATA;
	
	return data;
}
void gpx_rtc_write(INT8U addr,INT8U data)
{
	INT8U cmd = 1;
	cmd |= (addr << 3);
	
	R_RTC_IDPWR_ADDR = cmd;
		
	R_RTC_IDPWR_WDATA = data;
	R_RTC_IDPWR_CTRL |= 0x1;
	
	//while(R_RTC_IDPWR_CTRL_FLAG & 1);
	TINY_WHILE((R_RTC_IDPWR_CTRL_FLAG & 1) == 0,375);
	//while((gpx_rtc_read(0xB) & 1) == 1);

}

*/
#define RTC_RETRY 1024



/*
static int ext_rtc_wait2(void)
{
	int	N = 200;

	do {	// |b SYS_CLK=48MHz  üU??n 180 cycles
		if(R_EXT_RTC_READY & 0x01)
			break; // check if RTC controller is free (faster)
	}while(--N);

	return N;
}
*/


void rtc_schedule_enable(INT8U freq)
{
	R_RTC_CTRL |= RTC_EN;
	R_RTC_CTRL &= ~RTC_SCHSEL;
	R_RTC_CTRL |= freq;	
	R_RTC_CTRL |= RTC_SCHEN;
	R_RTC_INT_CTRL |= RTC_SCH_IEN;
}

void rtc_schedule_disable(void)
{
	R_RTC_CTRL &= ~RTC_SCHEN;
	R_RTC_INT_CTRL &= ~RTC_SCH_IEN;	
}

INT32S rtc_callback_set(INT8U int_idx, void (*user_isr)(void))
{
	INT32S status = STATUS_OK;
	
	if (int_idx > RTC_INT_MAX) {
		return STATUS_FAIL;
	}
	
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	sw_rtc_lock();
#endif	
	if (rtc_user_isr[int_idx] != 0) {
		status = STATUS_FAIL;
	}
	else {
		rtc_user_isr[int_idx] = user_isr;
	}	
#if _OPERATING_SYSTEM == 1	
	sw_rtc_unlock();
#endif

	return status;
}

INT32S rtc_callback_clear(INT8U int_idx)
{
	if (int_idx > RTC_INT_MAX) {
		return STATUS_FAIL;
	}
#if _OPERATING_SYSTEM == 1				// Soft Protect for critical section
	sw_rtc_lock();
#endif	
	rtc_user_isr[int_idx] = 0;
#if _OPERATING_SYSTEM == 1	
	sw_rtc_unlock();
#endif	
	return STATUS_OK;
}

void rtc_ext_to_int_set(void)
{
/*	
	R_RTC_SEC = gpx_rtc_read(0x0);
	while(R_RTC_BUSY & RTC_SEC_BUSY);
	
	R_RTC_MIN = gpx_rtc_read(0x1);
	while(R_RTC_BUSY & RTC_MIN_BUSY);
	
	R_RTC_HOUR = gpx_rtc_read(0x2);
	while(R_RTC_BUSY & RTC_HR_BUSY);
*/
	INT32U dwTimeOut ;
	t_rtc rtc_time={0} ;
	ext_rtc_time_get(&rtc_time) ;

	R_RTC_MIN = (INT32U)rtc_time.rtc_min;
	dwTimeOut = 0 ;
	while(R_RTC_BUSY & RTC_MIN_BUSY) {
		if (dwTimeOut++ > 100000) break ;
	}
	
	R_RTC_HOUR = (INT32U)rtc_time.rtc_hour;
	dwTimeOut = 0 ;
	while(R_RTC_BUSY & RTC_HR_BUSY) {
		if (dwTimeOut++ > 100000) break ;
	}

	R_RTC_SEC = (INT32U)rtc_time.rtc_sec;
	/* wait until not busy */
	dwTimeOut = 0 ;
	while(R_RTC_BUSY & RTC_SEC_BUSY) {
		if (dwTimeOut++ > 100000) break ;
	}

}



INT32S ext_rtc_read(INT8U addr, INT8U *data)
{
	INT32S N = 100;
	
	R_EXT_RTC_ADDR = addr;
	R_EXT_RTC_REQ = 0x02;	// triger read	

	//wait ready
	do {
		if(R_EXT_RTC_READY & 0x01) {
			*data = R_EXT_RTC_RD_DATA;
			return 0;
		} else {
			drv_msec_wait(1);
		} 
	} while(--N > 0);
	
	return -1;
}

INT32S ext_rtc_write(INT8U addr, INT8U data)
{
	INT32S N = 100;
	
	R_EXT_RTC_ADDR = addr;
	R_EXT_RTC_WR_DATA = data;
	R_EXT_RTC_REQ |= 0x01; //triger write

	//wait ready
	do {
		if(R_EXT_RTC_READY & 0x01) { // check if RTC controller is free
			break;
		} else {
			drv_msec_wait(1);
		} 
	} while(--N > 0);
		
	if(N == 0) {
		return -1;
	}

	switch(addr)
	{
	case 0x00:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x40:
	case 0x50:
		N = 100;
		do {
			ext_rtc_read(R_EXTRTC_CTRL, &data); 
			if((data & 0x10) == 0) { // check if RTC macro is free
				break;
			} else {
				drv_msec_wait(1);
			} 
		} while(--N > 0);
		
		if(N == 0) {
			return -1;
		}
		break;
		
	default:
		break;
	}
	
	return 0;
}


static void ext_rtc_isr(void)
{
	INT8U status;
	INT8U enable;
	INT32S ret;

	ret = ext_rtc_read(R_EXTRTC_INT_STATUS, &status);
	if(ret < 0) {
		goto __fail;
	}
	
	ret = ext_rtc_read(R_EXTRTC_INT_ENABLE, &enable);
	if(ret < 0) {
		goto __fail;
	}
	
//DBG_PRINT("Ext RTC ISR: status=%02X, enable=%02X\r\n", status, enable);	
	
	//clear flag
	status &= enable;
	ext_rtc_write(R_EXTRTC_INT_STATUS, 0x00);
		
	if (status & EXT_RTC_SEC_IEN) {
		if (rtc_user_isr[EXT_RTC_SEC_INT_INDEX]) {
			(*rtc_user_isr[EXT_RTC_SEC_INT_INDEX])();
		}
	}

	if (status & EXT_RTC_ALARM_IEN) {
		if (rtc_user_isr[EXT_RTC_ALARM_INT_INDEX]) {
			(*rtc_user_isr[EXT_RTC_ALARM_INT_INDEX])();
		}
	}

	if (status & EXT_RTC_WAKEUP_IEN) {
		if (rtc_user_isr[EXT_RTC_WAKEUP_INT_INDEX]) {
			(*rtc_user_isr[EXT_RTC_WAKEUP_INT_INDEX])();
		}
	}
	return;
	
__fail:	
	ext_rtc_write(R_EXTRTC_INT_STATUS, 0x00);
	return;
}

INT32S ext_rtc_chk_wakeup_status(void)
{
    INT8U temp = 0 ;
    INT32S ret;
    
	vic_irq_disable(VIC_RTC);
#if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	OSSchedLock();
#endif    

    // check if Ext RTC wakeup function trigger
	CHECK_RETURN(ext_rtc_read(R_EXTRTC_INT_STATUS, &temp)); 
	//DBG_PRINT("Ext RTC Int Value=0x%02X, Ret=%d\r\n",temp, ret);
    if (temp &0x04) // ext rtc wakeup 
    {
        ext_rtc_write(R_EXTRTC_INT_STATUS, 0); // clear it
        ret = 1;
    }
__exit:

#if _OPERATING_SYSTEM != _OS_NONE
	OSSchedUnlock();
#endif
	return ret;    
}


void ext_rtc_init(void)
{
	INT8U byReliable=0;

	R_EXT_RTC_CTRL = 0x01;					// enable ext rtc clock 
	if (INIT_MHZ==144) {
		(*((volatile INT32U *) 0xC009002C)) = 0x20;
	} else if (INIT_MHZ>144) {
	    (*((volatile INT32U *) 0xC009002C)) = 0x30;
	} else if (INIT_MHZ>96) {
	    (*((volatile INT32U *) 0xC009002C)) = 0x10;
	}
	else // < 96MHz
        (*((volatile INT32U *) 0xC009002C)) = 0x08;
        
	if (ext_rtc_chk_wakeup_status() > 0) {
		ext_rtc_pwr_on_flag = 1;
	} else {
		ext_rtc_pwr_on_flag = 0;
	}

	ext_rtc_write(R_EXTRTC_CTRL, 0x15);     // Up Count; RTC clk enable

	if (ext_rtc_get_reliable(&byReliable)<0) {
 		//DBG_PRINT("Failed to get Ext RTC reliable\r\n");
		ext_rtc_set_reliable(REALIBABLE);
	}

	if (byReliable!=REALIBABLE) {
		//DBG_PRINT("Ext RTC reliable is wrong=0x%02X, set again\r\n", byReliable);
		ext_rtc_set_reliable(REALIBABLE);
		ext_rtc_get_reliable(&byReliable);
		if (byReliable!=REALIBABLE) {
			//DBG_PRINT("Failed to set/get Ext RTC reliable!!\r\n");
		}
	} else {
		//DBG_PRINT("Ext RTC reliable is OK\r\n");
	}

    ext_rtc_alarm_enable(NULL, 0, NULL);
    ext_rtc_wakeup_int_enable(0, NULL);

	vic_irq_register(VIC_RTC, ext_rtc_isr);	/* register ext rtc isr */
	vic_irq_enable(VIC_RTC);
}


INT32S ext_rtc_pwm0_enable(INT8U byPole, INT16U wPeriod, INT16U wPreload, INT8U byEnable)
{
    INT32S ret ;
    INT32U iOK=-1 ;
    INT8U  byValue ;
    
#if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	sw_rtc_lock();
#endif    
    
    if (byEnable) {
        // period
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM0_PERIOD_L, wPeriod&0xFF));
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM0_PERIOD_H, (wPeriod>>8)&0xFF));
        // preload
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM0_PRELOAD_L, wPreload&0xFF));
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM0_PRELOAD_H, (wPreload>>8)&0xFF));
    }
    
    CHECK_RETURN(ext_rtc_read(R_EXTRTC_PWM_CTRL, &byValue));

	if (byEnable)
    	byValue |= 0x01 ;
    else
        byValue &= ~0x01 ;
    
    if (byPole)
    	byValue |= 0x04 ;
    else
        byValue &= ~0x04 ;
    
    CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM_CTRL, byValue)) ;
    iOK = 0 ;
    
__exit:    
#if _OPERATING_SYSTEM != _OS_NONE
	sw_rtc_unlock();
#endif
    return iOK ;
}

INT32S ext_rtc_pwm1_enable(INT8U byPole, INT16U wPeriod, INT16U wPreload, INT8U byEnable)
{
    INT32S ret ;
    INT32U iOK=-1 ;
    INT8U  byValue ;
    
#if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	sw_rtc_lock();
#endif    
    
    if (byEnable) {
        // period
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM1_PERIOD_L, wPeriod&0xFF));
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM1_PERIOD_H, (wPeriod>>8)&0xFF));
        // preload
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM1_PRELOAD_L, wPreload&0xFF));
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM1_PRELOAD_H, (wPreload>>8)&0xFF));
    }
    
    CHECK_RETURN(ext_rtc_read(R_EXTRTC_PWM_CTRL, &byValue));

	if (byEnable)
    	byValue |= 0x02 ;
    else
        byValue &= ~0x02 ;
    
    if (byPole)
    	byValue |= 0x08 ;
    else
        byValue &= ~0x08 ;
    
    CHECK_RETURN(ext_rtc_write(R_EXTRTC_PWM_CTRL, byValue)) ;
    iOK = 0 ;
    
__exit:    
#if _OPERATING_SYSTEM != _OS_NONE
	sw_rtc_unlock();
#endif
    return iOK ;
}


INT32S ext_rtc_set_reliable(INT8U byReliable)
{
    INT32S ret ;
    
    CHECK_RETURN(ext_rtc_write(R_EXTRTC_RELIABLE, byReliable));    
__exit:    
    return ret ;
}

INT32S ext_rtc_get_reliable(INT8U *pbyReliable)
{
    INT32S ret ;
    CHECK_RETURN(ext_rtc_read(R_EXTRTC_RELIABLE, pbyReliable));    
__exit:    
    return ret ;
}


INT32S ext_rtc_time_set(t_rtc *rtc_time)
{
	INT32S ret;
	INT64U sec;

	vic_irq_disable(VIC_RTC);
#if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	sw_rtc_lock();
#endif

	sec = (rtc_time->rtc_day * 60 * 60 * 24) + 
			(rtc_time->rtc_hour * 60 * 60) + 
			(rtc_time->rtc_min * 60) + 
			rtc_time->rtc_sec; 

	sec <<= 15;
	CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME0, (sec >> 0) & 0xFF)); 
	CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME1, (sec >> 8) & 0xFF));
	CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME2, (sec >> 16) & 0xFF));
	CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME3, (sec >> 24) & 0xFF));
	CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME4, (sec >> 32) & 0xFF));
	CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME5, (sec >> 40) & 0xFF));

__exit:
#if _OPERATING_SYSTEM != _OS_NONE
	sw_rtc_unlock();
#endif
	vic_irq_disable(VIC_RTC);
	return ret;
}

INT32S ext_rtc_time_get(t_rtc *rtc_time) 
{ 
        INT8U temp; 
        INT32S ret; 
        INT64U sec; 

        sec = 0; 
        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME0, &temp)); 
//DBG_PRINT("\r\nExt_RTC 0x10=0x%08X\r\n", temp);         
        sec += (INT64U)temp; 
        
        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME1, &temp)); 
//DBG_PRINT("Ext_RTC 0x11=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 8; 

        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME2, &temp)); 
//DBG_PRINT("Ext_RTC 0x12=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 16; 

        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME3, &temp)); 
//DBG_PRINT("Ext_RTC 0x13=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 24; 

        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME4, &temp)); 
//DBG_PRINT("Ext_RTC 0x14=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 32; 

        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME5, &temp)); 
//DBG_PRINT("Ext_RTC 0x15=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 40; 

        sec >>= 15; 
        rtc_time->rtc_day = (INT64U)sec/(60*60*24); 

        sec -= (INT64U)(rtc_time->rtc_day * 60 * 60 * 24); 
        rtc_time->rtc_hour = (INT64U)sec / (60*60); 

        sec -= (INT64U)(rtc_time->rtc_hour * 60 * 60); 
        rtc_time->rtc_min = (INT64U)sec / 60; 

        sec -= (INT64U)(rtc_time->rtc_min * 60); 
        rtc_time->rtc_sec = sec; 

__exit: 
        return ret; 
}


INT64U ext_rtc_time_get_value(void)
{
    INT32S ret; 
	INT8U temp;
	INT64U sec;

        sec = 0; 
        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME0, &temp)); 
//DBG_PRINT("\r\nExt_RTC 0x10=0x%08X\r\n", temp);         
        sec += (INT64U)temp; 
        
        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME1, &temp)); 
//DBG_PRINT("Ext_RTC 0x11=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 8; 

        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME2, &temp)); 
//DBG_PRINT("Ext_RTC 0x12=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 16; 

        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME3, &temp)); 
//DBG_PRINT("Ext_RTC 0x13=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 24; 

        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME4, &temp)); 
//DBG_PRINT("Ext_RTC 0x14=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 32; 

        CHECK_RETURN(ext_rtc_read(R_EXTRTC_TIME5, &temp)); 
//DBG_PRINT("Ext_RTC 0x15=0x%08X\r\n", temp);                 
        sec += (INT64U)temp << 40; 

        sec >>= 15; 

__exit:
	return sec;
}

INT64U ext_rtc_time_t_rtc_to_sec(t_rtc *rtc_time)
{
	INT64U sec;
	sec = (rtc_time->rtc_day * 60 * 60 * 24) + 
			(rtc_time->rtc_hour * 60 * 60) + 
			(rtc_time->rtc_min * 60) + 
			rtc_time->rtc_sec;
	return sec;
}

INT32S ext_rtc_time_add_now_and_set(t_rtc *rtc_time) // rtc_time -> added 
{ 
        INT32S ret; 
        INT64U sec; 
        INT64U now_sec; 
        
        INT64U sec_1; 
        INT64U now_sec_1; 
        
        t_rtc now_rtc_time ; 

DBG_PRINT("ext_rtc_time_set()~~~~~~~~~~~~~~~\r\n"); 

        vic_irq_disable(VIC_RTC); 
#if _OPERATING_SYSTEM != _OS_NONE                                // Soft Protect for critical section 
        sw_rtc_lock(); 
#endif 

        sec = (rtc_time->rtc_day * 60 * 60 * 24) + 
                        (rtc_time->rtc_hour * 60 * 60) + 
                        (rtc_time->rtc_min * 60) + 
                        rtc_time->rtc_sec; 

// 
sec_1 = sec<<15 ; 
DBG_PRINT("SPI time:\r\n"); 
DBG_PRINT("Ext RTC 0x10 = 0x%08X\r\n", (sec_1 >> 0) & 0xFF );         
DBG_PRINT("Ext RTC 0x11 = 0x%08X\r\n", (sec_1 >> 8) & 0xFF );         
DBG_PRINT("Ext RTC 0x12 = 0x%08X\r\n", (sec_1 >> 16) & 0xFF );                         
DBG_PRINT("Ext RTC 0x13 = 0x%08X\r\n", (sec_1 >> 24) & 0xFF );                                 
DBG_PRINT("Ext RTC 0x14 = 0x%08X\r\n", (sec_1 >> 32) & 0xFF );                                         
DBG_PRINT("Ext RTC 0x15 = 0x%08X\r\n", (sec_1 >> 40) & 0xFF );         
DBG_PRINT("========================\r\n"); 

    // get current time 
    ext_rtc_time_get(&now_rtc_time) ; 
        now_sec = (now_rtc_time.rtc_day * 60 * 60 * 24) + 
                        (now_rtc_time.rtc_hour * 60 * 60) + 
                        (now_rtc_time.rtc_min * 60) + 
                        now_rtc_time.rtc_sec; 

DBG_PRINT("Current time:\r\n"); 
now_sec_1 = now_sec<<15 ; 
DBG_PRINT("Ext RTC 0x10 = 0x%08X\r\n", (now_sec_1 >> 0) & 0xFF );         
DBG_PRINT("Ext RTC 0x11 = 0x%08X\r\n", (now_sec_1 >> 8) & 0xFF );         
DBG_PRINT("Ext RTC 0x12 = 0x%08X\r\n", (now_sec_1 >> 16) & 0xFF );                         
DBG_PRINT("Ext RTC 0x13 = 0x%08X\r\n", (now_sec_1 >> 24) & 0xFF );                                 
DBG_PRINT("Ext RTC 0x14 = 0x%08X\r\n", (now_sec_1 >> 32) & 0xFF );                                         
DBG_PRINT("Ext RTC 0x15 = 0x%08X\r\n", (now_sec_1 >> 40) & 0xFF );         
DBG_PRINT("========================\r\n"); 

    sec = now_sec + sec ; 



DBG_PRINT("Final time:\r\n"); 
        sec <<= 15; 
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME0, (sec >> 0) & 0xFF)); 
DBG_PRINT("Ext RTC 0x10 = 0x%08X\r\n", (sec >> 0) & 0xFF );         
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME1, (sec >> 8) & 0xFF)); 
DBG_PRINT("Ext RTC 0x11 = 0x%08X\r\n", (sec >> 8) & 0xFF );                 
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME2, (sec >> 16) & 0xFF)); 
DBG_PRINT("Ext RTC 0x12 = 0x%08X\r\n", (sec >> 16) & 0xFF );                         
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME3, (sec >> 24) & 0xFF)); 
DBG_PRINT("Ext RTC 0x13 = 0x%08X\r\n", (sec >> 24) & 0xFF );                                 
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME4, (sec >> 32) & 0xFF)); 
DBG_PRINT("Ext RTC 0x14 = 0x%08X\r\n", (sec >> 32) & 0xFF );                                         
        CHECK_RETURN(ext_rtc_write(R_EXTRTC_LOAD_TIME5, (sec >> 40) & 0xFF)); 
DBG_PRINT("Ext RTC 0x15 = 0x%08X\r\n", (sec >> 40) & 0xFF );         
DBG_PRINT("========================\r\n"); 

__exit: 
#if _OPERATING_SYSTEM != _OS_NONE 
        sw_rtc_unlock(); 
#endif 
        vic_irq_disable(VIC_RTC); 
        return ret; 
}

INT32S ext_rtc_alarm_enable(t_rtc *rtc_time, INT8U byEnable, void (*ext_rtc_alarm_isr)(void))
{
	INT32S ret;
	INT64U sec;
	INT8U  byValue;
	
	vic_irq_disable(VIC_RTC);
#if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	OSSchedLock();
#endif

    if (byEnable) {
    	sec = (rtc_time->rtc_day * 60 * 60 * 24) + 
    			(rtc_time->rtc_hour * 60 * 60) + 
    			(rtc_time->rtc_min * 60) + 
    			rtc_time->rtc_sec; 

    	sec <<= 15;
    	CHECK_RETURN(ext_rtc_write(R_EXTRTC_ALARM0, (sec >> 8) & 0xFF));
    	CHECK_RETURN(ext_rtc_write(R_EXTRTC_ALARM1, (sec >> 16) & 0xFF));
    	CHECK_RETURN(ext_rtc_write(R_EXTRTC_ALARM2, (sec >> 24) & 0xFF));
    	CHECK_RETURN(ext_rtc_write(R_EXTRTC_ALARM3, (sec >> 32) & 0xFF));
    	CHECK_RETURN(ext_rtc_write(R_EXTRTC_ALARM4, (sec >> 40) & 0xFF));
    }

	CHECK_RETURN(ext_rtc_read(R_EXTRTC_INT_ENABLE, &byValue));
	//DBG_PRINT("ext_rtc_alarm_enable()_1: byValue=0x%08X \r\n", byValue);
	if (byEnable)
    	byValue |= 0x02 ;
    else
        byValue &= ~0x02 ;
    CHECK_RETURN(ext_rtc_write(R_EXTRTC_INT_ENABLE, byValue));	
    //DBG_PRINT("ext_rtc_alarm_enable()_2: byValue=0x%08X \r\n", byValue);
	
__exit:
#if _OPERATING_SYSTEM != _OS_NONE
	OSSchedUnlock();
#endif
	rtc_callback_set(EXT_RTC_ALARM_INT_INDEX, ext_rtc_alarm_isr);

	if (byEnable)
    	vic_irq_enable(VIC_RTC);
	else
	    vic_irq_disable(VIC_RTC);
	return ret;
}


INT32S ext_rtc_wakeup_int_enable(INT8U byEnable, void (*ext_rtc_wakeup_isr)(void))
{
    INT32S ret;	
    INT8U  byValue ;
	vic_irq_disable(VIC_RTC);

#if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	sw_rtc_lock();
#endif

	CHECK_RETURN(ext_rtc_read(R_EXTRTC_INT_ENABLE, &byValue));
	if (byEnable)
    	byValue |= 0x04 ;
    else
        byValue &= ~0x04 ;
    CHECK_RETURN(ext_rtc_write(R_EXTRTC_INT_ENABLE, byValue));
	
__exit:
#if _OPERATING_SYSTEM != _OS_NONE
	sw_rtc_unlock();
#endif
	rtc_callback_set(EXT_RTC_WAKEUP_INT_INDEX, ext_rtc_wakeup_isr);
	if (byEnable)
    	vic_irq_enable(VIC_RTC);
	else
	    vic_irq_disable(VIC_RTC);
	return ret;
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_RTC) && (_DRV_L1_RTC == 1)              //
//================================================================//

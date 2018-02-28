/******************************************************************************
 * Paresent Header Include
 ******************************************************************************/
#include "drv_l1_sw_timer.h"
#include "drv_l1_rtc.h"
#include "drv_l1_sfr.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_SW_TIMER) && (_DRV_L1_SW_TIMER == 1)         //
//================================================================//


INT32U sw_timer_init(INT32U TimerId, INT32U freq_hz);
void sw_timer_delay(INT32U timer_delay_count);
void sw_timer_ms_delay(INT32U msec);
void sw_timer_1Khz_callback(void);
INT8U sw_timer_source_rtc=0;

/******************************************************************************
 * Defines
 ******************************************************************************/
#define GP_TIMER_OK   0
#define GP_TIMER_FAIL 0xFFFF



/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
INT32U sw_timer_init(INT32U TimerId, INT32U freq_hz);
INT32U sw_timer_get_counter_L(void);
INT32U sw_timer_get_counter_H(void);

/******************************************************************************
 * Function Bodies
 ******************************************************************************/
INT32U sw_timer_init(INT32U TimerId, INT32U freq_hz)
{
    sw_timer_count_H=0;
    sw_timer_count_L=0;
    sw_timer_source_rtc=0;
    if (TimerId<4)
    {
        // if (freq_hz>8192 || freq_hz<2) {return GP_TIMER_FAIL;}
        timer_freq_setup(TimerId, freq_hz, 0, sw_timer_isr);
        sw_timer_source_rtc=0; 
    }
    else if (TimerId == 6)  /* RTC schedual timer */
    {
        rtc_callback_set(RTC_SCH_INT_INDEX, sw_timer_isr);
       	rtc_schedule_enable(RTC_SCH_1024HZ);
        sw_timer_source_rtc=1;    
    }       
    return GP_TIMER_OK;
}

INT32U timer_counter_init(INT32U TimerId, INT32U freq_hz)
{
    sw_timer_count_H=0;
    sw_timer_count_L=0;
    sw_timer_source_rtc=0;
    if (TimerId<4)
    {
        // if (freq_hz>8192 || freq_hz<2) {return GP_TIMER_FAIL;}
        timer_freq_setup(TimerId, freq_hz, 0, sw_timer_isr);
        sw_timer_source_rtc=0; 
    }
    else if (TimerId == 6)  /* RTC schedual timer */
    {
        rtc_callback_set(RTC_SCH_INT_INDEX, sw_timer_isr);
       	rtc_schedule_enable(RTC_SCH_1024HZ);
        sw_timer_source_rtc=1;    
    }       
    return GP_TIMER_OK;
}

INT32U sw_timer_get_counter_L(void)
{
    if (sw_timer_source_rtc!=1)
    {return sw_timer_count_L;}
    else if (!(R_RTC_CTRL&RTC_SCH_1024HZ) || !(R_RTC_CTRL&RTC_SCHEN) || !(R_RTC_CTRL&RTC_RTCEN))
    {
        R_RTC_CTRL|=RTC_RTCEN;
        rtc_callback_set(RTC_SCH_INT_INDEX, sw_timer_isr);
	   	rtc_schedule_enable(RTC_SCH_1024HZ);
        return sw_timer_count_L;
    }
    else {return sw_timer_count_L;}
}

INT32U sw_timer_get_counter_H(void)
{
    return sw_timer_count_H;
}

void sw_timer_delay(INT32U timer_delay_count)   /* only used in TimerID=5*/
{
    INT32U timer_count_temp;
    timer_count_temp = sw_timer_count_L;
    while ((sw_timer_count_L-timer_count_temp) < timer_delay_count);
}

void sw_timer_ms_delay(INT32U msec)
{
    INT32S m_sec=(INT32U) msec;
    #if _OPERATING_SYSTEM == _OS_UCOS2
    //INT32U tick_count;
  
      INT32S timer_counter=0;
      INT32S time_interval=0;
      if (OSRunning==TRUE)
      {
          while ((m_sec>=10) && (m_sec>0))
          {
              timer_counter = sw_timer_get_counter_L();
              OSTimeDly(1);
              time_interval = sw_timer_get_counter_L()-timer_counter;
              m_sec = m_sec - time_interval;
          }
      }
      
    #endif
    
    while (m_sec>0)
    {
        m_sec--;
        TINY_WHILE(0, 375);  /* 1ms = 375 tiny (1ms = 2.666666 x 375)*/
    }
}

void sw_timer_us_delay(INT32U USEC)  //96Mhz
{
#if 1
   INT32S tiny_count = (INT32S) (USEC*1000/2666) + 1;

   while (tiny_count>0)
   {
        tiny_count--;
        TINY_WHILE(0, 1); 
   }
#else
   INT32U usec = USEC*159;
   if (MHZ==48) {usec = 79;}
   else if (MHZ==24) {usec = 39;}

   while (usec>10) {
  	usec-= 10 ;
  }
#endif   
}

extern void ap_display_TE_sync_I80(void);
void sw_timer_1Khz_callback(void)
{

	#if ((USE_PANEL_NAME == PANEL_400X240_I80)||(USE_PANEL_NAME == PANEL_T20P82_ST7789V))
		ap_display_TE_sync_I80();
	#endif
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_SW_TIMER) && (_DRV_L1_SW_TIMER == 1)    //
//================================================================//
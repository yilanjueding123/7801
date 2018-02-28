#ifndef __GP_TIMER_H__
#define __GP_TIMER_H__
/******************************************************************************
 * Paresent Header Include
 ******************************************************************************/
#include "driver_l1.h"


extern INT32U sw_timer_init(INT32U TimerId, INT32U freq_hz);
extern INT32U sw_timer_get_counter_L(void);
extern INT32U sw_timer_get_counter_H(void);
extern void sw_timer_isr(void);
extern void sw_schedual_isr(void);
extern void sw_timer_1Khz_callback(void);
extern void sw_timer_ms_delay(INT32U msec);
extern INT32U sw_timer_count_H;
extern INT32U sw_timer_count_L;
extern INT32U sw_schedual_count;


#endif

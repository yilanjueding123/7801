#include "driver_l2.h"

void drv_l2_init(void)
{
  #if (defined _DRV_L2_SYS_TIMER_EN) &&  (_DRV_L2_SYS_TIMER_EN == 1)
	sys_init_timer();
  #endif
  
  #if (defined _DRV_L2_EXT_RTC) && (_DRV_L2_EXT_RTC==DRV_L2_ON)
  	drv_l2_external_rtc_init();
  #endif
}

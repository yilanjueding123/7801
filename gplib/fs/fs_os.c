#include "fsystem.h"


#if _OPERATING_SYSTEM != _OS_NONE
OS_EVENT *gFS_sem;
#endif

INT32S FS_OS_Init(void) 
{
  #if _OPERATING_SYSTEM != _OS_NONE
    gFS_sem = OSSemCreate(1);
  #endif

    return (0);
}

INT32S FS_OS_Exit(void) 
{
    return (0);
}

void FS_OS_LOCK(void)
{
  #if _OPERATING_SYSTEM != _OS_NONE
	INT8U err = NULL;
	
	OSSemPend(gFS_sem, 0, &err);
  #endif
}

void FS_OS_UNLOCK(void)
{
  #if _OPERATING_SYSTEM != _OS_NONE
	OSSemPost(gFS_sem);
  #endif
}

/****************************************************************/
/*																*/
/*			       seek speedup									*/
/*																*/
/*		  seek speedup memery management function               */
/*																*/
/****************************************************************/
INT32U FS_SeekSpeedup_Malloc(INT32U len)
{
	return (INT32U)gp_malloc_align((len << 1), 4);
}

void FS_SeekSpeedup_Free(INT32U addr)
{
	gp_free((void*)addr);
}

/****************************************************************/
/*																*/
/*			       getdate function								*/
/*																*/
/*		  user write this file to assign the file system date   */
/*																*/
/****************************************************************/
void FS_OS_GetDate(dosdate_t *dd)
{
	TIME_T  tm;
		
	if(0 == cal_time_get(&tm))
	{
		dd->year = tm.tm_year;
		dd->month = tm.tm_mon;
		dd->monthday = tm.tm_mday;
	}
	else
	{
		dd->year = 2018;
		dd->month = 1;
		dd->monthday = 1;
	}
}




/****************************************************************/
/*																*/
/*			       gettime function								*/
/*																*/
/*		  user write this file to assign the file system time   */
/*																*/
/****************************************************************/
static INT8U adjust_sec = 0;

void FS_OS_GetTime(dostime_t *dt)
{
	TIME_T  tm;
	
	if(0 == cal_time_get(&tm))
	{
		dt->hour = tm.tm_hour;
		dt->minute = tm.tm_min;
		dt->second = tm.tm_sec;
		dt->hundredth = 0;
	}
	else
	{
		dt->hour = 23;
		dt->minute = 59;
		dt->second = 59;
		dt->hundredth = 0;
	}

	if(adjust_sec) {
		if((dt->hour == 0) && (dt->minute == 0) && (dt->second < 2)) {
			dt->second += 2;
		} else {
			if(dt->second >= 2) {
				dt->second -= 2;
			} else {
				if(dt->minute > 0) {
					dt->minute -= 1;
					dt->second += 58;
				} else {
					dt->hour -= 1;
					dt->minute += 59;
					dt->second += 58;
				}
			}
		}
	}
}

void F_OS_AdjustCrtTimeEnable(void)
{
	adjust_sec = 1;
}


void F_OS_AdjustCrtTimeDisable(void)
{
	adjust_sec = 0;
}


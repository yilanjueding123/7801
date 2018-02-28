/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _RTOS_DEF_H_
#define _RTOS_DEF_H_

#include <porting.h>

/*============OS parameter setting===================*/
typedef struct OsMsgQ_st
{
    void*       qpool;
    OS_EVENT*   msssageQ;
}OsMessgQ;
typedef void (*OsTask)(void *);
typedef void *              OsTaskHandle;
typedef void *              OsTimer;
typedef void (*OsTimerHandler)( OsTimer xTimer );

typedef OS_EVENT*           OsMutex;

typedef struct{
    OS_EVENT*   Sema;
    u16 MaxCnt;
}OsSemaphore_impl;

typedef OsSemaphore_impl* OsSemaphore;

typedef OsMessgQ*           OsMsgQ;

#define OS_TASK_ENTER_CRITICAL()        OS_CPU_SR cpu_sr; \
                                        OS_ENTER_CRITICAL()

#define OS_TASK_EXIT_CRITICAL()         OS_EXIT_CRITICAL()

#define TICK_RATE_MS (1000/OS_TICKS_PER_SEC)
#define TASK_IDLE_STK_SIZE OS_TASK_IDLE_STK_SIZE

extern void TaskSwHook(void);
typedef struct{
    u32 TaskCtr;
    u32 TaskTotExecTimeFromTick;
    u32 TaskTotExecTimeFromTiny;
    char TaskName[30];
    u8  valid;

}TASK_USER_DATA;
#if OS_TASK_CREATE_EXT_EN > 0
extern TASK_USER_DATA taskUserData[64];
#endif
#if ((OS_CPU_HOOKS_EN>0)&&(OS_TASK_STAT_EN==1))
extern void TaskStatHook(void);
extern void DispTaskStatus(void);
extern void ClearTaskStatus(void);
#endif

#endif

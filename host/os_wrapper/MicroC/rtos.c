/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include "rtos.h"
#include "os_cfg.h"
#include <log.h>


volatile u8 gOsFromISR;
static u8 os_init_flag = 0;

#define STATIC_STK
#ifdef STATIC_STK
//(TOTAL_STACK_SIZE<<4) --> transfer unit to bytes
//((TOTAL_STACK_SIZE<<4)>>2) --> transfer unit to word
u32 task_stack[((TOTAL_STACK_SIZE<<4)>>2)];
u32 stk_used=0;
OsMutex StkMutex;
#endif

u32 task_stack_size = 0;

OS_APIs s32  OS_Init( void )
{
    if (os_init_flag == 1)
    {
        return 0;
    }

    os_init_flag = 1;
    
    gOsFromISR = 0;
    //enable uc-os timer
    //timer_freq_setup(0, OS_TICKS_PER_SEC, 0, OSTimeTick);
#ifdef STATIC_STK
    OS_MutexInit(&StkMutex);
    task_stack_size = ((TOTAL_STACK_SIZE<<4)>>2) * 4;
    OS_MemSET(task_stack,0,task_stack_size);
    //OS_MemSET((void *)wifi_task_stack,0,sizeof(wifi_task_stack));
#endif

#if OS_TASK_CREATE_EXT_EN > 0
    ssv6xxx_memset(taskUserData,0,sizeof(taskUserData));
#endif
#if(OS_TASK_STAT_EN==1)
    OSStatInit();
#endif
#ifdef STATIC_STK
    LOG_PRINTF("Total Stack size is 0x%x (bytes)\r\n",task_stack_size);
#endif
    return OS_SUCCESS;
}

OS_APIs unsigned long OS_Random(void)
{
	return OSTimeGet()%65536+54*18;
}

OS_APIs void OS_Terminate( void )
{
    //vTaskEndScheduler();
}

OS_APIs u32 OS_EnterCritical(void)
{
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    return cpu_sr;
}

OS_APIs void OS_ExitCritical(u32 val)
{
    OS_CPU_SR cpu_sr = val;
    OS_EXIT_CRITICAL();
}

/* Task: */

OS_APIs s32 OS_TaskCreate( OsTask task, const char *name, u32 stackSize, void *param, u32 pri, OsTaskHandle *taskHandle )
{
    u32* stk_ptr = NULL;
    s32 ret;
    u32 stk_size=(stackSize>>2); //transfer unit to word

#ifdef STATIC_STK
    OS_MutexLock(StkMutex);
    if((stk_used+stk_size)>(task_stack_size>>2)){
        LOG_ERROR("Stack is not enough for new task %s\r\n",name);
        ASSERT(FALSE);
    }
    stk_ptr=&task_stack[stk_used];
    stk_used+=stk_size;
    LOG_PRINTF("Free Stack size is 0x%x (bytes)\r\n",(task_stack_size-(stk_used<<2)));
    OS_MutexUnLock(StkMutex);    
#else
    stk_ptr = (void*)OS_MemAlloc(stackSize);
    if(! stk_ptr){
        LOG_ERROR("alloc %s stack fail\n",name);
        return OS_FAILED;
    }
#endif
    LOG_PRINTF("OS_TaskCreate =%s,pri=%d,stackSize=%d, stack top=0x%x, stack botton=0x%x\r\n",name,pri,stackSize,&stk_ptr[stk_size-1],&(stk_ptr[0]));

#if OS_TASK_CREATE_EXT_EN > 0
    ssv6xxx_strcpy(taskUserData[pri].TaskName,name);
    LOG_PRINTF("task name:%s,%d\r\n",taskUserData[pri].TaskName,pri);
    taskUserData[pri].TaskCtr=0;
    taskUserData[pri].TaskTotExecTimeFromTiny=0;
    taskUserData[pri].TaskTotExecTimeFromTick=0;
    taskUserData[pri].valid=1;
#endif
#if (OS_STK_GROWTH==0)
    ret = (s32)OSTaskCreate(task, param, stk_ptr, (INT8U)pri);
#else
#ifdef STATIC_STK
#if OS_TASK_CREATE_EXT_EN > 0
        ret = (s32)OSTaskCreateExt(task, param, &(stk_ptr[stk_size - 1]), (INT8U)pri,
                                (INT8U)pri,&(stk_ptr[0]),stk_size,&taskUserData[pri],OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#else
        ret = (s32)OSTaskCreate(task, param, &(stk_ptr[stk_size - 1]), (INT8U)pri);
#endif
#else
#if OS_TASK_CREATE_EXT_EN > 0
        ret = (s32)OSTaskCreateExt(task, param, &(stk_ptr[stk_size - 1]), (INT8U)pri,
                                    (INT8U)pri,&(stk_ptr[0]),stk_size,&taskUserData[pri],OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#else
        ret = (s32)OSTaskCreate(task, param, (((u32 *)stk_ptr)+(stackSize/4) - 1), (INT8U)pri);
#endif

#endif
#endif
    if(ret == OS_NO_ERR)
        return OS_SUCCESS;
    else
        LOG_ERROR("OSTaskCreate ret=%d \r\n",ret);

    return OS_FAILED;
}


OS_APIs void OS_TaskDelete(OsTaskHandle taskHandle)
{
    //vTaskDelete(taskHandle);
    OSTaskDel(OS_PRIO_SELF);
}



OS_APIs void OS_StartScheduler( void )
{
    //vTaskStartScheduler();
}

OS_APIs u32 OS_GetSysTick(void)
{
    return OSTimeGet();
}


/* Mutex APIs: */
OS_APIs s32 OS_MutexInit( OsMutex *mutex )
{
    //u8 error;

    //*mutex = xSemaphoreCreateMutex();
    //*mutex = OSMutexCreate(0, &error);
    *mutex = OSSemCreate(1);

    if ( NULL == *mutex ){
        LOG_ERROR("OSSemCreate fail !!!\n");
        return OS_FAILED;
    }
    else{
        return OS_SUCCESS;
    }
}


OS_APIs void OS_MutexLock( OsMutex mutex )
{
    u8 err;
    //xSemaphoreTake( mutex, portMAX_DELAY);
    //OSMutexPend(mutex,0,&err);
    //LOG_PRINTF("+\r\n");
    if ( NULL == mutex ){
        LOG_ERROR("OS_MutexLock mutex = NULL fail !!!\n");
        return;
    }
    OSSemPend(mutex, 0, &err);
    if(err!=OS_NO_ERR) assert(FALSE);
}

OS_APIs void OS_TickDelay(u32 ticks)
{
        OSTimeDly ((u16)ticks);
}


OS_APIs void OS_MutexUnLock( OsMutex mutex )
{
    //xSemaphoreGive( mutex );
    //OSMutexPost(mutex);
    //LOG_PRINTF("-\r\n");
    if ( NULL == mutex ){
        LOG_ERROR("OS_MutexLock mutex = NULL fail !!!\n");
        return;
    }
    if(OS_NO_ERR!=OSSemPost(mutex)) assert(FALSE);
}

OS_APIs void OS_MutexDelete( OsMutex mutex )
{
    u8 err;

    //OSMutexDel(mutex, OS_DEL_ALWAYS, &err);
    OSSemDel(mutex, OS_DEL_ALWAYS, &err);
}

OS_APIs void OS_MsDelay(u32 ms)
{
#if OS_TIME_DLY_HMSM_EN > 0
    u8 sec = 0;
    if(ms >= 1000){
        sec = ms/1000;
        ms = ms%1000;
    }else if(ms < TICK_RATE_MS){
        ms = TICK_RATE_MS;
    }else{;}
    OSTimeDlyHMSM(0,0,sec,ms);
#else
	if(ms < TICK_RATE_MS)
	{
		ms = TICK_RATE_MS;
	}
	OSTimeDly(ms/TICK_RATE_MS);
#endif
}

#define SEMA_LOCK()		OSSchedLock()
#define SEMA_UNLOCK()		OSSchedUnlock()

OS_APIs s32 OS_SemInit( OsSemaphore* Sem, u16 maxcnt, u16 cnt)
{
    //u8 error;
    *Sem = (OsSemaphore)OS_MemAlloc(sizeof(OsSemaphore_impl));
    if(NULL==*Sem)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return OS_FAILED;
    }

    (*Sem)->MaxCnt = 0;
    (*Sem)->Sema = OSSemCreate(cnt);
    if ( NULL == (*Sem)->Sema ){
        return OS_FAILED;
    }
    else{
        (*Sem)->MaxCnt = maxcnt;
        return OS_SUCCESS;
    }
}

OS_APIs bool OS_SemWait( OsSemaphore Sem , u16 timeout)
{
    u8 err;

    OSSemPend(Sem->Sema, timeout, &err);
    if (err != OS_NO_ERR) {
        return OS_FAILED;
    } else {
        return OS_SUCCESS;
    }
}

OS_APIs u8 OS_SemSignal( OsSemaphore Sem)
{
    u8 ret;
    OS_SEM_DATA sem_data;
    
    SEMA_LOCK();
    ret = OSSemQuery(Sem->Sema, &sem_data);
    if(ret != OS_NO_ERR)
    {
        SEMA_UNLOCK();
        LOG_PRINTF("SemQuery err=%d\r\n",ret);
        return ret; 
    }
    
    if (sem_data.OSCnt < Sem->MaxCnt)
    {
        SEMA_UNLOCK();
        ret = OSSemPost(Sem->Sema);
        if(ret == OS_ERR_PEVENT_NULL)
            ret = OS_NO_ERR;
        
        if (ret != OS_NO_ERR)
            LOG_PRINTF("SemPost Post err=%d\r\n",ret);                    
    }
    else
    {   
        SEMA_UNLOCK();
        ret = OS_Q_FULL;        
    }
            
	return ret;
}

OS_APIs u32 OS_SemCntQuery( OsSemaphore Sem)
{
    u8 ret;
    OS_SEM_DATA sem_data;
    
    SEMA_LOCK();
    ret = OSSemQuery(Sem->Sema, &sem_data);
    if(ret != OS_NO_ERR)
    {
        SEMA_UNLOCK();
        LOG_PRINTF("SemQuery err=%d\r\n",ret);
        return ret; 
    }

    LOG_PRINTF("sem_data.OSCnt=%d, Sem->MaxCnt=%d\r\n",sem_data.OSCnt, Sem->MaxCnt);
    SEMA_UNLOCK();
    return sem_data.OSCnt;
}

OS_APIs u8 OS_SemSignal_FromISR( OsSemaphore Sem)
{
	return OS_SemSignal(Sem);
}

OS_APIs void OS_SemDelete(OsSemaphore Sem)
{
    u8 err;
    SEMA_LOCK();
	OSSemDel(Sem->Sema, OS_DEL_ALWAYS, &err);
    Sem->MaxCnt = 0;
    OS_MemFree(Sem);
    SEMA_UNLOCK();
}

#define MSGQ_LOCK()		OSSchedLock()
#define MSGQ_UNLOCK()		OSSchedUnlock()

/* Message Queue: */
OS_APIs s32 OS_MsgQCreate( OsMsgQ *MsgQ, u32 QLen )
{
    u16 size = sizeof( OsMsgQEntry )*QLen;
    void* qpool = OS_MemAlloc(size+sizeof(OsMessgQ));
    OsMsgQ tmpq;

    if(qpool)
    {
        OS_MemSET((void *)qpool, 0, size);	/* clear out msg q structure */
        *MsgQ = tmpq = (OsMsgQ)qpool;
        tmpq->qpool = qpool;
#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (defined(OS_Q_COPY_MSG) && (OS_Q_COPY_MSG > 0))
        tmpq->msssageQ = OSQCreate((void*)(((u8*)qpool)+sizeof(OsMessgQ)), (size/4), 4);
#endif     

#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (!defined(OS_Q_COPY_MSG) || (OS_Q_COPY_MSG == 0))
		//get double size msg q length
		tmpq->msssageQ = OSQCreate((void*)(((u8*)qpool)+sizeof(OsMessgQ)), (size/4));
#endif
    }
    else
    {
        LOG_ERROR("%s,size=%d\r\n",__func__,size);
    }

    if ( NULL == *MsgQ )
        return OS_FAILED;
    return OS_SUCCESS;
}

OS_APIs s32 OS_MsgQDelete( OsMsgQ MsgQ)
{
    u8 err=0;
    MSGQ_LOCK();
    
#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (defined(OS_Q_COPY_MSG) && (OS_Q_COPY_MSG > 0))
	err = OSQDel(MsgQ->msssageQ, OS_DEL_ALWAYS);
#endif  

#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (!defined(OS_Q_COPY_MSG) || (OS_Q_COPY_MSG == 0))
	OSQDel(MsgQ->msssageQ, OS_DEL_ALWAYS, &err);
#endif
    OS_MemFree(MsgQ->qpool);
    MSGQ_UNLOCK();
    return ( 0!=err )? OS_FAILED: OS_SUCCESS;
}

OS_APIs s32 OS_MsgQEnqueue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, bool fromISR )
{
    u8 err=0;
    
postQ:
	
#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (defined(OS_Q_COPY_MSG) && (OS_Q_COPY_MSG > 0))
	err = OSQPost(MsgQ->msssageQ, &MsgItem->MsgData);
#endif

#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (!defined(OS_Q_COPY_MSG) || (OS_Q_COPY_MSG == 0))
	err = OSQPost(MsgQ->msssageQ, MsgItem->MsgData);
#endif
    
    if(OS_Q_FULL == err){
        OS_MsDelay(1);
        goto postQ;    
    } 
    return ( 0!=err )? OS_FAILED: OS_SUCCESS;
}

OS_APIs s32 OS_MsgQEnqueueTry( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, bool fromISR )
{
	u8 err=0;
	
#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (defined(OS_Q_COPY_MSG) && (OS_Q_COPY_MSG > 0))
	err = OSQPost(MsgQ->msssageQ, &MsgItem->MsgData);
#endif
	 
#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (!defined(OS_Q_COPY_MSG) || (OS_Q_COPY_MSG == 0))
	err = OSQPost(MsgQ->msssageQ, MsgItem->MsgData);
#endif
	return ( 0!=err )? OS_FAILED: OS_SUCCESS;
}

OS_APIs s32 OS_MsgQDequeue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, u32 timeOut, bool fromISR )

{
    u8 err=0;

#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (defined(OS_Q_COPY_MSG) && (OS_Q_COPY_MSG > 0))
    err = OSQPend(MsgQ->msssageQ, timeOut, (void*)&MsgItem->MsgData);
    MsgItem->MsgCmd = 0;
#endif   

#if (OS_Q_EN > 0) && (OS_MAX_QS > 0) && (!defined(OS_Q_COPY_MSG) || (OS_Q_COPY_MSG == 0))
	MsgItem->MsgData = OSQPend(MsgQ->msssageQ, timeOut, &err);
	MsgItem->MsgCmd = 0;
#endif  
	
    return ( 0!=err )? OS_FAILED: OS_SUCCESS;

}


OS_APIs s32 OS_MsgQWaitingSize( OsMsgQ MsgQ )
{
    //return ( uxQueueMessagesWaiting( MsgQ ) );
    return 0;
}



/* Timer: */
OS_APIs s32 OS_TimerCreate( OsTimer *timer, u32 ms, u8 autoReload, void *args, OsTimerHandler timHandler )
{
#if 0
    ms = ( 0 == ms )? 1: ms;
    *timer = xTimerCreate( NULL, OS_MS2TICK(ms), autoReload, args, timHandler);
    if ( NULL == *timer )
        return OS_FAILED;
    return OS_SUCCESS;
#endif
    return OS_SUCCESS;

}

OS_APIs s32 OS_TimerSet( OsTimer timer, u32 ms, u8 autoReload, void *args )
{
#if 0
    if ( pdFAIL == xTimerChangeSetting( timer, OS_MS2TICK(ms), autoReload, args) )
        return OS_FAILED;
    return OS_SUCCESS;
#endif
    return OS_SUCCESS;

}

OS_APIs s32 OS_TimerStart( OsTimer timer )
{
    //return xTimerStart( timer, 0 );
    return OS_SUCCESS;
}

OS_APIs s32 OS_TimerStop( OsTimer timer )
{
    //return xTimerStop( timer, 0 );
    return OS_SUCCESS;
}

OS_APIs void *OS_TimerGetData( OsTimer timer )
{
   // return pvTimerGetTimerID(timer);
   return NULL;
}



#if 0
OS_APIs void OS_TimerGetSetting( OsTimer timer, u8 *autoReload, void **args )
{
    xTimerGetSetting(timer, autoReload, args);
}

OS_APIs bool OS_TimerIsRunning( OsTimer timer )
{
    return xTimerIsTimerActive(timer);
}

#endif

/*==================OS Profiling=========================*/
#if ((OS_CPU_HOOKS_EN>0)&&(OS_TASK_STAT_EN==1))
//#include <os.h>
//#include "drv_l1_timer.h"
//#include "drv_l1_sfr.h"


extern u8 cmd_top_enable;
u32 lastTinyCount=0;
u32 currentTinyCount=0;
u32 lastTickCount=0;
u32 currentTickCount=0;

u32 lastTinyCountStatTask=0;
u32 currentTinyCountStatTask=0;
u32 lastTickCountStatTask=0;
u32 currentTickCountStatTask=0;

u32 acc_cpu_usage=0;
u32 acc_counts=0;

u32 OSCtxSwCtrPerSec;
#define TINY_COUNT(x) ((x*96)/144) // tiny_counter_get is the reference
#define TINY_COUNT_TO_US(x) (((x*266)/100) )
#endif

#if OS_TASK_CREATE_EXT_EN > 0
TASK_USER_DATA taskUserData[64];
#endif

#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0) && (OS_TASK_CREATE_EXT_EN>0)
void output_message(char *s)
{
    char *p=s;
    while(*p!='\0'){
        hal_putchar(*p);
        p++;
    }

}
void TaskSwHook(void)
{
    //OS_TCB      *ptcb;
    u32 *p=NULL;
    TASK_USER_DATA *puser=OSTCBCur->OSTCBExtPtr;
    int flag = 0;
#if (OS_TASK_STAT_EN==1)
    //**************************************//
    //*********** Profile Task *********** //
    //**************************************//
    u32 time;
    currentTinyCount=R_TIMERD_UPCOUNT&0xFFFF;
    currentTickCount=OSTime;
    if((puser != (TASK_USER_DATA *)0)&&(cmd_top_enable==1)){

        puser->TaskCtr++;

        //Tiny counter as reference
        time = ((currentTinyCount|0x10000)-lastTinyCount)&0xFFFF;
        puser->TaskTotExecTimeFromTiny += time;

        //Tick counter as reference
        if(lastTickCount>currentTickCount)
            time = ((0xFFFFFFFF-lastTickCount+1)+currentTickCount);
        else
            time = (currentTickCount-lastTickCount);

        puser->TaskTotExecTimeFromTick += time;

    }
    lastTinyCount=currentTinyCount;
    lastTickCount=currentTickCount;
#endif

    //**************************************//
    //*********** check stack *********** //
    //**************************************//
    #if OS_STK_GROWTH == 1
    if(puser != (TASK_USER_DATA *)0){
        p = OSTCBCur->OSTCBStkBottom;
        if ((*p!=RTOS_STACK_CHECK_VALUE))
        {
            output_message("!!!STACK OVERFLOW  1!!! ");
            flag = 1;
        }
        if ((*(p+1)!=RTOS_STACK_CHECK_VALUE))
        {
            output_message("!!!STACK OVERFLOW 2!!! ");
            flag = 1;
        }
        if (OSTCBCur->OSTCBStkPtr<=p)
        {
            output_message("!!!STACK OVERFLOW 3!!! ");
            flag = 1;
        }
        if (flag)
        {
            output_message(puser->TaskName);
            while(1){;}
        }
        /*        
        if((*p!=FGTEST_STACK_CHECK_VALUE)||(*(p+1)!=FGTEST_STACK_CHECK_VALUE)||(OSTCBCur->OSTCBStkPtr<=p)){
            output_message("!!!STACK OVERFLOW!!! ");
            output_message(puser->TaskName);
            while(1){;}
        }
        */
    }
    #else
     * To Do *
    #endif

}
#endif

#if ((OS_CPU_HOOKS_EN>0)&&(OS_TASK_STAT_EN==1))
void TaskStatHook(void)
{
    u32 cpu_isr=0;
    cpu_isr=OS_EnterCritical();
    currentTinyCountStatTask=R_TIMERD_UPCOUNT&0xFFFF;
    currentTickCountStatTask=OSTime;

    if(cmd_top_enable==1){
        //LOG_PRINTF("current time=%d(us)\r\n",TINY_COUNT_TO_US(TINY_COUNT(R_TIMERD_UPCOUNT)));
        DispTaskStatus();
    }else if(cmd_top_enable==2){
        acc_cpu_usage+=OSCPUUsage;
        acc_counts++;
        if ((acc_cpu_usage > 4294967195) || (acc_counts > 4294967294))
        {
            u32 avg_cpu = (acc_cpu_usage/acc_counts);
            LOG_PRINTF("Update Average CPU usage = %d%% to avoid overflow\r\n", avg_cpu);
            acc_cpu_usage = 0;
            acc_counts = 0;
        }
    }
    else{
        ClearTaskStatus();
    }

    lastTinyCountStatTask=currentTinyCountStatTask;
    lastTickCountStatTask=currentTickCountStatTask;

    OS_ExitCritical(cpu_isr);
}

void DispTaskStatus(void)
{
    u8 i=0;
    u32 time2=0;
    u32 time1=0;
    OSCtxSwCtrPerSec=OSCtxSwCtr;
    OSCtxSwCtr=0;
    LOG_PRINTF("\r\n");
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0) && (OS_TASK_CREATE_EXT_EN>0)
    LOG_PRINTF("\33[32m%-30s %-10s %-10s %-10s %-10s\33[0m\r\n","Task Name","Priority","Counter","Tiny Time(us)","Tick Time(ms)");
    LOG_PRINTF("\r\n");
    for(i=0;i<sizeof(taskUserData)/sizeof(TASK_USER_DATA);i++){
        if(1==taskUserData[i].valid){
        LOG_PRINTF("%-30s %2d      %5d       %5u            %5u                 \r\n",
                taskUserData[i].TaskName,i,taskUserData[i].TaskCtr, TINY_COUNT_TO_US(TINY_COUNT(taskUserData[i].TaskTotExecTimeFromTiny)) ,taskUserData[i].TaskTotExecTimeFromTick*10 );
        taskUserData[i].TaskCtr=0;
        taskUserData[i].TaskTotExecTimeFromTiny=0;
        taskUserData[i].TaskTotExecTimeFromTick=0;
        }
    }
#endif
    LOG_PRINTF("\r\n");
    LOG_PRINTF("CPU usage = %d%% \r\n",OSCPUUsage);

    time1=((currentTinyCountStatTask|0x10000)-lastTinyCountStatTask)&0xFFFF;
    if(lastTickCountStatTask>currentTickCountStatTask)
        time2 = ((0xFFFFFFFF-lastTickCountStatTask+1)+currentTickCountStatTask);
    else
        time2 = (currentTickCountStatTask-lastTickCountStatTask);

    LOG_PRINTF("Calculate interval = %u(us,Tiny) %u(ms,Tick) \r\n",
            TINY_COUNT_TO_US(TINY_COUNT(time1)),time2*10);
}

void ClearTaskStatus(void)
{
    
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0) && (OS_TASK_CREATE_EXT_EN>0)
    u8 i=0;
    u32 avg_cpu = 0;
    if(acc_counts != 0)
    {
        avg_cpu = (acc_cpu_usage/acc_counts);
        LOG_PRINTF("Average CPU usage = %d%% \r\n", avg_cpu);
        acc_cpu_usage = 0;
        acc_counts = 0;
    }
    for(i=0;i<sizeof(taskUserData)/sizeof(TASK_USER_DATA);i++){
        if(1==taskUserData[i].valid){
        taskUserData[i].TaskCtr=0;
        taskUserData[i].TaskTotExecTimeFromTiny=0;
        taskUserData[i].TaskTotExecTimeFromTick=0;
        }
    }
#endif
    OSCtxSwCtr=0;    
}
#else
void TaskStatHook(void)
{
}
#endif




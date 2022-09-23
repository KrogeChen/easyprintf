//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//version:10
//date: 06/11/2019
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include ".\bsp_tick_small.h"
#include ".\pbc_tick_small.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define TASK_TOTAL      8   
//------------------------------------------------------------------------------
typedef struct
{
    sdt_bool       taskRun;         //运行标志
    sdt_int16u     timer;           //任务时间计数
    sdt_int16u     timerPeriod;     //任务周期
    pTaskFun_def   taskHook;          //任务函数指针
}taskSCH_def;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
taskSCH_def taskTable[TASK_TOTAL];
static sdt_bool system_cfged = sdt_false;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_int16u sysTickMillsecond;
static sdt_int16u sysTickSecond;
static sdt_int16u sysTickMinute;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void system_clock_cfg(void)
{
    sdt_int8u i;
    
    for(i = 0;i < TASK_TOTAL;i++) //期初，清理一次指针
    {
        taskTable[i].taskHook = 0;
    }  
    bsp_system_tick_cfg();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_int16u pbc_getSysTickMillisecond(void)
{
    sdt_int16u ReadData = 0;
    
    do
    {
        ReadData=sysTickMillsecond;
    }while(ReadData!=sysTickMillsecond);
    return(ReadData);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_int16u oneSecondCnt;
static sdt_int8u oneMinuteCnt;
static sdt_int16u tickBackup;
//------------------------------------------------------------------------------
static void pbc_sysTick_run(void)
{
    sdt_int16u currentTick;
    sdt_int16u goRunTick;
    sdt_int8u i;
    
    currentTick = bsp_pullSystemTickCount();
    goRunTick = currentTick - tickBackup;
    tickBackup = currentTick;
    while(goRunTick)
    {
        goRunTick--;
        sysTickMillsecond++;
        oneSecondCnt++;
        if(oneSecondCnt > 999)
        {
            oneSecondCnt = 0;
            sysTickSecond++;
            oneMinuteCnt++;
            if(oneMinuteCnt > 59)
            {
                oneMinuteCnt = 0;
                sysTickMinute++;
            }
        }
//------------------------------------------------------------------------------
        for(i = 0;i < TASK_TOTAL;i++)
        {
            if(0 == taskTable[i].taskHook)
            {
            }
            else
            {
                if(0 != taskTable[i].timer)
                {
                    taskTable[i].timer--;
                }
                if(0 == taskTable[i].timer)
                {
                    taskTable[i].taskRun = sdt_true;
                    taskTable[i].timer = taskTable[i].timerPeriod;
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++interface+++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
void pbc_task_process(void)
{
    sdt_int8u i;
    
    while(1)
    {
        if(system_cfged)
        {
            pbc_sysTick_run();
//------------------------------------------------------------------------------
            for(i = 0;i < TASK_TOTAL;i++)
            {
                if(0 == taskTable[i].taskHook)
                {
                }
                else if(taskTable[i].taskRun)
                {
                    taskTable[i].taskHook();
                    taskTable[i].taskRun = sdt_false;
                }
            }
        }
        else
        {
            system_cfged = sdt_true;
            system_clock_cfg();
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void pbc_timerClockRun_task(timerClock_def* in_timer)
{
    sdt_int16u currentTick;
    sdt_int16u goRunTick;
    sdt_int8u timer_type;
    
    pbc_sysTick_run();
    timer_type = in_timer->timStatusBits & timStatusBits_typeMask;
    if(timStatusBits_typeMillsecond == timer_type)
    {
        currentTick = sysTickMillsecond;
    }
    else if(timStatusBits_typeSecond == timer_type)
    {
        currentTick = sysTickSecond;
    }
    else if(timStatusBits_typeMinute == timer_type)
    {
        currentTick = sysTickMinute;
    }
    else
    {
        while(1);
    }
    goRunTick = currentTick - in_timer->keepTick;
    in_timer->keepTick = currentTick;
    
    if(timStatusBits_typeMillsecond == timer_type)
    {
        if(goRunTick > 2000)
        {
            in_timer->timClock = 0;
        }
    }
    else if(timStatusBits_typeSecond == timer_type)
    {
        if(goRunTick > 3)
        {
            in_timer->timClock = 0;
        }
    }
    else if(timStatusBits_typeMinute == timer_type)
    {
        if(goRunTick > 2)
        {
            in_timer->timClock = 0;
        }
    }
    
    while(goRunTick)
    {
        if(0 != in_timer->timClock)
        {
            in_timer->timClock--;
            if(0 == in_timer->timClock)
            {
                return;
            } 
        }
        else
        {
            return;
        }
        goRunTick--;
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void pbc_timerMillRun_task(timerClock_def* in_timer)
{
    in_timer->timStatusBits &= ~timStatusBits_typeMask;  //0 is millsecond type
    //in_timer->timStatusBits |= timStatusBits_typeMillsecond;
    pbc_timerClockRun_task(in_timer);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-------------------------------------------------------------------------------------------------
sdt_bool pbc_pull_timerIsCompleted(timerClock_def* in_timer)
{
    pbc_timerClockRun_task(in_timer);
    if(0 == in_timer->timClock)
    {
        return(sdt_true);
    }
    else
    {
        return(sdt_false);
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-------------------------------------------------------------------------------------------------
sdt_bool pbc_pull_timerIsOnceTriggered(timerClock_def* in_timer)
{
    pbc_timerClockRun_task(in_timer);
    if(in_timer->timStatusBits & timStatusBits_onceTriggered)
    {
    }
    else
    {
        if(0 == in_timer->timClock)
        {
            in_timer->timStatusBits |= timStatusBits_onceTriggered;
            return(sdt_true);
        }
    }
    return(sdt_false);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-------------------------------------------------------------------------------------------------
void pbc_stop_timerIsOnceTriggered(timerClock_def* in_timer)
{
    in_timer->timStatusBits |= timStatusBits_onceTriggered;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void pbc_reload_timerClock(timerClock_def* in_timer,sdt_int16u in_value)
{
    pbc_timerClockRun_task(in_timer);
    in_timer->timClock = in_value;
    in_timer->timStatusBits &= (~timStatusBits_onceTriggered);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
sdt_int8u pbc_task_create(pTaskFun_def in_taskPointer,sdt_int16u sch_period)
{
    sdt_int8u i;

    if(system_cfged)
    {
    }
    else
    {
        system_cfged = sdt_true;
        system_clock_cfg();
    }
    for(i = 0;i < TASK_TOTAL;i++)
    {
        if(0 == taskTable[i].taskHook)
        {
            taskTable[i].taskRun = sdt_false;
            taskTable[i].taskHook = in_taskPointer;
            taskTable[i].timer = 0;
            taskTable[i].timerPeriod = sch_period;
            return(i);
        }
    }
    return(Err_CreateTaskFail);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
void pbc_taskImmediatelyRun(pTaskFun_def in_taskPointer)
{
    in_taskPointer();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name: ms延时
//fun:  ms延时，阻塞模式
//in:   in_dlyms  ---延时时间
//      
//out:  none
//------------------------------------------------------------------------------
void pbc_block_delay_ms(sdt_int8u in_dlyms)
{
    sdt_int16u ref_millsecond;
    sdt_int16u interval_ms;
    
    pbc_sysTick_run();
    ref_millsecond = sysTickMillsecond;
    if(0 != in_dlyms)
    {
        while(1)
        {
            pbc_sysTick_run();
            interval_ms = sysTickMillsecond - ref_millsecond;
            if(0 != interval_ms)
            {
                ref_millsecond = sysTickMillsecond;
                while(interval_ms)
                {
                    interval_ms --;
                    in_dlyms --;
                    if(0 == in_dlyms)
                    {
                        return;
                    }
                }                
            }
        }     
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name: us延时
//fun:  us延时，阻塞模式
//in:   in_dlyus  ---延时时间
//      
//out:  none
//------------------------------------------------------------------------------
void pbc_block_delay_us(sdt_int16u in_dlyus)
{
    sdt_int16u ref_microsecond;
    sdt_int16u now_microsecond;
    sdt_int16u interval_us;
    
    ref_microsecond = bsp_pull_us_count_16bits();
    if(0 != in_dlyus)
    {
        while(1)
        {
            now_microsecond = bsp_pull_us_count_16bits();
            interval_us = now_microsecond - ref_microsecond;
            if(0 != interval_us)
            {
                ref_microsecond = now_microsecond;
                while(interval_us)
                {
                    interval_us --;
                    in_dlyus --;
                    if(0 == in_dlyus)
                    {
                        return;
                    }
                }
            }
        }     
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//获取us计数节拍
//------------------------------------------------------------------------------
sdt_int32u pbc_pull_us_tick_32bits(void)
{
    return(bsp_pull_us_count_32bits());
}
//------------------------------------------------------------------------------
sdt_int16u pbc_pull_us_tick_16bits(void)
{
    return(bsp_pull_us_count_16bits());
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef EPF_LOG_EN
sdt_bool pbc_easy_printf(sdt_int8s* in_pStr)
{
    return(bsp_easy_printf(in_pStr));
}
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
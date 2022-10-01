//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum
{
    bss_start        = 0x00,
    bss_waitUpdate   = 0x01,
    bss_legalCheck   = 0x02,
    bss_failedLegal  = 0x03,
    bss_inBootRun    = 0x04,
    bss_gotoUserApp  = 0x05,
}BOOT_STATUS_DEF;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static BOOT_STATUS_DEF bootStatus =  bss_start;
static sdt_int8u try_cnt = 0;
macro_createTimer(timer_updateWin,timerType_second,0);      //升级窗口时间
macro_createTimer(timer_try,timerType_millisecond,0);       //重试间隔
macro_createTimer(timer_toUserApp,timerType_second,0);      //进入APP区间的延时 
macro_createTimer(timer_log,timerType_millisecond,0);       //日志定时器
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_bootload_task(void)
{
    sdt_int8u update_tag;
    sdt_int8u error;
  
    pbc_timerClockRun_task(&timer_updateWin);
    pbc_timerClockRun_task(&timer_try);
    pbc_timerClockRun_task(&timer_toUserApp);
    pbc_timerClockRun_task(&timer_log);
    switch(bootStatus)
    {
        case bss_start:
        {
            error = mde_pull_stoUpdateTag(&update_tag);
            if(error)
            {
                bootStatus = bss_legalCheck;
            }
            else if(UPTAG_NEEDUP == update_tag)
            {
                pbc_reload_timerClock(&timer_updateWin,30);  //30秒的升级窗口时间
                bootStatus = bss_waitUpdate;
            }
            else
            {
                bootStatus = bss_legalCheck;
            }
            break;
        }
        case bss_waitUpdate:
        {
            if(pbc_pull_timerIsCompleted(&timer_updateWin))
            {
                mde_push_stoUpdateTag(UPTAG_NONE);         //清除升级标识
                bootStatus = bss_legalCheck;
            }
            break;
        }
        case bss_legalCheck:
        {
            if(mde_pull_appSectionFiles_iegal(HARDWARE_MARK))  
            {
                jumpToApplication();   //user.app数据合法
                //bootStatus = bss_inBootRun;
            }
            else
            {
                pbc_reload_timerClock(&timer_try,200); 
                bootStatus = bss_failedLegal;
            }
            break;
        }
        case bss_failedLegal:
        {
            if(pbc_pull_timerIsCompleted(&timer_try))
            {
                if(try_cnt < 5)
                {
                    try_cnt++;
                    bootStatus = bss_start;
                }                
            }
            break;
        }
        case bss_inBootRun:
        {
            break;
        }
        case bss_gotoUserApp:
        {
            if(pbc_pull_timerIsCompleted(&timer_toUserApp))
            {
                bootStatus = bss_legalCheck;
            }
            break;
        }
        default:
        {
            bootStatus = bss_start;
            break;
        }
    }
    if(pbc_pull_timerIsCompleted(&timer_log))
    {
        pbc_reload_timerClock(&timer_log,1000);
        pbc_easy_printf("boot");
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:驻留升级运行
//------------------------------------------------------------------------------
void app_bss_reside_update(void)
{
    if(bss_inBootRun != bootStatus)
    {
        bootStatus = bss_inBootRun;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_bss_goto_userApp(void)
{
    mde_push_stoUpdateTag(UPTAG_NONE);         //清除升级标识
    bootStatus = bss_gotoUserApp;
    pbc_reload_timerClock(&timer_toUserApp,4);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "app_cfg.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//绝对定位放置静态数据
//------------------------------------------------------------------------------
#pragma location = ".hardware_flag"
__root const sdt_int8u hardwareType[4] = {
                                             (HARDWARE_FLAG>>24)&0x000000FF,
                                             (HARDWARE_FLAG>>16)&0x000000FF,
                                             (HARDWARE_FLAG>>8)&0x000000FF,
                                             HARDWARE_FLAG&0x000000FF,
                                         };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//share ram
//------------------------------------------------------------------------------
sdt_int8u g_share_buff[128];
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
macro_cTimerTig(timer_epf,timerType_millisecond)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_general_task(void)
{
    mde_watchdog_reload();
    mde_pilot_light_task();
    mde_epf_accpet_task();
    app_mbus_task();
    app_easyink_reboot_task();
    
    pbc_timerClockRun_task(&timer_epf);
    if(pbc_pull_timerIsCompleted(&timer_epf))
    {
        pbc_reload_timerClock(&timer_epf,1000);
        pbc_easy_printf("This EPF test is succeed,luck for you!");
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
void main(void)
{
//------------------------------------------------------------------------------
//config
    mde_watchdog_cfg();
//    app_read_storage();
//    mde_dc_motor_push_cfg((SYNC_DCM_ARGU_PDEF)sync_dcMotor_argument);
//    mde_pt1000_cfg((SYNC_PT1000_ARGU_PDEF)sync_pt1000_argument);
    mde_rmcd_char_cfg();
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//create task
    pbc_task_create(app_general_task,0);
    pbc_task_create(app_epf_message_task,0);
    pbc_task_process();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
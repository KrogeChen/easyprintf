//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_general_task(void)
{
    macro_createTimer(timer_sysrst,timerType_millisecond,3000);
    static sdt_bool cfged = sdt_false;

    if(cfged)
    {
//------------------------------------------------------------------------------
        mde_watchdog_reload();
        mde_pilot_light_task();
        pbc_pull_us_tick_16bits();
        app_modbus_slave_task();
        app_measure_stroke_task();
        app_cmd_message_task();
        app_storage_task();
        mde_digit_input_task();
        mde_digit_output_task();
        mde_ws2812b_task();
//------------------------------------------------------------------------------
        if(pbc_pull_timerIsOnceTriggered(&timer_sysrst))
        {
            mde_push_pilot_light_ldms(0,500,500); //3s复位常亮,改变状态灯,闪亮
        }
//------------------------------------------------------------------------------
        macro_createTimer(timer_logtx,timerType_millisecond,10500);
        
        pbc_timerClockRun_task(&timer_logtx);
        if(pbc_pull_timerIsCompleted(&timer_logtx))
        {
            pbc_reload_timerClock(&timer_logtx,2000);
            //while(1);
        } 
//------------------------------------------------------------------------------
    }
    else
    {
        cfged = sdt_true;
        app_read_sto_parameter();
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void main(void)
{
    mde_watchdog_cfg();
    pbc_task_create(app_general_task,0);
//------------------------------------------------------------------------------
    pbc_task_process();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
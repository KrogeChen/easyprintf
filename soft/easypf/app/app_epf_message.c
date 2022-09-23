//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_int8s efp_char[128];
static sdt_int8u index = 0;
void app_epf_message_task(void)
{
    sdt_bool have_msg;
    sdt_int8s char_data;

    mde_epf_accpet_task();
    if(bsp_pull_accpet_one_byte(&have_msg,&char_data))
    {
        efp_char[index] = char_data;
        index++;
        if(index > 120)
        {
            index = 0;
        }
    }

    if(have_msg)
    {
        mde_push_pilot_light_pulsems(1,100);
        //mde_push_pilot_light_pulsems(2,100);
        efp_char[index] = sdt_chars_cr;
        efp_char[index + 1] = sdt_chars_lf;
        efp_char[index + 2] = 0;
        index = 0;
        mde_rmcd_transfet_string(efp_char);
    }
    else
    {

    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
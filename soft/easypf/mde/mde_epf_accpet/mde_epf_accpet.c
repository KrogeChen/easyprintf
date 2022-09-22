//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\bsp_epf_accpet.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_bool cfged = sdt_false;
void mde_epf_accpet_task(void)
{
    if(cfged)
    {
        bsp_epf_accpet_task();
    }
    else
    {
        cfged = sdt_true;
        bsp_epf_accpet_cfg();
    }
    
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
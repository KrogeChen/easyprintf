//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\bsp_rmcd_char.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mde_rmcd_char_task(void)
{

    
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void mde_rmcd_char_cfg(void)
{
    bsp_rmcd_char_cfg();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_int16u mde_rmcd_transfet_string(sdt_int8u* in_pString)
{
    sdt_int16u remain_bytes;
    sdt_int16u expect_bytes;
    
    expect_bytes = 0;
    while(0 != in_pString[expect_bytes])
    {
        expect_bytes++;
    }
    remain_bytes = bsp_transfet_bytes_tx_rmcd((sdt_int8u*)in_pString,expect_bytes);
    return(remain_bytes);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include "iostm8l052c6.h"
#include "intrinsics.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//STM8L052C6T6时钟配置模块
//------------------------------------------------------------------------------
//Timer2 ms tick 
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:STM8L052C6T6时钟配置
//功能:配置晶振、系统时钟
//HSI 16M
//入口:无
//出口:无
//------------------------------------------------------------------------------
static void bsp_clock_cfg(void)
{
    if(0 == CLK_ICKCR_bit.HSION)
    {
        CLK_ICKCR_bit.HSION = 1;
    }
    while(0 == CLK_ICKCR_bit.HSIRDY);
    if(0x01 != CLK_SCSR)
    {
        CLK_SWCR_bit.SWEN = 1;
        CLK_SWR = 0x01;                 //HSI
        while(0x01 != CLK_SCSR);
    }
    CLK_SWCR_bit.SWEN = 0;
    CLK_CKDIVR_bit.CKM = 0;         //Presacler =1
    
    #ifdef NDEBUG
        IWDG_KR = 0xAA;              //Reload Watchdog Count
    #endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_int16u tick_millsecond;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_int16u bsp_pullSystemTickCount(void)
{
    return(tick_millsecond);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:系统滴答时钟配置
//功能:中断方式
//TIMER2
//1ms的定时
//------------------------------------------------------------------------------
#define Poied    (16000-1)
//------------------------------------------------------------------------------
void bsp_system_tick_cfg(void)
{
    bsp_clock_cfg();
    
    CLK_PCKENR1_bit.PCKEN10 = 1;   //Enable Tim2 Peripheral clock
    TIM2_CR1 = 0x80;
    TIM2_PSCR_bit.PSC = 0;
    TIM2_ARRH = (sdt_int8u)(Poied>>8);
    TIM2_ARRL = (sdt_int8u)(Poied&0x00FF);
    TIM2_IER_bit.UIE = 1;
    TIM2_CR1_bit.CEN = 1;
    __enable_interrupt();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//TIM2 update interrupt
//------------------------------------------------------------------------------
#pragma vector = TIM2_OVR_UIF_vector
 __interrupt void TIM2_ISR(void)
 {
     if(TIM2_SR1_bit.UIF)
     {
         TIM2_SR1_bit.UIF = 0;
         tick_millsecond++;
     }
 }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_int32u bsp_pull_us_count_32bits(void)
{
    static sdt_int32u rd_us_cnt;


    rd_us_cnt++;
    
    return(rd_us_cnt);
}
//------------------------------------------------------------------------------
sdt_int16u bsp_pull_us_count_16bits(void)
{
    static sdt_int16u rd_us_cnt;
    
    rd_us_cnt++;
    return(rd_us_cnt);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
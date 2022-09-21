//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include "sysctrl.h"
#include "intrinsics.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//RCH 16M
//PLL 32M
//HCLK 32M
//PCLK 32M
//-----------------------------------------------------------------------------
//Timer0  us tick
//systick ms tick 
//-----------------------------------------------------------------------------*/
#define RC_TRIM_BASE_ADDR           ((volatile uint16_t*)   (0x00100C00ul))
#define RCH_CR_TRIM_24M_VAL         (*((volatile uint16_t*) (0x00100C00ul)))
#define RCH_CR_TRIM_22_12M_VAL      (*((volatile uint16_t*) (0x00100C02ul)))
#define RCH_CR_TRIM_16M_VAL         (*((volatile uint16_t*) (0x00100C04ul)))
#define RCH_CR_TRIM_8M_VAL          (*((volatile uint16_t*) (0x00100C06ul)))
#define RCH_CR_TRIM_4M_VAL          (*((volatile uint16_t*) (0x00100C08ul)))

#define RCL_CR_TRIM_38400_VAL       (*((volatile uint16_t*) (0x00100C20ul)))
#define RCL_CR_TRIM_32768_VAL       (*((volatile uint16_t*) (0x00100C22ul)))
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:HC32L136时钟配置
//功能:配置晶振、FLASH等待时间、系统时钟
//RCH 16M
//入口:无,在系统复位后,第一个调用
//出口:无
//-----------------------------------------------------------------------------
static void bsp_clock_cfg(void)
{
   #if defined (NDEBUG) && defined (BOOT_SUPPORT) //
       *(volatile uint32_t*)0xE000ED08 = 0x00000800;  //app_vect_table
   #endif
   M0P_SYSCTRL->PERI_CLKEN_f.FLASH = 1;
   M0P_BGR->CR_f.BGR_EN = 1;          //需要20us稳定时间,采用RCL时钟,38k，约一个指令周期，BGR PLL使用
   //时钟切换到RCL                                    
   M0P_SYSCTRL->RCL_CR_f.TRIM = RCL_CR_TRIM_38400_VAL;
   M0P_SYSCTRL->RCL_CR_f.STARTUP = 0x02;   //64个周期
   while(0 == M0P_SYSCTRL->SYSCTRL0_f.RCL_EN)
   {
       M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
       M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
       M0P_SYSCTRL->SYSCTRL0_f.RCL_EN = 1;       
   }
   while(0 == M0P_SYSCTRL->RCL_CR_f.STABLE)
   {
       #ifdef NDEBUG
       M0P_WDT->RST = 0x1E;
       M0P_WDT->RST = 0xE1;
       #endif
   }
   M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;            //unlock
   M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
   M0P_SYSCTRL->SYSCTRL0_f.CLKSW = SysctrlClkRCL;
   
   M0P_FLASH->BYPASS = 0x5A5A;
   M0P_FLASH->BYPASS = 0xA5A5;
   M0P_FLASH->CR_f.WAIT = 0x01;                 //flash 等待时间
//----------------------------------------------------
   //时钟切换到RCH+PLL
   M0P_SYSCTRL->RCH_CR_f.TRIM = RCH_CR_TRIM_16M_VAL;  //16M
   while(0 == M0P_SYSCTRL->SYSCTRL0_f.RCH_EN)
   {
       M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
       M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
       M0P_SYSCTRL->SYSCTRL0_f.RCH_EN = 1;
   }
   while(0 == M0P_SYSCTRL->RCH_CR_f.STABLE)
   {
       #ifdef NDEBUG
       M0P_WDT->RST = 0x1E;
       M0P_WDT->RST = 0xE1;
       #endif
   }
//----------------------------------------------------
   //PLL
   M0P_SYSCTRL->PLL_CR_f.REFSEL = 0x03;  //RCH source
   M0P_SYSCTRL->PLL_CR_f.FOSC = 0x03;             //24-36M
   M0P_SYSCTRL->PLL_CR_f.DIVN = 2;                //2*16 = 32M
   M0P_SYSCTRL->PLL_CR_f.FRSEL = 0x02;
   M0P_SYSCTRL->PLL_CR_f.STARTUP = 0x03;    //1024 cyc
   while(0 == M0P_SYSCTRL->SYSCTRL0_f.PLL_EN)
   {
       M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
       M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
       M0P_SYSCTRL->SYSCTRL0_f.PLL_EN = 1;
   }
   while(0 == M0P_SYSCTRL->PLL_CR_f.STABLE)
   {
       #ifdef NDEBUG
       M0P_WDT->RST = 0x1E;
       M0P_WDT->RST = 0xE1;
       #endif
   }
//----------------------------------------------------
   M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
   M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
   M0P_SYSCTRL->SYSCTRL0_f.CLKSW = SysctrlClkPLL;

   
   M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
   M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
   M0P_SYSCTRL->SYSCTRL0_f.RCL_EN = 0;     //关闭RCL
   
   M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
   M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
   M0P_SYSCTRL->SYSCTRL0_f.HCLK_PRS = SysctrlHclkDiv1;  //HCLK 32M
     
   M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
   M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
   M0P_SYSCTRL->SYSCTRL0_f.PCLK_PRS = SysctrlPclkDiv1;  //PCLK 32M

}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_int16u tick_millsecond;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_int16u bsp_pullSystemTickCount(void)
{
    return(tick_millsecond);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:系统滴答时钟配置
//功能:滴答时钟源选择,中断开启
//SysTick_Config函数,采用AHB时钟源
//1ms的定时
//-------------------------------------------------------------------------------------------------

void bsp_system_tick_cfg(void)
{
    bsp_clock_cfg();
    
//    SysTick_Config(32000);   //1ms
//-------------------------------------------------------------------------------
//    M0P_SYSCTRL->PERI_CLKEN_f.BASETIM = 1;  //TIM0
//    M0P_TIM0_MODE0->M0CR_f.MODE = 0;
//    M0P_TIM0_MODE0->M0CR_f.MD = 0;      //32bit   
//    M0P_TIM0_MODE0->M0CR_f.PRS = 0x04;  //PCLK/16  1us
//    M0P_TIM0_MODE0->M0CR_f.CT  = 0;     //PCLK
//    M0P_TIM0_MODE0->M0CR_f.CTEN = 1;
//-------------------------------------------------------------------------------
    M0P_SYSCTRL->PERI_CLKEN_f.BASETIM = 1;  //TIM0
    M0P_TIM0_MODE23->M23CR_f.MODE = 2;
    M0P_TIM0_MODE23->M23CR_f.PRS = 0x05;  //PCLK/32  1us
    M0P_TIM0_MODE23->M23CR_f.CT  = 0;     //PCLK
    M0P_TIM0_MODE23->M23CR_f.UIE = 1;
    M0P_TIM0_MODE23->CRCH0_f.CIEA = 1;
    M0P_TIM0_MODE23->CCR0A_f.CCR0A = 1000;
    M0P_TIM0_MODE23->ARR_f.ARR = 0xffff;
    M0P_TIM0_MODE23->M23CR_f.CTEN = 1;
    EnableNvic(TIM0_IRQn,IrqLevel0,TRUE);
//------------------------------------------------------------------------------
    __enable_interrupt();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//系统时钟中断，1ms
//------------------------------------------------------------------------------
void SysTick_Handler(void)
{
    //tick_millsecond++;
}
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//UIF,32位微秒计数处理
//CCR0A CA0F 比较中断,1ms系统时隙
//------------------------------------------------------------------------------
static sdt_int16u tick_mircs_h;
//------------------------------------------------------------------------------
void TIM0_IRQHandler(void)
{
    sdt_int16u distance_t;

    if(0 != M0P_TIM0_MODE23->IFR_f.CA0F)
    {
        do
        {
            M0P_TIM0_MODE23->ICLR_f.CA0F = 0;
            M0P_TIM0_MODE23->CCR0A_f.CCR0A += 1000;
            distance_t = M0P_TIM0_MODE23->CCR0A_f.CCR0A;
            distance_t = distance_t - M0P_TIM0_MODE23->CNT_f.CNT;
            tick_millsecond ++;            
        }while(distance_t > 1000);  //此现象由某个中断大于1ms的情况造成
    }
    if(0 != M0P_TIM0_MODE23->IFR_f.UIF)
    {
        M0P_TIM0_MODE23->ICLR_f.UIF = 0;
        tick_mircs_h ++;
    }
}
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_int32u bsp_pull_us_count_32bits(void)
{
    sdt_int32u rd_us_cnt;
    sdt_int16u rd_us_h;

    do
    {
        rd_us_h = tick_mircs_h;
        rd_us_cnt = rd_us_h;
        rd_us_cnt = rd_us_cnt << 16;
        rd_us_cnt |= M0P_TIM0_MODE23->CNT_f.CNT;
    }while(rd_us_h != tick_mircs_h);
    
    return(rd_us_cnt);
}
//------------------------------------------------------------------------------
sdt_int16u bsp_pull_us_count_16bits(void)
{
    sdt_int16u rd_us_cnt;
    
    rd_us_cnt = M0P_TIM0_MODE23->CNT_f.CNT;
    return(rd_us_cnt);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
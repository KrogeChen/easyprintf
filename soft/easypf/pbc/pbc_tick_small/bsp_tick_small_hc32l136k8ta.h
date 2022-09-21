//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include "sysctrl.h"
#include "intrinsics.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//XTH   16M
//PLL   48M
//HCLK  48M
//PCLK  48M
//-----------------------------------------------------------------------------
//Timer2  us tick 
//Timer2  ms tick 
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
   //时钟切换到XTH+PLL
    M0P_SYSCTRL->PERI_CLKEN_f.GPIO = 1;
    M0P_GPIO->PDADS_f.PD00 = 1;
    M0P_GPIO->PDADS_f.PD01 = 1;
    M0P_SYSCTRL->XTH_CR = 0x0000003A;   //00 11 10 10
    
    while(0 == M0P_SYSCTRL->SYSCTRL0_f.XTH_EN)
    {
        M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
        M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
        M0P_SYSCTRL->SYSCTRL0_f.XTH_EN = 1;
    }
    while(0 == M0P_SYSCTRL->XTH_CR_f.STABLE)
    {
       #ifdef NDEBUG
       M0P_WDT->RST = 0x1E;
       M0P_WDT->RST = 0xE1;
       #endif
    }
    volatile sdt_int16u i;//>10ms
    for(i = 0;i < 500;i++)
    {
      
    }
    #ifdef NDEBUG
    M0P_WDT->RST = 0x1E;
    M0P_WDT->RST = 0xE1;
    #endif
//----------------------------------------------------
   //PLL
   M0P_SYSCTRL->PLL_CR_f.REFSEL = 0x00;     //XTH source
   M0P_SYSCTRL->PLL_CR_f.FOSC = 0x04;       //36-48M
   M0P_SYSCTRL->PLL_CR_f.DIVN = 3;          //3*16 = 48M
   M0P_SYSCTRL->PLL_CR_f.FRSEL = 0x02;      //in osc 16M
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
   M0P_SYSCTRL->SYSCTRL0_f.HCLK_PRS = SysctrlHclkDiv1;  //HCLK 48M
     
   M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
   M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
   M0P_SYSCTRL->SYSCTRL0_f.PCLK_PRS = SysctrlPclkDiv1;  //PCLK 48M

}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_int16u tick_millsecond;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_int16u bsp_pullSystemTickCount(void)
{
    return(tick_millsecond);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//EPF 端口:PA02
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:系统滴答时钟配置
//功能:滴答时钟源选择,中断开启
//SysTick_Config函数,采用AHB时钟源
//1ms的定时
//------------------------------------------------------------------------------
#define BASE_CNT        375
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void bsp_system_tick_cfg(void)
{
    bsp_clock_cfg();
    
//    SysTick_Config(32000);   //1ms
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    M0P_SYSCTRL->PERI_CLKEN_f.BASETIM = 1;  //TIM012
    M0P_TIM2_MODE23->M23CR_f.MODE = 2;
    M0P_TIM2_MODE23->M23CR_f.PRS = 0x06;   //PCLK/64  750k 1.3us
    M0P_TIM2_MODE23->M23CR_f.CT  = 0;      //PCLK
    M0P_TIM2_MODE23->M23CR_f.UIE = 1;
    M0P_TIM2_MODE23->CRCH0_f.CIEA = 1;
    M0P_TIM2_MODE23->CCR0A_f.CCR0A = BASE_CNT;
    M0P_TIM2_MODE23->ARR_f.ARR = 0xffff;
    M0P_TIM2_MODE23->M23CR_f.CTEN = 1;
    EnableNvic(TIM2_IRQn,IrqLevel0,TRUE);
//------------------------------------------------------------------------------
    M0P_SYSCTRL->PERI_CLKEN_f.GPIO = 1;
    M0P_GPIO->PA02_SEL = 0;
    M0P_GPIO->PA02_SEL = 0;
    M0P_GPIO->PADIR_f.PA02 = 0;  //output
    M0P_GPIO->PAOUT_f.PA02 = 0;
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
//CCR0A CA0F 比较中断,500u中断,1ms系统时隙
//------------------------------------------------------------------------------
static sdt_int16u tick_mircs_h;
//------------------------------------------------------------------------------
static sdt_int8u epf_buff[64];
static sdt_int8u epf_pmd_in;
static sdt_int8u epf_pmd_ot;
static sdt_int32u epf_shift_reg;
static sdt_int32u epf_shift_mask;
static sdt_int32u epf_status_reg;
#define ESRG_PERAMBLE     0x01
#define ESRG_SYNC         0x02
#define ESRG_PAYLOAD      0x04
#define ESRG_EOF          0x08
//------------------------------------------------------------------------------
void TIM2_IRQHandler(void)
{
    static sdt_int8u base_tick;
    sdt_int8u temp_8u;
//    static sdt_int16u distance_t;

    if(0 != M0P_TIM2_MODE23->IFR_f.CA0F)
    {
        M0P_TIM2_MODE23->ICLR_f.CA0F = 0;
        M0P_TIM2_MODE23->CCR0A_f.CCR0A += BASE_CNT;
        base_tick ++;
        if(base_tick & 0x01)
        {
            tick_millsecond ++;
        }

        if(0 == epf_shift_mask)
        {
            if(epf_status_reg & ESRG_PERAMBLE)
            {
                epf_status_reg &= ~ESRG_PERAMBLE;
                epf_shift_reg = 0x00005555;
                epf_shift_mask = 0x0008000;
            }
            else if(epf_status_reg & ESRG_SYNC)
            {
                epf_status_reg &= ~ESRG_SYNC;
                epf_shift_reg = 0x0000007F;
                epf_shift_mask = 0x0000080;
            }
            else if(epf_status_reg & ESRG_PAYLOAD)
            {
                if(epf_pmd_in != epf_pmd_ot)
                {
                    temp_8u = epf_buff[epf_pmd_ot];//取字节数据
                    epf_pmd_ot++;
                    if(epf_pmd_ot > (sizeof(epf_buff) - 1))
                    {
                        epf_pmd_ot = 0;
                    }
                    if(epf_pmd_ot == epf_pmd_in)
                    {
                        epf_status_reg &= ~ESRG_PAYLOAD;
                    }
                    epf_shift_reg = 0;//生成发送电平
                    if(temp_8u & 0x01)
                    {
                        epf_shift_reg |= 0x000000001;
                    }
                    else 
                    {
                        epf_shift_reg |= 0x000000007;
                    }
                    if(temp_8u & 0x02)
                    {
                        epf_shift_reg |= (0x000000001 << 4);
                    }
                    else 
                    {
                        epf_shift_reg |= (0x000000007 << 4);
                    }
                    if(temp_8u & 0x04)
                    {
                        epf_shift_reg |= (0x000000001 << 8);
                    }
                    else 
                    {
                        epf_shift_reg |= (0x000000007 << 8);
                    }
                    if(temp_8u & 0x08)
                    {
                        epf_shift_reg |= (0x000000001 << 12);
                    }
                    else 
                    {
                        epf_shift_reg |= (0x000000007 << 12);
                    }
                    if(temp_8u & 0x10)
                    {
                        epf_shift_reg |= (0x000000001 << 16);
                    }
                    else 
                    {
                        epf_shift_reg |= (0x000000007 << 16);
                    }
                    if(temp_8u & 0x20)
                    {
                        epf_shift_reg |= (0x000000001 << 20);
                    }
                    else 
                    {
                        epf_shift_reg |= (0x000000007 << 20);
                    }
                    if(temp_8u & 0x40)
                    {
                        epf_shift_reg |= (0x000000001 << 24);
                    }
                    else 
                    {
                        epf_shift_reg |= (0x000000007 << 24);
                    }
                    if(temp_8u & 0x80)
                    {
                        epf_shift_reg |= (0x000000001 << 28);
                    }
                    else 
                    {
                        epf_shift_reg |= (0x000000007 << 28);
                    }
                    epf_shift_mask = 0x80000000;
                }
            }
            else if(epf_status_reg & ESRG_EOF)
            {
                epf_status_reg &= ~ESRG_EOF;
                epf_shift_reg = 0x00000001;
                epf_shift_mask = 0x0000080;
            }
        }
        if(0 != epf_shift_mask)
        {
            if(epf_shift_reg & epf_shift_mask)
            {
                M0P_GPIO->PAOUT_f.PA02 = 0;
            }
            else
            {
                M0P_GPIO->PAOUT_f.PA02 = 1;
            }
            epf_shift_mask = epf_shift_mask >> 1;
        }
//       do
//       {
//           M0P_TIM2_MODE23->ICLR_f.CA0F = 0;
//           M0P_TIM2_MODE23->CCR0A_f.CCR0A += BASE_CNT;
//           distance_t = M0P_TIM2_MODE23->CCR0A_f.CCR0A;
//           distance_t = distance_t - M0P_TIM2_MODE23->CNT_f.CNT;
//           base_tick ++;
//           if(base_tick & 0x01)
//           {
//               tick_millsecond ++;    
//           }
//       }while(distance_t > BASE_CNT);  //此现象由某个其他中断大于500us的情况造成
    }
    if(0 != M0P_TIM2_MODE23->IFR_f.UIF)
    {
        M0P_TIM2_MODE23->ICLR_f.UIF = 0;
        tick_mircs_h ++;               //us 高16bit
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
        rd_us_cnt |= M0P_TIM2_MODE23->CNT_f.CNT;
    }while(rd_us_h != tick_mircs_h);
    
    return(rd_us_cnt);
}
//------------------------------------------------------------------------------
sdt_int16u bsp_pull_us_count_16bits(void)
{
    sdt_int16u rd_us_cnt;
    
    rd_us_cnt = M0P_TIM2_MODE23->CNT_f.CNT;
    return(rd_us_cnt);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_bool bsp_easy_printf(sdt_int8s* in_pStr)
{
    sdt_int8u full;
    sdt_int8u rd_pmd_in;
    sdt_int8u rd_pmd_ot;

    full = sdt_false;
    rd_pmd_in = epf_pmd_in;
//    __disable_interrupt();
    rd_pmd_ot = epf_pmd_ot;
//    __enable_interrupt();
    rd_pmd_in ++;
    if(rd_pmd_in > (sizeof(epf_buff) - 1))
    {
        rd_pmd_in = 0;
    }
    
    if(rd_pmd_in == rd_pmd_ot)//full
    {
    }
    else
    {
        rd_pmd_ot --;
        if(rd_pmd_ot > (sizeof(epf_buff) - 1))
        {
            rd_pmd_ot = (sizeof(epf_buff) - 1);
        }
        while(0 != *in_pStr)
        {
            epf_buff[epf_pmd_in] = (sdt_int8u)*in_pStr;
            if(epf_pmd_in == rd_pmd_ot)//buff is full
            {
                full = sdt_true;
                break;
            }
            __disable_interrupt();
            epf_pmd_in++;
            if(epf_pmd_in > (sizeof(epf_buff) - 1))
            {
                epf_pmd_in = 0;
            }
            __enable_interrupt();
            in_pStr++;
        }
        if(0 == epf_status_reg)
        {
            epf_status_reg = (ESRG_PERAMBLE + ESRG_SYNC + ESRG_PAYLOAD + ESRG_EOF);
        }
        else
        {
            epf_status_reg |= ESRG_PAYLOAD;
        }
    }                         
    return(full);                 
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
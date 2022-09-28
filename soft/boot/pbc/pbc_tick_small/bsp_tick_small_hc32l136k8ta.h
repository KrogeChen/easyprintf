//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include "sysctrl.h"
#include "intrinsics.h"
#include "bsp_tick_small.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//XTH   16M
//PLL   48M
//HCLK  48M
//PCLK  48M
//-----------------------------------------------------------------------------
//Timer2  us tick 
//Timer2  ms tick 
//-----------------------------------------------------------------------------
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
//XTH16M
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
//LED 端口:PB02
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:系统滴答时钟配置
//功能:TIM2配置为系统时钟定时器
//500us的中断定时器
//------------------------------------------------------------------------------
#define BASE_CNT        375
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void bsp_system_tick_cfg(void)
{
    bsp_clock_cfg();
    
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
    EnableNvic(TIM2_IRQn,IrqLevel0,TRUE); //优先等级最高
//------------------------------------------------------------------------------
    #ifdef EPF_LOG_EN
    M0P_SYSCTRL->PERI_CLKEN_f.GPIO = 1;   //EPF字符端口
    M0P_GPIO->PA02_SEL = 0;
    M0P_GPIO->PADIR_f.PA02 = 0;  //output
    M0P_GPIO->PAOUT_f.PA02 = 0;    
    M0P_GPIO->PB02_SEL = 0;
    M0P_GPIO->PBDIR_f.PB02 = 0;  //output
    M0P_GPIO->PBOUT_f.PB02 = 0;  
    #endif

//------------------------------------------------------------------------------
    __enable_interrupt();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//UIF,32位微秒计数处理
//CCR0A CA0F 比较中断,500u中断,1ms系统时隙
//------------------------------------------------------------------------------
static sdt_int16u tick_mircs_h;
//------------------------------------------------------------------------------
#ifdef EPF_LOG_EN
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
#endif
//------------------------------------------------------------------------------
void TIM2_IRQHandler(void)
{
    static sdt_int8u base_tick;
    

    if(0 != M0P_TIM2_MODE23->IFR_f.CA0F)
    {
        M0P_TIM2_MODE23->ICLR_f.CA0F = 0;
        M0P_TIM2_MODE23->CCR0A_f.CCR0A += BASE_CNT;
        base_tick ++;
        if(base_tick & 0x01)
        {
            tick_millsecond ++;
        }
        #ifdef EPF_LOG_EN
        sdt_int8u temp_8u;
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
                M0P_GPIO->PBOUT_f.PB02 = 1;
            }
            else
            {
                M0P_GPIO->PAOUT_f.PA02 = 1;
                M0P_GPIO->PBOUT_f.PB02 = 0;
            }
            epf_shift_mask = epf_shift_mask >> 1;
        }        
        #endif

//------------------------------------------------------------------------------
//       static sdt_int16u distance_t;
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
//EPF打印接口
//in:字符串文本
//out:失败指示,sdt_true--转移失败(缓冲区无法容纳转移的字符串长度)
//------------------------------------------------------------------------------
#ifdef EPF_LOG_EN
sdt_bool bsp_easy_printf(sdt_int8s* in_pStr)
{
    
    sdt_int8u full;
    sdt_int8u rd_pmd_in;
    sdt_int8u rd_pmd_ot;
    sdt_int8u trs_bytes;
    sdt_int8u i;

    full = sdt_true;
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
        //计算字符串和缓冲区的大小,决定是否转移数据
        if(epf_pmd_in == rd_pmd_ot)//buff is full
        {
            return(sdt_true);
        }
        else if(epf_pmd_in > rd_pmd_ot)
        {
            trs_bytes = sizeof(epf_buff) - (epf_pmd_in - rd_pmd_ot);
        }
        else 
        {
            trs_bytes = (rd_pmd_ot - epf_pmd_in);
        }
        for(i = 0;i < trs_bytes;i++)
        {
            if(0 == in_pStr[i])
            {
                full = sdt_false;
                break;
            }            
        }
        if(sdt_false == full)
        {
            i = 0;
            while(0 != in_pStr[i])//转移数据
            {
                epf_buff[epf_pmd_in] = (sdt_int8u)in_pStr[i];
                __disable_interrupt();
                epf_pmd_in++;
                if(epf_pmd_in > (sizeof(epf_buff) - 1))
                {
                    epf_pmd_in = 0;
                }
                __enable_interrupt();
                i++;
            }
            __disable_interrupt();
            if(0 == epf_status_reg)
            {
                epf_status_reg = (ESRG_PERAMBLE + ESRG_SYNC + ESRG_PAYLOAD + ESRG_EOF);
            }
            else
            {
                epf_status_reg |= ESRG_PAYLOAD;
            }
            __enable_interrupt();          
        }
    }                         
    return(full);
}
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
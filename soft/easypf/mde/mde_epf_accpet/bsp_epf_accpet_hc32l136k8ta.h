//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef snail_data_types
    #include ".\snail_data_types.h"
#endif
//------------------------------------------------------------------------------
#include "hc32l13x.h"
#include "sysctrl.h"
#include "intrinsics.h"
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//此模块负责电平宽度的测量和数据解码
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//PA00 TIM0_CHA 低电平测量
//PA01 TIM1_CHB 高电平测量
//------------------------------------------------------------------------------
//TIM0 TIM1运行PWC模式
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//PCLK 48M
//TIM SCK 1.5M(48/32),500us--750CNT(电平宽度基准值),最大电平宽度基准值5250.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//电平宽度缓冲区,最高位代表电平类型
#define EPF_ELTY_HIGH      0x8000
static sdt_int16u epf_elwdt_buff[128];
static sdt_int8u  epf_elwdt_in;
static sdt_int8u  epf_elwdt_ot;
static sdt_int16u ref_elwidth;//电平基准宽度,前导计算值
static sdt_bool overflow;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: EPF 接收模块配置
//功能: 
//入口: 
//
//出口: 
//------------------------------------------------------------------------------
void bsp_epf_accpet_cfg(void)
{
    M0P_SYSCTRL->PERI_CLKEN_f.GPIO = 1;
//------------------------------------------------------------------------------
    M0P_GPIO->PA00_SEL = 0x07;//TIM0_CHA
    M0P_GPIO->PADIR_f.PA00 = 1;  //input
    M0P_GPIO->PA01_SEL = 0x05;//TIM1_CHB
    M0P_GPIO->PADIR_f.PA01 = 1;  //input
//------------------------------------------------------------------------------
    M0P_SYSCTRL->PERI_CLKEN_f.BASETIM = 1;  //TIM012
    M0P_TIM0_MODE1->M1CR_f.MODE = 1;
    M0P_TIM0_MODE1->M1CR_f.PRS = 0x05;  //PCLK/32
    M0P_TIM0_MODE1->M1CR_f.EDG2ND = 0;
    M0P_TIM0_MODE1->M1CR_f.EDG1ST = 1;
    M0P_TIM0_MODE1->M1CR_f.CT = 0;  //PCLK
    //M0P_TIM0_MODE1->M1CR_f.UIE = 1;
    M0P_TIM0_MODE1->MSCR_f.TS = 0x06; //CH0A
    M0P_TIM0_MODE1->FLTR_f.FLTA0 = 0;
    M0P_TIM0_MODE1->CR0_f.CIEA = 1;
    
    M0P_TIM0_MODE1->M1CR_f.CTEN = 1;
    
    EnableNvic(TIM0_IRQn,IrqLevel1,TRUE); //优先等级
    
    M0P_SYSCTRL->PERI_CLKEN_f.BASETIM = 1;  //TIM012
    M0P_TIM1_MODE1->M1CR_f.MODE = 1;
    M0P_TIM1_MODE1->M1CR_f.PRS = 0x05;  //PCLK/32
    M0P_TIM1_MODE1->M1CR_f.EDG2ND = 1;
    M0P_TIM1_MODE1->M1CR_f.EDG1ST = 0;
    M0P_TIM1_MODE1->M1CR_f.CT = 0;  //PCLK
    //M0P_TIM1_MODE1->M1CR_f.UIE = 1;
    M0P_TIM1_MODE1->MSCR_f.TS = 0x07; //CH0B
    M0P_TIM1_MODE1->FLTR_f.FLTB0 = 0;
    M0P_TIM1_MODE1->CR0_f.CIEA = 1;
    
    M0P_TIM1_MODE1->M1CR_f.CTEN = 1;
    
    EnableNvic(TIM1_IRQn,IrqLevel1,TRUE); //优先等级
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//低电平
void TIM0_IRQHandler(void)
{
    sdt_int16u rd_cnt;
    
    if(M0P_TIM0_MODE1->IFR_f.CA0F)
    {
        M0P_TIM0_MODE1->ICLR_f.CA0F = 0;
        rd_cnt = M0P_TIM0_MODE1->CCR0A;
        if(rd_cnt & EPF_ELTY_HIGH)//溢出
        {
            overflow = sdt_true;
            epf_elwdt_in = 0;
            epf_elwdt_ot = 0;
        }
        else
        {
            epf_elwdt_buff[epf_elwdt_in] = rd_cnt;
            epf_elwdt_in++;
            if(epf_elwdt_in > (sizeof(epf_elwdt_buff)/2 -1))
            {
                epf_elwdt_in = 0;
            }
        }
    }
    if(M0P_TIM0_MODE1->IFR_f.UIF)//溢出
    {
        M0P_TIM0_MODE1->ICLR_f.UIF = 0;
        overflow = sdt_true;
        epf_elwdt_in = 0;
        epf_elwdt_ot = 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void TIM1_IRQHandler(void)
{
    sdt_int16u rd_cnt;
    
    if(M0P_TIM1_MODE1->IFR_f.CA0F)
    {
        M0P_TIM1_MODE1->ICLR_f.CA0F = 0;
        rd_cnt = M0P_TIM1_MODE1->CCR0A;
        if(rd_cnt & EPF_ELTY_HIGH)//溢出
        {
            overflow = sdt_true;
            epf_elwdt_in = 0;
            epf_elwdt_ot = 0;
        }
        else
        {
            epf_elwdt_buff[epf_elwdt_in] = rd_cnt | EPF_ELTY_HIGH;
            epf_elwdt_in++;
            if(epf_elwdt_in > (sizeof(epf_elwdt_buff)/2 -1))
            {
                epf_elwdt_in = 0;
            }
        }
    }
    if(M0P_TIM1_MODE1->IFR_f.UIF) //43ms溢出一次
    {
        M0P_TIM1_MODE1->ICLR_f.UIF = 0;
        overflow = sdt_true;
        epf_elwdt_in = 0;
        epf_elwdt_ot = 0;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static enum
{
   FAT_IDLE   = 0x00,
   FAT_PREM   = 0x01,
   FAT_PREF   = 0x02,
   FAT_SYNC   = 0x03,
   FAT_ACPT   = 0x04,
   FAT_EOF    = 0x05,
}fat_sMah;
//------------------------------------------------------------------------------
static sdt_int16u first_sample_width;
static sdt_int16u ref_width,high_width,low_width;
static sdt_int32u acc_width;
static sdt_int8u acc_cnt;
static sdt_int8u fat_shift_reg,fat_shift_mask;
static sdt_int8u accpet_byte_buff[128];
static sdt_int8u epf_apb_in,epf_apb_ot;
static sdt_bool epf_accpet_block;
//------------------------------------------------------------------------------
void bsp_epf_accpet_task(void)
{
    sdt_int8u rd_elwdt_in,rd_elwdt_ot;
    sdt_int16u rd_width;
    
    do
    {
        rd_width = 0;
        
        __disable_interrupt();  
        if(overflow)
        {
            overflow = sdt_false;
            fat_sMah = FAT_IDLE;
        }
        if(epf_elwdt_in != epf_elwdt_ot)
        {
            rd_width = epf_elwdt_buff[epf_elwdt_ot];
            epf_elwdt_ot++;
            if(epf_elwdt_ot > (sizeof(epf_elwdt_buff)/2 -1))
            {
                epf_elwdt_ot = 0;
            }
        }
        __enable_interrupt();
        
        switch(fat_sMah)
        {
            case FAT_IDLE:
            {
                if(rd_width)//有电平宽度数据
                {
                    if(rd_width & EPF_ELTY_HIGH)
                    {
                    }
                    else
                    {//第一个低电平
                        fat_sMah = FAT_PREM;//进入前导处理
                    }
                }
                break;
            }
            case FAT_PREM:
            {
                if(rd_width)//有电平宽度数据
                {
                    if(rd_width & EPF_ELTY_HIGH)
                    {
                    }
                    else
                    {//第二个低电平
                        fat_sMah = FAT_PREF;//REF计算
                        first_sample_width = rd_width; //首个采用电平
                        acc_width = first_sample_width;
                        acc_cnt = 1;
                    }
                }
                break;
            }
            case FAT_PREF:
            {
                sdt_int16u diff;
                
                if(rd_width)//有电平宽度数据
                {
                    rd_width &= ~EPF_ELTY_HIGH; //去除电平属性
                    if(rd_width > first_sample_width)
                    {
                        diff = rd_width - first_sample_width;
                    }
                    else
                    {
                        diff = first_sample_width - rd_width;
                    }
                    if(diff < (first_sample_width/10)) //10%误差接受范围
                    {
                        acc_width += rd_width;
                        acc_cnt++;
                        if(8 == acc_cnt)
                        {
                            ref_width = acc_width/8;
                            if(ref_width > 4000)
                            {
                                fat_sMah = FAT_IDLE;//脉冲过宽
                            }
                            else
                            {
                                fat_sMah = FAT_SYNC;
                            }
                            
                        }
                    }
                    else
                    {
                        fat_sMah = FAT_IDLE;
                    }
                }
                break;
            }
            case FAT_SYNC:
            {
                sdt_int16u calc_width;
                
                if(rd_width & EPF_ELTY_HIGH)//高电平
                {
                    rd_width &= ~EPF_ELTY_HIGH; //去除电平属性
                    calc_width = ref_width * 7;
                    if(rd_width > calc_width)
                    {
                        calc_width = rd_width - calc_width;
                    }
                    else
                    {
                        calc_width = calc_width - rd_width;
                    }
                    if(calc_width < ref_width)
                    {
                        fat_sMah = FAT_ACPT;//数据电平
                        acc_cnt = 0;
                        fat_shift_mask = 0x80;
                        fat_shift_reg = 0;
                    }
                }
                break;
            }
            case FAT_ACPT:
            {
                sdt_int16u calc_width;
                
                if(rd_width)
                {
                    if(rd_width & EPF_ELTY_HIGH)
                    {
                        high_width = (rd_width & (~EPF_ELTY_HIGH));
                    }
                    else
                    {
                        calc_width = ref_width * 7;
                        if(rd_width > calc_width)
                        {
                            calc_width = rd_width - calc_width;
                        }
                        else
                        {
                            calc_width = calc_width - rd_width;
                        }
                        if(calc_width < ref_width)
                        {//EOF
                            fat_sMah = FAT_EOF;
                        }
                        else
                        {
                            low_width = rd_width;
                        }
                    }
                    acc_cnt++;
                    if(acc_cnt > 1)
                    {//输出1bit的数据
                        if(high_width > low_width)//bit = 0
                        {
                        }
                        else
                        {
                            fat_shift_reg |= fat_shift_mask;
                        }
                        fat_shift_mask = fat_shift_mask >> 1;
                        if(0 == fat_shift_mask) //完成8bit数据
                        {
                            accpet_byte_buff[epf_apb_in] = fat_shift_reg;
                            epf_apb_in++;
                            if(epf_apb_in > sizeof(accpet_byte_buff) - 1)
                            {
                                epf_apb_in = 0;
                            }
                            fat_shift_mask = 0x80;
                            fat_shift_reg = 0;
                        }
                        acc_cnt = 0;
                    }
                }
                break;
            }
            case FAT_EOF:
            {
                epf_accpet_block = sdt_true;
                fat_sMah = FAT_IDLE;
                break;
            }
            default:
            {
                fat_sMah = FAT_IDLE;
            }
        }
    }while(0 != rd_width);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_bool bsp_pull_accpet_one_byte(sdt_bool* pOut_msg,sdt_int8s* pOut_byte)
{
    sdt_bool g_byte;
    
    if(epf_accpet_block)
    {
        epf_accpet_block = sdt_false;
        *pOut_msg = sdt_true;
    }
    else
    {
        *pOut_msg = sdt_false;
    }
    g_byte = sdt_false;
    if(epf_apb_ot != epf_apb_in)
    {
        *pOut_byte = accpet_byte_buff[epf_apb_ot];
        epf_apb_ot++;
        if(epf_apb_ot > sizeof(accpet_byte_buff) - 1)
        {
            epf_apb_ot = 0;
        }
        g_byte = sdt_true;
    }  


    return(g_byte);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

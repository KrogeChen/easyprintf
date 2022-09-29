//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef snail_data_types
    #include ".\snail_data_types.h"
#endif
//------------------------------------------------------------------------------
#include ".\mde_mbus_link.h"
#include "hc32l13x.h"
#include "sysctrl.h"
#include "intrinsics.h"
//------------------------------------------------------------------------------
//LTX0  PB12
//LRX0  PB11
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//input floating
//#define macro_trs_cfg    //PG_DDR |= (0x04); PG_CR1 &= (~0x04); PG_CR2 &= (~0x04); PG_ODR &= (~0x04)
//#define macro_trs_receive       M0P_GPIO->PADIR_f.PA08 = 1;
//#define macro_trs_transmit      M0P_GPIO->PADIR_f.PA08 = 0;M0P_GPIO->PAOUT_f.PA08 = 1;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define BGK_S0     0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
void bsp_phy0_cfg(void)
{
    M0P_SYSCTRL->PERI_CLKEN_f.LPUART0 = 1;
    M0P_SYSCTRL->PERI_CLKEN_f.GPIO = 1;
//------------------------------------------------------------------------------
    M0P_GPIO->PB12_SEL = 3;   
    M0P_GPIO->PB11_SEL = 3;

    M0P_GPIO->PBDIR_f.PB12 = 0;  //output
    M0P_GPIO->PBOUT_f.PB12 = 1;
    M0P_GPIO->PBDIR_f.PB11 = 1;  //input
    M0P_GPIO->PBPU_f.PB11 = 1;
    
//------------------------------------------------------------------------------
    M0P_LPUART0->SCON_f.SM = 01;
    M0P_LPUART0->SCON_f.DMARXEN = 0;
    M0P_LPUART0->SCON_f.DMATXEN = 0;
    M0P_LPUART0->SCON_f.REN = 1;
    M0P_LPUART0->SCON_f.FEIE = 1; //帧错误中断
    M0P_LPUART0->SCON_f.RCIE = 1; 
    M0P_LPUART0->SCNT = 313;      //48M -- 9600
    EnableNvic(LPUART0_IRQn,IrqLevel3,TRUE);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_bool line_busy = sdt_false;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_bool tx_complete;
#define rx_buff_max   160
volatile static sdt_int8u tx_buffer[256];
static sdt_int8u tx_idx_str = 0,tx_idx_end = 0;
volatile static sdt_int8u rx_buffer[rx_buff_max];
static sdt_int8u rx_idx_str = 0,rx_idx_end = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void LPUART0_IRQHandler(void)
{
    if(0 != M0P_LPUART0->ISR_f.FE)
    {
        M0P_LPUART0->ICR_f.FECF = 0;
        M0P_LPUART0->ICR_f.RCCF = 0;
    }
    else
    {
        if(0 != M0P_LPUART0->SCON_f.TXEIE)
        {
            if(0 != M0P_LPUART0->ISR_f.TXE)
            {
                M0P_LPUART0->ICR_f.TCCF = 0;
                if(tx_idx_str != tx_idx_end)
                {
                    M0P_LPUART0->SBUF = tx_buffer[tx_idx_str];
                    tx_idx_str ++;
                }
                else
                {
                    M0P_LPUART0->SCON_f.TXEIE = 0;
                }
            }
        }
        if(0 != M0P_LPUART0->ISR_f.RC)
        {
            if(rx_idx_end > (rx_buff_max-1))
            {
                rx_idx_end = 0;
            }
            rx_buffer[rx_idx_end] = M0P_LPUART0->SBUF;
            rx_idx_end ++;
            if(rx_idx_end > (rx_buff_max-1))
            {
                rx_idx_end = 0;
            }
            M0P_LPUART0->ICR_f.RCCF = 0;
        }        
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_look_for_byte_rx_phy0(void)
{
    sdt_int8u now_rx_byte;
    sdt_int8u rd_rx_idx_end;
    
    __disable_interrupt();
    rd_rx_idx_end = rx_idx_end;
    __enable_interrupt();
    while(rd_rx_idx_end != rx_idx_str)  //一次性读取缓冲区数据
    {
        now_rx_byte = rx_buffer[rx_idx_str];
        rx_idx_str ++;
        if(rx_idx_str > (rx_buff_max-1))
        {
            rx_idx_str = 0;
        }
        push_mbus_one_receive_byte(BGK_S0,now_rx_byte);  //压入一个字节的数据到link
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_bool start_cpt;
//------------------------------------------------------------------------------
sdt_int16u bsp_transfet_bytes_tx_phy0(sdt_int8u* in_pByte,sdt_int16u in_expect_bytes)
{
    while(0 != in_expect_bytes)
    {
        __disable_interrupt(); 
        tx_buffer[tx_idx_end] = *in_pByte;
        tx_idx_end ++;
        __enable_interrupt();
        in_pByte ++;
        in_expect_bytes --;
        if(tx_idx_str == tx_idx_end)
        {
            break;
        }
    }
    M0P_LPUART0->SCON_f.TXEIE = 1;  //open tx interrupt
    return(in_expect_bytes);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_bool bsp_pull_complete_tx_phy0(void)
{
    if(0 != M0P_LPUART0->ISR_f.TXE)
    {
        if(0 != M0P_LPUART0->ISR_f.TC)
        {
            return(sdt_true);
        }        
    }
    return(sdt_false);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_phy_baudrate_calibrate_phy0(void)
{
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_entry_phy_rx_phy0(void)
{
//    USART1_CR2_bit.REN = 1;   //开启接收器
//    macro_trs_receive;
//    M0P_UART0->ICR_f.RCCF = 0;
//    M0P_UART0->SCON_f.REN = 1;  
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//对等总线，由发送报文时选择发送方式，此处采用空函数
//------------------------------------------------------------------------------
void bsp_entry_phy_tx_phy0(void)
{
//    M0P_UART0->SCON_f.REN = 0;  //关闭接收器
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

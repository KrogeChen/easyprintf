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
//UART1
//TXD PA04-17,RXD PA03-18
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_rmcd_char_cfg(void)
{
    M0P_SYSCTRL->PERI_CLKEN_f.UART1 = 1;
    M0P_SYSCTRL->PERI_CLKEN_f.GPIO = 1;
//------------------------------------------------------------------------------
    M0P_GPIO->PA04_SEL = 2;
    M0P_GPIO->PA03_SEL = 1;

    M0P_GPIO->PADIR_f.PA04 = 0;  //output
    M0P_GPIO->PAOUT_f.PA04 = 1;
    M0P_GPIO->PADIR_f.PA03 = 1;  //input
    M0P_GPIO->PAPU_f.PA03 = 1;
    
//------------------------------------------------------------------------------
    M0P_UART1->SCON_f.SM = 01;
    M0P_UART1->SCON_f.DMARXEN = 0;
    M0P_UART1->SCON_f.DMATXEN = 0;
    M0P_UART1->SCON_f.REN = 0;
    M0P_UART1->SCON_f.FEIE = 1; //帧错误中断
    M0P_UART1->SCON_f.RCIE = 1; 
    M0P_UART1->SCNT = 26;      //48M -- 115200
    EnableNvic(UART1_IRQn,IrqLevel3,TRUE);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define rx_buff_max   32
volatile static sdt_int8u tx_buffer[256];
static sdt_int8u tx_idx_str = 0,tx_idx_end = 0;
volatile static sdt_int8u rx_buffer[rx_buff_max];
static sdt_int8u rx_idx_str = 0,rx_idx_end = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_bool tx_complete;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//中断服务函数
//-----------------------------------------------------------------------------
void UART1_IRQHandler(void)
{
    if(0 != M0P_UART1->ISR_f.FE)
    {
        M0P_UART1->ICR_f.FECF = 0;
        M0P_UART1->ICR_f.RCCF = 0;
    }
    else
    {
        if(0 != M0P_UART1->SCON_f.TXEIE)
        {
            if(0 != M0P_UART1->ISR_f.TXE)
            {
                M0P_UART1->ICR_f.TCCF = 0;
                if(tx_idx_str != tx_idx_end)
                {
                    M0P_UART1->SBUF = tx_buffer[tx_idx_str];
                    tx_idx_str ++;
                }
                else
                {
                    M0P_UART1->SCON_f.TXEIE = 0;
                }
                
            }
        }
        if(0 != M0P_UART1->ISR_f.RC)
        {
            if(rx_idx_end > (rx_buff_max-1))
            {
                rx_idx_end = 0;
            }
            rx_buffer[rx_idx_end] = M0P_UART1->SBUF;
            rx_idx_end ++;
            if(rx_idx_end > (rx_buff_max-1))
            {
                rx_idx_end = 0;
            }
            M0P_UART1->ICR_f.RCCF = 0;
        }        
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_look_for_byte_rx_phy1(void)
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
       // mde_push_log_one_receive_byte(now_rx_byte);     //压入一个字节的数据到link
        //mde_push_modbus_slave_receive_byte(0,now_rx_byte); //modbus端口
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//static sdt_bool start_cpt;
//------------------------------------------------------------------------------
sdt_int16u bsp_transfet_bytes_tx_rmcd(sdt_int8u* in_pByte,sdt_int16u in_expect_bytes)
{

    while(0 != in_expect_bytes)
    {
        __disable_interrupt(); 
        tx_buffer[tx_idx_end] = *in_pByte;
        tx_idx_end ++;
        __enable_interrupt();
        in_pByte ++;
        in_expect_bytes --;
    }
    M0P_UART1->SCON_f.TXEIE = 1;  //open tx interrupt
    return(in_expect_bytes);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

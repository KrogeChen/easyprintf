//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:mbus链路代码，cj/t-188协议
//version:10
//date: 10/12/2020
//------------------------------------------------------------------------------
//modify:压缩内存消耗,采用统一的收、发数据缓冲区
//date:  09/18/2022
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "..\..\pbc\pbc_tick_small\pbc_tick_small.h"
//------------------------------------------------------------------------------
#include ".\bsp_mbus_link.h"
#include ".\mde_mbus_link.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SYB_PREAMBLE    0xfe
#define SYB_SFD         0x68
//------------------------------------------------------------------------------
#define TIMEOUTV        1000
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum
{
    mbslk_rs_idle            = 0x00,
    mbslk_rs_rx_sfd          = 0x01,
    mbslk_rs_rx_len          = 0x02,
    mbslk_rs_rx_data         = 0x03,
    mbslk_rs_rx_complete     = 0x04,
    
    mbslk_rs_tx_start        = 0x10,
    mbslk_rs_tx_doing        = 0x11,
    mbslk_rs_tx_complete     = 0x12,
}mbslk_run_status_def;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define mbslk_bits_message_rx     0x0001 //新报文生产
#define mbslk_bits_rx_byte        0x0002 //收一字节数据
#define mbslk_bits_tx_conflict    0x0004 //发送冲突检测模式
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
typedef struct
{
    sdt_int8u             rx_byte;
    sdt_int16u            mbslk_bits;  //状态字
    mbus_link_buff_def    mbus_link_buff;
    mbk_trans_mon_def     transmit_monitor;
    mbslk_run_status_def  mbslk_run_status;
    
    sdt_int16u            trx_len;
    sdt_int16u            trx_index;
    
    timerClock_def        timer_rx_timeout;    
    timerClock_def        timer_tx_timeout;  
    timerClock_def        timer_brate_cal;//波特率校准间隔
    
    void (*pull_look_for_byte_rx)(void);
    sdt_int16u (*transfet_bytes_to_phy_tx)(sdt_int8u* in_pByte,sdt_int16u in_expect_bytes);
    sdt_bool (*pull_complete_tx_data)(void);
    void(*phy_baudrate_calibrate)(void);
    void (*entry_phy_rx)(void);
    void (*entry_phy_tx)(void);
}mbslk_oper_def;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//soild match
#include ".\mde_mbus_link_match.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_int8u make_checksum(sdt_int8u* in_pBuff,sdt_int16u in_len)
{
    sdt_int8u checksum = 0;
    sdt_int16u i;
    
    for(i = 0;i < in_len;i++)
    {
        checksum += in_pBuff[i];
    }
    return(checksum);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void mbus_link_status_jump(mbslk_oper_def* mix_pMbslk_oper)
{
    mbslk_run_status_def rd_mbslk_run_status;
    
    pbc_timerClockRun_task(&mix_pMbslk_oper->timer_brate_cal);
    
    pbc_timerMillRun_task(&mix_pMbslk_oper->timer_rx_timeout);
    pbc_timerMillRun_task(&mix_pMbslk_oper->timer_tx_timeout);
    
    if(pbc_pull_timerIsOnceTriggered(&mix_pMbslk_oper->timer_rx_timeout))
    {
        mix_pMbslk_oper->mbslk_run_status = mbslk_rs_idle;
    }    
    if(pbc_pull_timerIsOnceTriggered(&mix_pMbslk_oper->timer_tx_timeout))
    {
        mix_pMbslk_oper->mbslk_run_status = mbslk_rs_idle;
        mix_pMbslk_oper->transmit_monitor = mbk_trans_mon_error;
    }
    
    do
    {
        rd_mbslk_run_status = mix_pMbslk_oper->mbslk_run_status;
        switch(rd_mbslk_run_status)
        {
            case mbslk_rs_idle:
            {
                pbc_reload_timerClock(&mix_pMbslk_oper->timer_brate_cal,120); //120s校准一次
                mix_pMbslk_oper->phy_baudrate_calibrate();
                mix_pMbslk_oper->entry_phy_rx();
                mix_pMbslk_oper->mbslk_run_status = mbslk_rs_rx_sfd;
                break;
            }
            case mbslk_rs_rx_sfd:
            {
                if(pbc_pull_timerIsCompleted(&mix_pMbslk_oper->timer_brate_cal))
                {
                    pbc_reload_timerClock(&mix_pMbslk_oper->timer_brate_cal,120);
                    mix_pMbslk_oper->phy_baudrate_calibrate();
                }
                if(mbslk_bits_rx_byte & mix_pMbslk_oper->mbslk_bits)
                {
                    mix_pMbslk_oper->mbslk_bits &= ~mbslk_bits_rx_byte;
                    if(SYB_SFD == mix_pMbslk_oper->rx_byte)
                    {
                        mix_pMbslk_oper->mbus_link_buff.raw_payload[RX_IDX_OFFSET] = mix_pMbslk_oper->rx_byte;
                        mix_pMbslk_oper->trx_index = RX_IDX_OFFSET + 1;
                        mix_pMbslk_oper->mbslk_run_status = mbslk_rs_rx_len;
                        
                        pbc_reload_timerClock(&mix_pMbslk_oper->timer_rx_timeout,TIMEOUTV);
                    }  
                }
                break;
            }
            case mbslk_rs_rx_len:
            {
                if(mbslk_bits_rx_byte & mix_pMbslk_oper->mbslk_bits)
                {
                    mix_pMbslk_oper->mbslk_bits &= ~mbslk_bits_rx_byte;
                    
                    mix_pMbslk_oper->mbus_link_buff.raw_payload[mix_pMbslk_oper->trx_index] = mix_pMbslk_oper->rx_byte;
                    mix_pMbslk_oper->trx_index ++;
                    if(mix_pMbslk_oper->trx_index > (10 + RX_IDX_OFFSET))
                    {
                        mix_pMbslk_oper->trx_len = (sdt_int16u)mix_pMbslk_oper->rx_byte + 13;
                        if(mix_pMbslk_oper->trx_len > MAX_RXD_LEN)  //长度溢出
                        {
                            mix_pMbslk_oper->mbslk_run_status = mbslk_rs_idle;
                            pbc_stop_timerIsOnceTriggered(&mix_pMbslk_oper->timer_rx_timeout);
                        }
                        else
                        {
                            mix_pMbslk_oper->mbslk_run_status = mbslk_rs_rx_data;
                        }
                        
                    }
                }
                break;
            }
            case mbslk_rs_rx_data:
            {
                if(mbslk_bits_rx_byte & mix_pMbslk_oper->mbslk_bits)
                {
                    mix_pMbslk_oper->mbslk_bits &= ~mbslk_bits_rx_byte;
                    
                    mix_pMbslk_oper->mbus_link_buff.raw_payload[mix_pMbslk_oper->trx_index] = mix_pMbslk_oper->rx_byte;
                    if((mix_pMbslk_oper->trx_index + 1 - RX_IDX_OFFSET) >= mix_pMbslk_oper->trx_len)
                    {
                        sdt_int8u calculate_cs;
                        
                        calculate_cs = make_checksum(&mix_pMbslk_oper->mbus_link_buff.raw_payload[RX_IDX_OFFSET],(mix_pMbslk_oper->trx_len - 2));
                        if((0x68 == mix_pMbslk_oper->mbus_link_buff.raw_payload[RX_IDX_OFFSET]) && (0x16 == mix_pMbslk_oper->mbus_link_buff.raw_payload[mix_pMbslk_oper->trx_len + RX_IDX_OFFSET - 1]) &&\
                           (calculate_cs == mix_pMbslk_oper->mbus_link_buff.raw_payload[mix_pMbslk_oper->trx_len + RX_IDX_OFFSET - 2]))
                        {
                            mix_pMbslk_oper->mbslk_bits |= mbslk_bits_message_rx;  //产生一个报文
                            mix_pMbslk_oper->mbslk_run_status = mbslk_rs_rx_complete;
                        }
                        else
                        {
                            mix_pMbslk_oper->mbslk_run_status = mbslk_rs_idle;
                        }
                        pbc_stop_timerIsOnceTriggered(&mix_pMbslk_oper->timer_rx_timeout);
                    }
                    mix_pMbslk_oper->trx_index ++;
                }
                break;
            }
            case mbslk_rs_rx_complete:
            {
                if(mix_pMbslk_oper->mbslk_bits & mbslk_bits_message_rx)//等待数据被读取
                {
                }
                else
                {
                    mix_pMbslk_oper->mbslk_run_status = mbslk_rs_idle;//数据被读取，转入空闲
                }
                break;
            }
            case mbslk_rs_tx_start:
            {
                
                sdt_int16u Data_Length;
                sdt_int16u PandSFD_Index = 0;
    
                while(PandSFD_Index < (PREABBLE_LEN+1))//encode
                {
                    if(PandSFD_Index == PREABBLE_LEN)
                    {
                        mix_pMbslk_oper->mbus_link_buff.raw_payload[PandSFD_Index] = SYB_SFD;
                    }
                    else
                    {
                        mix_pMbslk_oper->mbus_link_buff.raw_payload[PandSFD_Index] = SYB_PREAMBLE;
                    }
                    PandSFD_Index++;
                }

                Data_Length = mix_pMbslk_oper->mbus_link_buff.payload_len + 12;
                
                mix_pMbslk_oper->mbus_link_buff.raw_payload[(PandSFD_Index+10)+mix_pMbslk_oper->mbus_link_buff.payload_len] = make_checksum(&mix_pMbslk_oper->mbus_link_buff.raw_payload[PREABBLE_LEN],(mix_pMbslk_oper->mbus_link_buff.payload_len+11));
                mix_pMbslk_oper->mbus_link_buff.raw_payload[(PandSFD_Index+10)+mix_pMbslk_oper->mbus_link_buff.payload_len + 1] = 0x16;

                mix_pMbslk_oper->trx_len = PandSFD_Index+Data_Length;  //需要传送的字节数
                mix_pMbslk_oper->trx_index = 0;
                pbc_reload_timerClock(&mix_pMbslk_oper->timer_tx_timeout,TIMEOUTV);

                mix_pMbslk_oper->entry_phy_tx();               //进入tx模式
                mix_pMbslk_oper->mbslk_bits &= ~mbslk_bits_rx_byte;
                
                sdt_int16u remain_bytes;
                
                remain_bytes = mix_pMbslk_oper->transfet_bytes_to_phy_tx(&mix_pMbslk_oper->mbus_link_buff.raw_payload[mix_pMbslk_oper->trx_index],(mix_pMbslk_oper->trx_len - mix_pMbslk_oper->trx_index));
                mix_pMbslk_oper->trx_index = mix_pMbslk_oper->trx_len - remain_bytes;
                mix_pMbslk_oper->mbslk_run_status = mbslk_rs_tx_doing;
                break;
            }
            case mbslk_rs_tx_doing:
            {
                sdt_int16u remain_bytes;
                
                if(mix_pMbslk_oper->trx_index == mix_pMbslk_oper->trx_len) //数据转移完毕,检测PHY发送完成
                {
                    if(mix_pMbslk_oper->pull_complete_tx_data())
                    {

                        mix_pMbslk_oper->mbslk_run_status = mbslk_rs_tx_complete;                    
                    }

                }
                else
                {
                    remain_bytes = mix_pMbslk_oper->transfet_bytes_to_phy_tx(&mix_pMbslk_oper->mbus_link_buff.raw_payload[mix_pMbslk_oper->trx_index],(mix_pMbslk_oper->trx_len - mix_pMbslk_oper->trx_index));
                    mix_pMbslk_oper->trx_index = mix_pMbslk_oper->trx_len - remain_bytes;               
                }
                break;
            }
            
            case mbslk_rs_tx_complete:
            {
                mix_pMbslk_oper->transmit_monitor = mbk_trans_mon_complete;
                mix_pMbslk_oper->mbslk_run_status = mbslk_rs_idle;
                pbc_stop_timerIsOnceTriggered(&mix_pMbslk_oper->timer_tx_timeout);
                break;
            }
            default:
            {
                rd_mbslk_run_status = mbslk_rs_idle;
                break;
            }
        }
        
    }while(rd_mbslk_run_status != mix_pMbslk_oper->mbslk_run_status);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void alone_mbus_link_task(mbslk_oper_def* mix_pMbslk_oper)
{
    mix_pMbslk_oper->pull_look_for_byte_rx();
    mbus_link_status_jump(mix_pMbslk_oper);
}
//++++++++++++++++++++++++++++++++++++interface+++++++++++++++++++++++++++++++++
static sdt_bool cfged = sdt_false;
//------------------------------------------------------------------------------
void mde_mbus_link_task(void)
{
    if(cfged)
    {
        sdt_int8u i;
        for(i = 0;i < max_solid;i ++)
        {
            alone_mbus_link_task(&mbslk_oper_solid[i]);
        }
    }
    else
    {
        cfged = sdt_true;
        mbslk_solid_cfg();
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_bool mde_pull_mbus_recevie_one_message(sdt_int8u in_solid_number)
{
    if(in_solid_number < max_solid)
    {
        if(mbslk_bits_message_rx & mbslk_oper_solid[in_solid_number].mbslk_bits)
        {
            mbslk_oper_solid[in_solid_number].mbslk_bits &= ~mbslk_bits_message_rx;
            mbus_link_status_jump(&mbslk_oper_solid[in_solid_number]);
            
            return(sdt_true);
        }
    }
    else
    {
        while(1);
    }
    return(sdt_false);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
mbus_link_buff_def* mde_pull_mbus_message_pBuff(sdt_int8u in_solid_number)
{
    if(in_solid_number < max_solid)
    {
        return (&mbslk_oper_solid[in_solid_number].mbus_link_buff);
    }
    else
    {
        while(1);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
void mde_push_mbus_active_one_message_transmit(sdt_int8u in_solid_number)
{
    if(in_solid_number < max_solid)
    {
        mbslk_oper_solid[in_solid_number].transmit_monitor = mbk_trans_mon_doing;
        mbslk_oper_solid[in_solid_number].mbslk_run_status = mbslk_rs_tx_start;
    }
    else
    {
        while(1);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
mbk_trans_mon_def mde_pull_mbus_transmit_monitor(sdt_int8u in_solid_number)
{
    if(in_solid_number < max_solid)
    {
        return(mbslk_oper_solid[in_solid_number].transmit_monitor);
    }
    else
    {
        while(1);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
void push_mbus_one_receive_byte(sdt_int8u in_solid_number,sdt_int8u in_rx_byte)
{
    if(cfged)
    {
    }
    else
    {
        cfged = sdt_true;
        mbslk_solid_cfg();
    }
    if(in_solid_number < max_solid)
    {
        mbslk_oper_solid[in_solid_number].mbslk_bits |= mbslk_bits_rx_byte;
        mbslk_oper_solid[in_solid_number].rx_byte = in_rx_byte;
        mbus_link_status_jump(&mbslk_oper_solid[in_solid_number]);//数据进入后,马上处理状态
    }
    else
    {
        while(1);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
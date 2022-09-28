#ifndef mde_mbus_link_H
#define mde_mbus_link_H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef snail_data_types
    #include ".\depend\snail_data_types.h"
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define preamble_len         3
#define max_payload_len      136
#define TX_PAYLOD_LEN        8
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
typedef union
{
    struct
    {
        sdt_int8u      meter_type;                 //仪表类型
        sdt_int8u      meter_addr[7];              //仪表地址
        sdt_int8u      control_code;               //控制码
        sdt_int8u      payload_len;                //数据长度
        sdt_int8u      payload[max_payload_len];   //数据内容      
    };
    sdt_int8u raw_payload[max_payload_len+10];
    
}mbus_link_buff_def;;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: mbus链路任务
//功能: 负载链路数据的收发
//入口: 无
//
//出口: 无
//------------------------------------------------------------------------------
void mde_mbus_link_task(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取接收报文信息
//功能: 
//in:   in_solid_number -- 实例编号
//out:  sdt_true  -- 接收到一个完整的报文
//      sdt_false -- 没有接收到完整的报文
//------------------------------------------------------------------------------
sdt_bool mde_pull_mbus_recevie_one_message(sdt_int8u in_solid_number);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:获取链路缓冲区指针
//in:   in_solid_number -- 实例编号
//out:  缓冲区指针
//------------------------------------------------------------------------------
mbus_link_buff_def* mde_pull_mbus_message_pBuff(sdt_int8u in_solid_number);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:激活发送一个报文数据,先填充缓冲区数据，再激活发送
//in:   in_solid_number -- 实例编号
//      
//      
//out:  none
//------------------------------------------------------------------------------
void mde_push_mbus_active_one_message_transmit(sdt_int8u in_solid_number);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum
{
    mbk_trans_mon_none      = 0x00,
    mbk_trans_mon_doing     = 0x01,
    mbk_trans_mon_conflict  = 0x02,
    mbk_trans_mon_complete  = 0x03,
    mbk_trans_mon_error     = 0x04,
}mbk_trans_mon_def;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//name:数据发送状态监控
//in:  in_solid_number -- 实例编号
//out: mbk_trans_mon_none     -- 无意义
//     mbk_trans_mon_doing    -- 发送进行中
//     mbk_trans_mon_conflict -- PHY有冲突报文
//     mbk_trans_mon_complete -- 发送完成
//     mbk_trans_mon_error    -- 未知错误
//------------------------------------------------------------------------------
mbk_trans_mon_def mde_pull_mbus_transmit_monitor(sdt_int8u in_solid_number);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:推入一字节的接收数据到链路,一般由bsp层调用
//in:   in_solid_number -- 实例编号
//      in_rx_byte      -- 接收的数据
//out:  none
//------------------------------------------------------------------------------
void push_mbus_one_receive_byte(sdt_int8u in_solid_number,sdt_int8u in_rx_byte);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
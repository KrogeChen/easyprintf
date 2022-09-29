#ifndef bsp_mbus_link_H
#define bsp_mbus_link_H
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef snail_data_types
    #include ".\snail_data_types.h"
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: phy配置
//功能: 处理系统时钟节拍，调度任务
//入口: 无
//
//出口: 
//-------------------------------------------------------------------------------------------------
void bsp_phy0_cfg(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 寻找接收数据
//功能: 从phy寻找一个接收的数据
//入口: 无
//
//出口: 无
//-------------------------------------------------------------------------------------------------
void bsp_look_for_byte_rx_phy0(void);

sdt_int16u bsp_transfet_bytes_tx_phy0(sdt_int8u* in_pByte,sdt_int16u in_expect_bytes);
sdt_bool bsp_pull_complete_tx_phy0(void);
void bsp_phy_baudrate_calibrate_phy0(void);    
void bsp_entry_phy_rx_phy0(void);
void bsp_entry_phy_tx_phy0(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif 
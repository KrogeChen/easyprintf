#ifndef MDE_STORAGE_SMALL_H
#define MDE_STORAGE_SMALL_H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef snail_data_types
    #include ".\depend\snail_data_types.h"
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//VER_01 2022-08-09
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//最小版存储模块,带版本信息,方便存储区的升级管理
//适用于8位机的存储模块
//采用map+information的方式,inf size = INFOR.data(n byte)
//struct:
//|-----------------------------------------------------------------------|
//|                    MAP(FIX-16B)                       |  INFOR(VAR)   |
//|map tag|  cnt |inf ver|inf size|reserve|  id  |checksum| data |checksum|
//|1 byte |2 byte|1 byte | 1 byte |2 byte |8 byte| 1 byte |n byte| 1 byte |
//|-----------------------------------------------------------------------|
//map tag
//bit1 bit0 map version
//bit2 bit3 update tag
//bit4-bit7 reserve
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define UPTAG_NONE               0x00
#define UPTAG_NEEDUP             0x01 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 读取存储信息序列
//功能: 获取存储器的
//入口: 序列内存首指针,inf版本信息,最大接收字节数
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_read_storage_inf(sdt_int8u *pOut_inf,sdt_int8u *pOut_infVer,sdt_int8u in_maxBytes);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 写入信息序列数据
//功能: 
//入口: 信息数据指针，信息字节数，版本信息
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_write_storage_inf(sdt_int8u *pIn_inf,sdt_int8u in_bytes,sdt_int8u in_infVer);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取存储体的操作次数
//功能: 
//入口: 操作次数指针
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_pull_storage_life(sdt_int16u *pOut_cycles);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取信息段容量
//功能: 指示物理存储器可存储的最大容量和当前信息段大小及版本信息
//入口: 容量字节数指针,信息段字节数指针
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_pull_stoCapacity_inf(sdt_int8u *pOut_maxBytes,sdt_int8u *pOut_infBytes,sdt_int8u *pOut_infVer);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取存储id序列
//功能: 
//入口: 返回id数量，id首指针
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_pull_stoDeviceId(sdt_int8u *pOut_id,sdt_int8u in_idBytes);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取升级标识
//功能: 
//入口: 数据指针
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_pull_stoUpdateTag(sdt_int8u *pOut_tag);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 写入id数据
//功能: 
//入口: 数据指针，数据字节数
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_push_stoDeviceId(sdt_int8u *pIn_id,sdt_int8u in_idBytes);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 写入升级标识
//功能: 
//入口: 升级tag,0x00正常状态,0x01升级状态
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_push_stoUpdateTag(sdt_int8u in_newTag);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
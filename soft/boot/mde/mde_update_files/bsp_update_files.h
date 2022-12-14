#ifndef BSP_UPDATE_FILES_H
#define BSP_UPDATE_FILES_H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef snail_data_types
    #include ".\depend\snail_data_types.h"
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//task++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//interface+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:读取4bytes的数据,32bits
//入口:地址偏移量,缓冲区指针,缓冲区需要>=4bytes
//------------------------------------------------------------------------------
void bsp_read_userApp_4bytes(sdt_int32u in_addrOffset,sdt_int8u *pOut_buff);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:读取32bytes的信息区数据
//入口:指针，缓冲区必须>=32bytes,否则溢出
//------------------------------------------------------------------------------
void bsp_read_information_userApp(sdt_int8u *pOut_buff);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:写入32bytes的信息区数据
//入口:指针，缓冲区必须>=32bytes,否则溢出
//------------------------------------------------------------------------------
void bsp_write_information_userApp(sdt_int8u *pIn_buff);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:写入128bytes的数据到用户代码区
//入口:指针，缓冲区必须>=128bytes,否则溢出
//------------------------------------------------------------------------------
void bsp_write_userApp_128bytes(sdt_int32u in_addrOffset,sdt_int8u *pIn_buff);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:获取APP区硬件标识
//入口:none
//出口:32bit的硬件标识
//------------------------------------------------------------------------------
sdt_int32u bsp_pull_hardware_flag(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:跳转到APP区运行
//入口:
//------------------------------------------------------------------------------
void bsp_jump_userApp(void);
//end+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
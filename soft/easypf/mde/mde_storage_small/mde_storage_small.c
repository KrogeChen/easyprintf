//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//#include "..\..\pbc\pbc_tick_small\pbc_tick_small.h"
#include "..\..\pbc\pbc_data_convert\pbc_data_convert.h"
#include ".\mde_storage_small.h"
#include ".\bsp_storage_small.h"
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
#define LOC_MAPTAG     0
#define LOC_CNT_0      1
#define LOC_CNT_1      2
#define LOC_INFVER     3
#define LOC_INFSIZE    4
#define LOC_RES_0      5
#define LOC_RES_1      6
#define LOC_ID_0       7
#define LOC_ID_1       8
#define LOC_ID_2       9
#define LOC_ID_3       10
#define LOC_ID_4       11
#define LOC_ID_5       12
#define LOC_ID_6       13
#define LOC_ID_7       14
#define LOC_ID_CS      15

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define BLOCK_0     0x00
#define BLOCK_1     0x01
#define BLOCK_UN    0xFE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//static sdt_bool cfged = sdt_false;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:从eeprom里面读取MAP区的数据
//------------------------------------------------------------------------------
static void sto_read_eepMonery_map(sdt_int8u in_block,sdt_int8u *pOut_map)
{
    sdt_int8u i;
    
    for(i = 0;i < 16;i++)
    {
        pOut_map[i] = bsp_read_eeMomery_byte(in_block,i);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:校验文计算
//功能:
//入口:数据指针,数据长度
//出口:校验和
//------------------------------------------------------------------------------
static sdt_int8u sto_small_checksum(sdt_int8u *pIn_data,sdt_int16u in_bytes)
{
    sdt_int8u checksum;
    
    checksum = 0xa5;
    while(in_bytes)
    {
        checksum += *pIn_data;
        pIn_data++;
        in_bytes--;
    }
    return(checksum);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:校验附加计算
//功能:附加一个字节的校验文,
//入口:本次附加数据,原校验和(首次采用0xa5).
//出口:新校验和
//------------------------------------------------------------------------------
static sdt_int8u sto_small_cs_append(sdt_int8u *pIn_data,sdt_int16u in_bytes,sdt_int8u in_cs)
{
    sdt_int8u checksum;
    
    checksum = in_cs;
    while(in_bytes)
    {
        checksum += *pIn_data;
        pIn_data++;
        in_bytes--;
    }
    return(checksum);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//信息段校验
//出口:sdt_ture 校验错误
//------------------------------------------------------------------------------
static sdt_bool sto_small_inf_err_check(sdt_int8u in_blockNum,sdt_int8u inf_bytes)
{
    sdt_int8u offset;
    sdt_int8u eem_bytes;
    sdt_int8u make_cs,cs;
    sdt_int8u rd_eem_data;
    
    offset = 0;
    eem_bytes = inf_bytes + 16;
    make_cs = 0xa5;
    
    while(eem_bytes)
    {
        rd_eem_data = bsp_read_eeMomery_byte(in_blockNum,offset);
        make_cs = sto_small_cs_append(&rd_eem_data,1,make_cs);
        offset++;
        eem_bytes--;
    }
    cs = bsp_read_eeMomery_byte(in_blockNum,offset);
    if(make_cs == cs)
    {
        return(sdt_false);
    }
    return(sdt_true);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//存储镜像错误检测
//入口:数据指针,数据长度
//出口:错误标识,sdt_true 发生校验错误
//------------------------------------------------------------------------------
static sdt_bool sto_small_mirror_err_check(sdt_int8u *pIn_mirror,sdt_int8u in_bytes)
{
    sdt_int8u cs,make_cs;
    
    cs = pIn_mirror[in_bytes - 1];
    make_cs = sto_small_checksum(pIn_mirror,(in_bytes - 1));
    
    if(cs == make_cs)
    {
        return(sdt_false);
    }
    return(sdt_true);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//获取map区写入次数
//入口:map数据指针
//出口:写入次数
//------------------------------------------------------------------------------
static sdt_int16u sto_small_get_write_cycles(sdt_int8u *pIn_map)
{
    return(pbc_arrayToInt16u_bigEndian(&pIn_map[LOC_CNT_0]));
}
//------------------------------------------------------------------------------
//写入次数赋值给map数组
//入口:map数据指针，写入次数
//出口:
//------------------------------------------------------------------------------
static void sto_small_give_write_cycles(sdt_int8u *pIn_map,sdt_int16u in_cycles)
{
    pbc_int16uToArray_bigEndian(in_cycles,&pIn_map[LOC_CNT_0]);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//获取升级标识
//入口:map数据指针
//出口:id位置索引
//------------------------------------------------------------------------------
static sdt_int8u sto_small_get_updateTag(sdt_int8u *pIn_map)
{
    sdt_int8u tag;
    
    tag = (pIn_map[LOC_MAPTAG] & 0x0c) >> 2; //分离tag信息
    return(tag);
}
//------------------------------------------------------------------------------
//获取升级标识
//入口:map数据指针
//出口:id位置索引
//------------------------------------------------------------------------------
static void sto_small_give_updateTag(sdt_int8u *pIn_map,sdt_int8u in_tag)
{
    pIn_map[LOC_MAPTAG] &= ~0x0c;
    pIn_map[LOC_MAPTAG] |= (in_tag<<2) & 0x0c;  //掩码处理
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////功能:错误检测
////入口:none
////出口:错误代码
////------------------------------------------------------------------------------
//#define stoErrBits_map0          0x01
//#define stoErrBits_inf0          0x02
//#define stoErrBits_map1          0x04
//#define stoErrBits_inf1          0x08
////------------------------------------------------------------------------------
//static sdt_int8u sto_small_err_check(void)
//{
//    sdt_int8u errBits;
//    
//    errBits = 0;
//}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:获取当前有效数据块序号
//入口:none
//出口:块序号,0xFE无有效数据块
//------------------------------------------------------------------------------
static sdt_int8u get_sto_small_nowBlock(void)
{
    sdt_int8u now_block = 0;
    sdt_int8u rdSto_map[16];
    sdt_int16u map0_cycles,map1_cycles;
    sdt_int8u map0_inf_size,map1_inf_size;
    
    sto_read_eepMonery_map(BLOCK_0,&rdSto_map[0]);              //读取map0的数据
    map0_cycles = sto_small_get_write_cycles(&rdSto_map[0]);    //计算map0的写入次数
    map0_inf_size = rdSto_map[LOC_INFSIZE];
    if(sto_small_mirror_err_check(&rdSto_map[0],16))
    {//map0 error
        sto_read_eepMonery_map(BLOCK_1,&rdSto_map[0]);  //读取map1的数据
        if(sto_small_mirror_err_check(&rdSto_map[0],sizeof(rdSto_map)))
        {//map1 error
            now_block = BLOCK_UN;  //0xFE
        }
        else
        { //block 1 map有效
            if(sto_small_inf_err_check(BLOCK_1,map0_inf_size))
            {//inf1 error
                now_block = BLOCK_UN;
            }
            else
            {
                now_block = BLOCK_1;
            }
        }
    }
    else
    {
        sto_read_eepMonery_map(BLOCK_1,&rdSto_map[0]);  //读取map1的数据
        map1_cycles = sto_small_get_write_cycles(&rdSto_map[0]); 
        map1_inf_size = rdSto_map[LOC_INFSIZE];        
        if(sto_small_mirror_err_check(&rdSto_map[0],16))
        {
            if(sto_small_inf_err_check(BLOCK_0,map0_inf_size))  //信息段校验
            {
                now_block = BLOCK_UN;
            }
            else
            {
                now_block = BLOCK_0;
            }
        }
        else
        {
            if(sto_small_inf_err_check(BLOCK_0,map0_inf_size))
            {
                if(sto_small_inf_err_check(BLOCK_1,map1_inf_size))
                {
                    now_block = BLOCK_UN;
                }
                else
                {
                    now_block = BLOCK_1;
                }
            }
            else
            {
                if(sto_small_inf_err_check(BLOCK_1,map1_inf_size))
                {
                    now_block = BLOCK_0;
                }
                else
                {
                    while((0xffff == map0_cycles) ||(0xffff == map1_cycles))  //避免0xffff和0x0000比较产生错误
                    {
                        map0_cycles++;
                        map1_cycles++;
                    }
                    if(map1_cycles > map0_cycles)
                    {
                        now_block = BLOCK_1;
                    }
                    else
                    {
                        now_block = BLOCK_0;
                    }                    
                }
            }
        }
    }
    return(now_block);
}
//++++++++++++++++++++++++++++++++interface+++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 读取存储信息序列
//功能: 获取存储器的
//入口: 序列内存首指针,inf版本信息,最大接收字节数
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_read_storage_inf(sdt_int8u *pOut_inf,sdt_int8u *pOut_infVer,sdt_int8u in_maxBytes)
{
    sdt_int8u block_st;
    sdt_int8u rdSto_map[16];
    sdt_int8u inf_bytes;
    sdt_int8u cs,make_cs;
    sdt_int8u offset,rd_eem_data;
    sdt_int8u i;
    
//    if(sdt_false == cfged)
//    {
//        cfged = sdt_true;
//    }
    while(1)
    {
        block_st = get_sto_small_nowBlock();
        if(BLOCK_UN == block_st)
        {
            return(0x01);
        }
        sto_read_eepMonery_map(block_st,&rdSto_map[0]);//获取map并校验
        if(sto_small_mirror_err_check(&rdSto_map[0],16))
        {
        }
        else
        {//获取inf并校验
            *pOut_infVer = rdSto_map[LOC_INFVER];
            inf_bytes = rdSto_map[LOC_INFSIZE];                //提取信息段大小
            make_cs = sto_small_checksum(&rdSto_map[0],16);    //map区的checksum
            offset = 16;
            i = 0;
            
            while(inf_bytes)
            {
                rd_eem_data = bsp_read_eeMomery_byte(block_st,offset);
                make_cs = sto_small_cs_append(&rd_eem_data,1,make_cs);

                offset++;
                inf_bytes--;   
                
                if(i < in_maxBytes)  //数据转移
                {
                    pOut_inf[i] = rd_eem_data;
                    i++;
                }
            }
            cs = bsp_read_eeMomery_byte(block_st,offset);
            if(cs == make_cs)
            {
                return(0x00);
            }
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取存储体的操作次数
//功能: 
//入口: 操作次数指针
//
//出口:错误代码,00未发送错误，0x01 错误
//------------------------------------------------------------------------------
sdt_int8u mde_pull_storage_life(sdt_int16u *pOut_cycles)
{
    sdt_int8u rdSto_map[16];
    sdt_int8u block_st;

//    if(sdt_false == cfged)
//    {
//        cfged = sdt_true;
//    }
    while(1)
    {
        block_st = get_sto_small_nowBlock();
        if(BLOCK_UN == block_st)
        {
            return(0x01);
        }
        sto_read_eepMonery_map(block_st,&rdSto_map[0]);//获取map并校验
        if(sto_small_mirror_err_check(&rdSto_map[0],16))
        {
        }
        else
        {
            *pOut_cycles = sto_small_get_write_cycles(&rdSto_map[0]);
            return(0x00);
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取信息段容量
//功能: 指示物理存储器可存储的最大容量和当前信息段大小及版本信息
//入口: 容量字节数指针,信息段字节数指针
//
//出口:错误代码,00未发送错误，0x01 错误,
//------------------------------------------------------------------------------
sdt_int8u mde_pull_stoCapacity_inf(sdt_int8u *pOut_maxBytes,sdt_int8u *pOut_infBytes,sdt_int8u *pOut_infVer)
{
    sdt_int8u rdSto_map[16];
    sdt_int8u block_st;
    sdt_int8u rd_infBytes;

//    if(sdt_false == cfged)
//    {
//        cfged = sdt_true;
//    }
    while(1)
    {
        block_st = get_sto_small_nowBlock();
        if(BLOCK_UN == block_st)
        {
            return(0x01);
        }
        sto_read_eepMonery_map(block_st,&rdSto_map[0]);//获取map并校验
        if(sto_small_mirror_err_check(&rdSto_map[0],16))
        {
        }
        else
        {
            rd_infBytes = rdSto_map[LOC_INFSIZE];
            *pOut_maxBytes = (MAX_EEBYTES - 17);
            *pOut_infBytes = rd_infBytes;
            *pOut_infVer = rdSto_map[LOC_INFVER];
            return(0x00);
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取存储id序列
//功能: 
//入口: 返回id数量，id首指针
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_pull_stoDeviceId(sdt_int8u *pOut_id,sdt_int8u in_idBytes)
{
    sdt_int8u rdSto_map[16];
    sdt_int8u block_st;
    sdt_int8u dvId_index;
    sdt_int8u i;

//     if(sdt_false == cfged)
//    {
//        cfged = sdt_true;
//    }
    while(1)
    {
        block_st = get_sto_small_nowBlock();
        if(BLOCK_UN == block_st)
        {
            return(0x01);
        }
        sto_read_eepMonery_map(block_st,&rdSto_map[0]);//获取map并校验
        if(sto_small_mirror_err_check(&rdSto_map[0],16))
        {
        }
        else
        {
            dvId_index = LOC_ID_0;
            
            if(in_idBytes > 8)
            {
                in_idBytes = 8;
            }
            for(i = 0;i < in_idBytes;i++)
            {
                pOut_id[i] = rdSto_map[dvId_index];
                dvId_index++;
            }
            return(0x00);
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取升级标识
//功能: 
//入口: 数据指针
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_pull_stoUpdateTag(sdt_int8u *pOut_tag)
{
    sdt_int8u rdSto_map[16];
    sdt_int8u block_st;

//    if(sdt_false == cfged)
//    {
//        cfged = sdt_true;
//    }
    while(1)
    {
        block_st = get_sto_small_nowBlock();
        if(BLOCK_UN == block_st)
        {
            return(UPTAG_NEEDUP);
        }
        sto_read_eepMonery_map(block_st,&rdSto_map[0]);//获取map并校验
        if(sto_small_mirror_err_check(&rdSto_map[0],16))
        {
        }
        else
        {
            *pOut_tag = sto_small_get_updateTag(&rdSto_map[0]);
            return(UPTAG_NONE);
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 写入信息序列数据
//功能: 
//入口: 信息数据指针，信息字节数，版本信息
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_write_storage_inf(sdt_int8u *pIn_inf,sdt_int8u in_bytes,sdt_int8u in_infVer)
{
    sdt_int8u rdSto_map[16];
    sdt_int8u block_st;
    sdt_int8u i;
    sdt_int16u map_cycles;
    STORAGE_EEF_DEF sto_eef;

//    if(sdt_false == cfged)
//    {
//        cfged = sdt_true;
//    }

    block_st = get_sto_small_nowBlock();
    if((BLOCK_0 == block_st) || (BLOCK_1 == block_st))
    {
        sto_read_eepMonery_map(block_st,&rdSto_map[0]);
        map_cycles = sto_small_get_write_cycles(&rdSto_map[0]);
        map_cycles++;
        sto_small_give_write_cycles(&rdSto_map[0],map_cycles);
        rdSto_map[LOC_INFVER] = in_infVer;
        rdSto_map[LOC_INFSIZE] = in_bytes;
        rdSto_map[15] = sto_small_checksum(&rdSto_map[0],15);
        if(BLOCK_0 == block_st)
        {
            sto_eef.in_block = BLOCK_1;
        }
        else
        {
            sto_eef.in_block = BLOCK_0;
        }
        
        sto_eef.pIn_eefMap = &rdSto_map[0];
        sto_eef.in_eefMap_bytes = 16;
        sto_eef.pIn_eefInf = pIn_inf;  //
        sto_eef.in_eefInf_bytes = in_bytes;
        sto_eef.in_eefCs = sto_small_checksum(&rdSto_map[0],16);

        for(i = 0;i < in_bytes;i++)
        {
            sto_eef.in_eefCs = sto_small_cs_append(&pIn_inf[i],1, sto_eef.in_eefCs);
        }
        bsp_write_eeMomery_bytes(&sto_eef);
    }
    else
    {
        rdSto_map[0] = 0x00;
        rdSto_map[1] = 0x00;
        rdSto_map[2] = 0x00;
        rdSto_map[LOC_INFVER] = in_infVer;
        rdSto_map[LOC_INFSIZE] = in_bytes;
        rdSto_map[5] = 0xfe;
        rdSto_map[6] = 0xfe;
        for(i = 0;i < 8;i++)
        {
            rdSto_map[LOC_ID_0 + i] = 0xfe;
        }
        rdSto_map[15] = sto_small_checksum(&rdSto_map[0],15);
        
        sto_eef.in_block = BLOCK_0;
        sto_eef.pIn_eefMap = &rdSto_map[0];
        sto_eef.in_eefMap_bytes = 16;
        sto_eef.pIn_eefInf = pIn_inf;  //
        sto_eef.in_eefInf_bytes = in_bytes;
        sto_eef.in_eefCs = sto_small_checksum(&rdSto_map[0],16);
        for(i = 0;i < in_bytes;i++)
        {
            sto_eef.in_eefCs = sto_small_cs_append(&pIn_inf[i],1, sto_eef.in_eefCs);
        }
        bsp_write_eeMomery_bytes(&sto_eef);
    }
    return(0);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 写入id数据
//功能: 
//入口: 数据指针，数据字节数
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_push_stoDeviceId(sdt_int8u *pIn_id,sdt_int8u in_idBytes)
{
    sdt_int8u rdSto_map[16];
    sdt_int8u block_st,inf_bytes;
    sdt_int8u i;
    sdt_int8u offset,rd_eem_data;
    sdt_int16u map_cycles;
    STORAGE_EEF_DEF sto_eef;

//    if(sdt_false == cfged)
//    {
//        cfged = sdt_true;
//    }

    block_st = get_sto_small_nowBlock();
    if((BLOCK_0 == block_st) || (BLOCK_1 == block_st))
    {
        sto_read_eepMonery_map(block_st,&rdSto_map[0]);
        map_cycles = sto_small_get_write_cycles(&rdSto_map[0]);
        map_cycles++;
        sto_small_give_write_cycles(&rdSto_map[0],map_cycles);
        if(in_idBytes > 8)
        {
            in_idBytes = 8;
        }
        for(i = 0;i < in_idBytes;i++)
        {
            rdSto_map[LOC_ID_0 + i] = pIn_id[i];
        }
        rdSto_map[15] = sto_small_checksum(&rdSto_map[0],15);
        if(BLOCK_0 == block_st)
        {
            sto_eef.in_block = BLOCK_1;
        }
        else
        {
            sto_eef.in_block = BLOCK_0;
        }
        
        inf_bytes = rdSto_map[LOC_INFSIZE];
        sto_eef.pIn_eefMap = &rdSto_map[0];
        sto_eef.in_eefMap_bytes = 16;
        sto_eef.pIn_eefInf = &rdSto_map[0];  //无效参数，随便填
        sto_eef.in_eefInf_bytes = inf_bytes;
        sto_eef.in_eefCs = sto_small_checksum(&rdSto_map[0],16);
        offset = 16;
        while(inf_bytes)
        {
            rd_eem_data = bsp_read_eeMomery_byte(block_st,offset);
            sto_eef.in_eefCs = sto_small_cs_append(&rd_eem_data,1, sto_eef.in_eefCs);
            offset++;
            inf_bytes--;
        }
        bsp_write_eeMomery_map(&sto_eef);
    }
    else
    {
        rdSto_map[0] = 0x00;
        rdSto_map[1] = 0x00;
        rdSto_map[2] = 0x00;
        rdSto_map[3] = 0x00;
        rdSto_map[4] = 0x00;
        rdSto_map[5] = 0xfe;
        rdSto_map[6] = 0xfe;
        for(i = 0;i < 8;i++)
        {
            rdSto_map[LOC_ID_0 + i] = 0xfe;
        }
        rdSto_map[15] = sto_small_checksum(&rdSto_map[0],15);
        
        sto_eef.in_block = BLOCK_0;
        sto_eef.pIn_eefMap = &rdSto_map[0];
        sto_eef.in_eefMap_bytes = 16;
        sto_eef.pIn_eefInf = &rdSto_map[0];  //无inf数据，随便填
        sto_eef.in_eefInf_bytes = 0;
        sto_eef.in_eefCs = sto_small_checksum(&rdSto_map[0],16);
        bsp_write_eeMomery_bytes(&sto_eef);
    }
    return(0);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 写入升级标识
//功能: 
//入口: 升级tag,0x00正常状态,0x01升级状态
//
//出口:错误代码,00未发送错误，0x01 map错误,0x02 inf错误
//------------------------------------------------------------------------------
sdt_int8u mde_push_stoUpdateTag(sdt_int8u in_newTag)
{
    sdt_int8u rdSto_map[16];
    sdt_int8u block_st,inf_bytes;
    sdt_int8u i;
    sdt_int8u offset,rd_eem_data;
    sdt_int16u map_cycles;
    STORAGE_EEF_DEF sto_eef;

//    if(sdt_false == cfged)
//    {
//        cfged = sdt_true;
//    }
    block_st = get_sto_small_nowBlock();
    if((BLOCK_0 == block_st) || (BLOCK_1 == block_st))
    {
        sto_read_eepMonery_map(block_st,&rdSto_map[0]);
        map_cycles = sto_small_get_write_cycles(&rdSto_map[0]);
        map_cycles++;
        sto_small_give_write_cycles(&rdSto_map[0],map_cycles);
        sto_small_give_updateTag(&rdSto_map[0],in_newTag);
        rdSto_map[15] = sto_small_checksum(&rdSto_map[0],15);
        if(BLOCK_0 == block_st)
        {
            sto_eef.in_block = BLOCK_1;
        }
        else
        {
            sto_eef.in_block = BLOCK_0;
        }
        
        inf_bytes = rdSto_map[LOC_INFSIZE];
        sto_eef.pIn_eefMap = &rdSto_map[0];
        sto_eef.in_eefMap_bytes = 16;
        sto_eef.pIn_eefInf = &rdSto_map[0];  //无效参数，随便填
        sto_eef.in_eefInf_bytes = inf_bytes; //
        sto_eef.in_eefCs = sto_small_checksum(&rdSto_map[0],16);
        offset = 16;
        while(inf_bytes)
        {
            rd_eem_data = bsp_read_eeMomery_byte(block_st,offset);  //从当前块取数据
            sto_eef.in_eefCs = sto_small_cs_append(&rd_eem_data,1, sto_eef.in_eefCs);
            offset++;
            inf_bytes--;
        }
        bsp_write_eeMomery_map(&sto_eef);
    }
    else
    {
        rdSto_map[0] = 0x00;
        rdSto_map[1] = 0x00;
        rdSto_map[2] = 0x00;
        rdSto_map[3] = 0x00;
        rdSto_map[4] = 0x00;
        rdSto_map[5] = 0xfe;
        rdSto_map[6] = 0xfe;
        for(i = 0;i < 8;i++)
        {
            rdSto_map[LOC_ID_0 + i] = 0xfe;
        }
        sto_small_give_updateTag(&rdSto_map[0],in_newTag);
        rdSto_map[15] = sto_small_checksum(&rdSto_map[0],15);
        sto_eef.in_block = BLOCK_0;
        sto_eef.pIn_eefMap = &rdSto_map[0];
        sto_eef.in_eefMap_bytes = 16;
        sto_eef.pIn_eefInf = &rdSto_map[0];  //无inf数据，随便填
        sto_eef.in_eefInf_bytes = 0;
        sto_eef.in_eefCs = sto_small_checksum(&rdSto_map[0],16);
        bsp_write_eeMomery_bytes(&sto_eef);
    }
    
    return(0);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
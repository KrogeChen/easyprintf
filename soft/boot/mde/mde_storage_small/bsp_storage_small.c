//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include ".\bsp_storage_small.h"
#include "hc32l13x.h"
#include "intrinsics.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

G_SHARE_DEF g_share;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//information 512 page_unit_size
//0x0000 F800 -- 0x0000 FFFF     2k     4   page 
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define addr_pageid_0        0x0000f800
#define addr_pageid_1        0x0000fA00
#define addr_pageid_2        0x0000fC00
#define addr_pageid_3        0x0000fE00

#define FEE_ADDR0  addr_pageid_0
#define FEE_ADDR1  addr_pageid_1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 块存储配置,HCLK 48M
//功能: 
//入口: 
//
//出口: 
//------------------------------------------------------------------------------
void bsp_blockflash_cfg(void)
{
    static sdt_bool cfged = sdt_false;
    
    if(sdt_false == cfged)
    {
        cfged = sdt_true;
        do
        {
            M0P_FLASH->BYPASS  = 0x5A5A;  //配置48M的flash操作时间
            M0P_FLASH->BYPASS  = 0xA5A5; 
            M0P_FLASH->TNVS    = 0x180;        
        }while(0x180 != M0P_FLASH->TNVS);

        do
        {
            M0P_FLASH->BYPASS  = 0x5A5A;
            M0P_FLASH->BYPASS  = 0xA5A5; 
            M0P_FLASH->TPGS    = 0xFF;        
        }while(0xFF != M0P_FLASH->TPGS );

        do
        {
            M0P_FLASH->BYPASS  = 0x5A5A;
            M0P_FLASH->BYPASS  = 0xA5A5; 
            M0P_FLASH->TPROG   = 0x144;        
        }while(0x144 != M0P_FLASH->TPROG);

        do
        {
            M0P_FLASH->BYPASS  = 0x5A5A;
            M0P_FLASH->BYPASS  = 0xA5A5; 
            M0P_FLASH->TSERASE = 0x34BC0;        
        }while(0x34BC0 != M0P_FLASH->TSERASE);

        do
        {
            M0P_FLASH->BYPASS  = 0x5A5A;
            M0P_FLASH->BYPASS  = 0xA5A5; 
            M0P_FLASH->TMERASE = 0x19A280;        
        }while(0x19A280 != M0P_FLASH->TMERASE);

        do
        {
            M0P_FLASH->BYPASS  = 0x5A5A;
            M0P_FLASH->BYPASS  = 0xA5A5; 
            M0P_FLASH->TPRCV   = 0x120;        
        }while(0x120 != M0P_FLASH->TPRCV);

        do
        {
            M0P_FLASH->BYPASS  = 0x5A5A;
            M0P_FLASH->BYPASS  = 0xA5A5; 
            M0P_FLASH->TSRCV   = 0xB40;        
        }while(0xB40 != M0P_FLASH->TSRCV);

        do
        {
            M0P_FLASH->BYPASS  = 0x5A5A;
            M0P_FLASH->BYPASS  = 0xA5A5;
            M0P_FLASH->TMRCV   = 0x2EE0;
        }while(0x2EE0 != M0P_FLASH->TMRCV);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:eeprom 块编程代码，适用于HC32L136K8TA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void program_block_eeprom(sdt_int16u in_addr,sdt_int8u in_word_32bits)
{
    sdt_int8u i;
    sdt_int32u* pFlashaddr_dst;
    sdt_int32u* pRam_src;
    
    bsp_blockflash_cfg();
    pFlashaddr_dst = (sdt_int32u*)in_addr;
    pRam_src = &g_share.g_share_bf32[0];
    
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5;    
        M0P_FLASH->SLOCK = 0x80000000;   //unlock page
    }while(M0P_FLASH->SLOCK != 0x80000000);
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5; 
        M0P_FLASH->CR_f.OP = 0x02;       //sector erase
    }while(0x02 != M0P_FLASH->CR_f.OP);
    
    *pFlashaddr_dst = 0x00000000;                //Erase page
    //write_into_flash(pAddr,0x0000000000);
    while(0 != M0P_FLASH->CR_f.BUSY)
    {
    }
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5; 
        M0P_FLASH->CR_f.OP = 0x01;       //program
    }while(0x01 != M0P_FLASH->CR_f.OP);

    
    for(i = 0;i < in_word_32bits;i++)
    {
        *pFlashaddr_dst = *pRam_src;
        //write_into_flash(pAddr,*in_pMap);
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
        pFlashaddr_dst ++;
        pRam_src ++;
    }

    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5;    
        M0P_FLASH->SLOCK = 0x00000000;   // lock sector
    }while(M0P_FLASH->SLOCK != 0x00000000);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:从EEPROM里面读取1个字节的数据
//入口:存储块序号，偏移位置
//出口:读取的数据
//------------------------------------------------------------------------------
sdt_int8u bsp_read_eeMomery_byte(sdt_int8u in_block,sdt_int16u in_offset)
{
    if(0 == in_block)
    {
        return(*((sdt_int8u*)(FEE_ADDR0+in_offset)));
    }
    else if(1 == in_block)
    {
        return(*((sdt_int8u*)(FEE_ADDR1+in_offset)));
    }
    return(0);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:写入EEPROM一串数据
//入口:eep数据信息结构指针
//出口:无
//------------------------------------------------------------------------------
void bsp_write_eeMomery_bytes(STORAGE_EEF_DEF *mix_sto_eef)
{
    sdt_int16u i;
    sdt_int16u eeaddr_dst;
    sdt_int16u b8_size;
    sdt_int8u  b32_size;
    
    b8_size = mix_sto_eef->in_eefMap_bytes + mix_sto_eef->in_eefInf_bytes + 1;
    b32_size = b8_size/4;
    if(0 != (b8_size%4))
    {
        b32_size += 1;
    }
    if(b32_size > MAX_EEBYTES/4)
    {
        return;
    }

    if(0 == mix_sto_eef->in_block)
    {
        eeaddr_dst = FEE_ADDR0;
    }
    else if(1 == mix_sto_eef->in_block)
    {
        eeaddr_dst = FEE_ADDR1;
    }
    for(i = 0;i < mix_sto_eef->in_eefMap_bytes;i++)
    {
        g_share.g_share_bf8[i] = mix_sto_eef->pIn_eefMap[i];
    }
    for(i = 0;i < mix_sto_eef->in_eefInf_bytes;i++)
    {
        g_share.g_share_bf8[i + mix_sto_eef->in_eefMap_bytes] = mix_sto_eef->pIn_eefInf[i];
    }
    g_share.g_share_bf8[mix_sto_eef->in_eefMap_bytes + mix_sto_eef->in_eefInf_bytes] = mix_sto_eef->in_eefCs;

    
    program_block_eeprom(eeaddr_dst,b32_size);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:写入EEPROM MAP数据,inf数据模块间内部转移
//入口:eep数据信息结构指针,inf指针无意义
//出口:无
//------------------------------------------------------------------------------
void bsp_write_eeMomery_map(STORAGE_EEF_DEF *mix_sto_eef)
{
    sdt_int16u i;
    sdt_int16u eeaddr_src,eeaddr_dst;
    sdt_int16u b8_size;
    sdt_int8u  b32_size;
    
    b8_size = mix_sto_eef->in_eefMap_bytes + mix_sto_eef->in_eefInf_bytes + 1;
    b32_size = b8_size/4;
    if(0 != (b8_size%4))
    {
        b32_size += 1;
    }
    if(b32_size > MAX_EEBYTES/4)
    {
        return;
    }
    if(0 == mix_sto_eef->in_block)
    {
        eeaddr_dst = FEE_ADDR0;
        eeaddr_src = FEE_ADDR1;
    }
    else if(1 == mix_sto_eef->in_block)
    {
        eeaddr_dst = FEE_ADDR1;
        eeaddr_src = FEE_ADDR0;
    }
    for(i = 0;i < mix_sto_eef->in_eefMap_bytes;i++)
    {
        g_share.g_share_bf8[i] = mix_sto_eef->pIn_eefMap[i];
    }
    eeaddr_src = eeaddr_src + mix_sto_eef->in_eefMap_bytes;
    for(i = 0;i < mix_sto_eef->in_eefInf_bytes;i++)
    {
        g_share.g_share_bf8[i + mix_sto_eef->in_eefMap_bytes] = *((sdt_int8u*)eeaddr_src);
        eeaddr_src ++;
    }
    g_share.g_share_bf8[mix_sto_eef->in_eefMap_bytes + mix_sto_eef->in_eefInf_bytes] = mix_sto_eef->in_eefCs;
    

    
    program_block_eeprom(eeaddr_dst,b32_size);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
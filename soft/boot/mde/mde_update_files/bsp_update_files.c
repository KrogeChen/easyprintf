//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include "..\..\pbc\pbc_data_convert\pbc_data_convert.h"
#include ".\bsp_update_files.h"
#include "..\.\mde_storage_small\bsp_storage_small.h"
#include "hc32l13x.h"
#include "intrinsics.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//flash map hc32l136k8ta
//64k flash,smallest unit 512b,128page
//0x0000 0000 -- 0x0000 FFFF
//------------------------------------------------------------------------------
//boot
//0x0000 0000 -- 0x0000 27FF     10k    20  page
//user.app 运行区
//0x0000 2800 -- 0x0000 F7FF     52k    104  page  
//------------------------------------------------------------------------------
//reserve(information)                  
//0x0000 F800 -- 0x0000 FFFF     2k     4   page 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define FLASHADDR_USERAPP    0x00002800
#define FLASHADDR_USAINF     0x0000F780   //尾端32bytes,信息标识
#define FLASHADDR_LASTPAGE   0x0000F600   //最后一个块
#define PAGE_UNIT_SIZE       512
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//硬件标识位置
#define FLASHADDR_HWFG_APP   (FLASHADDR_USERAPP + 0x000000C0)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef void (*pFunction)(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:跳转到APP区运行
//入口:
//------------------------------------------------------------------------------
void bsp_jump_userApp(void)
{
    pFunction Jump_To_Application;
    
    #ifdef NDEBUG
    //M0P_WDT->RST = 0x1E;
    //M0P_WDT->RST = 0xE1;
    #endif
    __disable_interrupt();
    /* Jump to user application */
    Jump_To_Application = (pFunction)(*(__IO uint32_t*)(FLASHADDR_USERAPP + 4));
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) FLASHADDR_USERAPP);
    Jump_To_Application(); 
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void program_block_flash(sdt_int16u in_addr,sdt_bool in_inf)
{
    sdt_int8u i;
    sdt_int32u local_addr;
    
    local_addr = in_addr;

    bsp_blockflash_cfg();
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5;    
        M0P_FLASH->SLOCK = 0x7FFFFFE0;   //unlock page
    }while(M0P_FLASH->SLOCK != 0x7FFFFFE0);
    if(in_inf)
    {
        do
        {
            M0P_FLASH->BYPASS = 0x5A5A;      //unlock
            M0P_FLASH->BYPASS = 0xA5A5; 
            M0P_FLASH->CR_f.OP = 0x02;       //sector erase
        }while(0x02 != M0P_FLASH->CR_f.OP);

        *((sdt_int8u*)FLASHADDR_LASTPAGE) = 0x00;   //Erase  page
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
    }
    if(0 == (local_addr%PAGE_UNIT_SIZE))  //page地址(512)
    {
        if(FLASHADDR_LASTPAGE == local_addr)
        {
        }
        else
        {
            do
            {
                M0P_FLASH->BYPASS = 0x5A5A;      //unlock
                M0P_FLASH->BYPASS = 0xA5A5; 
                M0P_FLASH->CR_f.OP = 0x02;       //sector erase
            }while(0x02 != M0P_FLASH->CR_f.OP);

            *((sdt_int8u*)local_addr) = 0x00;   //Erase  page
            while(0 != M0P_FLASH->CR_f.BUSY)
            {
            }
        }
    }
    
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5; 
        M0P_FLASH->CR_f.OP = 0x01;      //program
    }while(0x01 != M0P_FLASH->CR_f.OP);
    
    for(i = 0;i < 32;i++)
    {
        #ifdef NDEBUG
        M0P_WDT->RST = 0x1E;
        M0P_WDT->RST = 0xE1; 
        #endif
        
        *(sdt_int32u*)local_addr = g_share.g_share_bf32[i];
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
        local_addr += 4;
    }
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5;    
        M0P_FLASH->SLOCK = 0x00000000;   // lock sector
    }while(M0P_FLASH->SLOCK != 0x00000000);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:读取4bytes的数据,32bits
//入口:地址偏移量,缓冲区指针,缓冲区需要>=4bytes
//------------------------------------------------------------------------------
void bsp_read_userApp_4bytes(sdt_int32u in_addrOffset,sdt_int8u *pOut_buff)
{
    sdt_int8u i;
    
    for(i = 0;i < 4;i++)
    {
        pOut_buff[i] = *(sdt_int8u*)(FLASHADDR_USERAPP + in_addrOffset + i);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:读取32bytes的信息区数据
//入口:指针，缓冲区必须>=32bytes,否则溢出
//------------------------------------------------------------------------------
void bsp_read_information_userApp(sdt_int8u *pOut_buff)
{
    sdt_int8u i;
    
    for(i = 0;i < 32;i++)
    {
        pOut_buff[i] = *(sdt_int8u*)(FLASHADDR_USAINF + 96 + i);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:写入32bytes的信息区数据
//入口:指针，缓冲区必须>=32bytes,否则溢出
//------------------------------------------------------------------------------
void bsp_write_information_userApp(sdt_int8u *pIn_buff)
{
    sdt_int8u i;
    for(i = 0;i < 96;i++)
    {
        g_share.g_share_bf8[i] = 0;
    }
    for(i = 0;i < 32;i++)
    {
        g_share.g_share_bf8[i + 96] = pIn_buff[i];
    }
    program_block_flash(FLASHADDR_USAINF,sdt_true);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:写入128bytes的数据到用户代码区，一个升级块的数据
//入口:指针，缓冲区必须>=128bytes,否则溢出
//------------------------------------------------------------------------------
void bsp_write_userApp_128bytes(sdt_int32u in_addrOffset,sdt_int8u *pIn_buff)
{
    sdt_int16u flash_addr;
    sdt_int8u i;
    
    for(i = 0;i < 128;i++)
    {
        g_share.g_share_bf8[i] = pIn_buff[i];
    }
    
    flash_addr = FLASHADDR_USERAPP + in_addrOffset;
    program_block_flash(flash_addr,sdt_false);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:获取APP区硬件标识
//入口:none
//出口:32bit的硬件标识
//------------------------------------------------------------------------------
sdt_int32u bsp_pull_hardware_flag(void)
{
    return(pbc_arrayToInt32u_bigEndian((sdt_int8u*)FLASHADDR_HWFG_APP));
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
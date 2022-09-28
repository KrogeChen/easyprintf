//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "..\..\pbc\pbc_data_convert\pbc_data_convert.h"
#include ".\mde_update_files.h"
#include ".\bsp_update_files.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//升级文件处理代码
//适用于只有一个APP区(user.app)、升级功能由BOOT区提供的应用.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//fireware update task
//------------------------------------------------------------------------------
typedef enum
{
    fwUpdate_status_waitFilesMap       = 0x00,
    fwUpdate_status_receiveFiles       = 0x01,
    fwUpdate_status_complete           = 0x02,
    fwUpdate_status_restart            = 0x03,
}fwUpdate_status_def;

//------------------------------------------------------------------------------
static fwUpdate_status_def fwUpdate_status = fwUpdate_status_waitFilesMap;
static sdt_int16u fwBlock_number;  
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define CheckSumWordEven          0x39ea2e76
#define CheckSumWordOdd           0x82b453c3
//------------------------------------------------------------------------------
sdt_int32u MakeOneCheckText(sdt_int32u Data,sdt_int32u Count)
{
    if(Count&0x00000001)
    {
        Data=Data^CheckSumWordOdd;
    }
    else
    {
        Data=Data^CheckSumWordEven;
    }

    return(Data);
}
//------------------------------------------------------------------------------
//生成校验和
//入口:8bits的数据指针，数据长度
//出口:32bit的校验和
//------------------------------------------------------------------------------
sdt_int32u MakeCheckSumText(sdt_int8u* pData,sdt_int32u Length)
{
    sdt_int32u iWords,This_ids;
    sdt_int32u Index;
    sdt_int32u MakeSum,ReadData;
    iWords = (Length)/4;
    This_ids = 0;
    MakeSum = 0;
    Index = 0;
    while(iWords)
    {
        ReadData = (sdt_int32u)pData[Index]&0x000000ff;
        ReadData = ReadData<<8;
        ReadData = ReadData|((sdt_int32u)pData[Index+1]&0x000000ff);
        ReadData = ReadData<<8;
        ReadData = ReadData|((sdt_int32u)pData[Index+2]&0x000000ff);
        ReadData = ReadData<<8;
        ReadData = ReadData|((sdt_int32u)pData[Index+3]&0x000000ff);
        MakeSum+=MakeOneCheckText(ReadData,This_ids);
        Index = Index+4;
        iWords--;
        This_ids++;
    }
    return(MakeSum);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//#define KEYVAL  0x89,0x8E,0x3C,0x72,0xA7,0x19,0xF4,0x5D
//------------------------------------------------------------------------------
//名称: 文件解码
//功能: 传入密文，传出明文
//入口: 
//        _Mix_binf   文件入口指针                    <<---->>
//        _In_Length  文件长度,必须是128的倍数        <<----
//出口: 
//------------------------------------------------------------------------------
static void Decrypt(sdt_int8u* _Mix_binf,sdt_int8u _In_Length)
{
    sdt_int8u Key[8]={(sdt_int8u)0x89,(sdt_int8u)0x8E,(sdt_int8u)0x3C,(sdt_int8u)0x72,(sdt_int8u)0xA7,(sdt_int8u)0x19,(sdt_int8u)0xF4,(sdt_int8u)0x5D,};
    sdt_int8u i;
    sdt_int8u Length,BaseIndex;

    if(_In_Length < 128)
    {
        return;
    }
    
    Length = _In_Length;
    BaseIndex = 0;
    while(0!=Length)
    {
        Key[0] = (sdt_int8u)0x89;
        Key[1] = (sdt_int8u)0x8E;
        Key[2] = (sdt_int8u)0x3C;
        Key[3] = (sdt_int8u)0x72;
        Key[4] = (sdt_int8u)0xA7;
        Key[5] = (sdt_int8u)0x19;
        Key[6] = (sdt_int8u)0xF4;
        Key[7] = (sdt_int8u)0x5D;

        for(i=0 ;i<16 ;i++)
        {
            _Mix_binf[0+i*8+BaseIndex] = _Mix_binf[0+i*8+BaseIndex] ^ Key[0];
            _Mix_binf[1+i*8+BaseIndex] = _Mix_binf[1+i*8+BaseIndex] ^ Key[1];
            _Mix_binf[2+i*8+BaseIndex] = _Mix_binf[2+i*8+BaseIndex] ^ Key[2];
            _Mix_binf[3+i*8+BaseIndex] = _Mix_binf[3+i*8+BaseIndex] ^ Key[3];
            _Mix_binf[4+i*8+BaseIndex] = _Mix_binf[4+i*8+BaseIndex] ^ Key[4];
            _Mix_binf[5+i*8+BaseIndex] = _Mix_binf[5+i*8+BaseIndex] ^ Key[5];
            _Mix_binf[6+i*8+BaseIndex] = _Mix_binf[6+i*8+BaseIndex] ^ Key[6];
            _Mix_binf[7+i*8+BaseIndex] = _Mix_binf[7+i*8+BaseIndex] ^ Key[7];

            char lastkeybit;
            if(Key[0] & 0x80)
            {
                lastkeybit = 0x01;
            }
            else
            {
                lastkeybit = 0x00;
            }
            unsigned char ics;
            for(ics=0;ics<7;ics++)
            {
                Key[ics] = Key[ics]<<1;
                if(Key[ics+1] & 0x80)
                {
                    Key[ics] |= 0x01;
                }
            }
            Key[ics] = Key[ics]<<1;
            Key[ics] |= lastkeybit;
        }
        if(Length < 128)
        {
            break;
        }
        Length = Length-128;
        BaseIndex += 128;
    }
}
//-------------------------------------------------------------------------------
static sdt_int32u makeData32u_bigEndian(sdt_int8u* in_data)
{
    sdt_int32u the_data;
    
    the_data = in_data[0];
    the_data = the_data<<8;
    the_data |= in_data[1];
    the_data = the_data<<8;
    the_data |= in_data[2];
    the_data = the_data<<8;
    the_data |= in_data[3];
    return(the_data);
}
//-------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//task++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//none
//interface+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:推入文件分布map数据到文件模块
//入口:map数据指针(128bytes)，是否续传 sdt_true 续传
//出口:错误指示
//------------------------------------------------------------------------------
sdt_int8u mde_push_updateFile_map(sdt_int8u *pIn_buff,sdt_bool in_resume)
{
    sdt_int8u error = ERRUF_NONE;
    sdt_int32u checkSum;
    sdt_int32u fileMap_ver;
    sdt_int32u update_checksum;
    sdt_int32u update_codesize;
    sdt_int8u wr_inf[32];
        
    checkSum = makeData32u_bigEndian(&pIn_buff[124]);
    if(checkSum == MakeCheckSumText(&pIn_buff[0],124)) //
    {
        fileMap_ver = pbc_arrayToInt32u_bigEndian(&pIn_buff[120]);  //version
        if(0x00000001 == fileMap_ver)
        {
            update_checksum = pbc_arrayToInt32u_bigEndian(&pIn_buff[108]);//校验文
            update_codesize = pbc_arrayToInt32u_bigEndian(&pIn_buff[112]);//代码字节数    
            if(in_resume)  //断点续传
            {
            }
            else
            {
                fwBlock_number = 0;  //非续传,固件块计数清零
                                       //存储信息块
                pbc_int32uToArray_bigEndian(0xfefefefe,&wr_inf[0]);
                pbc_int32uToArray_bigEndian(0xfefefefe,&wr_inf[4]);
                pbc_int32uToArray_bigEndian(~update_checksum,&wr_inf[8]);
                pbc_int32uToArray_bigEndian(~update_codesize,&wr_inf[12]);
                pbc_int32uToArray_bigEndian(0xfefefefe,&wr_inf[16]);
                pbc_int32uToArray_bigEndian(0xfefefefe,&wr_inf[20]);
                pbc_int32uToArray_bigEndian(update_checksum,&wr_inf[24]);
                pbc_int32uToArray_bigEndian(update_codesize,&wr_inf[28]);
                bsp_write_information_userApp(&wr_inf[0]);       //写入信息到upgrade区
            }
            fwUpdate_status = fwUpdate_status_receiveFiles; //进入文件传送状态    
            
        }
        else
        {
            error = ERRUF_MAP_VER;
        }
    }
    else
    {
        error = ERRUF_MAP_CS;
    }
    return(error);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:推入一个块的数据到文件模块
//入口:块序号、块数据指针，块大小默认为128bytes
//出口:错误指示
//------------------------------------------------------------------------------
sdt_int8u mde_push_updateFiles_oneBlock(sdt_int8u *pIn_buff,sdt_int16u in_block)
{
    sdt_int8u error = ERRUF_NONE;
    sdt_int8u rd_inf[32];
    sdt_int32u update_codesize,update_codesize_anti;
    
    switch(fwUpdate_status)
    {
        case fwUpdate_status_receiveFiles:
        {
            if(in_block != (fwBlock_number+1))  //文件块比固件块大1
            {
                error = ERRUF_BLOCK_NUM;
            }
            else
            {
                bsp_read_information_userApp(&rd_inf[0]); //读取信息块内容
                update_codesize_anti = pbc_arrayToInt32u_bigEndian(&rd_inf[12]);  
                update_codesize = pbc_arrayToInt32u_bigEndian(&rd_inf[28]);  
                if(0 == (update_codesize + update_codesize_anti + 1))
                {
                    Decrypt(&pIn_buff[0],128);
                    bsp_write_userApp_128bytes((sdt_int32u)fwBlock_number * 128, &pIn_buff[0]);//写入一个块的数据到upgrade区
                    fwBlock_number ++;
                    if(fwBlock_number < (update_codesize / 128))
                    {
                    }
                    else
                    {
                        fwUpdate_status = fwUpdate_status_complete;
                    }                      
                }
                else
                {
                    error = ERRUF_OTHER;
                }
            }
            break;
        }
        default:
        {
            error = ERRUF_OTHER;
            break;
        }
    }
    return(error);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
//名称:获取下一个数据块的序号值
//文件块比固件块大1
//入口:序号块指针
//出口:是否传输完毕，sdt_true -- 传输完毕
//------------------------------------------------------------------------------
sdt_bool mde_pull_updateFiles_nextBlock(sdt_int16u *pOut_block)
{
    *pOut_block = (fwBlock_number+1);
    if(fwUpdate_status_complete == fwUpdate_status)
    {
        return(sdt_true);
    }
    return(sdt_false);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
//名称:APP运行区的数据合法判断
//入口:
//出口:sdt_true -- 数据合法
//------------------------------------------------------------------------------
sdt_bool mde_pull_appSectionFiles_iegal(void)
{
    sdt_int8u rd_buff[32];
    sdt_int32u update_codesize,update_codesize_anti;
    sdt_int32u update_cs,update_cs_anti;
    sdt_int32u make_cs,rd_32bits;
    sdt_int32u i,words,offset;
    
    bsp_read_information_userApp(&rd_buff[0]);
    update_cs_anti = pbc_arrayToInt32u_bigEndian(&rd_buff[8]);
    update_codesize_anti = pbc_arrayToInt32u_bigEndian(&rd_buff[12]);
    update_cs = pbc_arrayToInt32u_bigEndian(&rd_buff[24]);
    update_codesize = pbc_arrayToInt32u_bigEndian(&rd_buff[28]);
    
    if(0 != (update_cs_anti + update_cs + 1))
    {
        return(sdt_false);
    }
    if(0 != (update_codesize_anti + update_codesize + 1))
    {
        return(sdt_false);
    }

    make_cs = 0;
    offset = 0;
    words = update_codesize/4;    //换算成4byts的word模式
    for(i = 0;i < words;i++)
    {
        bsp_read_userApp_4bytes(offset,&rd_buff[0]);
        rd_32bits = pbc_arrayToInt32u_bigEndian(&rd_buff[0]);
        make_cs += MakeOneCheckText(rd_32bits,i);
        offset += 4;      
    }
    if(make_cs != update_cs)
    {
        return(sdt_false);
    }
    return(sdt_true);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:PC指针跳转到APP区运行代码
//入口:
//出口:
//------------------------------------------------------------------------------
void jumpToApplication(void)
{
    bsp_jump_userApp();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


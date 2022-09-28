//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//查询设备的升级状态
//FE FE FE 68 42 AA AA AA AA AA AA AA 00 08 82 52 01 01 FF FF FF FF 2A 16
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//协议定位
#define PK_VER                   0
#define PK_CMD                   1
#define PK_STATUS                2
#define PK_ERROR                 3
#define PK_BNUM_0                4  //文件块序号,16bits 大端模式
#define PK_BNUM_1                5
#define PK_BFILES                6  //文件块内容
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define CMD_QUERY                0x01
#define CMD_ENTRY_BOOT           0x02
#define CMD_UPDATE               0x03
#define CMD_RESUME               0x04
#define CMD_FLIES                0x05
                               
#define CMD_ERR_RP               0xFE  //错误报告
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define STU_NOTBOOT              0x01
#define STU_REBOOT               0x02
#define STU_INBOOT               0x10
#define STU_QYMAP                0x11
#define STU_QYFLIE               0x12
#define STU_CPT                  0x1F //complete
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define INK_VERSION              0x01
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define FILL_MARK                0xFF //mbus链路采用0xFF填充,减少电流消耗
#define HARDWARE_SEQ             0x02 //硬件标识数据,第2个文件块的4个字节
#define HARDWARE_LOC             64   //硬件内容数组定位
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_int8u answer_buff[6];
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//task++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//interface+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:固件升级数据交换模块
//入口:接收的数据长度和指针,应答的数据长度和指针\
//出口:sdt_true 文件传输完成
//------------------------------------------------------------------------------
sdt_bool app_easyink_message_exchange(EZINK_MSG_DEF *mix_ezink_msg)
{
    sdt_bool   complete = sdt_false;
    sdt_int8u  error = FILL_MARK;    //
    sdt_int8u  status = FILL_MARK;
    sdt_int16u request_number = 0; 
    sdt_int16u files_number;
    
    if(mix_ezink_msg->eik_apt_length < 6)
    {
        error = ERRUF_LENGTH;
    }
    else if(INK_VERSION == mix_ezink_msg->pEik_apt_buff[PK_VER])
    {
        switch(mix_ezink_msg->pEik_apt_buff[PK_CMD])
        {
            case CMD_QUERY:
            {
                status = STU_INBOOT;
                break;
            }
            case CMD_ENTRY_BOOT:
            {
                status = STU_QYMAP;    //请求MAP
                break;
            }
            case CMD_UPDATE:     //升级
            {
                if(mix_ezink_msg->eik_apt_length < (6 + 128))
                {
                    error = ERRUF_LENGTH;
                }
                else
                {
                    status = STU_QYFLIE;
                    error = mde_push_updateFile_map(&mix_ezink_msg->pEik_apt_buff[PK_BFILES],sdt_false);
                    mde_pull_updateFiles_nextBlock(&request_number);                    
                }
                break;
            }
            case CMD_RESUME:    //续传
            {
                if(mix_ezink_msg->eik_apt_length < (6 + 128))
                {
                    error = ERRUF_LENGTH;
                }
                else
                {
                    status = STU_QYFLIE;
                    error = mde_push_updateFile_map(&mix_ezink_msg->pEik_apt_buff[PK_BFILES],sdt_true);
                    mde_pull_updateFiles_nextBlock(&request_number);                    
                }
                break;
            }
            case CMD_FLIES:
            {
                if(mix_ezink_msg->eik_apt_length < (6 + 128))
                {
                    error = ERRUF_LENGTH;
                }
                else
                {
                    files_number = pbc_arrayToInt16u_bigEndian(&mix_ezink_msg->pEik_apt_buff[PK_BNUM_0]);  //计数文件块编号
                    status = STU_QYFLIE;
                    error = mde_push_updateFiles_oneBlock(&mix_ezink_msg->pEik_apt_buff[PK_BFILES],files_number); //解码
                    if(HARDWARE_SEQ == files_number)
                    {
                        if(HARDWARE_MARK != pbc_arrayToInt32u_bigEndian(&mix_ezink_msg->pEik_apt_buff[PK_BFILES + HARDWARE_LOC]))
                        {
                            error = ERRUF_FIREWARE;  //固件标识错误
                        }
                    }
                    if(mde_pull_updateFiles_nextBlock(&request_number))
                    {//传输完毕,complete
                        status = STU_CPT;
                        complete = sdt_true;
                    }                       
                }
                break;
            }
            default:
            {
                error = ERRUF_CMD;
                break;
            }
        }
    }
    else
    {
        error = ERRUF_VERSION;
    }
//------------------------------------------------------------------------------answer message
    if((0 == error) ||(0xff == error))
    {
        answer_buff[PK_CMD] = (mix_ezink_msg->pEik_apt_buff[PK_CMD] | 0x80);
    }
    else
    {
        answer_buff[PK_CMD] = CMD_ERR_RP;
    }
    answer_buff[PK_VER] = INK_VERSION;
    
    answer_buff[PK_STATUS] = status;
    answer_buff[PK_ERROR] = error;
    pbc_int16uToArray_bigEndian(request_number,&answer_buff[PK_BNUM_0]);
    mix_ezink_msg->pEik_ans_buff = &answer_buff[0];
    mix_ezink_msg->eik_ans_lenght = 6;
    
    return(complete);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
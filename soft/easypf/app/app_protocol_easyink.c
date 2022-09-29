//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//只保留的查询和进入BOOT的命令,适合嵌入到APP组件内
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define ERRUF_NONE               0x00
#define ERRUF_LENGTH             0x01
#define ERRUF_VERSION            0x02
#define ERRUF_CMD                0x03
#define ERRUF_MAP_CS             0x04
#define ERRUF_FILE_CS            0x05
#define ERRUF_BLOCK_NUM          0x06
#define ERRUF_FIREWARE           0x07
#define ERRUF_MAP_VER            0x08
#define ERRUF_OTHER              0xFE
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
#define HARDWARE_SEQ             0x02
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//static sdt_int8u answer_buff[6];
//应答缓冲采用共享内存
#define  answer_buff  g_share_buff
//------------------------------------------------------------------------------
macro_cTimerTig(timer_reboot,timerType_millisecond);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//task++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_easyink_reboot_task(void)
{
    pbc_timerClockRun_task(&timer_reboot);     
    if(pbc_pull_timerIsOnceTriggered(&timer_reboot))
    {
        while(1);  //reboot
    }
}
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
    sdt_int16u request_number = 0xFFFF; 
    
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
                status = STU_NOTBOOT;
                break;
            }
            case CMD_ENTRY_BOOT:
            {
                status = STU_REBOOT;    //重启,进入BOOT状态
                pbc_reload_timerClock(&timer_reboot,1000);  //1000ms后reboot
                mde_push_stoUpdateTag(UPTAG_NEEDUP);        //升级标识
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
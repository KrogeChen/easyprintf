//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//debug
//FE FE FE 68 42 AA AA AA AA AA AA AA 00 08 82 53 01 01 FF FF FF FF 2B 16
//FE FE FE 68 42 FE FE FE FE FE FE FE 00 08 82 53 01 01 FF FF FF FF 77 16
//FE FE FE 68 42 AA AA AA AA AA AA AA 00 08 82 53 02 32 FF FF FF FF 5D 16    50%开度
//FE FE FE 68 42 AA AA AA AA AA AA AA 00 08 82 53 02 50 FF FF FF FF 7B 16    80%开度
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MBUS_S0     0x00
//------------------------------------------------------------------------------
#define TREES_MTP   0x42//仪表类型
//------------------------------------------------------------------------------
//easy ink upgrade
#define MKEXE0_EINK    0x82
#define MKEXE1_EINK    0x52
//------------------------------------------------------------------------------
//debug
#define MKEXE0_DEBUG   0x82
#define MKEXE1_DEBUG   0x53
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_bool cfged = sdt_false;
static sdt_int8u mbus_local_addr[7];
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define ADDR_NOT_MATCH         0x00
#define ADDR_MATCH             0x01
#define ADDR_BROADCAST_NACK    0x02
#define ADDR_BROADCAST_ACK     0x03
//------------------------------------------------------------------------------
//功能:地址匹配检查
//入口:地址指针
//出口:sdt_true 地址匹配
//------------------------------------------------------------------------------
static sdt_int8u address_match(sdt_int8u *pIn_addr)
{
    sdt_int8u i;
    sdt_int8u addr_match;
    
    addr_match = ADDR_BROADCAST_ACK;
    for(i = 0;i < 7;i++)
    {
        if(pIn_addr[i] != 0xaa)
        {
            addr_match = ADDR_NOT_MATCH;
            break;
        }
    }
    if(sdt_false == addr_match)
    {
        addr_match = ADDR_BROADCAST_NACK;
        for(i = 0;i < 7;i++)
        {
            if(pIn_addr[i] != 0xfe)
            {
                addr_match = ADDR_NOT_MATCH;
                break;
            }
        }
    }
    if(sdt_false == addr_match)
    {
        addr_match = ADDR_MATCH;
        for(i = 0;i < 7;i++)
        {
            if(pIn_addr[i] != mbus_local_addr[i])
            {
                addr_match = ADDR_NOT_MATCH;
                break;
            }
        }
    }
    return(addr_match);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_bool protocol_treesfeel_regist(mbus_link_buff_def* pIn_link_buff);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//task++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_mbus_task(void)
{
    sdt_bool answer_en = sdt_false;
    sdt_int8u match_type;
    mbus_link_buff_def *mbus_link_buff;
    EZINK_MSG_DEF   ezink_msg;
    sdt_int8u i;
//------------------------------------------------------------------------------
    if(sdt_false == cfged)
    {
        cfged = sdt_true;
        if(mde_pull_stoDeviceId(&mbus_local_addr[0],7))
        {//pull error
            for(i = 0;i < 7;i++)
            {
                mbus_local_addr[i] = 0xfe;
            }
        }
    }
//------------------------------------------------------------------------------
    mde_mbus_link_task();
    if(mde_pull_mbus_recevie_one_message(MBUS_S0))
    {
        mbus_link_buff = mde_pull_mbus_message_pBuff(MBUS_S0);
        if(TREES_MTP == mbus_link_buff->meter_type)  //treesfeel meter type
        {
            if(match_type = address_match(&mbus_link_buff->meter_addr[0]))//地址匹配
            {
                if((MKEXE0_EINK == mbus_link_buff->payload[0])&&(MKEXE1_EINK == mbus_link_buff->payload[1]))
                {
                    ezink_msg.pEik_apt_buff = &mbus_link_buff->payload[2];
                    ezink_msg.eik_apt_length = mbus_link_buff->payload_len - 2;
                    ezink_msg.pEik_ans_buff = &g_share_buff[0];
                    if(app_easyink_message_exchange(&ezink_msg))  //easyink protocol process
                    {//update complete
                    }
                    for(i = 0;i < 7;i++)
                    {
                        mbus_link_buff->meter_addr[i] = mbus_local_addr[i];
                    }
                    mbus_link_buff->meter_type = TREES_MTP;
                    mbus_link_buff->payload_len = ezink_msg.eik_ans_lenght + 2;
                    for(i = 0;i < ezink_msg.eik_ans_lenght;i++)
                    {
                        mbus_link_buff->payload[i+2] = ezink_msg.pEik_ans_buff[i];  //转移数据到mbus缓冲区
                    }
                    answer_en = sdt_true;
                }
                else if((MKEXE0_DEBUG == mbus_link_buff->payload[0])&&(MKEXE1_DEBUG == mbus_link_buff->payload[1]))
                {//debug

                }
                else
                {
                 
                }
                if(ADDR_BROADCAST_NACK == match_type)
                {
                    answer_en = sdt_false;
                }
            }                
        }
        if(answer_en)
        {
            mde_push_mbus_active_one_message_transmit(MBUS_S0);  //应答
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
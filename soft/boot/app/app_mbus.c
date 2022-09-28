//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define UPDATE_S0   0x00  //升级端口实列编号
//------------------------------------------------------------------------------
#define TREES_MTP   0x42
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static sdt_bool cfged = sdt_false;
static sdt_int8u mbus_addr[7];
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//task++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_mbus_task(void)
{
    sdt_bool addr_match;
    mbus_link_buff_def *mbus_link_buff;
    EZINK_MSG_DEF   ezink_msg;
    sdt_int8u i;
//------------------------------------------------------------------------------
    if(sdt_false == cfged)
    {
        cfged = sdt_true;
        if(mde_pull_stoDeviceId(&mbus_addr[0],7))
        {//pull error
            for(i = 0;i < 7;i++)
            {
                mbus_addr[i] = 0xfe;
            }
        }
    }
//------------------------------------------------------------------------------
    mde_mbus_link_task();
    if(mde_pull_mbus_recevie_one_message(UPDATE_S0))
    {
        mbus_link_buff = mde_pull_mbus_message_pBuff(UPDATE_S0);
        if(TREES_MTP == mbus_link_buff->meter_type)
        {
            addr_match = sdt_true;
            for(i = 0;i < 7;i++)
            {
                if(mbus_link_buff->meter_addr[i] != 0xaa)
                {
                    addr_match = sdt_false;
                    break;
                }
            }
            if(sdt_false == addr_match)
            {
                addr_match = sdt_true;
                for(i = 0;i < 7;i++)
                {
                    if(mbus_link_buff->meter_addr[i] != 0xfe)
                    {
                        addr_match = sdt_false;
                        break;
                    }
                }
            }
            if(sdt_false == addr_match)
            {
                addr_match = sdt_true;
                for(i = 0;i < 7;i++)
                {
                    if(mbus_link_buff->meter_addr[i] != mbus_addr[i])
                    {
                        addr_match = sdt_false;
                        break;
                    }
                }
            }
//------------------------------------------------------------------------------
            if(addr_match)
            {
                if((0x82 == mbus_link_buff->payload[0])&&(0x52 == mbus_link_buff->payload[1]))
                {
                    app_bss_reside_update();        //接收到升级报文,驻留boot运行,进行升级
                    ezink_msg.pEik_apt_buff = &mbus_link_buff->payload[2];
                    ezink_msg.eik_apt_length = mbus_link_buff->payload_len - 2;
                    ezink_msg.pEik_ans_buff = (sdt_int8u*)0x00;
                    if(app_easyink_message_exchange(&ezink_msg))
                    {//update complete
                        app_bss_goto_userApp();//进入user.app,函数自带默认延时
                    }
                    for(i = 0;i < 7;i++)
                    {
                        mbus_link_buff->meter_addr[i] = mbus_addr[i];
                    }
                    mbus_link_buff->meter_type = TREES_MTP;
                    mbus_link_buff->payload_len = ezink_msg.eik_ans_lenght + 2;
                    for(i = 0;i < ezink_msg.eik_ans_lenght;i++)
                    {
                        mbus_link_buff->payload[i+2] = ezink_msg.pEik_ans_buff[i];  //转移数据到mbus缓冲区
                    }
                    mde_push_mbus_active_one_message_transmit(UPDATE_S0);  //应答
                }
            }
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
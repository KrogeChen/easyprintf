//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const sdt_string pbc_acsii_table[]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//字符串清零
//-------------------------------------------------------------------------------------------------
void pbc_clear_string(sdt_string* out_string,sdt_int16u in_length)
{
    sdt_int16u i;
    
    for(i = 0;i < in_length; i++)
    {
        out_string[i] = 0;
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void pbc_int16u_to_bcdCode(sdt_int16u in_data,sdt_int8u* out_pBcdCode)
{
    sdt_int16u temp;

    out_pBcdCode[4]=(in_data/10000);
    temp=in_data%10000;
    out_pBcdCode[3]=(temp/1000);
    temp=in_data%1000;
    out_pBcdCode[2]=(temp/100);
    temp=in_data%100;
    out_pBcdCode[1]=(temp/10);
    temp=in_data%10;
    out_pBcdCode[0]=temp;  
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void pbc_int16u_to_string_convert(sdt_int16u in_data,sdt_string* out_string)
{
    sdt_int8u bcdCode[5];
    
    pbc_int16u_to_bcdCode(in_data,&bcdCode[0]);
    
    if(0 != bcdCode[4])
    {
        out_string[0] = pbc_acsii_table[bcdCode[4]];
        out_string[1] = pbc_acsii_table[bcdCode[3]];
        out_string[2] = pbc_acsii_table[bcdCode[2]];
        out_string[3] = pbc_acsii_table[bcdCode[1]];
        out_string[4] = pbc_acsii_table[bcdCode[0]];
    }
    else if(0 != bcdCode[3])
    {
        out_string[0] = pbc_acsii_table[bcdCode[3]];
        out_string[1] = pbc_acsii_table[bcdCode[2]];
        out_string[2] = pbc_acsii_table[bcdCode[1]];
        out_string[3] = pbc_acsii_table[bcdCode[0]];
    }
    else if(0 != bcdCode[2])
    {
        out_string[0] = pbc_acsii_table[bcdCode[2]];
        out_string[1] = pbc_acsii_table[bcdCode[1]];
        out_string[2] = pbc_acsii_table[bcdCode[0]];
    }
    else if(0 != bcdCode[1])
    {
        out_string[0] = pbc_acsii_table[bcdCode[1]];
        out_string[1] = pbc_acsii_table[bcdCode[0]];
    }
    else
    {
        out_string[0] = pbc_acsii_table[bcdCode[0]];
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void pbc_int16s_to_string_convert(sdt_int16s in_data,sdt_string* out_string)
{
    sdt_int16u hex_data;
    sdt_int8u bcdCode[5];
    
    if(in_data < 0)
    {
        hex_data = ~in_data + 1;
        pbc_int16u_to_bcdCode(hex_data,&bcdCode[0]);
        
        out_string[0] = 0x2d; //-
        if(0 != bcdCode[4])
        {
            out_string[1] = pbc_acsii_table[bcdCode[4]];
            out_string[2] = pbc_acsii_table[bcdCode[3]];
            out_string[3] = pbc_acsii_table[bcdCode[2]];
            out_string[4] = pbc_acsii_table[bcdCode[1]];
            out_string[5] = pbc_acsii_table[bcdCode[0]];
        }
        else if(0 != bcdCode[3])
        {
            out_string[1] = pbc_acsii_table[bcdCode[3]];
            out_string[2] = pbc_acsii_table[bcdCode[2]];
            out_string[3] = pbc_acsii_table[bcdCode[1]];
            out_string[4] = pbc_acsii_table[bcdCode[0]];
        }
        else if(0 != bcdCode[2])
        {
            out_string[1] = pbc_acsii_table[bcdCode[2]];
            out_string[2] = pbc_acsii_table[bcdCode[1]];
            out_string[3] = pbc_acsii_table[bcdCode[0]];
        }
        else if(0 != bcdCode[1])
        {
            out_string[1] = pbc_acsii_table[bcdCode[1]];
            out_string[2] = pbc_acsii_table[bcdCode[0]];
        }
        else
        {
            out_string[1] = pbc_acsii_table[bcdCode[0]];
        }
    }
    else
    {
        pbc_int16u_to_bcdCode(in_data,&bcdCode[0]);
        
        if(0 != bcdCode[4])
        {
            out_string[0] = pbc_acsii_table[bcdCode[4]];
            out_string[1] = pbc_acsii_table[bcdCode[3]];
            out_string[2] = pbc_acsii_table[bcdCode[2]];
            out_string[3] = pbc_acsii_table[bcdCode[1]];
            out_string[4] = pbc_acsii_table[bcdCode[0]];
        }
        else if(0 != bcdCode[3])
        {
            out_string[0] = pbc_acsii_table[bcdCode[3]];
            out_string[1] = pbc_acsii_table[bcdCode[2]];
            out_string[2] = pbc_acsii_table[bcdCode[1]];
            out_string[3] = pbc_acsii_table[bcdCode[0]];
        }
        else if(0 != bcdCode[2])
        {
            out_string[0] = pbc_acsii_table[bcdCode[2]];
            out_string[1] = pbc_acsii_table[bcdCode[1]];
            out_string[2] = pbc_acsii_table[bcdCode[0]];
        }
        else if(0 != bcdCode[1])
        {
            out_string[0] = pbc_acsii_table[bcdCode[1]];
            out_string[1] = pbc_acsii_table[bcdCode[0]];
        }
        else
        {
            out_string[0] = pbc_acsii_table[bcdCode[0]];
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void pbc_string_append(sdt_string* in_pAppend,sdt_string* out_string)
{
    sdt_int8u i = 0;
    sdt_int8u src_index = 0;
    
    while(out_string[src_index])
    {
        src_index++;
    }
    while(in_pAppend[i])
    {
        out_string[src_index++] = in_pAppend[i++];
    }
    out_string[src_index] = 0;//end add \0
}
//------------------------------------------------------------------------------
//定位字符在字符串的位置
//-1不在字符串内
//------------------------------------------------------------------------------
sdt_int16s pbc_string_seek(sdt_string* in_string,sdt_string in_seek_chat)
{
    sdt_int16s i = 0;
    
    while(1)
    {
        if(in_string[i] == 0)
        {
            return(-1);
        }
        else if(in_string[i] == in_seek_chat)
        {
            return(i);
        }
        i ++;
    }
}
//------------------------------------------------------------------------------
//字符串长度
sdt_int16s pbc_string_len(sdt_string* in_string)
{
    sdt_int16s i = 0;
    while(1)
    {
        if(in_string[i] == 0)
        {
            return(i);
        }
        else
        {
            i++;
        }
    }
}
//------------------------------------------------------------------------------
//字符串装HEX格式数据,one byte
//-1 非数字字符串
//------------------------------------------------------------------------------
sdt_int8s pbc_string_hex_byte(sdt_string in_string)
{
    if(0x30 == in_string)
    {
        return(0);
    }
    else if(0x31 == in_string)
    {
        return(1);
    }
    else if(0x32 == in_string)
    {
        return(2);
    }
    else if(0x33 == in_string)
    {
        return(3);
    }
    else if(0x34 == in_string)
    {
        return(4);
    }
    else if(0x35 == in_string)
    {
        return(5);
    }
    else if(0x36 == in_string)
    {
        return(6);
    }
    else if(0x37 == in_string)
    {
        return(7);
    }
    else if(0x38 == in_string)
    {
        return(8);
    }
    else if(0x39 == in_string)
    {
        return(9);
    }
    else
    {
        return(-1);
    }
}
//------------------------------------------------------------------------------
sdt_bool pbc_string_to_int16u(sdt_string* in_string,sdt_int16u* out_pHex)
{
    sdt_int16s slen = 0;
    sdt_int16u hex_data = 0;
    sdt_int16u mul = 1;
    sdt_int8s one_hex;
    
    slen = pbc_string_len(in_string);
    if(0 == slen)
    {
        return(sdt_false);
    }
    
    while(1)
    {
        slen --;
        one_hex = pbc_string_hex_byte(in_string[slen]);
        if(-1 == one_hex)
        {
            return(sdt_false);
        }
        hex_data += (sdt_int16u)one_hex * mul;
        if(0 == slen)
        {
            *out_pHex = hex_data;
            return(sdt_true);
        }
        else
        {
            mul = mul *10;
        }        
    }    
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_int16u pbc_arrayToInt16u_bigEndian(sdt_int8u* in_data)
{
    sdt_int16u the_data;
    
    the_data = in_data[0];
    the_data = the_data<<8;
    the_data |= in_data[1];
    return(the_data);
}
//-------------------------------------------------------------------------------------------------
sdt_int32u pbc_arrayToInt32u_bigEndian(sdt_int8u* in_data)
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
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void pbc_int16uToArray_bigEndian(sdt_int16u in_data,sdt_int8u* out_array)
{
    out_array[0] = in_data >> 8;
    out_array[1] = in_data;
}
//-------------------------------------------------------------------------------------------------
void pbc_int32uToArray_bigEndian(sdt_int32u in_data,sdt_int8u* out_array)
{
    out_array[0] = in_data >> 24;
    out_array[1] = in_data >> 16;
    out_array[2] = in_data >> 8;
    out_array[3] = in_data;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//16bit的冒泡排序算法
//-------------------------------------------------------------------------------------------------
void bubble_sort_16bits(sdt_int16u* mix_buff,sdt_int16u in_length)
{
    sdt_int16u i,j;
    
    for(i = 0; i < (in_length - 1); i++)
    {
        for(j = 0;j < (in_length -1 - i);j++)
        {
            sdt_int16u temp;
            
            if(mix_buff[j] > mix_buff[j+1])
            {
                temp = mix_buff[j+1];
                mix_buff[j+1] = mix_buff[j];
                mix_buff[j] = temp;
            }
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++end+++++++++++++++++++++++++++++++++++++++++++++++++++++
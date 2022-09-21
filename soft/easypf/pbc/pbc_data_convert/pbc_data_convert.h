#ifndef pbc_data_convert_H
#define pbc_data_convert_H
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef snail_data_types
    #include ".\snail_data_types.h"
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 16位数据转换成字符串
//功能:
//入口: 
//      in_data  ---16位的shu
//      out_string  字符串指针
//
//出口: 
//-------------------------------------------------------------------------------------------------
void pbc_clear_string(sdt_string* out_string,sdt_int16u in_length);
void pbc_int16u_to_string_convert(sdt_int16u in_data,sdt_string* out_string);
void pbc_int16s_to_string_convert(sdt_int16s in_data,sdt_string* out_string);
sdt_bool pbc_string_to_int16u(sdt_string* in_string,sdt_int16u* out_pHex);
sdt_int16s pbc_string_seek(sdt_string* in_string,sdt_string in_seek_chat);
//-------------------------------------------------------------------------------------------------
void pbc_string_append(sdt_string* in_pAppend,sdt_string* out_string);
//-------------------------------------------------------------------------------------------------
sdt_int16u pbc_arrayToInt16u_bigEndian(sdt_int8u* in_data);
//-------------------------------------------------------------------------------------------------
sdt_int32u pbc_arrayToInt32u_bigEndian(sdt_int8u* in_data);
//-------------------------------------------------------------------------------------------------
void pbc_int16uToArray_bigEndian(sdt_int16u in_data,sdt_int8u* out_array);
//-------------------------------------------------------------------------------------------------
void pbc_int32uToArray_bigEndian(sdt_int32u in_data,sdt_int8u* out_array);
//-------------------------------------------------------------------------------------------------
void bubble_sort_16bits(sdt_int16u* mix_buff,sdt_int16u in_length);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
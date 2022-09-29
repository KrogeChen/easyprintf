# module
最小版存储模块,带版本信息,方便存储区的升级管理
适用于8位机的存储模块
采用map+information的方式,data size = INFOR.data(n byte)
struct:
|-----------------------------------------------------------------------|
|                    MAP(FIX-16B)                       |  INFOR(VAR)   |
|map tag|  cnt |inf ver|inf size|reserve|  id  |checksum| data |checksum|
|1 byte |2 byte|1 byte | 1 byte |2 byte |8 byte| 1 byte |n byte| 1 byte |
|-----------------------------------------------------------------------|
map tag
bit1 bit0 map version
bit2 bit3 update tag
bit4-bit7 reserve
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define UPTAG_NONE               0x00
#define UPTAG_NEEDUP             0x01 


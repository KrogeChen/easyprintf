//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef APP_CFG_H
#define APP_CFG_H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define device_mark   0x01
//------------------------------------------------------------------------------
#define HARDWARE_FLAG  0x45410004
//------------------------------------------------------------------------------
#define MAIN_VERSION  0x01
#define SON_VERSION   0x00
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include "..\.\pbc\pbc_tick_small\pbc_tick_small.h"
#include "..\.\pbc\pbc_data_convert\pbc_data_convert.h"
////----------------------------------------------------------------------------
#include "..\.\mde\mde_pilot_light\mde_pilot_light.h"
#include "..\.\mde\mde_epf_accpet\mde_epf_accpet.h"
#include "..\.\mde\mde_rmcd_char\mde_rmcd_char.h"
#include "..\.\mde\mde_watchdog\mde_watchdog.h"
#include "..\.\mde\mde_storage_small\mde_storage_small.h"
#include "..\.\mde\mde_mbus_link\mde_mbus_link.h"
//
//#include "..\.\mde\mde_dc_motor\mde_dc_motor.h"
//#include "..\.\mde\mde_pt1000\mde_pt1000.h"

//#include "..\.\mde\mde_backlight\mde_backlight.h"
//#include "..\.\mde\mde_keyboard\mde_keyboard.h"
//#include "..\.\mde\mde_surface\mde_surface.h"
//#include "..\.\mde\mde_fancoilunit\mde_fancoilunit.h"
//#include "..\.\mde\mde_thermistor\mde_thermistor.h"

//#include "..\.\mde\mde_sx1278phy\mde_sx1278phy.h"
//#include "..\.\mde\mde_blockstorage_32bit\mde_blockstorage_32bit.h"
//
////----------------------------------------------------------------------------
#include ".\app_epf_message.h"
#include ".\app_mbus.h"
#include ".\app_protocol_easyink.h"
//#include ".\app_storage.h"
//#include ".\app_general.h"
//#include ".\app_key.h"
//#include ".\app_storage.h"
//#include ".\app_thermosta.h"
//#include ".\app_wireless.h"

//------------------------------------------------------------------------------
//采用共享内存的方式
//------------------------------------------------------------------------------
extern sdt_int8u g_share_buff[128];
#endif //APP_CFG_H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
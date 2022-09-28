//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef APP_CFG_H
#define APP_CFG_H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
#define  HARDWARE_MARK   0x45410004
//------------------------------------------------------------------------------
#define  MAIN_VERSION    0x21
#define  SON_VERSION     0x01
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include "..\.\pbc\pbc_tick_small\pbc_tick_small.h"
#include "..\.\pbc\pbc_data_convert\pbc_data_convert.h"
////----------------------------------------------------------------------------
#include "..\.\mde\mde_watchdog\mde_watchdog.h"
#include "..\.\mde\mde_storage_small\mde_storage_small.h"
#include "..\.\mde\mde_update_files\mde_update_files.h"
#include "..\.\mde\mde_mbus_link\mde_mbus_link.h"
////----------------------------------------------------------------------------
#include ".\app_protocol_easyink.h"
#include ".\app_mbus.h"
#include ".\app_bootload.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif //APP_CFG_H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
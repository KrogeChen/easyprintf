//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef APP_BOOTLOAD_H
#define APP_BOOTLOAD_H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef snail_data_types
    #include ".\depend\snail_data_types.h"
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//task++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_bootload_task(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//interface+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:驻留升级运行
//------------------------------------------------------------------------------
void app_bss_reside_update(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//功能:跳转到用户区运行
//------------------------------------------------------------------------------
void app_bss_goto_userApp(void);
//end+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
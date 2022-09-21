//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "chip_select.h"
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if defined(stm32f103rct6)

#elif defined(STM8L052C6T6)
#include ".\bsp_pilot_light_stm8l052c6t6.h"
#elif defined(HC32L136K8TA)
#include ".\bsp_pilot_light_hc32l136k8ta.h"
#elif defined(hc32l176kata)

#elif defined(fm33lc025n)
#include ".\bsp_pilot_light_fm33lc025n.h"
#elif defined(stc8g1k08)
#include ".\bsp_pilot_light_stc8g1k08.h"
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "chip_select.h"
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if defined(STM8L052C6T6)
#include ".\bsp_mbus_link_stm8l052c6t6.h"

#elif defined(HC32L136K8TA)
#include ".\bsp_mbus_link_hc32l136k8ta.h"
#elif defined(hc32l176kata)
#include ".\bsp_mbus_link_hc32l176kata.h"
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
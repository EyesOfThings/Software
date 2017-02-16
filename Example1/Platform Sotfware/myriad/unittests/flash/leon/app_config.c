#include "app_config.h"

#include <DrvCpr.h>

#define CMX_CONFIG_SLICE_7_0       (0x11111111)
#define CMX_CONFIG_SLICE_15_8      (0x11111111)

CmxRamLayoutCfgType  __attribute__((section(".cmx.ctrl")))
__cmx_config = {CMX_CONFIG_SLICE_7_0, CMX_CONFIG_SLICE_15_8};

int initClocksAndMemory(void)
{
    DrvCprInit();
    return 0;
}

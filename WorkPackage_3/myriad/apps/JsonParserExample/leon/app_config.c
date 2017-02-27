///
/// @brief    Application configuration Leon file
///

// 1: Includes
// ----------------------------------------------------------------------------
#include <OsDrvCpr.h>
#include <DrvShaveL2Cache.h>
#include <mv_types.h>
#include "app_config.h"

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define CMX_CONFIG_SLICE_7_0       (0x11111111)
#define CMX_CONFIG_SLICE_15_8      (0x11111111)

#define L2CACHE_CFG     (SHAVE_L2CACHE_NORMAL_MODE)

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
CmxRamLayoutCfgType  __attribute__((section(".cmx.ctrl"))) __cmx_config = {CMX_CONFIG_SLICE_7_0, CMX_CONFIG_SLICE_15_8};

// 4: Static Local Data
// ----------------------------------------------------------------------------

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------


// 6: Functions Implementation
// ----------------------------------------------------------------------------

// Setup all the clock configurations needed by this application and also the DDR
int initClocksAndMemory(void)
{
    u32 sc;

    tyAuxClkDividerCfg auxClkAllOn[] =
    {
        {AUX_CLK_MASK_UART, CLK_SRC_REFCLK0, 96, 625},   // Give the UART an SCLK that allows it to generate an output baud rate of of 115200 Hz (the uart divides by 16)
        {0,0,0,0}, // Null Terminated List
    };

    // Configure the system
    sc = OsDrvCprInit();
    if(sc)
        return sc;
    sc = OsDrvCprOpen();
    if(sc)
        return sc;
    sc = OsDrvCprAuxClockArrayConfig(auxClkAllOn);
    if(sc)
        return sc;
    // Set the shave L2 Cache mode
    sc = DrvShaveL2CacheSetMode(L2CACHE_CFG);
    if(sc)
        return sc;

    return 0;
}

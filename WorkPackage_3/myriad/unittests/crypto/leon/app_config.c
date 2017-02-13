///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Application configuration Leon file
///
#include "app_config.h"

#include <DrvCpr.h>

#define CMX_CONFIG_SLICE_7_0       (0x11111111)
#define CMX_CONFIG_SLICE_15_8      (0x11111111)

CmxRamLayoutCfgType  __attribute__((section(".cmx.ctrl")))
__cmx_config = {CMX_CONFIG_SLICE_7_0, CMX_CONFIG_SLICE_15_8};

int initClocksAndMemory(void)
{
    tyAuxClkDividerCfg auxClkAllOn[] = {
        // Give the UART an SCLK that allows it to generate an output baud rate of of 115200 Hz (the uart divides by 16)
        {AUX_CLK_MASK_UART, CLK_SRC_REFCLK0, 96, 625},
        {0,0,0,0}, // Null Terminated List
    };
    tySocClockConfig appClockConfig600_266 = {
            .refClk0InputKhz = 12000,       // Default 12Mhz input clock
            .refClk1InputKhz = 0,           // Assume no secondary oscillator for now
            .targetPll0FreqKhz = 266000,
            .targetPll1FreqKhz = 0,         // set in DDR driver
            .clkSrcPll1 = CLK_SRC_REFCLK0,  // Supply both PLLS from REFCLK0
            .masterClkDivNumerator = 1,
            .masterClkDivDenominator = 1,
            .cssDssClockEnableMask = DEFAULT_CORE_CSS_DSS_CLOCKS,
            .mssClockEnableMask = 0,        // Not enabling any MSS clocks for now
            .upaClockEnableMask = UPA_SHAVE_L2,
            .pAuxClkCfg = auxClkAllOn
    };
    DrvCprInit();
    DrvCprSetupClocks(&appClockConfig600_266);
    return 0;
}

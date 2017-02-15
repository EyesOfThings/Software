/**
 * @brief    Application configuration Leon file
 */
#include "app_config.h"
#include <registersMyriad.h>
#include <DrvRegUtils.h>
#include <DrvShaveL2Cache.h>
#include <DrvDdr.h>
#include <OsDrvCpr.h>
#include <OsDrvTimer.h>

#define CMX_CONFIG_SLICE_7_0       (0x11111111)
#define CMX_CONFIG_SLICE_15_8      (0x11111111)

CmxRamLayoutCfgType  __attribute__((section(".cmx.ctrl")))
__cmx_config = {CMX_CONFIG_SLICE_7_0, CMX_CONFIG_SLICE_15_8};

// Setup all clock configurations needed by the application and also the DDR.
int initClocksAndMemory(void)
{
    tyAuxClkDividerCfg auxClkAllOn[] = {
        // UART: 1.84MHz
        // Give the UART an SCLK
        {
            .auxClockEnableMask     = AUX_CLK_MASK_UART,
            .auxClockSource         = CLK_SRC_REFCLK0, // 12Mhz
            .auxClockDivNumerator   = 96,
            .auxClockDivDenominator = 625,
        },
        //// I2S0: 1.536MHz
        //// Give the I2S0 an SCLK
        {
            .auxClockEnableMask     = AUX_CLK_MASK_I2S0,
            .auxClockSource         = CLK_SRC_REFCLK0,
            .auxClockDivNumerator   = 32,
            .auxClockDivDenominator = 250
        },
        // GPIO1: 12MHz
        {
            .auxClockEnableMask     = AUX_CLK_MASK_GPIO1,
            .auxClockSource         = CLK_SRC_REFCLK0,
            .auxClockDivNumerator   = 0,
            .auxClockDivDenominator = 0,
            //.auxClockSource         = CLK_SRC_REFCLK0,
            //.auxClockDivNumerator   = 32,
            //.auxClockDivDenominator = 125,
        },
        {0,0,0,0}, // Null Terminated List
    };

    // Configure the system
    int retVal = OsDrvCprInit();
    if (retVal != OS_MYR_DRV_SUCCESS)
        return retVal;

    retVal = OsDrvCprOpen();
    if (retVal != OS_MYR_DRV_SUCCESS)
        return retVal;

    retVal = OsDrvCprAuxClockArrayConfig(auxClkAllOn);
    if (retVal != OS_MYR_DRV_SUCCESS)
        return retVal;

    DrvDdrInitialise(NULL);

    return OS_MYR_DRV_SUCCESS;
}

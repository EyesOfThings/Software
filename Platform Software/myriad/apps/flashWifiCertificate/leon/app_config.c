///   
/// 
/// 
/// 

#include <stdio.h>
#include <string.h>
#include "DrvDdr.h"
#include "assert.h"
#include "DrvShaveL2Cache.h"
#include "OsDrvTimer.h"
#include "OsDrvCpr.h"
#include "DrvCDCEL.h"
#include "DrvGpioDefines.h"
#include "app_config.h"

#define CMX_CONFIG_SLICE_7_0       (0x11111111)
#define CMX_CONFIG_SLICE_15_8      (0x11111111)

CmxRamLayoutCfgType __attribute__((section(".cmx.ctrl"))) __cmx_config = {CMX_CONFIG_SLICE_7_0, CMX_CONFIG_SLICE_15_8};

static tyAuxClkDividerCfg auxClk[] ={
    {
        .auxClockEnableMask = AUX_CLK_MASK_LCD, // LCD clock
        .auxClockSource = CLK_SRC_SYS_CLK,
        .auxClockDivNumerator = 1,
        .auxClockDivDenominator = 1
    },
    {
        .auxClockEnableMask = AUX_CLK_MASK_MEDIA, // SIPP Clock
        .auxClockSource = CLK_SRC_SYS_CLK_DIV2, //
        .auxClockDivNumerator = 1, //
        .auxClockDivDenominator = 1, //
    },
    {
        .auxClockEnableMask = (AUX_CLK_MASK_CIF0 | AUX_CLK_MASK_CIF1), // CIFs Clock
        .auxClockSource = CLK_SRC_SYS_CLK, //
        .auxClockDivNumerator = 1, //
        .auxClockDivDenominator = 1, //
    },
    {
        .auxClockEnableMask = CLOCKS_MIPICFG, // MIPI CFG + ECFG Clock
        .auxClockSource = CLK_SRC_SYS_CLK, //
        .auxClockDivNumerator = 1, //
        .auxClockDivDenominator = 19, //  the MIPI cfg clock should be <= 20 Mhz !
    },
    {
        .auxClockEnableMask = AUX_CLK_MASK_SDIO, //
        .auxClockSource = CLK_SRC_PLL0, //
        .auxClockDivNumerator = 1, //
        .auxClockDivDenominator = 2, //
    },
    {
        .auxClockEnableMask = AUX_CLK_MASK_UART,
        .auxClockSource = CLK_SRC_REFCLK0, // 12Mhz
        .auxClockDivNumerator = 96,
        .auxClockDivDenominator = 625,
    },
    //// I2S0: 1.536MHz
    //// Give the I2S0 an SCLK
    {
        .auxClockEnableMask = AUX_CLK_MASK_I2S0,
        .auxClockSource = CLK_SRC_REFCLK0,
        .auxClockDivNumerator = 32,
        .auxClockDivDenominator = 250
    },
    // GPIO1: 12MHz
    {
        .auxClockEnableMask = AUX_CLK_MASK_GPIO1,
        .auxClockSource = CLK_SRC_REFCLK0,
        .auxClockDivNumerator = 0,
        .auxClockDivDenominator = 0,
    },
    {0, 0, 0, 0}, // Null Terminated List
};


int initClocksAndMemory(void) {
    int retVal, i;

    // Configure the system
    OsDrvCprInit();
    OsDrvCprOpen();
    OsDrvCprAuxClockArrayConfig(auxClk);

    DrvCprSysDeviceAction(MSS_DOMAIN, ASSERT_RESET, DEV_MSS_LCD | DEV_MSS_MIPI | DEV_MSS_CIF0 | DEV_MSS_CIF1 | DEV_MSS_SIPP);

    DrvCprSysDeviceAction(MSS_DOMAIN, DEASSERT_RESET, -1);
    DrvCprSysDeviceAction(CSS_DOMAIN, DEASSERT_RESET, -1);
    DrvCprSysDeviceAction(UPA_DOMAIN, DEASSERT_RESET, -1);

    //OsDrvTimerInit();
    DrvDdrInitialise(NULL);

    // Init I2C for the camera before starting the WiFi
    /*s32 boardStatus;
    boardStatus = BoardInitialise(EXT_PLL_CFG_148_24_24MHZ);
    if (boardStatus != B_SUCCESS) {
        printf("Error: board initialization failed with %d status\n", boardStatus);
        return -1;
    }*/

    return 0;
}

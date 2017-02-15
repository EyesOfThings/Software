#include <OsDrvCpr.h>
#include "app_config.h"

#define CMX_CONFIG_SLICE_7_0       (0x11111111)
#define CMX_CONFIG_SLICE_15_8      (0x11111111)

CmxRamLayoutCfgType __attribute__((section(".cmx.ctrl"))) __cmx_config = {
    CMX_CONFIG_SLICE_7_0,
    CMX_CONFIG_SLICE_15_8
};

// Setup all the clock configurations needed by this application and also the ddr
int initClocksAndMemory( void ) {
    tyAuxClkDividerCfg auxClkAllOn[] = {
        // Give the UART an SCLK that allows it to generate an output baud rate of of 115200 Hz (the uart divides by 16)
        { AUX_CLK_MASK_UART, CLK_SRC_REFCLK0, 96, 625 },
        // Null Terminated List
        { 0, 0, 0, 0 }
    };
    // Configure the system
    OsDrvCprInit();
    OsDrvCprOpen();
    OsDrvCprAuxClockArrayConfig( auxClkAllOn );
    return 0;
}

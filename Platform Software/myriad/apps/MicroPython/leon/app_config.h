///
/// @brief     Application configuration Leon header
///  

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// 1: Includes
// ----------------------------------------------------------------------------
#include "mv_types.h"

#define SYS_CLK_KHZ             (12000) // 12MHz
#define PLL_DESIRED_FREQ_KHZ    (480000)

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------

// The clocks on the 2100 are different, test if we are using it
#ifdef MA2100
#define MSS_CLOCKS     (  DEV_MSS_APB_SLV     |     \
                              DEV_MSS_APB2_CTRL   |     \
                              DEV_MSS_RTBRIDGE    |     \
                              DEV_MSS_RTAHB_CTRL  |     \
                              DEV_MSS_LRT         |     \
                              DEV_MSS_LRT_DSU     |     \
                              DEV_MSS_LRT_L2C     |     \
                              DEV_MSS_LRT_ICB     |     \
                              DEV_MSS_AXI_BRIDGE  |     \
                              DEV_MSS_MXI_CTRL    |     \
                              DEV_MSS_MXI_DEFSLV  |     \
                              DEV_MSS_AXI_MON     |     \
                              DEV_MSS_MIPI        |     \
                              DEV_MSS_CIF0        |     \
                              DEV_MSS_LCD         |     \
                              DEV_MSS_AMC         |     \
                              DEV_MSS_SIPP        |     \
                              DEV_MSS_TIM         |     \
                              DEV_MSS_SIPP_ABPSLV )
#else
#define MSS_CLOCKS     (  DEV_MSS_APB_SLV     |     \
                              DEV_MSS_APB2_CTRL   |     \
                              DEV_MSS_RTBRIDGE    |     \
                              DEV_MSS_RTAHB_CTRL  |     \
                              DEV_MSS_LRT         |     \
                              DEV_MSS_LRT_DSU     |     \
                              DEV_MSS_LRT_L2C     |     \
                              DEV_MSS_LRT_ICB     |     \
                              DEV_MSS_AXI_BRIDGE  |     \
                              DEV_MSS_MXI_CTRL    |     \
                              DEV_MSS_MXI_DEFSLV  |     \
                              DEV_MSS_AXI_MON     |     \
                              DEV_MSS_MIPI        |     \
                              DEV_MSS_CIF0        |     \
                              DEV_MSS_LCD         |     \
                              DEV_MSS_AMC         |     \
                              DEV_MSS_SIPP )
#endif

#define APP_UPA_CLOCKS  (DEV_UPA_SH0      | \
                         DEV_UPA_SH1      | \
                         DEV_UPA_SH2      | \
                         DEV_UPA_SH3      | \
                         DEV_UPA_SH4      | \
                         DEV_UPA_SH5      | \
                         DEV_UPA_SH6      | \
                         DEV_UPA_SHAVE_L2 | \
                         DEV_UPA_CDMA     | \
                         DEV_UPA_CTRL      )

#define APP_SIPP_CLOCKS ( DEV_SIPP_LUT      | \
                          DEV_SIPP_MIPI_RX1  )

//clock defines
#define CLOCKS_MIPICFG (AUX_CLK_MASK_MIPI_ECFG | AUX_CLK_MASK_MIPI_CFG)

// 3:  Exported Global Data (generally better to avoid)
// ----------------------------------------------------------------------------

// 4:  Exported Functions (non-inline)
// ----------------------------------------------------------------------------
/// Setup all the clock configurations needed by this application and also the ddr
///
/// @return    0 on success, non-zero otherwise
int initClocksAndMemory(void);

#endif // APP_CONFIG_H

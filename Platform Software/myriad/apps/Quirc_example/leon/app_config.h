///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Application configuration Leon header
///

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

// 1: Includes
// ----------------------------------------------------------------------------

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

#define DEFAULT_APP_CLOCK_KHZ		266000
#define DEFAULT_OSC_CLOCK_KHZ		12000
#define L2CACHE_NORMAL_MODE 		(0x6)  // In this mode the L2Cache acts as a cache for the DRAM
#define L2CACHE_CFG     		(L2CACHE_NORMAL_MODE)
#define BIGENDIANMODE   		(0x01000786)

#define CMX_CONFIG_SLICE_7_0       (0x11111111)
#define CMX_CONFIG_SLICE_15_8      (0x11111111)


// 3:  Exported Functions (non-inline)
// ----------------------------------------------------------------------------
/// Setup all the clock configurations needed by this application and also the ddr
///
/// @return    0 on success, non-zero otherwise
int initClocksAndMemory(void);
#endif // _APP_CONFIG_H_
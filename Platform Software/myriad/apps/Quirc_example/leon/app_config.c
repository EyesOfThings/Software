///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief    Application configuration Leon file
///

// 1: Includes
// ----------------------------------------------------------------------------
#include <registersMyriad.h>
#include <OsDrvCpr.h> //API for the Clock Power Reset Module.
#include "OsDrvTimer.h" //API for the Clock Power Reset Module.
#include <DrvRegUtils.h> //Macros and functions to ease register manipulation.
#include "DrvDdr.h" //API for the DRAM Driver.
#include "app_config.h"

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------


// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------


// 4: Static Local Data
// ----------------------------------------------------------------------------

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------


// 6: Functions Implementation
// ----------------------------------------------------------------------------
int initClocksAndMemory(void)
{
	int retVal;

	tyAuxClkDividerCfg auxClkAllOn[] =
	{
		{AUX_CLK_MASK_UART, CLK_SRC_REFCLK0, 96, 625}, // Give the UART an SCLK that allows it to generate an output baud rate of of 115200 Hz (the uart divides by 16)
		{0,0,0,0}, // Null Terminated List
	};

	//Configure the system
	OsDrvCprInit();
	OsDrvCprOpen();
	OsDrvCprAuxClockArrayConfig(auxClkAllOn);

	return 0;
}

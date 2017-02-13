///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     main leon file 
///
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mv_types.h>

#include "app_config.h"
#include "CryptoUnitTest.h"

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------
// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
// 4: Static Local Data
// ----------------------------------------------------------------------------
// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
// 6: Functions Implementation
// ----------------------------------------------------------------------------
int main(void)
{
    initClocksAndMemory();

    TestRunner_start();
    TestRunner_runTest( Crypto_test() );
    TestRunner_end();

    printf("\n");

    return 0;
}

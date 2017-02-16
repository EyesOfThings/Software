#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mv_types.h>

#include "app_config.h"
#include "FlashIOUnitTest.h"

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
    TestRunner_runTest( FlashFileOpen_test() );
    TestRunner_runTest( FlashFileWrite_test() );
    TestRunner_runTest( FlashFileRead_test() );
    TestRunner_runTest( FlashFileMisc_test() );
    TestRunner_runTest( FlashFileUseCase_test() );
    TestRunner_end();

    printf("\n");

    return 0;
}

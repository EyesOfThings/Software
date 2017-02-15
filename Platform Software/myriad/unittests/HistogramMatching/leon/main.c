#include <bsp.h>
#include <stdlib.h>
#include <stdio.h>
#include <rtems.h>

#include <mv_types.h>

#include "app_config.h"
#include "rtems_config.h"

#include "HistogramMatchingUnitTest.h"

void POSIX_Init(void* args)
{
    initClocksAndMemory();

	TestRunner_start();
    TestRunner_runTest( Histogram_test() );
    TestRunner_end();
    printf("\n");
    exit(0);
}

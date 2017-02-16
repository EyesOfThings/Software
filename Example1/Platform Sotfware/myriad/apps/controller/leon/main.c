#include "app_config.h"
#include "rtems_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mv_types.h>
#include <stdint.h>
#include <rtems.h>

#include <FlashIO.h>
#include <Crypto.h>
#include <SDCardIO.h>

void POSIX_Init(void *args)
{
    initClocksAndMemory();

    // 1. wireless communication
    // 2. enter event loop:
    // * event: upload and flash an application
    // * event: run application ( optional? )
    // * event: download data from SD card
    // * event: change network password
    // * event: remove network password
    // * event: request a camera snapshot
    // * event: update myriad clock/time

    exit( 0 );
}

// User extension to be able to catch abnormal terminations
void Fatal_extension(
  Internal_errors_Source  the_source,
  bool                    is_internal,
  uint32_t                the_error
)
{
    switch(the_source) {
    case RTEMS_FATAL_SOURCE_EXIT:
		if(the_error)
			printk("Exited with error code %d\n", the_error);
        break; // normal exit
    case RTEMS_FATAL_SOURCE_ASSERT:
        printk("%s : %d in %s \n",
               ((rtems_assert_context *)the_error)->file,
               ((rtems_assert_context *)the_error)->line,
               ((rtems_assert_context *)the_error)->function);
        break;
    case RTEMS_FATAL_SOURCE_EXCEPTION:
        rtems_exception_frame_print((const rtems_exception_frame *) the_error);
        break;
    default:
		printk ("\nSource %d Internal %d Error %d  0x%X:\n",
				the_source, is_internal, the_error, the_error);
        break;
    }
}

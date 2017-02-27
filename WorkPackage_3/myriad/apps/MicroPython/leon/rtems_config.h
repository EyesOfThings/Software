///
/// @brief     Application configuration Leon header
///

#ifndef _RTEMS_CONFIG_H_
#define _RTEMS_CONFIG_H_

// 1: Includes
// ----------------------------------------------------------------------------
#include <rtems.h>
#include "app_config.h"
#include <SDCardIORTEMSConfig.h>

// 2: Defines
// ----------------------------------------------------------------------------
#if defined(__RTEMS__)

#if !defined (__CONFIG__)
#define __CONFIG__

/* ask the system to generate a configuration table */
#define CONFIGURE_INIT

#ifndef RTEMS_POSIX_API
#define RTEMS_POSIX_API
#endif

#define CONFIGURE_MICROSECONDS_PER_TICK         1000    /* 1 millisecond */

#define CONFIGURE_TICKS_PER_TIMESLICE           10      /* 10 milliseconds */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_POSIX_INIT_THREAD_TABLE

#define CONFIGURE_MINIMUM_TASK_STACK_SIZE      16384

#define CONFIGURE_MAXIMUM_TASKS                 4

#define CONFIGURE_MAXIMUM_POSIX_THREADS         4

#define CONFIGURE_MAXIMUM_POSIX_MUTEXES         8

#define CONFIGURE_MAXIMUM_POSIX_KEYS            8

#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES      8

#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES  8

#define CONFIGURE_MAXIMUM_POSIX_TIMERS          4

#define CONFIGURE_MAXIMUM_TIMERS                4

#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES	4

/*
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_FILESYSTEM_DOSFS
#define CONFIGURE_MAXIMUM_DRIVERS 10
#define CONFIGURE_MAXIMUM_SEMAPHORES (4 * RTEMS_DOSFS_SEMAPHORES_PER_INSTANCE)
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 30
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_POSIX_INIT_TASKS_TABLE
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS  (16)
#define CONFIGURE_BDBUF_MAX_WRITE_BLOCKS       (64)
#define CONFIGURE_BDBUF_BUFFER_MIN_SIZE        (512)
#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE        (32 * 1024)
#define CONFIGURE_BDBUF_CACHE_MEMORY_SIZE      (4 * 1024 * 1024)
*/

// User extension to be able to catch abnormal terminations
static void Fatal_extension(
        Internal_errors_Source  the_source,
        bool                    is_internal,
        uint32_t                the_error
)
{
    switch(the_source)
    {
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

#define CONFIGURE_MAXIMUM_USER_EXTENSIONS    1
#define CONFIGURE_INITIAL_EXTENSIONS         { .fatal = Fatal_extension }

void POSIX_Init (void *args);

#include <rtems/confdefs.h>

#endif // __CONFIG__

// Set the system clocks at Startup
BSP_SET_CLOCK(SYS_CLK_KHZ, PLL_DESIRED_FREQ_KHZ, 1, 1,DEFAULT_CORE_CSS_DSS_CLOCKS,  MSS_CLOCKS,  APP_UPA_CLOCKS,  APP_SIPP_CLOCKS, 0x0);

// Set the L2C at startup
BSP_SET_L2C_CONFIG(0, L2C_REPL_LRU, 0, L2C_MODE_COPY_BACK, 0, NULL);

#endif // __RTEMS__

#endif // _RTEMS_CONFIG_H_

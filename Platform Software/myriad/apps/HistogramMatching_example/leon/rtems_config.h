#ifndef RTEMS_CONFIG_H_
#define RTEMS_CONFIG_H_

// 1: Includes
// ----------------------------------------------------------------------------
#include "app_config.h"

#if defined(__RTEMS__)

#if !defined (__CONFIG__)
#define __CONFIG__

// 2: Defines
// ----------------------------------------------------------------------------

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

#define  CONFIGURE_MINIMUM_TASK_STACK_SIZE      4096

#define CONFIGURE_MAXIMUM_TASKS                 4

#define CONFIGURE_MAXIMUM_POSIX_THREADS         4

#define CONFIGURE_MAXIMUM_POSIX_MUTEXES         8

#define CONFIGURE_MAXIMUM_POSIX_KEYS            8

#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES      8

#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES  8

#define CONFIGURE_MAXIMUM_POSIX_TIMERS          4

#define CONFIGURE_MAXIMUM_TIMERS                4


#define CONFIGURE_MAXIMUM_USER_EXTENSIONS    1
#define CONFIGURE_INITIAL_EXTENSIONS         { .fatal = Fatal_extension }

// 3:  Exported Global Data (generally better to avoid)
// ----------------------------------------------------------------------------
// 4:  Functions (non-inline)
// ----------------------------------------------------------------------------

static void Fatal_extension (
  Internal_errors_Source  the_source,
  bool                    is_internal,
  uint32_t                the_error
);
void POSIX_Init (void *args);

#include <rtems/confdefs.h>

#endif // __CONFIG__

#endif // __RTEMS__

// Program the booting clocks
// Programme the system clocks at Startup
BSP_SET_CLOCK(12000, 266000, 1, 1, DEFAULT_RTEMS_CSS_LOS_CLOCKS, DEFAULT_RTEMS_MSS_LRT_CLOCKS, UPA_SHAVE_L2, 0, 0);

// Programme the  L2C at startup
BSP_SET_L2C_CONFIG(0, L2C_REPL_LRU, 0, L2C_MODE_COPY_BACK, 0, NULL);

#endif // LEON_RTEMS_CONFIG_H_

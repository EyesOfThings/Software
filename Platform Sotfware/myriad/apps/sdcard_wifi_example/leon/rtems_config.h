#ifndef LEON_RTEMS_CONFIG_H_
#define LEON_RTEMS_CONFIG_H_

#ifndef _RTEMS_CONFIG_H_
#define _RTEMS_CONFIG_H_

#include <OsDrvCpr.h>

#if defined(__RTEMS__)

#if !defined (__CONFIG__)
#define __CONFIG__

/* ask the system to generate a configuration table */
#define CONFIGURE_INIT

#ifndef RTEMS_POSIX_API
#define RTEMS_POSIX_API
#endif

#define CONFIGURE_MICROSECONDS_PER_TICK             1000 /* 1 millisecond */
#define CONFIGURE_TICKS_PER_TIMESLICE               50   /* 50 milliseconds */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_POSIX_INIT_THREAD_TABLE
#define  CONFIGURE_MINIMUM_TASK_STACK_SIZE           (8192)
#define CONFIGURE_MAXIMUM_TASKS                      20
#define CONFIGURE_MAXIMUM_POSIX_THREADS              5
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES              8
#define CONFIGURE_MAXIMUM_POSIX_KEYS                 8
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES           8
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES       8
#define CONFIGURE_MAXIMUM_POSIX_TIMERS               4
#define CONFIGURE_MAXIMUM_TIMERS                     4

void POSIX_Init( void *args );

static void Fatal_extension( Internal_errors_Source the_source,
bool is_internal,
                             uint32_t the_error );

#define CONFIGURE_MAXIMUM_USER_EXTENSIONS    1

#define CONFIGURE_INITIAL_EXTENSIONS         { .fatal = Fatal_extension }

#include <SDCardIORTEMSConfig.h>
#include <rtems/confdefs.h>

#endif // __CONFIG__

#endif // __RTEMS__

// Set the system clocks
BSP_SET_CLOCK( DEFAULT_REFCLOCK, 200000, 1, 1,
               DEFAULT_RTEMS_CSS_LOS_CLOCKS, DEFAULT_RTEMS_MSS_LRT_CLOCKS,
               0, 0, 0 );
// Set L2 cache behaviour
BSP_SET_L2C_CONFIG( 1, DEFAULT_RTEMS_L2C_REPLACEMENT_POLICY,
                    DEFAULT_RTEMS_L2C_WAYS, DEFAULT_RTEMS_L2C_MODE, 0, 0 );

#endif // _RTEMS_CONFIG_H_

#endif // LEON_RTEMS_CONFIG_H_

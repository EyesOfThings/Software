#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rtems.h>

#include <semaphore.h>
#include <pthread.h>
#include <sched.h>
#include <fcntl.h>
#include <mv_types.h>
#include <rtems/cpuuse.h>
#include <bsp.h>

#include <DrvLeon.h>
#include <DrvLeonL2C.h>
#include <DrvShaveL2Cache.h>
#include <DrvTimer.h>
#include <OsDrvCpr.h>
#include <OsDrvTimer.h>
#include "OsDrvSdio.h"

#include "rtems_config.h"
#include "WifiFunctions.h"
#include "PulgaMQTTBrokerControl.h"



// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// 4: Static Local Data
// ----------------------------------------------------------------------------
/* Sections decoration is require here for downstream tools */
static int rc1;
static pthread_t thread1;
static sem_t sem;


// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
void *wifi_thread(void *arg);
//extern connectToAP(int, char**);

// 6: Functions Implementation
// ----------------------------------------------------------------------------

void POSIX_Init (void *args)
{
    int result;
    pthread_attr_t attr;

    initClocksAndMemory();

    // AP
    generateAPFromProfileOnErrorDefault(0);

    // Pulga
    pulgamqttbroker *pulga = pulga_create();
    pulga_start(pulga);




    /*printk ("\n");
    printk ("RTEMS connectToAP started\n");


    if(pthread_attr_init(&attr) !=0) {
        printk("pthread_attr_init error");
    }
    if(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
        printk("pthread_attr_setinheritsched error");
    }
    if(pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0) {
        printk("pthread_attr_setschedpolicy error");
    }

    if(sem_init(&sem, 0, 0) == -1) {
        printk("sem_init error\n");
    }

    if ((rc1=pthread_create(&thread1, &attr, &wifi_thread, NULL))) {
        printk("Thread 1 creation failed: %d\n", rc1);
    }
    else {
        printk("Thread 1 created\n");
    }*/

    // wait for thread to finish
    /*result = pthread_join( thread1, NULL);
    if(result != 0) {
        printk("pthread_join error (%d)!\n", result);
    }*/

    exit(0);
    return;
}


void *wifi_thread(void *arg)
{
    pulgamqttbroker *pulga = pulga_create();
    

    //start from pofile
    generateAPFromProfileOnErrorDefault(0);
    //generateAP("Myriad2Wifi", "visilabap", 2, 8);
    
    //clear profiles
    //removeProfiles();
    //generateAPFromProfileOnErrorDefault(0);
    
    //set power of AP
    //setWlanPower(8);
    pulga_start(pulga);

    pthread_exit(0);
}



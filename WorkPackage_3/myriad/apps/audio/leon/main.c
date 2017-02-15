#include <bsp.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <rtems.h>
#include <sched.h>

#include <mv_types.h>
#include <DrvCpr.h>
#include <DrvTimer.h>
#include <DrvGpio.h>

#include "app_config.h"
#include "rtems_config.h"

#include <stdbool.h>
#include <mv_types.h>

#include <Audio.h>

void* main(void* arg);

static pthread_attr_t mainThreadAttr;
static pthread_t mainThread;

void POSIX_Init(void* args) {

    initClocksAndMemory();
    if (pthread_attr_init(&mainThreadAttr)) {
        exit(0);
    }
    if (pthread_attr_setinheritsched(&mainThreadAttr, PTHREAD_EXPLICIT_SCHED)) {
        exit(0);
    }
    if (pthread_attr_setschedpolicy(&mainThreadAttr, SCHED_RR)) {
        exit(0);
    }
    if (pthread_create(&mainThread, &mainThreadAttr, main, NULL)) {
        exit(0);
    }
    if (pthread_join(mainThread, NULL)) {
        exit(0);
    }
    exit(0);
}

void* main(void* arg) {
    AudioPlay("/mnt/sdcard/audioExample.wav");
    AudioSetVolume(40);
    printf("playing...\n");
    while (true) {
        // stop playing after 30 seconds
        if(AudioGetPosition() > 30000.0f) {
            AudioStop();
        }
        if (AudioIsStopped()) {
            break;
        }
    }
    AudioRecord("/mnt/sdcard/audioRecord.wav");
    printf("recording...\n");
    while (true) {
        // stop recording after 10 seconds
		if(AudioGetPosition() > 10000.0f) {
		    AudioStop();
        }
        if (AudioIsStopped()) {
            break;
        }
    }
    AudioPlay("/mnt/sdcard/audioRecord.wav");
    AudioSetVolume(100);
    printf("playing...\n");
    while (true) {
        if (AudioIsStopped()) {
            break;
        }
    }

    pthread_exit(0);
}

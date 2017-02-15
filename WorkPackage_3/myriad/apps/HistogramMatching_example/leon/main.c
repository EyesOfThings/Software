// 1: Includes
// ----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <rtems.h>
#include "rtems_config.h"
#include <swcFrameTypes.h>
#include "ColorConversion.h"
#include "ColorHistogram.h"
#include "HistogramMatching.h"

//4: Static Local Data
// ----------------------------------------------------------------------------
static pthread_t mainThread;

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
void main(void*);

// 6: Functions Implementation
// ----------------------------------------------------------------------------
void POSIX_Init(void* args)
{
    pthread_attr_t attr;
    int sc = initClocksAndMemory();
    if (sc)
        exit(sc);

    sc = pthread_attr_init(&attr);
    if (sc)
    {
        printk("pthread_attr_init error");
        exit(sc);
    }
    sc = pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
    if (sc)
    {
        printk("pthread_attr_setinheritsched error");
        exit(sc);
    }
    sc = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    if (sc)
    {
        printk("pthread_attr_setschedpolicy error");
        exit(sc);
    }

    int rc = pthread_create(&mainThread, &attr, (void*) &main, NULL);
    if (rc)
    {
        printk("Thread 1 creation failed: %d\n", rc);
        exit(rc);
    }

    sc = pthread_join(mainThread, NULL);
    if (sc)
    {
        printk("pthread_join error!");
        exit(sc);
    }

    exit(0);
}

void main(void* args)
{
    // create two images with 1 row and 4 columns
    frameSpec specs = {RGB888, 1, 4, 4, 1};

    frameBuffer imageOne = {specs, NULL, NULL, NULL};
    imageOne.p1 = malloc(5);
    imageOne.p2 = malloc(5);
    imageOne.p3 = malloc(5);

    frameBuffer imageTwo = {specs, NULL, NULL, NULL};
    imageTwo.spec.type = NONE;
    imageTwo.p1 = malloc(4);
    imageTwo.p2 = malloc(4);
    imageTwo.p3 = malloc(4);

    // fill images with some data
    imageOne.p1[0] = 255;
    imageOne.p2[0] = 16;
    imageOne.p3[0] = 13;
    imageOne.p1[1] = 25;
    imageOne.p2[1] = 245;
    imageOne.p3[1] = 83;
    imageOne.p1[2] = 0;
    imageOne.p2[2] = 21;
    imageOne.p3[2] = 250;
    imageOne.p1[3] = 0;
    imageOne.p2[3] = 255;
    imageOne.p3[3] = 0;

    imageTwo.p1[0] = 137;
    imageTwo.p2[0] = 207;
    imageTwo.p3[0] = 192;
    imageTwo.p1[1] = 217;
    imageTwo.p2[1] = 49;
    imageTwo.p3[1] = 190;
    imageTwo.p1[2] = 84;
    imageTwo.p2[2] = 202;
    imageTwo.p3[2] = 24;
    imageTwo.p1[3] = 0;
    imageTwo.p2[3] = 0;
    imageTwo.p3[3] = 255;

    // convert imageOne into CIE Lab color space (in-place)
    convertColor(&imageOne, &imageOne, COLOR_RGB2Lab_D65);

    for (int i = 0; i < 4; ++i)
    {
        printf("Image 1: %i %i %i\n",
               imageOne.p1[i],
               imageOne.p2[i],
               imageOne.p3[i]);
        printf("Image 2: %i %i %i\n",
               imageTwo.p1[i],
               imageTwo.p2[i],
               imageTwo.p3[i]);
    }

    u8 bins[3] = {8, 8, 8};
    int* histogramOne = malloc((8+8+8) * sizeof(int));
    int* histogramTwo = malloc((8+8+8) * sizeof(int));

    // create histograms
    computeHistogram(&imageOne, histogramOne, bins);
    computeHistogram(&imageTwo, histogramTwo, bins);

    // compare histograms
    float d = hellingerDistance(24, histogramOne, histogramTwo);

    printf("Hellinger distance: %f\n", d);

    d = histogramIntersectionDistance(24, histogramOne, histogramTwo);

    printf("Histogram intersection distance: %f\n", d);
}

// User extension to be able to catch abnormal terminations
static void Fatal_extension(Internal_errors_Source the_source,
                            bool is_internal,
                            uint32_t the_error)
{
    switch (the_source)
    {
    case RTEMS_FATAL_SOURCE_EXIT:
        if (the_error)
            printk("Exited with error code %d\n", the_error);
        break; // normal exit
    case RTEMS_FATAL_SOURCE_ASSERT:
        printk("%s : %d in %s \n",
               ((rtems_assert_context * )the_error)->file,
               ((rtems_assert_context * )the_error)->line,
               ((rtems_assert_context * )the_error)->function);
        break;
    case RTEMS_FATAL_SOURCE_EXCEPTION:
        rtems_exception_frame_print((const rtems_exception_frame *) the_error);
        break;
    default:
        printk("\nSource %d Internal %d Error %d  0x%X:\n",
               the_source,
               is_internal, the_error, the_error);
        break;
    }
}

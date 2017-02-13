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

#include "rtems_config.h"

//#include "WifiFunctions.h"
//#include "PulgaMQTTBrokerControl.h"

#include <SDCardIO.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>







// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
static int rc1;
static pthread_t thread1;
static sem_t sem;


// 4: Static Local Data
// ----------------------------------------------------------------------------
void *mainFunction(void *arg);

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------

// 6: Functions Implementation
// ----------------------------------------------------------------------------


void POSIX_Init (void *args)
{
    int result;
    pthread_attr_t attr;

    initClocksAndMemory();
    printk ("\n");
    printk ("RTEMS connectToAP started\n");  /* initialise variables */


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

    if ((rc1=pthread_create(&thread1, &attr, &mainFunction, NULL))) {
        printk("Thread 1 creation failed: %d\n", rc1);
    }
    else {
        printk("Thread 1 created\n");
    }

    // wait for thread to finish
    result = pthread_join( thread1, NULL);
    if(result != 0) {
        printk("pthread_join error (%d)!\n", result);
    }

    exit(0);
    return;
}

void myassert(int a, int b){
    if(a==b){
        printf("Test passed: OK\n");
    }else{
        printf("Test passed: Error\n");
    }
}


//lkdemo.c
int testLKdemo() {
    IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp;

    int win_size = 10;
    const int MAX_COUNT = 500;
    CvPoint2D32f * points[2] = {0, 0}, *swap_points;
    char* status = 0;
    int count = 0;
    int need_to_init = 1;
    int night_mode = 0;
    int flags = 0;
    int add_remove_pt = 0;
    CvPoint pt;

    int current_frame = 0;
    int run_algorithm = 1;

    while (run_algorithm) {

        IplImage* frame = 0;
        int i, k, c;

        
        char* filename = (char*)malloc(150*sizeof(char));
        sprintf(filename,"/mnt/sdcard/OpticalFlowLK_OpenCVTests/data/frame%d.png",current_frame);
        
        frame = cvLoadImage(filename, 1);
        //if (!frame)
        //    break;

        if (!image) {
            //printf("1\n");
            /* allocate all the buffers */
            image = cvCreateImage(cvGetSize(frame), 8, 3);
            image->origin = frame->origin;
            grey = cvCreateImage(cvGetSize(frame), 8, 1);
            prev_grey = cvCreateImage(cvGetSize(frame), 8, 1);
            pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
            prev_pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
            points[0] = (CvPoint2D32f*) cvAlloc(MAX_COUNT * sizeof (points[0][0]));
            points[1] = (CvPoint2D32f*) cvAlloc(MAX_COUNT * sizeof (points[0][0]));
            status = (char*) cvAlloc(MAX_COUNT);
            flags = 0;
        }

        cvCopy(frame, image, 0);
        cvCvtColor(image, grey, CV_BGR2GRAY);

        struct timeval currentTime;
        struct timeval previousTime;
        int previousUsec = 0;
        int currentUsec = 0;
        IplImage* src;
        IplImage* dst;
        int res1;


        gettimeofday(&previousTime, NULL);
        previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;
        
        if (night_mode)
            cvZero(image);

        if (need_to_init) {
            //printf("2\n");
            /* automatic initialization */
            IplImage* eig = cvCreateImage(cvGetSize(grey), 32, 1);
            IplImage* temp = cvCreateImage(cvGetSize(grey), 32, 1);
            double quality = 0.01;
            double min_distance = 10;

            count = MAX_COUNT;
            cvGoodFeaturesToTrack(grey, eig, temp, points[1], &count,
                    quality, min_distance, 0, 3, 0, 0.04);
            cvFindCornerSubPix(grey, points[1], count,
                    cvSize(win_size, win_size), cvSize(-1, -1),
                    cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
            cvReleaseImage(&eig);
            cvReleaseImage(&temp);

            add_remove_pt = 0;
        } else if (count > 0) {
            //printf("3\n");
            cvCalcOpticalFlowPyrLK(prev_grey, grey, prev_pyramid, pyramid,
                    points[0], points[1], count, cvSize(win_size, win_size), 3, status, 0,
                    cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03), flags);
            
            flags |= CV_LKFLOW_PYR_A_READY;
            for (i = k = 0; i < count; i++) {
                if (add_remove_pt) {
                    double dx = pt.x - points[1][i].x;
                    double dy = pt.y - points[1][i].y;

                    if (dx * dx + dy * dy <= 25) {
                        add_remove_pt = 0;
                        continue;
                    }
                }

                if (!status[i])
                    continue;

                points[1][k++] = points[1][i];
                cvCircle(image, cvPointFrom32f(points[1][i]), 3, CV_RGB(0, 255, 0), -1, 8, 0);
            }
            count = k;
            
        }

        if (add_remove_pt && count < MAX_COUNT) {
            points[1][count++] = cvPointTo32f(pt);
            cvFindCornerSubPix(grey, points[1] + count - 1, 1,
                    cvSize(win_size, win_size), cvSize(-1, -1),
                    cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
            add_remove_pt = 0;
        }

        CV_SWAP(prev_grey, grey, swap_temp);
        CV_SWAP(prev_pyramid, pyramid, swap_temp);
        CV_SWAP(points[0], points[1], swap_points);
        need_to_init = 0;
        
        gettimeofday(&currentTime, NULL);
        currentUsec = currentTime.tv_sec * 1000000 + currentTime.tv_usec;

        printf("Usec CPU time inside: %d - Current frame: %d\n", currentUsec - previousUsec,current_frame);
        
        char* filename_save = (char*)malloc(150*sizeof(char));
        sprintf(filename_save,"/mnt/sdcard/OpticalFlowLK_OpenCVTests/results/frame%d.png",current_frame);
        res1 = cvSaveImage(filename_save, image);
        
        current_frame++;
        if(current_frame==51){
            run_algorithm=0;
        }

    }
    
    //cvShowImage("LkDemo", image);
    
    
    

    return 0;
}




//cvGoodFeaturesToTrack(grey, eig, temp, points[1], &count, quality, min_distance, 0, 3, int use_harris=0, 0.04);
int testLKdemoHarris() {
    IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp;

    int win_size = 10;
    const int MAX_COUNT = 500;
    CvPoint2D32f * points[2] = {0, 0}, *swap_points;
    char* status = 0;
    int count = 0;
    int need_to_init = 1;
    int night_mode = 0;
    int flags = 0;
    int add_remove_pt = 0;
    CvPoint pt;

    int current_frame = 0;
    int run_algorithm = 1;

    while (run_algorithm) {

        IplImage* frame = 0;
        int i, k, c;

        
        char* filename = (char*)malloc(150*sizeof(char));
        sprintf(filename,"/mnt/sdcard/OpticalFlowLK_OpenCVTests/data/frame%d.png",current_frame);
        
        frame = cvLoadImage(filename, 1);
        //if (!frame)
        //    break;

        if (!image) {
            //printf("1\n");
            /* allocate all the buffers */
            image = cvCreateImage(cvGetSize(frame), 8, 3);
            image->origin = frame->origin;
            grey = cvCreateImage(cvGetSize(frame), 8, 1);
            prev_grey = cvCreateImage(cvGetSize(frame), 8, 1);
            pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
            prev_pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
            points[0] = (CvPoint2D32f*) cvAlloc(MAX_COUNT * sizeof (points[0][0]));
            points[1] = (CvPoint2D32f*) cvAlloc(MAX_COUNT * sizeof (points[0][0]));
            status = (char*) cvAlloc(MAX_COUNT);
            flags = 0;
        }

        cvCopy(frame, image, 0);
        cvCvtColor(image, grey, CV_BGR2GRAY);

        struct timeval currentTime;
        struct timeval previousTime;
        int previousUsec = 0;
        int currentUsec = 0;
        IplImage* src;
        IplImage* dst;
        int res1;


        gettimeofday(&previousTime, NULL);
        previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;
        
        if (night_mode)
            cvZero(image);

        if (need_to_init) {
            //printf("2\n");
            /* automatic initialization */
            IplImage* eig = cvCreateImage(cvGetSize(grey), 32, 1);
            IplImage* temp = cvCreateImage(cvGetSize(grey), 32, 1);
            double quality = 0.01;
            double min_distance = 10;

            count = MAX_COUNT;
            
            //Original
            cvGoodFeaturesToTrack(grey, eig, temp, points[1], &count,
                    quality, min_distance, 0, 3, 1, 0.04);
            
            //testing
            //cvGoodFeaturesToTrack(grey, eig, temp, points[1], &count,
            //        0.1, 20, 0, 3, 1, 0.04);
            
            cvFindCornerSubPix(grey, points[1], count,
                    cvSize(win_size, win_size), cvSize(-1, -1),
                    cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
            cvReleaseImage(&eig);
            cvReleaseImage(&temp);

            add_remove_pt = 0;
        } else if (count > 0) {
            //printf("3\n");
            cvCalcOpticalFlowPyrLK(prev_grey, grey, prev_pyramid, pyramid,
                    points[0], points[1], count, cvSize(win_size, win_size), 3, status, 0,
                    cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03), flags);
            
            flags |= CV_LKFLOW_PYR_A_READY;
            for (i = k = 0; i < count; i++) {
                if (add_remove_pt) {
                    double dx = pt.x - points[1][i].x;
                    double dy = pt.y - points[1][i].y;

                    if (dx * dx + dy * dy <= 25) {
                        add_remove_pt = 0;
                        continue;
                    }
                }

                if (!status[i])
                    continue;

                points[1][k++] = points[1][i];
                cvCircle(image, cvPointFrom32f(points[1][i]), 3, CV_RGB(0, 255, 0), -1, 8, 0);
            }
            count = k;
            
        }

        if (add_remove_pt && count < MAX_COUNT) {
            points[1][count++] = cvPointTo32f(pt);
            cvFindCornerSubPix(grey, points[1] + count - 1, 1,
                    cvSize(win_size, win_size), cvSize(-1, -1),
                    cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
            add_remove_pt = 0;
        }

        CV_SWAP(prev_grey, grey, swap_temp);
        CV_SWAP(prev_pyramid, pyramid, swap_temp);
        CV_SWAP(points[0], points[1], swap_points);
        need_to_init = 0;
        
        gettimeofday(&currentTime, NULL);
        currentUsec = currentTime.tv_sec * 1000000 + currentTime.tv_usec;

        printf("Usec CPU time inside: %d - Current frame: %d\n", currentUsec - previousUsec,current_frame);
        
        char* filename_save = (char*)malloc(150*sizeof(char));
        sprintf(filename_save,"/mnt/sdcard/OpticalFlowLK_OpenCVTests/results/frame%d.png",current_frame);
        res1 = cvSaveImage(filename_save, image);
        
        current_frame++;
        if(current_frame==51){
            run_algorithm=0;
        }

    }
    
    //cvShowImage("LkDemo", image);
    
    
    

    return 0;
}












void *mainFunction(void *arg)
{

    printf("Starting run\n");
    

  
    int result=SDCardMount();
    printf("SD card mounted? %d\n",result);
    int res;
    
    
    struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;
    IplImage* src;
    IplImage* dst;
    int res1;
    
    
    gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;
    
        
    //res = testLKdemo();
    res = testLKdemoHarris();
    
    assert(res==0);
    printf("+ Optical Flow LK. OK\n");
    
    gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("Usec CPU time: %d \n",currentUsec - previousUsec);
        
    printf("Unmounting SD card...\n");
    SDCardUnmount();
    
    printf("Tests ended \n");

    pthread_exit(0);
}


static void Fatal_extension(
  Internal_errors_Source  the_source,
  bool                    is_internal,
  uint32_t                the_error
)
{ 
    if (the_source != RTEMS_FATAL_SOURCE_EXIT)
        printk ("\nSource %d Internal %d Error %d\n", the_source, is_internal, the_error);
    if (the_source == RTEMS_FATAL_SOURCE_EXCEPTION)
        rtems_exception_frame_print((void *) the_error);
}





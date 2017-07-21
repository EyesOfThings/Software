///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Simple Hello World

// 1: Includes
// ----------------------------------------------------------------------------
#include <stdio.h>
#include "DrvGpioDefines.h"
#include "DrvGpio.h"
#include <DrvTimer.h>
#include <DrvI2cMaster.h>	
#include "DrvCpr.h"
#include "bmx055/imuApi.h"
#include "ProjectSections.h"
#include "brdMv0182.h"
#include <stdlib.h>
#include <math.h>

#include "rtems_config.h"

#include <cv.h>
#include <ccv.h>
#include <SDCardIO.h>

#include "camera.h"
#include "rtsp.h"

#include <rtems/cpuuse.h>

#include <DrvShaveL2Cache.h>
#include "OsDrvSvu.h"

#include "faceDetection.h"

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------

#define GPIO_57 57
#define GPIO_34 58

#define I2C3_SDA  80
#define I2C3_SCL  79

#define I2C_SPEED_KHZ_DEFAULT  (100)

#define RadsToDegrees 180/3.1415

#define CalibrateX 88
#define CalibrateY 0
#define CalibrateZ 87

#define Calibratecms -100

/*
// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

// 4: Static Local Data
// ----------------------------------------------------------------------------
*/

static tyI2cConfig  i2c2MasterCfg =
{
	.device = IIC3_BASE_ADR,
	.sclPin = I2C3_SCL,
	.sdaPin = I2C3_SDA,
	.speedKhz =  I2C_SPEED_KHZ_DEFAULT,
	.addressSize = ADDR_7BIT,
	.errorHandler = NULL
};

static u8 messageString[6][32] =
{
	"IMU I2C SUCCES!",
	"BMX055 ACCEL NOT RESPONDING!",
	"BMX055 GYRO NOT RESPONDING!",
	"BMX055 MAG NOT RESPONDING!",
	"BMP180 NOT RESPONDING!",
	"I2C BUS ERROR!",
};


static volatile u32 imuCMXIndex = 0;
static tyTimeStamp globalTimestamp;
static u8* msgResponse;

RtspServer * server = NULL;
pthread_t accelThread;
int degreesX, degreesY;
int axisXcms;
ccv_bbf_classifier_cascade_t* cascade;

u8 testFail = FALSE;
s32 result;
u32 test[1];
I2CM_Device I2cDevice;
imuBmx055ConfigData_t Configuration2;
imuBmx055InterruptConfig_t IntConf =
									{
										.gpioInt1 = GPIO_34,
										.gpioInt3 = GPIO_57,
										.handlerPriority = 10
									};

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------

// 6: Functions Implementation
// ----------------------------------------------------------------------------
static void SelectMsg(s32 res)
{
	switch (res)
	{
	case 0:
		msgResponse = messageString[0];
		break;
	case 11:
	case 12:
	case 13:
		msgResponse = messageString[1];
		break;
	case 21:
	case 22:
	case 23:
		msgResponse = messageString[2];
		break;
	case 31:
	case 32:
	case 33:
		msgResponse = messageString[3];
		break;
	case 41:
	case 42:
		msgResponse = messageString[4];
		break;
	default:
		msgResponse = messageString[5];
		break;
	}
}

static void StoreData(imuType_t DataPacketType)
{
	imuPacket_t newPacket;
	imuType_t imuPacketType = DataPacketType;

	imuGetDataPacket(imuPacketType, &newPacket);
	float rho, phi, theta;
	int x, y, z;
	x = newPacket.dataX;
	y = newPacket.dataY;
	z = newPacket.dataZ;

	axisXcms = newPacket.dataX;

	degreesX = (atan(sqrt((y*y)+(z*z))/x) * RadsToDegrees) + CalibrateX;
	degreesY = (atan(sqrt((x*x)+(z*z))/y) * RadsToDegrees) + CalibrateY;

}

void processing_loop()
{
	int i=0;

	for (i = 0; i< N_SHAVES; i++) 
		DrvShaveL2CachePartitionFlushAndInvalidate(i);
	
	swcLeonDataCacheFlush();

	struct timeval currentTime;
	struct timeval previousTime;
	int previousUsec = 0;
	int currentUsec = 0;

	int w, h, x;

	colorYCbCr* last_frame;
	ccv_comp_t cara_Anterior;
	cara_Anterior.rect.x = -1;
	cara_Anterior.rect.y = -1;
	int caraEnRecorte = 0;
	float angulo = 0;

	getImgSize (&w, &h);
	CvPoint2D32f center;
	center.x = w/2.0f;
	center.y = h/2.0f;
	CvMat *mapMatrix = cvCreateMat( 2, 3, CV_32FC1 );
	ccv_dense_matrix_t* img_ccv = 0;
	IplImage* src = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);
	IplImage* dst = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);
	
	ccv_array_t* seq;
	ccv_comp_t* comp;

	for(;;)
	{
		// Captures the frame (w=480, h=256)
		getYCbCrFrame(&last_frame,&w,&h);
		StoreData(IMU_BMX055_ACCEL);


		if (axisXcms >= Calibratecms && axisXcms <= 0)
			angulo = 0;
		else if (axisXcms < Calibratecms)
			angulo = (float) degreesY;
		else 
			angulo = (float) -degreesY;

		// Fills the IplImage
		for( x=0; x<w*h; x++ ) { 
			(src->imageData)[x] = last_frame[x].Y+0x80;
		}

		if (angulo != 0) {
			cv2DRotationMatrix(center, angulo, 1, mapMatrix);
			cvWarpAffine(src, dst, mapMatrix, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
			ccv_read (dst->imageData, &img_ccv, CCV_IO_GRAY_RAW, dst->height, dst->width, dst->widthStep);
		} else {
			ccv_read (src->imageData, &img_ccv, CCV_IO_GRAY_RAW, src->height, src->width, src->widthStep);
		}

		if (cara_Anterior.rect.x != -1 && cara_Anterior.rect.y != -1) {
			int j;
			caraEnRecorte = 0;
			ccv_dense_matrix_t* recorte = 0;
			
			ccv_slice(img_ccv, (ccv_matrix_t**)&recorte, 0,
				cara_Anterior.rect.y,
				cara_Anterior.rect.x,
				cara_Anterior.rect.height,
				cara_Anterior.rect.width);

			seq = _ccv_bbf_detect_objects_Parallel(recorte, &cascade, 1, ccv_bbf_custom);

			for (j=0; j < seq->rnum; j++)
			{
				comp = (ccv_comp_t*) ccv_array_get (seq, j);

				drawSquare (img_ccv,
					comp->rect.x + cara_Anterior.rect.x,
					comp->rect.y + cara_Anterior.rect.y,
					comp->rect.width, 150);
				
				cara_Anterior.rect.x += comp->rect.x - marginX;
				cara_Anterior.rect.y += comp->rect.y - marginY;

				if (cara_Anterior.rect.x < 0) cara_Anterior.rect.x = 0;
				if (cara_Anterior.rect.y < 0) cara_Anterior.rect.y = 0;

				if (cara_Anterior.rect.x + widthAdd + comp->rect.width >= w) {
					cara_Anterior.rect.width = w - cara_Anterior.rect.x;
				} 
				else 
					cara_Anterior.rect.width = comp->rect.width + widthAdd;

				if (cara_Anterior.rect.y + heightAdd + comp->rect.height >= h) {
					cara_Anterior.rect.height = h - cara_Anterior.rect.y;
				}
				else 
					cara_Anterior.rect.height = comp->rect.height + heightAdd;

				caraEnRecorte = 1;

			}

			ccv_matrix_free(recorte);

			if (!caraEnRecorte) {
				cara_Anterior.rect.x = -1;
				cara_Anterior.rect.y = -1;
			}
		}

		if (cara_Anterior.rect.x == -1 && cara_Anterior.rect.y == -1)
		{
			seq = _ccv_bbf_detect_objects_Parallel (img_ccv, &cascade, 1, ccv_bbf_custom);
			int j = 0;
			if (seq->rnum > 1) {
				j = select_face (seq, (int)center.x, (int)center.y);
			}
			if (seq->rnum >= 1) {
				comp = (ccv_comp_t*) ccv_array_get (seq, j);
				cara_Anterior.rect.x = comp->rect.x - marginX;
				cara_Anterior.rect.y = comp->rect.y - marginY;
				if (cara_Anterior.rect.x < 0) cara_Anterior.rect.x = 0;
				if (cara_Anterior.rect.y < 0) cara_Anterior.rect.y = 0;
				
				if (cara_Anterior.rect.x + widthAdd + comp->rect.width >= w) {
					cara_Anterior.rect.width = w - cara_Anterior.rect.x;
				} 
				else 
					cara_Anterior.rect.width = comp->rect.width + widthAdd;

				if (cara_Anterior.rect.y + heightAdd + comp->rect.height >= h) {
					cara_Anterior.rect.height = h - cara_Anterior.rect.y;
				}
				else 
					cara_Anterior.rect.height = comp->rect.height + heightAdd;

				drawSquare (img_ccv, comp->rect.x, comp->rect.y, comp->rect.width, 0);
			}
		}

		// Gets the values for the RTSP
		for( x=0; x<w*h; x++ ) { 
			last_frame[x].Y = ((uchar*)(img_ccv->data.u8))[x]-0x80; // 2YUV (JPEG needs it). Cb and Cr channels remain 0x00
		}

		ccv_matrix_free(img_ccv);
		// Ensures the RTSP server has already been created
		if( (server!=NULL) ) 
			sendToAllClients(last_frame,w,h);

		
	}

}

void POSIX_Init (void *args)
{
	
	u32 revision;

	pthread_attr_t attr;
	rtems_status_code sc;   
	int status, rc1;
	pthread_t thread1;
	u32 i;
	struct stat st;

	initClocksAndMemory();

	SDCardMount();
	cascade = ccv_bbf_read_classifier_cascade("/mnt/sdcard/Rotation-invariant_faceDetector/face");
	SDCardUnmount();
	
	brd182GetPcbRevison(&revision);
	if (revision == MV0182_R5)
	{
		printf("Wrong board revision. BMX055 no longer resides on R5\n");
		exit(1);
	}

	DrvTimerSleepMs(100);

	u8 testFail = FALSE;
	s32 result;
	u32 test[1];
	I2CM_Device I2cDevice;
	imuBmx055ConfigData_t Configuration;
	imuBmx055InterruptConfig_t IntConf =
										 {
											 .gpioInt1 = GPIO_34,
											 .gpioInt3 = GPIO_57,
											 .handlerPriority = 10
										 };

	//init I2C
	result = DrvI2cMInitFromConfig(&I2cDevice, &i2c2MasterCfg);
	if (result != I2CM_STAT_OK) printf ("Error inicializando el I2C\n");
	DrvTimerSleepMs(5);
	result = imuI2cTest(&I2cDevice);
	SelectMsg(result);
	printf("I2C test result=%d => %s \n", result, msgResponse);

	Configuration = imuBmx055DefaultConfiguration;

	/// Init timestamp
	DrvTimerStart(&globalTimestamp);

	result = imuComponentInit(&I2cDevice, &Configuration);
	if (result != 0)
	{
		printf("Bad init! %d", result);
	}

	// RTSP server initialization
	initRTSPServer(&server);

	// Camera initialization
	init_camera();
	start_camera();

	// CV thread creation

	if(pthread_attr_init(&attr) !=0) {
		printk("pthread_attr_init error");
	}  
	if(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
		printk("pthread_attr2_setinheritsched error");
	}
	if(pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0) {
		printk("pthread_attr2_setschedpolicy error");
	}

	if ((rc1 = pthread_create( &thread1, &attr, &processing_loop,NULL))) {
		printk("\nThread 1 creation failed: %d\n", rc1);
		exit(rc1);
	}
	else {
		printk("\nThread camera created\n");
	}
	DrvTimerSleepMs(100);

	// RTSP starts accepting connections
	runRTSPServer(server);

	pthread_join (thread1, NULL);
	imuBmx055DisableInterrupts();
	exit(0);

	

}


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

#include <SDCardIO.h>
#include <ccv.h>
#include "case.h"
#include "ccv_case.h"
#include "case_main.h"




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
void *libccv_thread(void *arg);
int test1_canny();
int test2_pedestrian();
int test3_matrixAddition();
int test4_sobel();
int test5_otsu();
int test6_imageContrast();
int test7_perspectiveTransform();
int test8_hog();
int test9_sift();
int test10_cblasMatMultiplication();
int test11_convNetwork();
int test12_convNetwork1();
int test13_convNetwork2();
int test14_detectText();
int test15_detectFaceBBF();


//extern connectToAP(int, char**);

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

    if ((rc1=pthread_create(&thread1, &attr, &libccv_thread, NULL))) {
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


void *libccv_thread(void *arg)
{
  
    printf("Starting run\n");
    
    SDCardMount();
    
    //EXAMPLE 1 (CANNY)
    test1_canny();
    
    //EXAMPLE 2 (PEDESTRIAN)
    test2_pedestrian();
    
    //EXAMPLE 3 matrix addition
    test3_matrixAddition();
    
    //EXAMPLE 4 sobel
    test4_sobel();
    
    //EXAMPLE 5 otsu
    test5_otsu();
    
    //EXAMPLE 6 image contrast
    test6_imageContrast();
    
    //EXAMPLE 7 matrix perspective transform
    test7_perspectiveTransform();
    
    //EXAMPLE 8 HOG
    test8_hog();
    
    //EXAMPLE 9 SIFT
    test9_sift();
    
    //EXAMPLE 10 CBLAS, matrix multiplication
    test10_cblasMatMultiplication();
    
    //EXAMPLE 11, Convolutional network
    test11_convNetwork();
    
    //EXAMPLE 12, Convolutional network 2
    test12_convNetwork1();
    
    //EXAMPLE 13, Convolutional network 3
    test13_convNetwork2();
    
    //EXAMPLE 14, Detect text
    test14_detectText();
    
    //EXAMPLE 15, Detect pedestrian BFF
    test15_detectFaceBBF();
    
    printf("Tests ended \n");
    
    SDCardUnmount();

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
}

int test1_canny()
{
    printf("Example 1, canny and write png\n");
    ccv_dense_matrix_t* image = 0;
    ccv_read("/mnt/sdcard/samples/blackbox.png", &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);
    ccv_dense_matrix_t* x = 0;
    ccv_canny(image, &x, 0, 3, 36, 36 * 3);
    REQUIRE_MATRIX_FILE_EQ(x, "/mnt/sdcard/data/blackbox.canny.bin", "Canny edge detector on artificial image");
    printf("Value of image: rows - %d, cols - %d, type - %d \n",x->rows,x->cols,x->type);
    ccv_write(x, "/mnt/sdcard/samples/test1.png", 0, CCV_IO_PNG_FILE, 0);
    ccv_matrix_free(image);
    ccv_matrix_free(x);
    //ccv_write(canny, "/mnt/sdcard/samples/nature.bin", 0, CCV_IO_BINARY_FILE, 0);
    
    
}

int test2_pedestrian()
{
    printf("Example 2, pedestrian cascade detector (several days)\n");
    int i;
    ccv_enable_default_cache();
    ccv_dense_matrix_t* image = 0;
    printf("Loading files \n");
    ccv_icf_classifier_cascade_t* cascade = ccv_icf_read_classifier_cascade("/mnt/sdcard/samples/pedestrian.icf");
    ccv_read("/mnt/sdcard/samples/pedestrian2.png", &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);

    //unsigned int elapsed_time = get_current_time();
    printf("Detecting pedestrians \n");
    ccv_array_t* seq = ccv_icf_detect_objects(image, &cascade, 1, ccv_icf_default_params);
    //elapsed_time = get_current_time() - elapsed_time;
    printf("Results: \n");
    for (i = 0; i < seq->rnum; i++) {
        ccv_comp_t* comp = (ccv_comp_t*) ccv_array_get(seq, i);
        printf("%d %d %d %d %f\n", comp->rect.x, comp->rect.y, comp->rect.width, comp->rect.height, comp->classification.confidence);
    }
    printf("total : %d detected\n", seq->rnum);
    ccv_array_free(seq);
    ccv_matrix_free(image);

    ccv_icf_classifier_cascade_free(cascade);
    ccv_disable_cache();
}

int test3_matrixAddition()
{
    printf("Example 3, matrix addition\n");
    ccv_dense_matrix_t* a = ccv_dense_matrix_new(3, 2, CCV_64F | CCV_C1, 0, 0);
    a->data.f64[0] = 0.11;
    a->data.f64[1] = 0.12;
    a->data.f64[2] = 0.13;
    a->data.f64[3] = 0.21;
    a->data.f64[4] = 0.22;
    a->data.f64[5] = 0.23;
    ccv_dense_matrix_t* b = ccv_dense_matrix_new(3, 2, CCV_64F | CCV_C1, 0, 0);
    b->data.f64[0] = 1011;
    b->data.f64[1] = 1012;
    b->data.f64[2] = 1021;
    b->data.f64[3] = 1022;
    b->data.f64[4] = 1031;
    b->data.f64[5] = 1032;
    ccv_dense_matrix_t* y = 0;
    ccv_add(a, b, (ccv_matrix_t**) & y, 0);
    double hy[6] = {1011.11, 1012.12, 1021.13, 1022.21, 1031.22, 1032.23};
    REQUIRE_ARRAY_EQ_WITH_TOLERANCE(double, hy, y->data.f64, 6, 1e-6, "3x2, 3x2 matrix addition failure");
    ccv_matrix_free(a);
    ccv_matrix_free(b);
    ccv_matrix_free(y);
}

int test4_sobel()
{
    printf("Example 4, sobel\n");
    ccv_dense_matrix_t* image = 0;
    ccv_read("/mnt/sdcard/samples/chessbox.bmp", &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);
    ccv_dense_matrix_t* x = 0;
    ccv_sobel(image, &x, 0, 0, 1);
    REQUIRE_MATRIX_FILE_EQ(x, "/mnt/sdcard/data/chessbox.sobel.x.bin", "should be sobel of partial derivative on x");
    ccv_dense_matrix_t* y = 0;
    ccv_sobel(image, &y, 0, 1, 0);
    REQUIRE_MATRIX_FILE_EQ(y, "/mnt/sdcard/data/chessbox.sobel.y.bin", "should be sobel of partial derivative on y");
    ccv_dense_matrix_t* u = 0;
    ccv_sobel(image, &u, 0, 1, 1);
    REQUIRE_MATRIX_FILE_EQ(u, "/mnt/sdcard/data/chessbox.sobel.u.bin", "should be sobel of derivative along diagonal");
    ccv_dense_matrix_t* v = 0;
    ccv_sobel(image, &v, 0, -1, 1);
    REQUIRE_MATRIX_FILE_EQ(v, "/mnt/sdcard/data/chessbox.sobel.v.bin", "should be sobel of derivative along the other diagonal");
    ccv_dense_matrix_t* x3 = 0;
    ccv_sobel(image, &x3, 0, 0, 3);
    REQUIRE_MATRIX_FILE_EQ(x3, "/mnt/sdcard/data/chessbox.sobel.x.3.bin", "should be sobel of partial derivative on x within 3x3 window");
    ccv_dense_matrix_t* y3 = 0;
    ccv_sobel(image, &y3, 0, 3, 0);
    REQUIRE_MATRIX_FILE_EQ(y3, "/mnt/sdcard/data/chessbox.sobel.y.3.bin", "should be sobel of partial derivative on y within 3x3 window");
    ccv_dense_matrix_t* x5 = 0;
    ccv_sobel(image, &x5, 0, 0, 5);
    REQUIRE_MATRIX_FILE_EQ(x5, "/mnt/sdcard/data/chessbox.sobel.x.5.bin", "should be sobel of partial derivative on x within 5x5 window");
    ccv_dense_matrix_t* y5 = 0;
    ccv_sobel(image, &y5, 0, 5, 0);
    REQUIRE_MATRIX_FILE_EQ(y5, "/mnt/sdcard/data/chessbox.sobel.y.5.bin", "should be sobel of partial derivative on y within 5x5 window");
    ccv_write(y5, "/mnt/sdcard/samples/test4.png", 0, CCV_IO_PNG_FILE, 0);
    ccv_matrix_free(image);
    ccv_matrix_free(x);
    ccv_matrix_free(y);
    ccv_matrix_free(u);
    ccv_matrix_free(v);
    ccv_matrix_free(x3);
    ccv_matrix_free(y3);
    ccv_matrix_free(x5);
    ccv_matrix_free(y5);
}

int test5_otsu()
{
    
    printf("Example 5, otsu threshold\n");
    ccv_dense_matrix_t* image = ccv_dense_matrix_new(6, 6, CCV_32S | CCV_C1, 0, 0);
    // the test case is grabbed from: http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html
    image->data.i32[0] = image->data.i32[1] = image->data.i32[6] = image->data.i32[22] = image->data.i32[23] = image->data.i32[28] = image->data.i32[29] = image->data.i32[35] = 0;
    image->data.i32[2] = image->data.i32[7] = image->data.i32[12] = image->data.i32[16] = image->data.i32[21] = image->data.i32[27] = image->data.i32[34] = 1;
    image->data.i32[15] = image->data.i32[26] = 2;
    image->data.i32[8] = image->data.i32[10] = image->data.i32[13] = image->data.i32[17] = image->data.i32[20] = image->data.i32[33] = 3;
    image->data.i32[3] = image->data.i32[4] = image->data.i32[9] = image->data.i32[11] = image->data.i32[14] = image->data.i32[18] = image->data.i32[19] = image->data.i32[25] = image->data.i32[32] = 4;
    image->data.i32[5] = image->data.i32[24] = image->data.i32[30] = image->data.i32[31] = 5;
    double var;
    int threshold = ccv_otsu(image, &var, 6);
    REQUIRE_EQ(threshold, 2, "threshold should be 2 (inclusive)");
    REQUIRE_EQ_WITH_TOLERANCE(var, 2.6287, 0.0001, "between class variance should be 2.6287");
    ccv_matrix_free(image);
    
}
int test6_imageContrast()
{
    
    printf("Example 6, image contrast\n");
    ccv_dense_matrix_t* image = 0;
    ccv_read("/mnt/sdcard/samples/nature.bmp", &image, CCV_IO_RGB_COLOR | CCV_IO_ANY_FILE);
    ccv_dense_matrix_t* b = 0;
    ccv_contrast(image, &b, 0, 0.5);
    REQUIRE_MATRIX_FILE_EQ(b, "/mnt/sdcard/data/nature.contrast.0.5.bin", "should be decontrasted image");
    ccv_write(b, "/mnt/sdcard/samples/test6_1.png", 0, CCV_IO_PNG_FILE, 0);
    ccv_matrix_free(b);
    b = 0;
    ccv_contrast(image, &b, 0, 1.5);
    REQUIRE_MATRIX_FILE_EQ(b, "/mnt/sdcard/data/nature.contrast.1.5.bin", "should be overcontrasted image");
    ccv_write(b, "/mnt/sdcard/samples/test6_2.png", 0, CCV_IO_PNG_FILE, 0);
    ccv_matrix_free(b);
    ccv_matrix_free(image);
}

int test7_perspectiveTransform()
{
    printf("Example 7, perspective transform\n");
    ccv_dense_matrix_t* image = 0;
    ccv_read("/mnt/sdcard/samples/chessbox.bmp", &image, CCV_IO_ANY_FILE);
    ccv_dense_matrix_t* b = 0;
    ccv_perspective_transform(image, &b, 0, cosf(CCV_PI / 6), 0, 0, 0, 1, 0, -sinf(CCV_PI / 6), 0, cosf(CCV_PI / 6));
    REQUIRE_MATRIX_FILE_EQ(b, "/mnt/sdcard/data/chessbox.perspective.transform.bin", "should have data/chessbox.png rotated along y-axis for 30");
    ccv_write(b, "/mnt/sdcard/samples/test7.png", 0, CCV_IO_PNG_FILE, 0);
    ccv_matrix_free(image);
    ccv_matrix_free(b);
}

int test8_hog()
{
    printf("Example 8, HOG\n");
    ccv_dense_matrix_t* image = 0;
    ccv_read("/mnt/sdcard/samples/nature.bmp", &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);
    ccv_dense_matrix_t* x = 0;
    ccv_hog(image, &x, 0, 9, 8);
    ccv_write(x, "/mnt/sdcard/samples/test8.png", 0, CCV_IO_PNG_FILE, 0);
    ccv_matrix_free(image);
    ccv_matrix_free(x);
    //ccv_write(canny, "/mnt/sdcard/samples/nature.bin", 0, CCV_IO_BINARY_FILE, 0);
    
    
}


int test9_sift()
{
    printf("Example 9, SIFT\n");
    ccv_enable_default_cache();
    ccv_dense_matrix_t* object = 0;
    ccv_dense_matrix_t* image = 0;
    ccv_read("/mnt/sdcard/samples/book.png", &object, CCV_IO_GRAY | CCV_IO_ANY_FILE);
    //ccv_read("/mnt/sdcard/samples/scene.bmp", &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);
    ccv_read("/mnt/sdcard/samples/book.png", &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);
    
    printf("Pictures read \n");
    
    ccv_sift_param_t params = {
        .noctaves = 3,
        .nlevels = 6,
        .up2x = 1,
        .edge_threshold = 10,
        .norm_threshold = 0,
        .peak_threshold = 0,
    };
    ccv_array_t* obj_keypoints = 0;
    ccv_dense_matrix_t* obj_desc = 0;
    ccv_sift(object, &obj_keypoints, &obj_desc, 0, params);
    
    printf("Keypoints calculated obj %d \n",obj_keypoints->rnum);
    
    ccv_array_t* image_keypoints = 0;
    ccv_dense_matrix_t* image_desc = 0;
    ccv_sift(image, &image_keypoints, &image_desc, 0, params);

    printf("Keypoints calculated img %d \n",image_keypoints->rnum);
    
    int i, j, k;
    int match = 0;
    for (i = 0; i < obj_keypoints->rnum; i++) {
        float* odesc = obj_desc->data.f32 + i * 128;
        int minj = -1;
        double mind = 1e6, mind2 = 1e6;
        for (j = 0; j < image_keypoints->rnum; j++) {
            float* idesc = image_desc->data.f32 + j * 128;
            double d = 0;
            for (k = 0; k < 128; k++) {
                d += (odesc[k] - idesc[k]) * (odesc[k] - idesc[k]);
                if (d > mind2)
                    break;
            }
            if (d < mind) {
                mind2 = mind;
                mind = d;
                minj = j;
            } else if (d < mind2) {
                mind2 = d;
            }
        }
        if (mind < mind2 * 0.36) {
            ccv_keypoint_t* op = (ccv_keypoint_t*) ccv_array_get(obj_keypoints, i);
            ccv_keypoint_t* kp = (ccv_keypoint_t*) ccv_array_get(image_keypoints, minj);
            printf("%f %f => %f %f\n", op->x, op->y, kp->x, kp->y);
            match++;
        }
    }
    printf("%dx%d on %dx%d\n", object->cols, object->rows, image->cols, image->rows);
    printf("%d keypoints out of %d are matched\n", match, obj_keypoints->rnum);
    printf("elpased time : %d\n", 0);
    ccv_array_free(obj_keypoints);
    ccv_array_free(image_keypoints);
    ccv_matrix_free(obj_desc);
    ccv_matrix_free(image_desc);
    ccv_matrix_free(object);
    ccv_matrix_free(image);
    ccv_disable_cache();
    
    return 1;
    
    
}

int test10_cblasMatMultiplication(){
    printf("Example 10, CBLAS mat multiplication\n");
    ccv_dense_matrix_t* a = ccv_dense_matrix_new(3, 2, CCV_64F | CCV_C1, 0, 0);
    a->data.f64[0] = 0.11;
    a->data.f64[1] = 0.12;
    a->data.f64[2] = 0.13;
    a->data.f64[3] = 0.21;
    a->data.f64[4] = 0.22;
    a->data.f64[5] = 0.23;
    ccv_dense_matrix_t* b = ccv_dense_matrix_new(3, 2, CCV_64F | CCV_C1, 0, 0);
    b->data.f64[0] = 1011;
    b->data.f64[1] = 1012;
    b->data.f64[2] = 1021;
    b->data.f64[3] = 1022;
    b->data.f64[4] = 1031;
    b->data.f64[5] = 1032;
    ccv_dense_matrix_t* y = 0;
    ccv_gemm(a, b, 1, 0, 0, CCV_A_TRANSPOSE, (ccv_matrix_t**) & y, 0);
    double hy[4] = {470.760000, 471.220000, 572.860000, 573.420000};
    REQUIRE_ARRAY_EQ_WITH_TOLERANCE(double, hy, y->data.f64, 4, 1e-6, "2x3, 3x2 matrix multiplication failure");
    ccv_matrix_free(a);
    ccv_matrix_free(b);
    ccv_matrix_free(y);
    
    
}


int test11_convNetwork(){
    printf("Example 11, Convolutional network\n");
    ccv_convnet_layer_param_t params = {
		.type = CCV_CONVNET_CONVOLUTIONAL,
		.bias = 0,
		.glorot = sqrtf(2),
		.input = {
			.matrix = {
				.rows = 225,
				.cols = 225,
				.channels = 3,
				.partition = 1,
			},
		},
		.output = {
			.convolutional = {
				.count = 4,
				.strides = 4,
				.border = 1,
				.rows = 11,
				.cols = 11,
				.channels = 3,
				.partition = 1,
			},
		},
	};
	ccv_convnet_t* convnet = ccv_convnet_new(0, ccv_size(225, 225), &params, 1);
	int i, x, y;
	for (i = 0; i < 11 * 11 * 3 * 4; i++)
		convnet->layers[0].w[i] = 1;
	ccv_dense_matrix_t* a = ccv_dense_matrix_new(225, 225, CCV_32F | CCV_C3, 0, 0);
	for (i = 0; i < 225 * 225 * 3; i++)
		a->data.f32[i] = 1;
	ccv_dense_matrix_t* b = 0;
	ccv_convnet_encode(convnet, &a, &b, 1);
	ccv_matrix_free(a);
	REQUIRE(b->rows == 55 && b->cols == 55, "11x11 convolves on 225x255 with strides 4 should produce 55x55 matrix");
	ccv_dense_matrix_t* c = ccv_dense_matrix_new(55, 55, CCV_32F | 4, 0, 0);
	for (y = 0; y < 55; y++)
		for (x = 0; x < 55; x++)
			for (i = 0; i < 4; i++)
			c->data.f32[(y * 55 + x) * 4 + i] = ((x == 0 && y == 0) || (x == 0 && y == 54) || (x == 54 && y == 0) || (x == 54 && y == 54)) ? 300 : ((x == 0 || y == 0 || x == 54 || y == 54) ? 330 : 363);
	REQUIRE_MATRIX_EQ(b, c, "55x55 matrix should be exactly a matrix fill 363, with 300 on the corner and 330 on the border");
	ccv_matrix_free(b);
	ccv_matrix_free(c);
	ccv_convnet_free(convnet);
    
}

int test12_convNetwork1() {
    printf("Example 12, convolutional network of 5x5 on 27x27 with non-uniform weights\n");
    	ccv_convnet_layer_param_t params = {
		.type = CCV_CONVNET_CONVOLUTIONAL,
		.bias = 0,
		.glorot = sqrtf(2),
		.input = {
			.matrix = {
				.rows = 27,
				.cols = 27,
				.channels = 1,
				.partition = 1,
			},
		},
		.output = {
			.convolutional = {
				.count = 4,
				.strides = 1,
				.border = 2,
				.rows = 5,
				.cols = 5,
				.channels = 1,
				.partition = 1,
			},
		},
	};
	ccv_convnet_t* convnet = ccv_convnet_new(0, ccv_size(27, 27), &params, 1);
	int i, x, y;
	for (x = 0; x < 4; x++)
		for (i = 0; i < 5 * 5; i++)
			convnet->layers->w[x * 5 * 5 + i] = i + 1;
	ccv_dense_matrix_t* a = ccv_dense_matrix_new(27, 27, CCV_32F | CCV_C1, 0, 0);
	for (i = 0; i < 27 * 27; i++)
		a->data.f32[i] = i + 1;
	ccv_dense_matrix_t* b = 0;
	ccv_convnet_encode(convnet, &a, &b, 1);
	REQUIRE(b->rows == 27 && b->cols == 27, "5x5 convolves on 27x27 with border 2 should produce 27x27 matrix");
	ccv_matrix_free(a);
	ccv_dense_matrix_t* c = ccv_dense_matrix_new(27, 27, CCV_32F | 4, 0, 0);
	// the first column
	float sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 3; x++)
			sum += ((y + 2) * 5 + x + 3) * (y * 27 + x + 1);
	for (i = 0; i < 4; i++)
		c->data.f32[i] = sum;
	sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 4; x++)
			sum += ((y + 2) * 5 + x + 2) * (y * 27 + x + 1);
	for (i = 0; i < 4; i++)
		c->data.f32[4 + i] = sum;
	sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 5; x++)
			sum += ((y + 2) * 5 + x + 1) * (y * 27 + x + 1);
	for (x = 2; x < 25; x++)
		for (i = 0; i < 4; i++)
			c->data.f32[x * 4 + i] = sum + (x - 2) * 36 * 15 / 2;
	sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 4; x++)
			sum += ((y + 2) * 5 + x + 1) * (y * 27 + x + 24);
	for (i = 0; i < 4; i++)
		c->data.f32[25 * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 3; x++)
			sum += ((y + 2) * 5 + x + 1) * (y * 27 + x + 25);
	for (i = 0; i < 4; i++)
		c->data.f32[26 * 4 + i] = sum;
	// the second column
	sum = 0;
	for (y = 0; y < 4; y++)
		for (x = 0; x < 3; x++)
			sum += ((y + 1) * 5 + x + 3) * (y * 27 + x + 1);
	for (i = 0; i < 4; i++)
		c->data.f32[27 * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 4; y++)
		for (x = 0; x < 4; x++)
			sum += ((y + 1) * 5 + x + 2) * (y * 27 + x + 1);
	for (i = 0; i < 4; i++)
		c->data.f32[28 * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 4; y++)
		for (x = 0; x < 5; x++)
			sum += ((y + 1) * 5 + x + 1) * (y * 27 + x + 1);
	for (x = 2; x < 25; x++)
		for (i = 0; i < 4; i++)
			c->data.f32[(27 + x) * 4 + i] = sum + (x - 2) * 31 * 20 / 2;
	sum = 0;
	for (y = 0; y < 4; y++)
		for (x = 0; x < 4; x++)
			sum += ((y + 1) * 5 + x + 1) * (y * 27 + x + 24);
	for (i = 0; i < 4; i++)
		c->data.f32[52 * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 4; y++)
		for (x = 0; x < 3; x++)
			sum += ((y + 1) * 5 + x + 1) * (y * 27 + x + 25);
	for (i = 0; i < 4; i++)
		c->data.f32[53 * 4 + i] = sum;
	sum = 0;
	// the last 2nd column
	for (y = 0; y < 4; y++)
		for (x = 0; x < 3; x++)
			sum += (y * 5 + x + 3) * ((y + 23) * 27 + x + 1);
	for (i = 0; i < 4; i++)
		c->data.f32[27 * 25 * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 4; y++)
		for (x = 0; x < 4; x++)
			sum += (y * 5 + x + 2) * ((y + 23) * 27 + x + 1);
	for (i = 0; i < 4; i++)
		c->data.f32[(27 * 25 + 1) * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 4; y++)
		for (x = 0; x < 5; x++)
			sum += (y * 5 + x + 1) * ((y + 23) * 27 + x + 1);
	for (x = 2; x < 25; x++)
		for (i = 0; i < 4; i++)
			c->data.f32[(27 * 25 + x) * 4 + i] = sum + (x - 2) * 21 * 20 / 2;
	sum = 0;
	for (y = 0; y < 4; y++)
		for (x = 0; x < 4; x++)
			sum += (y * 5 + x + 1) * ((y + 23) * 27 + x + 24);
	for (i = 0; i < 4; i++)
		c->data.f32[(27 * 25 + 25) * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 4; y++)
		for (x = 0; x < 3; x++)
			sum += (y * 5 + x + 1) * ((y + 23) * 27 + x + 25);
	for (i = 0; i < 4; i++)
		c->data.f32[(27 * 25 + 26) * 4 + i] = sum;
	// the last column
	sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 3; x++)
			sum += (y * 5 + x + 3) * ((y + 24) * 27 + x + 1);
	for (i = 0; i < 4; i++)
		c->data.f32[27 * 26 * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 4; x++)
			sum += (y * 5 + x + 2) * ((y + 24) * 27 + x + 1);
	for (i = 0; i < 4; i++)
		c->data.f32[(27 * 26 + 1) * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 5; x++)
			sum += (y * 5 + x + 1) * ((y + 24) * 27 + x + 1);
	for (x = 2; x < 25; x++)
		for (i = 0; i < 4; i++)
			c->data.f32[(27 * 26 + x) * 4 + i] = sum + (x - 2) * 16 * 15 / 2;
	sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 4; x++)
			sum += (y * 5 + x + 1) * ((y + 24) * 27 + x + 24);
	for (i = 0; i < 4; i++)
		c->data.f32[(27 * 26 + 25) * 4 + i] = sum;
	sum = 0;
	for (y = 0; y < 3; y++)
		for (x = 0; x < 3; x++)
			sum += (y * 5 + x + 1) * ((y + 24) * 27 + x + 25);
	for (i = 0; i < 4; i++)
		c->data.f32[(27 * 26 + 26) * 4 + i] = sum;
	float border[] = {
		0, 0, 0, 0
	};
	for (y = 0; y < 5; y++)
		for (x = 0; x < 3; x++)
			border[0] += (y * 5 + x + 3) * (y * 27 + x + 1);
	for (y = 0; y < 5; y++)
		for (x = 0; x < 4; x++)
			border[1] += (y * 5 + x + 2) * (y * 27 + x + 1);
	for (y = 0; y < 5; y++)
		for (x = 0; x < 4; x++)
			border[2] += (y * 5 + x + 1) * (y * 27 + x + 24);
	for (y = 0; y < 5; y++)
		for (x = 0; x < 3; x++)
			border[3] += (y * 5 + x + 1) * (y * 27 + x + 25);
	sum = 0;
	for (y = 0; y < 5; y++)
		for (x = 0; x < 5; x++)
			sum += (y * 5 + x + 1) * (y * 27 + x + 1);
	for (y = 2; y < 25; y++)
	{
		for (i = 0; i < 4; i++)
		{
			c->data.f32[y * 27 * 4 + i] = border[0] + (y - 2) * 27 * (3 + 4 + 5 + 8 + 9 + 10 + 13 + 14 + 15 + 18 + 19 + 20 + 23 + 24 + 25);
			c->data.f32[(y * 27 + 1) * 4 + i] = border[1] + (y - 2) * 27 * (2 + 3 + 4 + 5 + 7 + 8 + 9 + 10 + 12 + 13 + 14 + 15 + 17 + 18 + 19 + 20 + 22 + 23 + 24 + 25);
			for (x = 2; x < 25; x++)
				c->data.f32[(y * 27 + x) * 4 + i] = sum + ((y - 2) * 27 + x - 2) * 26 * 25 / 2;
			c->data.f32[(y * 27 + 25) * 4 + i] = border[2] + (y - 2) * 27 * (1 + 2 + 3 + 4 + 6 + 7 + 8 + 9 + 11 + 12 + 13 + 14 + 16 + 17 + 18 + 19 + 21 + 22 + 23 + 24);
			c->data.f32[(y * 27 + 26) * 4 + i] = border[3] + (y - 2) * 27 * (1 + 2 + 3 + 6 + 7 + 8 + 11 + 12 + 13 + 16 + 17 + 18 + 21 + 22 + 23);
		}
	}
	REQUIRE_MATRIX_EQ(b, c, "27x27 matrix should be exactly the same");
	ccv_matrix_free(b);
	ccv_matrix_free(c);
	ccv_convnet_free(convnet);
}

int test13_convNetwork2() {
    printf("Example 13, convolutional network of 5x5x4 on 27x27x8 partitioned by 2\n");
    ccv_convnet_layer_param_t params = {
		.type = CCV_CONVNET_CONVOLUTIONAL,
		.bias = 0,
		.glorot = sqrtf(2),
		.input = {
			.matrix = {
				.rows = 27,
				.cols = 27,
				.channels = 4,
				.partition = 2,
			},
		},
		.output = {
			.convolutional = {
				.count = 8,
				.strides = 1,
				.border = 2,
				.rows = 5,
				.cols = 5,
				.channels = 4,
				.partition = 2,
			},
		},
	};
	ccv_convnet_t* convnet = ccv_convnet_new(0, ccv_size(27, 27), &params, 1);
	int i, k;
	for (i = 0; i < convnet->layers->wnum; i++)
		convnet->layers->w[i] = i;
	for (i = 0; i < convnet->layers->net.convolutional.count; i++)
		convnet->layers->bias[i] = i + 1;
	ccv_dense_matrix_t* a = ccv_dense_matrix_new(27, 27, CCV_32F | 4, 0, 0);
	for (i = 0; i < 27 * 27 * 4; i++)
		a->data.f32[i] = 20 - i;
	ccv_dense_matrix_t* b = 0;
	ccv_convnet_encode(convnet, &a, &b, 1);
	ccv_convnet_layer_param_t partitioned_params = {
		.type = CCV_CONVNET_CONVOLUTIONAL,
		.bias = 0,
		.glorot = sqrtf(2),
		.input = {
			.matrix = {
				.rows = 27,
				.cols = 27,
				.channels = 2,
				.partition = 1,
			},
		},
		.output = {
			.convolutional = {
				.count = 4,
				.strides = 1,
				.border = 2,
				.rows = 5,
				.cols = 5,
				.channels = 2,
				.partition = 1,
			},
		},
	};
	ccv_convnet_t* partitioned_convnet = ccv_convnet_new(0, ccv_size(27, 27), &partitioned_params, 1);
	memcpy(partitioned_convnet->layers->w, convnet->layers->w, sizeof(float) * (convnet->layers->wnum / 2));
	memcpy(partitioned_convnet->layers->bias, convnet->layers->bias, sizeof(float) * (convnet->layers->net.convolutional.count / 2));
	ccv_dense_matrix_t* aa = ccv_dense_matrix_new(27, 27, CCV_32F | 2, 0, 0);
	for (i = 0; i < 27 * 27; i++)
		for (k = 0; k < 2; k++)
			aa->data.f32[i * 2 + k] = a->data.f32[i * 4 + k];
	ccv_dense_matrix_t* bb = ccv_dense_matrix_new(27, 27, CCV_32F | 8, 0, 0);
	ccv_dense_matrix_t* cc = 0;
	ccv_convnet_encode(partitioned_convnet, &aa, &cc, 1);
	for (i = 0; i < 27 * 27; i++)
		for (k = 0; k < 4; k++)
			bb->data.f32[i * 8 + k] = cc->data.f32[i * 4 + k];
	memcpy(partitioned_convnet->layers->w, convnet->layers->w + (convnet->layers->wnum / 2), sizeof(float) * (convnet->layers->wnum / 2));
	memcpy(partitioned_convnet->layers->bias, convnet->layers->bias + (convnet->layers->net.convolutional.count / 2), sizeof(float) * (convnet->layers->net.convolutional.count / 2));
	for (i = 0; i < 27 * 27; i++)
		for (k = 0; k < 2; k++)
			aa->data.f32[i * 2 + k] = a->data.f32[i * 4 + 2 + k];
	ccv_convnet_encode(partitioned_convnet, &aa, &cc, 1);
	for (i = 0; i < 27 * 27; i++)
		for (k = 0; k < 4; k++)
			bb->data.f32[i * 8 + 4 + k] = cc->data.f32[i * 4 + k];
	REQUIRE_MATRIX_EQ(b, bb, "27x27x8 matrix computed from convnet with partition and partitioned convnet should be exactly the same");
	ccv_matrix_free(a);
	ccv_matrix_free(b);
	ccv_matrix_free(aa);
	ccv_matrix_free(bb);
	ccv_matrix_free(cc);
	ccv_convnet_free(convnet);
	ccv_convnet_free(partitioned_convnet);
}

int test14_detectText() {
    printf("Example 14, Text detect\n");
    ccv_enable_default_cache();
    ccv_dense_matrix_t* image = 0;
    ccv_read("/mnt/sdcard/samples/text-detect.png", &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);

    //unsigned int elapsed_time = get_current_time();
    ccv_array_t* words = ccv_swt_detect_words(image, ccv_swt_default_params);
    //elapsed_time = get_current_time() - elapsed_time;
    if (words) {
        int i;
        for (i = 0; i < words->rnum; i++) {
            ccv_rect_t* rect = (ccv_rect_t*) ccv_array_get(words, i);
            printf("%d %d %d %d\n", rect->x, rect->y, rect->width, rect->height);
        }
        printf("total : %d detected \n", words->rnum);
        ccv_array_free(words);
    }
    ccv_matrix_free(image);
    ccv_drain_cache();
}


int test15_detectFaceBBF() {
    printf("Example 15, Detect faces\n");
    int i;
    ccv_enable_default_cache();
    ccv_dense_matrix_t* image = 0;
    ccv_bbf_classifier_cascade_t* cascade = ccv_bbf_read_classifier_cascade("/mnt/sdcard/samples/face");
    ccv_read("/mnt/sdcard/samples/suit.png", &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);
    //unsigned int elapsed_time = get_current_time();
    ccv_array_t* seq = ccv_bbf_detect_objects(image, &cascade, 1, ccv_bbf_default_params);
    //elapsed_time = get_current_time() - elapsed_time;
    for (i = 0; i < seq->rnum; i++) {
        ccv_comp_t* comp = (ccv_comp_t*) ccv_array_get(seq, i);
        printf("%d %d %d %d %f\n", comp->rect.x, comp->rect.y, comp->rect.width, comp->rect.height, comp->classification.confidence);
    }
    printf("total : %d detected \n", seq->rnum);
    ccv_array_free(seq);
    ccv_matrix_free(image);
    ccv_bbf_classifier_cascade_free(cascade);
    ccv_disable_cache();
}
    


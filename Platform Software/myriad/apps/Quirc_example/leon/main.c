#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rtems.h>

#include <mv_types.h>

#include "rtems_config.h"
#include "app_config.h"

#include <SDCardIO.h>
#include <png.h>

#include <rtems/cpuuse.h>

#include "quirc.h"

// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// 4: Static Local Data
// ----------------------------------------------------------------------------
/* Sections decoration is require here for downstream tools */
// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
// 6: Functions Implementation
// ----------------------------------------------------------------------------

char * readQR (struct quirc *qr) 
{
	struct timeval currentTimeReadQR;
    struct timeval previousTimeReadQR;
    int previousUsecReadQR = 0;
    int currentUsecReadQR = 0;

	gettimeofday(&previousTimeReadQR, NULL);
    previousUsecReadQR = previousTimeReadQR.tv_sec * 1000000 + previousTimeReadQR.tv_usec;

	int num_codes;
	int i;
	char * output = "";
	*output = 0;
	num_codes = quirc_count(qr);
	printf("num_codes: %d\n", num_codes);

	for (i=0; i<num_codes; i++) 
	{
		struct quirc_code code;
		struct quirc_data data;

		quirc_decode_error_t err;

		quirc_extract(qr, i, &code);
		err = quirc_decode(&code, &data);
		strcat(output, "Data: ");
		if (err) {
			strcat(output, quirc_strerror(err));
		}
		else {
			strcat(output, data.payload);
		}
		strcat(output, " ");
	}

	gettimeofday(&currentTimeReadQR,NULL);
    currentUsecReadQR = currentTimeReadQR.tv_sec*1000000+currentTimeReadQR.tv_usec;
    
    printf("Time in readQR: %d \n",currentUsecReadQR - previousUsecReadQR);

	return output;
}

void createPNG (png_structp *png, png_infop *info, FILE *file) 
{

	int color;
	int bits;

	*png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png) 
		exit(3);
	
	if(setjmp(png_jmpbuf(*png))) exit(4);

	*info = png_create_info_struct(*png);

	if (!info) 
		exit(5);
	
	png_init_io (*png, file);
	png_read_info(*png, *info);

	color = png_get_color_type (*png, *info);
	bits = png_get_bit_depth (*png, *info);

	 if(color & PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(*png);
    if(color == PNG_COLOR_TYPE_GRAY && bits < 8)
        png_set_expand_gray_1_2_4_to_8(*png);
    if(bits == 16)
        png_set_strip_16(*png);
    if(color & PNG_COLOR_MASK_ALPHA)
        png_set_strip_alpha(*png);
    if(color & PNG_COLOR_MASK_COLOR)
        png_set_rgb_to_gray_fixed(*png, 1, -1, -1);
}

void loadCode(const char *path, struct quirc **qr) 
{

	struct timeval currentTimeLoadCode;
    struct timeval previousTimeLoadCode;
    int previousUsecLoadCode = 0;
    int currentUsecLoadCode = 0;

	gettimeofday(&previousTimeLoadCode, NULL);
    previousUsecLoadCode = previousTimeLoadCode.tv_sec * 1000000 + previousTimeLoadCode.tv_usec;

	uint8_t *raw;
    int width = 0;
    int height = 0;
    int i;

    png_structp png;
    png_infop info;

    *qr = quirc_new();

	FILE *file = fopen(path, "rb");
	if (!file) 
		exit(2);

	createPNG (&png, &info, file);

    /* allocate image */

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);

    if (quirc_resize(*qr, width, height) < 0) 
    	exit (6);
	
	raw = quirc_begin (*qr, &width, &height);
    png_bytep rows[height];

    for(i = 0; i < height; i++)
        rows[i] = raw + (width * i);
    png_read_image(png, rows);
    
    quirc_end(*qr);
    png_destroy_read_struct (&png, &info, (png_infopp)NULL);
	fclose(file);

    gettimeofday(&currentTimeLoadCode,NULL);
    currentUsecLoadCode = currentTimeLoadCode.tv_sec*1000000+currentTimeLoadCode.tv_usec;
    
    printf("Time in loadCode: %d \n",currentUsecLoadCode - previousUsecLoadCode);
    

}

void test1_TestQR (const char *path) 
{
	struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;

	gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

	printf ("Test 1\n");
	struct quirc *qr;
	loadCode (path, &qr);
	char * data = readQR (qr);
	
	printf ("%s\n", data);
	int i = 0;
	quirc_destroy (qr);

	gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 1 Usec CPU time: %d \n",currentUsec - previousUsec);
	printf ("----------------\n\n");

	
}

void test2_TestText (const char *path) 
{

	struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;

	gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

	printf ("Test 2\n");
	struct quirc *qr;
	loadCode (path, &qr);
	
	printf ("%s\n", readQR (qr));
	quirc_destroy (qr);

	gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 2 Usec CPU time: %d \n",currentUsec - previousUsec);
	printf ("----------------\n\n");
}

void test3_Test500x500 (const char *path) 
{

	struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;

	gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

	printf ("Test 3\n");
	struct quirc *qr;
	loadCode (path, &qr);
	printf ("%s\n", readQR (qr));
	quirc_destroy (qr);

	gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 3 Usec CPU time: %d \n",currentUsec - previousUsec);
	printf ("----------------\n\n");
}

void test4_TestRotated (const char *path) 
{

	struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;

	gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

	printf ("Test 4\n");
	struct quirc *qr;
	loadCode (path, &qr);
	printf ("%s\n", readQR (qr));
	quirc_destroy (qr);

	gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 4 Usec CPU time: %d \n",currentUsec - previousUsec);
	printf ("----------------\n\n");
}

void test5_TestRotatedError (const char *path) 
{

	struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;

	gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

	printf ("Test 5\n");
	struct quirc *qr;
	loadCode (path, &qr);
	printf ("%s\n", readQR (qr));
	quirc_destroy (qr);

	gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 5 Usec CPU time: %d \n",currentUsec - previousUsec);

	printf ("----------------\n\n");
}

void test6_TestRedCode (const char *path) 
{

	struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;

	gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

	printf ("Test 6\n");
	struct quirc *qr;
	loadCode (path, &qr);
	printf ("%s\n", readQR (qr));
	quirc_destroy (qr);

	gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 6 Usec CPU time: %d \n",currentUsec - previousUsec);

	printf ("----------------\n\n");
}

void test7_TestTwoQR (const char *path) 
{

	struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;

	gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

	printf ("Test 7\n");
	struct quirc *qr;
	loadCode (path, &qr);
	printf ("%s\n", readQR (qr));
	quirc_destroy (qr);

	gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 7 Usec CPU time: %d \n",currentUsec - previousUsec);

	printf ("----------------\n\n");
}

void test8_TestTooSmall (const char *path) 
{

	struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;

	gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

	printf ("Test 8\n");
	struct quirc *qr;
	loadCode (path, &qr);
	printf ("%s\n", readQR (qr));
	quirc_destroy (qr);

	gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 8 Usec CPU time: %d \n",currentUsec - previousUsec);

	printf ("----------------\n\n");
}

void POSIX_Init(void *args)  
{

	initClocksAndMemory();
	
	SDCardMount();
	if (SDCardIsMounted () == false) {
		exit(1);
	}
	//Example 1 (TestQR)
	test1_TestQR ("mnt/sdcard/QuircTest/TestQR.png");
	//Example 2 (TestText)
	test2_TestText ("mnt/sdcard/QuircTest/TestText.png");
	//Example 3 (Test500x500)
	test3_Test500x500 ("mnt/sdcard/QuircTest/Test500x500.png");
	//Example 4 (TestRotated)
	test4_TestRotated ("mnt/sdcard/QuircTest/TestRotated.png");
	//Example 5 (TestRotatedError)
	test5_TestRotatedError ("mnt/sdcard/QuircTest/TestRotatedError.png");
	//Example 6 (TestRedCode)
	test6_TestRedCode ("mnt/sdcard/QuircTest/TestRedCode.png");
	//Example 7 (TestTwoQR)
	test7_TestTwoQR ("mnt/sdcard/QuircTest/TestTwoQR.png");
	//Example 8 (TestTooSmall)
	test8_TestTooSmall ("mnt/sdcard/QuircTest/TestTooSmall.png");

    SDCardUnmount();
	
	exit(0);
}


void Fatal_extension(
	Internal_errors_Source	the_source,
	bool 					is_internal,
	uint32_t				the_error
)
{
	switch(the_source) {
	case RTEMS_FATAL_SOURCE_EXIT:
		if (the_error)
			printk("Exited with error code %d\n", the_error);
		break;
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
		printk("\nSource %d Internal %d Error %d 0x%X:\n", 
			the_source, is_internal, the_error, the_error);
		break;
	}
}
#include "HistogramMatchingUnitTest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <ColorHistogram.h>
#include <HistogramMatching.h>

#define FLT_EPSILON 0.000001 // decimal constant

static void setUp( void ) {
// setup
}

static void tearDown( void ) {
// tearDown
}

static void testHistogramOne( void ) {
	frameBuffer image = { {RGB888, 8, 8, 8, 1}, NULL, NULL, NULL };
    image.p1 = malloc(image.spec.width * image.spec.height);
    image.p2 = malloc(image.spec.width * image.spec.height);
    image.p3 = malloc(image.spec.width * image.spec.height);
    memset(image.p1, 0, image.spec.width * image.spec.height);
    memset(image.p2, 64, image.spec.width * image.spec.height);
    memset(image.p3, 128, image.spec.width * image.spec.height);

	u8 bins[3] = {4, 4, 4};
    int histogramBuffer[3*4];

	computeHistogram(&image, &histogramBuffer[0], bins);

	TEST_ASSERT_EQUAL_INT( 64, histogramBuffer[4 * 0 + 0] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer[4 * 0 + 1] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer[4 * 0 + 2] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer[4 * 0 + 3] );
	TEST_ASSERT_EQUAL_INT( 0, histogramBuffer[4 * 1 + 0] ); TEST_ASSERT_EQUAL_INT( 64, histogramBuffer[4 * 1 + 1] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer[4 * 1 + 2] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer[4 * 1 + 3] );
	TEST_ASSERT_EQUAL_INT( 0, histogramBuffer[4 * 2 + 0] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer[4 * 2 + 1] ); TEST_ASSERT_EQUAL_INT( 64, histogramBuffer[4 * 2 + 2] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer[4 * 2 + 3] );
}
static void testHistogramTwo( void ) {
	frameBuffer image2 = { {RGB888, 8, 8, 8, 1}, NULL, NULL, NULL };
    image2.p1 = malloc(image2.spec.width * image2.spec.height);
    image2.p2 = malloc(image2.spec.width * image2.spec.height);
    image2.p3 = malloc(image2.spec.width * image2.spec.height);
    memset(image2.p1, 128, image2.spec.width * image2.spec.height);
    memset(image2.p2, 64, image2.spec.width * image2.spec.height);
    memset(image2.p3, 128, image2.spec.width * image2.spec.height);

	u8 bins[3] = {4, 4, 4};
    int histogramBuffer2[3*4];

	computeHistogram(&image2, &histogramBuffer2[0], bins);

	TEST_ASSERT_EQUAL_INT( 0, histogramBuffer2[4 * 0 + 0] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer2[4 * 0 + 1] ); TEST_ASSERT_EQUAL_INT( 64, histogramBuffer2[4 * 0 + 2] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer2[4 * 0 + 3] );
	TEST_ASSERT_EQUAL_INT( 0, histogramBuffer2[4 * 1 + 0] ); TEST_ASSERT_EQUAL_INT( 64, histogramBuffer2[4 * 1 + 1] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer2[4 * 1 + 2] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer2[4 * 1 + 3] );
	TEST_ASSERT_EQUAL_INT( 0, histogramBuffer2[4 * 2 + 0] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer2[4 * 2 + 1] ); TEST_ASSERT_EQUAL_INT( 64, histogramBuffer2[4 * 2 + 2] ); TEST_ASSERT_EQUAL_INT( 0, histogramBuffer2[4 * 2 + 3] );
}
static void testEMD( void ) {
	frameBuffer image = { {RGB888, 8, 8, 8, 1}, NULL, NULL, NULL };
    image.p1 = malloc(image.spec.width * image.spec.height);
    image.p2 = malloc(image.spec.width * image.spec.height);
    image.p3 = malloc(image.spec.width * image.spec.height);
    memset(image.p1, 0, image.spec.width * image.spec.height);
    memset(image.p2, 64, image.spec.width * image.spec.height);
    memset(image.p3, 128, image.spec.width * image.spec.height);

    frameBuffer image2 = { {RGB888, 8, 8, 8, 1}, NULL, NULL, NULL };
    image2.p1 = malloc(image.spec.width * image.spec.height);
    image2.p2 = malloc(image.spec.width * image.spec.height);
    image2.p3 = malloc(image.spec.width * image.spec.height);
    memset(image2.p1, 128, image.spec.width * image.spec.height);
    memset(image2.p2, 64, image.spec.width * image.spec.height);
    memset(image2.p3, 128, image.spec.width * image.spec.height);

    u8 bins[3] = {4, 4, 4};
    int histogramBuffer[3*4];
    int histogramBuffer2[3*4];

    computeHistogram(&image, &histogramBuffer[0], bins);
    computeHistogram(&image2, &histogramBuffer2[0], bins);

	u16 totalBins = 12;
    float score = earthMoversDistance(totalBins, histogramBuffer, histogramBuffer2);
	TEST_ASSERT( fabsf(score-0.666667) < FLT_EPSILON );
}

TestRef Histogram_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("testHistogramOne", testHistogramOne),
        new_TestFixture("testHistogramTwo", testHistogramTwo),
		new_TestFixture("testEarthMoversDistanceTwo", testEMD)
    };
    EMB_UNIT_TESTCALLER(HistogramTest, "HistogramTest", setUp, tearDown, fixtures);
    return (TestRef)&HistogramTest;
}


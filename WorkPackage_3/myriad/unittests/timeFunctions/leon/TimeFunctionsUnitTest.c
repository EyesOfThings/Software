#include "TimeFunctionsUnitTest.h"

#include <stdio.h>
#include <string.h>

static void setUp( void ) {
//setup
}

static void tearDown( void ) {
// tearDown
}

static void testGetSetGetTime( void ) {
    struct tm timestamp = getCurrentTime();

    TEST_ASSERT( timestamp.tm_hour == 0 );

    setCurrentTime(3, 3, 3, 3, 3, 3);

    struct tm timestamp2 = getCurrentTime();

    TEST_ASSERT( timestamp2.tm_hour == 3 );

    printf("test Get, Set, Get Time OK\n");
}

TestRef TimeFunction_set_get_time_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("testGetSetGetTime", testGetSetGetTime),
    };

    EMB_UNIT_TESTCALLER(TimeFunctionProfilesTest, "TimeFunctionProfilesTest", setUp, tearDown, fixtures);
    return (TestRef)&TimeFunctionProfilesTest;
}

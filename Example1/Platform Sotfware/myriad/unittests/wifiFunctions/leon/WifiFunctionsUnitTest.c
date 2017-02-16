#include "WifiFunctionsUnitTest.h"

#include <stdio.h>
#include <string.h>

static void setUp( void ) {
//setup
}

static void tearDown( void ) {
// tearDown
}

static void testSaveRestoreProfile( void ) {
    /*
     * Only works with WPA/WPA2
     */
    _i16 retVal = 0;
    WifiConnectionState profile = {
        .ssid_name = "testProfile",
        .password = "123456789",
        .security = SL_SEC_TYPE_WPA_WPA2,
        .channel = 3
    };

    retVal = generateAP("test", "123456789", SL_SEC_TYPE_WPA_WPA2, 5);
    TEST_ASSERT( retVal >= 0 );

    retVal = saveProfile(profile.ssid_name, profile.password, 
            profile.security, profile.channel);
    TEST_ASSERT( retVal >= 0 );

    WifiConnectionState profile_readed = {
        .ssid_name = "",
        .password = "",
        .security = 0,
        .channel = 0
    };

    retVal = getProfile(retVal, &profile_readed);
    TEST_ASSERT( retVal >= 0 );

    TEST_ASSERT_EQUAL_INT( profile.channel,  profile_readed.channel );
    TEST_ASSERT_EQUAL_INT( profile.security, profile_readed.security );
    TEST_ASSERT_EQUAL_STRING( profile.ssid_name, profile_readed.ssid_name);
    TEST_ASSERT_EQUAL_STRING( profile.password, profile_readed.password);

    printf("test Save Restore Profile OK\n");
}

static void testRestoreProfileError( void ) {
    _i16 retVal = 0;
    retVal = generateAP("test", "123456789", SL_SEC_TYPE_WPA_WPA2, 5);
    TEST_ASSERT( retVal >= 0 );

    retVal = removeProfiles();
    TEST_ASSERT( retVal >= 0 );

    retVal = restoreProfile(0);
    TEST_ASSERT( retVal < 0 );

    printf("test Save Restore Profile Error OK\n");
}

static void testGenerateAPFromDefaultProfile( void ) {
    _i16 retVal = 0;
    retVal = generateAPFromDefaultProfile();
    TEST_ASSERT( retVal >= 0 );

    printf("test Generate AP From Default Profile OK\n");
}

static void testGenerateAPFromProfileIndex( void ) {
    _i16 retVal = 0;
    int index = 0;
    retVal = generateAPFromProfile(index);
    TEST_ASSERT( retVal >= 0 );

    printf("test Generate AP From Profile Index OK\n");
}

TestRef WifiFunction_profiles_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("testSaveRestoreProfile", testSaveRestoreProfile),
        new_TestFixture("testGenerateAPFromProfileIndex", testGenerateAPFromProfileIndex),
        new_TestFixture("testRestoreProfileError", testRestoreProfileError),
        new_TestFixture("testGenerateAPFromDefaultProfile", testGenerateAPFromDefaultProfile),
    };

    EMB_UNIT_TESTCALLER(WifiFunctionProfilesTest, "WifiFunctionProfilesTest", setUp, tearDown, fixtures);
    return (TestRef)&WifiFunctionProfilesTest;
}

static void testsetModeSTA( void ) {
    TEST_ASSERT( wlanSetMode(ROLE_STA) >= 0 );
    printf("test set mode to Station OK\n");
}

static void testsetModeAP( void ) {
    TEST_ASSERT( wlanSetMode(ROLE_AP) >= 0 );
    printf("test set mode to AP OK\n");
}

TestRef WifiFunction_mode_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("testsetModeSTA", testsetModeSTA),
        new_TestFixture("testsetModeAP", testsetModeAP),
    };

    EMB_UNIT_TESTCALLER(WifiFunctionModeTest, "WifiFunctionModeTest", setUp, tearDown, fixtures);
    return (TestRef)&WifiFunctionModeTest;
}
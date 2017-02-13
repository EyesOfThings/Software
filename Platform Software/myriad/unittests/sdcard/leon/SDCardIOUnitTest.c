#include "SDCardIOUnitTest.h"

#include <stdio.h>
#include <string.h>

#include <SDCardIO.h>

static const char* loremIpsum =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Suspendisse porta eleifend orci ac bibendum. "
        "Nunc ac tortor rutrum turpis facilisis hendrerit luctus ut mi. "
        "Mauris pulvinar quam nisi. "
        "Proin ex lectus, convallis vitae odio ac, scelerisque condimentum risus. "
        "Cras facilisis elementum est, ut luctus ligula fermentum quis. "
        "Nunc vitae luctus augue, nec pulvinar nisl. "
        "Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. "
        "Nullam congue magna et lacus placerat, id metus.";

static void setUp( void ) {
// setup
}

static void tearDown( void ) {
// tearDown
}

static void testMountSDCard( void ) {
    bool boolValue = false;
    // sd card is not mounted: expect false
    boolValue = SDCardIsMounted();
    TEST_ASSERT( false == boolValue );
    // mount sd card: expect true
    boolValue = SDCardMount();
    TEST_ASSERT( true == boolValue );
    // is already mounted: expect false
    boolValue = SDCardMount();
    TEST_ASSERT( false == boolValue );
    // sd card is mounted: expect true
    boolValue = SDCardIsMounted();
    TEST_ASSERT( true == boolValue );
    // unmount sd card: expect true
    boolValue = SDCardUnmount();
    TEST_ASSERT( true == boolValue );
    // is already unmounted: expect false
    boolValue = SDCardUnmount();
    TEST_ASSERT( false == boolValue );
    // sd card is not mounted: expect false
    boolValue = SDCardIsMounted();
    TEST_ASSERT( false == boolValue );

    // mount sd card: expect true
    boolValue = SDCardMount();
    TEST_ASSERT( true == boolValue );
    // unmount sd card: expect true
    boolValue = SDCardUnmount();
    TEST_ASSERT( true == boolValue );
}
TestRef SDCardMount_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("testMountSDCard", testMountSDCard),
    };
    EMB_UNIT_TESTCALLER(SDCardMountTest, "SDCardMountTest", setUp, tearDown, fixtures);
    return (TestRef)&SDCardMountTest;
}

static void testCreateDirectories( void ) {
    bool boolValue = false;
    // create a DIR handler: expect not a NULL pointer
    SDCardDir* dirHandler = SDCardDirOpen();
    TEST_ASSERT_NOT_NULL( dirHandler );
    // create "TestDir" folder: expect true
    boolValue = SDCardDirMakeDirectoryWithDir( dirHandler, "TestDir" );
    TEST_ASSERT( true == boolValue );
    // create "Hello" folder: expect true
    boolValue = SDCardDirMakeDirectoryWithDir( dirHandler, "Hello" );
    TEST_ASSERT( true == boolValue );
    // try to create a already existing folder: expect false
    boolValue = SDCardDirMakeDirectoryWithDir( dirHandler, "Hello" );
    TEST_ASSERT( false == boolValue );
    // close DIR handler: expect a null pointer
    boolValue = SDCardDirClose( &dirHandler );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( dirHandler );
    // create a DIR handler: expect not a NULL pointer
    SDCardDir* dirHandler2  =  SDCardDirOpen();
    TEST_ASSERT_NOT_NULL( dirHandler2 );
    // create "World" folder: expect true
    boolValue = SDCardDirMakeDirectoryWithDir( dirHandler2, "World" );
    TEST_ASSERT( true == boolValue );
    // close DIR handler: expect a null pointer
    boolValue = SDCardDirClose( &dirHandler2 );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( dirHandler2 );
}
static void validateTestCreateDirectories( void ) {
    bool boolValue = false;
    // create a DIR handler: expect not a NULL pointer
    SDCardDir* dirHandler  =  SDCardDirOpen();
    TEST_ASSERT_NOT_NULL( dirHandler );
    // check if "TestDir" exists: expect true
    boolValue = SDCardDirExists(dirHandler, "TestDir");
    TEST_ASSERT( true == boolValue );
    // check if "Hello" exists:  expect true
    boolValue = SDCardDirExists(dirHandler, "Hello");
    TEST_ASSERT( true == boolValue );
    // check if "World" exists:  expect true
    boolValue = SDCardDirExists(dirHandler, "World");
    TEST_ASSERT( true == boolValue );
    // remove "TestDir": expect  true
    boolValue = SDCardDirRemoveDirectoryWithDir(dirHandler, "TestDir");
    TEST_ASSERT( true == boolValue );
    // remove "Hello": expect true
    boolValue = SDCardDirRemoveDirectoryWithDir(dirHandler, "Hello");
    TEST_ASSERT( true == boolValue );
    // remove "World": expect true
    boolValue = SDCardDirRemoveDirectoryWithDir(dirHandler, "World");
    TEST_ASSERT( true == boolValue );
    // try to remove "World": expect false -> is already removed
    boolValue = SDCardDirRemoveDirectoryWithDir(dirHandler, "World");
    TEST_ASSERT( false == boolValue );
    // close DIR handler: expect a null pointer
    boolValue = SDCardDirClose( &dirHandler );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( dirHandler );
}
static void testCreateContent( void ) {
    bool boolValue = false;
    u32 unsignedIntValue = 0;
    // sd card is not mounted: expect false
    boolValue = SDCardIsMounted();
    TEST_ASSERT( false == boolValue );
    // create a DIR handler: expect not a NULL pointer
    SDCardDir* dirHandler  =  SDCardDirOpen();
    TEST_ASSERT_NOT_NULL( dirHandler );
    //
    // create content
    //
    // mkdir Crypto
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "crypto");
    TEST_ASSERT( true == boolValue );
    // mkdir data
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "data");
    TEST_ASSERT( true == boolValue );
    // mkdir tmp
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "tmp");
    TEST_ASSERT( true == boolValue );
    // create readme file
    {
        SDCardFile* fileHandler = SDCardFileOpenWithDir( dirHandler, "readme", "w", false );
        TEST_ASSERT_NOT_NULL(fileHandler);
        unsignedIntValue = SDCardFileWrite( fileHandler, loremIpsum, strlen(loremIpsum) );
        TEST_ASSERT( unsignedIntValue == strlen(loremIpsum) );
        boolValue = SDCardFileClose( &fileHandler );
        TEST_ASSERT( true == boolValue );
        TEST_ASSERT_NULL( fileHandler );
    }
    // cd ./Crypto
    boolValue = SDCardDirCd(dirHandler, "crypto");
    TEST_ASSERT( true == boolValue );
    // mkdir foo
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "foo");
    TEST_ASSERT( true == boolValue );
    // mkdir data
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "data");
    TEST_ASSERT( true == boolValue );
    // create Hello.txt file
    {
        SDCardFile* fileHandler = SDCardFileOpenWithDir( dirHandler, "hello.txt", "w", true );
        TEST_ASSERT_NOT_NULL(fileHandler);
        unsignedIntValue = SDCardFileWrite( fileHandler, "hello!!!!", 9 );
        TEST_ASSERT( unsignedIntValue == 9 );
        boolValue = SDCardFileClose( &fileHandler );
        TEST_ASSERT( true == boolValue );
        TEST_ASSERT_NULL( fileHandler );
    }
    // create World.txt file
    {
        SDCardFile* fileHandler = SDCardFileOpenWithDir( dirHandler, "world.txt", "w", true );
        TEST_ASSERT_NOT_NULL(fileHandler);
        unsignedIntValue = SDCardFileWrite( fileHandler, "world!!!!", 9 );
        TEST_ASSERT( unsignedIntValue == 9 );
        boolValue = SDCardFileClose( &fileHandler );
        TEST_ASSERT( true == boolValue );
        TEST_ASSERT_NULL( fileHandler );
    }
    // cd ./data
    boolValue = SDCardDirCd(dirHandler, "data");
    TEST_ASSERT( true == boolValue );
    // mkdir foo
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "foo");
    TEST_ASSERT( true == boolValue );
    // create hello.txt file
    {
        SDCardFile* fileHandler = SDCardFileOpenWithDir( dirHandler, "hello.txt", "w", true );
        TEST_ASSERT_NOT_NULL(fileHandler);
        unsignedIntValue = SDCardFileWrite( fileHandler, "hello!!!!", 9 );
        TEST_ASSERT( unsignedIntValue == 9 );
        boolValue = SDCardFileClose( &fileHandler );
        TEST_ASSERT( true == boolValue );
        TEST_ASSERT_NULL( fileHandler );
    }
    // create world.txt file
    {
        SDCardFile* fileHandler = SDCardFileOpenWithDir( dirHandler, "world.txt", "w", true );
        TEST_ASSERT_NOT_NULL(fileHandler);
        unsignedIntValue = SDCardFileWrite( fileHandler, "world!!!!", 9 );
        TEST_ASSERT( unsignedIntValue == 9 );
        boolValue = SDCardFileClose( &fileHandler );
        TEST_ASSERT( true == boolValue );
        TEST_ASSERT_NULL( fileHandler );
    }
    // cd ../../data
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( true == boolValue );
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( true == boolValue );
    boolValue = SDCardDirCd(dirHandler, "data");
    TEST_ASSERT( true == boolValue );
    // create hello.txt file
    {
        SDCardFile* fileHandler = SDCardFileOpenWithDir( dirHandler, "hello.txt", "w", false );
        TEST_ASSERT_NOT_NULL(fileHandler);
        unsignedIntValue = SDCardFileWrite( fileHandler, "hello!!!!", 9 );
        TEST_ASSERT( unsignedIntValue == 9 );
        boolValue = SDCardFileClose( &fileHandler );
        TEST_ASSERT( true == boolValue );
        TEST_ASSERT_NULL( fileHandler );
    }
    // create world.txt file
    {
        SDCardFile* fileHandler = SDCardFileOpenWithDir( dirHandler, "world.txt", "w", false );
        TEST_ASSERT_NOT_NULL(fileHandler);
        unsignedIntValue = SDCardFileWrite( fileHandler, "world!!!!", 9 );
        TEST_ASSERT( unsignedIntValue == 9 );
        boolValue = SDCardFileClose( &fileHandler );
        TEST_ASSERT( true == boolValue );
        TEST_ASSERT_NULL( fileHandler );
    }
    // cd ../tmp
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( true == boolValue );
    boolValue = SDCardDirCd(dirHandler, "tmp");
    TEST_ASSERT( true == boolValue );
    // mkdir foo
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "foo");
    TEST_ASSERT( true == boolValue );
    // cd ./foo
    boolValue = SDCardDirCd(dirHandler, "foo");
    TEST_ASSERT( true == boolValue );
    // mkdir bar
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "bar");
    TEST_ASSERT( true == boolValue );
    // mkdir foo
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "foo");
    TEST_ASSERT( true == boolValue );
    // mkdir EoT
    boolValue = SDCardDirMakeDirectoryWithDir(dirHandler, "eot");
    TEST_ASSERT( true == boolValue );
    // close DIR handler: expect a null pointer
    boolValue = SDCardDirClose( &dirHandler );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( dirHandler );
}
static void testRemoveContent( void ) {
    bool boolValue = false;
    u32 unsignedIntValue = 0;
    // sd card is not mounted: expect false
    boolValue = SDCardIsMounted();
    TEST_ASSERT( false == boolValue );
    // create a DIR handler: expect not a NULL pointer
    SDCardDir* dirHandler = SDCardDirOpen();
    TEST_ASSERT_NOT_NULL( dirHandler );
    // try to remove tmp: expect false, because folder is not empty
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "tmp" );
    TEST_ASSERT( false == boolValue );
    // rm ./readme
    boolValue = SDCardFileRemoveWithDir( dirHandler, "readme");
    TEST_ASSERT( true == boolValue );
    // cd ./Crypto
    boolValue = SDCardDirCd( dirHandler, "crypto" );
    TEST_ASSERT( true == boolValue );
    // rm ./Hello.txt
    boolValue = SDCardFileRemoveWithDir( dirHandler, "hello.txt");
    TEST_ASSERT( true == boolValue );
    // rm ./World.txt
    boolValue = SDCardFileRemoveWithDir( dirHandler, "world.txt");
    TEST_ASSERT( true == boolValue );
    // rm -r ./foo
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "foo" );
    TEST_ASSERT( true == boolValue );
    // cd ./data
    boolValue = SDCardDirCd( dirHandler, "data" );
    TEST_ASSERT( true == boolValue );
    // rm -r ./foo
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "foo" );
    TEST_ASSERT( true == boolValue );
    // rm ./hello.txt
    boolValue = SDCardFileRemoveWithDir( dirHandler, "hello.txt");
    TEST_ASSERT( true == boolValue );
    // rm ./world.txt
    boolValue = SDCardFileRemoveWithDir( dirHandler, "world.txt");
    TEST_ASSERT( true == boolValue );
    // cd ..
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( true == boolValue );
    // rm -r ./data
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "data" );
    TEST_ASSERT( true == boolValue );
    // cd ..
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( true == boolValue );
    // rm -r Crypto
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "crypto" );
    TEST_ASSERT( true == boolValue );
    // cd ./data
    boolValue = SDCardDirCd( dirHandler, "data" );
    TEST_ASSERT( true == boolValue );
    // rm ./hello.txt
    boolValue = SDCardFileRemoveWithDir( dirHandler, "hello.txt");
    TEST_ASSERT( true == boolValue );
    // rm ./world.txt
    boolValue = SDCardFileRemoveWithDir( dirHandler, "world.txt");
    TEST_ASSERT( true == boolValue );
    // cd ..
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( true == boolValue );
    // rm -r ./data
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "data" );
    TEST_ASSERT( true == boolValue );
    // cd ./tmp
    boolValue = SDCardDirCd( dirHandler, "tmp" );
    TEST_ASSERT( true == boolValue );
    // cd ./foo
    boolValue = SDCardDirCd( dirHandler, "foo" );
    TEST_ASSERT( true == boolValue );
    // rm -r ./bar
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "bar" );
    TEST_ASSERT( true == boolValue );
    // rm -r ./foo
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "foo" );
    TEST_ASSERT( true == boolValue );
    // rm -r ./EoT
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "eot" );
    TEST_ASSERT( true == boolValue );
    // cd ..
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( true == boolValue );
    // rm -r ./foo
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "foo" );
    TEST_ASSERT( true == boolValue );
    // cd ..
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( true == boolValue );
    // rm -r ./tmp
    boolValue = SDCardDirRemoveDirectoryWithDir( dirHandler, "tmp" );
    TEST_ASSERT( true == boolValue );
    // close DIR handler: expect a null pointer
    boolValue = SDCardDirClose( &dirHandler );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( dirHandler );
}
static void testTraversal( void ) {
    bool boolValue = false;
    u32 unsignedIntValue = 0;
    const char* cStringValue = NULL;
    // sd card is not mounted: expect false
    boolValue = SDCardIsMounted();
    TEST_ASSERT( false == boolValue );
    // create a DIR handler: expect not a NULL pointer
    SDCardDir* dirHandler  =  SDCardDirOpen();
    TEST_ASSERT_NOT_NULL( dirHandler );
    // cd .. : expect false -> is already in the root directory
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( false == boolValue );
    // check name: expect "/"
    cStringValue = SDCardDirGetName( dirHandler );
    TEST_ASSERT_EQUAL_STRING( "/", cStringValue );
    // check path: expect "/mnt/sdcard"
    cStringValue = SDCardDirGetPath( dirHandler );
    TEST_ASSERT_EQUAL_STRING( "/mnt/sdcard", cStringValue );
    // count files: expect 1
    unsignedIntValue = SDCardDirCountFilesWithDir( dirHandler );
    TEST_ASSERT_EQUAL_INT( 1, unsignedIntValue );
    // check file name: expect "readme"
    cStringValue = SDCardDirGetFilename( dirHandler, 0 );
    TEST_ASSERT_EQUAL_STRING( "readme", cStringValue );
    // count directories: expect 3
    unsignedIntValue = SDCardDirCountDirectoriesWithDir( dirHandler );
    TEST_ASSERT_EQUAL_INT( 3, unsignedIntValue );
	bool existsCryptoFolder = false;
	bool existsDataFolder = false;
	bool existsTmpFolder = false;
	for(int i = 0; i < 3; ++i) {
		cStringValue = SDCardDirGetDirectoryName( dirHandler, i );
		if( 0 == strcmp("crypto", cStringValue) ) {
			existsCryptoFolder = true;
		}
		if( 0 == strcmp("data", cStringValue) ) {
			existsDataFolder = true;
		}
		if( 0 == strcmp("tmp", cStringValue) ) {
			existsTmpFolder = true;
		}
	}
    // check directory "Crypto" exists 
	TEST_ASSERT( true == existsCryptoFolder );
	// check directory "data" exists 
	TEST_ASSERT( true == existsDataFolder );
	// check directory "tmp" exists 
	TEST_ASSERT( true == existsTmpFolder );
    // cd ./data
    boolValue = SDCardDirCd( dirHandler, "data" );
    TEST_ASSERT( true == boolValue );
    // count files: expect 2
    unsignedIntValue = SDCardDirCountFilesWithDir( dirHandler );
    TEST_ASSERT_EQUAL_INT( 2, unsignedIntValue );
    // check first filename: expect "hello.txt"
    cStringValue = SDCardDirGetFilename( dirHandler, 0 );
    TEST_ASSERT_EQUAL_STRING( "hello.txt", cStringValue );
    // check second filename: expect "world.txt"
    cStringValue = SDCardDirGetFilename( dirHandler, 1 );
    TEST_ASSERT_EQUAL_STRING( "world.txt", cStringValue );
    // count directories: expect 0
    unsignedIntValue = SDCardDirCountDirectoriesWithDir( dirHandler );
    TEST_ASSERT_EQUAL_INT( 0, unsignedIntValue );
    // cd ..
    boolValue = SDCardDirCdUp( dirHandler );
    TEST_ASSERT( true == boolValue );
    // cd ./tmp
    boolValue = SDCardDirCd( dirHandler, "tmp" );
    TEST_ASSERT( true == boolValue );
    // cd ./foo
    boolValue = SDCardDirCd( dirHandler, "foo" );
    TEST_ASSERT( true == boolValue );
    // count files: expect 0
    unsignedIntValue = SDCardDirCountFilesWithDir( dirHandler );
    TEST_ASSERT_EQUAL_INT( 0, unsignedIntValue );
    // count directories: expect 3
    unsignedIntValue = SDCardDirCountDirectoriesWithDir( dirHandler );
    TEST_ASSERT_EQUAL_INT( 3, unsignedIntValue );
    // check first directory name: expect "bar"
    cStringValue = SDCardDirGetDirectoryName( dirHandler, 0 );
    TEST_ASSERT_EQUAL_STRING( "bar", cStringValue );
    // check second directory name: expect "foo"
    cStringValue = SDCardDirGetDirectoryName( dirHandler, 1 );
    TEST_ASSERT_EQUAL_STRING( "foo", cStringValue );
    // check third directory name: expect "EoT"
    cStringValue = SDCardDirGetDirectoryName( dirHandler, 2 );
    TEST_ASSERT_EQUAL_STRING( "eot", cStringValue );
    // check dir name: expect "foo"
    cStringValue = SDCardDirGetName( dirHandler );
    TEST_ASSERT_EQUAL_STRING( "foo", cStringValue );
    // check path: expect "/mnt/sdcard/tmp/foo"
    cStringValue = SDCardDirGetPath( dirHandler );
    TEST_ASSERT_EQUAL_STRING( "/mnt/sdcard/tmp/foo", cStringValue );

    // close DIR handler: expect a null pointer
    boolValue = SDCardDirClose( &dirHandler );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( dirHandler );
}

TestRef SDCardDir_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("testCreateDirectories", testCreateDirectories),
        new_TestFixture("validateTestCreateDirectories", validateTestCreateDirectories),
        new_TestFixture("testCreateContent", testCreateContent),
        new_TestFixture("testTraversal", testTraversal),
        new_TestFixture("testRemoveContent", testRemoveContent),
    };
    EMB_UNIT_TESTCALLER(SDCardDirectoryTest, "SDCardDirectoryTest", setUp, tearDown, fixtures);
    return (TestRef)&SDCardDirectoryTest;
}

static void testWriteFiles( void ) {
    bool boolValue = false;
    u32 unsignedIntValue = 0;
    SDCardFile* fileHandler = NULL;
    SDCardFile* fileHandler2 = NULL;
    const char* cStringValue = NULL;
    // sd card is not mounted: expect false
    boolValue = SDCardIsMounted();
    TEST_ASSERT( false == boolValue );
    // create a DIR handler: expect not a NULL pointer
    SDCardDir* dirHandler  =  SDCardDirOpen();
    TEST_ASSERT_NOT_NULL( dirHandler );
    // copy /readme file to /tmp/myReadme.txt
    boolValue = SDCardFileCopy( "/mnt/sdcard/readme", "/mnt/sdcard/tmp/myReadme.txt" );
    TEST_ASSERT( true == boolValue );
    // move /tmp/myReadme.txt to /data/loremIpsum.txt
    boolValue = SDCardFileRename( "/mnt/sdcard/tmp/myReadme.txt", "/mnt/sdcard/data/loremIpsum.txt" );
    TEST_ASSERT( true == boolValue );
    // cd ./data
    boolValue = SDCardDirCd( dirHandler, "data" );
    TEST_ASSERT( true == boolValue );
    // create helloWorld.txt file
    fileHandler = SDCardFileOpenWithDir( dirHandler, "helloWorld.txt", "w", false );
    TEST_ASSERT_NOT_NULL( fileHandler );
    // create helloWorld_crypt.txt file
    fileHandler2 = SDCardFileOpenWithDir( dirHandler, "helloWorld_crypt.txt", "w", true );
    TEST_ASSERT_NOT_NULL( fileHandler2 );
    // write content
    unsignedIntValue = SDCardFileWrite( fileHandler, "Hello World :)", 14 );
    TEST_ASSERT_EQUAL_INT( 14, unsignedIntValue );
    unsignedIntValue = SDCardFileWrite( fileHandler2, "Hello World :)", 14 );
    TEST_ASSERT_EQUAL_INT( 14, unsignedIntValue );
    // change position
    boolValue = SDCardFileSetPosition( fileHandler, 11 );
    TEST_ASSERT( true == boolValue );
    boolValue = SDCardFileSetPosition( fileHandler2, 11 );
    TEST_ASSERT( true == boolValue );
    // write content
    unsignedIntValue = SDCardFileWrite( fileHandler, "!!!!", 4 );
    TEST_ASSERT_EQUAL_INT( 4, unsignedIntValue );
    unsignedIntValue = SDCardFileWrite( fileHandler2, "!!!!", 4 );
    TEST_ASSERT_EQUAL_INT( 4, unsignedIntValue );
    // flush content
    boolValue = SDCardFileFlush( fileHandler );
    TEST_ASSERT( true == boolValue );
    boolValue = SDCardFileFlush( fileHandler2 );
    TEST_ASSERT( true == boolValue );
    // close file handler
    boolValue = SDCardFileClose( &fileHandler );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( fileHandler );
    boolValue = SDCardFileClose( &fileHandler2 );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( fileHandler2 );
    // close DIR handler: expect a null pointer
    boolValue = SDCardDirClose( &dirHandler );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( dirHandler );
}
static void testReadFiles( void ) {
    bool boolValue = false;
    u32 unsignedIntValue = 0;
    SDCardFile* fileHandler = NULL;
    SDCardFile* fileHandler2 = NULL;
    const char* cStringValue = NULL;
    char buffer[20] = { 0 };
    char secondBuffer[20] = { 0 };
    // sd card is not mounted: expect false
    boolValue = SDCardIsMounted();
    TEST_ASSERT( false == boolValue );
    // create a DIR handler: expect not a NULL pointer
    SDCardDir* dirHandler  =  SDCardDirOpen();
    TEST_ASSERT_NOT_NULL( dirHandler );
    // cd ./data
    boolValue = SDCardDirCd( dirHandler, "data" );
    TEST_ASSERT( true == boolValue );
    // create helloWorld.txt file handler in read mode
    fileHandler = SDCardFileOpenWithDir( dirHandler, "helloWorld.txt", "r", false );
    TEST_ASSERT_NOT_NULL( fileHandler );
    // create helloWorld_crypt.txt file handler in read mode
    fileHandler2 = SDCardFileOpenWithDir( dirHandler, "helloWorld_crypt.txt", "r", true );
    TEST_ASSERT_NOT_NULL( fileHandler2 );
    // check "helloWorld.txt" file size
    unsignedIntValue = SDCardFileGetSize( fileHandler );
    TEST_ASSERT_EQUAL_INT( 15, unsignedIntValue );
    // check "helloWorld_crypt.txt" file size
    unsignedIntValue = SDCardFileGetSize( fileHandler2 );
    TEST_ASSERT_EQUAL_INT( 15, unsignedIntValue );
    // peek content
    unsignedIntValue = SDCardFilePeek( fileHandler, buffer, 5 );
    TEST_ASSERT_EQUAL_INT( 5, unsignedIntValue );
    TEST_ASSERT_EQUAL_STRING( "Hello", buffer);
    // read content
    unsignedIntValue = SDCardFileRead( fileHandler, buffer, 20 );
    TEST_ASSERT_EQUAL_INT( 15, unsignedIntValue );
    TEST_ASSERT_EQUAL_STRING( "Hello World!!!!", buffer);
    // change the file handler position to 0
    boolValue = SDCardFileSetPosition( fileHandler, 0 );
    TEST_ASSERT( true == boolValue );
    memset( buffer, 0, 20 );
    // read content
    unsignedIntValue = SDCardFileRead( fileHandler, buffer, 5);
    TEST_ASSERT_EQUAL_INT( 5, unsignedIntValue );
    unsignedIntValue = SDCardFileRead( fileHandler2, secondBuffer, 5);
    TEST_ASSERT_EQUAL_INT( 5, unsignedIntValue );
    // compare
    boolValue = 0 == memcmp( buffer, secondBuffer, 5 );
    TEST_ASSERT( true == boolValue );
    // change file handler position
    boolValue = SDCardFileSetPosition( fileHandler, 6 );
    TEST_ASSERT( true == boolValue );
    boolValue = SDCardFileSetPosition( fileHandler2, 6 );
    TEST_ASSERT( true == boolValue );
    // read content
    unsignedIntValue = SDCardFileRead( fileHandler, buffer, 9);
    TEST_ASSERT_EQUAL_INT( 9, unsignedIntValue );
    unsignedIntValue = SDCardFileRead( fileHandler2, secondBuffer, 9);
    TEST_ASSERT_EQUAL_INT( 9, unsignedIntValue );
    // compare
    boolValue = 0 == memcmp( buffer, secondBuffer, 9 );
    TEST_ASSERT( true == boolValue );
    // close file handler
    boolValue = SDCardFileClose( &fileHandler );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( fileHandler );
    boolValue = SDCardFileClose( &fileHandler2 );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( fileHandler2 );
    // remove /data/loremIpsum.txt file
    SDCardFileRemoveWithDir( dirHandler, "loremIpsum.txt");
    // remove /data/helloWorld.txt file
    SDCardFileRemoveWithDir( dirHandler, "helloWorld.txt");
    // remove /data/helloWorld_crypt.txt file
    SDCardFileRemoveWithDir( dirHandler, "helloWorld_crypt.txt");
    // close DIR handler: expect a null pointer
    boolValue = SDCardDirClose( &dirHandler );
    TEST_ASSERT( true == boolValue );
    TEST_ASSERT_NULL( dirHandler );
}
TestRef SDCardFile_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("testCreateContent", testCreateContent),
        new_TestFixture("testWriteFiles", testWriteFiles),
        new_TestFixture("testReadFiles", testReadFiles),
        new_TestFixture("testRemoveContent", testRemoveContent)
    };
    EMB_UNIT_TESTCALLER(SDCardFilesTest, "SDCardFilesTest", setUp, tearDown, fixtures);
    return (TestRef)&SDCardFilesTest;
}


#include "FlashIOUnitTest.h"

#include <stdio.h>
#include <string.h>

#include <DrvEeprom.h>
#include <FlashIO.h>

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
static const char* helloWorld = "Hello World\n";
static const u8 randomContent[4097];

static void setUp( void ) {
// setup
}

static void tearDown( void ) {
// tearDown
}

static void testOpenNotExistingFiles( void ) {
    bool closed = false;
    bool removed = false;

    // ReadOnly Mode
    FlashFile* readHandler = FlashFileOpen( "myReadFile.txt", ReadOnly );
    TEST_ASSERT_NULL( readHandler );
    closed = FlashFileClose( &readHandler );
    TEST_ASSERT( false == closed );
    // AppendOnly Mode
    FlashFile* appendHandler = FlashFileOpen( "myAppendFile.txt", AppendOnly );
    TEST_ASSERT_NULL( appendHandler );
    closed = FlashFileClose( &appendHandler );
    TEST_ASSERT( false == closed );
    // WriteOnly Mode
    FlashFile* writeHandler = FlashFileOpen( "myWriteFile.txt", WriteOnly );
    TEST_ASSERT_NOT_NULL( writeHandler );
    TEST_ASSERT( true == writeHandler->created );
    // close handler
    closed = FlashFileClose( &writeHandler );
    TEST_ASSERT( true == closed );

    // clean up
    removed = FlashFileRemove( "myWriteFile.txt" );
    TEST_ASSERT( true == removed );
}
static void testMultipleFileCreationAtTheSameTime( void ) {
    bool closed = false;
    bool removed = false;
    // create first file
    FlashFile* firstHandler = FlashFileOpen( "firstFile", WriteOnly );
    TEST_ASSERT_NOT_NULL( firstHandler );
    TEST_ASSERT( true == firstHandler->created );
    // try to create second file
    FlashFile* secondHandler = FlashFileOpen( "secondFile", WriteOnly );
    TEST_ASSERT_NULL( secondHandler );
    closed = FlashFileClose( &secondHandler );
    TEST_ASSERT( false == closed );
    // close handler
    closed = FlashFileClose( &firstHandler );
    TEST_ASSERT( true == closed );

    // clean up
    removed = FlashFileRemove( "firstFile" );
    TEST_ASSERT( true == removed );
}
static void twoFileHandlerPointingToOneFile( const char* filename,
                                             FlashFileMode firstMode,
                                             FlashFileMode secondMode ) {
    bool closed = false;
    FlashFile* firstHandler = NULL;
    FlashFile* secondHandler = NULL;
    firstHandler = FlashFileOpen( filename, firstMode );
    TEST_ASSERT_NOT_NULL( firstHandler );
    secondHandler = FlashFileOpen( filename, secondMode );
    TEST_ASSERT_NULL( secondHandler );
    closed = FlashFileClose( &secondHandler );
    TEST_ASSERT( false == closed );
    closed = FlashFileClose( &firstHandler );
    TEST_ASSERT( true == closed );
}
static void testMultipleFileHandlerPointingToOneFile( void ) {
    bool closed = false;
    bool removed = false;
    const char* filename = "AwesomeFile";
    FlashFile* firstHandler = NULL;
    FlashFile* secondHandler = NULL;
    // create File
    FlashFile* handler = FlashFileOpen( filename, WriteOnly );
    TEST_ASSERT_NOT_NULL( handler );
    closed = FlashFileClose( &handler );
    TEST_ASSERT( true == closed );

    // ReadOnly & ReadOnly
    twoFileHandlerPointingToOneFile( filename, ReadOnly, ReadOnly );
    // ReadOnly & AppendOnly
    twoFileHandlerPointingToOneFile( filename, ReadOnly, AppendOnly );
    // ReadOnly & WriteOnly
    twoFileHandlerPointingToOneFile( filename, ReadOnly, WriteOnly );

    // AppendOnly & ReadOnly
    twoFileHandlerPointingToOneFile( filename, AppendOnly, ReadOnly );
    // AppendOnly & AppendOnly
    twoFileHandlerPointingToOneFile( filename, AppendOnly, AppendOnly );
    // AppendOnly & WriteOnly
    twoFileHandlerPointingToOneFile( filename, AppendOnly, WriteOnly );

    // WriteOnly & ReadOnly
    twoFileHandlerPointingToOneFile( filename, WriteOnly, ReadOnly );
    // WriteOnly & AppendOnly
    twoFileHandlerPointingToOneFile( filename, WriteOnly, AppendOnly );
    // WriteOnly & WriteOnly
    twoFileHandlerPointingToOneFile( filename, WriteOnly, WriteOnly );

    // clean up
    removed = FlashFileRemove( filename );
    TEST_ASSERT( true == removed );
}
TestRef FlashFileOpen_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("testOpenNotExistingFiles", testOpenNotExistingFiles),
        new_TestFixture("testMultipleFileCreationAtTheSameTime", testMultipleFileCreationAtTheSameTime),
        new_TestFixture("testMultipleFileHandlerPointingToOneFile", testMultipleFileHandlerPointingToOneFile)
    };
    EMB_UNIT_TESTCALLER(FlashFileOpenTest, "FlashFileOpenTest", setUp, tearDown, fixtures);
    return (TestRef)&FlashFileOpenTest;
}

static void testWritingData( void ) {
    bool closed = false;
    bool removed = false;
    const char* filename = "writeTest";
    u32 size = 0;
    FlashFile* fileHandler = NULL;
    // create file
    fileHandler = FlashFileOpen( filename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    // write data that fits in one page
    size = FlashFileWrite( fileHandler, loremIpsum, 250 );
    TEST_ASSERT_EQUAL_INT( 250, size );
    // write data that need to chunk because of the next page sector
    size = FlashFileWrite( fileHandler, helloWorld, 10 );
    TEST_ASSERT_EQUAL_INT( 10, size );
    // write data
    size = FlashFileWrite( fileHandler, randomContent,
                           3826/*4KByte-(250+10+10)*/);
    TEST_ASSERT_EQUAL_INT( 3826, size );
    // write data that need to chunk because of the next sub-sector
    size = FlashFileWrite( fileHandler, loremIpsum, 20 );
    TEST_ASSERT_EQUAL_INT( 20, size );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );
    // clean up
    removed = FlashFileRemove( filename );
    TEST_ASSERT( true == removed );
}
static void testAppendMode( void ) {
    bool closed = false;
    bool removed = false;
    const char* filename = "appendTest";
    u32 size = 0;
    FlashFile* fileHandler = NULL;
    // create file
    fileHandler = FlashFileOpen( filename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    // write data that fits in one page
    size = FlashFileWrite( fileHandler, "Hello ", 6 );
    TEST_ASSERT_EQUAL_INT( 6, size );

    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    fileHandler = FlashFileOpen( filename, AppendOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );

    // write data that need to chunk because of the next page sector
    size = FlashFileWrite( fileHandler, "World!", 6 );
    TEST_ASSERT_EQUAL_INT( 6, size );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );
    // clean up
    removed = FlashFileRemove( filename );
    TEST_ASSERT( true == removed );
}
TestRef FlashFileWrite_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
    new_TestFixture("testWritingData", testWritingData),
    new_TestFixture("testAppendMode", testAppendMode)
};
EMB_UNIT_TESTCALLER(FlashFileWriteTest, "FlashFileWriteTest", setUp, tearDown, fixtures);
return (TestRef)&FlashFileWriteTest;
}

static void testPeekData( void ) {
    bool closed = false;
    bool removed = false;
    bool positionChanged = false;
    const char* filename = "peek.txt";
    u32 size = 0;
    u32 postion = 0;
    FlashFile* fileHandler = NULL;
    char readBuffer[32];
    int compare = 0;
    // create file
    fileHandler = FlashFileOpen( filename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    // write data that fits in one page
    size = FlashFileWrite( fileHandler, helloWorld, 11 );
    TEST_ASSERT_EQUAL_INT( 11, size );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    // test
    fileHandler = FlashFileOpen( filename, ReadOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    postion = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 0, postion );
    positionChanged = FlashFileSetPosition( fileHandler, 1 );
    TEST_ASSERT( true == positionChanged );
    size = FlashFilePeek( fileHandler, readBuffer, 4 );
    TEST_ASSERT_EQUAL_INT( 4, size );
    compare = memcmp( readBuffer, "ello", 4 );
    TEST_ASSERT_EQUAL_INT( 0, compare );
    // peek again on the same position
    size = FlashFilePeek( fileHandler, readBuffer, 4 );
    TEST_ASSERT_EQUAL_INT( 4, size );
    compare = memcmp( readBuffer, "ello", 4 );
    TEST_ASSERT_EQUAL_INT( 0, compare );

    positionChanged = FlashFileSetPosition( fileHandler, 6 );
    TEST_ASSERT( true == positionChanged );
    size = FlashFilePeek( fileHandler, readBuffer, 10 );
    TEST_ASSERT_EQUAL_INT( 5, size );
    compare = memcmp( readBuffer, "World", size );
    TEST_ASSERT_EQUAL_INT( 0, compare );
    postion = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 6, postion );

    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    // clean up
    removed = FlashFileRemove( filename );
    TEST_ASSERT( true == removed );
}

static void testReadData( void ) {
    bool closed = false;
    bool removed = false;
    bool positionChanged = false;
    const char* filename = "read.txt";
    u32 size = 0;
    u32 postion = 0;
    FlashFile* fileHandler = NULL;
    char readBuffer[32];
    int compare = 0;
    // create file
    fileHandler = FlashFileOpen( filename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    // write data that fits in one page
    size = FlashFileWrite( fileHandler, helloWorld, 11 );
    TEST_ASSERT_EQUAL_INT( 11, size );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    // test
    fileHandler = FlashFileOpen( filename, ReadOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    postion = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 0, postion );
    positionChanged = FlashFileSetPosition( fileHandler, 1 );
    TEST_ASSERT( true == positionChanged );
    size = FlashFileRead( fileHandler, readBuffer, 4 );
    TEST_ASSERT_EQUAL_INT( 4, size );
    compare = memcmp( readBuffer, "ello", 4 );
    TEST_ASSERT_EQUAL_INT( 0, compare );
    postion = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 5, postion );

    positionChanged = FlashFileSetPosition( fileHandler, 6 );
    TEST_ASSERT( true == positionChanged );
    size = FlashFileRead( fileHandler, readBuffer, 10 );
    TEST_ASSERT_EQUAL_INT( 5, size );
    compare = memcmp( readBuffer, "World", size );
    TEST_ASSERT_EQUAL_INT( 0, compare );
    postion = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 11, postion );

    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    // clean up
    removed = FlashFileRemove( filename );
    TEST_ASSERT( true == removed );
}

TestRef FlashFileRead_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
    new_TestFixture("testPeekData", testPeekData),
    new_TestFixture("testReadData", testReadData)
};
EMB_UNIT_TESTCALLER(FlashFileReadTest, "FlashFileReadTest", setUp, tearDown, fixtures);
return (TestRef)&FlashFileReadTest;
}

static void testFlashMemoryIsEmpty( void ) {
    u32 vailableMemory = FlashFileAvailableMemory();
    TEST_ASSERT_EQUAL_INT( 8187904/*8MB-(64KB*3+4KB)*/,vailableMemory );
}
static void testPosition( void ) {
    bool closed = false;
    bool removed = false;
    bool moved = false;
    const char* filename = "position.txt";
    u32 size = 0;
    u32 position = 0;
    FlashFile* fileHandler = NULL;

    // create file
    fileHandler = FlashFileOpen( filename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    size = FlashFileWrite( fileHandler, loremIpsum, 64 );
    TEST_ASSERT_EQUAL_INT( 64, size );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    // test
    moved = FlashFileSetPosition( NULL, 0 );
    TEST_ASSERT( false == moved );
    position = FlashFileGetPosition( NULL );
    TEST_ASSERT_EQUAL_INT( 0, position );

    fileHandler = FlashFileOpen( filename, ReadOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );

    moved = FlashFileSetPosition( fileHandler, 64 );
    TEST_ASSERT( true == moved );
    position = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 64, position );

    moved = FlashFileSetPosition( fileHandler, 0 );
    TEST_ASSERT( true == moved );
    position = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 0, position );

    moved = FlashFileSetPosition( fileHandler, 128 );
    TEST_ASSERT( false == moved );
    position = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 0, position );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    fileHandler = FlashFileOpen( filename, AppendOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );

    moved = FlashFileSetPosition( fileHandler, 64 );
    TEST_ASSERT( false == moved );
    position = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 64, position );

    moved = FlashFileSetPosition( fileHandler, 0 );
    TEST_ASSERT( false == moved );
    position = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 64, position );

    moved = FlashFileSetPosition( fileHandler, 128 );
    TEST_ASSERT( false == moved );
    position = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 64, position );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    fileHandler = FlashFileOpen( filename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );

    moved = FlashFileSetPosition( fileHandler, 64 );
    TEST_ASSERT( false == moved );
    position = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 0, position );

    moved = FlashFileSetPosition( fileHandler, 0 );
    TEST_ASSERT( false == moved );
    position = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 0, position );

    moved = FlashFileSetPosition( fileHandler, 128 );
    TEST_ASSERT( false == moved );
    position = FlashFileGetPosition( fileHandler );
    TEST_ASSERT_EQUAL_INT( 0, position );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    // clean up
    removed = FlashFileRemove( filename );
    TEST_ASSERT( true == removed );
}
static void testAvailableSpace( void ) {
    bool closed = false;
    bool removed = false;
    const char* filename = "availableSpace.txt";
    const char* secondFilename = "availableSpace2.txt";
    u32 availableSpace = 0;
    u32 size = 0;
    u32 position = 0;
    FlashFile* fileHandler = NULL;

    // create file
    fileHandler = FlashFileOpen( filename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );

    availableSpace = FlashFileGetAvailableSpace( fileHandler );
    TEST_ASSERT_EQUAL_INT( 8187904/*8MByte-(64KBytes*3+4KBytes)*/,
                           availableSpace );

    FlashFileWrite( fileHandler, loremIpsum, 128 );
    availableSpace = FlashFileGetAvailableSpace( fileHandler );
    TEST_ASSERT_EQUAL_INT( 8187776/*8MByte-(64KBytes*3+4KBytes+128Bytes)*/,
                           availableSpace );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    // create new file
    fileHandler = FlashFileOpen( secondFilename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );

    availableSpace = FlashFileGetAvailableSpace( fileHandler );
    TEST_ASSERT_EQUAL_INT( 8179712/*8MByte-(64KBytes*3+4KBytes+2*4KBytes)*/,
                           availableSpace );

    FlashFileWrite( fileHandler, loremIpsum, 128 );
    availableSpace = FlashFileGetAvailableSpace( fileHandler );
    TEST_ASSERT_EQUAL_INT(
            8179584/*8MByte-(64KBytes*3+4KBytes+2*4KBytes+128Bytes)*/,
            availableSpace );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    fileHandler = FlashFileOpen( filename, AppendOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );

    availableSpace = FlashFileGetAvailableSpace( fileHandler );
    TEST_ASSERT_EQUAL_INT( 8064/*8KByte-(128Bytes)*/,availableSpace );
    // close file handler
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    // clean up
    removed = FlashFileRemove( filename );
    TEST_ASSERT( true == removed );
    removed = FlashFileRemove( secondFilename );
    TEST_ASSERT( true == removed );
}
static void testRemove( void ) {
    bool closed = false;
    bool removed = false;
    const char* noExistingFilename = "notExistingFile.txt";
    const char* existingFilename = "existingFile.txt";
    const char* secondExistingFilename = "secondExistingFile.txt";
    FlashFile* fileHandler = NULL;
    // create files
    fileHandler = FlashFileOpen( existingFilename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );
    fileHandler = FlashFileOpen( secondExistingFilename, WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );

    // test
    removed = FlashFileRemove( noExistingFilename );
    TEST_ASSERT( false == removed );

    removed = FlashFileRemove( existingFilename );
    TEST_ASSERT( true == removed );

    removed = FlashFileRemove( secondExistingFilename );
    TEST_ASSERT( false == removed );

    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    removed = FlashFileRemove( secondExistingFilename );
    TEST_ASSERT( true == removed );
}
static void testRename( void ) {
    bool closed = false;
    bool removed = false;
    bool renamed = false;
    FlashFile* fileHandler = NULL;
    fileHandler = FlashFileOpen( "renameFile.txt", WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );
    fileHandler = FlashFileOpen( "rename2File.txt", WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    renamed = FlashFileRename( "renameFile.txt", "rename2File.txt" );
    TEST_ASSERT( false == renamed );

    renamed = FlashFileRename( "renameFile.txt",
                               "renameFilenameWithoutPath.longfilename" );
    TEST_ASSERT( true == renamed );
    renamed = FlashFileRename( "renameFilenameWithoutPath.longfilename",
                               "firstFile" );
    TEST_ASSERT( true == renamed );

    renamed = FlashFileRename( "rename2File.txt", "secondFile" );
    TEST_ASSERT( true == renamed );

    // clean up
    removed = FlashFileRemove( "firstFile" );
    TEST_ASSERT( true == removed );
    removed = FlashFileRemove( "secondFile" );
    TEST_ASSERT( true == removed );
}
static void testExists( void ) {
    bool exists = false;
    bool closed = false;
    bool removed = false;
    FlashFile* fileHandler = NULL;
    fileHandler = FlashFileOpen( "exists", WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandler );
    closed = FlashFileClose( &fileHandler );
    TEST_ASSERT( true == closed );

    exists = FlashFileExists( "returnFalse.txt" );
    TEST_ASSERT( false == exists );

    exists = FlashFileExists( "exists" );
    TEST_ASSERT( true == exists );

    removed = FlashFileRemove( "exists" );
    TEST_ASSERT( true == removed );
}
TestRef FlashFileMisc_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
    new_TestFixture("testFlashMemoryIsEmpty", testFlashMemoryIsEmpty),
    new_TestFixture("testPositon", testPosition),
    new_TestFixture("testAvailableSpace", testAvailableSpace),
    new_TestFixture("testRemove", testRemove),
    new_TestFixture("testRename", testRename),
    new_TestFixture("testExists", testExists)
};
EMB_UNIT_TESTCALLER(FlashFileMiscTest, "FlashFileMiscTest", setUp, tearDown, fixtures);
return (TestRef)&FlashFileMiscTest;
}

static void testWriteReadUseCase() {
    // create "helloWorld.txt" file
    FlashFile* fileHandlerWriteMode = FlashFileOpen( "helloWorld.txt",
                                                     WriteOnly );
    TEST_ASSERT_NOT_NULL( fileHandlerWriteMode );
    // write 270 characters
    u32 writeSize = FlashFileWrite( fileHandlerWriteMode, loremIpsum, 270 );
    TEST_ASSERT_EQUAL_INT( 270, writeSize );
    // check position of the file handler
    u32 filePostion = FlashFileGetPosition( fileHandlerWriteMode );
    TEST_ASSERT_EQUAL_INT( 270, filePostion );
    // append more data
    writeSize = FlashFileWrite( fileHandlerWriteMode, helloWorld, 12 );
    TEST_ASSERT_EQUAL_INT( 12, writeSize );
    // check position again
    filePostion = FlashFileGetPosition( fileHandlerWriteMode );
    TEST_ASSERT_EQUAL_INT( 282, filePostion );
    // close file handler
    bool closed = FlashFileClose( &fileHandlerWriteMode );
    TEST_ASSERT( true == closed );
    // check if the file exists
    bool exists = FlashFileExists( "helloWorld.txt" );
    TEST_ASSERT( true == exists );
    // rename it
    bool renamed = FlashFileRename( "helloWorld.txt", "loremIpsum.txt" );
    TEST_ASSERT( true == renamed );
    // return the size of the file
    u32 fileSizeInBytes = FlashFileGetSize( "loremIpsum.txt" );
    TEST_ASSERT_EQUAL_INT( 282, fileSizeInBytes );
    // create a read file handler
    FlashFile* fileHandlerReadMode = FlashFileOpen( "loremIpsum.txt",
                                                    ReadOnly );
    TEST_ASSERT_NOT_NULL( fileHandlerReadMode );
    // read from file
    char readBuffer[11];
    u32 readSizeInBytes = FlashFileRead( fileHandlerReadMode, readBuffer, 10 );
    readBuffer[10] = '\0';
    TEST_ASSERT_EQUAL_INT( 10, readSizeInBytes );
    TEST_ASSERT_EQUAL_STRING( "Lorem ipsu", readBuffer );
    // close file handler
    closed = FlashFileClose( &fileHandlerReadMode );
    TEST_ASSERT( true == closed );
    // remove file
    bool removed = FlashFileRemove( "loremIpsum.txt" );
    TEST_ASSERT( true == removed );
}
static void testCreatingFilesUseCase() {
    FlashFile* imageHandler = NULL;
    FlashFile* textHandler = NULL;
    FlashFile* binaryHandler = NULL;
    FlashFile* dummyHandler = NULL;
    u32 writeSize = 0;
    u32 fileSize = 0;
    u32 fileMaxSize = 0;
    bool closed = false;
    bool removed = false;
    const char* imageFileName = "image.jpg";
    const char* textFileName = "config.json";
    const char* binaryFileName = "binary";
    const char* dummyFileName = "blob";
    imageHandler = FlashFileOpen( imageFileName, WriteOnly );
    TEST_ASSERT_NOT_NULL( imageHandler );
    TEST_ASSERT( true == imageHandler->created );

    writeSize = FlashFileWrite( imageHandler, randomContent, 4095 );
    TEST_ASSERT_EQUAL_INT( 4095, writeSize );

    textHandler = FlashFileOpen( textFileName, WriteOnly );
    TEST_ASSERT_NULL( textHandler );

    closed = FlashFileClose( &imageHandler );
    TEST_ASSERT_MESSAGE( true == closed, "expected return value was not false" );

    fileSize = FlashFileGetSize( imageFileName );
    TEST_ASSERT_EQUAL_INT( 4095, fileSize );

    fileMaxSize = FlashFileGetMaxSize( imageFileName );
    TEST_ASSERT_EQUAL_INT( 0, fileMaxSize );

    textHandler = FlashFileOpen( textFileName, WriteOnly );
    TEST_ASSERT_NOT_NULL( textHandler );
    TEST_ASSERT( true == textHandler->created );

    writeSize = FlashFileWrite( textHandler, randomContent, 4097 );
    TEST_ASSERT_EQUAL_INT( 4097, writeSize );

    closed = FlashFileClose( &textHandler );
    TEST_ASSERT( true == closed );

    fileSize = FlashFileGetSize( textFileName );
    TEST_ASSERT_EQUAL_INT( 4097, fileSize );

    fileMaxSize = FlashFileGetMaxSize( textFileName );
    TEST_ASSERT_EQUAL_INT( 0, fileMaxSize );

    fileSize = FlashFileGetSize( imageFileName );
    TEST_ASSERT_EQUAL_INT( 4095, fileSize );

    fileMaxSize = FlashFileGetMaxSize( imageFileName );
    TEST_ASSERT_EQUAL_INT( 8192, fileMaxSize );

    binaryHandler = FlashFileOpen( binaryFileName, WriteOnly );
    TEST_ASSERT_NOT_NULL( binaryHandler );
    TEST_ASSERT( true == binaryHandler->created );

    writeSize = FlashFileWrite( binaryHandler, randomContent, 4096 );
    TEST_ASSERT_EQUAL_INT( 4096, writeSize );

    closed = FlashFileClose( &binaryHandler );
    TEST_ASSERT( true == closed );

    dummyHandler = FlashFileOpen( dummyFileName, WriteOnly );
    TEST_ASSERT_NOT_NULL( dummyHandler );
    TEST_ASSERT( true == dummyHandler->created );

    closed = FlashFileClose( &dummyHandler );
    TEST_ASSERT( true == closed );

    fileMaxSize = FlashFileGetMaxSize( textFileName );
    TEST_ASSERT_EQUAL_INT( 12288, fileMaxSize );

    fileMaxSize = FlashFileGetMaxSize( binaryFileName );
    TEST_ASSERT_EQUAL_INT( 8192, fileMaxSize );

    // clean up
    removed = FlashFileRemove( dummyFileName );
    TEST_ASSERT( true == removed );

    removed = FlashFileRemove( binaryFileName );
    TEST_ASSERT( true == removed );

    removed = FlashFileRemove( textFileName );
    TEST_ASSERT( true == removed );

    removed = FlashFileRemove( imageFileName );
    TEST_ASSERT( true == removed );
}

TestRef FlashFileUseCase_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
    new_TestFixture("testWriteReadUseCase", testWriteReadUseCase),
    new_TestFixture("testCreatingFilesUseCase", testCreatingFilesUseCase)
};
EMB_UNIT_TESTCALLER(FlashFileUseCaseTest, "FlashFileUseCaseTest", setUp, tearDown, fixtures);
return (TestRef)&FlashFileUseCaseTest;
}

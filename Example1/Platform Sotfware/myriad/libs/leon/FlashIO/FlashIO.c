#include "FlashIO.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <DrvCpr.h>
#include <stdio.h>

#include "DrvEeprom.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define PAGE_SIZE 256/*256 Bytes*/
#define SECTOR_SIZE 4096/*4 KBytes*/
#define BLOCK_SIZE 65536/*64 KBytes*/
#define MAX_FLASH_FILENAME_SIZE 31

typedef struct __sFileAllocationTableHeader {
    unsigned char ident[4];     // magic number
    u32 firstWritableSector;    // points to the first writable eeprom block.
    u32 lastWritableSector;     // points to the last writable eeprom sector
    int numberOfFiles;          // holds the number of files
} FATHeader;
typedef struct __sFileAllocationTableFileHeader_Internal {
    unsigned char filename[MAX_FLASH_FILENAME_SIZE + 1];
    u32 startAddress;   // holds the start address
    u32 size;           // holds the current size of the file
    u32 maxSize;        // holds the maximum size
} FATFileHeader_Internal;
typedef struct __sFileAllocationTableFileHeader {
    FATFileHeader_Internal internal;
    u32 referenceCounter;
} FATFileHeader;

// holds the number of FlashFile handler that are open
static u8 flashFileHandlerReferenceCounter = 0;
// holds the address of the last valid sub sector
static u32 lastValidSubSectorAddress = 0x0;
// FATHeader object
static FATHeader eepromFATHeader = { { 0 }, 0, 0, 0 };
// array of FATFileHeader objects
static FATFileHeader* eepromFATFileHeaders = NULL;
// holds the device id
static u8 deviceId[20] = { 0x00 };

//
// declare functions
//
static bool initFlashMemory( void );
static bool initFileAllocationTable( void );
static bool createFileAllocationTable( void );
static void createFileAllocationTableBasedOnFlasher( void );
static void parseFileAllocationTable( void );
static bool isPageEmpty( u8* pageData, u32 pageDataSize );
static u32 findEndOfTheBootloaderApplication( void );
static void closeFlashMemory( bool withoutSaving );
static void cleanUpFileAllocationTable( void );
static void saveFileAllocationTable( void );
static FlashFile* createFlashFileObject( int fileHeaderIdx, FlashFileMode mode,
                                         u32 position, bool created );
static FlashFile* createNewFile( const char* filename );
static int findHighestValidFileHeaderIndex( void );
static u32 calcMaxSizeForFlashFile( const FATFileHeader* fileHeader );
static void addNewFileToFAT( const char* filename );

static int findFlashFile( const char* filename );

/**
 * Initialize the flash memory
 */
static bool initFlashMemory( void ) {
    // init EEPROM
    DrvEepromInit();
    DrvEepromPowerUp();
    // read the eeprom Id
    DrvEepromReadId( deviceId, sizeof(deviceId) );
    if( 0x00 == deviceId[0] ) {
        // looks like the eeprom has still no power
        // -> power up the eeprom takes some time
        // try to power up the eeprom again
        DrvEepromPowerUp();
        // try to read the eeprom Id again
        DrvEepromReadId( deviceId, sizeof(deviceId) );
    }
    // check ManufacturerID and FlashMemoryID
    if( 0x20 != deviceId[0] || 0xBB != deviceId[1] ) {
        return false;
    }
    // check memory capacity
    switch( deviceId[2] ) {
    case 0x17: // 8MB capacity
        // set address of the last sub sector
        lastValidSubSectorAddress = 0x007FF000;
        break;
    case 0x20: // 64MB capacity
        // set address of the last sub sector
        lastValidSubSectorAddress = 0x03FFF000;
        break;
    default:
        return false;
    }
    return true;
}

/**
 * create or load the "File Allocation Table"
 */
static bool initFileAllocationTable( void ) {
    DrvEepromFastRead( lastValidSubSectorAddress, (u8*) &eepromFATHeader,
                       sizeof(FATHeader) );
    // check if the sub-sector is blank
    if( 0xFF == eepromFATHeader.ident[0] && 0xFF == eepromFATHeader.ident[1]
        && 0xFF == eepromFATHeader.ident[2] && 0xFF == eepromFATHeader.ident[3]
        && 0xFFFFFFFF == eepromFATHeader.firstWritableSector
        && 0xFFFFFFFF == eepromFATHeader.lastWritableSector
        && 0xFFFFFFFF == (u32) eepromFATHeader.numberOfFiles ) {
        // create FAT
        return createFileAllocationTable();
    }
    // check if flasher data exists
    if( 'F' == eepromFATHeader.ident[0] && 'L' == eepromFATHeader.ident[1]
		&& 'A' == eepromFATHeader.ident[2]
		&& '\0' == eepromFATHeader.ident[3] ) {
		// load FAT
    	createFileAllocationTableBasedOnFlasher();
		return true;
	}
    // check if the magic number is correct
    if( 'F' == eepromFATHeader.ident[0] && 'F' == eepromFATHeader.ident[1]
        && 'T' == eepromFATHeader.ident[2]
        && '\0' == eepromFATHeader.ident[3] ) {
        // load FAT
        parseFileAllocationTable();
        return true;
    }
    return false;
}
/**
 * create FAT
 */
static bool createFileAllocationTable( void ) {
    // set the magic number
    memcpy( eepromFATHeader.ident, "FFT", 4 );
    eepromFATHeader.firstWritableSector = findEndOfTheBootloaderApplication();

    if( 0 == eepromFATHeader.firstWritableSector ) {
        return false;
    }

    eepromFATHeader.lastWritableSector = lastValidSubSectorAddress;
    eepromFATHeader.numberOfFiles = 0;

    eepromFATFileHeaders = malloc( sizeof(FATFileHeader) );

    return true;
}

/**
 * create FAT based on the flasher data
 */
static void createFileAllocationTableBasedOnFlasher( void ) {
    printf("\n\ncreateFileAllocationTableBasedOnFlasher()\n\n");
	u32 firstFreeSector = eepromFATHeader.firstWritableSector;
	printf("\n\nfirstWritableSector: %X\n", firstFreeSector);
	// set the magic number
	memcpy( eepromFATHeader.ident, "FFT", 4 );
	// starting point of the control app
	eepromFATHeader.firstWritableSector = 0x80000;

	eepromFATHeader.lastWritableSector = lastValidSubSectorAddress;
	eepromFATHeader.numberOfFiles = 1;

	eepromFATFileHeaders = malloc( sizeof(FATFileHeader) );

	strncpy((&eepromFATFileHeaders[0])->internal.filename,
	        "ControlApp.elf", MAX_FLASH_FILENAME_SIZE );
	(&eepromFATFileHeaders[0])->internal.startAddress = 0x80000;
	(&eepromFATFileHeaders[0])->internal.size = firstFreeSector - 0x80000;
	(&eepromFATFileHeaders[0])->internal.maxSize = 0;
	(&eepromFATFileHeaders[0])->referenceCounter = 0;
}

/**
 * create a array for FATFileHeader objects and load them from the flash memory
 */
static void parseFileAllocationTable( void ) {
    if( 0 == eepromFATHeader.numberOfFiles ) {
        free( eepromFATFileHeaders );
        eepromFATFileHeaders = malloc( sizeof(FATFileHeader) );
    } else {
        free( eepromFATFileHeaders );
        eepromFATFileHeaders = malloc(
                eepromFATHeader.numberOfFiles * sizeof(FATFileHeader) );
        int i;
        u32 flashFATFileHeaderPosition = lastValidSubSectorAddress + sizeof(FATHeader);
        for( i = 0; i < eepromFATHeader.numberOfFiles;
                ++i, flashFATFileHeaderPosition +=
                        sizeof(FATFileHeader_Internal) ) {
            DrvEepromFastRead( flashFATFileHeaderPosition,
                               (u8*) (&eepromFATFileHeaders[i].internal),
                               sizeof(FATFileHeader_Internal) );
            eepromFATFileHeaders[i].referenceCounter = 0;
        }
    }
}
/**
 * Returns a address of the first free sector
 */
static u32 findEndOfTheBootloaderApplication( void ) {
    // defines the search range
    u32 blockAddress = 0x0;
    const u32 endBlockPosition = (lastValidSubSectorAddress / BLOCK_SIZE) * BLOCK_SIZE;
    // read buffer for a whole page
    u8 pageData[PAGE_SIZE];
    // holds the current page address
    u32 currentPagePosition;
    // check each eeprom sector
    for( blockAddress = 0x0; blockAddress < endBlockPosition; blockAddress +=
    BLOCK_SIZE ) {
        currentPagePosition = blockAddress;
        bool isEmpty = true;
        // try to read the block in chunks
        for( ; currentPagePosition < (blockAddress + BLOCK_SIZE);
                currentPagePosition += PAGE_SIZE ) {
            DrvEepromFastRead( currentPagePosition, pageData, PAGE_SIZE );
            // check if the page is empty
            if( !(isEmpty = isPageEmpty( pageData, PAGE_SIZE )) ) {
                // page is not empty, so we have to check the next sector
                break;
            }
        }
        // current block is empty
        if( isEmpty ) {
            // is the first block that are not used anymore for the bootloader
            return blockAddress;
        }
    }
    // error
    return 0;
}
/**
 * Returns true if the page is empty; otherwise returns false.
 */
static bool isPageEmpty( u8* pageData, u32 pageDataSize ) {
    u32 i;
    for( i = 0; i < pageDataSize; ++i ) {
        if( 0xFF != pageData[i] ) {
            return false;
        }
    }
    return true;
}

/**
 * save the FAT data and power down the eeprom
 * @param[in] withoutSaving skip the saving process if true.
 *        Warning: You should only set it to true if you know
 *                 what the consequences are.
 */
static void closeFlashMemory( bool withoutSaving ) {
    cleanUpFileAllocationTable();
    if( false == withoutSaving ) {
        // erase sector
        DrvEepromSetWriteEnable();
        DrvEepromSectorErase( lastValidSubSectorAddress );
        DrvEepromWaitWriteInProgress();
        // save table
        saveFileAllocationTable();
    }
    // clean up
    free( eepromFATFileHeaders );
    eepromFATFileHeaders = NULL;
    // power down
    DrvEepromPowerDown();
    DrvEepromClose();
}

/**
 * clean up FAT
 */
static void cleanUpFileAllocationTable( void ) {
    int i;
    int idx;
    int fileHeaderNumberWithDeletedEntries = eepromFATHeader.numberOfFiles;
    //reset
    eepromFATHeader.numberOfFiles = 0;
    // count valid files
    for( i = 0; i < fileHeaderNumberWithDeletedEntries; ++i ) {
        if( '\0' == eepromFATFileHeaders[i].internal.filename[0] ) {
            continue;
        }
        ++eepromFATHeader.numberOfFiles;
    }
    FATFileHeader* newEepromFATFileHeaders =
            malloc( MAX(1, eepromFATHeader.numberOfFiles) * sizeof(FATFileHeader) );
    idx = 0;
    // copy valid FileHeader data
    for( i = 0; i < fileHeaderNumberWithDeletedEntries; ++i ) {
        // ignore deleted files
        if( '\0' == eepromFATFileHeaders[i].internal.filename[0] ) {
            continue;
        }
        memcpy( &(newEepromFATFileHeaders[idx]), &eepromFATFileHeaders[i],
                sizeof(FATFileHeader) );
        ++idx;
    }
    // replace the old FileHeader array with the new one
    free( eepromFATFileHeaders );
    eepromFATFileHeaders = newEepromFATFileHeaders;

}
/**
 * clean up FAT index and save it
 */
static void saveFileAllocationTable( void ) {
    int i;
    u32 address = lastValidSubSectorAddress;

    // save FATHeader object
    DrvEepromSetWriteEnable();
    DrvEepromPageProgram( address, (u8*) &eepromFATHeader, sizeof(FATHeader) );
    DrvEepromWaitWriteInProgress();
    address += sizeof(FATHeader);

    // save valid FATFileHeader objects
    for( i = 0; i < eepromFATHeader.numberOfFiles; ++i ) {
        DrvEepromSetWriteEnable();
        DrvEepromPageProgram( address,
                              (u8*) (&eepromFATFileHeaders[i].internal),
                              sizeof(FATFileHeader_Internal) );
        DrvEepromWaitWriteInProgress();

        address += (sizeof(FATFileHeader_Internal));
    }
}
/**
 * create FlashFile object: malloc and initialization
 */
static FlashFile* createFlashFileObject( int fileHeaderIdx, FlashFileMode mode,
                                         u32 position, bool created ) {
    // create a new fileHandler object
    FlashFile* fileHandler = malloc( sizeof(FlashFile) );
    // initialization
    fileHandler->fileHeaderIdx = fileHeaderIdx;
    fileHandler->mode = mode;
    fileHandler->position = position;
    fileHandler->created = created;

    return fileHandler;
}

static bool isCreatingModeActive = false;
/**
 * Create a FlashFile handler and make an entry in the FAT index
 */
static FlashFile* createNewFile( const char* filename ) {
    // create a new FlashFile object
    FlashFile* fileHandler = createFlashFileObject(
            eepromFATHeader.numberOfFiles, WriteOnly, 0, true );
    // add the new file to the index
    addNewFileToFAT( filename );

    // increase reference counter
    eepromFATFileHeaders[fileHandler->fileHeaderIdx].referenceCounter++;

    isCreatingModeActive = true;

    return fileHandler;
}
/**
 * Add a new entry
 */
static void addNewFileToFAT( const char* filename ) {

    // create a new array
    FATFileHeader* newFlashFATFileHeaders = malloc(
            (eepromFATHeader.numberOfFiles + 1) * sizeof(FATFileHeader) );
    int maxValidFileHeaderIndex = findHighestValidFileHeaderIndex();
    if( -1 == maxValidFileHeaderIndex ) {
        // initialization new file header
        (&newFlashFATFileHeaders[eepromFATHeader.numberOfFiles])->internal
                .startAddress = eepromFATHeader.firstWritableSector;
    } else {
        memcpy( newFlashFATFileHeaders, eepromFATFileHeaders,
                eepromFATHeader.numberOfFiles * sizeof(FATFileHeader) );
        // set the max size of
        (&newFlashFATFileHeaders[maxValidFileHeaderIndex])->internal.maxSize =
                calcMaxSizeForFlashFile(
                        &newFlashFATFileHeaders[maxValidFileHeaderIndex] );
        // initialization new file header
        (&newFlashFATFileHeaders[eepromFATHeader.numberOfFiles])->internal
                .startAddress =
                (&newFlashFATFileHeaders[maxValidFileHeaderIndex])->internal
                        .startAddress
                + (&newFlashFATFileHeaders[maxValidFileHeaderIndex])->internal
                        .maxSize;
    }
    strncpy(
            (&newFlashFATFileHeaders[eepromFATHeader.numberOfFiles])
                    ->internal.filename,
            filename, MAX_FLASH_FILENAME_SIZE );
    (&newFlashFATFileHeaders[eepromFATHeader.numberOfFiles])->internal
            .filename[MAX_FLASH_FILENAME_SIZE] = '\0';
    (&newFlashFATFileHeaders[eepromFATHeader.numberOfFiles])->internal.size =
            0;
    (&newFlashFATFileHeaders[eepromFATHeader.numberOfFiles])->internal
            .maxSize = 0;
    (&newFlashFATFileHeaders[eepromFATHeader.numberOfFiles])
            ->referenceCounter = 0;

    free( eepromFATFileHeaders );
    eepromFATFileHeaders = newFlashFATFileHeaders;
    eepromFATHeader.numberOfFiles++;
}

static int findHighestValidFileHeaderIndex( void ) {
    int i;
    for( i = eepromFATHeader.numberOfFiles - 1; i >= 0; --i ) {
        if( '\0' != eepromFATFileHeaders[i].internal.filename[0] ) {
            return i;
        }
    }
    return -1;
}

static u32 calcMaxSizeForFlashFile( const FATFileHeader* fileHeader ) {
    if( 0 != fileHeader->internal.maxSize ) {
        return fileHeader->internal.maxSize;
    }
    u32 endAddress = fileHeader->internal.startAddress
            + fileHeader->internal.size;
    // align to the next sector (minimum size is 4KB)
    endAddress = ((endAddress + (2 * SECTOR_SIZE) - 1) / SECTOR_SIZE)
            * SECTOR_SIZE;
    // endAddress should not be greater than eepromFATHeader.lastWritableSector
    endAddress = MIN( endAddress, eepromFATHeader.lastWritableSector );

    return endAddress - fileHeader->internal.startAddress;
}

FlashFile* FlashFileOpen( const char* filename, const FlashFileMode mode ) {
    // find file
    int fileIdx = findFlashFile( filename );
    if( -1 == fileIdx ) {
        // file does not exist: Read and Append will not work
        switch( mode ) {
        case ReadOnly:
            // no break
        case AppendOnly:
            return NULL;
        }
        // case WriteOnly:
        if( true == isCreatingModeActive || /*creating mode is already running*/
        0 == FlashFileAvailableMemory() ) { /*not enough space*/
            return NULL;
        }
    }
    // more than one handler for a file is not allowed at the same time.
    if( NULL != eepromFATFileHeaders && -1 != fileIdx
        && 0 != eepromFATFileHeaders[fileIdx].referenceCounter ) {
        return NULL;
    }
    if( NULL == eepromFATFileHeaders ) {
        // activate file system
        if( !initFlashMemory() || !initFileAllocationTable() ) {
            return NULL;
        }
    }
    FlashFile* fileHandler;
    if( -1 == fileIdx ) {
        // create a file
        fileHandler = createNewFile( filename );
    } else {
        // create a new fileHandler object
        fileHandler = createFlashFileObject( fileIdx, mode, 0, false );
        // increase reference counter
        eepromFATFileHeaders[fileHandler->fileHeaderIdx].referenceCounter++;
        switch( mode ) {
        case AppendOnly:
            fileHandler->position = eepromFATFileHeaders[fileHandler
                    ->fileHeaderIdx].internal.size;
            break;
        case WriteOnly:
            // override file
            eepromFATFileHeaders[fileHandler->fileHeaderIdx].internal.size = 0;
            break;
        }
    }

    ++flashFileHandlerReferenceCounter;

    return fileHandler;
}

bool FlashFileClose( FlashFile** fileHandler ) {
    if( NULL == *fileHandler ) {
        return false;
    }
    eepromFATFileHeaders[(*fileHandler)->fileHeaderIdx].referenceCounter--;

    if( true == (*fileHandler)->created ) {
        isCreatingModeActive = false;
    }

    // save FAT to the flash memory
    if( 1 == flashFileHandlerReferenceCounter ) {
        closeFlashMemory( false );
    }

    free( *fileHandler );
    *fileHandler = NULL;

    --flashFileHandlerReferenceCounter;
    return true;
}

u32 FlashFileWrite( FlashFile* fileHandler, const u8* writeBuffer,
                    const u32 writeBufferSize ) {
    if( NULL == fileHandler || ReadOnly == fileHandler->mode ) {
        return 0;
    }

    FATFileHeader* fileHeader =
            &eepromFATFileHeaders[fileHandler->fileHeaderIdx];
    const u32 eofAddress =
            fileHeader->internal.maxSize == 0 ?
                    eepromFATHeader.lastWritableSector :
                    fileHeader->internal.startAddress + fileHeader->internal
                            .maxSize;
    u32 writeAddress = fileHeader->internal.startAddress
            + fileHandler->position;

    if( (writeAddress + writeBufferSize) > eofAddress ) {
        return 0;
    }

    u32 _writeBufferSize = writeBufferSize;
    u32 _writeBufferIdx = 0;
    while( _writeBufferSize > 0 ) {
        u32 nextPageAddress = ((writeAddress + PAGE_SIZE) / PAGE_SIZE)
                * PAGE_SIZE;
        u32 writeChunkSize = nextPageAddress - writeAddress;
        writeChunkSize = MIN( writeChunkSize, _writeBufferSize );
        // erase sector
        if( 0 == (writeAddress % SECTOR_SIZE) ) {
            DrvEepromSetWriteEnable();
            DrvEepromSectorErase( writeAddress );
            DrvEepromWaitWriteInProgress();
        }
        // program
        DrvEepromSetWriteEnable();
        DrvEepromPageProgram( writeAddress, &writeBuffer[_writeBufferIdx],
                              writeChunkSize );
        DrvEepromWaitWriteInProgress();

        _writeBufferIdx += writeChunkSize;
        _writeBufferSize -= writeChunkSize;

        fileHandler->position += writeChunkSize;
        fileHeader->internal.size += writeChunkSize;
        writeAddress += writeChunkSize;
    }

    return writeBufferSize;
}

u32 FlashFileRead( FlashFile* fileHandler, u8* readBuffer,
                   const u32 readBufferSize ) {

    u32 _readBufferSize = FlashFilePeek( fileHandler, readBuffer,
                                         readBufferSize );

    //update position
    fileHandler->position += _readBufferSize;

    return _readBufferSize;
}

u32 FlashFilePeek( const FlashFile* fileHandler, u8* readBuffer,
                   const u32 readBufferSize ) {
    if( NULL == fileHandler || ReadOnly != fileHandler->mode ) {
        return 0;
    }

    FATFileHeader* fileHeader =
            &eepromFATFileHeaders[fileHandler->fileHeaderIdx];
    const u32 eofAddress = fileHeader->internal.startAddress
            + fileHeader->internal.size;
    u32 readAddress = fileHeader->internal.startAddress + fileHandler->position;
    u32 _readBufferSize = readBufferSize;

    if( (readAddress + readBufferSize) > eofAddress ) {
        _readBufferSize = eofAddress - readAddress;
    }

    DrvEepromFastRead( readAddress, readBuffer, _readBufferSize );

    return _readBufferSize;
}

bool FlashFileSetPosition( FlashFile* fileHandler, const u32 position ) {
    if( NULL == fileHandler || ReadOnly != fileHandler->mode ) {
        return false;
    }

    FATFileHeader* fileHeader =
            &eepromFATFileHeaders[fileHandler->fileHeaderIdx];

    if( position > fileHeader->internal.size ) {
        return false;
    }
    fileHandler->position = position;

    return true;
}

u32 FlashFileGetPosition( const FlashFile* fileHandler ) {
    if( NULL == fileHandler ) {
        return 0;
    }
    return fileHandler->position;
}

u32 FlashFileGetAvailableSpace( FlashFile* fileHandler ) {
    if( NULL == fileHandler ) {
        return 0;
    }
    FATFileHeader* fileHeader =
            &eepromFATFileHeaders[fileHandler->fileHeaderIdx];
    const u32 totalEofAddress =
            fileHeader->internal.maxSize == 0 ?
                    eepromFATHeader.lastWritableSector :
                    fileHeader->internal.startAddress + fileHeader->internal
                            .maxSize;
    const u32 currentEofAddress = fileHeader->internal.startAddress
            + fileHeader->internal.size;

    return totalEofAddress - currentEofAddress;
}

bool FlashFileRemove( const char* filename ) {
    bool isInitialized = true;
    if( NULL == eepromFATFileHeaders ) {
        isInitialized = false;
        if( !initFlashMemory() || !initFileAllocationTable() ) {
            return false;
        }
    }

    bool isFileRemoved = false;
    int fileIdx = findFlashFile( filename );
    if( -1 != fileIdx && 0 == eepromFATFileHeaders[fileIdx].referenceCounter ) {
        eepromFATFileHeaders[fileIdx].internal.filename[0] = '\0';
        isFileRemoved = true;
    }

    if( false == isInitialized ) {
        closeFlashMemory( !isFileRemoved );
    }
    return isFileRemoved;
}

bool FlashFileRename( const char* oldFilename, const char* newFilename ) {
    bool isInitialized = true;
    if( NULL == eepromFATFileHeaders ) {
        isInitialized = false;
        if( !initFlashMemory() || !initFileAllocationTable() ) {
            return false;
        }
    }

    bool isFileRenamed = false;
    int oldFilenameIdx = findFlashFile( oldFilename );
    int newFilenameIdx = findFlashFile( newFilename );
    if( -1 != oldFilenameIdx && // oldFilename must exists
    -1 == newFilenameIdx /*newFilename must not exists*/) {
        strncpy( eepromFATFileHeaders[oldFilenameIdx].internal.filename,
                 newFilename, MAX_FLASH_FILENAME_SIZE );
        eepromFATFileHeaders[oldFilenameIdx].internal.filename[MAX_FLASH_FILENAME_SIZE] =
                '\0';
        isFileRenamed = true;
    }

    if( false == isInitialized ) {
        closeFlashMemory( !isFileRenamed );
    }
    return isFileRenamed;
}

u32 FlashFileGetSize( const char* filename ) {
    bool isInitialized = true;
    if( NULL == eepromFATFileHeaders ) {
        isInitialized = false;
        if( !initFlashMemory() || !initFileAllocationTable() ) {
            return false;
        }
    }

    u32 fileSize = 0;
    int idx = findFlashFile( filename );
    if( -1 != idx ) {
        fileSize = eepromFATFileHeaders[idx].internal.size;
    }

    if( false == isInitialized ) {
        closeFlashMemory( true );
    }
    return fileSize;
}
u32 FlashFileGetMaxSize( const char* filename ) {
    bool isInitialized = true;
    if( NULL == eepromFATFileHeaders ) {
        isInitialized = false;
        if( !initFlashMemory() || !initFileAllocationTable() ) {
            return false;
        }
    }

    u32 fileMaxSize = 0;
    int idx = findFlashFile( filename );
    if( -1 != idx ) {
        fileMaxSize = eepromFATFileHeaders[idx].internal.maxSize;
    }

    if( false == isInitialized ) {
        closeFlashMemory( true );
    }
    return fileMaxSize;
}

static int findFlashFile( const char* filename ) {
    int index = -1;
    bool isInitialized = true;
    if( NULL == eepromFATFileHeaders ) {
        isInitialized = false;
        if( !initFlashMemory() || !initFileAllocationTable() ) {
            return index;
        }
    }

    int i;
    for( i = 0; i < eepromFATHeader.numberOfFiles; ++i ) {
        if( 0 == strncmp( filename, eepromFATFileHeaders[i].internal.filename,
        MAX_FLASH_FILENAME_SIZE ) ) {
            index = i;
            break;
        }
    }

    if( false == isInitialized ) {
        closeFlashMemory( true );
    }

    return index;
}

bool FlashFileExists( const char* filename ) {
    return findFlashFile( filename ) != -1;
}

u32 FlashFileAvailableMemory( void ) {
    bool isInitialized = true;
    if( NULL == eepromFATFileHeaders ) {
        isInitialized = false;
        if( !initFlashMemory() || !initFileAllocationTable() ) {
            return 0;
        }
    }

    u32 endAddress = eepromFATHeader.firstWritableSector;
    int highestValidIndex = findHighestValidFileHeaderIndex();
    if( -1 != highestValidIndex ) {
        FATFileHeader* fileHeader = &eepromFATFileHeaders[highestValidIndex];
        endAddress =
                fileHeader->internal.startAddress + MAX(
                        fileHeader->internal.size,
                        fileHeader->internal.maxSize );
        // align to the next sector
        endAddress = ((endAddress + (2 * SECTOR_SIZE) - 1) / SECTOR_SIZE)
                * SECTOR_SIZE;
        // endAddress should not be greater than eepromFATHeader.lastWritableSector
        endAddress = MIN( endAddress, eepromFATHeader.lastWritableSector );
    }
    u32 availableMemoryInBytes = eepromFATHeader.lastWritableSector - endAddress;

    if( false == isInitialized ) {
        closeFlashMemory( true );
    }

    return availableMemoryInBytes;
}

const u8* FlashFileGetDeviceID( void ) {
    bool isInitialized = true;
    const u8 emptyDeviceID[20] = { 0x00 };
    if( 0 == memcmp( deviceId, emptyDeviceID, sizeof(deviceId) ) ) {
        isInitialized = false;
        initFlashMemory();
    }
    if( false == isInitialized ) {
        closeFlashMemory( true );
    }
    return deviceId;
}

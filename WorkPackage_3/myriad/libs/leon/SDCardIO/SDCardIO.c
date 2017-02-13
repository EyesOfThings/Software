#include "SDCardIO.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <rtems/fsmount.h>
#include <rtems/bdpart.h>
#include <errno.h>
#include <dirent.h>
#include <OsDrvCpr.h>
#include <OsDrvSdio.h>
#include <DrvGpio.h>

#include <Crypto.h>

#define DEFAULT_SDIO_INT_PRIORITY	10
#define SDIO_SLOT_USED				1
#define SDIO_DEVNAME_USED			"/dev/sdc0"
#define SDIO_ROOT_PATH				"/mnt/sdcard"

//These defines indicate if the operation is realised by default
#define FILE_SHREDDING_DEFAULT 1 //file shreding when removing a file
#define ENCRYPTED_OPERATION_DEFAULT 1 //all operations are encrypted

// Provide info to the sd card driver
static osDrvSdioEntries_t sdCardInfo = {
	1,  // Number of slots
	DEFAULT_SDIO_INT_PRIORITY, // Interrupt priority
	{
		{
			SDIO_SLOT_USED, // Card slot to be used
			SDIO_DEVNAME_USED // Device name
		}
	}
};
/* Mount Table */
static const rtems_fstab_entry FS_TABLE [] = {
   {
     .source = SDIO_DEVNAME_USED,
     .target = SDIO_ROOT_PATH,
     .type = RTEMS_FILESYSTEM_TYPE_DOSFS,
     .options = RTEMS_FILESYSTEM_READ_WRITE,
     .report_reasons = RTEMS_FSTAB_NONE,
     .abort_reasons = RTEMS_FSTAB_OK
    },
	{
     .source = SDIO_DEVNAME_USED"1",
     .target = SDIO_ROOT_PATH,
     .type = RTEMS_FILESYSTEM_TYPE_DOSFS,
     .options = RTEMS_FILESYSTEM_READ_WRITE,
     .report_reasons = RTEMS_FSTAB_NONE,
     .abort_reasons = RTEMS_FSTAB_NONE
    }
};

// Setup the clock configuration needed by this application
static void initClock() {
    tyAuxClkDividerCfg auxClkAllOn[] = {
            { AUX_CLK_MASK_SDIO, CLK_SRC_PLL0, 1, 2 },  // SDIO
            { 0, 0, 0, 0 }                              // Null Terminated List
    };
    OsDrvCprAuxClockArrayConfig( auxClkAllOn );
}

// Configure only needed GPIOs for SDcard to work
static void configGPIOMode() {
    // SDIO
    SET_REG_WORD( GPIO_MODE16_ADR, 0x3 );		// sd_hst1_dat_3      -in-out
    SET_REG_WORD( GPIO_MODE17_ADR, 0x3 );		// sd_hst1_clk
    SET_REG_WORD( GPIO_MODE18_ADR, 0x3 );		// sd_hst1_cmd        -in-out
    SET_REG_WORD( GPIO_MODE19_ADR, 0x3 );		// sd_hst1_dat_0      -in-out
    SET_REG_WORD( GPIO_MODE20_ADR, 0x3 );		// sd_hst1_dat_1      -in-out
    SET_REG_WORD( GPIO_MODE21_ADR, 0x3 );		// sd_hst1_dat_2      -in-out
}

static bool isFolder( const char* path ) {
    struct stat st;
    stat( path, &st );
    return S_ISDIR( st.st_mode );
}
static bool isFile( const char* path ) {
    struct stat st;
    stat( path, &st );
    return S_ISREG( st.st_mode );
}

// holds true if the SD Card is initialized; otherwise false
static bool sdioInitialized = false;
// holds true if the SD Card is mounted; otherwise false
static bool isMounted = false;
// holds the number of SDCardDir- and SDCardFile- handler that are open
static u32 referenceCounter = 0;

bool SDCardMount( void ) {
    if( !isMounted ) {
        initClock();
        configGPIOMode();
        int status = 0;
        if( false == sdioInitialized ) {
            status = OsDrvSdioInit( &sdCardInfo );
            if( 0 != status ) {
                return false;
            }
            
            status = rtems_bdpart_register_from_disk(SDIO_DEVNAME_USED);
            if( 0 != status ) {
                return false;
            }

            sdioInitialized = true;
        }
        status = rtems_fsmount( FS_TABLE,
                                sizeof(FS_TABLE) / sizeof(FS_TABLE[0]), NULL );
        if( 0 != status ) {
            return false;
        }

        isMounted = true;
        return true;
    }
    return false;
}
bool SDCardUnmount( void ) {
    if( isMounted && 0 == referenceCounter ) {
        int status = 0;
        status = unmount( SDIO_ROOT_PATH );
        if( 0 == status ) {
            isMounted = false;
            return true;
        }
    }
    return false;
}
bool SDCardIsMounted( void ) {
    return isMounted;
}
bool SDCardExists( const char* name ) {
    if( NULL == name || 0 == strlen( name ) ) {
        return false;
    }
    bool _isMounted = isMounted;
    if( !_isMounted ) {
        SDCardMount();
    }

    struct stat s;
    bool exists = stat( name, &s ) != -1;

    if( !_isMounted ) {
        SDCardUnmount();
    }
    return exists;
}

SDCardDir* SDCardDirOpen( void ) {
    return SDCardDirOpenWithPath( SDIO_ROOT_PATH );
}

SDCardDir* SDCardDirOpenWithPath( const char* path ) {
    if( NULL == path ) {
        return NULL;
    }
    if( !isMounted ) {
        if( !SDCardMount() ) {
            return NULL;
        }
    }
    char internPath[PATH_MAX] = { 0 };
    const char* pch = strstr( path, SDIO_ROOT_PATH );
    if( NULL == pch || (pch-path) != 0 ) {
        if( 0 == strlen( path ) ) {
            strcpy( internPath, SDIO_ROOT_PATH );
        }
        else if( '/' == path[0] ) {
            snprintf( internPath, PATH_MAX, "%s%s", SDIO_ROOT_PATH, path );
        }
        else {
            snprintf( internPath, PATH_MAX, "%s/%s", SDIO_ROOT_PATH, path );
        }
    }
    else {
        strcpy( internPath, path );
    }
    if( '/' == internPath[strlen(internPath)-1] ) {
        internPath[strlen(internPath)-1] = '\0';
    }

    // opendir
    DIR* dir = opendir( internPath );
    if( NULL == dir ) {
        return NULL;
    }
    SDCardDir* dirHandler = malloc( sizeof(SDCardDir) );
    memset( dirHandler, 0, sizeof(SDCardDir) );

    strcpy( dirHandler->path, internPath );
    if( 0 == strcmp( internPath, SDIO_ROOT_PATH ) ) {
        strcpy( dirHandler->name, "/" );
    } else {
        strcpy( dirHandler->name, strrchr( internPath, '/' ) + 1 );
    }
    dirHandler->directory = dir;

    // increment reference counter
    ++referenceCounter;

    return dirHandler;
}

bool SDCardDirClose( SDCardDir** dirHandler ) {
    if( NULL == *dirHandler ) {
        return false;
    }
    if( 0 != closedir( (*dirHandler)->directory ) ) {
        return false;
    }

    free( *dirHandler );
    *dirHandler = NULL;

    --referenceCounter;
    if( 0 == referenceCounter ) {
        SDCardUnmount();
    }
    return true;
}

bool SDCardDirCd( SDCardDir* dirHandler, const char* dirName ) {
    if( NULL == dirHandler || NULL == dirName || 0 == strlen( dirName ) ) {
        return false;
    }
    char dirPath[NAME_MAX] = { 0 };
    snprintf( dirPath, NAME_MAX, "%s/%s", dirHandler->path, dirName );
    bool folderExists = SDCardExists( dirPath ) && isFolder( dirPath );

    if( folderExists ) {
        DIR* dir = opendir( dirPath );
        if( NULL == dir ) {
            return false;
        }
        closedir( dirHandler->directory );
        dirHandler->directory = dir;
        strcpy( dirHandler->name, dirName );
        strcpy( dirHandler->path, dirPath );
    }

    return folderExists;
}

bool SDCardDirCdUp( SDCardDir* dirHandler ) {
    if( NULL == dirHandler ) {
        return false;
    }
    if( 0 == strcmp( dirHandler->path, SDIO_ROOT_PATH ) ) {
        return false;
    }
    int subLength = strlen( dirHandler->path )
            - (strlen( dirHandler->name ) + 1);
    char dirPath[NAME_MAX] = { 0 };
    char dirName[NAME_MAX] = { 0 };
    strncpy( dirPath, dirHandler->path, subLength );
    dirPath[NAME_MAX - 1] = '\0';
    if( 0 == strcmp( dirPath, SDIO_ROOT_PATH ) ) {
        strcpy( dirName, "/" );
    } else {
        strcpy( dirName, strrchr( dirPath, '/' ) + 1 );
    }

    DIR* dir = opendir( dirPath );
    if( NULL == dir ) {
        return false;
    }
    closedir( dirHandler->directory );
    dirHandler->directory = dir;
    strcpy( dirHandler->name, dirName );
    strcpy( dirHandler->path, dirPath );

    return true;
}

u32 SDCardDirCountFiles( const char* path ) {
    if( NULL == path || 0 == strlen( path ) ) {
        return false;
    }
    SDCardDir* dirHandler = SDCardDirOpenWithPath( path );
    u32 result = SDCardDirCountFilesWithDir( dirHandler );
    SDCardDirClose( &dirHandler );
    return result;
}

u32 SDCardDirCountFilesWithDir( const SDCardDir* dirHandler ) {
    if( NULL == dirHandler ) {
        return 0;
    }
    u32 result = 0;
    struct dirent* dirEntry = NULL;
    char filepath[NAME_MAX] = { 0 };
    // reset position
    rewinddir( dirHandler->directory );
    while( NULL != (dirEntry = readdir( dirHandler->directory )) ) {
        snprintf( filepath, NAME_MAX, "%s/%s", dirHandler->path, dirEntry->d_name );
        if( isFile( filepath ) ) {
            ++result;
        }
    }
    return result;
}
const char* SDCardDirGetFilename( const SDCardDir* dirHandler, const u32 index ) {
    if( NULL == dirHandler ) {
        return NULL;
    }
    u32 inc = 0;
    struct dirent* dirEntry = NULL;
    char filepath[NAME_MAX] = { 0 };
    // reset position
    rewinddir( dirHandler->directory );
    while( NULL != (dirEntry = readdir( dirHandler->directory )) ) {
        snprintf( filepath, NAME_MAX, "%s/%s", dirHandler->path, dirEntry->d_name );
        if( isFile( filepath ) ) {
            if( index == inc ) {
                return dirEntry->d_name;
            }
            ++inc;
        }
    }
    return NULL;
}

u32 SDCardDirCountDirectories( const char* path ) {
    if( NULL == path || 0 == strlen( path ) ) {
        return false;
    }
    SDCardDir* dirHandler = SDCardDirOpenWithPath( path );
    u32 result = SDCardDirCountDirectoriesWithDir( dirHandler );
    SDCardDirClose( &dirHandler );
    return result;
}
u32 SDCardDirCountDirectoriesWithDir( const SDCardDir* dirHandler ) {
    if( NULL == dirHandler ) {
        return 0;
    }
    u32 result = 0;
    struct dirent* dirEntry = NULL;
    char filepath[NAME_MAX] = { 0 };
    rewinddir( dirHandler->directory ); // reset position
    while( NULL != (dirEntry = readdir( dirHandler->directory )) ) {
        if( !strcmp( dirEntry->d_name, "." ) ||
            !strcmp( dirEntry->d_name, ".." ) ) {
            continue;
        }
        snprintf( filepath, NAME_MAX, "%s/%s", dirHandler->path, dirEntry->d_name );
        if( isFolder( filepath ) ) {
            ++result;
        }
    }
    return result;
}
const char* SDCardDirGetDirectoryName( const SDCardDir* dirHandler,
                                       const u32 index ) {
    if( NULL == dirHandler ) {
        return NULL;
    }
    u32 inc = 0;
    struct dirent* dirEntry = NULL;
    char dirpath[NAME_MAX] = { 0 };
    // reset position
    rewinddir( dirHandler->directory );
    while( NULL != (dirEntry = readdir( dirHandler->directory )) ) {
        if( !strcmp( dirEntry->d_name, "." )
            || !strcmp( dirEntry->d_name, ".." ) ) {
            continue;
        }
        snprintf( dirpath, NAME_MAX, "%s/%s", dirHandler->path, dirEntry->d_name );
        if( isFolder( dirpath ) ) {
            if( index == inc ) {
                return dirEntry->d_name;
            }
            ++inc;
        }
    }
    return NULL;
}

bool SDCardDirMakeDirectory( const char* path ) {
    if( NULL == path || 0 == strlen( path ) ) {
        return false;
    }
    char internalPath[PATH_MAX] = { 0 };
    char dirName[PATH_MAX] = { 0 };
    strcpy( internalPath, path );
    if( '/' == internalPath[strlen(internalPath)-1] ) {
        internalPath[strlen(internalPath)-1] = '\0';
    }
    strcpy( dirName, strrchr( internalPath, '/' ) + 1 );
    *(strrchr( internalPath, '/' )) = '\0';

    SDCardDir* dirHandler = SDCardDirOpenWithPath( internalPath );
    bool succeeded = SDCardDirMakeDirectoryWithDir( dirHandler, dirName );
    SDCardDirClose( &dirHandler );
    return succeeded;
}
bool SDCardDirMakeDirectoryWithDir( const SDCardDir* dirHandler, const char* dirName ) {
    if( NULL == dirHandler || NULL == dirName || 0 == strlen( dirName ) ) {
        return false;
    }
    char makeDirectory[NAME_MAX] = { 0 };
    snprintf( makeDirectory, NAME_MAX, "%s/%s", dirHandler->path, dirName );
    return 0 == mkdir( makeDirectory,
                       S_IRUSR | S_IWUSR | S_IXUSR |
                       S_IRGRP | S_IWGRP | S_IXGRP |
                       S_IROTH | S_IWOTH | S_IXOTH );
}

bool SDCardDirRemoveDirectory( const char* path ) {
    if( NULL == path || 0 == strlen( path ) ) {
        return false;
    }
    char dirName[NAME_MAX] = { 0 };
    SDCardDir* dirHandler = SDCardDirOpenWithPath( path );
    if( NULL == dirHandler ) {
        return false;
    }
    strcpy( dirName, dirHandler->name );
    SDCardDirCdUp( dirHandler );
    bool removedDir = SDCardDirRemoveDirectoryWithDir( dirHandler, dirName );
    SDCardDirClose( &dirHandler );
    return removedDir;
}
bool SDCardDirRemoveDirectoryWithDir( const SDCardDir* dirHandler,
                                      const char* dirName ) {
    if( NULL == dirHandler || NULL == dirName || 0 == strlen( dirName ) ) {
        return false;
    }
    char removeDirectory[NAME_MAX] = { 0 };
    snprintf( removeDirectory, NAME_MAX, "%s/%s", dirHandler->path, dirName );
    return 0 == rmdir( removeDirectory );
}

bool SDCardDirRemoveDirectoryRecursive( const char* path ) {
    if( NULL == path || 0 == strlen( path ) ) {
        return false;
    }
    SDCardDir* dirHandler = SDCardDirOpenWithPath( path );
    if( NULL == dirHandler )  {
        return false;
    }
    char dirName[NAME_MAX] = { 0 };
    strcpy( dirName, dirHandler->name );
    SDCardDirCdUp( dirHandler );
    bool removed = SDCardDirRemoveDirectoryRecursiveWithDir( dirHandler, dirName );
    SDCardDirClose( &dirHandler );
    return removed;
}
bool SDCardDirRemoveDirectoryRecursiveWithDir( SDCardDir* dirHandler,
                                               const char* dirName ) {
    if( NULL == dirHandler || NULL == dirName || 0 == strlen(dirName) ) {
        return false;
    }
    char internalPath[PATH_MAX] = { 0 };
    strcpy( internalPath, dirHandler->path );

    SDCardDirCd( dirHandler, dirName );

    while( 0 != strcmp( internalPath, dirHandler->path ) ) {
        while( 0 != SDCardDirCountFilesWithDir( dirHandler ) ) {
            SDCardFileRemoveWithDir( dirHandler, SDCardDirGetFilename( dirHandler, 0 ) );
        }
        if( 0 != SDCardDirCountDirectoriesWithDir( dirHandler ) ) {
            SDCardDirCd( dirHandler, SDCardDirGetDirectoryName( dirHandler, 0 ) );
        }
        else {
            char folderName[PATH_MAX] = { 0 };
            strcpy( folderName, dirHandler->name );
            SDCardDirCdUp( dirHandler );
            SDCardDirRemoveDirectoryWithDir( dirHandler, folderName );
        }
    }

    return true;
}

const char* SDCardDirGetName( const SDCardDir* dirHandler ) {
    if( NULL == dirHandler ) {
        return NULL;
    }
    return dirHandler->name;
}

const char* SDCardDirGetPath( const SDCardDir* dirHandler ) {
    if( NULL == dirHandler ) {
        return NULL;
    }
    return dirHandler->path;
}

bool SDCardDirExists( const SDCardDir* dirHandler, const char* name ) {
    if( NULL == dirHandler || NULL == name || 0 == strlen( name ) ) {
        return false;
    }
    char dirOrFile[NAME_MAX] = { 0 };
    snprintf( dirOrFile, NAME_MAX, "%s/%s", dirHandler->path, name );
    return SDCardExists( dirOrFile );
}

bool SDCardRemoveAllFromDirectory( const char* path ) {
    if( NULL == path || 0 ==  strlen( path ) ) {
        return false;
    }
    SDCardDir* dirHandler = SDCardDirOpenWithPath( path );
    bool removedAll = SDCardRemoveAllFromDirectoryWithDir( dirHandler );
    SDCardDirClose( &dirHandler );
    return removedAll;
}
bool SDCardRemoveAllFromDirectoryWithDir( SDCardDir* dirHandler ) {
    if( NULL == dirHandler ) {
        return false;
    }
    // delete all files
    while( 0 != SDCardDirCountFilesWithDir( dirHandler ) ) {
        SDCardFileRemoveWithDir( dirHandler, SDCardDirGetFilename( dirHandler, 0 ) );
    }
    // delet all sub directories
    while( 0 != SDCardDirCountDirectoriesWithDir( dirHandler ) ) {
        SDCardDirRemoveDirectoryRecursiveWithDir( dirHandler, SDCardDirGetDirectoryName( dirHandler, 0 ) );
    }

    return true;
}

SDCardEntryStatus* SDCardLs( const char* path, int* size ) {
    if( NULL == path || NULL == size ) {
        return NULL;
    }
    SDCardDir* dirHandler = SDCardDirOpenWithPath( path );
    SDCardEntryStatus* list = SDCardLsWithDir( dirHandler, size );
    SDCardDirClose( &dirHandler );
    return list;
}
SDCardEntryStatus* SDCardLsWithDir( const SDCardDir* dirHandler, int* size ) {
    if( NULL == dirHandler || NULL == size ) {
        return NULL;
    }
    SDCardEntryStatus* list = NULL;
    int numberOfEntries = SDCardDirCountDirectoriesWithDir( dirHandler ) +
            SDCardDirCountFilesWithDir( dirHandler );
    if( 0 == numberOfEntries ) {
        *size = 0;
        return NULL;
    }
    list = malloc( sizeof(SDCardEntryStatus)*numberOfEntries );
    if( NULL == list ) {
        *size = 0;
        return NULL;
    }

    int index = 0;
    struct dirent* directoryEntry = NULL;
    rewinddir( dirHandler->directory ); // reset position
    directoryEntry = NULL;
    while( NULL != (directoryEntry = readdir( dirHandler->directory )) ) {
        if( !strcmp( directoryEntry->d_name, "." ) ||
            !strcmp( directoryEntry->d_name, ".." ) ) {
            continue;
        }
        char filepath[NAME_MAX] = { 0 };
        snprintf( filepath, NAME_MAX, "%s/%s", dirHandler->path, directoryEntry->d_name );
        strcpy( list[index].name, directoryEntry->d_name );
        list[index].isFile = isFile( filepath );
        struct stat st;
        stat( filepath, &st );
        list[index].changeTime = *localtime( &st.st_ctime );
        list[index].accessTime = *localtime( &st.st_atime );
        list[index].modificationTime = *localtime( &st.st_mtime );
        ++index;
    }
    *size = numberOfEntries;

    return list;
}
void SDCardEntryStatusDestroy( SDCardEntryStatus** entries ) {
    free( *entries );
    *entries = NULL;
}

SDCardFile* SDCardFileOpen( const char* filename, const char* mode,
                            bool enableEncryption ) {
    if( NULL == filename || 0 == strlen( filename )
        || NULL == mode
        || 0 == strlen( mode )
        || (true == enableEncryption && NULL != strchr( mode, 'a' )) ) {
        return NULL;
    }
    if( !isMounted ) {
        if( !SDCardMount() ) {
            return NULL;
        }
    }
    FILE* pfile = fopen( filename, mode );
    if( NULL == pfile ) {
        return NULL;
    }
    SDCardFile* fileHandler = malloc( sizeof(SDCardFile) );
    
    if (ENCRYPTED_OPERATION_DEFAULT)
        fileHandler->encryptionEnabled = true;
    else
        fileHandler->encryptionEnabled = enableEncryption;
    
    fileHandler->file = pfile;

    // increment reference counter
    ++referenceCounter;

    return fileHandler;
}

SDCardFile* SDCardFileOpenWithDir( const SDCardDir* dirHandler, const char* filename,
                                   const char* mode, bool enableEncryption ) {
    if( NULL == dirHandler ) {
        return NULL;
    }
    char filePathAndName[NAME_MAX] = { 0 };
    snprintf( filePathAndName, NAME_MAX, "%s/%s", dirHandler->path, filename );

    return SDCardFileOpen( filePathAndName, mode, enableEncryption );
}

bool SDCardFileClose( SDCardFile** fileHandler ) {
    if( NULL == *fileHandler ) {
        return false;
    }
    if( 0 != fclose( (*fileHandler)->file ) ) {
        return false;
    }

    free( *fileHandler );
    *fileHandler = NULL;

    --referenceCounter;
    if( 0 == referenceCounter ) {
        SDCardUnmount();
    }
    return true;
}

bool SDCardFileRemove( const char* filename ) {

    bool removed;

    if (FILE_SHREDDING_DEFAULT) {
        removed = SDCardFileShred(filename);
    } else {

        if (NULL == filename || 0 == strlen(filename)) {
            return false;
        }
        bool _isMounted = isMounted;
        if (!_isMounted) {
            SDCardMount();
        }
        removed = false;
        if (SDCardExists(filename) && isFile(filename)) {
            removed = 0 == remove(filename);
        }

        if (!_isMounted) {
            SDCardUnmount();
        }
    }
    
    return removed;
}
bool SDCardFileRemoveWithDir( const SDCardDir* dirHandler, const char* filename ) {
    if( NULL == dirHandler ) {
        return false;
    }
    char filePathAndName[NAME_MAX] = { 0 };
    snprintf( filePathAndName, NAME_MAX, "%s/%s", dirHandler->path, filename );

    return SDCardFileRemove( filePathAndName );
}

bool SDCardFileRename( const char* currentFilename, const char* newFilename ) {
    if( NULL == currentFilename || 0 == strlen( currentFilename ) ||
    NULL == newFilename
        || 0 == strlen( newFilename ) ) {
        return false;
    }
    bool _isMounted = isMounted;
    if( !_isMounted ) {
        SDCardMount();
    }
    bool renamed = false;
    if( SDCardExists( currentFilename ) && isFile( currentFilename )
        && !SDCardExists( newFilename ) ) {
        renamed = 0 == rename( currentFilename, newFilename );
    }

    if( !_isMounted ) {
        SDCardUnmount();
    }
    return renamed;
}

bool SDCardFileCopy( const char* sourceFilename, const char* destFilename ) {
    if( NULL == sourceFilename || 0 == strlen( sourceFilename ) ||
    NULL == destFilename
        || 0 == strlen( destFilename ) ) {
        return false;
    }
    bool _isMounted = isMounted;
    if( !_isMounted ) {
        SDCardMount();
    }
    bool copied = false;
    if( SDCardExists( sourceFilename ) && isFile( sourceFilename )
        && !SDCardExists( destFilename ) ) {

        int sourceFileDescriptor = open( sourceFilename, O_RDONLY );
        int destFileDescriptor = open( destFilename, O_WRONLY | O_CREAT );
        if( -1 != sourceFileDescriptor && -1 != destFileDescriptor ) {

            u32 size;
            char buffer[BUFSIZ];

            while( (size = read( sourceFileDescriptor, buffer, BUFSIZ )) > 0 ) {
                write( destFileDescriptor, buffer, size );
            }
            copied = true;
        }
        close( sourceFileDescriptor );
        close( destFileDescriptor );
    }

    if( !_isMounted ) {
        SDCardUnmount();
    }
    return copied;
}

u32 SDCardFileGetSize( SDCardFile* fileHandler ) {
    if( NULL == fileHandler ) {
        return 0;
    }
    u32 sizeInByte = 0;
    long pos = ftell( fileHandler->file );

    fseek( fileHandler->file, 0, SEEK_END );
    sizeInByte = ftell( fileHandler->file );

    fseek( fileHandler->file, pos, SEEK_SET );
    return sizeInByte;
}

u32 SDCardFileWrite( SDCardFile* fileHandler, const u8* writeBuffer,
                     const u32 writeBufferSize ) {
    if( NULL == fileHandler || NULL == writeBuffer || 0 == writeBufferSize ) {
        return 0;
    }
    u32 sizeInByte = 0;
    if( fileHandler->encryptionEnabled ) {
        sizeInByte = CryptoFileWrite( fileHandler->file, writeBuffer,
                                      writeBufferSize );
    } else {
        sizeInByte = fwrite( writeBuffer, sizeof(u8), writeBufferSize,
                             fileHandler->file );
    }
    return sizeInByte;
}

u32 SDCardFileRead( SDCardFile* fileHandler, u8* readBuffer,
                    const u32 readBufferSize ) {
    if( NULL == fileHandler || NULL == readBuffer || 0 == readBufferSize ) {
        return 0;
    }
    u32 sizeInByte = 0;
    if( fileHandler->encryptionEnabled ) {
        sizeInByte = CryptoFileRead( fileHandler->file, readBuffer,
                                     readBufferSize );
    } else {
        sizeInByte = fread( readBuffer, sizeof(u8), readBufferSize,
                            fileHandler->file );
    }

    return sizeInByte;
}

u32 SDCardFilePeek( SDCardFile* fileHandler, u8* readBuffer,
                    const u32 readBufferSize ) {
    if( NULL == fileHandler ) {
        return 0;
    }
    long pos = ftell( fileHandler->file );
    u32 sizeInByte = SDCardFileRead( fileHandler, readBuffer, readBufferSize );
    fseek( fileHandler->file, pos, SEEK_SET );

    return sizeInByte;
}

bool SDCardFileSetPosition( SDCardFile* fileHandler, const u32 pos ) {
    if( NULL == fileHandler ) {
        return false;
    }
    return 0 == fseek( fileHandler->file, pos, SEEK_SET );
}

u32 SDCardFileGetPostion( SDCardFile* fileHandler ) {
    if( NULL == fileHandler ) {
        return 0;
    }
    return ftell( fileHandler->file );
}

bool SDCardFileFlush( SDCardFile* fileHandler ) {
    if( NULL == fileHandler ) {
        return false;
    }
    return 0 == fflush( fileHandler->file );
}



bool SDCardFileShred( const char* filename ) {
    bool boolean = false;
    SDCardFile* fileHandler=SDCardFileOpen(filename, "r+", false );
    u32 size = SDCardFileGetSize(fileHandler);
    
    //3 Write n 0’s to the file
    u8* writeBuffer = malloc(sizeof(u8) * size);
    memset(writeBuffer,0,size);
    size = SDCardFileWrite( fileHandler, writeBuffer, size );
    
    boolean = SDCardFileFlush( fileHandler );
    boolean = SDCardFileClose( &fileHandler );
    
    //boolean = SDCardFileRemove(filename );
    bool _isMounted = isMounted;
    if (!_isMounted) {
        SDCardMount();
    }

    boolean = false;
    if (SDCardExists(filename) && isFile(filename)) {
        boolean = 0 == remove(filename);
    }
    
    if (!_isMounted) {
        SDCardUnmount();
    }
    
    return boolean;
}
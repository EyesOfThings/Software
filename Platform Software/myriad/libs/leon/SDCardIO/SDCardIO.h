#ifndef _SD_CARD_IO_H_
#define _SD_CARD_IO_H_

#ifndef __RTEMS__
#error "SDCardIO depends on RTEMS"
#else

#include <stdbool.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include <sys/syslimits.h>
#include <mv_types.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct __sSDCardDir {
        DIR* directory;
        char name[NAME_MAX];
        char path[PATH_MAX];
    } SDCardDir;
    typedef struct __sSDCardFile {
        FILE* file;
        bool encryptionEnabled;
    } SDCardFile;
    typedef struct __sSDCardEntryStatus {
        char name[NAME_MAX];
        bool isFile;
        struct tm changeTime;
        struct tm accessTime;
        struct tm modificationTime;
    } SDCardEntryStatus;
    /**
     * Try to mount a SD Card.
     * Returns true on success; otherwise returns false.
     */
    bool SDCardMount( void );
    /**
     * Try to unmount a SD Card.
     * Returns true on success; otherwise returns false.
     */
    bool SDCardUnmount( void );
    /**
     * Returns true if the SD Card is mounted; otherwise returns false.
     */
    bool SDCardIsMounted( void );
    /**
     * Returns true if the directory/file called name exists; otherwise returns false.
     */
    bool SDCardExists( const char* name );
    /**
     * Opens the root directory of the SD Card.
     */
    SDCardDir* SDCardDirOpen( void );
    /**
     * Returns a valid pointer if successful; otherwise returns NULL.
     */
    SDCardDir* SDCardDirOpenWithPath( const char* path );
    /**
     * Close the SDCardDir handler.
     * @param[out] dirHandler pointer to a SDCardDir object
     * @return If the SDCardDir-handler is successfully closed, the function returns true.
     * Otherwise, false is returned.
     */
    bool SDCardDirClose( SDCardDir** dirHandler );
    /**
     * Changes directory to dirName.
     * Return true if the directory exists; otherwise returns false.
     */
    bool SDCardDirCd( SDCardDir* dirHandler, const char* dirName );
    /**
     * Changes directory by moving one directory up.
     * Returns true if the directory exists; otherwise returns false.
     */
    bool SDCardDirCdUp( SDCardDir* dirHandler );
    /**
     * Returns the total number of files in a directory specified by path.
     */
    u32 SDCardDirCountFiles( const char* path );
    /**
     * Returns the total number of files in the current directory.
     */
    u32 SDCardDirCountFilesWithDir( const SDCardDir* dirHandler );
    /**
     * Return a filename if the index is valid; otherwise returns null.
     */
    const char* SDCardDirGetFilename( const SDCardDir* dirHandler,
                                      const u32 index );
    /**
     * Returns the total number of folders in a directory specified by path.
     */
    u32 SDCardDirCountDirectories( const char* path );
    /**
     * Returns the total number of folders in the directory.
     */
    u32 SDCardDirCountDirectoriesWithDir( const SDCardDir* dirHandler );
    /**
     * Return a directory name if the index is valid; otherwise returns null.
     */
    const char* SDCardDirGetDirectoryName( const SDCardDir* dirHandler,
                                           const u32 index );
    /**
     * Create a sub-directory.
     * Returns true on success; otherwise returns false.
     */
    bool SDCardDirMakeDirectory( const char* path );
    /**
     * Create a sub-directory called dirName.
     * Returns true on success; otherwise returns false.
     */
    bool SDCardDirMakeDirectoryWithDir( const SDCardDir* dirHandler,
                                        const char* dirName );
    /**
     * Removes the directory specified by path.
     * The directory must be empty to succeed.
     * Return true if successful; otherwise returns false.
     */
    bool SDCardDirRemoveDirectory( const char* path );
    /**
     * Removes the directory specified by dirName.
     * The directory must be empty to succeed.
     * Return true if successful; otherwise returns false.
     */
    bool SDCardDirRemoveDirectoryWithDir( const SDCardDir* dirHandler,
                                          const char* dirName );
    /**
     * Removes the directory specified by path.
     * Return true if successful; otherwise returns false.
     */
    bool SDCardDirRemoveDirectoryRecursive( const char* path );
    /**
     * Removes the directory specified by dirName.
     * Return true if successful; otherwise returns false.
     */
    bool SDCardDirRemoveDirectoryRecursiveWithDir( SDCardDir* dirHandler,
                                                   const char* dirName );
    /**
     * Returns the name of the directory.
     */
    const char* SDCardDirGetName( const SDCardDir* dirHandler );
    /**
     * Returns the path. The returned path is absolute.
     */
    const char* SDCardDirGetPath( const SDCardDir* dirHandler );
    /**
     * Returns true if the directory/file called name exists; otherwise returns false.
     */
    bool SDCardDirExists( const SDCardDir* dirHandler, const char* name );
    /**
     * Delete all Elements inside the directory.
     */
    bool SDCardRemoveAllFromDirectory( const char* path );
    /**
     * Delete all Elements inside the directory.
     */
    bool SDCardRemoveAllFromDirectoryWithDir( SDCardDir* dirHandler );
    /**
     * Returns an array of SDCardEntryStatus objects for all the files and directories in the directory.
     * Returns a NULL if the directory is unreadable, does not exist or is empty.
     */
    SDCardEntryStatus* SDCardLs( const char* path, int* size );
    /**
     * Returns an array of SDCardEntryStatus objects for all the files and directories in the directory.
     * Returns a NULL if the directory is unreadable, does not exist or is empty.
     */
    SDCardEntryStatus* SDCardLsWithDir( const SDCardDir* dirHandler, int* size );
    /**
     * Frees an array of SDCardEntryStatus objects.
     */
    void SDCardEntryStatusDestroy( SDCardEntryStatus** entries );
    /**
     * Returns a valid pointer if successful; otherwise returns NULL.
     * @param[in] filename represent the file with the given name (absolute path)
     * @param[in] mode supports read ("r") and write ("w") access.
     * For more details @see [fopen](http://www.cplusplus.com/reference/cstdio/fopen/)
     * @param[in] enableEncryption encrypt and decrypt automatically if true
     * @return If the file is successfully opened, the function returns a pointer to a SDCardFile object.
     * Otherwise, a null pointer is returned.
     */
    SDCardFile* SDCardFileOpen( const char* filename, const char* mode,
                                bool enableEncryption );
    /**
     * Returns a valid pointer if successful; otherwise returns NULL.
     * @param[in] dirHandler holds the current path
     * @param[in] filename represent the file with the given name
     * @param[in] mode supports read ("r") and write ("w") access.
     * For more details @see [fopen](http://www.cplusplus.com/reference/cstdio/fopen/)
     * @param[in] enableEncryption encrypt and decrypt automatically if true
     * @return If the file is successfully opened, the function returns a pointer to a SDCardFile object.
     * Otherwise, a null pointer is returned.
     */
    SDCardFile* SDCardFileOpenWithDir( const SDCardDir* dirHandler, const char* filename,
                                       const char* mode, bool enableEncryption );
    /**
     * Close the SDCardFile handler.
     * @param[out] fileHandler pointer to a SDCardFile object
     * @return If the SDCardFile-handler is successfully closed, the function returns true.
     * Otherwise, false is returned.
     */
    bool SDCardFileClose( SDCardFile** fileHandler );
    /**
     * Removes the file specified by the 'filename'(absolute path) given.
     * Returns true if successful; otherwise return false.
     */
    bool SDCardFileRemove( const char* filename );
    /**
     * Removes the file specified by the 'filename' given.
     * Returns true if successful; otherwise return false.
     */
    bool SDCardFileRemoveWithDir( const SDCardDir* dirHandler, const char* filename );
    /**
     * Rename the file 'currentFilename' to 'newFilename'. Return true if successful; otherwise returns false.
     * If a file with the name 'newFilename' already exists, SDCardFileRename() returns false.
     */
    bool SDCardFileRename( const char* currentFilename,
                           const char* newFilename );
    /**
     * Copy the file 'sourceFilename' to 'destFilename'. Return true if successful; otherwise returns false.
     * If a file with the name 'destFilename' already exists, SDCardFileCopy() returns false.
     */
    bool SDCardFileCopy( const char* sourceFilename, const char* destFilename );

    /**
     * Returns the size of the file.
     */
    u32 SDCardFileGetSize( SDCardFile* fileHandler );
    /**
     * Writes at most 'writeBufferSize' bytes of data from 'writeBuffer' to the file.
     * Returns the number of bytes that were actually written, or 0 if an error occurred.
     * @param[in] fileHandler pointer to a SDCardFile object
     * @param[in] writeBuffer a data array
     * @param[in] writeBufferSize number of bytes
     */
    u32 SDCardFileWrite( SDCardFile* fileHandler, const u8* writeBuffer,
                         const u32 writeBufferSize );
    /**
     * Reads at most 'readBufferSize' bytes from the file into 'readBuffer', and returns the number of bytes read.
     * If an error occurs, such as when attempting to read from a file opened in WriteOnly mode, this function returns 0.
     * @param[in] fileHandler pointer to a SDCardFile object
     * @param[out] readBuffer a data array
     * @param[in] readBufferSize number of bytes
     */
    u32 SDCardFileRead( SDCardFile* fileHandler, u8* readBuffer,
                        const u32 readBufferSize );
    /**
     * Reads at most 'readBufferSize' bytes from the file into 'readBuffer', without side effects (i.e., if you call
     * SDCardFileRead() after SDCardFilePeek(), you will get the same data). Returns the number of bytes read. If an error occurs,
     * such as when attempting to peek a file opened in WriteOnly mode, this function returns 0.
     */
    u32 SDCardFilePeek( SDCardFile* fileHandler, u8* readBuffer,
                        const u32 readBufferSize );
    /**
     * Sets the current position from the fileHandler to 'pos'.
     * Returns true on success, or false if an error occurred.
     */
    bool SDCardFileSetPosition( SDCardFile* fileHandler, const u32 pos );
    /**
     * Returns the current position from the fileHandler.
     */
    u32 SDCardFileGetPostion( SDCardFile* fileHandler );
    /**
     * Flushes any buffered data to the file.
     * Returns true if successful; otherwise returns false.
     */
    bool SDCardFileFlush( SDCardFile* fileHandler );
     /**
     * This function shreds a file, specified by the 'filename'(absolute path) given, writing 0’s in the card before deleting it, protecting it from a malicious user.
     * Returns true if successful; otherwise returns false.
     */
    bool SDCardFileShred( const char* filename );

#ifdef __cplusplus
}
#endif

#endif

#endif /* _SD_CARD_IO_H_ */
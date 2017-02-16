/**
 * @file Audio.h
 *
 * @brief This file provides functions for read/write files from/to the Flash-EEPROM.
 */
#ifndef _FLASH_IO_H_
#define _FLASH_IO_H_

#include <stdbool.h>
#include <mv_types.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * This enum is used with FlashFileOpen() to describe the mode in which a FlashFile is opened.
     */
    typedef enum __eFlashFileMode {
        ReadOnly,           // The FlashFile is open for reading.
        WriteOnly,          // The FlashFile is open for writing.
                            //  Note: all earlier contents are lost, If the file exists.
        AppendOnly // The FlashFile is opened in append mode so that all data is written to the end of the file.
    } FlashFileMode;
    /**
     * This enum describes the errors that may be returned by the errorFlashFile() function.
     */
    typedef enum __eFlashFileError {
        NoError,            // No error occurred.
        ReadError,          // An error occurred when reading from the file.
        WriteError,         // An error occurred when writing to the file.
        FatalError,         // A fatal error occurred.
        ResourceError, // Out of resources (e.g., too many open files, out of memory, etc.)
        OpenError,          // The file could not be opened.
        AbortError,         // The operation was aborted.
        UnspecifiedError,   // An unspecified error occurred.
        RemoveError,        // The file could not be removed.
        RenameError,        // The file could not be renamed.
        PositionError,      // The position in the file could not be changed.
        ResizeError,        // The file could not be resized.
        PermissionError,    // The file could not be accessed.
        CopyError           // The file could not be copied.
    } FlashFileError;

    /**
     * Object containing information to control a stream.
     * FlashFile objects are created by a call of FlashFileOpen, which returns a pointer to one of these objects.
     */
    typedef struct __sFlashFile {
        int fileHeaderIdx;  // This property holds an index value.
        FlashFileMode mode; // This property holds the mode.
        u32 position;        // This property holds the current position.
        bool created; // This property holds true if file is new; otherwise holds false.
    } FlashFile;

    /**
     * Returns a valid pointer if successful; otherwise returns NULL.
     * @param[in] filename represent the file with the given name
     * @param[in] mode the mode must be ReadOnly, WriteOnly or Append
     * @return If the file is successfully opened, the function returns a pointer to a FlashFile object.
     * Otherwise, a null pointer is returned.
     */
    FlashFile* FlashFileOpen( const char* filename, const FlashFileMode mode );
    /**
     * Close the FlashFile handler.
     * @param[out] fileHandler pointer to a FlashFile object
     * @return If the FlashFile-handler is successfully closed, the function returns true.
     * Otherwise, false is returned.
     */
    bool FlashFileClose( FlashFile** fileHandler );
    /**
     * Writes at most 'writeBufferSize' bytes of data from 'writeBuffer' to the file.
     * Returns the number of bytes that were actually written, or 0 if an error occurred.
     * @param[in] fileHandler pointer to a FlashFile object
     * @param[in] writeBuffer a data array
     * @param[in] writeBufferSize number of bytes
     */
    u32 FlashFileWrite( FlashFile* fileHandler, const u8* writeBuffer,
                        const u32 writeBufferSize );
    /**
     * Reads at most 'readBufferSize' bytes from the file into 'readBuffer', and returns the number of bytes read.
     * If an error occurs, such as when attempting to read from a file opened in WriteOnly mode, this function returns 0.
     * @param[in] fileHandler pointer to a FlashFile object
     * @param[out] readBuffer a data array
     * @param[in] readBufferSize number of bytes
     */
    u32 FlashFileRead( FlashFile* fileHandler, u8* readBuffer,
                       const u32 readBufferSize );
    /**
     * Reads at most 'readBufferSize' bytes from the file into 'readBuffer', without side effects (i.e., if you call
     * FlashFileRead() after FlashFilePeek(), you will get the same data). Returns the number of bytes read. If an error occurs,
     * such as when attempting to peek a file opened in WriteOnly mode, this function returns 0.
     */
    u32 FlashFilePeek( const FlashFile* fileHandler, u8* readBuffer,
                       const u32 readBufferSize );
    /**
     * Sets the current position from the fileHandler to 'pos'.
     * Returns true on success, or false if an error occurred.
     * Note: This function will only change the position if the fileHandler is in ReadOnly mode.
     */
    bool FlashFileSetPosition( FlashFile* fileHandler, const u32 pos );
    /**
     * Returns the current position from the fileHandler.
     */
    u32 FlashFileGetPosition( const FlashFile* fileHandler );
    /**
     * Returns the available space for the fileHandler in bytes.
     */
    u32 FlashFileGetAvailableSpace( FlashFile* fileHandler );
    /**
     * Removes the file specified by the 'filename' given.
     * Returns true if successful; otherwise return false.
     */
    bool FlashFileRemove( const char* filename );
    /**
     * Rename the file 'oldFilename' to 'newFilename'. Return true if successful; otherwise returns false.
     * If a file with the name 'newFilename' already exists, FlashFileRename() returns false.
     */
    bool FlashFileRename( const char* oldFilename, const char* newFilename );
    /**
     * Returns the size of the file.
     */
    u32 FlashFileGetSize( const char* filename );
    /**
     * Returns the maximum size of the file
     * If the file is not existing or the maximum size is not set yet, this function returns 0.
     */
    u32 FlashFileGetMaxSize( const char* filename );
    /**
     * Returns true if the file specified by 'filename' exists; otherwise returns false.
     */
    bool FlashFileExists( const char* filename );
    /**
     * Returns the available space of the flash in bytes.
     */
    u32 FlashFileAvailableMemory( void );
    /**
     * Returns the device id.
     */
    const u8* FlashFileGetDeviceID( void );

#ifdef __cplusplus
}
#endif

#endif /* _FLASH_IO_H_ */

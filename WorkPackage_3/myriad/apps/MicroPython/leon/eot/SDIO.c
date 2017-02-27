/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "SDIO.h"
#include <string.h>
#include <mv_types.h>

/**
 * Try to mount a SD Card.
 * \note Python syntax: SDIO.mount(), Where SDIO is the import name of this module.
 * @return **int** Returns 1 on success; otherwise returns 0.
 */
static mp_obj_t eot_SDIO_mount() {
    return mp_obj_new_int (SDCardMount());
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(eot_SDIO_mount_obj, eot_SDIO_mount);

/**
 * Check if SD card is mounted.
 * \note Python syntax: SDIO.isMounted(), Where SDIO is the import name of this module.
 * @return **int**  Returns 1 if the SD Card is mounted; otherwise returns 0.
 */
static mp_obj_t eot_SDIO_isMounted() {
    return  mp_obj_new_int (SDCardIsMounted());
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(eot_SDIO_isMounted_obj, eot_SDIO_isMounted);

/**
 * Try to unmount a SD Card.
 * \note Python syntax: SDIO.unmount(), Where SDIO is the import name of this module.
 * @return **int** Returns 1 on success; otherwise returns 0.
 */
static mp_obj_t eot_SDIO_unmount() {
    return mp_obj_new_int (SDCardUnmount());
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(eot_SDIO_unmount_obj, eot_SDIO_unmount);

/**
 * Check if the directory/file called name exists.
 * \note Python syntax: SDIO.exists(path), Where SDIO is the import name of this module.
 * \note Python example: SDIO.exists ("/mnt/sdcard/lena.png")
 * @param path:string File/Directory path
 * @return **int**  Returns 1 if the directory/file called name exists; otherwise 0 false.
 */
static mp_obj_t eot_SDIO_exists (mp_obj_t path) {
    return mp_obj_new_int(SDCardExists (mp_obj_str_get_str(path)));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_SDIO_exists_obj, eot_SDIO_exists);
/**
 * Removes the file specified by the 'filename'(absolute path) given.
 * \note Python syntax: SDIO.FileRemove(path), Where SDIO is the import name of this module.
 * \note Python example: SDIO.FileRemove ("/mnt/sdcard/lena.png")
 * @param path:string File path
 * @return **int** Returns 1 on success; otherwise returns 0.
 */
static mp_obj_t eot_SDIO_FileRemove (mp_obj_t path) {
    return mp_obj_new_int(SDCardFileRemove (mp_obj_str_get_str(path)));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_SDIO_FileRemove_obj, eot_SDIO_FileRemove);

/**
 * Returns a list of list. Each list contains the information of all files and directories in the directory.
 * \note Python syntax: SDIO.Ls(path), Where SDIO is the import name of this module.
 * \note Python syntax: SDIO.Ls("/mnt/sdcard")
 * @param path:string File/Directory path
 * @return **list of lists** The list of list contains the information of all file and directories in the path. [ [name, isFile, changeTime, accessTime, modificationTime], [name, isFile, changeTime, accessTime, modificationTime] ]
 */
static mp_obj_t eot_SDIO_Ls (mp_obj_t path) {
    int size;
    SDCardEntryStatus* SDlist = SDCardLs(mp_obj_str_get_str(path), &size);

    mp_obj_list_t *resLs = MP_OBJ_TO_PTR(mp_obj_new_list(size, NULL));
    int i = 0;
    for (i=0; i < size; i++) {
        struct tm cTime = SDlist[i].changeTime;
        struct tm aTime = SDlist[i].accessTime;
        struct tm mTime = SDlist[i].modificationTime;
        resLs->items[i] = MP_OBJ_TO_PTR(mp_obj_new_list(5, NULL));
        ((mp_obj_list_t*) (resLs->items[i]))->items[0] = (mp_obj_str_t*) MP_OBJ_TO_PTR(mp_obj_new_str(SDlist[i].name, strlen(SDlist[i].name), false));
        ((mp_obj_list_t*) (resLs->items[i]))->items[1] = mp_obj_new_int(SDlist[i].isFile);
        ((mp_obj_list_t*) (resLs->items[i]))->items[2] = (mp_obj_str_t*) MP_OBJ_TO_PTR(mp_obj_new_str(asctime(&cTime),  strlen(asctime(&cTime))-1, false));
        ((mp_obj_list_t*) (resLs->items[i]))->items[3] = (mp_obj_str_t*) MP_OBJ_TO_PTR(mp_obj_new_str(asctime(&aTime),  strlen(asctime(&aTime))-1, false));
        ((mp_obj_list_t*) (resLs->items[i]))->items[4] = (mp_obj_str_t*) MP_OBJ_TO_PTR(mp_obj_new_str(asctime(&mTime),  strlen(asctime(&mTime))-1, false));
    }
    return MP_OBJ_FROM_PTR(resLs);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_SDIO_Ls_obj, eot_SDIO_Ls);

/**
 * Try to open a file. 
 * \note Python syntax: FileIO = SDIO.FileOpen(path, mode, enableEncryption), Where SDIO is the import name of this module.
 * \note Python example: myFile = SDIO.FileOpen("/mnt/sdcard/lena.png", "r+", 0)
 * @param filename:string It represents the file with the given name (absolute path)
 * @param mode:string It supports read ("r") and write ("w") access. For more details @see [fopen](http://www.cplusplus.com/reference/cstdio/fopen/)
 * @param enableEncryption:int encrypt and decrypt automatically if 1
 * @return **FileIO** If the file is successfully opened, the function returns a the object. Otherwise, a null pointer is returned.
 */
static mp_obj_t eot_SDIO_FileOpen (mp_obj_t path, mp_obj_t mode, mp_obj_t enableEncryption) {
    eot_FileIO_obj_t* o = m_new_obj(eot_FileIO_obj_t);
    o->base.type = &eot_FileIO_type;
    o->_cobj = SDCardFileOpen(mp_obj_str_get_str(path), mp_obj_str_get_str(mode), mp_obj_get_int(enableEncryption));
    
    return (mp_obj_t) o;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_SDIO_FileOpen_obj, eot_SDIO_FileOpen);
/**
 * Try to close a file. 
 * \note Python syntax: myFile.close().
 * @param myFile:FileIO File to close.
 * @return **int** If the file is successfully closed, the function returns 1. Otherwise, 0 is returned.
 */
static mp_obj_t eot_FileIO_FileClose(mp_obj_t self_in) {
    eot_FileIO_obj_t *self = self_in;
    return (mp_obj_t) SDCardFileClose (&(self->_cobj));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_FileIO_FileClose_obj, eot_FileIO_FileClose);

/**
 * Reads at most 'readBufferSize' bytes from the file,and returns a tuple with the number of bytes read and a string with the string read.
 * If an error occurs, such as when attempting to read from a file opened in WriteOnly mode, this function returns 0.
 * \note Python syntax: myFile.FileRead (readBufferSize)
 * \note Python example: myFile.FileRead (10)
 * @param myFile:FileIO File object
 * @param readBufferSize:int number of bytes
 * @return Tuple [int, string] [sizeInBytesRead, bufferRead]
 */
static mp_obj_t eot_FileIO_FileRead(mp_obj_t self_in, mp_obj_t readBufferSize) {
    eot_FileIO_obj_t *self = self_in;
    int _readBufferSize = mp_obj_get_int(readBufferSize);
    if (_readBufferSize == 0) return 0;
    char readBuffer[_readBufferSize];
    int sizeInBytes = SDCardFileRead (self->_cobj, readBuffer, _readBufferSize);
    mp_obj_str_t *o = MP_OBJ_TO_PTR(mp_obj_new_str(readBuffer, sizeInBytes, false));

    mp_obj_t result_tuple[2];

    result_tuple[0] = mp_obj_new_int(sizeInBytes);
    result_tuple[1] = o;

    return mp_obj_new_tuple(2, result_tuple);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(eot_FileIO_FileRead_obj, eot_FileIO_FileRead);
/**
 * Reads at most 'readBufferSize' bytes from the file, without side effects (i.e., if you call
 * FileRead() after FilePeek(), you will get the same data). 
 * \note Python syntax: myFile.FilePeek (readBufferSize)
 * \note Python example: myFile.FilePeek (10)
 * @param myFile:FileIO File object
 * @param readBufferSize:int number of bytes
 * @return Tuple [int, string] [sizeInBytesRead, bufferRead] Returns the number of bytes read and the string read. If an error occurs, such as when attempting to peek a file opened in WriteOnly mode, this function returns 0.
 */
static mp_obj_t eot_FileIO_FilePeek(mp_obj_t self_in, mp_obj_t readBufferSize) {
    eot_FileIO_obj_t *self = self_in;
    int _readBufferSize = mp_obj_get_int(readBufferSize);
    if (_readBufferSize == 0) return 0;
    char readBuffer[_readBufferSize];
    int sizeInBytes = SDCardFilePeek (self->_cobj, readBuffer, _readBufferSize);
    mp_obj_str_t *o = MP_OBJ_TO_PTR(mp_obj_new_str(readBuffer, sizeInBytes, false));
    
    mp_obj_t result_tuple[2];
    result_tuple[0] = mp_obj_new_int(sizeInBytes);
    result_tuple[1] = o;
    return mp_obj_new_tuple(2, result_tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(eot_FileIO_FilePeek_obj, eot_FileIO_FilePeek);

/**
 * Writes at most 'writeBufferSize' bytes of data from 'writeBuffer' to the file.
 * \note Python syntax: myFile.FileWrite (writeBuffer, writeBufferSize)
 * \note Python example: myFile.FileWrite ("SDCard", 4)
 * @param myFile:FileIO FileObject
 * @param writeBuffer:string string to write
 * @param writeBufferSize:int number of bytes
 * @return **int** Returns the number of bytes that were actually written, or 0 if an error occurred.
 */
static mp_obj_t eot_FileIO_FileWrite(mp_obj_t self_in, mp_obj_t writeBuffer, mp_obj_t writeBufferSize) {
    eot_FileIO_obj_t *self = self_in;
    
    mp_uint_t sep_len;
    const char* _writeBuffer = mp_obj_str_get_data(writeBuffer, &sep_len);
    int _writeBufferSize =  mp_obj_get_int(writeBufferSize);
    int _nOfBytesWritten = SDCardFileWrite (self->_cobj, _writeBuffer, _writeBufferSize);
    return mp_obj_new_int (_nOfBytesWritten);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(eot_FileIO_FileWrite_obj, eot_FileIO_FileWrite);

/**
 * Sets the current position from the FileIO to 'pos'.
 * \note Python syntax: myFile.set_position(pos).
 * \note Python example: myFile.set_position(10).
 * @param myFile:FileIO File Object
 * @param pos:int Position
 * Returns **int** 1 on success, or 0 if an error occurred.
 */
static mp_obj_t eot_FileIO_setPosition(mp_obj_t self_in, mp_obj_t pos) {
    eot_FileIO_obj_t *self = self_in;
    return mp_obj_new_int (SDCardFileSetPosition (self->_cobj, mp_obj_get_int(pos)));
    
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(eot_FileIO_setPosition_obj, eot_FileIO_setPosition);

/**
 * Get the current position from the FileIO.
 * \note Python syntax: myFile.get_position().
 * @param myFile:FileIO File Object
 * Returns **int** The current position from the FileIO
 */
static mp_obj_t eot_FileIO_getPosition(mp_obj_t self_in) {
    eot_FileIO_obj_t *self = self_in;
    return mp_obj_new_int (SDCardFileGetPostion (self->_cobj));
    
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_FileIO_getPosition_obj, eot_FileIO_getPosition);
/**
 * Flushes any buffered data to the file.
 * \note Python syntax: myFile.flush().
 * @param myFile:FileIO File Object
 * Returns true if successful; otherwise returns false.
 */
static mp_obj_t eot_FileIO_flush(mp_obj_t self_in) {
    eot_FileIO_obj_t *self = self_in;
    return mp_obj_new_int (SDCardFileFlush (self->_cobj));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(eot_FileIO_flush_obj, eot_FileIO_flush);

STATIC void eot_FileIO_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    eot_FileIO_obj_t *self = self_in;
    mp_printf(print, "File descriptor %d encryptionEnabled %d", self->_cobj->file, self->_cobj->encryptionEnabled);
}

STATIC mp_obj_t eot_FileIO_unary_op(mp_uint_t op, mp_obj_t self_in) {
   eot_FileIO_obj_t *self = self_in;
    switch (op) {
        case MP_UNARY_OP_LEN:
            return mp_obj_new_int (SDCardFileGetSize(self->_cobj));
        default:
            return MP_OBJ_NULL; // op not supported
    }
}


STATIC const mp_map_elem_t eot_FileIO_locals_dict_table[] = {
    
       { MP_OBJ_NEW_QSTR(MP_QSTR_mount), (mp_obj_t) &eot_SDIO_mount_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_isMounted), (mp_obj_t) &eot_SDIO_isMounted_obj },    //
       { MP_OBJ_NEW_QSTR(MP_QSTR_unmount), (mp_obj_t) &eot_SDIO_unmount_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_exists), (mp_obj_t) &eot_SDIO_exists_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_FileRemove), (mp_obj_t) &eot_SDIO_FileRemove_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_FileOpen), (mp_obj_t) &eot_SDIO_FileOpen_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_Ls), (mp_obj_t) &eot_SDIO_Ls_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_FileClose), (mp_obj_t) &eot_FileIO_FileClose_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_FileRead), (mp_obj_t) &eot_FileIO_FileRead_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_FilePeek), (mp_obj_t) &eot_FileIO_FilePeek_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_FileWrite), (mp_obj_t) &eot_FileIO_FileWrite_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_setPosition), (mp_obj_t) &eot_FileIO_setPosition_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_getPosition), (mp_obj_t) &eot_FileIO_getPosition_obj },
       { MP_OBJ_NEW_QSTR(MP_QSTR_flush), (mp_obj_t) &eot_FileIO_flush_obj },
       

};



STATIC MP_DEFINE_CONST_DICT(eot_FileIO_locals_dict, eot_FileIO_locals_dict_table);


const mp_obj_type_t eot_FileIO_type = {
    { &mp_type_type },
    .name = MP_QSTR_FileIO,
    .print = eot_FileIO_obj_print, // Set the funciton to print the object
    .locals_dict = (mp_obj_t) &eot_FileIO_locals_dict,
    .unary_op = eot_FileIO_unary_op,
};
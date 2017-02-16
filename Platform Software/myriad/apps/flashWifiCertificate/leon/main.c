/// =====================================================================================
///
///        @file:      main.c
///        @brief:     
/// =====================================================================================
///

/// System Includes
/// -------------------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DrvDdr.h"
#include "assert.h"
#include "DrvShaveL2Cache.h"
#include "OsDrvTimer.h"
#include "OsDrvCpr.h"
#include "DrvCDCEL.h"
#include "DrvGpioDefines.h"

#include <rtems.h>
#include "rtems_config.h"

#include "WifiFunctions.h"
#include <SDCardIO.h>


void POSIX_Init(void *args) {

    generateAPFromProfileOnErrorDefault(0);
    
    SDCardMount();
    SDCardFile* fh = SDCardFileOpen("/mnt/sdcard/google.der", "r", false);
    int file_size = SDCardFileGetSize(fh);
    char* filecontent = (char*)malloc(file_size*sizeof(char));
    SDCardFileRead(fh, filecontent, file_size);
    SDCardFileClose(&fh);
    SDCardUnmount();
    
    printf(" Certificate read \n\r");
    
    storeFileFlash("/cert/google.der", filecontent, file_size);
    
    printf(" Device successfully wrote/verified the data to/on serial-flash \n\r");
   
    exit(0);
}















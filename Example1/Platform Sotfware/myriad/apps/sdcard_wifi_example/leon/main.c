#include "app_config.h"
#include "rtems_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <mv_types.h>
#include <stdint.h>
#include <rtems.h>

#include <SDCardIO.h>
#include <WifiFunctions.h>

#define BUFFER_SIZE (1024*1024*5)
/* Read/Write buffers */
static u8 writeBuffer[BUFFER_SIZE];
static u8 readBuffer[BUFFER_SIZE];

void POSIX_Init(void *args)
{
    initClocksAndMemory();

    generateAP("prueba_dani", "123456789", 2, 7);
    printf("Own IP\n");
    printPrettyIPv4_u32(getOwnIP());
    waitClients();
    
    int i = 0;
    bool boolean = false;
    u32 sizeInBytes = 0;
    const char file[] = "/mnt/sdcard/myfile";

	for (i = 0; i < BUFFER_SIZE; i++) {
		// This buffer should contain data
		writeBuffer[i] = i+1;
		// This buffer should be empty
		readBuffer[i] = 0;
	}
			
	// Try to create the file if does not exist
	printf("\nCreating file %s\n", file);
	SDCardFile* fileHandler = SDCardFileOpen( file, "w", false );
	assert( fileHandler );	
		
	printf("\nWriting %d bytes to file\n", BUFFER_SIZE);
	sizeInBytes = SDCardFileWrite( fileHandler, writeBuffer, BUFFER_SIZE );
	assert(sizeInBytes);
		
	printf("\nPerform fsync\n");
	boolean = SDCardFileFlush( fileHandler );
	assert( (boolean == true) );
	
	printf("\nClosing file\n");
	boolean = SDCardFileClose( &fileHandler );
	assert( (boolean == true) );
	
	// Validate written data
	printf("\nOpening file %s \n", file);
	fileHandler = SDCardFileOpen( file, "r", false );
	assert( fileHandler );
	
	printf("\nRead %d characters\n", BUFFER_SIZE);
	sizeInBytes = SDCardFileRead( fileHandler, readBuffer, BUFFER_SIZE);
	assert( sizeInBytes );
	
	boolean = SDCardFileClose( &fileHandler );
	// Check Values of read and written data
	printf("\nVerifying data...\n");
			
	assert(memcmp(readBuffer, writeBuffer, BUFFER_SIZE) == 0);

	if( SDCardIsMounted() == false ) {
		printf("\nCard successfully unmounted\n\n");
	} else {
		printf("\nError unmounting card\n");
	}

	connectToAP("visilab", "123456789", SL_SEC_TYPE_WPA_WPA2, 20);

    printf("Own IP\n");
    printPrettyIPv4_u32(getOwnIP());
    
    exit(0);
}

// User extension to be able to catch abnormal terminations
void Fatal_extension(
  Internal_errors_Source  the_source,
  bool                    is_internal,
  uint32_t                the_error
)
{
    switch(the_source) {
    case RTEMS_FATAL_SOURCE_EXIT:
		if(the_error)
			printk("Exited with error code %d\n", the_error);
        break; // normal exit
    case RTEMS_FATAL_SOURCE_ASSERT:
        printk("%s : %d in %s \n",
               ((rtems_assert_context *)the_error)->file,
               ((rtems_assert_context *)the_error)->line,
               ((rtems_assert_context *)the_error)->function);
        break;
    case RTEMS_FATAL_SOURCE_EXCEPTION:
        rtems_exception_frame_print((const rtems_exception_frame *) the_error);
        break;
    default:
		printk ("\nSource %d Internal %d Error %d  0x%X:\n",
				the_source, is_internal, the_error, the_error);
        break;
    }
}

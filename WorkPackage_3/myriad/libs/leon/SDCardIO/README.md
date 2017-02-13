# SDCardIO README
### Note
* RTEMS needs additional macro definitions for supporting FAT32:
Replace in file *rtems_config.h* the line
```
#include <rtems/confdefs.h>
```
with
```
#include <SDCardIORTEMSConfig.h>
#include <rtems/confdefs.h>
```
* The PLL (Phase-locked loop) frequency should be >= 100000 to work properly with the SD card.
```
BSP_SET_CLOCK( DEFAULT_REFCLOCK, 100000, 1, 1,
               DEFAULT_RTEMS_CSS_LOS_CLOCKS, DEFAULT_RTEMS_MSS_LRT_CLOCKS,
               0, 0, 0 );
```

### Default operations

File shredding when removing a file and file system encryption are done by default. The following two parameters should be modified to 0 in SDCardIO.c if necessary:

```
#define FILE_SHREDDING_DEFAULT 1 //file shreding when removing a file
#define ENCRYPTED_OPERATION_DEFAULT 1 //all operations are encrypted
```
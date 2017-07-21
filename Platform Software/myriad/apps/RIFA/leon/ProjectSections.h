#ifndef __PROJECT_SECTIONS_H
#define __PROJECT_SECTIONS_H

#define CMX_TEXT         __attribute__((section(".cmx.text")))
#define DDR_TEXT         __attribute__((section(".ddr.data")))
#define CMX_MYDATA       __attribute__((section(".cmx.mydata")))
#define DDR_DATA         __attribute__((section(".ddr.data")))



#endif

#ifndef FLASH_IO_UNIT_TEST_H
#define FLASH_IO_UNIT_TEST_H

#include <embUnit/embUnit.h>

TestRef FlashFileOpen_test(void);
TestRef FlashFileWrite_test(void);
TestRef FlashFileRead_test(void);
TestRef FlashFileMisc_test(void);
TestRef FlashFileUseCase_test(void);

#endif /*FLASH_IO_UNIT_TEST_H*/

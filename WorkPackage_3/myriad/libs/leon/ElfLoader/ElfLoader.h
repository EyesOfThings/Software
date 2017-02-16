/*!
    @file
 
    @brief     Function library to load an elf binary from the flash into the memory
*/
#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_

#include <mv_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Load an elf binary from the flash into the memory.
 * @param[in] elfBinaryName holds the name of the binary
 * @return If loading is completed, the function returns the entry point address.
 * Otherwise, a zero value is returned. 
 */
u32 LoadElf( const char* elfBinaryName );

#ifdef __cplusplus
}
#endif

#endif /* _ELF_HEADER_H_ */

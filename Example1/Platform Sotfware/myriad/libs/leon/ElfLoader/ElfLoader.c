#include "ElfLoader.h"

#include <stdlib.h>
#include <string.h>

#include <FlashIO.h>

#include "elf.h"

#define BUFFER_SIZE 512
static u8 buffer[BUFFER_SIZE];

u32 LoadElf( const char* elfBinaryName ) {
    if( NULL == elfBinaryName || 0 == strlen(elfBinaryName) ) {
        return 0;
    }
    FlashFile* elfHandler = FlashFileOpen( elfBinaryName, ReadOnly );
    if( NULL == elfHandler ) {
        return 0;
    }
    // holds the eld header data
    Elf32_Ehdr elfHeader;
    memset( &elfHeader, 0, sizeof( Elf32_Ehdr ) );
    FlashFileRead( elfHandler, (u8*)&elfHeader, sizeof( Elf32_Ehdr ) );

    Elf32_Phdr programHeaderTableEntry;

    for(int i=0; i < elfHeader.e_phnum; ++i ) {
        // set programHeaderTableEntry to default value
        memset( &programHeaderTableEntry, 0, sizeof( Elf32_Phdr ) );
        // fill programHeaderTableEntry with the next program table entry
        FlashFileSetPosition( elfHandler, elfHeader.e_phoff + (i*elfHeader.e_phentsize) );
        FlashFileRead( elfHandler, (u8*)&programHeaderTableEntry, elfHeader.e_phentsize );
        // map elt data to memory
        u32 fileSize = programHeaderTableEntry.p_filesz;
        FlashFileSetPosition( elfHandler, programHeaderTableEntry.p_offset );
        while( 0 < fileSize ) {
            int chunkSize = ( fileSize < BUFFER_SIZE ) ? fileSize : BUFFER_SIZE;
            // reset buffer to default value
            memset( buffer, 0, BUFFER_SIZE );
            FlashFileRead( elfHandler, buffer, chunkSize );
            // copy elf data to memory
            // to the non-cacheable memory: CMX
            if( programHeaderTableEntry.p_paddr >= 0x70000000 &&
                programHeaderTableEntry.p_paddr < 0x78000000 ) {
                u32 nonCacheableAddr = programHeaderTableEntry.p_paddr + 0x8000000;
                memcpy( ((void*)nonCacheableAddr), buffer, chunkSize );
            }
            // to the non-cacheable memory: DDR
            if( programHeaderTableEntry.p_paddr >= 0x80000000 &&
                programHeaderTableEntry.p_paddr < 0xC0000000 ) {
                u32 nonCacheableAddr = programHeaderTableEntry.p_paddr + 0x40000000;
                memcpy( ((void*)nonCacheableAddr), buffer, chunkSize );
            }
            // to the cacheable memory
            memcpy( ((void*)programHeaderTableEntry.p_paddr), buffer, chunkSize );

            // update address position
            programHeaderTableEntry.p_paddr += chunkSize;
            fileSize -= chunkSize;
        }
        u32 fillWithNull = programHeaderTableEntry.p_memsz - programHeaderTableEntry.p_filesz;
        if( 0 != fillWithNull ) {
            // fill memory with zeros
            // to the non-cacheable memory: CMX
            if( programHeaderTableEntry.p_paddr >= 0x70000000 &&
                programHeaderTableEntry.p_paddr < 0x78000000 ) {
                u32 nonCacheableAddr = programHeaderTableEntry.p_paddr + 0x8000000;
                memset( ((void*)nonCacheableAddr), 0, fillWithNull );
            }
            // to the non-cacheable memory: DDR
            if( programHeaderTableEntry.p_paddr >= 0x80000000 &&
                programHeaderTableEntry.p_paddr < 0xC0000000 ) {
                u32 nonCacheableAddr = programHeaderTableEntry.p_paddr + 0x40000000;
                memset( ((void*)nonCacheableAddr), 0, fillWithNull );
            }
            // to the cacheable memory
            memset( ((void*)programHeaderTableEntry.p_paddr), 0, fillWithNull );
        }
    }

    // close file handler
    FlashFileClose( &elfHandler );

    return elfHeader.e_entry;
}

#include "Crypto.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#ifdef __RTEMS__
#include <pthread.h>
#endif

#include <FlashIO.h>

#include "AES/aes.h"

#include <stdio.h>

#define KEY_LENGTH 16/*16 Bytes or 128 bits*/
#define BLOCK_SIZE KEY_LENGTH/*16 Bytes or 128 bits*/
#define HALF_BLOCK_SIZE 8/*8 Bytes or 64 bits*/

#ifdef __RTEMS__
static pthread_mutex_t cryptoMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static u8 key[KEY_LENGTH] = { 0x00 };
static const u8 INITIALIZATION_VECTOR[KEY_LENGTH] = { 0xA3, 0x69, 0x96, 0x34,
                                                      0xBE, 0xA9, 0x02, 0xC4,
                                                      0x55, 0x9F, 0x0F, 0x39,
                                                      0x95, 0x5f, 0x61, 0x4A };
// holds a flag if the key is initialized
static bool isKeyInitialized = false;
// holds true if a custom nonce generator is defined; otherwise false
static bool isCustomNonceGeneratorInitialized = false;
// holds the custom nonce generator (function pointer)
static nonceGeneratorFunction nonceGenerator;
// holds the state for the nonce generater. Note: is not thread safe
static u32 nonceState = 0;

//
// declare functions
//
static const u8* CryptoGetKey();
static void CryptoInitNonce( u32 blockIdx );
static void CryptoGetNonce( u8 nonce[BLOCK_SIZE] );
static void CryptoNextNonce( u8 nonce[BLOCK_SIZE] );

#ifdef __RTEMS__
u32 CryptoFileWrite( FILE* fileHandler, const u8* writeBuffer,
                     const u32 writeBufferSize ) {
    if( NULL == fileHandler || NULL == writeBuffer || 0 == writeBufferSize ) {
        return 0;
    }
    u32 sizeInByte = 0;
    long int filePos = ftell( fileHandler );
    if( -1L == filePos ) {
        return 0;
    }
    u8* encryptedBuffer = malloc( writeBufferSize );
    CryptoEncrypt( writeBuffer, writeBufferSize, filePos, encryptedBuffer );
    sizeInByte = fwrite( encryptedBuffer, sizeof(u8), writeBufferSize,
                         fileHandler );
    free( encryptedBuffer );
    return sizeInByte;
}
u32 CryptoFileRead( FILE* fileHandler, u8* readBuffer,
                    const u32 readBufferSize ) {
    if( NULL == fileHandler || NULL == readBuffer || 0 == readBufferSize ) {
        return 0;
    }
    u32 sizeInByte = 0;
    long int filePos = ftell( fileHandler );
    if( -1L == filePos ) {
        return 0;
    }
    sizeInByte = fread( readBuffer, sizeof(u8), readBufferSize, fileHandler );
    CryptoDecrypt( readBuffer, sizeInByte, filePos, readBuffer );
    return sizeInByte;
}
#endif

void CryptoSetKey( u8 key128[16] ) {
    memcpy( key, key128, 16 );
    isKeyInitialized = true;
}
void CryptoSetNonceGenerator( nonceGeneratorFunction generator) {
    nonceGenerator = generator;
    isCustomNonceGeneratorInitialized = true;
}

u32 CryptoEncrypt( const u8* unencryptedBuffer, const u32 unencryptedBufferSize,
                   const u32 offset, u8* encryptedBuffer ) {
    if( NULL == unencryptedBuffer || 0 == unencryptedBufferSize
        || NULL == encryptedBuffer ) {
        return 0;
    }
    u32 blockIndex = offset / BLOCK_SIZE;
    u8 blockOffset = offset % BLOCK_SIZE;
    u8 nonce[BLOCK_SIZE];
#ifdef __RTEMS__
    // lock
    pthread_mutex_lock( &cryptoMutex );
#endif
    CryptoInitNonce( blockIndex );
    CryptoGetNonce( nonce );
    aes_encrypt_ctx encrypt;
    aes_encrypt_key128( CryptoGetKey(), &encrypt );
    encrypt.inf.b[2] = blockOffset;
    aes_ctr_encrypt( unencryptedBuffer, encryptedBuffer, unencryptedBufferSize,
                     nonce, CryptoNextNonce, &encrypt );
#ifdef __RTEMS__
    // unlock
    pthread_mutex_unlock( &cryptoMutex );
#endif
    return unencryptedBufferSize;
}

u32 CryptoDecrypt( const u8* encryptedBuffer, const u32 encryptedBufferSize,
                   const u32 offset, u8* decryptedBuffer ) {
    return CryptoEncrypt( encryptedBuffer,
                          encryptedBufferSize,
                          offset,
                          decryptedBuffer );
}

static const u8* CryptoGetKey() {
    if( false == isKeyInitialized ) {
        const u8* flashMemoryID = FlashFileGetDeviceID();
        key[0] = flashMemoryID[0];
        key[1] = (flashMemoryID[1] ^ flashMemoryID[2])
                ^ ((flashMemoryID[3] ^ flashMemoryID[4]) ^ flashMemoryID[5]);
        memcpy( &key[2], &flashMemoryID[6], 14 );
        isKeyInitialized = true;
    }
    return key;
}

static void CryptoInitNonce( u32 blockIdx ) {
    nonceState = blockIdx;
}
static void CryptoGetNonce( u8 nonce[BLOCK_SIZE] ) {
    if( true == isCustomNonceGeneratorInitialized ) {
        u8* newNonce = nonceGenerator( nonceState );
        memcpy( &nonce[0],newNonce, BLOCK_SIZE );
    }
    else {
        memcpy( &nonce[0], INITIALIZATION_VECTOR, HALF_BLOCK_SIZE );
        unsigned int seed1 = nonceState * 2;
        unsigned int seed2 = nonceState * 2 + 1;
        srand( seed1 );
        int random1 = rand();
        srand( seed2 );
        int random2 = rand();
        memcpy( &nonce[HALF_BLOCK_SIZE], &random1, sizeof(int) );
        memcpy( &nonce[HALF_BLOCK_SIZE + 4], &random2, sizeof(int) );
    }
}
static void CryptoNextNonce( u8 nonce[BLOCK_SIZE] ) {
    ++nonceState;
    CryptoGetNonce( nonce );
}

/*!
    @file
 
    @brief     Management library for File System encryption
*/
#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#ifdef __RTEMS__
#include <stdio.h>
#endif
#include <mv_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __RTEMS__
    /**
     * AES 128bit Stream Cipher Encryption (CTR)
     *
     * Encrypt the 'writeBuffer' data and writes to a file.
     * Returns the number of bytes that were actually written, or 0 if an error occurred.
     */
    u32 CryptoFileWrite( FILE* fileHandler,
                         const u8* writeBuffer,
                         const u32 writeBufferSize );
    /**
     * AES 128 bit Stream Cipher Decryption (CTR)
     *
     * Reads at most 'readBufferSize' bytes from a file and put the decrypted data into 'readBuffer', and returns the number of bytes read.
     * If an error occurs, such as when attempting to read from a file opened in write only mode, this function returns 0.
     */
    u32 CryptoFileRead( FILE* fileHandler,
                        u8* readBuffer,
                        const u32 readBufferSize );
#endif
    /**
     * Set a specific 128 bit key. (optional)
     */
    void CryptoSetKey( u8 key128[16] );
    /**
     * Define function pointer type
     */
    typedef u8* (*nonceGeneratorFunction)(u32 state);
    /**
     * Set a specific nonce generator. (optional)
     */
    void CryptoSetNonceGenerator( nonceGeneratorFunction generator);
    /**
     * AES 128 bit Stream Cipher Encryption (CTR)
     *
     * @param[in]  unencryptedBuffer A pointer to an unencrypted buffer
     * @param[in]  unencryptedBufferSize The number of bytes
     * @param[in]  offset Number of bytes to offset from origin (default: 0)
     * @param[out] encryptedBuffer A pointer to an encrypted buffer
     * @return If encryption is completed, the function returns the number of bytes.
     * Otherwise, a zero value is returned.
     */
    u32 CryptoEncrypt( const u8* unencryptedBuffer,
                       const u32 unencryptedBufferSize,
                       const u32 offset,
                       u8* encryptedBuffer );
    /**
     * AES 128 bit Stream Cipher Decryption (CTR)
     *
     * @param[in]  encryptedBuffer A pointer to an encrypted buffer
     * @param[in]  encryptedBufferSize The number of bytes
     * @param[in]  offset Number of bytes to offset from origin (default: 0)
     * @param[out] decryptedBuffer A pointer to a decrypted buffer
     * @return If decryption is completed, the function returns the number of bytes.
     * Otherwise, a zero value is returned.
     */
    u32 CryptoDecrypt( const u8* encryptedBuffer,
                       const u32 encryptedBufferSize,
                       const u32 offset,
                       u8* decryptedBuffer );
#ifdef __cplusplus
}
#endif

#endif /* _CRYPTO_H_ */

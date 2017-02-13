/*
---------------------------------------------------------------------------
Copyright (c) 1998-2013, Brian Gladman, Worcester, UK. All rights reserved.

The redistribution and use of this software (with or without changes)
is allowed without the payment of fees or royalties provided that:

  source code distributions include the above copyright notice, this
  list of conditions and the following disclaimer;

  binary distributions include the above copyright notice, this list
  of conditions and the following disclaimer in their documentation.

This software is provided 'as is' with no explicit or implied warranties
in respect of its operation, including, but not limited to, correctness
and fitness for purpose.
---------------------------------------------------------------------------
Issue Date: 20/12/2007

 These subroutines implement multiple block AES modes for ECB, CBC, CFB,
 OFB and CTR encryption,  The code provides support for the VIA Advanced
 Cryptography Engine (ACE).

 NOTE: In the following subroutines, the AES contexts (ctx) must be
 16 byte aligned if VIA ACE is being used
*/

#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "aesopt.h"

#if defined( AES_MODES )
#if defined(__cplusplus)
extern "C"
{
#endif


#define BFR_BLOCKS      8

/* These values are used to detect long word alignment in order to */
/* speed up some buffer operations. This facility may not work on  */
/* some machines so this define can be commented out if necessary  */

//#define FAST_BUFFER_OPERATIONS

#define lp32(x)         ((uint32_t*)(x))


#define aligned_array(type, name, no, stride) type name[no]
#define aligned_auto(type, name, no, stride)  type name[no]


#define via_cwd(cwd, ty, dir, len)              \
    aligned_auto(unsigned long, cwd, 4, 16);    \
    cwd[1] = cwd[2] = cwd[3] = 0;               \
    cwd[0] = neh_##dir##_##ty##_key(len)


/* test the code for detecting and setting pointer alignment */

AES_RETURN aes_test_alignment_detection(unsigned int n)	/* 4 <= n <= 16 */
{	uint8_t	p[16];
    uint32_t i, count_eq = 0, count_neq = 0;

    if(n < 4 || n > 16) {
        return EXIT_FAILURE;
    }

    for(i = 0; i < n; ++i)
    {
        uint8_t *qf = ALIGN_FLOOR(p + i, n),
                *qh =  ALIGN_CEIL(p + i, n);
        
        if(qh == qf) {
            ++count_eq;
        } else if(qh == qf + n) {
            ++count_neq;
        } else {
            return EXIT_FAILURE;
        }
    }
    return (count_eq != 1 || count_neq != n - 1 ? EXIT_FAILURE : EXIT_SUCCESS);
}

AES_RETURN aes_mode_reset(aes_encrypt_ctx ctx[1])
{
    ctx->inf.b[2] = 0;
    return EXIT_SUCCESS;
}

AES_RETURN aes_ecb_encrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, const aes_encrypt_ctx ctx[1])
{   int nb = len >> 4;

    if(len & (AES_BLOCK_SIZE - 1)) {
        return EXIT_FAILURE;
    }


    while(nb--)
    {
        if(aes_encrypt(ibuf, obuf, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;
        ibuf += AES_BLOCK_SIZE;
        obuf += AES_BLOCK_SIZE;
    }
    return EXIT_SUCCESS;
}

AES_RETURN aes_ecb_decrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, const aes_decrypt_ctx ctx[1])
{   int nb = len >> 4;

    if(len & (AES_BLOCK_SIZE - 1)) {
        return EXIT_FAILURE;
    }



    while(nb--)
    {
        if(aes_decrypt(ibuf, obuf, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;
        ibuf += AES_BLOCK_SIZE;
        obuf += AES_BLOCK_SIZE;
    }
    return EXIT_SUCCESS;
}

AES_RETURN aes_cbc_encrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, const aes_encrypt_ctx ctx[1])
{   int nb = len >> 4;

    if(len & (AES_BLOCK_SIZE - 1)) {
        return EXIT_FAILURE;
    }


        while(nb--)
        {
            iv[ 0] ^= ibuf[ 0]; iv[ 1] ^= ibuf[ 1];
            iv[ 2] ^= ibuf[ 2]; iv[ 3] ^= ibuf[ 3];
            iv[ 4] ^= ibuf[ 4]; iv[ 5] ^= ibuf[ 5];
            iv[ 6] ^= ibuf[ 6]; iv[ 7] ^= ibuf[ 7];
            iv[ 8] ^= ibuf[ 8]; iv[ 9] ^= ibuf[ 9];
            iv[10] ^= ibuf[10]; iv[11] ^= ibuf[11];
            iv[12] ^= ibuf[12]; iv[13] ^= ibuf[13];
            iv[14] ^= ibuf[14]; iv[15] ^= ibuf[15];
            if(aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS)
                return EXIT_FAILURE;
            memcpy(obuf, iv, AES_BLOCK_SIZE);
            ibuf += AES_BLOCK_SIZE;
            obuf += AES_BLOCK_SIZE;
        }
    return EXIT_SUCCESS;
}

AES_RETURN aes_cbc_decrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, const aes_decrypt_ctx ctx[1])
{   unsigned char tmp[AES_BLOCK_SIZE];
    int nb = len >> 4;

    if(len & (AES_BLOCK_SIZE - 1)) {
        return EXIT_FAILURE;
    }

        while(nb--)
        {
            memcpy(tmp, ibuf, AES_BLOCK_SIZE);
            if(aes_decrypt(ibuf, obuf, ctx) != EXIT_SUCCESS)
                return EXIT_FAILURE;
            obuf[ 0] ^= iv[ 0]; obuf[ 1] ^= iv[ 1];
            obuf[ 2] ^= iv[ 2]; obuf[ 3] ^= iv[ 3];
            obuf[ 4] ^= iv[ 4]; obuf[ 5] ^= iv[ 5];
            obuf[ 6] ^= iv[ 6]; obuf[ 7] ^= iv[ 7];
            obuf[ 8] ^= iv[ 8]; obuf[ 9] ^= iv[ 9];
            obuf[10] ^= iv[10]; obuf[11] ^= iv[11];
            obuf[12] ^= iv[12]; obuf[13] ^= iv[13];
            obuf[14] ^= iv[14]; obuf[15] ^= iv[15];
            memcpy(iv, tmp, AES_BLOCK_SIZE);
            ibuf += AES_BLOCK_SIZE;
            obuf += AES_BLOCK_SIZE;
        }

    return EXIT_SUCCESS;
}

AES_RETURN aes_cfb_encrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, aes_encrypt_ctx ctx[1])
{   int cnt = 0, b_pos = (int)ctx->inf.b[2], nb;

    if(b_pos)           /* complete any partial block   */
    {
        while(b_pos < AES_BLOCK_SIZE && cnt < len)
        {
            *obuf++ = (iv[b_pos++] ^= *ibuf++);
            cnt++;
        }

        b_pos = (b_pos == AES_BLOCK_SIZE ? 0 : b_pos);
    }

    if((nb = (len - cnt) >> 4) != 0)    /* process whole blocks */
    {
            while(cnt + AES_BLOCK_SIZE <= len)
            {
                assert(b_pos == 0);
                if(aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS) {
                    return EXIT_FAILURE;
                }
                obuf[ 0] = iv[ 0] ^= ibuf[ 0]; obuf[ 1] = iv[ 1] ^= ibuf[ 1];
                obuf[ 2] = iv[ 2] ^= ibuf[ 2]; obuf[ 3] = iv[ 3] ^= ibuf[ 3];
                obuf[ 4] = iv[ 4] ^= ibuf[ 4]; obuf[ 5] = iv[ 5] ^= ibuf[ 5];
                obuf[ 6] = iv[ 6] ^= ibuf[ 6]; obuf[ 7] = iv[ 7] ^= ibuf[ 7];
                obuf[ 8] = iv[ 8] ^= ibuf[ 8]; obuf[ 9] = iv[ 9] ^= ibuf[ 9];
                obuf[10] = iv[10] ^= ibuf[10]; obuf[11] = iv[11] ^= ibuf[11];
                obuf[12] = iv[12] ^= ibuf[12]; obuf[13] = iv[13] ^= ibuf[13];
                obuf[14] = iv[14] ^= ibuf[14]; obuf[15] = iv[15] ^= ibuf[15];
                ibuf += AES_BLOCK_SIZE;
                obuf += AES_BLOCK_SIZE;
                cnt  += AES_BLOCK_SIZE;
            }
    }

    while(cnt < len)
    {
        if(!b_pos && aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        while(cnt < len && b_pos < AES_BLOCK_SIZE)
        {
            *obuf++ = (iv[b_pos++] ^= *ibuf++);
            cnt++;
        }

        b_pos = (b_pos == AES_BLOCK_SIZE ? 0 : b_pos);
    }

    ctx->inf.b[2] = (uint8_t)b_pos;
    return EXIT_SUCCESS;
}

AES_RETURN aes_cfb_decrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, aes_encrypt_ctx ctx[1])
{   int cnt = 0, b_pos = (int)ctx->inf.b[2], nb;

    if(b_pos > 0)           /* complete any partial block   */
    {   uint8_t t;

        while(b_pos < AES_BLOCK_SIZE && cnt < len)
        {
            t = *ibuf++;
            *obuf++ = t ^ iv[b_pos];
            iv[b_pos++] = t;
            cnt++;
        }

        b_pos = (b_pos == AES_BLOCK_SIZE ? 0 : b_pos);
    }

    if((nb = (len - cnt) >> 4) != 0)    /* process whole blocks */
    {
            while(cnt + AES_BLOCK_SIZE <= len)
            {   uint8_t t;

                assert(b_pos == 0);
                if(aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS) {
                    return EXIT_FAILURE;
                }
                t = ibuf[ 0], obuf[ 0] = t ^ iv[ 0], iv[ 0] = t;
                t = ibuf[ 1], obuf[ 1] = t ^ iv[ 1], iv[ 1] = t;
                t = ibuf[ 2], obuf[ 2] = t ^ iv[ 2], iv[ 2] = t;
                t = ibuf[ 3], obuf[ 3] = t ^ iv[ 3], iv[ 3] = t;
                t = ibuf[ 4], obuf[ 4] = t ^ iv[ 4], iv[ 4] = t;
                t = ibuf[ 5], obuf[ 5] = t ^ iv[ 5], iv[ 5] = t;
                t = ibuf[ 6], obuf[ 6] = t ^ iv[ 6], iv[ 6] = t;
                t = ibuf[ 7], obuf[ 7] = t ^ iv[ 7], iv[ 7] = t;
                t = ibuf[ 8], obuf[ 8] = t ^ iv[ 8], iv[ 8] = t;
                t = ibuf[ 9], obuf[ 9] = t ^ iv[ 9], iv[ 9] = t;
                t = ibuf[10], obuf[10] = t ^ iv[10], iv[10] = t;
                t = ibuf[11], obuf[11] = t ^ iv[11], iv[11] = t;
                t = ibuf[12], obuf[12] = t ^ iv[12], iv[12] = t;
                t = ibuf[13], obuf[13] = t ^ iv[13], iv[13] = t;
                t = ibuf[14], obuf[14] = t ^ iv[14], iv[14] = t;
                t = ibuf[15], obuf[15] = t ^ iv[15], iv[15] = t;
                ibuf += AES_BLOCK_SIZE;
                obuf += AES_BLOCK_SIZE;
                cnt  += AES_BLOCK_SIZE;
            }
    }

    while(cnt < len)
    {   uint8_t t;

        if(!b_pos && aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        while(cnt < len && b_pos < AES_BLOCK_SIZE)
        {
            t = *ibuf++;
            *obuf++ = t ^ iv[b_pos];
            iv[b_pos++] = t;
            cnt++;
        }

        b_pos = (b_pos == AES_BLOCK_SIZE ? 0 : b_pos);
    }

    ctx->inf.b[2] = (uint8_t)b_pos;
    return EXIT_SUCCESS;
}

AES_RETURN aes_ofb_crypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, aes_encrypt_ctx ctx[1])
{   int cnt = 0, b_pos = (int)ctx->inf.b[2], nb;

    if(b_pos > 0)           /* complete any partial block   */
    {
        while(b_pos < AES_BLOCK_SIZE && cnt < len)
        {
            *obuf++ = iv[b_pos++] ^ *ibuf++;
            cnt++;
        }

        b_pos = (b_pos == AES_BLOCK_SIZE ? 0 : b_pos);
    }

    if((nb = (len - cnt) >> 4) != 0)   /* process whole blocks */
    {
        while(cnt + AES_BLOCK_SIZE <= len)
        {
            assert(b_pos == 0);
            if(aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            obuf[ 0] = iv[ 0] ^ ibuf[ 0]; obuf[ 1] = iv[ 1] ^ ibuf[ 1];
            obuf[ 2] = iv[ 2] ^ ibuf[ 2]; obuf[ 3] = iv[ 3] ^ ibuf[ 3];
            obuf[ 4] = iv[ 4] ^ ibuf[ 4]; obuf[ 5] = iv[ 5] ^ ibuf[ 5];
            obuf[ 6] = iv[ 6] ^ ibuf[ 6]; obuf[ 7] = iv[ 7] ^ ibuf[ 7];
            obuf[ 8] = iv[ 8] ^ ibuf[ 8]; obuf[ 9] = iv[ 9] ^ ibuf[ 9];
            obuf[10] = iv[10] ^ ibuf[10]; obuf[11] = iv[11] ^ ibuf[11];
            obuf[12] = iv[12] ^ ibuf[12]; obuf[13] = iv[13] ^ ibuf[13];
            obuf[14] = iv[14] ^ ibuf[14]; obuf[15] = iv[15] ^ ibuf[15];
            ibuf += AES_BLOCK_SIZE;
            obuf += AES_BLOCK_SIZE;
            cnt  += AES_BLOCK_SIZE;
        }
    }

    while(cnt < len)
    {
        if(!b_pos && aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        while(cnt < len && b_pos < AES_BLOCK_SIZE)
        {
            *obuf++ = iv[b_pos++] ^ *ibuf++;
            cnt++;
        }

        b_pos = (b_pos == AES_BLOCK_SIZE ? 0 : b_pos);
    }

    ctx->inf.b[2] = (uint8_t)b_pos;
    return EXIT_SUCCESS;
}

#define BFR_LENGTH  (BFR_BLOCKS * AES_BLOCK_SIZE)

AES_RETURN aes_ctr_crypt(const unsigned char *ibuf, unsigned char *obuf,
            int len, unsigned char *cbuf, cbuf_inc ctr_inc, aes_encrypt_ctx ctx[1])
{
    int i;
    int blen;
    unsigned int b_pos = (int)(ctx->inf.b[2]);
    uint8_t buf[BFR_LENGTH] = {0x00};

    if(b_pos > 0)
    {
        memcpy(buf, cbuf, AES_BLOCK_SIZE);
        if(aes_ecb_encrypt(buf, buf, AES_BLOCK_SIZE, ctx) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        while(b_pos < AES_BLOCK_SIZE && len > 0)
        {
            *obuf++ = *ibuf++ ^ buf[b_pos++];
            --len;
        }
        if(b_pos >= AES_BLOCK_SIZE) {
            ctr_inc(cbuf);
            b_pos = b_pos % AES_BLOCK_SIZE;
        }
    }
    while(0 < len)
    {
        blen = (len > BFR_LENGTH ? BFR_LENGTH : len);
        len -= blen;

        for(i = 0; i < (blen >> 4); ++i)
        {
            memcpy(&(buf[i*AES_BLOCK_SIZE]), cbuf, AES_BLOCK_SIZE);
            ctr_inc(cbuf);
        }

        if(blen & (AES_BLOCK_SIZE - 1)) {
            memcpy(&(buf[i*AES_BLOCK_SIZE]), cbuf, AES_BLOCK_SIZE);
            ++i;
        }

        if(aes_ecb_encrypt(&(buf[0]), &(buf[0]), i * AES_BLOCK_SIZE, ctx) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        i = 0;
        while(i + AES_BLOCK_SIZE <= blen)
        {
            obuf[ 0] = ibuf[ 0] ^ buf[i+ 0]; obuf[ 1] = ibuf[ 1] ^ buf[i+ 1];
            obuf[ 2] = ibuf[ 2] ^ buf[i+ 2]; obuf[ 3] = ibuf[ 3] ^ buf[i+ 3];
            obuf[ 4] = ibuf[ 4] ^ buf[i+ 4]; obuf[ 5] = ibuf[ 5] ^ buf[i+ 5];
            obuf[ 6] = ibuf[ 6] ^ buf[i+ 6]; obuf[ 7] = ibuf[ 7] ^ buf[i+ 7];
            obuf[ 8] = ibuf[ 8] ^ buf[i+ 8]; obuf[ 9] = ibuf[ 9] ^ buf[i+ 9];
            obuf[10] = ibuf[10] ^ buf[i+10]; obuf[11] = ibuf[11] ^ buf[i+11];
            obuf[12] = ibuf[12] ^ buf[i+12]; obuf[13] = ibuf[13] ^ buf[i+13];
            obuf[14] = ibuf[14] ^ buf[i+14]; obuf[15] = ibuf[15] ^ buf[i+15];
            i += AES_BLOCK_SIZE;
            ibuf += AES_BLOCK_SIZE;
            obuf += AES_BLOCK_SIZE;
        }
        while(i < blen) {
            *obuf++ = *ibuf++ ^ buf[i];
            ++b_pos;
            ++i;
        }
    }

    ctx->inf.b[2] = (uint8_t)b_pos;
    return EXIT_SUCCESS;
}

#if defined(__cplusplus)
}
#endif
#endif

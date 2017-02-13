#include "CryptoUnitTest.h"

#include <stdio.h>
#include <string.h>

#include <Crypto.h>

static void setUp( void ) {
// setup
}

static void tearDown( void ) {
// tearDown
}

static u8 nonce[16] = {
    0xf0, 0xf1, 0xf2, 0xf3,
    0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xfb,
    0xfc, 0xfd, 0xfe, 0xff
};
static u8* nonceGenerator( u32 state ) {
    if( 0 == state ) {
       nonce[0] = 0xf0; nonce[1] = 0xf1; nonce[2] = 0xf2; nonce[3] = 0xf3;
       nonce[4] = 0xf4; nonce[5] = 0xf5; nonce[6] = 0xf6; nonce[7] = 0xf7;
       nonce[8] = 0xf8; nonce[9] = 0xf9; nonce[10]= 0xfa; nonce[11]= 0xfb;
       nonce[12]= 0xfc; nonce[13]= 0xfd; nonce[14]= 0xfe; nonce[15]= 0xff;
    }
    else if( 1 == state ) {
        nonce[0] = 0xf0; nonce[1] = 0xf1; nonce[2] = 0xf2; nonce[3] = 0xf3;
        nonce[4] = 0xf4; nonce[5] = 0xf5; nonce[6] = 0xf6; nonce[7] = 0xf7;
        nonce[8] = 0xf8; nonce[9] = 0xf9; nonce[10]= 0xfa; nonce[11]= 0xfb;
        nonce[12]= 0xfc; nonce[13]= 0xfd; nonce[14]= 0xff; nonce[15]= 0x00;
    }
    else if( 2 == state ) {
        nonce[0] = 0xf0; nonce[1] = 0xf1; nonce[2] = 0xf2; nonce[3] = 0xf3;
        nonce[4] = 0xf4; nonce[5] = 0xf5; nonce[6] = 0xf6; nonce[7] = 0xf7;
        nonce[8] = 0xf8; nonce[9] = 0xf9; nonce[10]= 0xfa; nonce[11]= 0xfb;
        nonce[12]= 0xfc; nonce[13]= 0xfd; nonce[14]= 0xff; nonce[15]= 0x01;
    }
    else if( 3 == state ) {
        nonce[0] = 0xf0; nonce[1] = 0xf1; nonce[2] = 0xf2; nonce[3] = 0xf3;
        nonce[4] = 0xf4; nonce[5] = 0xf5; nonce[6] = 0xf6; nonce[7] = 0xf7;
        nonce[8] = 0xf8; nonce[9] = 0xf9; nonce[10]= 0xfa; nonce[11]= 0xfb;
        nonce[12]= 0xfc; nonce[13]= 0xfd; nonce[14]= 0xff; nonce[15]= 0x02;
    }
    return nonce;
}
static u8 key[16] = {
    0x2b, 0x7e, 0x15, 0x16,
    0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88,
    0x09, 0xcf, 0x4f, 0x3c
};
static void testEncryption( void ) {
    CryptoSetKey(key);
    CryptoSetNonceGenerator(nonceGenerator);
    {
        u8 cipherText[16] = { 0 };
        u8 refCipherText[16] = {
            0x87, 0x4d, 0x61, 0x91,
            0xb6, 0x20, 0xe3, 0x26,
            0x1b, 0xef, 0x68, 0x64,
            0x99, 0x0d, 0xb6, 0xce
        };
        u8 data[16] = {
            0x6b, 0xc1, 0xbe, 0xe2,
            0x2e, 0x40, 0x9f, 0x96,
            0xe9, 0x3d, 0x7e, 0x11,
            0x73, 0x93, 0x17, 0x2a
        };
        CryptoEncrypt(data, 16, 0, cipherText);
        int i=0;
        for(i=0; i<16; ++i) {
            TEST_ASSERT_EQUAL_INT( refCipherText[i], cipherText[i] );
        }
    }
    {
        u8 cipherText[16] = { 0 };
        u8 refCipherText[16] = {
            0x98, 0x06, 0xf6, 0x6b,
            0x79, 0x70, 0xfd, 0xff,
            0x86, 0x17, 0x18, 0x7b,
            0xb9, 0xff, 0xfd, 0xff
        };
        u8 data[16] = {
            0xae, 0x2d, 0x8a, 0x57,
            0x1e, 0x03, 0xac, 0x9c,
            0x9e, 0xb7, 0x6f, 0xac,
            0x45, 0xaf, 0x8e, 0x51
        };
        CryptoEncrypt(data, 16, 16, cipherText);
        int i=0;
        for(i=0; i<16; ++i) {
            TEST_ASSERT_EQUAL_INT( refCipherText[i], cipherText[i] );
        }
    }
    {
        u8 cipherText[16] = { 0 };
        u8 refCipherText[16] = {
            0x5a, 0xe4, 0xdf, 0x3e,
            0xdb, 0xd5, 0xd3, 0x5e,
            0x5b, 0x4f, 0x09, 0x02,
            0x0d, 0xb0, 0x3e, 0xab
        };
        u8 data[16] = {
            0x30, 0xc8, 0x1c, 0x46,
            0xa3, 0x5c, 0xe4, 0x11,
            0xe5, 0xfb, 0xc1, 0x19,
            0x1a, 0x0a, 0x52, 0xef
        };
        CryptoEncrypt(data, 16, 32, cipherText);
        int i=0;
        for(i=0; i<16; ++i) {
            TEST_ASSERT_EQUAL_INT( refCipherText[i], cipherText[i] );
        }
    }
    {
        u8 cipherText[16] = { 0 };
        u8 refCipherText[16] = {
            0x1e, 0x03, 0x1d, 0xda,
            0x2f, 0xbe, 0x03, 0xd1,
            0x79, 0x21, 0x70, 0xa0,
            0xf3, 0x00, 0x9c, 0xee
        };
        u8 data[16] = {
            0xf6, 0x9f, 0x24, 0x45,
            0xdf, 0x4f, 0x9b, 0x17,
            0xad, 0x2b, 0x41, 0x7b,
            0xe6, 0x6c, 0x37, 0x10
        };
        CryptoEncrypt(data, 16, 48, cipherText);
        int i=0;
        for(i=0; i<16; ++i) {
            TEST_ASSERT_EQUAL_INT( refCipherText[i], cipherText[i] );
        }
    }
}
static void testDecryption( void ) {
    CryptoSetKey(key);
    CryptoSetNonceGenerator(nonceGenerator);
    {
        u8 cipherText[16] = { 0 };
        u8 refCipherText[16] = {
            0x6b, 0xc1, 0xbe, 0xe2,
            0x2e, 0x40, 0x9f, 0x96,
            0xe9, 0x3d, 0x7e, 0x11,
            0x73, 0x93, 0x17, 0x2a
        };
        u8 data[16] = {
            0x87, 0x4d, 0x61, 0x91,
            0xb6, 0x20, 0xe3, 0x26,
            0x1b, 0xef, 0x68, 0x64,
            0x99, 0x0d, 0xb6, 0xce
        };
        CryptoEncrypt(data, 16, 0, cipherText);
        int i=0;
        for(i=0; i<16; ++i) {
            TEST_ASSERT_EQUAL_INT( refCipherText[i], cipherText[i] );
        }
    }
    {
        u8 cipherText[16] = { 0 };
        u8 refCipherText[16] = {
            0xae, 0x2d, 0x8a, 0x57,
            0x1e, 0x03, 0xac, 0x9c,
            0x9e, 0xb7, 0x6f, 0xac,
            0x45, 0xaf, 0x8e, 0x51
        };
        u8 data[16] = {
            0x98, 0x06, 0xf6, 0x6b,
            0x79, 0x70, 0xfd, 0xff,
            0x86, 0x17, 0x18, 0x7b,
            0xb9, 0xff, 0xfd, 0xff
        };
        CryptoEncrypt(data, 16, 16, cipherText);
        int i=0;
        for(i=0; i<16; ++i) {
            TEST_ASSERT_EQUAL_INT( refCipherText[i], cipherText[i] );
        }
    }
    {
        u8 cipherText[16] = { 0 };
        u8 refCipherText[16] = {
            0x30, 0xc8, 0x1c, 0x46,
            0xa3, 0x5c, 0xe4, 0x11,
            0xe5, 0xfb, 0xc1, 0x19,
            0x1a, 0x0a, 0x52, 0xef
        };
        u8 data[16] = {
            0x5a, 0xe4, 0xdf, 0x3e,
            0xdb, 0xd5, 0xd3, 0x5e,
            0x5b, 0x4f, 0x09, 0x02,
            0x0d, 0xb0, 0x3e, 0xab
        };
        CryptoEncrypt(data, 16, 32, cipherText);
        int i=0;
        for(i=0; i<16; ++i) {
            TEST_ASSERT_EQUAL_INT( refCipherText[i], cipherText[i] );
        }
    }
    {
        u8 cipherText[16] = { 0 };
        u8 refCipherText[16] = {
            0xf6, 0x9f, 0x24, 0x45,
            0xdf, 0x4f, 0x9b, 0x17,
            0xad, 0x2b, 0x41, 0x7b,
            0xe6, 0x6c, 0x37, 0x10
        };
        u8 data[16] = {
            0x1e, 0x03, 0x1d, 0xda,
            0x2f, 0xbe, 0x03, 0xd1,
            0x79, 0x21, 0x70, 0xa0,
            0xf3, 0x00, 0x9c, 0xee
        };
        CryptoEncrypt(data, 16, 48, cipherText);
        int i=0;
        for(i=0; i<16; ++i) {
            TEST_ASSERT_EQUAL_INT( refCipherText[i], cipherText[i] );
        }
    }
}
TestRef Crypto_test( void ) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("testEncryption", testEncryption),
        new_TestFixture("testDecryption", testDecryption)
    };
    EMB_UNIT_TESTCALLER(CryptoTest, "CryptoTest", setUp, tearDown, fixtures);
    return (TestRef)&CryptoTest;
}

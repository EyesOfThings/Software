/***************************************************************************/
/*                                                                         */
/*  File: savejpg.h                                                        */
/*  Author: bkenwright@xbdev.net                                           */
/*  URL: www.xbdev.net                                                     */
/*  Date: 19-01-06                                                         */
/*                                                                         */
/***************************************************************************/
/*

Tiny Simplified C source of a JPEG encoder.


The principles of designing my JPEG encoder were mainly clarity and portability.
Kept as simple as possibly, using basic c in most places for portability.

The FDCT routine is taken with minor modifications from Independent JPEG Group's
JPEG software . So don't ask me details about that. 

The program should give the same results on any C compiler which provides at least
256 kb of free memory. I needed that for the precalculated bitcode (3*64k) and
category (64k) arrays , not to mention the memory needed for the truecolor BMP.

Since it's made to encode a truecolor BMP into a JPG file, I think that it
should be no problem for you to figure out how to modify the C source in order
to use it with any RGB image you want to compress (small enough to fit into
your memory) .

A note: The JPG format I used is a personal choice:
        The sampling factors are 1:1 for simplicity reasons, the Huffman tables
 are those given in the standard (so it's not optimized for that particular
 image you want to encode), etc.
I coded only what I needed from a JPEG encoder. Probably it could have 
more advanced features.

There are a lot of other JPG formats (markers in another order, another markers),
so don't ask me why a JPG file not encoded with this encoder has a different
format (different locations , another markers...).

 */

/***************************************************************************/

#ifndef SAVEJPG_H
#define SAVEJPG_H

#include <ccv.h>

#define MAXBUFFERJPEG 368640


#define BYTE unsigned char
#define SBYTE signed char
#define SWORD signed short int
#define WORD unsigned short int
#define DWORD unsigned long int
#define SDWORD signed long int

static struct APP0infotype {
    WORD marker; // = 0xFFE0
    WORD length; // = 16 for usual JPEG, no thumbnail
    BYTE JFIFsignature[5]; // = "JFIF",'\0'
    BYTE versionhi; // 1
    BYTE versionlo; // 1
    BYTE xyunits; // 0 = no units, normal density
    WORD xdensity; // 1
    WORD ydensity; // 1
    BYTE thumbnwidth; // 0
    BYTE thumbnheight; // 0
} APP0info = {0xFFE0, 16, 'J', 'F', 'I', 'F', 0, 1, 1, 0, 1, 1, 0, 0};

static struct SOF0infotype {
    WORD marker; // = 0xFFC0
    WORD length; // = 17 for a truecolor YCbCr JPG
    BYTE precision; // Should be 8: 8 bits/sample
    WORD height;
    WORD width;
    BYTE nrofcomponents; //Should be 3: We encode a truecolor JPG
    BYTE IdY; // = 1
    BYTE HVY; // sampling factors for Y (bit 0-3 vert., 4-7 hor.)
    BYTE QTY; // Quantization Table number for Y = 0
    BYTE IdCb; // = 2
    BYTE HVCb;
    BYTE QTCb; // 1
    BYTE IdCr; // = 3
    BYTE HVCr;
    BYTE QTCr; // Normally equal to QTCb = 1
} SOF0info = {0xFFC0, 17, 8, 0, 0, 3, 1, 0x22, 0, 2, 0x11, 1, 3, 0x11, 1};
// Default sampling factors are 1,1 for every image component: No downsampling

static struct DQTinfotype {
    WORD marker; // = 0xFFDB
    WORD length; // = 132
    BYTE QTYinfo; // = 0:  bit 0..3: number of QT = 0 (table for Y)
    //       bit 4..7: precision of QT, 0 = 8 bit
    BYTE Ytable[64];
    BYTE QTCbinfo; // = 1 (quantization table for Cb,Cr}
    BYTE Cbtable[64];
} DQTinfo;
// Ytable from DQTinfo should be equal to a scaled and zizag reordered version
// of the table which can be found in "tables.h": std_luminance_qt
// Cbtable , similar = std_chrominance_qt
// We'll init them in the program using set_DQTinfo function

static struct DHTinfotype {
    WORD marker; // = 0xFFC4
    WORD length; //0x01A2
    BYTE HTYDCinfo; // bit 0..3: number of HT (0..3), for Y =0
    //bit 4  :type of HT, 0 = DC table,1 = AC table
    //bit 5..7: not used, must be 0
    BYTE YDC_nrcodes[16]; //at index i = nr of codes with length i
    BYTE YDC_values[12];
    BYTE HTYACinfo; // = 0x10
    BYTE YAC_nrcodes[16];
    BYTE YAC_values[162]; //we'll use the standard Huffman tables
    BYTE HTCbDCinfo; // = 1
    BYTE CbDC_nrcodes[16];
    BYTE CbDC_values[12];
    BYTE HTCbACinfo; //  = 0x11
    BYTE CbAC_nrcodes[16];
    BYTE CbAC_values[162];
} DHTinfo;

static struct SOSinfotype {
    WORD marker; // = 0xFFDA
    WORD length; // = 12
    BYTE nrofcomponents; // Should be 3: truecolor JPG
    BYTE IdY; //1
    BYTE HTY; //0 // bits 0..3: AC table (0..3)
    // bits 4..7: DC table (0..3)
    BYTE IdCb; //2
    BYTE HTCb; //0x11
    BYTE IdCr; //3
    BYTE HTCr; //0x11
    BYTE Ss, Se, Bf; // not interesting, they should be 0,63,0
} SOSinfo = {0xFFDA, 12, 3, 1, 0, 2, 0x11, 3, 0x11, 0, 0x3F, 0};

typedef struct {
    BYTE Y, Cb, Cr;
} colorYCbCr;

typedef struct {
    BYTE length;
    WORD value;
} bitstring;

#define  Y(R,G,B) ((BYTE)( (YRtab[(R)]+YGtab[(G)]+YBtab[(B)])>>16 ) - 128)
#define Cb(R,G,B) ((BYTE)( (CbRtab[(R)]+CbGtab[(G)]+CbBtab[(B)])>>16 ) )
#define Cr(R,G,B) ((BYTE)( (CrRtab[(R)]+CrGtab[(G)]+CrBtab[(B)])>>16 ) )



//extern unsigned char image_to_show[368640];
//extern int image_size_on_bytes;

//#define writebyte(b) fputc((b),fp_jpeg_stream)
//#define writeword(w) writebyte((w)/256);writebyte((w)%256);
#define writebyte(b) {image_to_show[image_size_in_bytes_to_show]=b;image_size_in_bytes_to_show++; }//fputc((b),fp_jpeg_stream)
#define writeword(w) {image_to_show[image_size_in_bytes_to_show]=(w)/256;image_size_in_bytes_to_show++;image_to_show[image_size_in_bytes_to_show]=(w)%256;image_size_in_bytes_to_show++;}   //((w)/256);writebyte((w)%256);



static BYTE zigzag[64] = {0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7, 13, 16, 26, 29, 42,
    3, 8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63};

//These are the sample quantization tables given in JPEG spec section K.1.
//	The spec says that the values given produce "good" quality, and
//	when divided by 2, "very good" quality
static BYTE std_luminance_qt[64] = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
};
static BYTE std_chrominance_qt[64] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};
// Standard Huffman tables (cf. JPEG standard section K.3)

static BYTE std_dc_luminance_nrcodes[17] = {0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
static BYTE std_dc_luminance_values[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static BYTE std_dc_chrominance_nrcodes[17] = {0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
static BYTE std_dc_chrominance_values[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static BYTE std_ac_luminance_nrcodes[17] = {0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d};
static BYTE std_ac_luminance_values[162] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

static BYTE std_ac_chrominance_nrcodes[17] = {0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77};
static BYTE std_ac_chrominance_values[162] = {
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

/**
 * Converts an image in YCbCr (YUV) to JPEG. The result is stored in 
 * 'image_to_show'. Variable'image_size_in_bytes_to_show' contains the size of the
 * image buffer 'image'.
 * @param *imageBuffer Pointer to the buffer where the image is stored.
 * @param Ximage_original Width of the image.
 * @param Yimage_original Height of the image.
 */
void convert2Jpeg(colorYCbCr *imageBuffer, WORD Ximage_original, WORD Yimage_original);
void convertDenseMatrix2Jpeg(ccv_dense_matrix_t* img_src);
#endif

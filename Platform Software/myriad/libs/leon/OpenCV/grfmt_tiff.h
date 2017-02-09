/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef _GRFMT_TIFF_H_
#define _GRFMT_TIFF_H_

#include "grfmt_base.h"


// native simple TIFF codec
enum TiffCompression
{
    TIFF_UNCOMP = 1,
    TIFF_HUFFMAN = 2,
    TIFF_PACKBITS = 32773
};

enum TiffByteOrder
{
    TIFF_ORDER_II = 0x4949,
    TIFF_ORDER_MM = 0x4d4d
};


enum  TiffTag
{
    TIFF_TAG_WIDTH  = 256,
    TIFF_TAG_HEIGHT = 257,
    TIFF_TAG_BITS_PER_SAMPLE = 258,
    TIFF_TAG_COMPRESSION = 259,
    TIFF_TAG_PHOTOMETRIC = 262,
    TIFF_TAG_STRIP_OFFSETS = 273,
    TIFF_TAG_STRIP_COUNTS = 279,
    TIFF_TAG_SAMPLES_PER_PIXEL = 277,
    TIFF_TAG_ROWS_PER_STRIP = 278,
    TIFF_TAG_PLANAR_CONFIG = 284,
    TIFF_TAG_COLOR_MAP = 320
};


enum TiffFieldType
{
    TIFF_TYPE_BYTE = 1,
    TIFF_TYPE_SHORT = 3,
    TIFF_TYPE_LONG = 4
};



#ifdef HAVE_TIFF

// libtiff based TIFF codec

class GrFmtTiffReader : public GrFmtReader
{
public:
    
    GrFmtTiffReader( const char* filename );
    ~GrFmtTiffReader();

    bool  CheckFormat( const char* signature );
    bool  ReadData( uchar* data, int step, int color );
    bool  ReadHeader();
    void  Close();

protected:

    void* m_tif;
};


#else

class GrFmtTiffReader : public GrFmtReader
{
public:
    
    GrFmtTiffReader( const char* filename );
    ~GrFmtTiffReader();

    bool  CheckFormat( const char* signature );
    bool  ReadData( uchar* data, int step, int color );
    bool  ReadHeader();
    void  Close();

protected:
    
    RLByteStream     m_strm;
    PaletteEntry     m_palette[256];
    int              m_bpp;
    int*             m_temp_palette;
    int              m_max_pal_length;
    int*             m_offsets;
    int              m_maxoffsets;
    int              m_strips;
    int              m_rows_per_strip;
    TiffCompression  m_compression;
    TiffByteOrder    m_byteorder;

    int  GetWordEx();
    int  GetDWordEx();
    int  ReadTable( int offset, int count, TiffFieldType fieldtype,
                    int*& array, int& arraysize );
};

#endif

// ... and writer
class GrFmtTiffWriter : public GrFmtWriter
{
public:
    
    GrFmtTiffWriter( const char* filename );
    ~GrFmtTiffWriter();

    bool  WriteImage( const uchar* data, int step,
                      int width, int height, int depth, int channels );
protected:

    WLByteStream  m_strm;

    void  WriteTag( TiffTag tag, TiffFieldType fieldType,
                    int count, int value );
};


// TIFF filter factory
class GrFmtTiff : public GrFmtFilterFactory
{
public:
    
    GrFmtTiff();
    ~GrFmtTiff();

    GrFmtReader* NewReader( const char* filename );
    GrFmtWriter* NewWriter( const char* filename );
    bool CheckSignature( const char* signature );
};

#endif/*_GRFMT_TIFF_H_*/

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

#ifndef _GRFMT_EXR_H_
#define _GRFMT_EXR_H_

#ifdef HAVE_ILMIMF

#include <ImfChromaticities.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImathBox.h>
#include "grfmt_base.h"

using namespace Imf;
using namespace Imath;

/* libpng version only */

class GrFmtExrReader : public GrFmtReader
{
public:

    GrFmtExrReader( const char* filename );
    ~GrFmtExrReader();

    bool  ReadData( uchar* data, int step, int color );
    bool  ReadHeader();
    void  Close();

protected:
    void  UpSample( uchar *data, int xstep, int ystep, int xsample, int ysample );
    void  UpSampleX( float *data, int xstep, int xsample );
    void  UpSampleY( uchar *data, int xstep, int ystep, int ysample );
    void  ChromaToBGR( float *data, int numlines, int step );
    void  RGBToGray( float *in, float *out );

    InputFile      *m_file;
    PixelType       m_type;
    Box2i           m_datawindow;
    bool            m_ischroma;
    const Channel  *m_red;
    const Channel  *m_green;
    const Channel  *m_blue;
    Chromaticities  m_chroma;
};


class GrFmtExrWriter : public GrFmtWriter
{
public:

    GrFmtExrWriter( const char* filename );
    ~GrFmtExrWriter();

    bool  IsFormatSupported( int depth );
    bool  WriteImage( const uchar* data, int step,
                      int width, int height, int depth, int channels );
protected:
};


// Exr filter factory
class GrFmtExr : public GrFmtFilterFactory
{
public:

    GrFmtExr();
    ~GrFmtExr();

    GrFmtReader* NewReader( const char* filename );
    GrFmtWriter* NewWriter( const char* filename );
//    bool CheckSignature( const char* signature );
};

#endif

#endif/*_GRFMT_EXR_H_*/

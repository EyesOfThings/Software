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

#include "_highgui.h"

#ifdef HAVE_ILMIMF

#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfStandardAttributes.h>
#include <OpenEXR/half.h>
#include "grfmt_exr.h"

#if defined _MSC_VER && _MSC_VER >= 1200
#pragma comment(lib, "Half.lib")
#pragma comment(lib, "Iex.lib")
#pragma comment(lib, "IlmImf.lib")
#pragma comment(lib, "IlmThread.lib")
#pragma comment(lib, "Imath.lib")

#undef UINT
#define UINT ((Imf::PixelType)0)
#undef HALF
#define HALF ((Imf::PixelType)1)
#undef FLOAT
#define FLOAT ((Imf::PixelType)2)
#undef uint
#define uint unsigned

#endif

// Exr Filter Factory
GrFmtExr::GrFmtExr()
{
    m_sign_len = 4;
    m_signature = "\x76\x2f\x31\x01";
    m_description = "OpenEXR Image files (*.exr)";
}


GrFmtExr::~GrFmtExr()
{
}


GrFmtReader* GrFmtExr::NewReader( const char* filename )
{
    return new GrFmtExrReader( filename );
}


GrFmtWriter* GrFmtExr::NewWriter( const char* filename )
{
    return new GrFmtExrWriter( filename );
}


/////////////////////// GrFmtExrReader ///////////////////

GrFmtExrReader::GrFmtExrReader( const char* filename ) : GrFmtReader( filename )
{
    m_file = new InputFile( filename );
    m_red = m_green = m_blue = 0;
}


GrFmtExrReader::~GrFmtExrReader()
{
    Close();
}


void  GrFmtExrReader::Close()
{
    if( m_file )
    {
        delete m_file;
        m_file = 0;
    }

    GrFmtReader::Close();
}

bool  GrFmtExrReader::ReadHeader()
{
    bool result = false;

    if( !m_file ) // probably paranoid
        return false;

    m_datawindow = m_file->header().dataWindow();
    m_width = m_datawindow.max.x - m_datawindow.min.x + 1;
    m_height = m_datawindow.max.y - m_datawindow.min.y + 1;

    // the type HALF is converted to 32 bit float
    // and the other types supported by OpenEXR are 32 bit anyway
    m_bit_depth = 32;

    if( hasChromaticities( m_file->header() ))
        m_chroma = chromaticities( m_file->header() );

    const ChannelList &channels = m_file->header().channels();
    m_red = channels.findChannel( "R" );
    m_green = channels.findChannel( "G" );
    m_blue = channels.findChannel( "B" );
    if( m_red || m_green || m_blue )
    {
        m_iscolor = true;
        m_ischroma = false;
        result = true;
    }
    else
    {
        m_green = channels.findChannel( "Y" );
        if( m_green )
        {
            m_ischroma = true;
            m_red = channels.findChannel( "RY" );
            m_blue = channels.findChannel( "BY" );
            m_iscolor = (m_blue || m_red);
            result = true;
        }
        else
            result = false;
    }

    if( result )
    {
        int uintcnt = 0;
        int chcnt = 0;
        if( m_red )
        {
            chcnt++;
            uintcnt += ( m_red->type == UINT );
        }
        if( m_green )
        {
            chcnt++;
            uintcnt += ( m_green->type == UINT );
        }
        if( m_blue )
        {
            chcnt++;
            uintcnt += ( m_blue->type == UINT );
        }
        m_type = (chcnt == uintcnt) ? UINT : FLOAT;
    
        m_isfloat = (m_type == FLOAT);
    }

    if( !result )
        Close();

    return result;
}


bool  GrFmtExrReader::ReadData( uchar* data, int step, int color )
{
    bool justcopy = m_native_depth;
    bool chromatorgb = false;
    bool rgbtogray = false;
    bool result = true;
    FrameBuffer frame;
    int xsample[3] = {1, 1, 1};
    char *buffer;
    int xstep;
    int ystep;

    xstep = m_native_depth ? 4 : 1;

    if( !m_native_depth || (!color && m_iscolor ))
    {
        buffer = (char *)new float[ m_width * 3 ];
        ystep = 0;
    }
    else
    {
        buffer = (char *)data;
        ystep = step;
    }

    if( m_ischroma )
    {
        if( color )
        {
            if( m_iscolor )
            {
                if( m_blue )
                {
                    frame.insert( "BY", Slice( m_type,
                                    buffer - m_datawindow.min.x * 12 - m_datawindow.min.y * ystep,
                                    12, ystep, m_blue->xSampling, m_blue->ySampling, 0.0 ));
                    xsample[0] = m_blue->ySampling;
                }
                if( m_green )
                {
                    frame.insert( "Y", Slice( m_type,
                                    buffer - m_datawindow.min.x * 12 - m_datawindow.min.y * ystep + 4,
                                    12, ystep, m_green->xSampling, m_green->ySampling, 0.0 ));
                    xsample[1] = m_green->ySampling;
                }
                if( m_red )
                {
                    frame.insert( "RY", Slice( m_type,
                                    buffer - m_datawindow.min.x * 12 - m_datawindow.min.y * ystep + 8,
                                    12, ystep, m_red->xSampling, m_red->ySampling, 0.0 ));
                    xsample[2] = m_red->ySampling;
                }
                chromatorgb = true;
            }
            else
            {
                frame.insert( "Y", Slice( m_type,
                              buffer - m_datawindow.min.x * 12 - m_datawindow.min.y * ystep,
                              12, ystep, m_green->xSampling, m_green->ySampling, 0.0 ));
                frame.insert( "Y", Slice( m_type,
                              buffer - m_datawindow.min.x * 12 - m_datawindow.min.y * ystep + 4,
                              12, ystep, m_green->xSampling, m_green->ySampling, 0.0 ));
                frame.insert( "Y", Slice( m_type,
                              buffer - m_datawindow.min.x * 12 - m_datawindow.min.y * ystep + 8,
                              12, ystep, m_green->xSampling, m_green->ySampling, 0.0 ));
                xsample[0] = m_green->ySampling;
                xsample[1] = m_green->ySampling;
                xsample[2] = m_green->ySampling;
            }
        }
        else
        {
            frame.insert( "Y", Slice( m_type,
                            buffer - m_datawindow.min.x * 4 - m_datawindow.min.y * ystep,
                            4, ystep, m_green->xSampling, m_green->ySampling, 0.0 ));
            xsample[0] = m_green->ySampling;
        }
    }
    else
    {
        if( m_blue )
        {
            frame.insert( "B", Slice( m_type,
                            buffer - m_datawindow.min.x * 12 - m_datawindow.min.y * ystep,
                            12, ystep, m_blue->xSampling, m_blue->ySampling, 0.0 ));
            xsample[0] = m_blue->ySampling;
        }
        if( m_green )
        {
            frame.insert( "G", Slice( m_type,
                            buffer - m_datawindow.min.x * 12 - m_datawindow.min.y * ystep + 4,
                            12, ystep, m_green->xSampling, m_green->ySampling, 0.0 ));
            xsample[1] = m_green->ySampling;
        }
        if( m_red )
        {
            frame.insert( "R", Slice( m_type,
                            buffer - m_datawindow.min.x * 12 - m_datawindow.min.y * ystep + 8,
                            12, ystep, m_red->xSampling, m_red->ySampling, 0.0 ));
            xsample[2] = m_red->ySampling;
        }
        if(color == 0)
        {
            rgbtogray = true;
            justcopy = false;
        }
    }

    m_file->setFrameBuffer( frame );
    if( justcopy )
    {
        m_file->readPixels( m_datawindow.min.y, m_datawindow.max.y );

        if( color )
        {
            if( m_blue && (m_blue->xSampling != 1 || m_blue->ySampling != 1) )
                UpSample( data, 3, step / xstep, xsample[0], m_blue->ySampling );
            if( m_green && (m_green->xSampling != 1 || m_green->ySampling != 1) )
                UpSample( data + xstep, 3, step / xstep, xsample[1], m_green->ySampling );
            if( m_red && (m_red->xSampling != 1 || m_red->ySampling != 1) )
                UpSample( data + 2 * xstep, 3, step / xstep, xsample[2], m_red->ySampling );
        }
        else if( m_green && (m_green->xSampling != 1 || m_green->ySampling != 1) )
            UpSample( data, 1, step / xstep, xsample[0], m_green->ySampling );
    }
    else
    {
        uchar *out = data;
        int x, y;
        for( y = m_datawindow.min.y; y <= m_datawindow.max.y; y++ )
        {
            m_file->readPixels( y, y );

            if( rgbtogray )
            {
                if( xsample[0] != 1 )
                    UpSampleX( (float *)buffer, 3, xsample[0] );
                if( xsample[1] != 1 )
                    UpSampleX( (float *)buffer + 4, 3, xsample[1] );
                if( xsample[2] != 1 )
                    UpSampleX( (float *)buffer + 8, 3, xsample[2] );

                RGBToGray( (float *)buffer, (float *)out );
            }
            else
            {
                if( xsample[0] != 1 )
                    UpSampleX( (float *)buffer, 3, xsample[0] );
                if( xsample[1] != 1 )
                    UpSampleX( (float *)(buffer + 4), 3, xsample[1] );
                if( xsample[2] != 1 )
                    UpSampleX( (float *)(buffer + 8), 3, xsample[2] );

                if( chromatorgb )
                    ChromaToBGR( (float *)buffer, 1, step );

                if( m_type == FLOAT )
                {
                    float *fi = (float *)buffer;
                    for( x = 0; x < m_width * 3; x++)
                    {
                        int t = cvRound(fi[x]*5);
                        out[x] = CV_CAST_8U(t);
                    }
                }
                else
                {
                    uint *ui = (uint *)buffer;
                    for( x = 0; x < m_width * 3; x++)
                    {
                        uint t = ui[x];
                        out[x] = CV_CAST_8U(t);
                    }
                }
            }

            out += step;
        }
        if( color )
        {
            if( m_blue && (m_blue->xSampling != 1 || m_blue->ySampling != 1) )
                UpSampleY( data, 3, step / xstep, m_blue->ySampling );
            if( m_green && (m_green->xSampling != 1 || m_green->ySampling != 1) )
                UpSampleY( data + xstep, 3, step / xstep, m_green->ySampling );
            if( m_red && (m_red->xSampling != 1 || m_red->ySampling != 1) )
                UpSampleY( data + 2 * xstep, 3, step / xstep, m_red->ySampling );
        }
        else if( m_green && (m_green->xSampling != 1 || m_green->ySampling != 1) )
            UpSampleY( data, 1, step / xstep, m_green->ySampling );
    }

    if( chromatorgb )
        ChromaToBGR( (float *)data, m_height, step / xstep );

    Close();

    return result;
}

/**
// on entry pixel values are stored packed in the upper left corner of the image
// this functions expands them by duplication to cover the whole image
 */
void  GrFmtExrReader::UpSample( uchar *data, int xstep, int ystep, int xsample, int ysample )
{
    for( int y = (m_height - 1) / ysample, yre = m_height - ysample; y >= 0; y--, yre -= ysample )
    {
        for( int x = (m_width - 1) / xsample, xre = m_width - xsample; x >= 0; x--, xre -= xsample )
        {
            for( int i = 0; i < ysample; i++ )
            {
                for( int n = 0; n < xsample; n++ )
                {
                    if( !m_native_depth )
                        data[(yre + i) * ystep + (xre + n) * xstep] = data[y * ystep + x * xstep];
                    else if( m_type == FLOAT )
                        ((float *)data)[(yre + i) * ystep + (xre + n) * xstep] = ((float *)data)[y * ystep + x * xstep];
                    else
                        ((uint *)data)[(yre + i) * ystep + (xre + n) * xstep] = ((uint *)data)[y * ystep + x * xstep];
                }
            }
        }
    }
}

/**
// on entry pixel values are stored packed in the upper left corner of the image
// this functions expands them by duplication to cover the whole image
 */
void  GrFmtExrReader::UpSampleX( float *data, int xstep, int xsample )
{
    for( int x = (m_width - 1) / xsample, xre = m_width - xsample; x >= 0; x--, xre -= xsample )
    {
        for( int n = 0; n < xsample; n++ )
        {
            if( m_type == FLOAT )
                ((float *)data)[(xre + n) * xstep] = ((float *)data)[x * xstep];
            else
                ((uint *)data)[(xre + n) * xstep] = ((uint *)data)[x * xstep];
        }
    }
}

/**
// on entry pixel values are stored packed in the upper left corner of the image
// this functions expands them by duplication to cover the whole image
 */
void  GrFmtExrReader::UpSampleY( uchar *data, int xstep, int ystep, int ysample )
{
    for( int y = m_height - ysample, yre = m_height - ysample; y >= 0; y -= ysample, yre -= ysample )
    {
        for( int x = 0; x < m_width; x++ )
        {
            for( int i = 1; i < ysample; i++ )
            {
                if( !m_native_depth )
                    data[(yre + i) * ystep + x * xstep] = data[y * ystep + x * xstep];
                else if( m_type == FLOAT )
                    ((float *)data)[(yre + i) * ystep + x * xstep] = ((float *)data)[y * ystep + x * xstep];
                else
                    ((uint *)data)[(yre + i) * ystep + x * xstep] = ((uint *)data)[y * ystep + x * xstep];
            }
        }
    }
}

/**
// algorithm from ImfRgbaYca.cpp
 */
void  GrFmtExrReader::ChromaToBGR( float *data, int numlines, int step )
{
    int x, y, t;
    
    for( y = 0; y < numlines; y++ )
    {
        for( x = 0; x < m_width; x++ )
        {
            double b, Y, r;
            if( !m_native_depth )
            {
                b = ((uchar *)data)[y * step + x * 3];
                Y = ((uchar *)data)[y * step + x * 3 + 1];
                r = ((uchar *)data)[y * step + x * 3 + 2];
            }
            else if( m_type == FLOAT )
            {
                b = data[y * step + x * 3];
                Y = data[y * step + x * 3 + 1];
                r = data[y * step + x * 3 + 2];
            }
            else
            {
                b = ((uint *)data)[y * step + x * 3];
                Y = ((uint *)data)[y * step + x * 3 + 1];
                r = ((uint *)data)[y * step + x * 3 + 2];
            }
            r = (r + 1) * Y;
            b = (b + 1) * Y;
            Y = (Y - b * m_chroma.blue[1] - r * m_chroma.red[1]) / m_chroma.green[1];

            if( !m_native_depth )
            {
                int t = cvRound(b);
                ((uchar *)data)[y * step + x * 3] = CV_CAST_8U(t);
                t = cvRound(Y);
                ((uchar *)data)[y * step + x * 3 + 1] = CV_CAST_8U(t);
                t = cvRound(r);
                ((uchar *)data)[y * step + x * 3 + 2] = CV_CAST_8U(t);
            }
            else if( m_type == FLOAT )
            {
                data[y * step + x * 3] = (float)b;
                data[y * step + x * 3 + 1] = (float)Y;
                data[y * step + x * 3 + 2] = (float)r;
            }
            else
            {
                int t = cvRound(b);
                ((uint *)data)[y * step + x * 3] = (uint)MAX(t,0);
                t = cvRound(Y);
                ((uint *)data)[y * step + x * 3 + 1] = (uint)MAX(t,0);
                t = cvRound(r);
                ((uint *)data)[y * step + x * 3 + 2] = (uint)MAX(t,0);
            }
        }
    }
}


/**
// convert one row to gray
*/
void  GrFmtExrReader::RGBToGray( float *in, float *out )
{
    if( m_type == FLOAT )
    {
        if( m_native_depth )
        {
            for( int i = 0, n = 0; i < m_width; i++, n += 3 )
                out[i] = in[n] * m_chroma.blue[0] + in[n + 1] * m_chroma.green[0] + in[n + 2] * m_chroma.red[0];
        }
        else
        {
            uchar *o = (uchar *)out;
            for( int i = 0, n = 0; i < m_width; i++, n += 3 )
                o[i] = (uchar) (in[n] * m_chroma.blue[0] + in[n + 1] * m_chroma.green[0] + in[n + 2] * m_chroma.red[0]);
        }
    }
    else // UINT
    {
        if( m_native_depth )
        {
            uint *ui = (uint *)in;
            for( int i = 0; i < m_width * 3; i++ )
                ui[i] -= 0x80000000;
            int *si = (int *)in;
            for( int i = 0, n = 0; i < m_width; i++, n += 3 )
                ((int *)out)[i] = int(si[n] * m_chroma.blue[0] + si[n + 1] * m_chroma.green[0] + si[n + 2] * m_chroma.red[0]);
        }
        else // how to best convert float to uchar?
        {
            uint *ui = (uint *)in;
            for( int i = 0, n = 0; i < m_width; i++, n += 3 )
                ((uchar *)out)[i] = uchar((ui[n] * m_chroma.blue[0] + ui[n + 1] * m_chroma.green[0] + ui[n + 2] * m_chroma.red[0]) * (256.0 / 4294967296.0));
        }
    }
}

/////////////////////// GrFmtExrWriter ///////////////////


GrFmtExrWriter::GrFmtExrWriter( const char* filename ) : GrFmtWriter( filename )
{
}


GrFmtExrWriter::~GrFmtExrWriter()
{
}


bool  GrFmtExrWriter::IsFormatSupported( int depth )
{
    return depth == IPL_DEPTH_8U || depth == IPL_DEPTH_8S ||
           depth == IPL_DEPTH_16U || depth == IPL_DEPTH_16S ||
           depth == IPL_DEPTH_32S || depth == IPL_DEPTH_32F;
           // TODO: do (or should) we support 64f?
}


// TODO scale appropriately
bool  GrFmtExrWriter::WriteImage( const uchar* data, int step,
                                  int width, int height, int depth, int channels )
{
    bool result = false;

    Header header( width, height );
    PixelType type;
    bool issigned = depth < 0;
    bool isfloat = depth == IPL_DEPTH_32F || depth == IPL_DEPTH_64F;

    if(depth == IPL_DEPTH_8U || depth == IPL_DEPTH_8S)
        type = HALF;
    else if(isfloat)
        type = FLOAT;
    else
        type = UINT;

    depth &= 255;

    if( channels == 3 )
    {
        header.channels().insert( "R", Channel( type ));
        header.channels().insert( "G", Channel( type ));
        header.channels().insert( "B", Channel( type ));
        //printf("bunt\n");
    }
    else
    {
        header.channels().insert( "Y", Channel( type ));
        //printf("gray\n");
    }

    OutputFile file( m_filename, header );

    FrameBuffer frame;

    char *buffer;
    int bufferstep;
    int size;
    if( type == FLOAT && depth == 32 )
    {
        buffer = (char *)const_cast<uchar *>(data);
        bufferstep = step;
        size = 4;
    }
    else if( depth > 16 || type == UINT )
    {
        buffer = (char *)new uint[width * channels];
        bufferstep = 0;
        size = 4;
    }
    else
    {
        buffer = (char *)new half[width * channels];
        bufferstep = 0;
        size = 2;
    }

    //printf("depth %d %s\n", depth, types[type]);

    if( channels == 3 )
    {
        frame.insert( "B", Slice( type, buffer, size * 3, bufferstep ));
        frame.insert( "G", Slice( type, buffer + size, size * 3, bufferstep ));
        frame.insert( "R", Slice( type, buffer + size * 2, size * 3, bufferstep ));
    }
    else
        frame.insert( "Y", Slice( type, buffer, size, bufferstep ));

    file.setFrameBuffer( frame );

    int offset = issigned ? 1 << (depth - 1) : 0;

    result = true;
    if( type == FLOAT && depth == 32 )
    {
        try
        {
            file.writePixels( height );
        }
        catch(...)
        {
            result = false;
        }
    }
    else
    {
    //    int scale = 1 << (32 - depth);
    //    printf("scale %d\n", scale);
        for(int line = 0; line < height; line++)
        {
            if(type == UINT)
            {
                uint *buf = (uint *)buffer; // FIXME 64-bit problems

                if( depth <= 8 )
                {
                    for(int i = 0; i < width * channels; i++)
                        buf[i] = data[i] + offset;
                }
                else if( depth <= 16 )
                {
                    unsigned short *sd = (unsigned short *)data;
                    for(int i = 0; i < width * channels; i++)
                        buf[i] = sd[i] + offset;
                }
                else
                {
                    int *sd = (int *)data; // FIXME 64-bit problems
                    for(int i = 0; i < width * channels; i++)
                        buf[i] = (uint) sd[i] + offset;
                }
            }
            else
            {
                half *buf = (half *)buffer;

                if( depth <= 8 )
                {
                    for(int i = 0; i < width * channels; i++)
                        buf[i] = data[i];
                }
                else if( depth <= 16 )
                {
                    unsigned short *sd = (unsigned short *)data;
                    for(int i = 0; i < width * channels; i++)
                        buf[i] = sd[i];
                }
            }
            try
            {
                file.writePixels( 1 );
            }
            catch(...)
            {
                result = false;
                break;
            }
            data += step;
        }
        delete buffer;
    }

    return result;
}

#endif

/* End of file. */

#include "ColorHistogram.h"
#include "string.h"
#include <stdio.h>

void computeHistogram(frameBuffer* image, int histogramBuffer[], u8 bins[])
{
    // compute number of channels
    u8 channels = 1;
    if (image->p2)
        ++channels;
    if (image->p3)
        ++channels;
    // compute the total number of bins
    size_t totalBins = 0;
    for (int i = 0; i < channels; ++i)
    {
        totalBins += bins[i];
    }
    memset((void*) histogramBuffer, 0, totalBins * sizeof(int));

    unsigned char* imagePlane[3] =
    { image->p1, image->p2, image->p3 };

    u16 offset = 0;
    for (u8 i = 0; i < channels; ++i)
    {
        if (bins[i] > 0 && imagePlane[i] != NULL)
        {
            for (unsigned int row = 0; row < image->spec.height; ++row)
            {
                for (unsigned int col = 0; col < image->spec.width; ++col)
                {
                    u16 pixelVal = imagePlane[i][row * image->spec.stride + col];
                    ++histogramBuffer[offset + pixelVal * bins[i] / 256];
                }
            }
        }
        offset += bins[i];
    }
}

void computeHistogramInRoi(frameBuffer* image,
                           int histogramBuffer[],
                           u8 bins[],
                           Rect roi)
{
    // compute number of channels
    u8 channels = 1;
    if (image->p2)
        ++channels;
    if (image->p3)
        ++channels;
    // compute the total number of bins
    size_t totalBins = 0;
    for (int i = 0; i < channels; ++i)
    {
        totalBins += bins[i];
    }
    memset((void*) histogramBuffer, 0, totalBins);

    unsigned char* imagePlane[3] = { image->p1, image->p2, image->p3 };

    u16 offset = 0;
    for (u8 i = 0; i < channels; ++i)
    {
        if (bins[i] > 0 && imagePlane[i] != NULL)
        {
            for (unsigned int row = roi.y;
                 row < roi.y + roi.height && row < image->spec.height;
                 ++row)
            {
                for (unsigned int col = roi.x;
                     col < roi.x + roi.width && col < image->spec.width;
                     ++col)
                {
                    u16 pixelVal = imagePlane[i][row * image->spec.stride + col];
                    ++histogramBuffer[offset + pixelVal * bins[i] / 256];
                }
            }
        }
        offset += bins[i];
    }
}

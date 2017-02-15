/**
 * @file ColorHistogram.h
 * @brief Provides functions to compute color histograms.
 */
#ifndef LEON_COLORHISTOGRAM_H_
#define LEON_COLORHISTOGRAM_H_

#include <mv_types.h>
#include <swcFrameTypes.h>

/**
 * @brief Defines a rectangle in an image. The origin is in the upper left
 * corner.
 */
typedef struct rectangle
{
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
} Rect;

/**
 * @brief Computes the color histogram of the specified image.
 *
 * @param image A pointer to the image whose histogram shall be computed.
 * @param histogramBuffer The buffer in which to store the histogram.
 * @param bins The number of bins to use for each channel of the provided image.
 */
void computeHistogram(frameBuffer* image, int histogramBuffer[], u8 bins[]);

/**
 * @brief Computes the color histogram in the specified region of interest in
 * the image.
 *
 * @param image A pointer to the image whose histogram shall be computed.
 * @param histogramBuffer The buffer in which to store the histogram.
 * @param bins The number of bins to use for each channel of the provided image.
 * @param roi The region (rectangle) of interest in which to compute the
 *  histogram.
 */
void computeHistogramInRoi(frameBuffer* image,
                           int histogramBuffer[],
                           u8 bins[],
                           Rect roi);

#endif /* LEON_COLORHISTOGRAM_H_ */

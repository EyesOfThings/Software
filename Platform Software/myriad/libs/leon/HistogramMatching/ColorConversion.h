/**
 * @file ColorConversion.h
 * @brief Provides functions to convert images into different color spaces.
 */
#ifndef LEON_COLORCONVERSION_H_
#define LEON_COLORCONVERSION_H_

#include <swcFrameTypes.h>

/**
 * @brief This code specifies source and target color space for a color space
 *  conversion.
 */
typedef enum ConversionCode
{
    /**
     * @brief Convert sRGB to HSV.
     *
     * The hue channel H is mapped into the range [0, 180] as follows:
     * H := H / 2.
     * The S and V channels are mapped into the range [0, 255] as follows:
     * S := S * 255, V := V * 255.
     */
    COLOR_RGB2HSV,
    /**
     * @brief Convert sRGB to the hue channel H of the HSV color space.
     *
     * The hue channel H is mapped into the range [0, 180] as follows:
     * H := H / 2.
     */
    COLOR_RGB2H,
    /**
     * @brief Convert sRGB to CIELAB 1976 with white point D50.
     *
     * The LAB components are mapped into the range [0, 255] as follows:
     * L := L * 255/100, A := A + 128, B := B + 128.
     */
    COLOR_RGB2Lab_D50,
    /**
     * @brief Converts sRGB to CIELAB 1976 with white point D65.
     *
     * The LAB components are mapped into the range [0, 255] as follows:
     * L := L * 255/100, A := A + 128, B := B + 128.
     */
    COLOR_RGB2Lab_D65
} ColorConversionCode;

/**
 * @brief Converts an image from one color space into another.
 *
 * @param image The image to convert.
 * @param converted The converted image. Has to be pre-allocated.
 *  In-place operation is supported.
 * @param code The color conversion code specifying from which color space to
 *  convert into which target color space.
 */
void convertColor(const frameBuffer* image,
                  frameBuffer* converted,
                  ColorConversionCode code);

#endif /* LEON_COLORCONVERSION_H_ */

#include "ColorConversion.h"
#include <math.h>
#include <mv_types.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void convertColor(const frameBuffer* image,
                  frameBuffer* converted,
                  ColorConversionCode code)
{
    switch (code)
    {
    case COLOR_RGB2HSV:
        for (unsigned int row = 0; row < image->spec.height; ++row)
        {
            for (unsigned int col = 0; col < image->spec.width; ++col)
            {
                unsigned int offset = row * image->spec.stride + col;
                short r = image->p1[offset];
                short g = image->p2[offset];
                short b = image->p3[offset];
                short max = MAX(MAX(r, g), b);
                short min = MIN(MIN(r, g), b);
                short h;
                // compute H channel
                if (max == min)
                {
                    h = 0;
                }
                else if (max == r)
                {
                    h = 0 + 30 * (g - b) / (max - min);
                }
                else if (max == g)
                {
                    h = 60 + 30 * (b - r) / (max - min);
                }
                else /*if (max == b)*/
                {
                    h = 120 + 30 * (r - g) / (max - min);
                }
                if (h < 0)
                    h = h + 180;
                converted->p1[offset] = (unsigned char) h;
                // compute S channel
                converted->p2[offset] = (unsigned char) ((max - min) / max);
                // compute V channel
                converted->p3[offset] = (unsigned char) max;
            }
        }
        break;
    case COLOR_RGB2H:
        for (unsigned int row = 0; row < image->spec.height; ++row)
        {
            for (unsigned int col = 0; col < image->spec.width; ++col)
            {
                unsigned int offset = row * image->spec.stride + col;
                short r = image->p1[offset];
                short g = image->p2[offset];
                short b = image->p3[offset];
                short max = MAX(MAX(r, g), b);
                short min = MIN(MIN(r, g), b);
                short h;
                // compute H channel
                if (max == min)
                {
                    h = 0;
                }
                else if (max == r)
                {
                    h = 0 + 30 * (g - b) / (max - min);
                }
                else if (max == g)
                {
                    h = 60 + 30 * (b - r) / (max - min);
                }
                else /*if (max == b)*/
                {
                    h = 120 + 30 * (r - g) / (max - min);
                }
                if (h < 0)
                    h = h + 180;
                converted->p1[offset] = (unsigned char) h;
            }
        }
        break;
    case COLOR_RGB2Lab_D50:
    case COLOR_RGB2Lab_D65:
    {
        const float threshold = 216.0f / 24389.0f;
        float xN, zN;
        if (code == COLOR_RGB2Lab_D50)
        {
            xN = 0.96422f;
            zN = 0.82521f;
        }
        else
        {
            xN = 0.95047f;
            zN = 1.08883f;
        }

        for (unsigned int row = 0; row < image->spec.height; ++row)
        {
            for (unsigned int col = 0; col < image->spec.width; ++col)
            {
                unsigned int offset = row * image->spec.stride + col;
                // scale to [0,1] range
                float r = (float) image->p1[offset] / 255.0f;
                float g = (float) image->p2[offset] / 255.0f;
                float b = (float) image->p3[offset] / 255.0f;
                // apply gamma correction
                if (r > 0.04045)
                    r = powf((r + 0.055)/1.055, 2.4);
                else
                    r = r / 12.92;
                if (g > 0.04045)
                    g = powf((g + 0.055)/1.055, 2.4);
                else
                    g = g / 12.92;
                if (b > 0.04045)
                    b = powf((b + 0.055)/1.055, 2.4);
                else
                    b = b / 12.92;

                // compute X
                float x = (0.4124564f * r + 0.3575761f * g + 0.1804375f * b)/xN;
                // compute Y
                float y = (0.2126729f * r + 0.7151522f * g + 0.0721750f * b);
                // compute Z
                float z = (0.0193339f * r + 0.1191920f * g + 0.9503041f * b)/zN;

                if (x > threshold)
                    x = cbrtf(x);
                else
                    x = (24389.0f/27.0f * x + 16.0f) / 116.0f;

                if (y > threshold)
                    y = cbrtf(y);
                else
                    y = (24389.0f/27.0f * y + 16.0f) / 116.0f;

                if (z > threshold)
                    z = cbrtf(z);
                else
                    z = (24389.0f/27.0f * z + 16.0f) / 116.0f;

                // compute L*
                converted->p1[offset] = (unsigned char)
                        ((116.0f * y - 16.0f) * 2.55f);
                // compute a*
                converted->p2[offset] = (unsigned char)
                        (500.0f * (x - y) + 128.0f);
                // compute b*
                converted->p3[offset] = (unsigned char)
                        (200.0f * (y - z) + 128.0f);
            }
        }
    }
    break;
    }
}

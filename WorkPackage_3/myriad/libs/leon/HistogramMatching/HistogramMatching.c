#include "HistogramMatching.h"
#include "math.h"

float hellingerDistance(u16 bins, int histogramOne[], int histogramTwo[])
{
    float sumOne = 0;
    float sumTwo = 0;
    for (u16 i = 0; i < bins; ++i)
    {
        sumOne += (float) histogramOne[i];
    }
    for (u16 i = 0; i < bins; ++i)
    {
        sumTwo += (float) histogramTwo[i];
    }
    float bc = 0;
    for (u16 i = 0; i < bins; ++i)
    {
        bc += sqrtf((histogramOne[i] / sumOne) * (histogramTwo[i] / sumTwo));
    }
    return 1.0f - bc;
}

float histogramIntersectionDistance(u16 bins,
                                    int histogramOne[],
                                    int histogramTwo[])
{
    float sumOne = 0;
    float sumTwo = 0;
    for (u16 i = 0; i < bins; ++i)
    {
        sumOne += (float) histogramOne[i];
    }
    for (u16 i = 0; i < bins; ++i)
    {
        sumTwo += (float) histogramTwo[i];
    }
    float sum = 0;
    for (u16 i = 0; i < bins; ++i)
    {
        sum += fminf((histogramOne[i] / sumOne), (histogramTwo[i] / sumTwo));
    }
    return 1.0f - sum;
}

float earthMoversDistance(u16 bins, int histogramOne[], int histogramTwo[])
{
    float sumOne = 0;
    float sumTwo = 0;
    for (u16 i = 0; i < bins; ++i)
    {
        sumOne += (float) histogramOne[i];
    }
    for (u16 i = 0; i < bins; ++i)
    {
        sumTwo += (float) histogramTwo[i];
    }
    float sum = 0;
    float emd = 0;
    for (u16 i = 0; i < bins; ++i)
    {
        emd += (histogramOne[i] / sumOne) - (histogramTwo[i] / sumTwo);
        sum += fabsf(emd);
    }
    return sum;
}

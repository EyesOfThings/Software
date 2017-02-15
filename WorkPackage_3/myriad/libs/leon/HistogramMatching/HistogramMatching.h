/**
 * @file HistogramMatching.h
 * @brief Provides functions to compute histogram matching scores.
 */
#ifndef LEON_HISTOGRAMMATCHING_H_
#define LEON_HISTOGRAMMATCHING_H_

#include <mv_types.h>

/**
 * @brief Computes the Hellinger distance of a pair of histograms.
 * @param bins The number of bins to compare.
 * @param histogramOne The first histogram to compare.
 * @param histogramTwo The second histogram to compare.
 * @return The distance between the histograms. A floating point value in the
 *  range [0, 1].
 */
float hellingerDistance(u16 bins, int histogramOne[], int histogramTwo[]);

/**
 * @brief Computes a distance for a pair of histograms based on histogram
 *  intersection.
 * @param bins The number of bins to compare.
 * @param histogramOne The first histogram to compare.
 * @param histogramTwo The second histogram to compare.
 * @return The distance between the histograms. A floating point value in the
 *  range [0, 1].
 */
float histogramIntersectionDistance(u16 bins,
                                    int histogramOne[],
                                    int histogramTwo[]);

/**
 * @brief Computes the earth mover's distance of a pair of histograms.
 * @param bins The number of bins to compare.
 * @param histogramOne The first histogram to compare.
 * @param histogramTwo The second histogram to compare.
 * @return The distance between the histograms. A floating point value in the
 *  range [0, 1].
 */
float earthMoversDistance(u16 bins, int histogramOne[], int histogramTwo[]);

#endif /* LEON_HISTOGRAMMATCHING_H_ */

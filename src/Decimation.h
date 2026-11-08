#pragma once

#include <vector>

#include "SampleBuffer.h"

namespace rtp
{

/** @brief Per-pixel scratch used by decimateMinMax(). Reuse it across calls to avoid allocations. */
struct MinMaxBucket {
    SampleBuffer::Point minPt;
    SampleBuffer::Point maxPt;
    bool                assigned = false;
};

/**
 * @brief Reduces points to at most two per pixel column (the column's min and max, in X order).
 *
 * Points outside [xMin, xMax] are copied unchanged. If no more than 2 * width points
 * lie inside the range, the input is copied unchanged. The visual shape, including
 * every peak, is preserved.
 *
 * @param in Points sorted by X.
 * @param xMin Left edge of the visible range.
 * @param xMax Right edge of the visible range.
 * @param width Number of pixel columns.
 * @param out Receives the reduced points. It is cleared first.
 * @param buckets Scratch storage.
 */
void decimateMinMax(const std::vector<SampleBuffer::Point>& in, double xMin, double xMax, int width,
                    std::vector<SampleBuffer::Point>& out, std::vector<MinMaxBucket>& buckets);

/**
 * @brief Clips the segment (x0, y0)-(x1, y1) to the rectangle [xLo, xHi] x [yLo, yHi] (Liang-Barsky).
 *
 * Works in double precision, so segments whose ends lie far outside the view keep
 * their exact slope once converted to float for the GPU.
 *
 * @return False if no part of the segment lies inside the rectangle.
 */
bool clipSegment(double& x0, double& y0, double& x1, double& y1, double xLo, double xHi, double yLo, double yHi);

}  // namespace rtp

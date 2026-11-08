// Checks min/max decimation and the segment clipper used before uploading to the GPU.

#include <vector>

#include "Decimation.h"
#include "TestUtil.h"

using Point = SampleBuffer::Point;

int main()
{
    std::vector<Point>             out;
    std::vector<rtp::MinMaxBucket> buckets;

    // Outside neighbours pass through untouched while the inside is reduced.
    {
        std::vector<Point> in;
        in.push_back({-1000.0, 7.0});
        for (int i = 0; i < 10000; ++i)
            in.push_back({i * 0.01, (i % 10 == 0) ? 5.0 : 0.0});
        in.push_back({5000.0, -7.0});

        rtp::decimateMinMax(in, 0.0, 100.0, 50, out, buckets);
        CHECK(out.size() <= 50 * 2 + 2);
        CHECK(out.front().x == -1000.0 && out.front().y == 7.0);
        CHECK(out.back().x == 5000.0 && out.back().y == -7.0);
        double mx = -1e300;
        for (size_t i = 1; i + 1 < out.size(); ++i)
        {
            CHECK(out[i].x >= 0.0 && out[i].x <= 100.0);
            mx = std::max(mx, out[i].y);
        }
        CHECK(mx == 5.0);  // peaks survive
    }

    // Small inputs are copied as they are.
    {
        const std::vector<Point> in = {{-1, 0}, {0.5, 1}, {2, 0}};
        rtp::decimateMinMax(in, 0.0, 1.0, 10, out, buckets);
        CHECK(out.size() == 3);
    }

    // Clipping keeps the exact line through the rectangle.
    {
        double x0 = -1e12, y0 = -1e12, x1 = 1e12, y1 = 1e12;  // the diagonal y = x, far beyond the box
        CHECK(rtp::clipSegment(x0, y0, x1, y1, 0, 1, 0, 1));
        CHECK(nearlyEqual(x0, 0) && nearlyEqual(y0, 0) && nearlyEqual(x1, 1) && nearlyEqual(y1, 1));

        double a0 = -5, b0 = 0.25, a1 = 5, b1 = 0.25;  // horizontal line crossing the box
        CHECK(rtp::clipSegment(a0, b0, a1, b1, 0, 1, 0, 1));
        CHECK(a0 == 0 && a1 == 1 && b0 == 0.25 && b1 == 0.25);

        double c0 = 2, d0 = 2, c1 = 3, d1 = 5;  // entirely outside
        CHECK(!rtp::clipSegment(c0, d0, c1, d1, 0, 1, 0, 1));

        double e0 = 0.2, f0 = 0.3, e1 = 0.4, f1 = 0.6;  // entirely inside: unchanged
        CHECK(rtp::clipSegment(e0, f0, e1, f1, 0, 1, 0, 1));
        CHECK(e0 == 0.2 && f0 == 0.3 && e1 == 0.4 && f1 == 0.6);

        // Tiny box far from the origin, as in a deep zoom on 10^-20 data.
        double g0 = 0, h0 = 0, g1 = 2e-20, h1 = 2e-20;
        CHECK(rtp::clipSegment(g0, h0, g1, h1, 0.5e-20, 1.5e-20, 0.5e-20, 1.5e-20));
        CHECK(nearlyEqual(g0 * 1e20, 0.5) && nearlyEqual(g1 * 1e20, 1.5));
    }

    return testResult("test_decimation");
}

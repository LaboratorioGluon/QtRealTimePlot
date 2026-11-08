#include "Decimation.h"

#include <algorithm>

namespace rtp
{

void decimateMinMax(const std::vector<SampleBuffer::Point>& in, double xMin, double xMax, int width,
                    std::vector<SampleBuffer::Point>& out, std::vector<MinMaxBucket>& buckets)
{
    out.clear();
    if (width <= 0 || !(xMax > xMin))
        return;

    // Points outside [xMin, xMax] (the neighbours that let lines reach the edges)
    // are kept as they are; only the points inside the range are reduced.
    auto inBegin =
        std::lower_bound(in.begin(), in.end(), xMin, [](const SampleBuffer::Point& p, double v) { return p.x < v; });
    auto inEnd =
        std::upper_bound(inBegin, in.end(), xMax, [](double v, const SampleBuffer::Point& p) { return v < p.x; });

    const size_t inside    = static_cast<size_t>(std::distance(inBegin, inEnd));
    const size_t maxPoints = static_cast<size_t>(width) * 2;
    if (inside <= maxPoints)
    {
        out.assign(in.begin(), in.end());
        return;
    }

    out.reserve(maxPoints + (in.size() - inside));
    out.insert(out.end(), in.begin(), inBegin);

    buckets.assign(static_cast<size_t>(width), MinMaxBucket{});
    const double scale = static_cast<double>(width) / (xMax - xMin);
    for (auto it = inBegin; it != inEnd; ++it)
    {
        const auto&   p   = *it;
        const int     col = std::clamp(static_cast<int>((p.x - xMin) * scale), 0, width - 1);
        MinMaxBucket& b   = buckets[col];
        if (!b.assigned)
        {
            b.minPt    = p;
            b.maxPt    = p;
            b.assigned = true;
        }
        else
        {
            if (p.y < b.minPt.y)
                b.minPt = p;
            if (p.y > b.maxPt.y)
                b.maxPt = p;
        }
    }

    for (const auto& b : buckets)
    {
        if (!b.assigned)
            continue;
        const bool minFirst = b.minPt.x < b.maxPt.x;
        out.push_back(minFirst ? b.minPt : b.maxPt);
        out.push_back(minFirst ? b.maxPt : b.minPt);
    }

    out.insert(out.end(), inEnd, in.end());
}

bool clipSegment(double& x0, double& y0, double& x1, double& y1, double xLo, double xHi, double yLo, double yHi)
{
    const double dx = x1 - x0;
    const double dy = y1 - y0;
    double       t0 = 0.0, t1 = 1.0;
    int          edge0 = -1, edge1 = -1;  // edge that set t0 / t1 (0: xLo, 1: xHi, 2: yLo, 3: yHi)

    // For each edge: p * t <= q must hold for the part of the segment inside.
    const double p[4] = {-dx, dx, -dy, dy};
    const double q[4] = {x0 - xLo, xHi - x0, y0 - yLo, yHi - y0};
    for (int i = 0; i < 4; ++i)
    {
        if (p[i] == 0.0)
        {
            if (q[i] < 0.0)
                return false;  // parallel to this edge and outside it
            continue;
        }
        const double t = q[i] / p[i];
        if (p[i] < 0.0)
        {
            if (t > t0)
            {
                t0    = t;
                edge0 = i;
            }
        }
        else if (t < t1)
        {
            t1    = t;
            edge1 = i;
        }
        if (t0 > t1)
            return false;
    }

    // Place each clipped end exactly on the edge it crosses, extrapolating from the
    // nearer original endpoint. Interpolating from a far endpoint (x0 + t * dx) would
    // lose precision when the segment is many times longer than the rectangle.
    const double edgeValue[4] = {xLo, xHi, yLo, yHi};
    auto         onEdge       = [&](double t, int edge, double& ox, double& oy) {
        const double rx = (t < 0.5) ? x0 : x1;
        const double ry = (t < 0.5) ? y0 : y1;
        if (edge < 2)
        {
            ox = edgeValue[edge];
            oy = ry + (ox - rx) * (dy / dx);
        }
        else
        {
            oy = edgeValue[edge];
            ox = rx + (oy - ry) * (dx / dy);
        }
    };

    double nx0 = x0, ny0 = y0, nx1 = x1, ny1 = y1;
    if (edge0 >= 0)
        onEdge(t0, edge0, nx0, ny0);
    if (edge1 >= 0)
        onEdge(t1, edge1, nx1, ny1);
    x0 = nx0;
    y0 = ny0;
    x1 = nx1;
    y1 = ny1;
    return true;
}

}  // namespace rtp

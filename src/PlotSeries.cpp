#include "PlotSeries.h"

#include <algorithm>
#include <cmath>

namespace {

template <typename Container>
auto lowerByX(const Container& c, double x)
{
    return std::lower_bound(c.begin(), c.end(), x, [](const PlotSeries::Point& p, double v) { return p.x < v; });
}

template <typename Container>
auto upperByX(const Container& c, double x)
{
    return std::upper_bound(c.begin(), c.end(), x, [](double v, const PlotSeries::Point& p) { return v < p.x; });
}

}  // namespace

PlotSeries::PlotSeries(QString name, QColor color, size_t maxPoints)
    : m_maxPoints(maxPoints), m_name(std::move(name)), m_color(color)
{}

bool PlotSeries::pushPoint(double x, double y)
{
    return pushPoint(Point{x, y});
}

bool PlotSeries::pushPoint(const Point& p)
{
    if (!std::isfinite(p.x) || !std::isfinite(p.y))
    {
        ++m_dropped;
        return false;
    }

    std::lock_guard<std::mutex> lg(m_mutex);

    auto& raw = m_lodLevels[0].points;
    if (!raw.empty() && p.x < raw.back().x)
    {
        ++m_dropped;
        return false;
    }

    raw.push_back(p);
    const uint64_t idx = m_lodLevels[0].base + raw.size() - 1;

    while (!m_yMinQueue.empty() && m_yMinQueue.back().second >= p.y)
        m_yMinQueue.pop_back();
    m_yMinQueue.emplace_back(idx, p.y);

    while (!m_yMaxQueue.empty() && m_yMaxQueue.back().second <= p.y)
        m_yMaxQueue.pop_back();
    m_yMaxQueue.emplace_back(idx, p.y);

    const uint64_t rawEnd  = m_lodLevels[0].base + raw.size();
    const uint64_t pending = rawEnd - std::max(m_lodLevels[1].nextSource, m_lodLevels[0].base);
    if (pending >= LOD_FACTOR)
        updateLodLevels();

    enforceCapacity();

    ++m_version;
    return true;
}

void PlotSeries::clear()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    for (auto& level : m_lodLevels)
    {
        level.points.clear();
        level.stats.clear();
        level.base       = 0;
        level.nextSource = 0;
    }
    m_yMinQueue.clear();
    m_yMaxQueue.clear();
    ++m_version;
}

void PlotSeries::setMaxPoints(size_t maxPoints)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_maxPoints = maxPoints;
    enforceCapacity();
    ++m_version;
}

size_t PlotSeries::maxPoints() const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_maxPoints;
}

std::unique_lock<std::mutex> PlotSeries::lock() const
{
    return std::unique_lock<std::mutex>(m_mutex);
}

size_t PlotSeries::size() const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_lodLevels[0].points.size();
}

double PlotSeries::xMin() const
{
    const auto& raw = m_lodLevels[0].points;
    return raw.empty() ? std::numeric_limits<double>::max() : raw.front().x;
}

double PlotSeries::xMax() const
{
    const auto& raw = m_lodLevels[0].points;
    return raw.empty() ? -std::numeric_limits<double>::max() : raw.back().x;
}

double PlotSeries::yMin() const
{
    return m_yMinQueue.empty() ? std::numeric_limits<double>::max() : m_yMinQueue.front().second;
}

double PlotSeries::yMax() const
{
    return m_yMaxQueue.empty() ? -std::numeric_limits<double>::max() : m_yMaxQueue.front().second;
}

void PlotSeries::enforceCapacity()
{
    auto& L0 = m_lodLevels[0];
    if (m_maxPoints == 0 || L0.points.size() <= m_maxPoints)
        return;

    while (L0.points.size() > m_maxPoints)
    {
        const uint64_t idx = L0.base;
        if (!m_yMinQueue.empty() && m_yMinQueue.front().first == idx)
            m_yMinQueue.pop_front();
        if (!m_yMaxQueue.empty() && m_yMaxQueue.front().first == idx)
            m_yMaxQueue.pop_front();
        L0.points.pop_front();
        ++L0.base;
    }

    // Trim coarser levels so they never describe evicted samples.
    auto& L1 = m_lodLevels[1];
    while (!L1.stats.empty() && L1.stats.front().firstIndex < L0.base)
        L1.stats.pop_front();

    for (size_t lvl = 1; lvl < NUM_LOD_LEVELS; ++lvl)
    {
        auto&        L     = m_lodLevels[lvl];
        const auto&  below = m_lodLevels[lvl - 1].points;
        const double front = below.empty() ? std::numeric_limits<double>::max() : below.front().x;
        while (!L.points.empty() && L.points.front().x < front)
        {
            L.points.pop_front();
            ++L.base;
        }
    }
}

void PlotSeries::updateLodLevels()
{
    // Level 1: min/max + statistics of each block of LOD_FACTOR raw samples.
    {
        const auto& L0  = m_lodLevels[0];
        auto&       L1  = m_lodLevels[1];
        uint64_t    next = std::max(L1.nextSource, L0.base);
        const uint64_t end = L0.base + L0.points.size();

        while (end - next >= LOD_FACTOR)
        {
            const size_t i      = static_cast<size_t>(next - L0.base);
            size_t       minIdx = i;
            size_t       maxIdx = i;

            BlockStats block;
            block.xStart     = L0.points[i].x;
            block.xEnd       = L0.points[i + LOD_FACTOR - 1].x;
            block.min        = std::numeric_limits<double>::max();
            block.max        = -std::numeric_limits<double>::max();
            block.sum        = 0.0;
            block.sumSq      = 0.0;
            block.count      = LOD_FACTOR;
            block.firstIndex = next;

            for (size_t k = i; k < i + LOD_FACTOR; ++k)
            {
                const double y = L0.points[k].y;
                if (y < block.min)
                {
                    block.min = y;
                    minIdx    = k;
                }
                if (y > block.max)
                {
                    block.max = y;
                    maxIdx    = k;
                }
                block.sum += y;
                block.sumSq += y * y;
            }

            L1.points.push_back(L0.points[std::min(minIdx, maxIdx)]);
            L1.points.push_back(L0.points[std::max(minIdx, maxIdx)]);
            L1.stats.push_back(block);
            next += LOD_FACTOR;
        }
        L1.nextSource = next;
    }

    // Level 2: min/max of each block of LOD_FACTOR level-1 points.
    {
        const auto& L1   = m_lodLevels[1];
        auto&       L2   = m_lodLevels[2];
        uint64_t    next = std::max(L2.nextSource, L1.base);
        const uint64_t end = L1.base + L1.points.size();

        while (end - next >= LOD_FACTOR)
        {
            const size_t i      = static_cast<size_t>(next - L1.base);
            size_t       minIdx = i;
            size_t       maxIdx = i;
            for (size_t k = i + 1; k < i + LOD_FACTOR; ++k)
            {
                if (L1.points[k].y < L1.points[minIdx].y)
                    minIdx = k;
                if (L1.points[k].y > L1.points[maxIdx].y)
                    maxIdx = k;
            }
            L2.points.push_back(L1.points[std::min(minIdx, maxIdx)]);
            L2.points.push_back(L1.points[std::max(minIdx, maxIdx)]);
            next += LOD_FACTOR;
        }
        L2.nextSource = next;
    }
}

void PlotSeries::gatherPoints(size_t level, double lo, double hi, bool loInclusive, bool hiInclusive,
                              std::vector<Point>& out) const
{
    if (lo > hi)
        return;

    const auto& pts = m_lodLevels[level].points;

    if (level > 0 && pts.empty())
    {
        gatherPoints(level - 1, lo, hi, loInclusive, hiInclusive, out);
        return;
    }

    // Region before this level's coverage (e.g. right after old samples were evicted).
    if (level > 0 && lo < pts.front().x)
    {
        const double front = pts.front().x;
        if (hi < front)
            gatherPoints(level - 1, lo, hi, loInclusive, hiInclusive, out);
        else
            gatherPoints(level - 1, lo, front, loInclusive, false, out);
    }

    auto first = loInclusive ? lowerByX(pts, lo) : upperByX(pts, lo);
    auto last  = hiInclusive ? upperByX(pts, hi) : lowerByX(pts, hi);
    if (first < last)
        out.insert(out.end(), first, last);

    // Newest samples that have not been folded into this level yet.
    if (level > 0 && hi > pts.back().x)
    {
        const double back = pts.back().x;
        if (lo > back)
            gatherPoints(level - 1, lo, hi, loInclusive, hiInclusive, out);
        else
            gatherPoints(level - 1, back, hi, false, hiInclusive, out);
    }
}

std::vector<PlotSeries::Point>& PlotSeries::getVisiblePoints(double xMin, double xMax, int targetWidth)
{
    m_visibleBuffer.clear();
    m_gatherBuffer.clear();

    if (targetWidth <= 0 || !(xMax > xMin))
        return m_visibleBuffer;

    {
        std::lock_guard<std::mutex> lg(m_mutex);
        const auto&                 raw = m_lodLevels[0].points;
        if (raw.empty())
            return m_visibleBuffer;

        const size_t totalVisibleRaw = std::distance(lowerByX(raw, xMin), upperByX(raw, xMax));
        if (totalVisibleRaw == 0)
            return m_visibleBuffer;

        const double pointsPerPixel = static_cast<double>(totalVisibleRaw) / targetWidth;
        size_t       level          = 0;
        if (pointsPerPixel > 10000.0)
            level = 2;
        else if (pointsPerPixel > 100.0)
            level = 1;

        gatherPoints(level, xMin, xMax, true, true, m_gatherBuffer);
    }

    // Everything below runs without the lock.
    const size_t maxPointsThreshold = static_cast<size_t>(targetWidth) * 2;
    if (m_gatherBuffer.size() <= maxPointsThreshold)
    {
        std::swap(m_visibleBuffer, m_gatherBuffer);
        return m_visibleBuffer;
    }

    m_visibleBuffer.reserve(maxPointsThreshold);
    m_buckets.assign(static_cast<size_t>(targetWidth), BucketData{});

    const double scale = static_cast<double>(targetWidth) / (xMax - xMin);
    for (const Point& p : m_gatherBuffer)
    {
        int pixelIdx = static_cast<int>((p.x - xMin) * scale);
        pixelIdx     = std::clamp(pixelIdx, 0, targetWidth - 1);

        BucketData& b = m_buckets[pixelIdx];
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

    for (const BucketData& b : m_buckets)
    {
        if (!b.assigned)
            continue;
        if (b.minPt.x < b.maxPt.x)
        {
            m_visibleBuffer.push_back(b.minPt);
            m_visibleBuffer.push_back(b.maxPt);
        }
        else
        {
            m_visibleBuffer.push_back(b.maxPt);
            m_visibleBuffer.push_back(b.minPt);
        }
    }

    return m_visibleBuffer;
}

PlotSeries::Point PlotSeries::getClosestPointToX(double xValue) const
{
    std::lock_guard<std::mutex> lg(m_mutex);
    const auto&                 raw = m_lodLevels[0].points;

    if (raw.empty())
        return Point{0.0, 0.0};

    auto it = lowerByX(raw, xValue);
    if (it == raw.begin())
        return raw.front();
    if (it == raw.end())
        return raw.back();

    auto prevIt = std::prev(it);
    return (std::abs(prevIt->x - xValue) < std::abs(it->x - xValue)) ? *prevIt : *it;
}

PlotSeries::IntervalStats PlotSeries::calculateIntervalStats(double xMin, double xMax) const
{
    std::lock_guard<std::mutex> lg(m_mutex);

    IntervalStats res;
    const auto&   L0 = m_lodLevels[0];
    if (xMin > xMax || L0.points.empty())
        return res;

    const uint64_t startIdx = L0.base + std::distance(L0.points.begin(), lowerByX(L0.points, xMin));
    const uint64_t endIdx   = L0.base + std::distance(L0.points.begin(), upperByX(L0.points, xMax));

    double totalSum   = 0.0;
    double totalSumSq = 0.0;
    size_t totalCount = 0;

    // Whole precomputed blocks inside [startIdx, endIdx).
    const auto& stats   = m_lodLevels[1].stats;
    auto        blockIt = std::lower_bound(stats.begin(), stats.end(), startIdx,
                                           [](const BlockStats& b, uint64_t v) { return b.firstIndex < v; });

    uint64_t blocksStart = endIdx;
    uint64_t blocksEnd   = endIdx;
    if (blockIt != stats.end() && blockIt->firstIndex + blockIt->count <= endIdx)
    {
        blocksStart = blockIt->firstIndex;
        for (; blockIt != stats.end() && blockIt->firstIndex + blockIt->count <= endIdx; ++blockIt)
        {
            res.min = std::min(res.min, blockIt->min);
            res.max = std::max(res.max, blockIt->max);
            totalSum += blockIt->sum;
            totalSumSq += blockIt->sumSq;
            totalCount += blockIt->count;
            blocksEnd = blockIt->firstIndex + blockIt->count;
        }
    }

    // Raw samples outside the blocks.
    auto processRaw = [&](uint64_t from, uint64_t to) {
        for (uint64_t i = from; i < to; ++i)
        {
            const double y = L0.points[static_cast<size_t>(i - L0.base)].y;
            res.min        = std::min(res.min, y);
            res.max        = std::max(res.max, y);
            totalSum += y;
            totalSumSq += y * y;
            ++totalCount;
        }
    };
    processRaw(startIdx, blocksStart);
    processRaw(blocksEnd, endIdx);

    if (totalCount > 0)
    {
        res.numSamples = static_cast<int>(totalCount);
        res.timeDiff   = xMax - xMin;
        res.mean       = totalSum / totalCount;
        res.rms        = std::sqrt(totalSumSq / totalCount);
    }

    return res;
}

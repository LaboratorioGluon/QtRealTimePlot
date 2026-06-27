#include "PlotSeries.h"

#include <QOpenGLContext>

PlotSeries::PlotSeries(QString name, QColor color)
    : m_name(std::move(name)), m_color(color)
{
    m_lodLevels.resize(NUM_LOD_LEVELS);
}

void PlotSeries::pushPoint(double x, double y)
{
    pushPoint({x, y});
}

void PlotSeries::pushPoint(const Point &p)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_lodLevels[0].points.push_back(p);
    updateBounds(p);

    m_points.push_back(p);
    updateBounds(p);

    if (m_lodLevels[0].points.size() - m_lodLevels[1].lastProcessedCrudeSize >= 1000)
    {
        updateLodLevels();
    }
}

void PlotSeries::clear()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_points.clear();
    for (auto &level : m_lodLevels)
    {
        level.points.clear();
        level.lastProcessedCrudeSize = 0;
    }
    m_pointsInGPU = 0;
    m_xMin = std::numeric_limits<double>::max();
    m_xMax = -std::numeric_limits<double>::max();
    m_yMin = std::numeric_limits<double>::max();
    m_yMax = -std::numeric_limits<double>::max();
}

std::unique_lock<std::mutex> PlotSeries::lock() const
{
    return std::unique_lock<std::mutex>(m_mutex);
}

void PlotSeries::updateBounds(const Point &p)
{
    if (p.x < m_xMin)
        m_xMin = p.x;
    if (p.x > m_xMax)
        m_xMax = p.x;
    if (p.y < m_yMin)
        m_yMin = p.y;
    if (p.y > m_yMax)
        m_yMax = p.y;
}

void PlotSeries::recomputeBounds()
{
    m_xMin = std::numeric_limits<double>::max();
    m_xMax = -std::numeric_limits<double>::max();
    m_yMin = std::numeric_limits<double>::max();
    m_yMax = -std::numeric_limits<double>::max();
    for (const auto &p : m_points)
        updateBounds(p);
}

void PlotSeries::initGLBuffers()
{
    if (m_glInitialized)
        return;
    initializeOpenGLFunctions(); // Inicializa el d_ptr de esta serie

    m_vao.create();
    m_vbo.create();

    m_vbo.bind();

    m_vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);

    m_vbo.release();

    m_glInitialized = true;
}

void PlotSeries::uploadVisiblePoints(const std::vector<Point> &visiblePts)
{
    if (!m_glInitialized || visiblePts.empty())
        return;

    m_vbo.bind();
    m_vbo.allocate(visiblePts.data(), static_cast<int>(visiblePts.size() * sizeof(Point)));
    m_vbo.release();
}

void PlotSeries::destroyGLBuffers()
{
    if (!m_glInitialized)
        return;
    m_vao.destroy();
    m_vbo.destroy();
    m_glInitialized = false;
}

// Sube el std::vector entero a la GPU de golpe
void PlotSeries::updateVBO()
{
    /*std::lock_guard<std::mutex> lg(m_mutex);
    if (m_points.empty() || !m_glInitialized)
        return;

    m_vbo.bind();
    // glBufferData nativo a través del wrapper de Qt:
    m_vbo.allocate(m_points.data(), static_cast<int>(m_points.size() * sizeof(Point)));
    m_vbo.release();*/
}

void PlotSeries::syncWithGPU()
{
    std::lock_guard<std::mutex> lg(m_mutex);

    if (!m_glInitialized || m_points.empty())
        return;
    if (m_points.size() == m_pointsInGPU)
        return;

    size_t newPointsCount = m_points.size() - m_pointsInGPU;

    m_vbo.bind();

    int offsetBytes = static_cast<int>(m_pointsInGPU * sizeof(Point));
    int sizeBytes = static_cast<int>(newPointsCount * sizeof(Point));

    const Point *dataPtr = &m_points[m_pointsInGPU];

    m_vbo.write(offsetBytes, dataPtr, sizeBytes);

    m_vbo.release();

    m_pointsInGPU = m_points.size();
}

std::vector<PlotSeries::Point> &PlotSeries::getVisiblePoints(double xMin, double xMax, int targetWidth)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_visibleBuffer.clear();

    if (m_points.empty() || targetWidth <= 0)
        return m_visibleBuffer;

    const_cast<PlotSeries *>(this)->updateLodLevels();

    const auto &crudePoints = m_lodLevels[0].points;
    auto itStartCrude = std::lower_bound(crudePoints.begin(), crudePoints.end(), xMin,
                                         [](const Point &p, double val)
                                         { return p.x < val; });
    auto itEndCrude = std::upper_bound(crudePoints.begin(), crudePoints.end(), xMax,
                                       [](double val, const Point &p)
                                       { return val < p.x; });

    size_t totalVisibleCrude = std::distance(itStartCrude, itEndCrude);
    if (totalVisibleCrude == 0)
        return m_visibleBuffer;

    size_t selectedLevel = 0;
    double pointsPerPixel = static_cast<double>(totalVisibleCrude) / targetWidth;

    if (pointsPerPixel > 10000.0)
    {
        selectedLevel = 2;
    }
    else if (pointsPerPixel > 100.0)
    {
        selectedLevel = 1;
    }
    else
    {
        selectedLevel = 0;
    }

    // qDebug() << "Level: " << selectedLevel << " Visible: " << totalVisibleCrude << "Density: " << pointsPerPixel;

    const auto &sourcePoints = m_lodLevels[selectedLevel].points;

    auto itStart = std::lower_bound(sourcePoints.begin(), sourcePoints.end(), xMin,
                                    [](const Point &p, double val)
                                    { return p.x < val; });
    auto itEnd = std::upper_bound(sourcePoints.begin(), sourcePoints.end(), xMax,
                                  [](double val, const Point &p)
                                  { return val < p.x; });

    size_t totalVisible = std::distance(itStart, itEnd);
    if (totalVisible == 0)
        return m_visibleBuffer;

    int maxPointsThreshold = targetWidth * 2;
    if (totalVisible <= static_cast<size_t>(maxPointsThreshold))
    {
        m_visibleBuffer.assign(itStart, itEnd);
        return m_visibleBuffer;
    }

    if (m_visibleBuffer.capacity() < static_cast<size_t>(maxPointsThreshold))
    {
        m_visibleBuffer.reserve(maxPointsThreshold);
    }

    double xRange = xMax - xMin;
    if (xRange <= 0)
        return m_visibleBuffer;

    double inverseXRangeWithWidth = static_cast<double>(targetWidth) / xRange;

    struct BucketData
    {
        Point minPt;
        Point maxPt;
        bool assigned = false;
    };

    static std::vector<BucketData> buckets;
    if (buckets.size() != static_cast<size_t>(targetWidth))
    {
        buckets.resize(targetWidth);
    }
    for (int i = 0; i < targetWidth; ++i)
        buckets[i].assigned = false;

    for (auto it = itStart; it != itEnd; ++it)
    {

        int pixelIdx = static_cast<int>((it->x - xMin) * inverseXRangeWithWidth);

        if (pixelIdx < 0)
            pixelIdx = 0;
        if (pixelIdx >= targetWidth)
            pixelIdx = targetWidth - 1;

        BucketData &b = buckets[pixelIdx];
        if (!b.assigned)
        {
            b.minPt = *it;
            b.maxPt = *it;
            b.assigned = true;
        }
        else
        {
            if (it->y < b.minPt.y)
                b.minPt = *it;
            if (it->y > b.maxPt.y)
                b.maxPt = *it;
        }
    }

    for (int i = 0; i < targetWidth; ++i)
    {
        if (!buckets[i].assigned)
            continue;

        if (buckets[i].minPt.x < buckets[i].maxPt.x)
        {
            m_visibleBuffer.push_back(buckets[i].minPt);
            m_visibleBuffer.push_back(buckets[i].maxPt);
        }
        else
        {
            m_visibleBuffer.push_back(buckets[i].maxPt);
            m_visibleBuffer.push_back(buckets[i].minPt);
        }
    }

    return m_visibleBuffer;
}

void PlotSeries::updateLodLevels()
{
    auto &crude = m_lodLevels[0].points; // Asumo que m_points y crude tienen los mismos datos
    auto &lod1 = m_lodLevels[1].points;

    size_t startIdx = m_lodLevels[1].lastProcessedCrudeSize;
    size_t endIdx = crude.size() - (crude.size() % LOD_FACTOR);

    for (size_t i = startIdx; i < endIdx; i += LOD_FACTOR)
    {
        size_t minIdx = i;
        size_t maxIdx = i;

        BlockStats block;
        block.xStart = crude[i].x;
        block.xEnd = crude[i + LOD_FACTOR - 1].x;
        block.min = std::numeric_limits<double>::max();
        block.max = -std::numeric_limits<double>::max();
        block.sum = 0.0;
        block.sumSq = 0.0;
        block.count = LOD_FACTOR;

        for (size_t k = 0; k < LOD_FACTOR; ++k)
        {
            size_t currentIdx = i + k;
            double y = crude[currentIdx].y;

            if (y < block.min)
                block.min = y;
            if (y > block.max)
                block.max = y;
            block.sum += y;
            block.sumSq += (y * y);

            if (crude[currentIdx].y < crude[minIdx].y)
                minIdx = currentIdx;
            if (crude[currentIdx].y > crude[maxIdx].y)
                maxIdx = currentIdx;
        }

        if (minIdx < maxIdx)
        {
            lod1.push_back(crude[minIdx]);
            lod1.push_back(crude[maxIdx]);
        }
        else
        {
            lod1.push_back(crude[maxIdx]);
            lod1.push_back(crude[minIdx]);
        }

        m_lodLevels[1].stats.push_back(block);
    }

    m_lodLevels[1].lastProcessedCrudeSize = endIdx;

    auto &lod2 = m_lodLevels[2].points;
    size_t startIdx2 = m_lodLevels[2].lastProcessedCrudeSize;
    size_t endIdx2 = lod1.size() - (lod1.size() % LOD_FACTOR);

    for (size_t i = startIdx2; i < endIdx2; i += LOD_FACTOR)
    {
        size_t minIdx = i;
        size_t maxIdx = i;
        for (size_t k = 1; k < LOD_FACTOR; ++k)
        {
            if (lod1[i + k].y < lod1[minIdx].y)
                minIdx = i + k;
            if (lod1[i + k].y > lod1[maxIdx].y)
                maxIdx = i + k;
        }
        if (minIdx < maxIdx)
        {
            lod2.push_back(lod1[minIdx]);
            lod2.push_back(lod1[maxIdx]);
        }
        else
        {
            lod2.push_back(lod1[maxIdx]);
            lod2.push_back(lod1[minIdx]);
        }
    }
    m_lodLevels[2].lastProcessedCrudeSize = endIdx2;
}

PlotSeries::Point PlotSeries::getClosestPointToX(double xValue)
{

    if (m_lodLevels[0].points.empty())
        return Point{0.0, 0.0};

    auto it = std::lower_bound(m_lodLevels[0].points.begin(), m_lodLevels[0].points.end(), xValue,
                               [](const Point &pt, double val)
                               {
                                   return pt.x < val;
                               });

    if (it == m_lodLevels[0].points.begin())
        return m_lodLevels[0].points.front();
    if (it == m_lodLevels[0].points.end())
        return m_lodLevels[0].points.back();

    auto prevIt = std::prev(it);

    double distCurrent = std::abs(it->x - xValue);
    double distPrev = std::abs(prevIt->x - xValue);

    if (distPrev < distCurrent)
    {
        return *prevIt;
    }
    else
    {
        return *it;
    }
}

PlotSeries::IntervalStats PlotSeries::calculateIntervalStats(double xMin, double xMax)
{

    std::unique_lock<std::mutex> lck(m_mutex);

    IntervalStats res;
    if (xMin > xMax)
    {
        return res;
    }
    double totalSum = 0.0;
    double totalSumSq = 0.0;
    size_t totalCount = 0;

    const auto &lod1 = m_lodLevels[1];
    const auto &lod0Points = m_lodLevels[0].points;

    auto itStart = std::lower_bound(lod1.stats.begin(), lod1.stats.end(), xMin,
                                    [](const BlockStats &b, double val)
                                    { return b.xStart < val; });

    auto itEnd = std::upper_bound(lod1.stats.begin(), lod1.stats.end(), xMax,
                                  [](double val, const BlockStats &b)
                                  { return val < b.xEnd; });

    double exactLodXMin = xMax;
    double exactLodXMax = xMin;

    if (itStart < itEnd && itStart != lod1.stats.end())
    {
        exactLodXMin = itStart->xStart;

        for (auto it = itStart; it != itEnd; ++it)
        {
            if (it->min < res.min)
                res.min = it->min;
            if (it->max > res.max)
                res.max = it->max;
            totalSum += it->sum;
            totalSumSq += it->sumSq;
            totalCount += it->count;
            exactLodXMax = it->xEnd;
        }
    }

    auto ptStart = std::lower_bound(lod0Points.begin(), lod0Points.end(), xMin,
                                    [](const Point &pt, double val)
                                    { return pt.x < val; });
    auto ptEnd = std::upper_bound(lod0Points.begin(), lod0Points.end(), xMax,
                                  [](double val, const Point &pt)
                                  { return val < pt.x; });

    auto processPointRange = [&](auto start, auto end)
    {
        for (auto it = start; it != end; ++it)
        {
            if (it->y < res.min)
                res.min = it->y;
            if (it->y > res.max)
                res.max = it->y;
            totalSum += it->y;
            totalSumSq += it->y * it->y;
            totalCount++;
        }
    };

    if (exactLodXMin < exactLodXMax)
    {

        auto ptMidStart = std::lower_bound(ptStart, ptEnd, exactLodXMin,
                                           [](const Point &pt, double val)
                                           { return pt.x < val; });
        auto ptMidEnd = std::upper_bound(ptMidStart, ptEnd, exactLodXMax,
                                         [](double val, const Point &pt)
                                         { return val < pt.x; });

        processPointRange(ptStart, ptMidStart);
        processPointRange(ptMidEnd, ptEnd);
    }
    else
    {

        processPointRange(ptStart, ptEnd);
    }

    if (totalCount > 0)
    {
        res.mean = totalSum / totalCount;
        res.rms = std::sqrt(totalSumSq / totalCount);
    }

    return res;
}
#include "PlotSeries.h"

PlotSeries::PlotSeries(QString name, QColor color, size_t maxPoints)
    : m_maxPoints(maxPoints), m_name(std::move(name)), m_color(color)
{}

void PlotSeries::pushPoint(double x, double y)
{
    pushPoint({x, y});
}

void PlotSeries::pushPoint(const Point& p)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    if (m_points.size() >= m_maxPoints)
        m_points.pop_front();
    m_points.push_back(p);
    updateBounds(p);
}

void PlotSeries::clear()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_points.clear();
    m_xMin =  std::numeric_limits<double>::max();
    m_xMax = -std::numeric_limits<double>::max();
    m_yMin =  std::numeric_limits<double>::max();
    m_yMax = -std::numeric_limits<double>::max();
}

std::unique_lock<std::mutex> PlotSeries::lock() const
{
    return std::unique_lock<std::mutex>(m_mutex);
}

void PlotSeries::setMaxPoints(size_t n)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_maxPoints = n;
    while (m_points.size() > m_maxPoints)
        m_points.pop_front();
    recomputeBounds();
}

void PlotSeries::updateBounds(const Point& p)
{
    if (p.x < m_xMin) m_xMin = p.x;
    if (p.x > m_xMax) m_xMax = p.x;
    if (p.y < m_yMin) m_yMin = p.y;
    if (p.y > m_yMax) m_yMax = p.y;
}

void PlotSeries::recomputeBounds()
{
    m_xMin =  std::numeric_limits<double>::max();
    m_xMax = -std::numeric_limits<double>::max();
    m_yMin =  std::numeric_limits<double>::max();
    m_yMax = -std::numeric_limits<double>::max();
    for (const auto& p : m_points)
        updateBounds(p);
}

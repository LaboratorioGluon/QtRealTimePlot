#include "PlotView.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

void PlotView::setArea(const QRect& area)
{
    m_area = QRect(area.topLeft(), QSize(std::max(1, area.width()), std::max(1, area.height())));
}

PlotView::Region PlotView::regionAt(QPoint px) const
{
    if (m_area.contains(px))
        return Region::Plot;
    if (px.x() >= m_area.left() && px.x() <= m_area.right() && px.y() > m_area.bottom())
        return Region::XAxis;
    if (px.x() < m_area.left() && px.y() >= m_area.top() && px.y() <= m_area.bottom())
        return Region::YAxis;
    return Region::Outside;
}

void PlotView::enforceMinSpan(double& lo, double& hi)
{
    if (!std::isfinite(lo) || !std::isfinite(hi))
    {
        lo = 0.0;
        hi = 1.0;
        return;
    }
    if (hi < lo)
        std::swap(lo, hi);

    // Below ~1e-12 relative span, doubles can no longer resolve pixels.
    const double magnitude = std::max(std::abs(lo), std::abs(hi));
    const double minSpan   = std::max(magnitude * 1e-12, std::numeric_limits<double>::min() * 1e6);
    if (hi - lo < minSpan)
    {
        const double center = lo + (hi - lo) / 2.0;
        lo                  = center - minSpan / 2.0;
        hi                  = center + minSpan / 2.0;
    }
}

void PlotView::setRange(double xMin, double xMax, double yMin, double yMax)
{
    enforceMinSpan(xMin, xMax);
    enforceMinSpan(yMin, yMax);
    m_xMin = xMin;
    m_xMax = xMax;
    m_yMin = yMin;
    m_yMax = yMax;
}

void PlotView::fit(double xMin, double xMax, double yMin, double yMax, double xPadFraction, double yPadFraction)
{
    // A flat range gets padding relative to its own magnitude, so a constant 1e-20
    // signal is not squashed into a view one unit tall. Exactly zero falls back to 0.5.
    auto padFor = [](double lo, double hi, double fraction) {
        const double pad = (hi - lo) * fraction;
        if (pad > 0.0)
            return pad;
        const double magnitude = std::max(std::abs(lo), std::abs(hi));
        return magnitude > 0.0 ? magnitude * 0.05 : 0.5;
    };
    const double dx = padFor(xMin, xMax, xPadFraction);
    const double dy = padFor(yMin, yMax, yPadFraction);
    setRange(xMin - dx, xMax + dx, yMin - dy, yMax + dy);
}

QPointF PlotView::pixelToData(QPoint px) const
{
    const double fx = static_cast<double>(px.x() - m_area.left()) / m_area.width();
    const double fy = static_cast<double>(px.y() - m_area.top()) / m_area.height();
    return {m_xMin + fx * (m_xMax - m_xMin), m_yMax - fy * (m_yMax - m_yMin)};  // Y is flipped
}

QPoint PlotView::dataToPixel(double x, double y) const
{
    // Clamp far off-screen values so the int conversion cannot overflow.
    const double fx = std::clamp(normX(x), -1e6, 1e6);
    const double fy = std::clamp(normY(y), -1e6, 1e6);
    return {m_area.left() + static_cast<int>(fx * m_area.width()),
            m_area.bottom() - static_cast<int>(fy * m_area.height())};
}

double PlotView::pixelToX(int px) const
{
    return pixelToData(QPoint(px, m_area.top())).x();
}

int PlotView::xToPixel(double x) const
{
    return dataToPixel(x, m_yMin).x();
}

void PlotView::zoom(double factor, QPoint anchor, Axes axes)
{
    if (!(factor > 0.0))
        return;

    const QPointF a  = pixelToData(anchor);
    double        x0 = m_xMin, x1 = m_xMax, y0 = m_yMin, y1 = m_yMax;
    if (axes & AxisX)
    {
        x0 = a.x() - (a.x() - m_xMin) / factor;
        x1 = a.x() + (m_xMax - a.x()) / factor;
    }
    if (axes & AxisY)
    {
        y0 = a.y() - (a.y() - m_yMin) / factor;
        y1 = a.y() + (m_yMax - a.y()) / factor;
    }
    setRange(x0, x1, y0, y1);
}

void PlotView::pan(QPoint deltaPx, Axes axes)
{
    const double dx = (axes & AxisX) ? -deltaPx.x() * (m_xMax - m_xMin) / m_area.width() : 0.0;
    const double dy = (axes & AxisY) ? deltaPx.y() * (m_yMax - m_yMin) / m_area.height() : 0.0;
    setRange(m_xMin + dx, m_xMax + dx, m_yMin + dy, m_yMax + dy);
}

void PlotView::zoomToPixelRect(const QRect& rect, Axes axes)
{
    const QRect   r  = rect.normalized();
    const QPointF tl = pixelToData(r.topLeft());
    const QPointF br = pixelToData(r.bottomRight());

    double x0 = m_xMin, x1 = m_xMax, y0 = m_yMin, y1 = m_yMax;
    if (axes & AxisX)
    {
        x0 = std::min(tl.x(), br.x());
        x1 = std::max(tl.x(), br.x());
    }
    if (axes & AxisY)
    {
        y0 = std::min(tl.y(), br.y());
        y1 = std::max(tl.y(), br.y());
    }
    setRange(x0, x1, y0, y1);
}

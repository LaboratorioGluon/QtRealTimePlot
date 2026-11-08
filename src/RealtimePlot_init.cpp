#include "RealtimePlot.h"

#include <algorithm>
#include <cmath>
#include <limits>

const std::vector<QColor> RealtimePlot::colorList = {Qt::red, Qt::yellow, Qt::green};

// ==========================================================================
RealtimePlot::RealtimePlot(QWidget* parent)
    : QOpenGLWidget(parent),
      m_timer(new QTimer(this)),
      m_cursorRange(1, QColor(255, 200, 50, 200), RtpCursor::MarkerStyle::MARKER_SIMPLE)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_tickFont  = QFont("Monospace", 12);
    m_labelFont = QFont("Sans", 14, QFont::Bold);

    // Only repaint when some series changed since the last frame.
    connect(m_timer, &QTimer::timeout, this, [this]() {
        if (dataVersion() != m_paintedVersion)
            update();
    });

    m_legend.setSeriesSource(&m_series);
    m_cursorRange.setSeriesSource(&m_series);

    // set disabled by default.
    m_cursorRange.enable(false);
}

RealtimePlot::~RealtimePlot()
{
    cleanupGL();
}

// --------------------------------------------------------------------------
void RealtimePlot::setRefreshRate(int fps)
{
    m_timer->setInterval(fps > 0 ? 1000 / fps : 16);
}

void RealtimePlot::start()
{
    m_timer->start();
}
void RealtimePlot::stop()
{
    m_timer->stop();
}

uint64_t RealtimePlot::dataVersion() const
{
    uint64_t v = m_series.size();
    for (const auto& s : m_series)
        v += s->version();
    return v;
}

// --------------------------------------------------------------------------
std::shared_ptr<PlotSeries> RealtimePlot::addSeries(const QString& name, QColor color, size_t maxPoints)
{
    auto s = std::make_shared<PlotSeries>(name, color, maxPoints);
    addSeries(s);
    return s;
}

void RealtimePlot::addSeries(std::shared_ptr<PlotSeries> serie)
{
    if (!serie)
        return;
    m_series.push_back(std::move(serie));
    update();
}

void RealtimePlot::removeSeries(const std::shared_ptr<PlotSeries>& series)
{
    // GPU buffers of removed series are released at the next paint, with the context current.
    m_series.erase(std::remove(m_series.begin(), m_series.end(), series), m_series.end());
    update();
}

void RealtimePlot::clearSeriesData()
{
    for (auto& s : m_series)
        s->clear();
    update();
}

void RealtimePlot::clearSeries()
{
    m_series.clear();
    update();
}

// --------------------------------------------------------------------------
void RealtimePlot::addCursor(RtpCursor::MarkerStyle style)
{
    addCursor(m_xMin + (m_xMax - m_xMin) / 2.0, style);
}

void RealtimePlot::addCursor(double xPos, RtpCursor::MarkerStyle style)
{
    addCursor(xPos, QColor(Qt::green), style);
}

void RealtimePlot::addCursor(double xPos, QColor color, RtpCursor::MarkerStyle style)
{
    if (!color.isValid() || color.alpha() == 0)
        color = colorList[m_cursors.size() % colorList.size()];

    m_cursors.emplace_back(static_cast<int>(m_cursors.size()), color, style);
    RtpCursor& c = m_cursors.back();
    c.setSeriesSource(&m_series);
    c.setUnit(m_cursorUnit);
    c.setPos(xPos);
    update();
}

void RealtimePlot::setCursorUnit(const QString& unit)
{
    m_cursorUnit = unit;
    for (auto& c : m_cursors)
        c.setUnit(unit);
    update();
}

// --------------------------------------------------------------------------
void RealtimePlot::enforceMinSpan(double& lo, double& hi)
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

void RealtimePlot::setViewRange(double xMin, double xMax, double yMin, double yMax)
{
    enforceMinSpan(xMin, xMax);
    enforceMinSpan(yMin, yMax);
    m_xMin            = xMin;
    m_xMax            = xMax;
    m_yMin            = yMin;
    m_yMax            = yMax;
    m_viewInitialized = true;
    emitViewChanged();
    if (!m_inPaint)
        update();
}

QRectF RealtimePlot::viewRange() const
{
    return QRectF(m_xMin, m_yMin, m_xMax - m_xMin, m_yMax - m_yMin);
}

bool RealtimePlot::dataBounds(double& xMin, double& xMax, double& yMin, double& yMax) const
{
    xMin = yMin = std::numeric_limits<double>::max();
    xMax = yMax = -std::numeric_limits<double>::max();
    bool any    = false;

    for (const auto& s : m_series)
    {
        if (!s->visible())
            continue;
        auto lk = s->lock();
        if (s->points().empty())
            continue;
        xMin = std::min(xMin, s->xMin());
        xMax = std::max(xMax, s->xMax());
        yMin = std::min(yMin, s->yMin());
        yMax = std::max(yMax, s->yMax());
        any  = true;
    }
    return any;
}

void RealtimePlot::autoFitRolling()
{
    double xMin, xMax, yMin, yMax;
    if (!dataBounds(xMin, xMax, yMin, yMax))
        return;

    // 1 % padding of the window on each side in X, 5 % in Y.
    double dx = m_zoomWindow * 0.01;
    double dy = (yMax - yMin) * 0.05;
    if (dx == 0)
        dx = 0.5;
    if (dy == 0)
        dy = 0.5;

    setViewRange(xMax - m_zoomWindow - dx, xMax + dx, yMin - dy, yMax + dy);
}

void RealtimePlot::autoFit()
{
    double xMin, xMax, yMin, yMax;
    if (!dataBounds(xMin, xMax, yMin, yMax))
        return;

    // 5 % padding on each side
    double dx = (xMax - xMin) * 0.05;
    double dy = (yMax - yMin) * 0.05;
    if (dx == 0)
        dx = 0.5;
    if (dy == 0)
        dy = 0.5;

    setViewRange(xMin - dx, xMax + dx, yMin - dy, yMax + dy);
}

// --------------------------------------------------------------------------
// Coordinate conversion helpers
// --------------------------------------------------------------------------
QRect RealtimePlot::plotArea() const
{
    return QRect(m_margin.left, m_margin.top, std::max(1, width() - m_margin.left - m_margin.right),
                 std::max(1, height() - m_margin.top - m_margin.bottom));
}

QPointF RealtimePlot::pixelToData(QPoint px) const
{
    QRect  area = plotArea();
    double fx   = static_cast<double>(px.x() - area.left()) / area.width();
    double fy   = static_cast<double>(px.y() - area.top()) / area.height();
    return {m_xMin + fx * (m_xMax - m_xMin), m_yMax - fy * (m_yMax - m_yMin)};  // Y is flipped
}

QPoint RealtimePlot::dataToPixel(double x, double y) const
{
    QRect  area = plotArea();
    double fx   = (x - m_xMin) / (m_xMax - m_xMin);
    double fy   = (y - m_yMin) / (m_yMax - m_yMin);
    // Clamp far off-screen values so the int conversion cannot overflow.
    fx = std::clamp(fx, -1e6, 1e6);
    fy = std::clamp(fy, -1e6, 1e6);
    return {area.left() + static_cast<int>(fx * area.width()), area.bottom() - static_cast<int>(fy * area.height())};
}

void RealtimePlot::emitViewChanged()
{
    emit viewChanged(m_xMin, m_xMax, m_yMin, m_yMax);
}

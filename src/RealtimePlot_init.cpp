#include "RealtimePlot.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QApplication>
#include <QtMath>
#include <algorithm>
#include <cmath>

// --------------------------------------------------------------------------
// Vertex / fragment shaders (GLSL 1.20 – compatible with GL 2.1 and ES 2.0)
// --------------------------------------------------------------------------
static const char *kVertSrc = R"GLSL(
    attribute vec2 a_position;
    uniform   vec2 u_scale;
    uniform   vec2 u_offset;
    void main() {
        vec2 ndc = (a_position + u_offset) * u_scale * vec2(2.0, 2.0) - vec2(1.0, 1.0);
        gl_Position = vec4(ndc, 0.0, 1.0);
    }
)GLSL";

static const char *kFragSrc = R"GLSL(
    uniform vec4 u_color;
    void main() {
        gl_FragColor = u_color;
    }
)GLSL";

// ==========================================================================
RealtimePlot::RealtimePlot(QWidget *parent)
    : QOpenGLWidget(parent), m_timer(new QTimer(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_tickFont = QFont("Monospace", 12);
    m_labelFont = QFont("Sans", 9, QFont::Bold);
    m_cursorFont = QFont("Monospace", 12);

    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&RealtimePlot::update));

    m_legend.setSeriesSource(&m_series);
}

RealtimePlot::~RealtimePlot()
{
    makeCurrent();
    delete m_shader;
    doneCurrent();
}

// --------------------------------------------------------------------------
void RealtimePlot::setRefreshRate(int fps)
{
    m_timer->setInterval(fps > 0 ? 1000 / fps : 16);
}

void RealtimePlot::start() { m_timer->start(); }
void RealtimePlot::stop() { m_timer->stop(); }

// --------------------------------------------------------------------------
std::shared_ptr<PlotSeries> RealtimePlot::addSeries(const QString &name,
                                                    QColor color,
                                                    size_t maxPoints)
{
    auto s = std::make_shared<PlotSeries>(name, color);
    addSeries(s);
    return s;
}

void RealtimePlot::addSeries(std::shared_ptr<PlotSeries> serie)
{
    m_series.push_back(serie);
}

void RealtimePlot::removeSeries(const std::shared_ptr<PlotSeries> &series)
{
    m_series.erase(std::remove(m_series.begin(), m_series.end(), series),
                   m_series.end());
}

void RealtimePlot::clearSeries()
{
    m_series.clear();
}

// --------------------------------------------------------------------------
void RealtimePlot::setViewRange(double xMin, double xMax, double yMin, double yMax)
{
    m_xMin = xMin;
    m_xMax = xMax;
    m_yMin = yMin;
    m_yMax = yMax;
    m_viewInitialized = true;
    emitViewChanged();
}

QRectF RealtimePlot::viewRange() const
{
    return QRectF(m_xMin, m_yMin, m_xMax - m_xMin, m_yMax - m_yMin);
}

void RealtimePlot::autoFit()
{
    double xMin = 1e300, xMax = -1e300;
    double yMin = 1e300, yMax = -1e300;
    bool any = false;

    for (const auto &s : m_series)
    {
        if (!s->visible() || s->points().empty())
            continue;
        auto lk = s->lock();
        xMin = std::min(xMin, s->xMin());
        xMax = std::max(xMax, s->xMax());
        yMin = std::min(yMin, s->yMin());
        yMax = std::max(yMax, s->yMax());
        any = true;
    }

    if (!any)
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
    return QRect(m_margin.left,
                 m_margin.top,
                 width() - m_margin.left - m_margin.right,
                 height() - m_margin.top - m_margin.bottom);
}

QPointF RealtimePlot::pixelToData(QPoint px) const
{
    QRect area = plotArea();
    double fx = static_cast<double>(px.x() - area.left()) / area.width();
    double fy = static_cast<double>(px.y() - area.top()) / area.height();
    return {m_xMin + fx * (m_xMax - m_xMin),
            m_yMax - fy * (m_yMax - m_yMin)}; // Y is flipped
}

QPoint RealtimePlot::dataToPixel(double x, double y) const
{
    QRect area = plotArea();
    double fx = (x - m_xMin) / (m_xMax - m_xMin);
    double fy = (y - m_yMin) / (m_yMax - m_yMin);
    return {area.left() + static_cast<int>(fx * area.width()),
            area.bottom() - static_cast<int>(fy * area.height())};
}

void RealtimePlot::emitViewChanged()
{
    emit viewChanged(m_xMin, m_xMax, m_yMin, m_yMax);
}

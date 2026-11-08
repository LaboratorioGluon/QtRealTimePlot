#include "RealtimePlot.h"

#include <QOpenGLContext>
#include <QPainter>
#include <QResizeEvent>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>

#include "PlotRenderer.h"

const std::vector<QColor> RealtimePlot::colorList = {Qt::red, Qt::yellow, Qt::green};

// ==========================================================================
//  Lifecycle
// ==========================================================================
RealtimePlot::RealtimePlot(QWidget* parent)
    : QOpenGLWidget(parent),
      m_timer(new QTimer(this)),
      m_renderer(std::make_unique<rtp::PlotRenderer>()),
      m_cursorRange(1, QColor(255, 200, 50, 200), PlotCursor::MarkerStyle::MARKER_SIMPLE)
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

    m_legend.setOnToggled([this](int) {
        emit visibilityChanged();
        update();
    });
    m_cursorRange.enable(false);
    syncViewArea();
}

RealtimePlot::~RealtimePlot()
{
    cleanupGL();
}

void RealtimePlot::initializeGL()
{
    // Called again whenever Qt recreates the context (e.g. after reparenting).
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &RealtimePlot::cleanupGL, Qt::UniqueConnection);
    m_renderer->initialize(context());
}

void RealtimePlot::cleanupGL()
{
    if (!m_renderer || !m_renderer->isInitialized())
        return;
    makeCurrent();
    m_renderer->cleanup();
    doneCurrent();
    if (context())
        disconnect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &RealtimePlot::cleanupGL);
}

void RealtimePlot::resizeGL(int /*w*/, int /*h*/)
{
    // QOpenGLWidget handles glViewport automatically
}

void RealtimePlot::resizeEvent(QResizeEvent* event)
{
    QOpenGLWidget::resizeEvent(event);
    syncViewArea();
}

// ==========================================================================
//  Refresh
// ==========================================================================
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

void RealtimePlot::viewModified()
{
    emit viewChanged(m_view.xMin(), m_view.xMax(), m_view.yMin(), m_view.yMax());
    if (!m_inPaint)
        update();
}

// ==========================================================================
//  Series
// ==========================================================================
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
    // The renderer releases GPU buffers of removed series at the next paint, with the context current.
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

// ==========================================================================
//  Cursors and overlays
// ==========================================================================
void RealtimePlot::addCursor(PlotCursor::MarkerStyle style)
{
    addCursor(m_view.xMin() + (m_view.xMax() - m_view.xMin()) / 2.0, style);
}

void RealtimePlot::addCursor(double xPos, PlotCursor::MarkerStyle style)
{
    addCursor(xPos, QColor(Qt::green), style);
}

void RealtimePlot::addCursor(double xPos, QColor color, PlotCursor::MarkerStyle style)
{
    if (!color.isValid() || color.alpha() == 0)
        color = colorList[m_cursors.size() % colorList.size()];

    m_cursors.emplace_back(static_cast<int>(m_cursors.size()), color, style);
    PlotCursor& c = m_cursors.back();
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

void RealtimePlot::setRangeCursorEnable(bool enable)
{
    m_cursorRange.enable(enable);
    update();
}

void RealtimePlot::setRangeCursorPos(double start, double end)
{
    m_cursorRange.setPosStart(start);
    m_cursorRange.setPosEnd(end);
    update();
}

std::vector<PlotOverlay*> RealtimePlot::overlays()
{
    // Bottom to top. The first cursor added is drawn on top of later ones, and the legend above all.
    std::vector<PlotOverlay*> list;
    list.reserve(m_cursors.size() + 2);
    list.push_back(&m_cursorRange);
    for (auto it = m_cursors.rbegin(); it != m_cursors.rend(); ++it)
        list.push_back(&*it);
    list.push_back(&m_legend);
    return list;
}

// ==========================================================================
//  Configuration
// ==========================================================================
void RealtimePlot::setGrid(const GridStyle& g)
{
    m_grid = g;
    update();
}

void RealtimePlot::setMargins(const MarginPx& m)
{
    m_margin = m;
    syncViewArea();
    update();
}

void RealtimePlot::setBackgroundColor(QColor c)
{
    m_bgColor = c;
    update();
}

void RealtimePlot::setAxisColor(QColor c)
{
    m_axisColor = c;
    update();
}

void RealtimePlot::setTextColor(QColor c)
{
    m_textColor = c;
    update();
}

void RealtimePlot::setTitle(const QString& t)
{
    m_title = t;
    update();
}

void RealtimePlot::setXLabel(const QString& l)
{
    m_xAxis.setLabel(l);
    update();
}

void RealtimePlot::setYLabel(const QString& l)
{
    m_yAxis.setLabel(l);
    update();
}

void RealtimePlot::setLegendVisible(bool v)
{
    m_legend.enable(v);
    update();
}

void RealtimePlot::setAutoZoom(bool a, double window)
{
    m_zoomAuto   = a;
    m_zoomWindow = window > 0.0 ? window : 0.0;
    update();
}

// ==========================================================================
//  View
// ==========================================================================
QRect RealtimePlot::plotArea() const
{
    return QRect(m_margin.left, m_margin.top, width() - m_margin.left - m_margin.right,
                 height() - m_margin.top - m_margin.bottom);
}

void RealtimePlot::syncViewArea()
{
    m_view.setArea(plotArea());
}

PlotView::Axes RealtimePlot::zoomAxes() const
{
    switch (m_zoomMode)
    {
        case ZoomMode::XOnly:
            return PlotView::AxisX;
        case ZoomMode::YOnly:
            return PlotView::AxisY;
        default:
            return PlotView::AxisBoth;
    }
}

void RealtimePlot::setViewRange(double xMin, double xMax, double yMin, double yMax)
{
    m_view.setRange(xMin, xMax, yMin, yMax);
    m_viewInitialized = true;
    viewModified();
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

void RealtimePlot::autoFit()
{
    double xMin, xMax, yMin, yMax;
    if (!dataBounds(xMin, xMax, yMin, yMax))
        return;
    m_view.fit(xMin, xMax, yMin, yMax, 0.05, 0.05);
    m_viewInitialized = true;
    viewModified();
}

void RealtimePlot::autoFitRolling()
{
    double xMin, xMax, yMin, yMax;
    if (!dataBounds(xMin, xMax, yMin, yMax))
        return;
    // 1 % of the window as padding on each side in X, 5 % in Y.
    m_view.fit(xMax - m_zoomWindow, xMax, yMin, yMax, 0.01, 0.05);
    m_viewInitialized = true;
    viewModified();
}

// ==========================================================================
//  Paint
// ==========================================================================
void RealtimePlot::paintGL()
{
#ifdef RTP_ENABLE_DEBUG
    const auto startTimer = std::chrono::high_resolution_clock::now();
#endif
    m_inPaint        = true;
    m_paintedVersion = dataVersion();
    syncViewArea();

    if (m_zoomAuto)
    {
        if (m_zoomWindow > 0.0)
            autoFitRolling();
        else
            autoFit();
    }
    if (!m_viewInitialized)
        autoFit();

    const QRect area   = m_view.area();
    const auto  xTicks = m_xAxis.ticks(m_view.xMin(), m_view.xMax(), area.width());
    const auto  yTicks = m_yAxis.ticks(m_view.yMin(), m_view.yMax(), area.height());

    m_renderer->beginFrame(size(), static_cast<float>(devicePixelRatioF()), m_view, m_bgColor);
    if (m_grid.enabled)
        m_renderer->drawGrid(m_view, xTicks.ticks, yTicks.ticks, m_grid.color);
    m_renderer->drawSeries(m_view, m_series);
    m_renderer->drawFrame(m_view, xTicks.ticks, yTicks.ticks, m_axisColor);
    m_renderer->endFrame();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    paintLabels(painter, xTicks, yTicks);

    const PlotContext ctx = overlayContext();
    for (PlotOverlay* o : overlays())
        o->draw(painter, ctx);

    paintSelection(painter);
    painter.end();

    m_inPaint = false;

#ifdef RTP_ENABLE_DEBUG
    const std::chrono::duration<double, std::milli> elapsed    = std::chrono::high_resolution_clock::now() - startTimer;
    static int                                      frameCount = 0;
    if (++frameCount % 30 == 0)
        qDebug() << "paintGL():" << elapsed.count() << "ms";
#endif
}

void RealtimePlot::paintLabels(QPainter& painter, const PlotAxis::TickSet& xTicks, const PlotAxis::TickSet& yTicks)
{
    const QRect area = m_view.area();

    painter.setPen(m_textColor);
    painter.setFont(m_tickFont);
    for (const auto& t : xTicks.ticks)
    {
        const QPoint px = m_view.dataToPixel(t.value, m_view.yMin());
        painter.drawText(QRect(px.x() - 30, px.y() + 5, 60, 16), Qt::AlignHCenter | Qt::AlignTop, t.label);
    }
    for (const auto& t : yTicks.ticks)
    {
        const QPoint py = m_view.dataToPixel(m_view.xMin(), t.value);
        painter.drawText(QRect(2, py.y() - 8, m_margin.left - 6, 16), Qt::AlignRight | Qt::AlignVCenter, t.label);
    }

    // Shared multiplier / offset, shown once per axis: X at the bottom right, Y above the axis.
    if (!xTicks.scaleLabel.isEmpty())
        painter.drawText(QRect(area.left(), height() - m_margin.bottom + 22, area.width(), m_margin.bottom - 22),
                         Qt::AlignRight | Qt::AlignTop, xTicks.scaleLabel);
    if (!yTicks.scaleLabel.isEmpty())
        painter.drawText(QRect(2, 0, width() / 2, area.top() - 10), Qt::AlignLeft | Qt::AlignBottom,
                         yTicks.scaleLabel);  // clear of the top tick label, which is centred on area.top()

    painter.setFont(m_labelFont);
    if (!m_title.isEmpty())
        painter.drawText(QRect(0, 2, width(), m_margin.top - 2), Qt::AlignHCenter | Qt::AlignBottom, m_title);

    if (!m_xAxis.label().isEmpty())
        painter.drawText(QRect(area.left(), height() - m_margin.bottom + 22, area.width(), m_margin.bottom - 22),
                         Qt::AlignHCenter | Qt::AlignTop, m_xAxis.label());

    if (!m_yAxis.label().isEmpty())
    {
        painter.save();
        painter.translate(14, area.top() + area.height() / 2);
        painter.rotate(-90);
        painter.drawText(QRect(-60, -10, 120, 20), Qt::AlignHCenter | Qt::AlignVCenter, m_yAxis.label());
        painter.restore();
    }
}

void RealtimePlot::paintSelection(QPainter& painter)
{
    if (m_interaction.state != Interaction::State::BoxSelecting)
        return;

    const QRect sel = QRect(m_interaction.selStart, m_interaction.selEnd).normalized();
    if (sel.width() <= 2 || sel.height() <= 2)
        return;

    painter.setPen(QPen(QColor(255, 200, 50, 200), 1, Qt::DashLine));
    painter.setBrush(QColor(255, 200, 50, 30));
    painter.drawRect(sel);

    if (sel.width() > 30)
    {
        const QPointF tl = m_view.pixelToData(sel.topLeft());
        const QPointF br = m_view.pixelToData(sel.bottomRight());
        const QString hint =
            QString("Δx=%1  Δy=%2").arg(std::abs(br.x() - tl.x()), 0, 'g', 4).arg(std::abs(br.y() - tl.y()), 0, 'g', 4);
        painter.setFont(m_tickFont);
        painter.setPen(QColor(255, 200, 50));
        painter.drawText(sel.bottomLeft() + QPoint(4, -4), hint);
    }
}

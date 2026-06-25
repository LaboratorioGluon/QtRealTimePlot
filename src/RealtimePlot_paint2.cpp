#include "RealtimePlot.h"
#include <QPainter>
#include <QOpenGLShaderProgram>
#include <algorithm>
#include <cmath>

// ==========================================================================
//  Vertex / Fragment shaders
//  Expects pixel-space coordinates. Transforms to NDC internally.
//  Y axis: pixel 0 = top, so we negate.
// ==========================================================================
static const char *kVertSrc = R"GLSL(
    attribute vec2 a_position;
    uniform   vec2 u_viewport;   // (width, height) in pixels
    void main() {
        float ndcX =  (a_position.x / u_viewport.x) * 2.0 - 1.0;
        float ndcY = -((a_position.y / u_viewport.y) * 2.0 - 1.0);
        gl_Position = vec4(ndcX, ndcY, 0.0, 1.0);
    }
)GLSL";

static const char *kFragSrc = R"GLSL(
    uniform vec4 u_color;
    void main() {
        gl_FragColor = u_color;
    }
)GLSL";

// ==========================================================================
//  OpenGL lifecycle
// ==========================================================================
void RealtimePlot::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(m_bgColor.redF(), m_bgColor.greenF(),
                 m_bgColor.blueF(), 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    m_shader = new QOpenGLShaderProgram(this);
    if (m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, kVertSrc) &&
        m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, kFragSrc) &&
        m_shader->link())
    {
        m_shaderReady = true;
    }
}

void RealtimePlot::resizeGL(int /*w*/, int /*h*/)
{
    // QOpenGLWidget handles glViewport automatically
}

// ==========================================================================
//  Main paint
// ==========================================================================
void RealtimePlot::paintGL()
{
    // Auto-scroll: shift X window to follow newest data
    if (m_autoScroll && !m_series.empty())
    {
        double xNewest = -1e300;
        for (const auto &s : m_series)
        {
            if (s->visible() && !s->points().empty())
            {
                auto lk = s->lock();
                xNewest = std::max(xNewest, s->xMax());
            }
        }
        if (xNewest > -1e300)
        {
            m_xMax = xNewest;
            m_xMin = xNewest - m_scrollWindow;
        }
    }

    // Auto-init view on first paint
    if (!m_viewInitialized)
    {
        autoFit();
        m_viewInitialized = true;
    }

    // Compute ticks once per frame
    QRect area = plotArea();
    auto xTicks = PlotAxis::computeTicks(m_xMin, m_xMax, std::max(3, area.width() / 80));
    auto yTicks = PlotAxis::computeTicks(m_yMin, m_yMax, std::max(3, area.height() / 50));

    // Clear
    glClearColor(m_bgColor.redF(), m_bgColor.greenF(), m_bgColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawGrid(area, xTicks, yTicks);
    drawSeries(area);
    drawAxes(area, xTicks, yTicks);

    // QPainter pass for text / overlays
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.setPen(m_textColor);
    painter.setFont(m_tickFont);

    // X-axis tick labels
    for (const auto &t : xTicks)
    {
        QPoint px = dataToPixel(t.value, m_yMin);
        QRect r(px.x() - 30, px.y() + 5, 60, 16);
        painter.drawText(r, Qt::AlignHCenter | Qt::AlignTop, t.label);
    }
    // Y-axis tick labels
    for (const auto &t : yTicks)
    {
        QPoint py = dataToPixel(m_xMin, t.value);
        QRect r(2, py.y() - 8, m_margin.left - 6, 16);
        painter.drawText(r, Qt::AlignRight | Qt::AlignVCenter, t.label);
    }

    // Title
    if (!m_title.isEmpty())
    {
        painter.setFont(m_labelFont);
        painter.drawText(QRect(0, 2, width(), m_margin.top - 2),
                         Qt::AlignHCenter | Qt::AlignBottom, m_title);
    }

    // X label
    if (!m_xLabel.isEmpty())
    {
        painter.setFont(m_labelFont);
        painter.drawText(QRect(m_margin.left, height() - m_margin.bottom + 22,
                               area.width(), m_margin.bottom - 22),
                         Qt::AlignHCenter | Qt::AlignTop, m_xLabel);
    }

    // Y label (rotated)
    if (!m_yLabel.isEmpty())
    {
        painter.setFont(m_labelFont);
        painter.save();
        painter.translate(14, m_margin.top + area.height() / 2);
        painter.rotate(-90);
        painter.drawText(QRect(-60, -10, 120, 20),
                         Qt::AlignHCenter | Qt::AlignVCenter, m_yLabel);
        painter.restore();
    }

    // Legend
    if (m_legendVisible)
        drawLegend(area, painter);

    // Box-zoom selection rectangle
    if (m_selecting)
    {
        QRect sel = QRect(m_selStart, m_selEnd).normalized();
        if (!sel.isEmpty() && sel.width() > 2 && sel.height() > 2)
        {

            painter.setPen(QPen(QColor(255, 200, 50, 200), 1, Qt::DashLine));
            painter.setBrush(QColor(255, 200, 50, 30));
            painter.drawRect(sel);

            // Show selected data range
            if (sel.width() > 30)
            {
                QPointF tl = pixelToData(sel.topLeft());
                QPointF br = pixelToData(sel.bottomRight());
                QString hint = QString("Δx=%1  Δy=%2")
                                   .arg(std::abs(br.x() - tl.x()), 0, 'g', 4)
                                   .arg(std::abs(br.y() - tl.y()), 0, 'g', 4);
                painter.setFont(m_tickFont);
                painter.setPen(QColor(255, 200, 50));
                painter.drawText(sel.bottomLeft() + QPoint(4, -4), hint);
            }
        }
    }

    painter.end();
}

// ==========================================================================
//  Grid
// ==========================================================================
void RealtimePlot::drawGrid(const QRect &area,
                            const QVector<PlotAxis::Tick> &xTicks,
                            const QVector<PlotAxis::Tick> &yTicks)
{
    if (!m_grid.enabled)
        return;

    const float dpr = static_cast<float>(devicePixelRatioF());

    std::vector<float> verts;
    verts.reserve((xTicks.size() + yTicks.size()) * 4);

    for (const auto &t : xTicks)
    {
        float px = (float)dataToPixel(t.value, 0).x();
        verts.push_back(px * dpr);
        verts.push_back((float)area.top() * dpr);
        verts.push_back(px * dpr);
        verts.push_back((float)area.bottom() * dpr);
    }
    for (const auto &t : yTicks)
    {
        float py = (float)dataToPixel(0, t.value).y();
        verts.push_back((float)area.left() * dpr);
        verts.push_back(py * dpr);
        verts.push_back((float)area.right() * dpr);
        verts.push_back(py * dpr);
    }

    // Grid lines are line SEGMENTS, not a strip; draw as GL_LINES
    drawLines(verts, m_grid.color, 1.0f * dpr);
}

// ==========================================================================
//  Series rendering  (data → pixel conversion on CPU)
// ==========================================================================
void RealtimePlot::drawSeries(const QRect &area)
{
    glEnable(GL_SCISSOR_TEST);
    const float dpr = static_cast<float>(devicePixelRatioF());
    // OpenGL Y is bottom-up; area.bottom() is the largest Y pixel (bottom of rect)
    glScissor(area.left() * dpr,
              (height() - area.bottom() - 1) * dpr,
              area.width() * dpr,
              area.height() * dpr);

    for (const auto &s : m_series)
    {
        if (!s->visible())
            continue;
        auto lk = s->lock();
        const auto &pts = s->points();
        if (pts.size() < 2)
            continue;

        // Convert data → pixel space on CPU
        std::vector<float> verts;
        verts.reserve(pts.size() * 2);
        for (const auto &p : pts)
        {
            QPoint px = dataToPixel(p.x, p.y);
            verts.push_back((float)px.x() * dpr);
            verts.push_back((float)px.y() * dpr);
        }

        drawLineStrip(verts, s->color(), s->lineWidth());
    }

    glDisable(GL_SCISSOR_TEST);
}

// ==========================================================================
//  Axes border + tick marks
// ==========================================================================
void RealtimePlot::drawAxes(const QRect &area,
                            const QVector<PlotAxis::Tick> &xTicks,
                            const QVector<PlotAxis::Tick> &yTicks)
{

    const float dpr = static_cast<float>(devicePixelRatioF());
    // Plot border
    std::vector<float> border = {
        (float)area.left() * dpr, (float)area.top() * dpr,
        (float)area.right() * dpr, (float)area.top() * dpr,
        (float)area.right() * dpr, (float)area.bottom() * dpr,
        (float)area.left() * dpr, (float)area.bottom() * dpr,
        (float)area.left() * dpr, (float)area.top() * dpr};
    drawLineStrip(border, m_axisColor, 1.5f);

    // Tick marks as line segments
    std::vector<float> ticks;
    for (const auto &t : xTicks)
    {
        float px = (float)dataToPixel(t.value, 0).x();
        ticks.push_back(px * dpr);
        ticks.push_back((float)area.bottom() * dpr);
        ticks.push_back(px * dpr);
        ticks.push_back((float)(area.bottom() + 5) * dpr);
    }
    for (const auto &t : yTicks)
    {
        float py = (float)dataToPixel(0, t.value).y();
        ticks.push_back((float)(area.left() - 5) * dpr);
        ticks.push_back(py * dpr);
        ticks.push_back((float)area.left() * dpr);
        ticks.push_back(py * dpr);
    }
    drawLines(ticks, m_axisColor, 1.0f * dpr);
}

// ==========================================================================
//  Legend
// ==========================================================================
void RealtimePlot::drawLegend(const QRect &area, QPainter &painter)
{
    int visCount = 0;
    for (const auto &s : m_series)
        if (s->visible())
            ++visCount;
    if (visCount == 0)
        return;

    painter.setFont(m_tickFont);

    const int lineH = 18;
    const int padding = 8;
    const int swatch = 20;
    const int boxW = 140;
    const int boxH = visCount * lineH + padding * 2;
    const int boxX = area.right() - boxW - 6;
    const int boxY = area.top() + 6;

    painter.setPen(QPen(m_axisColor, 1));
    painter.setBrush(QColor(m_bgColor.red(), m_bgColor.green(),
                            m_bgColor.blue(), 210));
    painter.drawRect(boxX, boxY, boxW, boxH);

    int row = 0;
    for (const auto &s : m_series)
    {
        if (!s->visible())
            continue;
        int y = boxY + padding + row * lineH;
        painter.setPen(QPen(s->color(), 2.5f));
        painter.drawLine(boxX + 6, y + lineH / 2, boxX + 6 + swatch, y + lineH / 2);
        painter.setPen(m_textColor);
        QString label = s->name();
        if (label.length() > 15)
            label = label.left(14) + "…";
        painter.drawText(boxX + 6 + swatch + 5, y,
                         boxW - swatch - 14, lineH,
                         Qt::AlignLeft | Qt::AlignVCenter, label);
        ++row;
    }
}

// ==========================================================================
//  Low-level GL: line strip (connected)
// ==========================================================================
void RealtimePlot::drawLineStrip(const std::vector<float> &verts,
                                 QColor color, float lineWidth)
{
    if (verts.size() < 4 || !m_shaderReady)
        return;

    m_shader->bind();
    const float dpr = static_cast<float>(devicePixelRatioF());
    m_shader->setUniformValue("u_viewport", (float)width() * dpr, (float)height() * dpr);
    m_shader->setUniformValue("u_color",
                              (float)color.redF(), (float)color.greenF(),
                              (float)color.blueF(), (float)color.alphaF());

    int loc = m_shader->attributeLocation("a_position");
    m_shader->enableAttributeArray(loc);
    m_shader->setAttributeArray(loc, GL_FLOAT, verts.data(), 2);

    glLineWidth(lineWidth);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(verts.size() / 2));

    m_shader->disableAttributeArray(loc);
    m_shader->release();
}

// ==========================================================================
//  Low-level GL: independent line segments (not a strip)
// ==========================================================================
void RealtimePlot::drawLines(const std::vector<float> &verts,
                             QColor color, float lineWidth)
{
    if (verts.size() < 4 || !m_shaderReady)
        return;
    const float dpr = static_cast<float>(devicePixelRatioF());

    m_shader->bind();

    m_shader->setUniformValue("u_viewport", (float)width() * dpr, (float)height() * dpr);
    m_shader->setUniformValue("u_color",
                              (float)color.redF(), (float)color.greenF(),
                              (float)color.blueF(), (float)color.alphaF());

    int loc = m_shader->attributeLocation("a_position");
    m_shader->enableAttributeArray(loc);
    m_shader->setAttributeArray(loc, GL_FLOAT, verts.data(), 2);

    glLineWidth(lineWidth);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verts.size() / 2));

    m_shader->disableAttributeArray(loc);
    m_shader->release();
}

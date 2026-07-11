#include <QOpenGLShaderProgram>
#include <QPainter>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <ranges>
#include "RealtimePlot.h"

// ==========================================================================
//  Vertex / Fragment shaders
//  Expects pixel-space coordinates. Transforms to NDC internally.
//  Y axis: pixel 0 = top, so we negate.
// ==========================================================================
static const char* kVertSrc = R"GLSL(
    attribute vec2 a_position;
    uniform   vec2 u_viewport;   // (width, height) in pixels
    uniform   vec4 u_plot_limits;// (xmin, xMax, yMin, yMax)
    uniform   vec4 u_margins;     // (left, right, top, bottom) 
    void main() {

        float xMin = u_plot_limits.x;
        float xMax = u_plot_limits.y;
        float yMin = u_plot_limits.z;
        float yMax = u_plot_limits.w;

        float mLeft   = u_margins.x;
        float mRight  = u_margins.y;
        float mTop    = u_margins.z;
        float mBottom = u_margins.w;

        float plotWidth  = u_viewport.x - mLeft - mRight;
        float plotHeight = u_viewport.y - mTop - mBottom;

        float pixelX = mLeft + ((a_position.x - xMin) / (xMax - xMin)) * plotWidth;
        float pixelY = mTop + (1.0 - (a_position.y - yMin) / (yMax - yMin)) * plotHeight;

        float ndcX =  (pixelX / u_viewport.x) * 2.0 - 1.0;
        float ndcY = -((pixelY / u_viewport.y) * 2.0 - 1.0);

        gl_Position = vec4(ndcX, ndcY, 0.0, 1.0);
    }
)GLSL";

static const char* kFragSrc = R"GLSL(
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
    glClearColor(m_bgColor.redF(), m_bgColor.greenF(), m_bgColor.blueF(), 1.0f);
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
#ifdef RTP_ENABLE_DEBUG
    auto startTimer = std::chrono::high_resolution_clock::now();
#endif

    if (m_zoomAuto)
    {
        autoFit();
    }
    // Auto-init view on first paint
    if (!m_viewInitialized)
    {
        autoFit();
        m_viewInitialized = true;
    }

    // Compute ticks once per frame
    QRect area = plotArea();
    auto  xTicks =
        PlotAxis::computeTicks(m_xMin, m_xMax, std::max(3, area.width() / 80));
    auto yTicks =
        PlotAxis::computeTicks(m_yMin, m_yMax, std::max(3, area.height() / 50));

    int fontSize = 12;
    m_tickFont.setPointSize(fontSize);
    m_labelFont.setPointSize(fontSize + 2);

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
    for (const auto& t : xTicks)
    {
        QPoint px = dataToPixel(t.value, m_yMin);
        QRect  r(px.x() - 30, px.y() + 5, 60, 16);
        painter.drawText(r, Qt::AlignHCenter | Qt::AlignTop, t.label);
    }
    // Y-axis tick labels
    for (const auto& t : yTicks)
    {
        QPoint py = dataToPixel(m_xMin, t.value);
        QRect  r(2, py.y() - 8, m_margin.left - 6, 16);
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
    m_legend.draw(painter, plotArea());

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
                QPointF tl   = pixelToData(sel.topLeft());
                QPointF br   = pixelToData(sel.bottomRight());
                QString hint = QString("Δx=%1  Δy=%2")
                                   .arg(std::abs(br.x() - tl.x()), 0, 'g', 4)
                                   .arg(std::abs(br.y() - tl.y()), 0, 'g', 4);
                painter.setFont(m_tickFont);
                painter.setPen(QColor(255, 200, 50));
                painter.drawText(sel.bottomLeft() + QPoint(4, -4), hint);
            }
        }
    }

    m_cursorRange.draw(painter, plotArea(),
                       dataToPixel(m_cursorRange.getPosStart(), 0).x(),
                       dataToPixel(m_cursorRange.getPosEnd(), 0).x());

    int cIndex = 1;
    for (auto it = std::rbegin(m_cursors); it != std::rend(m_cursors); ++it)
    {
        auto& c = *it;
        c.draw(painter, plotArea(), dataToPixel(c.getPos(), 0).x(),
               QString("Marker %1").arg(cIndex++));
    }

    painter.end();

    glFinish();

#ifdef RTP_ENABLE_DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - startTimer;

    static int frameCount = 0;
    if (++frameCount % 30 == 0)
    {
        qDebug() << "Tiempo de paintGL():" << elapsed.count() << "ms"
                 << "| FPS teóricos:" << (1000.0 / elapsed.count())
                 << " | Puntos: " << m_series[0]->size();
    }
#endif
}

// ==========================================================================
//  Grid
// ==========================================================================
void RealtimePlot::drawGrid(const QRect&                   area,
                            const QVector<PlotAxis::Tick>& xTicks,
                            const QVector<PlotAxis::Tick>& yTicks)
{
    if (!m_grid.enabled)
        return;

    const float dpr = static_cast<float>(devicePixelRatioF());

    std::vector<float> verts;
    verts.reserve((xTicks.size() + yTicks.size()) * 4);

    for (const auto& t : xTicks)
    {
        verts.push_back((float)t.value);
        verts.push_back((float)m_yMin);
        verts.push_back((float)t.value);
        verts.push_back((float)m_yMax);
    }
    for (const auto& t : yTicks)
    {
        verts.push_back((float)m_xMin);
        verts.push_back((float)t.value);
        verts.push_back((float)m_xMax);
        verts.push_back((float)t.value);
    }

    // Grid lines are line SEGMENTS, not a strip; draw as GL_LINES
    drawLines(verts, m_grid.color, 1.0f * dpr);
}

// ==========================================================================
//  Series rendering  (data → pixel conversion on CPU)
// ==========================================================================
void RealtimePlot::drawSeries(const QRect& area)
{

    auto startTimer = std::chrono::high_resolution_clock::now();
    if (!m_shaderReady || m_series.empty())
        return;

    glEnable(GL_SCISSOR_TEST);
    const float dpr = static_cast<float>(devicePixelRatioF());

    glScissor(area.left() * dpr, (height() - area.bottom() - 1) * dpr,
              area.width() * dpr, area.height() * dpr);
    m_shader->bind();

    m_shader->setUniformValue("u_viewport", (float)width() * dpr,
                              (float)height() * dpr);
    m_shader->setUniformValue("u_plot_limits", (float)m_xMin, (float)m_xMax,
                              (float)m_yMin, (float)m_yMax);
    m_shader->setUniformValue(
        "u_margins", (float)m_margin.left * dpr, (float)m_margin.right * dpr,
        (float)m_margin.top * dpr, (float)m_margin.bottom * dpr);

    int loc = m_shader->attributeLocation("a_position");

    for (const auto& s : m_series)
    {

        s->initGLBuffers();

        if (!s->visible())
            continue;

        const std::vector<PlotSeries::Point>& visiblePts =
            s->getVisiblePoints(m_xMin, m_xMax, area.width());
        if (visiblePts.size() < 2)
            continue;

        s->uploadVisiblePoints(visiblePts);

        m_shader->setUniformValue(
            "u_color", (float)s->color().redF(), (float)s->color().greenF(),
            (float)s->color().blueF(), (float)s->color().alphaF());
        glLineWidth(s->lineWidth() * dpr);

        s->vao()->bind();
        s->vbo().bind();

        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 2, GL_DOUBLE, GL_FALSE,
                              sizeof(PlotSeries::Point), nullptr);

        glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(visiblePts.size()));

        s->vao()->release();
        s->vbo().release();
    }

    m_shader->release();
    glDisable(GL_SCISSOR_TEST);
}

// ==========================================================================
//  Axes border + tick marks
// ==========================================================================
void RealtimePlot::drawAxes(const QRect&                   area,
                            const QVector<PlotAxis::Tick>& xTicks,
                            const QVector<PlotAxis::Tick>& yTicks)
{

    const float dpr = static_cast<float>(devicePixelRatioF());

    // Plot border
    std::vector<float> border = {(float)m_xMin, (float)m_yMax, (float)m_xMax,
                                 (float)m_yMax, (float)m_xMax, (float)m_yMin,
                                 (float)m_xMin, (float)m_yMin, (float)m_xMin,
                                 (float)m_yMax};
    drawLineStrip(border, m_axisColor, 1.5f * dpr);

    // Tick marks as line segments
    std::vector<float> ticks;

    double xTickSize =
        (m_xMax - m_xMin) / (area.width() > 0 ? area.width() : 1) * 5.0;
    double yTickSize =
        (m_yMax - m_yMin) / (area.height() > 0 ? area.height() : 1) * 5.0;

    for (const auto& t : xTicks)
    {
        ticks.push_back((float)t.value);
        ticks.push_back((float)m_yMin);
        ticks.push_back((float)t.value);
        ticks.push_back((float)(m_yMin - yTickSize));
    }
    for (const auto& t : yTicks)
    {
        ticks.push_back((float)m_xMin);
        ticks.push_back((float)t.value);
        ticks.push_back((float)(m_xMin - xTickSize));
        ticks.push_back((float)t.value);
    }
    drawLines(ticks, m_axisColor, 1.0f * dpr);
}

// ==========================================================================
//  Low-level GL: line strip (connected)
// ==========================================================================
void RealtimePlot::drawLineStrip(const std::vector<float>& verts, QColor color,
                                 float lineWidth)
{
    if (verts.size() < 4 || !m_shaderReady)
        return;

    m_shader->bind();
    const float dpr = static_cast<float>(devicePixelRatioF());
    m_shader->setUniformValue("u_viewport", (float)width() * dpr,
                              (float)height() * dpr);
    m_shader->setUniformValue("u_color", (float)color.redF(),
                              (float)color.greenF(), (float)color.blueF(),
                              (float)color.alphaF());
    m_shader->setUniformValue("u_plot_limits", (float)m_xMin, (float)m_xMax,
                              (float)m_yMin, (float)m_yMax);
    m_shader->setUniformValue(
        "u_margins", (float)m_margin.left * dpr, (float)m_margin.right * dpr,
        (float)m_margin.top * dpr, (float)m_margin.bottom * dpr);

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
void RealtimePlot::drawLines(const std::vector<float>& verts, QColor color,
                             float lineWidth)
{
    if (verts.size() < 4 || !m_shaderReady)
        return;
    const float dpr = static_cast<float>(devicePixelRatioF());

    m_shader->bind();

    m_shader->setUniformValue("u_viewport", (float)width() * dpr,
                              (float)height() * dpr);
    m_shader->setUniformValue("u_color", (float)color.redF(),
                              (float)color.greenF(), (float)color.blueF(),
                              (float)color.alphaF());
    m_shader->setUniformValue("u_plot_limits", (float)m_xMin, (float)m_xMax,
                              (float)m_yMin, (float)m_yMax);
    m_shader->setUniformValue(
        "u_margins", (float)m_margin.left * dpr, (float)m_margin.right * dpr,
        (float)m_margin.top * dpr, (float)m_margin.bottom * dpr);

    int loc = m_shader->attributeLocation("a_position");
    m_shader->enableAttributeArray(loc);
    m_shader->setAttributeArray(loc, GL_FLOAT, verts.data(), 2);

    glLineWidth(lineWidth);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verts.size() / 2));

    m_shader->disableAttributeArray(loc);
    m_shader->release();
}

void RealtimePlot::addCursor()
{
    double position = (m_xMax - m_xMin) / 2;
    addCursor(position);
}

void RealtimePlot::addCursor(double xPos)
{

    m_cursors.push_back(RtpCursor(m_cursors.size(), Qt::green,
                                  RtpCursor::MarkerStyle::MARKER_SIMPLE));
    m_cursors[m_cursors.size() - 1].setSeriesSource(&m_series);
    m_cursors[m_cursors.size() - 1].setPos(xPos);
}
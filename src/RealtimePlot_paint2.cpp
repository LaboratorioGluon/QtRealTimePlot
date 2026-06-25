#include "RealtimePlot.h"
#include <QPainter>
#include <QOpenGLShaderProgram>
#include <algorithm>
#include <cmath>
#include <chrono>

// ==========================================================================
//  Vertex / Fragment shaders
//  Expects pixel-space coordinates. Transforms to NDC internally.
//  Y axis: pixel 0 = top, so we negate.
// ==========================================================================
static const char *kVertSrc = R"GLSL(
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

    auto start = std::chrono::high_resolution_clock::now();

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

    int fontSize = 12;
    m_tickFont.setPointSize(fontSize);
    m_labelFont.setPointSize(fontSize + 2);

    // Clear
    glClearColor(m_bgColor.redF(), m_bgColor.greenF(), m_bgColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // qDebug() << "Pre draw: " << std::chrono::duration<double, std::milli>((std::chrono::high_resolution_clock::now() - start)).count();
    drawGrid(area, xTicks, yTicks);
    drawSeries(area);
    // qDebug() << "Post draw: " << std::chrono::duration<double, std::milli>((std::chrono::high_resolution_clock::now() - start)).count();
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

    drawCursorH(painter);

    painter.end();

    glFinish();

    // 3. Calcular la diferencia de tiempo
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    // 4. Imprimir en la consola de Qt de forma controlada (ej: cada 30 frames para no saturar)
    static int frameCount = 0;
    if (++frameCount % 30 == 0)
    {
        qDebug() << "Tiempo de paintGL():" << elapsed.count() << "ms"
                 << "| FPS teóricos:" << (1000.0 / elapsed.count()) << " | Puntos: " << m_series[0]->size();
    }
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
        verts.push_back((float)t.value);
        verts.push_back((float)m_yMin);
        verts.push_back((float)t.value);
        verts.push_back((float)m_yMax);
    }
    for (const auto &t : yTicks)
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
void RealtimePlot::drawSeries(const QRect &area)
{

    auto start = std::chrono::high_resolution_clock::now();
    if (!m_shaderReady || m_series.empty())
        return;

    glEnable(GL_SCISSOR_TEST);
    const float dpr = static_cast<float>(devicePixelRatioF());
    // OpenGL Y is bottom-up; area.bottom() is the largest Y pixel (bottom of rect)
    glScissor(area.left() * dpr,
              (height() - area.bottom() - 1) * dpr,
              area.width() * dpr,
              area.height() * dpr);
    m_shader->bind();

    m_shader->setUniformValue("u_viewport", (float)width() * dpr, (float)height() * dpr);
    m_shader->setUniformValue("u_plot_limits", (float)m_xMin, (float)m_xMax, (float)m_yMin, (float)m_yMax);
    m_shader->setUniformValue("u_margins", (float)m_margin.left * dpr, (float)m_margin.right * dpr,
                              (float)m_margin.top * dpr, (float)m_margin.bottom * dpr);

    int loc = m_shader->attributeLocation("a_position");
    // qDebug() << "Pre series: " << std::chrono::duration<double, std::milli>((std::chrono::high_resolution_clock::now() - start)).count();
    for (const auto &s : m_series)
    {
        // start = std::chrono::high_resolution_clock::now();
        s->initGLBuffers();
        // qDebug() << "\tInit buffers: " << std::chrono::duration<double, std::milli>((std::chrono::high_resolution_clock::now() - start)).count();

        if (!s->visible())
            continue;

        // start = std::chrono::high_resolution_clock::now();
        std::vector<PlotSeries::Point> &visiblePts = s->getVisiblePoints(m_xMin, m_xMax, area.width());
        if (visiblePts.size() < 2)
            continue;

        // qDebug() << "\tgetVisible: " << std::chrono::duration<double, std::milli>((std::chrono::high_resolution_clock::now() - start)).count();

        // start = std::chrono::high_resolution_clock::now();
        s->uploadVisiblePoints(visiblePts);
        // qDebug() << "\tpload points: " << std::chrono::duration<double, std::milli>((std::chrono::high_resolution_clock::now() - start)).count();
        m_shader->setUniformValue("u_color", (float)s->color().redF(), (float)s->color().greenF(),
                                  (float)s->color().blueF(), (float)s->color().alphaF());
        glLineWidth(s->lineWidth() * dpr);

        // 3. Enlazar el VAO (Esto activa el VBO y configura el puntero de atributos automáticamente)
        s->vao()->bind();
        s->vbo().bind();

        // Por seguridad, si el ID de localización dinámico no es cero, forzamos su asignación
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 2, GL_DOUBLE, GL_FALSE, sizeof(PlotSeries::Point), nullptr);

        // 4. Pintar directamente desde la memoria de la tarjeta gráfica
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
void RealtimePlot::drawAxes(const QRect &area,
                            const QVector<PlotAxis::Tick> &xTicks,
                            const QVector<PlotAxis::Tick> &yTicks)
{

    const float dpr = static_cast<float>(devicePixelRatioF());

    // Plot border
    std::vector<float> border = {
        (float)m_xMin, (float)m_yMax,
        (float)m_xMax, (float)m_yMax,
        (float)m_xMax, (float)m_yMin,
        (float)m_xMin, (float)m_yMin,
        (float)m_xMin, (float)m_yMax};
    drawLineStrip(border, m_axisColor, 1.5f * dpr);

    // Tick marks as line segments
    std::vector<float> ticks;

    double xTickSize = (m_xMax - m_xMin) / (area.width() > 0 ? area.width() : 1) * 5.0;
    double yTickSize = (m_yMax - m_yMin) / (area.height() > 0 ? area.height() : 1) * 5.0;

    for (const auto &t : xTicks)
    {
        ticks.push_back((float)t.value);
        ticks.push_back((float)m_yMin);
        ticks.push_back((float)t.value);
        ticks.push_back((float)(m_yMin - yTickSize)); // Hacia abajo del eje X
    }
    for (const auto &t : yTicks)
    {
        ticks.push_back((float)m_xMin);
        ticks.push_back((float)t.value);
        ticks.push_back((float)(m_xMin - xTickSize)); // Hacia la izquierda del eje Y
        ticks.push_back((float)t.value);
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
    m_shader->setUniformValue("u_plot_limits", (float)m_xMin, (float)m_xMax, (float)m_yMin, (float)m_yMax);
    m_shader->setUniformValue("u_margins", (float)m_margin.left * dpr, (float)m_margin.right * dpr,
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
    m_shader->setUniformValue("u_plot_limits", (float)m_xMin, (float)m_xMax, (float)m_yMin, (float)m_yMax);
    m_shader->setUniformValue("u_margins", (float)m_margin.left * dpr, (float)m_margin.right * dpr,
                              (float)m_margin.top * dpr, (float)m_margin.bottom * dpr);

    int loc = m_shader->attributeLocation("a_position");
    m_shader->enableAttributeArray(loc);
    m_shader->setAttributeArray(loc, GL_FLOAT, verts.data(), 2);

    glLineWidth(lineWidth);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verts.size() / 2));

    m_shader->disableAttributeArray(loc);
    m_shader->release();
}

void RealtimePlot::drawCursorH(QPainter &painter)
{

    QFontMetrics fm(m_cursorFont);
    int lineHeight = fm.height();

    // Lines
    QPen cursorPen(Qt::red, 2, Qt::DashLine);
    painter.save();
    painter.setPen(QPen(QColor(255, 200, 50, 200), 1, Qt::SolidLine));
    painter.setFont(m_cursorFont);
    if (m_showXCursors)
    {

        painter.setPen(cursorPen);
        int px1 = dataToPixel(m_cursorX1, m_yMin).x();
        int px2 = dataToPixel(m_cursorX2, m_yMin).x();
        QRect plotRect = plotArea();

        if (px1 < px2)
        {
            painter.setPen(QPen(QColor(255, 200, 50, 200), 0, Qt::SolidLine));
            painter.setBrush(QColor(255, 200, 50, 30));
            painter.drawRect(QRect(QPoint(px1, plotRect.top()), QPoint(px2, plotRect.bottom())));
        }

        // Cursor X1
        if (px1 >= plotRect.left() && px1 <= plotRect.right())
        {
            painter.setPen(QPen(QColor(255, 200, 50, 200), 1, Qt::SolidLine));
            painter.drawLine(px1, plotRect.top(), px1, plotRect.bottom());

            m_rectLabelX1 = QRect(px1, plotRect.top() + 5, -120, lineHeight * (m_series.size() + 1) + 4).normalized();

            painter.setBrush(QColor(10, 10, 10, 255));

            painter.drawRect(m_rectLabelX1);
            painter.drawText(QRect(px1 - 4, plotRect.top() + 5, -120, lineHeight).normalized(),
                             Qt::AlignRight | Qt::AlignTop,
                             QString("X1: %1").arg(m_cursorX1, 0, 'f', 2));

            int index = 1;
            for (auto &s : m_series)
            {
                painter.setPen(s->color());
                painter.drawText(QRect(px1 - 4, plotRect.top() + 5 + lineHeight * index++, -120, lineHeight).normalized(),
                                 Qt::AlignRight | Qt::AlignTop,
                                 QString("%1").arg(s->getClosestPointToX(m_cursorX1).y, 0, 'f', 2));
            }
        }
        else
        {
            m_rectLabelX1 = QRect();
        }

        // Cursor X2
        if (px2 >= plotRect.left() && px2 <= plotRect.right())
        {
            painter.setPen(QPen(QColor(255, 200, 50, 200), 1, Qt::SolidLine));
            painter.drawLine(px2, plotRect.top(), px2, plotRect.bottom());

            m_rectLabelX2 = QRect(px2, plotRect.top() + 5, 120, lineHeight * (m_series.size() + 1) + 4);
            painter.setBrush(QColor(10, 10, 10, 255));

            painter.drawRect(m_rectLabelX2);
            painter.drawText(QRect(px2 - 4, plotRect.top() + 5, 120, lineHeight),
                             Qt::AlignRight | Qt::AlignTop,
                             QString("X2: %1").arg(m_cursorX2, 0, 'f', 2));
            int index = 1;
            for (auto &s : m_series)
            {
                painter.setPen(s->color());
                painter.drawText(QRect(px2 - 4, plotRect.top() + 5 + lineHeight * index++, 120, lineHeight),
                                 Qt::AlignRight | Qt::AlignTop,
                                 QString("%1").arg(s->getClosestPointToX(m_cursorX2).y, 0, 'f', 2));
            }
        }
        else
        {
            m_rectLabelX2 = QRect();
        }
    }
    painter.restore();

    if (m_showYCursors)
    {
        painter.setPen(cursorPen);
        int py1 = dataToPixel(m_xMin, m_cursorY1).y();
        int py2 = dataToPixel(m_xMin, m_cursorY2).y();
        QRect plotRect = plotArea();

        // Cursor Y1
        if (py1 >= plotRect.top() && py1 <= plotRect.bottom())
        {
            painter.drawLine(plotRect.left(), py1, plotRect.right(), py1);

            m_rectLabelY1 = QRect(plotRect.left() + 5, py1 - 22, 75, 18);
            painter.drawText(m_rectLabelY1, Qt::AlignLeft | Qt::AlignVCenter, QString("Y1: %1").arg(m_cursorY1, 0, 'f', 2));
        }
        else
        {
            m_rectLabelY1 = QRect();
        }

        // Cursor Y2
        if (py2 >= plotRect.top() && py2 <= plotRect.bottom())
        {
            painter.drawLine(plotRect.left(), py2, plotRect.right(), py2);

            m_rectLabelY2 = QRect(plotRect.left() + 5, py2 - 22, 75, 18);
            painter.drawText(m_rectLabelY2, Qt::AlignLeft | Qt::AlignVCenter, QString("Y2: %1").arg(m_cursorY2, 0, 'f', 2));
        }
        else
        {
            m_rectLabelY2 = QRect();
        }
    }
}
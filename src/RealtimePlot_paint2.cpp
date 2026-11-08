#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QSurfaceFormat>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <unordered_set>
#include "RealtimePlot.h"

// ==========================================================================
//  Vertex / Fragment shaders
//  Input positions are normalized to the plot area ([0, 1] on both axes).
//  The header is chosen at runtime so the same body works on GL 2.1,
//  GL 3.2+ core profiles and OpenGL ES 2.0+.
// ==========================================================================
static const char* kVertBody = R"GLSL(
    IN_ATTR vec2 a_position;
    uniform vec2 u_viewport;   // (width, height) in pixels
    uniform vec4 u_margins;    // (left, right, top, bottom)
    void main() {
        float plotWidth  = u_viewport.x - u_margins.x - u_margins.y;
        float plotHeight = u_viewport.y - u_margins.z - u_margins.w;

        float pixelX = u_margins.x + a_position.x * plotWidth;
        float pixelY = u_margins.z + (1.0 - a_position.y) * plotHeight;

        float ndcX =  (pixelX / u_viewport.x) * 2.0 - 1.0;
        float ndcY = -((pixelY / u_viewport.y) * 2.0 - 1.0);

        gl_Position = vec4(ndcX, ndcY, 0.0, 1.0);
    }
)GLSL";

static const char* kFragBody = R"GLSL(
    uniform vec4 u_color;
    void main() {
        FRAG_COLOR = u_color;
    }
)GLSL";

static constexpr int kPositionLocation = 0;

namespace {

struct ShaderHeaders {
    QByteArray vert;
    QByteArray frag;
};

ShaderHeaders shaderHeadersFor(const QOpenGLContext* ctx)
{
    const QSurfaceFormat fmt = ctx->format();
    if (ctx->isOpenGLES())
    {
        return {"#version 100\n#define IN_ATTR attribute\n",
                "#version 100\nprecision mediump float;\n#define FRAG_COLOR gl_FragColor\n"};
    }
    const bool modern = fmt.profile() == QSurfaceFormat::CoreProfile && fmt.version() >= qMakePair(3, 2);
    if (modern)
    {
        return {"#version 150\n#define IN_ATTR in\n", "#version 150\nout vec4 fragColor;\n#define FRAG_COLOR fragColor\n"};
    }
    return {"#version 120\n#define IN_ATTR attribute\n", "#version 120\n#define FRAG_COLOR gl_FragColor\n"};
}

/**
 * Expands GL_LINES / GL_LINE_STRIP vertices into GL_TRIANGLES quads of the given pixel width.
 * Each segment is extended by half the width at both ends so strip joins have no gaps.
 */
void expandLinesToTriangles(const float* xy, int vertexCount, GLenum mode, float widthPx, float plotWidthPx,
                            float plotHeightPx, std::vector<float>& out)
{
    out.clear();
    if (vertexCount < 2 || plotWidthPx <= 0.0f || plotHeightPx <= 0.0f)
        return;

    const int step         = (mode == GL_LINES) ? 2 : 1;
    const int segmentCount = (mode == GL_LINES) ? vertexCount / 2 : vertexCount - 1;
    out.reserve(static_cast<size_t>(segmentCount) * 12);

    const float half = widthPx * 0.5f;
    for (int s = 0, i = 0; s < segmentCount; ++s, i += step)
    {
        const float x0 = xy[2 * i] * plotWidthPx, y0 = xy[2 * i + 1] * plotHeightPx;
        const float x1 = xy[2 * i + 2] * plotWidthPx, y1 = xy[2 * i + 3] * plotHeightPx;
        float       dx = x1 - x0, dy = y1 - y0;
        const float len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-6f)
        {
            dx = 1.0f;
            dy = 0.0f;
        }
        else
        {
            dx /= len;
            dy /= len;
        }
        const float tx = dx * half, ty = dy * half;    // along the segment
        const float nx = -dy * half, ny = dx * half;   // perpendicular

        const float ax = x0 - tx + nx, ay = y0 - ty + ny;
        const float bx = x0 - tx - nx, by = y0 - ty - ny;
        const float cx = x1 + tx + nx, cy = y1 + ty + ny;
        const float ex = x1 + tx - nx, ey = y1 + ty - ny;

        const float quad[12] = {ax, ay, bx, by, cx, cy, cx, cy, bx, by, ex, ey};
        for (int k = 0; k < 12; k += 2)
        {
            out.push_back(quad[k] / plotWidthPx);
            out.push_back(quad[k + 1] / plotHeightPx);
        }
    }
}

}  // namespace

// ==========================================================================
//  OpenGL lifecycle
// ==========================================================================
void RealtimePlot::initializeGL()
{
    // Called again whenever Qt recreates the context (e.g. after reparenting).
    initializeOpenGLFunctions();

    QOpenGLContext* ctx = context();
    connect(ctx, &QOpenGLContext::aboutToBeDestroyed, this, &RealtimePlot::cleanupGL, Qt::UniqueConnection);

    const bool isCore = !ctx->isOpenGLES() && ctx->format().profile() == QSurfaceFormat::CoreProfile;

    glClearColor(m_bgColor.redF(), m_bgColor.greenF(), m_bgColor.blueF(), 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (!isCore && !ctx->isOpenGLES())
    {
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    }

    // Widest line the driver draws natively; wider lines are drawn as triangles.
    GLfloat range[2] = {1.0f, 1.0f};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
    m_maxLineWidth = isCore ? 1.0f : std::max(1.0f, range[1]);

    const ShaderHeaders headers = shaderHeadersFor(ctx);
    delete m_shader;
    m_shader = new QOpenGLShaderProgram(this);
    m_shader->bindAttributeLocation("a_position", kPositionLocation);
    m_shaderReady =
        m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, headers.vert + kVertBody) &&
        m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, headers.frag + kFragBody) && m_shader->link();
    if (!m_shaderReady)
        qWarning() << "RealtimePlot: shader build failed:" << m_shader->log();

    m_overlayGpu = std::make_unique<GpuBuffer>();
}

void RealtimePlot::cleanupGL()
{
    if (!m_shader && m_seriesGpu.empty() && !m_overlayGpu)
        return;

    makeCurrent();
    for (auto& entry : m_seriesGpu)
    {
        entry.second->vao.destroy();
        entry.second->vbo.destroy();
    }
    m_seriesGpu.clear();
    if (m_overlayGpu)
    {
        m_overlayGpu->vao.destroy();
        m_overlayGpu->vbo.destroy();
        m_overlayGpu.reset();
    }
    delete m_shader;
    m_shader      = nullptr;
    m_shaderReady = false;
    doneCurrent();

    if (context())
        disconnect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &RealtimePlot::cleanupGL);
}

void RealtimePlot::resizeGL(int /*w*/, int /*h*/)
{
    // QOpenGLWidget handles glViewport automatically
}

RealtimePlot::GpuBuffer& RealtimePlot::gpuBufferFor(const PlotSeries* series)
{
    auto& slot = m_seriesGpu[series];
    if (!slot)
        slot = std::make_unique<GpuBuffer>();
    return *slot;
}

void RealtimePlot::uploadToBuffer(GpuBuffer& buf, const void* data, int bytes)
{
    if (!buf.vbo.isCreated())
    {
        buf.vao.create();  // May fail on plain GL 2.1; attributes are set on every draw anyway.
        buf.vbo.create();
        buf.vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
    }

    buf.vbo.bind();
    if (bytes > buf.capacityBytes)
        buf.capacityBytes = std::max(bytes + bytes / 2, 4096);
    // Orphan the previous storage so the driver never waits for the GPU to finish reading it.
    buf.vbo.allocate(buf.capacityBytes);
    buf.vbo.write(0, data, bytes);
}

void RealtimePlot::drawPrimitive(GpuBuffer& buf, const float* xy, int vertexCount, GLenum mode, QColor color,
                                 float widthPx, const QRect& area)
{
    if (vertexCount < 2 || !m_shaderReady)
        return;

    const float* data     = xy;
    int          count    = vertexCount;
    GLenum       drawMode = mode;

    if (widthPx > m_maxLineWidth + 0.01f)
    {
        const float dpr = static_cast<float>(devicePixelRatioF());
        expandLinesToTriangles(xy, vertexCount, mode, widthPx, area.width() * dpr, area.height() * dpr, m_triScratch);
        data     = m_triScratch.data();
        count    = static_cast<int>(m_triScratch.size() / 2);
        drawMode = GL_TRIANGLES;
        if (count == 0)
            return;
    }
    else
    {
        glLineWidth(widthPx);
    }

    if (buf.vao.isCreated())
        buf.vao.bind();
    uploadToBuffer(buf, data, count * 2 * static_cast<int>(sizeof(float)));

    m_shader->setUniformValue("u_color", static_cast<float>(color.redF()), static_cast<float>(color.greenF()),
                              static_cast<float>(color.blueF()), static_cast<float>(color.alphaF()));

    glEnableVertexAttribArray(kPositionLocation);
    glVertexAttribPointer(kPositionLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glDrawArrays(drawMode, 0, count);
    glDisableVertexAttribArray(kPositionLocation);

    buf.vbo.release();
    if (buf.vao.isCreated())
        buf.vao.release();
}

// ==========================================================================
//  Main paint
// ==========================================================================
void RealtimePlot::paintGL()
{
#ifdef RTP_ENABLE_DEBUG
    auto startTimer = std::chrono::high_resolution_clock::now();
#endif
    m_inPaint        = true;
    m_paintedVersion = dataVersion();

    if (m_zoomAuto)
    {
        if (m_zoomWindow <= 0.0)
            autoFit();
        else
            autoFitRolling();
    }
    // Auto-init view on first paint
    if (!m_viewInitialized)
    {
        autoFit();
        m_viewInitialized = true;
    }

    // Compute ticks once per frame
    QRect area   = plotArea();
    auto  xTicks = PlotAxis::computeTicks(m_xMin, m_xMax, std::max(10, area.width() / 80));
    auto  yTicks = PlotAxis::computeTicks(m_yMin, m_yMax, std::max(10, area.height() / 50));

    // QPainter may have changed GL state during the previous frame; restore what we rely on.
    const float dpr = static_cast<float>(devicePixelRatioF());
    glViewport(0, 0, static_cast<GLsizei>(width() * dpr), static_cast<GLsizei>(height() * dpr));
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(m_bgColor.redF(), m_bgColor.greenF(), m_bgColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_shaderReady)
    {
        m_shader->bind();
        m_shader->setUniformValue("u_viewport", width() * dpr, height() * dpr);
        m_shader->setUniformValue("u_margins", m_margin.left * dpr, m_margin.right * dpr, m_margin.top * dpr,
                                  m_margin.bottom * dpr);

        drawGrid(area, xTicks, yTicks);
        drawSeries(area);
        drawAxes(area, xTicks, yTicks);

        m_shader->release();
    }

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
        painter.drawText(QRect(0, 2, width(), m_margin.top - 2), Qt::AlignHCenter | Qt::AlignBottom, m_title);
    }

    // X label
    if (!m_xLabel.isEmpty())
    {
        painter.setFont(m_labelFont);
        painter.drawText(QRect(m_margin.left, height() - m_margin.bottom + 22, area.width(), m_margin.bottom - 22),
                         Qt::AlignHCenter | Qt::AlignTop, m_xLabel);
    }

    // Y label (rotated)
    if (!m_yLabel.isEmpty())
    {
        painter.setFont(m_labelFont);
        painter.save();
        painter.translate(14, m_margin.top + area.height() / 2);
        painter.rotate(-90);
        painter.drawText(QRect(-60, -10, 120, 20), Qt::AlignHCenter | Qt::AlignVCenter, m_yLabel);
        painter.restore();
    }

    // Legend
    if (m_legendVisible)
        m_legend.draw(painter, area);

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

    m_cursorRange.draw(painter, area, dataToPixel(m_cursorRange.getPosStart(), 0).x(),
                       dataToPixel(m_cursorRange.getPosEnd(), 0).x());

    int cIndex = 1;
    for (auto it = m_cursors.rbegin(); it != m_cursors.rend(); ++it)
    {
        auto& c = *it;
        c.draw(painter, area, dataToPixel(c.getPos(), 0).x(), QString("Marker %1").arg(cIndex++));
    }

    painter.end();
    m_inPaint = false;

#ifdef RTP_ENABLE_DEBUG
    auto                                      end     = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - startTimer;

    static int frameCount = 0;
    if (++frameCount % 30 == 0)
    {
        qDebug() << "paintGL():" << elapsed.count() << "ms"
                 << "| Points:" << (m_series.empty() ? 0 : m_series[0]->size());
    }
#endif
}

// ==========================================================================
//  Grid
// ==========================================================================
void RealtimePlot::drawGrid(const QRect& area, const QVector<PlotAxis::Tick>& xTicks,
                            const QVector<PlotAxis::Tick>& yTicks)
{
    if (!m_grid.enabled || !m_overlayGpu)
        return;

    const float dpr = static_cast<float>(devicePixelRatioF());

    double xRange = m_xMax - m_xMin;
    double yRange = m_yMax - m_yMin;
    if (xRange <= 0.0 || yRange <= 0.0)
        return;

    std::vector<float> verts;
    verts.reserve((xTicks.size() + yTicks.size()) * 4);

    for (const auto& t : xTicks)
    {
        float xNorm = static_cast<float>((t.value - m_xMin) / xRange);
        verts.insert(verts.end(), {xNorm, 0.0f, xNorm, 1.0f});
    }
    for (const auto& t : yTicks)
    {
        float yNorm = static_cast<float>((t.value - m_yMin) / yRange);
        verts.insert(verts.end(), {0.0f, yNorm, 1.0f, yNorm});
    }

    drawPrimitive(*m_overlayGpu, verts.data(), static_cast<int>(verts.size() / 2), GL_LINES, m_grid.color,
                  1.0f * dpr, area);
}

// ==========================================================================
//  Series rendering
// ==========================================================================
void RealtimePlot::drawSeries(const QRect& area)
{
    const float dpr = static_cast<float>(devicePixelRatioF());

    // Release buffers of series that are no longer part of this plot.
    // Every add/remove cycle leaves one stale entry, so a size mismatch is enough to detect them.
    if (m_seriesGpu.size() > m_series.size())
    {
        std::unordered_set<const PlotSeries*> alive;
        for (const auto& s : m_series)
            alive.insert(s.get());
        for (auto it = m_seriesGpu.begin(); it != m_seriesGpu.end();)
        {
            if (alive.count(it->first) == 0)
            {
                it->second->vao.destroy();
                it->second->vbo.destroy();
                it = m_seriesGpu.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    if (m_series.empty())
        return;

    glEnable(GL_SCISSOR_TEST);
    glScissor(static_cast<GLint>(area.left() * dpr), static_cast<GLint>((height() - area.bottom() - 1) * dpr),
              static_cast<GLsizei>(area.width() * dpr), static_cast<GLsizei>(area.height() * dpr));

    const double xRange = m_xMax - m_xMin;
    const double yRange = m_yMax - m_yMin;
    // Decimate to physical pixels so HiDPI screens get full detail.
    const int targetWidth = std::max(1, static_cast<int>(std::lround(area.width() * dpr)));

    for (const auto& s : m_series)
    {
        if (!s->visible())
            continue;

        const std::vector<PlotSeries::Point>& visiblePts = s->getVisiblePoints(m_xMin, m_xMax, targetWidth);
        if (visiblePts.size() < 2)
            continue;

        m_normScratch.clear();
        m_normScratch.reserve(visiblePts.size());
        for (const auto& pt : visiblePts)
        {
            double xn = (xRange > 0.0) ? (pt.x - m_xMin) / xRange : 0.0;
            double yn = (yRange > 0.0) ? (pt.y - m_yMin) / yRange : 0.0;
            m_normScratch.push_back({static_cast<float>(xn), static_cast<float>(yn)});
        }

        static_assert(sizeof(PlotSeries::NormalizedPoint) == 2 * sizeof(float), "NormalizedPoint must be packed");
        drawPrimitive(gpuBufferFor(s.get()), reinterpret_cast<const float*>(m_normScratch.data()),
                      static_cast<int>(m_normScratch.size()), GL_LINE_STRIP, s->color(), s->lineWidth() * dpr, area);
    }

    glDisable(GL_SCISSOR_TEST);
}

// ==========================================================================
//  Axes border + tick marks
// ==========================================================================
void RealtimePlot::drawAxes(const QRect& area, const QVector<PlotAxis::Tick>& xTicks,
                            const QVector<PlotAxis::Tick>& yTicks)
{
    if (!m_overlayGpu)
        return;

    const float dpr = static_cast<float>(devicePixelRatioF());

    double xRange = m_xMax - m_xMin;
    double yRange = m_yMax - m_yMin;
    if (xRange <= 0.0 || yRange <= 0.0)
        return;

    const float border[] = {
        0.0f, 1.0f,  // Top-Left
        1.0f, 1.0f,  // Top-Right
        1.0f, 0.0f,  // Bottom-Right
        0.0f, 0.0f,  // Bottom-Left
        0.0f, 1.0f   // Back to Top-Left
    };
    drawPrimitive(*m_overlayGpu, border, 5, GL_LINE_STRIP, m_axisColor, 1.5f * dpr, area);

    std::vector<float> ticks;
    ticks.reserve((xTicks.size() + yTicks.size()) * 4);

    float xTickSizeNorm = (area.width() > 0) ? 5.0f / area.width() : 0.01f;
    float yTickSizeNorm = (area.height() > 0) ? 5.0f / area.height() : 0.01f;

    for (const auto& t : xTicks)
    {
        float xNorm = static_cast<float>((t.value - m_xMin) / xRange);
        ticks.insert(ticks.end(), {xNorm, 0.0f, xNorm, -yTickSizeNorm});  // Below the bottom border
    }
    for (const auto& t : yTicks)
    {
        float yNorm = static_cast<float>((t.value - m_yMin) / yRange);
        ticks.insert(ticks.end(), {0.0f, yNorm, -xTickSizeNorm, yNorm});  // Left of the left border
    }
    drawPrimitive(*m_overlayGpu, ticks.data(), static_cast<int>(ticks.size() / 2), GL_LINES, m_axisColor, 1.0f * dpr,
                  area);
}

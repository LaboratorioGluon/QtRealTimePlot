#include "PlotRenderer.h"

#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace rtp
{

namespace
{

// Input positions are normalized to the plot area ([0, 1] on both axes).
// The header is chosen at runtime so the same body works on every GL flavour.
const char* kVertBody = R"GLSL(
    IN_ATTR vec2 a_position;
    uniform vec2 u_viewport;   // (width, height) in device pixels
    uniform vec4 u_area;       // plot area (x, y, width, height) in device pixels, Y down
    void main() {
        float pixelX = u_area.x + a_position.x * u_area.z;
        float pixelY = u_area.y + (1.0 - a_position.y) * u_area.w;
        gl_Position  = vec4((pixelX / u_viewport.x) * 2.0 - 1.0, 1.0 - (pixelY / u_viewport.y) * 2.0, 0.0, 1.0);
    }
)GLSL";

const char* kFragBody = R"GLSL(
    uniform vec4 u_color;
    void main() {
        FRAG_COLOR = u_color;
    }
)GLSL";

constexpr int kPositionLocation = 0;

struct ShaderHeaders {
    QByteArray vert;
    QByteArray frag;
};

ShaderHeaders shaderHeadersFor(const QOpenGLContext* ctx)
{
    const QSurfaceFormat fmt = ctx->format();
    if (ctx->isOpenGLES())
        return {"#version 100\n#define IN_ATTR attribute\n",
                "#version 100\nprecision mediump float;\n#define FRAG_COLOR gl_FragColor\n"};
    if (fmt.profile() == QSurfaceFormat::CoreProfile && fmt.version() >= qMakePair(3, 2))
        return {"#version 150\n#define IN_ATTR in\n",
                "#version 150\nout vec4 fragColor;\n#define FRAG_COLOR fragColor\n"};
    return {"#version 120\n#define IN_ATTR attribute\n", "#version 120\n#define FRAG_COLOR gl_FragColor\n"};
}

/**
 * Expands GL_LINES / GL_LINE_STRIP vertices into GL_TRIANGLES quads of the given pixel width.
 * Each segment is extended by half the width at both ends so strip joins have no gaps.
 */
void expandLinesToTriangles(const float* xy, int vertexCount, GLenum mode, float widthPx, float areaWidthPx,
                            float areaHeightPx, std::vector<float>& out)
{
    out.clear();
    if (vertexCount < 2 || areaWidthPx <= 0.0f || areaHeightPx <= 0.0f)
        return;

    const int step         = (mode == GL_LINES) ? 2 : 1;
    const int segmentCount = (mode == GL_LINES) ? vertexCount / 2 : vertexCount - 1;
    out.reserve(static_cast<size_t>(segmentCount) * 12);

    const float half = widthPx * 0.5f;
    for (int s = 0, i = 0; s < segmentCount; ++s, i += step)
    {
        const float x0 = xy[2 * i] * areaWidthPx, y0 = xy[2 * i + 1] * areaHeightPx;
        const float x1 = xy[2 * i + 2] * areaWidthPx, y1 = xy[2 * i + 3] * areaHeightPx;
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
        const float tx = dx * half, ty = dy * half;   // along the segment
        const float nx = -dy * half, ny = dx * half;  // perpendicular

        const float quad[12] = {x0 - tx + nx, y0 - ty + ny, x0 - tx - nx, y0 - ty - ny, x1 + tx + nx, y1 + ty + ny,
                                x1 + tx + nx, y1 + ty + ny, x0 - tx - nx, y0 - ty - ny, x1 + tx - nx, y1 + ty - ny};
        for (int k = 0; k < 12; k += 2)
        {
            out.push_back(quad[k] / areaWidthPx);
            out.push_back(quad[k + 1] / areaHeightPx);
        }
    }
}

}  // namespace

PlotRenderer::PlotRenderer() = default;

PlotRenderer::~PlotRenderer()
{
    // The owner must call cleanup() with the context current; this only frees CPU memory.
    delete m_shader;
}

void PlotRenderer::initialize(QOpenGLContext* ctx)
{
    initializeOpenGLFunctions();
    cleanup();

    const bool isCore = !ctx->isOpenGLES() && ctx->format().profile() == QSurfaceFormat::CoreProfile;
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
    m_shader                    = new QOpenGLShaderProgram;
    m_shader->bindAttributeLocation("a_position", kPositionLocation);
    m_shaderReady = m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, headers.vert + kVertBody) &&
                    m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, headers.frag + kFragBody) &&
                    m_shader->link();
    if (!m_shaderReady)
        qWarning() << "RealtimePlot: shader build failed:" << m_shader->log();

    m_overlayGpu = std::make_unique<GpuBuffer>();
}

void PlotRenderer::releaseBuffer(GpuBuffer& buf)
{
    buf.vao.destroy();
    buf.vbo.destroy();
    buf.capacityBytes = 0;
}

void PlotRenderer::cleanup()
{
    for (auto& entry : m_seriesGpu)
        releaseBuffer(*entry.second);
    m_seriesGpu.clear();
    if (m_overlayGpu)
    {
        releaseBuffer(*m_overlayGpu);
        m_overlayGpu.reset();
    }
    delete m_shader;
    m_shader      = nullptr;
    m_shaderReady = false;
}

PlotRenderer::GpuBuffer& PlotRenderer::bufferFor(const PlotSeries* series)
{
    auto& slot = m_seriesGpu[series];
    if (!slot)
        slot = std::make_unique<GpuBuffer>();
    return *slot;
}

void PlotRenderer::upload(GpuBuffer& buf, const void* data, int bytes)
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

void PlotRenderer::drawPrimitive(GpuBuffer& buf, const float* xy, int vertexCount, GLenum mode, QColor color,
                                 float widthPx)
{
    if (vertexCount < 2 || !m_frameActive)
        return;

    const float* data     = xy;
    int          count    = vertexCount;
    GLenum       drawMode = mode;

    if (widthPx > m_maxLineWidth + 0.01f)
    {
        expandLinesToTriangles(xy, vertexCount, mode, widthPx, static_cast<float>(m_areaPx.width()),
                               static_cast<float>(m_areaPx.height()), m_triangles);
        data     = m_triangles.data();
        count    = static_cast<int>(m_triangles.size() / 2);
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
    upload(buf, data, count * 2 * static_cast<int>(sizeof(float)));

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

void PlotRenderer::beginFrame(const QSize& widgetSize, float dpr, const PlotView& view, QColor bg)
{
    m_widgetSize  = widgetSize;
    m_dpr         = dpr;
    const QRect a = view.area();
    m_areaPx = QRect(qRound(a.left() * dpr), qRound(a.top() * dpr), qRound(a.width() * dpr), qRound(a.height() * dpr));

    // QPainter may have changed GL state during the previous frame; restore what we rely on.
    const int vw = qRound(widgetSize.width() * dpr);
    const int vh = qRound(widgetSize.height() * dpr);
    glViewport(0, 0, vw, vh);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(bg.redF(), bg.greenF(), bg.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_frameActive = m_shaderReady && m_shader->bind();
    if (!m_frameActive)
        return;
    m_shader->setUniformValue("u_viewport", static_cast<float>(vw), static_cast<float>(vh));
    m_shader->setUniformValue("u_area", static_cast<float>(m_areaPx.x()), static_cast<float>(m_areaPx.y()),
                              static_cast<float>(m_areaPx.width()), static_cast<float>(m_areaPx.height()));
}

void PlotRenderer::endFrame()
{
    if (m_frameActive)
        m_shader->release();
    m_frameActive = false;
}

void PlotRenderer::drawGrid(const PlotView& view, const QVector<PlotAxis::Tick>& xTicks,
                            const QVector<PlotAxis::Tick>& yTicks, QColor color)
{
    m_vertices.clear();
    for (const auto& t : xTicks)
    {
        const float x = static_cast<float>(view.normX(t.value));
        m_vertices.insert(m_vertices.end(), {x, 0.0f, x, 1.0f});
    }
    for (const auto& t : yTicks)
    {
        const float y = static_cast<float>(view.normY(t.value));
        m_vertices.insert(m_vertices.end(), {0.0f, y, 1.0f, y});
    }
    drawPrimitive(*m_overlayGpu, m_vertices.data(), static_cast<int>(m_vertices.size() / 2), GL_LINES, color,
                  1.0f * m_dpr);
}

void PlotRenderer::drawSeries(const PlotView& view, const std::vector<std::shared_ptr<PlotSeries>>& series)
{
    // Every add/remove cycle leaves one stale entry, so a size mismatch is enough to detect them.
    if (m_seriesGpu.size() > series.size())
    {
        std::unordered_set<const PlotSeries*> alive;
        for (const auto& s : series)
            alive.insert(s.get());
        for (auto it = m_seriesGpu.begin(); it != m_seriesGpu.end();)
        {
            if (alive.count(it->first) == 0)
            {
                releaseBuffer(*it->second);
                it = m_seriesGpu.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    if (series.empty() || !m_frameActive)
        return;

    glEnable(GL_SCISSOR_TEST);
    const int viewportHeight = qRound(m_widgetSize.height() * m_dpr);
    glScissor(m_areaPx.x(), viewportHeight - m_areaPx.y() - m_areaPx.height(), m_areaPx.width(), m_areaPx.height());

    // Decimate to device pixels so HiDPI screens get full detail.
    const int    targetWidth = std::max(1, m_areaPx.width());
    const double xMin = view.xMin(), xMax = view.xMax();

    for (const auto& s : series)
    {
        if (!s->visible())
            continue;

        s->copyVisibleRange(xMin, xMax, targetWidth, m_gather);
        decimateMinMax(m_gather, xMin, xMax, targetWidth, m_visible, m_buckets);
        if (m_visible.size() < 2)
            continue;

        // Clip every segment, in data coordinates and double precision, to a guard band of
        // one view span around the plot area. When zoomed in, neighbours can lie millions of
        // view-widths away; normalizing them unclipped would overflow float precision and bend
        // the visible part of the line. The scissor test hides what lies in the guard band.
        const double spanX = view.xMax() - view.xMin();
        const double spanY = view.yMax() - view.yMin();
        const double xLo = view.xMin() - spanX, xHi = view.xMax() + spanX;
        const double yLo = view.yMin() - spanY, yHi = view.yMax() + spanY;

        m_vertices.clear();
        m_vertices.reserve(m_visible.size() * 4);
        for (size_t i = 1; i < m_visible.size(); ++i)
        {
            double x0 = m_visible[i - 1].x, y0 = m_visible[i - 1].y;
            double x1 = m_visible[i].x, y1 = m_visible[i].y;
            if (!clipSegment(x0, y0, x1, y1, xLo, xHi, yLo, yHi))
                continue;
            m_vertices.push_back(static_cast<float>(view.normX(x0)));
            m_vertices.push_back(static_cast<float>(view.normY(y0)));
            m_vertices.push_back(static_cast<float>(view.normX(x1)));
            m_vertices.push_back(static_cast<float>(view.normY(y1)));
        }
        if (m_vertices.empty())
            continue;

        drawPrimitive(bufferFor(s.get()), m_vertices.data(), static_cast<int>(m_vertices.size() / 2), GL_LINES,
                      s->color(), s->lineWidth() * m_dpr);
    }

    glDisable(GL_SCISSOR_TEST);
}

void PlotRenderer::drawFrame(const PlotView& view, const QVector<PlotAxis::Tick>& xTicks,
                             const QVector<PlotAxis::Tick>& yTicks, QColor color)
{
    const float border[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    drawPrimitive(*m_overlayGpu, border, 5, GL_LINE_STRIP, color, 1.5f * m_dpr);

    const QRect a        = view.area();
    const float xTickLen = 5.0f / static_cast<float>(a.width());  // ticks are 5 px long
    const float yTickLen = 5.0f / static_cast<float>(a.height());

    m_vertices.clear();
    for (const auto& t : xTicks)
    {
        const float x = static_cast<float>(view.normX(t.value));
        m_vertices.insert(m_vertices.end(), {x, 0.0f, x, -yTickLen});  // below the bottom border
    }
    for (const auto& t : yTicks)
    {
        const float y = static_cast<float>(view.normY(t.value));
        m_vertices.insert(m_vertices.end(), {0.0f, y, -xTickLen, y});  // left of the left border
    }
    drawPrimitive(*m_overlayGpu, m_vertices.data(), static_cast<int>(m_vertices.size() / 2), GL_LINES, color,
                  1.0f * m_dpr);
}

}  // namespace rtp

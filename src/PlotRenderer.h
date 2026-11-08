#pragma once

#include <QColor>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QSize>
#include <QVector>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Decimation.h"
#include "PlotAxis.h"
#include "PlotSeries.h"
#include "PlotView.h"

class QOpenGLContext;

namespace rtp
{

/**
 * @class PlotRenderer
 * @brief Draws grid, series and axis frame with OpenGL for one widget.
 *
 * Owns every GL object (shader, per-series buffers) of one context. Works on
 * desktop GL 2.1, GL 3.2+ core profiles and OpenGL ES 2.0+. Lines wider than
 * the driver supports are drawn as triangles.
 *
 * All methods must be called with the owning context current.
 */
class PlotRenderer : protected QOpenGLFunctions
{
   public:
    PlotRenderer();
    ~PlotRenderer();

    PlotRenderer(const PlotRenderer&)            = delete;
    PlotRenderer& operator=(const PlotRenderer&) = delete;

    /** @brief Creates GL resources for ctx. Safe to call again after cleanup() for a new context. */
    void initialize(QOpenGLContext* ctx);
    /** @brief Releases every GL resource. */
    void cleanup();
    bool isInitialized() const { return m_shader != nullptr; }

    /** @brief Resets GL state, clears to bg and prepares the shader for this frame. */
    void beginFrame(const QSize& widgetSize, float dpr, const PlotView& view, QColor bg);
    void drawGrid(const PlotView& view, const QVector<PlotAxis::Tick>& xTicks, const QVector<PlotAxis::Tick>& yTicks,
                  QColor color);
    void drawSeries(const PlotView& view, const std::vector<std::shared_ptr<PlotSeries>>& series);
    /** @brief Border around the plot area plus tick marks outside it. */
    void drawFrame(const PlotView& view, const QVector<PlotAxis::Tick>& xTicks, const QVector<PlotAxis::Tick>& yTicks,
                   QColor color);
    void endFrame();

   private:
    struct GpuBuffer {
        QOpenGLBuffer            vbo{QOpenGLBuffer::VertexBuffer};
        QOpenGLVertexArrayObject vao;
        int                      capacityBytes = 0;
    };

    void drawPrimitive(GpuBuffer& buf, const float* xy, int vertexCount, GLenum mode, QColor color, float widthPx);
    void upload(GpuBuffer& buf, const void* data, int bytes);
    GpuBuffer& bufferFor(const PlotSeries* series);
    void       releaseBuffer(GpuBuffer& buf);

    QOpenGLShaderProgram* m_shader       = nullptr;
    bool                  m_shaderReady  = false;
    float                 m_maxLineWidth = 1.0f;

    // Per-frame state
    bool  m_frameActive = false;
    float m_dpr         = 1.0f;
    QSize m_widgetSize;
    QRect m_areaPx; /**< Plot area in device pixels, GL-independent Y-down convention. */

    std::unordered_map<const PlotSeries*, std::unique_ptr<GpuBuffer>> m_seriesGpu;
    std::unique_ptr<GpuBuffer>                                        m_overlayGpu;

    // Reused scratch, so steady-state frames do not allocate.
    std::vector<SampleBuffer::Point> m_gather;
    std::vector<SampleBuffer::Point> m_visible;
    std::vector<MinMaxBucket>        m_buckets;
    std::vector<float>               m_vertices;
    std::vector<float>               m_triangles;
};

}  // namespace rtp

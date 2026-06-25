#pragma once

#include <QColor>
#include <QString>
#include <vector>
#include <mutex>
#include <limits>

#include <GL/gl.h>

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

/**
 * @brief Thread-safe circular buffer for a single plot series.
 *
 * Stores (x, y) pairs with a configurable max capacity.
 * Data can be pushed from any thread; rendering reads under lock.
 */
class PlotSeries : protected QOpenGLFunctions
{
public:
    struct Point
    {
        double x;
        double y;
    };

    explicit PlotSeries(QString name, QColor color = Qt::cyan);

    // --- Data ingestion (thread-safe) ---
    void pushPoint(double x, double y);
    void pushPoint(const Point &p);
    void clear();

    // --- Accessors (call under lock or from render thread only) ---
    std::unique_lock<std::mutex> lock() const;
    const std::vector<Point> &points() const { return m_points; }

    // Metadata
    const QString &name() const { return m_name; }
    QColor color() const { return m_color; }
    void setColor(QColor c) { m_color = c; }
    void setName(const QString &n) { m_name = n; }
    bool visible() const { return m_visible; }
    void setVisible(bool v) { m_visible = v; }
    float lineWidth() const { return m_lineWidth; }
    void setLineWidth(float w) { m_lineWidth = w; }

    // Bounding box (computed lazily, reset on push)
    double xMin() const { return m_xMin; }
    double xMax() const { return m_xMax; }
    double yMin() const { return m_yMin; }
    double yMax() const { return m_yMax; }

    // VBO for OpenGL
    void initGLBuffers();
    void destroyGLBuffers();
    void updateVBO();
    QOpenGLVertexArrayObject *vao() { return &m_vao; }
    size_t vertexCount() const { return m_points.size(); }
    QOpenGLBuffer &vbo() { return m_vbo; }
    void syncWithGPU();

    std::vector<PlotSeries::Point> &getVisiblePoints(double xMin, double xMax, int targetWidth);
    void uploadVisiblePoints(const std::vector<Point> &visiblePts);

    int size() { return m_points.size(); }

private:
    void updateBounds(const Point &p);
    void recomputeBounds();

    mutable std::mutex m_mutex;
    std::vector<Point> m_points;
    std::vector<Point> m_visibleBuffer;

    QString m_name;
    QColor m_color;
    bool m_visible = true;
    float m_lineWidth = 1.5f;

    double m_xMin = std::numeric_limits<double>::max();
    double m_xMax = -std::numeric_limits<double>::max();
    double m_yMin = std::numeric_limits<double>::max();
    double m_yMax = -std::numeric_limits<double>::max();

    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vao;
    bool m_glInitialized = false;
    size_t m_pointsInGPU = 0;
};

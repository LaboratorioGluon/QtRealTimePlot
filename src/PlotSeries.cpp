#include "PlotSeries.h"

#include <QOpenGLContext>

PlotSeries::PlotSeries(QString name, QColor color)
    : m_name(std::move(name)), m_color(color)
{
}

void PlotSeries::pushPoint(double x, double y)
{
    pushPoint({x, y});
}

void PlotSeries::pushPoint(const Point &p)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    m_points.push_back(p);
    updateBounds(p);
}

void PlotSeries::clear()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_points.clear();
    m_xMin = std::numeric_limits<double>::max();
    m_xMax = -std::numeric_limits<double>::max();
    m_yMin = std::numeric_limits<double>::max();
    m_yMax = -std::numeric_limits<double>::max();
}

std::unique_lock<std::mutex> PlotSeries::lock() const
{
    return std::unique_lock<std::mutex>(m_mutex);
}

void PlotSeries::updateBounds(const Point &p)
{
    if (p.x < m_xMin)
        m_xMin = p.x;
    if (p.x > m_xMax)
        m_xMax = p.x;
    if (p.y < m_yMin)
        m_yMin = p.y;
    if (p.y > m_yMax)
        m_yMax = p.y;
}

void PlotSeries::recomputeBounds()
{
    m_xMin = std::numeric_limits<double>::max();
    m_xMax = -std::numeric_limits<double>::max();
    m_yMin = std::numeric_limits<double>::max();
    m_yMax = -std::numeric_limits<double>::max();
    for (const auto &p : m_points)
        updateBounds(p);
}

void PlotSeries::initGLBuffers()
{
    if (m_glInitialized)
        return;
    initializeOpenGLFunctions(); // Inicializa el d_ptr de esta serie

    m_vao.create();
    m_vbo.create();

    m_vbo.bind();
    // m_vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    size_t maxMemory = 10'000'000 * sizeof(Point);
    m_vbo.allocate(nullptr, static_cast<int>(maxMemory));
    m_vbo.release();

    m_glInitialized = true;
}

void PlotSeries::destroyGLBuffers()
{
    if (!m_glInitialized)
        return;
    m_vao.destroy();
    m_vbo.destroy();
    m_glInitialized = false;
}

// Sube el std::vector entero a la GPU de golpe
void PlotSeries::updateVBO()
{
    /*std::lock_guard<std::mutex> lg(m_mutex);
    if (m_points.empty() || !m_glInitialized)
        return;

    m_vbo.bind();
    // glBufferData nativo a través del wrapper de Qt:
    m_vbo.allocate(m_points.data(), static_cast<int>(m_points.size() * sizeof(Point)));
    m_vbo.release();*/
}

void PlotSeries::syncWithGPU()
{
    std::lock_guard<std::mutex> lg(m_mutex);

    if (!m_glInitialized || m_points.empty())
        return;
    if (m_points.size() == m_pointsInGPU)
        return; // Nada nuevo que subir

    size_t newPointsCount = m_points.size() - m_pointsInGPU;

    m_vbo.bind();

    // Calculamos el offset en bytes a partir del último punto que ya subimos
    int offsetBytes = static_cast<int>(m_pointsInGPU * sizeof(Point));
    int sizeBytes = static_cast<int>(newPointsCount * sizeof(Point));

    // Obtenemos el puntero directo a los datos nuevos en la RAM
    const Point *dataPtr = &m_points[m_pointsInGPU];

    // Subimos de golpe solo el bloque de datos nuevos de forma segura
    m_vbo.write(offsetBytes, dataPtr, sizeBytes);

    m_vbo.release();

    // Actualizamos nuestro contador de control
    m_pointsInGPU = m_points.size();
}
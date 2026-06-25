#include "PlotSeries.h"

#include <QOpenGLContext>

PlotSeries::PlotSeries(QString name, QColor color)
    : m_name(std::move(name)), m_color(color)
{
    m_lodLevels.resize(NUM_LOD_LEVELS);
}

void PlotSeries::pushPoint(double x, double y)
{
    pushPoint({x, y});
}

void PlotSeries::pushPoint(const Point &p)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_lodLevels[0].points.push_back(p);
    updateBounds(p);

    m_points.push_back(p);
    updateBounds(p);

    if (m_lodLevels[0].points.size() - m_lodLevels[1].lastProcessedCrudeSize >= 1000)
    {
        updateLodLevels();
    }
}

void PlotSeries::clear()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_points.clear();
    for (auto &level : m_lodLevels)
    {
        level.points.clear();
        level.lastProcessedCrudeSize = 0;
    }
    m_pointsInGPU = 0;
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

    m_vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);

    m_vbo.release();

    m_glInitialized = true;
}

void PlotSeries::uploadVisiblePoints(const std::vector<Point> &visiblePts)
{
    if (!m_glInitialized || visiblePts.empty())
        return;

    m_vbo.bind();
    // allocate() vacía el buffer anterior en la GPU y sube el nuevo tamaño exacto.
    // Como visiblePts suele tener un tamaño máximo de ~4000 puntos (unos 64 KB),
    // esta transferencia a través del PCIe tarda prácticamente 0 milisegundos.
    m_vbo.allocate(visiblePts.data(), static_cast<int>(visiblePts.size() * sizeof(Point)));
    m_vbo.release();
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

std::vector<PlotSeries::Point> &PlotSeries::getVisiblePoints(double xMin, double xMax, int targetWidth)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_visibleBuffer.clear();

    if (m_points.empty() || targetWidth <= 0)
        return m_visibleBuffer;

    const_cast<PlotSeries *>(this)->updateLodLevels();

    const auto &crudePoints = m_lodLevels[0].points;
    auto itStartCrude = std::lower_bound(crudePoints.begin(), crudePoints.end(), xMin,
                                         [](const Point &p, double val)
                                         { return p.x < val; });
    auto itEndCrude = std::upper_bound(crudePoints.begin(), crudePoints.end(), xMax,
                                       [](double val, const Point &p)
                                       { return val < p.x; });

    size_t totalVisibleCrude = std::distance(itStartCrude, itEndCrude);
    if (totalVisibleCrude == 0)
        return m_visibleBuffer;

    size_t selectedLevel = 0;
    double pointsPerPixel = static_cast<double>(totalVisibleCrude) / targetWidth;

    if (pointsPerPixel > 10000.0)
    {
        selectedLevel = 2; // Gran cantidad de datos (Zoom Out total) -> LOD 2 (x100)
    }
    else if (pointsPerPixel > 50.0)
    {
        selectedLevel = 1; // Transición suave -> LOD 1 (x10)
    }
    else
    {
        selectedLevel = 0; // Máxima fidelidad -> LOD 0 (Dato crudo)
    }

    qDebug() << "Level: " << selectedLevel << " Visible: " << totalVisibleCrude << "Density: " << pointsPerPixel;

    const auto &sourcePoints = m_lodLevels[selectedLevel].points;

    auto itStart = std::lower_bound(sourcePoints.begin(), sourcePoints.end(), xMin,
                                    [](const Point &p, double val)
                                    { return p.x < val; });
    auto itEnd = std::upper_bound(sourcePoints.begin(), sourcePoints.end(), xMax,
                                  [](double val, const Point &p)
                                  { return val < p.x; });

    size_t totalVisible = std::distance(itStart, itEnd);
    if (totalVisible == 0)
        return m_visibleBuffer;

    int maxPointsThreshold = targetWidth * 2;
    if (totalVisible <= static_cast<size_t>(maxPointsThreshold))
    {
        m_visibleBuffer.assign(itStart, itEnd);
        return m_visibleBuffer;
    }

    if (m_visibleBuffer.capacity() < static_cast<size_t>(maxPointsThreshold))
    {
        m_visibleBuffer.reserve(maxPointsThreshold);
    }

    // 2. ─── MÁXIMA OPTIMIZACIÓN: ALGORITMO DE UNA SOLA PASADA ───
    double xRange = xMax - xMin;
    if (xRange <= 0)
        return m_visibleBuffer;

    double inverseXRangeWithWidth = static_cast<double>(targetWidth) / xRange;

    // Estructura para llevar el control de cada columna de píxel (bucket)
    struct BucketData
    {
        Point minPt;
        Point maxPt;
        bool assigned = false;
    };

    // Usamos un vector temporal estático/local para los píxeles (tamaño fijo muy pequeño, ej: 1920)
    static std::vector<BucketData> buckets;
    if (buckets.size() != static_cast<size_t>(targetWidth))
    {
        buckets.resize(targetWidth);
    }
    for (int i = 0; i < targetWidth; ++i)
        buckets[i].assigned = false;

    // Clasificamos los 1.4M de puntos en su píxel correspondiente en un único bucle plano
    for (auto it = itStart; it != itEnd; ++it)
    {
        // Calculamos a qué píxel horizontal (columna) pertenece matemáticamente este punto
        int pixelIdx = static_cast<int>((it->x - xMin) * inverseXRangeWithWidth);

        // Control de límites por seguridad decimal
        if (pixelIdx < 0)
            pixelIdx = 0;
        if (pixelIdx >= targetWidth)
            pixelIdx = targetWidth - 1;

        BucketData &b = buckets[pixelIdx];
        if (!b.assigned)
        {
            b.minPt = *it;
            b.maxPt = *it;
            b.assigned = true;
        }
        else
        {
            if (it->y < b.minPt.y)
                b.minPt = *it;
            if (it->y > b.maxPt.y)
                b.maxPt = *it;
        }
    }

    // 3. Volcar los resultados respetando el orden temporal básico
    for (int i = 0; i < targetWidth; ++i)
    {
        if (!buckets[i].assigned)
            continue;

        if (buckets[i].minPt.x < buckets[i].maxPt.x)
        {
            m_visibleBuffer.push_back(buckets[i].minPt);
            m_visibleBuffer.push_back(buckets[i].maxPt);
        }
        else
        {
            m_visibleBuffer.push_back(buckets[i].maxPt);
            m_visibleBuffer.push_back(buckets[i].minPt);
        }
    }

    return m_visibleBuffer;
}

void PlotSeries::updateLodLevels()
{
    // ─── NIVEL 1: Procesa desde el Nivel 0 (Crudo) ───
    auto &crude = m_lodLevels[0].points;
    auto &lod1 = m_lodLevels[1].points;
    size_t startIdx = m_lodLevels[1].lastProcessedCrudeSize;
    size_t endIdx = crude.size() - (crude.size() % LOD_FACTOR); // Bloques completos de 10

    for (size_t i = startIdx; i < endIdx; i += LOD_FACTOR)
    {
        // Buscamos el min y max del bloque de 10 elementos
        size_t minIdx = i;
        size_t maxIdx = i;
        for (size_t k = 1; k < LOD_FACTOR; ++k)
        {
            if (crude[i + k].y < crude[minIdx].y)
                minIdx = i + k;
            if (crude[i + k].y > crude[maxIdx].y)
                maxIdx = i + k;
        }
        // Los insertamos manteniendo orden cronológico aproximado en X
        if (minIdx < maxIdx)
        {
            lod1.push_back(crude[minIdx]);
            lod1.push_back(crude[maxIdx]);
        }
        else
        {
            lod1.push_back(crude[maxIdx]);
            lod1.push_back(crude[minIdx]);
        }
    }
    m_lodLevels[1].lastProcessedCrudeSize = endIdx;

    // ─── NIVEL 2: Procesa desde el Nivel 1 ───
    auto &lod2 = m_lodLevels[2].points;
    size_t startIdx2 = m_lodLevels[2].lastProcessedCrudeSize;
    // Como Lod1 mete 2 puntos por bloque, el paso es (LOD_FACTOR * 2) si procesamos sobre la misma base
    // Pero lo más fácil es tratar a Lod1 como un vector normal y colapsar cada 10 puntos (5 parejas) en MinMax
    size_t endIdx2 = lod1.size() - (lod1.size() % LOD_FACTOR);

    for (size_t i = startIdx2; i < endIdx2; i += LOD_FACTOR)
    {
        size_t minIdx = i;
        size_t maxIdx = i;
        for (size_t k = 1; k < LOD_FACTOR; ++k)
        {
            if (lod1[i + k].y < lod1[minIdx].y)
                minIdx = i + k;
            if (lod1[i + k].y > lod1[maxIdx].y)
                maxIdx = i + k;
        }
        if (minIdx < maxIdx)
        {
            lod2.push_back(lod1[minIdx]);
            lod2.push_back(lod1[maxIdx]);
        }
        else
        {
            lod2.push_back(lod1[maxIdx]);
            lod2.push_back(lod1[minIdx]);
        }
    }
    m_lodLevels[2].lastProcessedCrudeSize = endIdx2;
}
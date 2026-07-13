#pragma once

#include <QColor>
#include <QString>
#include <limits>
#include <mutex>
#include <vector>

#include <GL/gl.h>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>

/**
 * @class PlotSeries
 * @brief Thread-safe circular buffer for a single plot series.
 *
 * Stores (x, y) coordinate pairs with support for multi-level Level of Detail (LOD)
 * and real-time statistics calculation. Data can be pushed from any thread 
 * (e.g., data acquisition threads), while the rendering loop reads data 
 * safely using mutual exclusion locks (mutex).
 */
class PlotSeries : protected QOpenGLFunctions
{
    friend class RealtimePlot;

   public:
    /**
     * @struct Point
     * @brief Represents a two-dimensional point in the chart.
     */
    struct Point {
        double x; /**< X-axis coordinate (typically time or sample index). */
        double y; /**< Y-axis coordinate (signal value). */
    };

    /**
     * @struct BlockStats
     * @brief Accumulated statistics for a specific block of data.
     *
     * Used to optimize metric calculations over data subsets and for 
     * point reduction routines within Level of Detail (LOD) generation.
     */
    struct BlockStats {
        double xStart; /**< Starting X value of this data block. */
        double xEnd;   /**< Ending X value of this data block. */
        double min;    /**< Minimum Y value found within this block. */
        double max;    /**< Maximum Y value found within this block. */
        double
            sum; /**< Accumulated sum of Y values. Useful for fast mean calculation (mean = sum / count). */
        double
            sumSq; /**< Sum of squares of Y values. Useful for Root Mean Square (RMS) or standard deviation. */
        size_t count; /**< Total number of samples processed in this block. */
    };

    /**
     * @struct IntervalStats
     * @brief Statistical metrics calculated over a specific chart window.
     */
    struct IntervalStats {
        double min = std::numeric_limits<
            double>::max(); /**< Minimum value in the interval. */
        double max = -std::numeric_limits<
            double>::max(); /**< Maximum value in the interval. */
        double mean = 0.0;  /**< Mean (average) value in the interval. */
        double rms  = 0.0; /**< Root Mean Square (RMS) value in the interval. */
        int    numSamples =
            0; /**< Total number of samples contained in the interval. */
        double timeDiff =
            0.0; /**< Time delta (X_max - X_min) of the analyzed interval. */
    };

    /**
     * @brief Constructs the plot data series.
     * @param name Identifying name of the series (e.g., "Channel 1").
     * @param color Color used to render the series by default (Default: Qt::cyan).
     */
    explicit PlotSeries(QString name, QColor color = Qt::cyan);

    // --- Data ingestion (thread-safe) ---

    /**
     * @brief Pushes a new point into the series using explicit X and Y components.
     * @note This method is thread-safe.
     * @param x X coordinate of the new point.
     * @param y Y coordinate of the new point.
     */
    void pushPoint(double x, double y);

    /**
     * @brief Pushes a structured point into the series.
     * @note This method is thread-safe.
     * @param p Point structure containing the data to append.
     */
    void pushPoint(const Point& p);

    /**
     * @brief Completely flushes the data series, resetting bounding boxes and LOD tiers.
     * @note This method is thread-safe.
     */
    void clear();

    // --- Accessors (call under lock or from render thread only) ---

    /**
     * @brief Acquires the internal mutex lock for external safe operations.
     * @return A std::unique_lock object managing the instance's mutex.
     */
    std::unique_lock<std::mutex> lock() const;

    /**
     * @brief Retrieves a constant reference to the raw points vector.
     * @warning Must be called under lock() if accessed from a non-rendering thread.
     * @return Constant reference to the underlying std::vector of raw points.
     */
    const std::vector<Point>& points() const { return m_points; }

    // Metadata

    /** @return The current name of the series. */
    const QString& name() const { return m_name; }

    /** @return The color assigned to this series. */
    QColor color() const { return m_color; }

    /** @param c New color for the series rendering. */
    void setColor(QColor c) { m_color = c; }

    /** @param n New name for the series. */
    void setName(const QString& n) { m_name = n; }

    /** @return True if the series is currently configured to be visible. */
    bool visible() const { return m_visible; }

    /** @param v Desired visibility state. */
    void setVisible(bool v) { m_visible = v; }

    /** @return Line width in pixels. */
    float lineWidth() const { return m_lineWidth; }

    /** @param w New line width in pixels (e.g., 1.5f). */
    void setLineWidth(float w) { m_lineWidth = w; }

    // Bounding box (computed lazily, reset on push)

    /** @return Current minimum absolute value along the X-axis. */
    double xMin() const { return m_xMin; }
    /** @return Current maximum absolute value along the X-axis. */
    double xMax() const { return m_xMax; }
    /** @return Current minimum absolute value along the Y-axis. */
    double yMin() const { return m_yMin; }
    /** @return Current maximum absolute value along the Y-axis. */
    double yMax() const { return m_yMax; }

    /**
     * @brief Extracts a subset of points optimized for a target horizontal pixel resolution.
     *
     * Filters or decimates points based on the visible viewport boundary (xMin to xMax)
     * and the pixel width of the widget container to prevent performance-killing overdraw.
     *
     * @param xMin Lower visible threshold of the X-axis.
     * @param xMax Upper visible threshold of the X-axis.
     * @param targetWidth Width in pixels of the drawing area on the screen.
     * @return Reference to an internal vector containing ready-to-render points.
     */
    std::vector<PlotSeries::Point>& getVisiblePoints(double xMin, double xMax,
                                                     int targetWidth);

    /**
     * @brief Gets raw points stored in a specific Level of Detail (LOD) tier.
     * @param lod Index of the target LOD tier (defaults to 0, maximum resolution).
     * @return Reference to the selected LOD tier point vector.
     */
    std::vector<PlotSeries::Point>& getDataPoints(uint32_t lod = 0)
    {
        return m_lodLevels[lod].points;
    }

    /** @return Total number of entries in the main series buffer. */
    int size() { return m_points.size(); }

    /**
     * @brief Safely searches for the closest point relative to a given X value.
     * @param xValue Reference lookup value along the X-axis.
     * @return The @ref Point structure sitting closest to the searched value.
     */
    PlotSeries::Point getClosestPointToX(double xValue);

    /**
     * @brief Computes advanced metrics (min, max, mean, RMS) within a specified X interval.
     * @param xMin Starting edge of the analysis window.
     * @param xMax Ending edge of the analysis window.
     * @return An @ref IntervalStats structure populated with the mathematical findings.
     */
    PlotSeries::IntervalStats calculateIntervalStats(double xMin, double xMax);

   protected:
    /**
     * @struct LodLevel
     * @brief Container for a specific Level of Detail (LOD) tier.
     *
     * Stores decimated or simplified versions of the original signal to accelerate 
     * rendering when viewing large time windows.
     */
    struct LodLevel {
        std::vector<Point>
            points; /**< Decimated points corresponding to this LOD level. */
        std::vector<BlockStats>
               stats; /**< Statistics of the blocks that make up this level. */
        size_t lastProcessedCrudeSize =
            0; /**< Amount of raw elements already processed for this level. */
    };

    /**
     * @brief Dynamically expands the current bounding box metrics to encompass a new point.
     * @param p Point structure to evaluate.
     */
    void updateBounds(const Point& p);

    /**
     * @brief Linearly recomputes absolute boundaries (mins and maxes) by scanning the entire series.
     */
    void recomputeBounds();

    mutable std::mutex
        m_mutex; /**< Mutex to ensure mutual exclusion across multi-threaded operations. */
    std::vector<Point>
        m_points; /**< Primary storage containing raw signal data. */
    std::vector<Point>
        m_visibleBuffer; /**< Scratchpad buffer used for exchanging and filtering displayable points. */

    std::vector<LodLevel>
        m_lodLevels; /**< Dynamic array holding discrete LOD data structures. */
    static constexpr size_t NUM_LOD_LEVELS =
        3; /**< Total number of supported Level of Detail tiers. */
    static constexpr size_t LOD_FACTOR =
        100; /**< Decimation scaling factor between consecutive LOD levels. */

    /**
     * @brief Increments or completely updates the LOD data arrays using newly injected points.
     */
    void updateLodLevels();

    QString m_name;  /**< Name identifier of the series. */
    QColor  m_color; /**< Rendering color profile. */
    bool    m_visible =
        true; /**< Control flag to skip or commit execution of the render pipeline. */
    float m_lineWidth =
        1.5f; /**< Line thickness configuration for rasterized rendering commands. */

    double m_xMin = std::numeric_limits<
        double>::max(); /**< Cached absolute minimum X boundary. */
    double m_xMax = -std::numeric_limits<
        double>::max(); /**< Cached absolute maximum X boundary. */
    double m_yMin = std::numeric_limits<
        double>::max(); /**< Cached absolute minimum Y boundary. */
    double m_yMax = -std::numeric_limits<
        double>::max(); /**< Cached absolute maximum Y boundary. */

    QOpenGLBuffer m_vbo{
        QOpenGLBuffer::
            VertexBuffer}; /**< Qt abstraction wrapper for the Vertex Buffer Object. */
    QOpenGLVertexArrayObject
         m_vao; /**< Qt abstraction wrapper for the Vertex Array Object. */
    bool m_glInitialized =
        false; /**< Confirms whether initGLBuffers was run successfully. */
    size_t m_pointsInGPU =
        0; /**< Total count of vertices currently allocated on VRAM. */

    // VBO for OpenGL

    /**
     * @brief Initializes OpenGL objects (VBO and VAO) in the current context.
     * @note Must be called from the thread that owns the OpenGL context (Render Thread).
     */
    void initGLBuffers();

    /**
     * @brief Releases and destroys the OpenGL buffers linked to this series.
     */
    void destroyGLBuffers();

    /**
     * @brief Gets the Vertex Array Object managed by this series.
     * @return Pointer to the QOpenGLVertexArrayObject instance.
     */
    QOpenGLVertexArrayObject* vao() { return &m_vao; }

    /** @return Total vertex count currently stored in RAM. */
    size_t vertexCount() const { return m_points.size(); }

    /**
     * @brief Gets a reference to the underlying Vertex Buffer Object (VBO).
     * @return Reference to the VertexBuffer-type QOpenGLBuffer.
     */
    QOpenGLBuffer& vbo() { return m_vbo; }

    /**
     * @brief Uploads an external buffer of pre-filtered visible points straight to the GPU VBO.
     * @param visiblePts Vector containing the pre-processed points to be written to VRAM.
     */
    void uploadVisiblePoints(const std::vector<Point>& visiblePts);
};
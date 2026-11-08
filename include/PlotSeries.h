#pragma once

#include <QColor>
#include <QString>
#include <atomic>
#include <cstdint>
#include <deque>
#include <limits>
#include <mutex>
#include <utility>
#include <vector>

/**
 * @class PlotSeries
 * @brief Thread-safe, optionally bounded sample buffer for a single plot series.
 *
 * Stores (x, y) coordinate pairs with support for multi-level Level of Detail (LOD)
 * and real-time statistics calculation. Data can be pushed from any thread
 * (e.g., data acquisition threads), while the rendering loop reads data
 * safely using mutual exclusion locks (mutex).
 *
 * Storage is chunked (std::deque), so appending never copies the existing
 * history. When a maximum number of points is configured, the oldest samples
 * are discarded as new ones arrive (ring-buffer behaviour).
 *
 * X values must be non-decreasing. Samples with an X value lower than the last
 * stored one, or with a non-finite coordinate, are rejected and counted in
 * droppedPoints().
 *
 * This class holds no GPU resources, so the same series can be shown in
 * several plots at once.
 */
class PlotSeries
{
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
     * @struct NormalizedPoint
     * @brief Point mapped to the visible range, ready to be uploaded to the GPU.
     */
    struct NormalizedPoint {
        float x_norm; /**< Range [0.0, 1.0] inside the visible window. */
        float y_norm; /**< Range [0.0, 1.0] inside the visible window. */
    };

    /**
     * @struct BlockStats
     * @brief Accumulated statistics for a specific block of data.
     *
     * Used to optimize metric calculations over data subsets and for
     * point reduction routines within Level of Detail (LOD) generation.
     */
    struct BlockStats {
        double   xStart;     /**< Starting X value of this data block. */
        double   xEnd;       /**< Ending X value of this data block. */
        double   min;        /**< Minimum Y value found within this block. */
        double   max;        /**< Maximum Y value found within this block. */
        double   sum;        /**< Accumulated sum of Y values (mean = sum / count). */
        double   sumSq;      /**< Sum of squares of Y values (for RMS). */
        size_t   count;      /**< Total number of samples processed in this block. */
        uint64_t firstIndex; /**< Absolute index of the first raw sample covered by this block. */
    };

    /**
     * @struct IntervalStats
     * @brief Statistical metrics calculated over a specific chart window.
     */
    struct IntervalStats {
        double min        = std::numeric_limits<double>::max();  /**< Minimum value in the interval. */
        double max        = -std::numeric_limits<double>::max(); /**< Maximum value in the interval. */
        double mean       = 0.0;                                 /**< Mean (average) value in the interval. */
        double rms        = 0.0;                                 /**< Root Mean Square (RMS) value in the interval. */
        int    numSamples = 0;   /**< Total number of samples contained in the interval. */
        double timeDiff   = 0.0; /**< Time delta (X_max - X_min) of the analyzed interval. */
    };

    /**
     * @brief Constructs the plot data series.
     * @param name Identifying name of the series (e.g., "Channel 1").
     * @param color Color used to render the series by default (Default: Qt::cyan).
     * @param maxPoints Maximum number of raw samples kept. 0 means unlimited.
     */
    explicit PlotSeries(QString name, QColor color = Qt::cyan, size_t maxPoints = 0);

    // --- Data ingestion (thread-safe) ---

    /**
     * @brief Pushes a new point into the series using explicit X and Y components.
     * @note This method is thread-safe.
     * @return False if the sample was rejected (X going backwards or non-finite values).
     */
    bool pushPoint(double x, double y);

    /**
     * @brief Pushes a structured point into the series.
     * @note This method is thread-safe.
     * @return False if the sample was rejected (X going backwards or non-finite values).
     */
    bool pushPoint(const Point& p);

    /**
     * @brief Completely flushes the data series, resetting bounding boxes, LOD tiers and statistics.
     * @note This method is thread-safe.
     */
    void clear();

    /**
     * @brief Sets the maximum number of raw samples kept. Oldest samples are discarded first.
     * @param maxPoints New capacity. 0 means unlimited.
     * @note This method is thread-safe.
     */
    void setMaxPoints(size_t maxPoints);

    /** @return The configured capacity (0 = unlimited). */
    size_t maxPoints() const;

    // --- Accessors ---

    /**
     * @brief Acquires the internal mutex lock for external safe operations.
     * @return A std::unique_lock object managing the instance's mutex.
     */
    std::unique_lock<std::mutex> lock() const;

    /**
     * @brief Retrieves a constant reference to the raw points.
     * @warning Must be called while holding lock().
     */
    const std::deque<Point>& points() const { return m_lodLevels[0].points; }

    // Metadata (GUI thread only). Setters bump version() so plots repaint.

    /** @return The current name of the series. */
    const QString& name() const { return m_name; }

    /** @return The color assigned to this series. */
    QColor color() const { return m_color; }

    /** @param c New color for the series rendering. */
    void setColor(QColor c)
    {
        m_color = c;
        ++m_version;
    }

    /** @param n New name for the series. */
    void setName(const QString& n)
    {
        m_name = n;
        ++m_version;
    }

    /** @return True if the series is currently configured to be visible. */
    bool visible() const { return m_visible; }

    /** @param v Desired visibility state. */
    void setVisible(bool v)
    {
        m_visible = v;
        ++m_version;
    }

    /** @return Line width in pixels. */
    float lineWidth() const { return m_lineWidth; }

    /** @param w New line width in pixels (e.g., 1.5f). */
    void setLineWidth(float w)
    {
        m_lineWidth = w;
        ++m_version;
    }

    // Bounding box of the stored samples. Must be called while holding lock().

    /** @return Minimum X value currently stored. */
    double xMin() const;
    /** @return Maximum X value currently stored. */
    double xMax() const;
    /** @return Minimum Y value currently stored. */
    double yMin() const;
    /** @return Maximum Y value currently stored. */
    double yMax() const;

    /**
     * @brief Extracts a subset of points optimized for a target horizontal pixel resolution.
     *
     * The visible range is copied while holding the lock. The min/max decimation
     * runs afterwards without the lock, so producers are not blocked by rendering.
     *
     * @param xMin Lower visible threshold of the X-axis.
     * @param xMax Upper visible threshold of the X-axis.
     * @param targetWidth Width in physical pixels of the drawing area on the screen.
     * @return Reference to an internal vector containing ready-to-render points.
     * @note Call from the GUI thread only. The returned buffer is reused on the next call.
     */
    std::vector<PlotSeries::Point>& getVisiblePoints(double xMin, double xMax, int targetWidth);

    /**
     * @brief Gets the points stored in a specific Level of Detail (LOD) tier.
     * @param lod Index of the target LOD tier (0 = full resolution).
     * @warning Must be called while holding lock().
     */
    const std::deque<PlotSeries::Point>& getDataPoints(uint32_t lod = 0) const { return m_lodLevels[lod].points; }

    /** @return Total number of raw samples currently stored. Thread-safe. */
    size_t size() const;

    /** @return Number of samples rejected by pushPoint(). Thread-safe. */
    uint64_t droppedPoints() const { return m_dropped.load(std::memory_order_relaxed); }

    /** @return Counter that changes whenever data or appearance changes. Thread-safe. */
    uint64_t version() const { return m_version.load(std::memory_order_relaxed); }

    /**
     * @brief Searches for the stored point closest to a given X value. Thread-safe.
     * @param xValue Reference lookup value along the X-axis.
     * @return The closest point, or {0, 0} if the series is empty.
     */
    PlotSeries::Point getClosestPointToX(double xValue) const;

    /**
     * @brief Computes metrics (min, max, mean, RMS) within a specified X interval. Thread-safe.
     * @param xMin Starting edge of the analysis window.
     * @param xMax Ending edge of the analysis window.
     */
    PlotSeries::IntervalStats calculateIntervalStats(double xMin, double xMax) const;

   protected:
    /**
     * @struct LodLevel
     * @brief Container for a specific Level of Detail (LOD) tier.
     */
    struct LodLevel {
        std::deque<Point>      points;          /**< Points of this level (level 0 = raw samples). */
        std::deque<BlockStats> stats;           /**< Block statistics (level 1 only). */
        uint64_t               base       = 0;  /**< Absolute index of points.front(). */
        uint64_t               nextSource = 0;  /**< Absolute index of the next unprocessed point of the level below. */
    };

    /** @brief Builds any complete LOD blocks that are pending. Caller holds the lock. */
    void updateLodLevels();

    /** @brief Drops the oldest raw samples beyond the capacity. Caller holds the lock. */
    void enforceCapacity();

    /** @brief Copies points of a level within [lo, hi] into out, falling back to finer levels where the level has no coverage yet. */
    void gatherPoints(size_t level, double lo, double hi, bool loInclusive, bool hiInclusive,
                      std::vector<Point>& out) const;

    mutable std::mutex m_mutex; /**< Protects all sample data below. */

    static constexpr size_t NUM_LOD_LEVELS = 3;   /**< Total number of supported Level of Detail tiers. */
    static constexpr size_t LOD_FACTOR     = 100; /**< Decimation factor between consecutive LOD levels. */

    LodLevel m_lodLevels[NUM_LOD_LEVELS]; /**< LOD tiers. Level 0 holds the raw samples. */
    size_t   m_maxPoints = 0;             /**< Capacity of level 0 (0 = unlimited). */

    // Sliding-window extrema of Y over the stored samples: (absolute index, y).
    std::deque<std::pair<uint64_t, double>> m_yMinQueue;
    std::deque<std::pair<uint64_t, double>> m_yMaxQueue;

    std::atomic<uint64_t> m_version{0}; /**< Bumped on every change. */
    std::atomic<uint64_t> m_dropped{0}; /**< Number of rejected samples. */

    // Render-thread scratch buffers (never touched by producers).
    std::vector<Point> m_gatherBuffer;
    std::vector<Point> m_visibleBuffer;
    struct BucketData {
        Point minPt;
        Point maxPt;
        bool  assigned = false;
    };
    std::vector<BucketData> m_buckets;

    QString m_name;             /**< Name identifier of the series. */
    QColor  m_color;            /**< Rendering color. */
    bool    m_visible   = true; /**< Whether the series is drawn. */
    float   m_lineWidth = 1.5f; /**< Line thickness in logical pixels. */
};

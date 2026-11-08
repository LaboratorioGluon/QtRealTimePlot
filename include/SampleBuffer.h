#pragma once

#include <atomic>
#include <cstdint>
#include <deque>
#include <limits>
#include <mutex>
#include <utility>
#include <vector>

/**
 * @class SampleBuffer
 * @brief Thread-safe, optionally bounded storage of (x, y) samples.
 *
 * Every public method is thread-safe unless documented otherwise: producers can
 * push from any thread while the GUI thread reads.
 *
 * Besides the raw samples it keeps two coarser Level of Detail (LOD) tiers
 * (min/max of every LOD_FACTOR samples) and per-block statistics, so wide
 * windows can be drawn and analysed without touching every sample.
 *
 * Storage is chunked (std::deque), so appending never copies the existing
 * history. When a maximum number of points is configured, the oldest samples
 * are discarded as new ones arrive (ring-buffer behaviour).
 *
 * X values must be non-decreasing. Samples with an X value lower than the last
 * stored one, or with a non-finite coordinate, are rejected and counted in
 * droppedPoints().
 */
class SampleBuffer
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
     * @struct BlockStats
     * @brief Accumulated statistics for one block of LOD_FACTOR raw samples.
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

    /** @param maxPoints Maximum number of raw samples kept. 0 means unlimited. */
    explicit SampleBuffer(size_t maxPoints = 0);
    virtual ~SampleBuffer() = default;

    SampleBuffer(const SampleBuffer&)            = delete;
    SampleBuffer& operator=(const SampleBuffer&) = delete;

    // --- Data ingestion ---

    /** @return False if the sample was rejected (X going backwards or non-finite values). */
    bool pushPoint(double x, double y);
    /** @return False if the sample was rejected (X going backwards or non-finite values). */
    bool pushPoint(const Point& p);

    /** @brief Removes every sample, LOD tier and statistic. */
    void clear();

    /** @param maxPoints New capacity. 0 means unlimited. Oldest samples are discarded first. */
    void   setMaxPoints(size_t maxPoints);
    /** @return The configured capacity (0 = unlimited). */
    size_t maxPoints() const;

    // --- Queries ---

    /** @return Total number of raw samples currently stored. */
    size_t size() const;

    /** @return Number of samples rejected by pushPoint(). */
    uint64_t droppedPoints() const { return m_dropped.load(std::memory_order_relaxed); }

    /** @return Counter that changes whenever the buffer (or a derived class's appearance) changes. Used to replot data. */
    uint64_t version() const { return m_version.load(std::memory_order_relaxed); }

    /** @return The stored point closest to xValue, or {0, 0} if the buffer is empty. */
    Point getClosestPointToX(double xValue) const;

    /** @brief Computes min, max, mean and RMS of the samples with X in [xMin, xMax]. */
    IntervalStats calculateIntervalStats(double xMin, double xMax) const;

    /**
     * @brief Copies the samples needed to draw [xMin, xMax] at a given pixel width.
     *
     * The coarsest LOD tier that still has enough detail for targetWidth is used,
     * falling back to finer tiers where it has no coverage yet (newest samples,
     * or right after old samples were evicted). The lock is held only while copying.
     *
     * The nearest raw sample on each side of the range is also included (when it
     * exists), so a line drawn through the result crosses the whole range even when
     * no sample falls inside it.
     *
     * @param out Receives the points, sorted by X. It is cleared first.
     */
    void copyVisibleRange(double xMin, double xMax, int targetWidth, std::vector<Point>& out) const;

    // --- Raw access (caller must hold lock()) ---

    /** @return A lock on the internal mutex, for the accessors below. */
    std::unique_lock<std::mutex> lock() const;

    /** @warning Must be called while holding lock(). */
    const std::deque<Point>& points() const { return m_lodLevels[0].points; }
    /** @warning Must be called while holding lock(). lod is 0 (raw) to NUM_LOD_LEVELS - 1. */
    const std::deque<Point>& getDataPoints(uint32_t lod = 0) const { return m_lodLevels[lod].points; }

    /** @warning Must be called while holding lock(). */
    double xMin() const;
    /** @warning Must be called while holding lock(). */
    double xMax() const;
    /** @warning Must be called while holding lock(). */
    double yMin() const;
    /** @warning Must be called while holding lock(). */
    double yMax() const;

    static constexpr size_t NUM_LOD_LEVELS = 3;   /**< Number of LOD tiers, including the raw samples. */
    static constexpr size_t LOD_FACTOR     = 100; /**< Decimation factor between consecutive tiers. */

   protected:
    /** @brief Marks the buffer as changed so plots repaint. */
    void bumpVersion() { ++m_version; }

   private:
    struct LodLevel {
        std::deque<Point>      points;         /**< Points of this level (level 0 = raw samples). */
        std::deque<BlockStats> stats;          /**< Block statistics (level 1 only). */
        uint64_t               base       = 0; /**< Absolute index of points.front(). */
        uint64_t               nextSource = 0; /**< Absolute index of the next unprocessed point of the level below. */
    };

    void updateLodLevels();
    void enforceCapacity();
    void gatherPoints(size_t level, double lo, double hi, bool loInclusive, bool hiInclusive,
                      std::vector<Point>& out) const;

    mutable std::mutex m_mutex;

    LodLevel m_lodLevels[NUM_LOD_LEVELS];
    size_t   m_maxPoints = 0;

    // Sliding-window extrema of Y over the stored samples: (absolute index, y).
    std::deque<std::pair<uint64_t, double>> m_yMinQueue;
    std::deque<std::pair<uint64_t, double>> m_yMaxQueue;

    std::atomic<uint64_t> m_version{0};
    std::atomic<uint64_t> m_dropped{0};
};

#pragma once

#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>

/**
 * @class PlotView
 * @brief The visible data range and the pixel rectangle it is drawn into.
 *
 * Owns every data <-> pixel conversion and the zoom, pan and fit arithmetic.
 * It has no widget or OpenGL dependency, so it can be unit-tested on its own.
 * Ranges are never allowed to collapse below the representable precision.
 */
class PlotView
{
   public:
    /** @brief Which axes an operation applies to. */
    enum Axes : unsigned {
        AxisX    = 1u, /**< Horizontal axis only. */
        AxisY    = 2u, /**< Vertical axis only. */
        AxisBoth = 3u  /**< Both axes. */
    };

    /** @brief Part of the widget under a pixel. */
    enum class Region {
        Plot,   /**< Inside the plot rectangle. */
        XAxis,  /**< Below the plot rectangle, horizontally within it. */
        YAxis,  /**< Left of the plot rectangle, vertically within it. */
        Outside /**< Anywhere else (corners, title, right margin). */
    };

    // --- Geometry ---

    /** @param area Plot rectangle in widget pixels. Empty sizes are clamped to 1 px. */
    void  setArea(const QRect& area);
    QRect area() const { return m_area; }

    /** @return The region of the widget that contains px. */
    Region regionAt(QPoint px) const;

    // --- Range ---

    /** @brief Sets the visible range. Inverted bounds are swapped and tiny spans widened. */
    void setRange(double xMin, double xMax, double yMin, double yMax);

    /**
     * @brief Fits the range to data bounds, adding a fraction of the span as padding on each side.
     * A zero span is padded by 5 % of its magnitude, or by 0.5 units when it is exactly at zero.
     */
    void fit(double xMin, double xMax, double yMin, double yMax, double xPadFraction, double yPadFraction);

    double xMin() const { return m_xMin; }
    double xMax() const { return m_xMax; }
    double yMin() const { return m_yMin; }
    double yMax() const { return m_yMax; }
    /** @return x = xMin, y = yMin, width = X span, height = Y span. */
    QRectF range() const { return QRectF(m_xMin, m_yMin, m_xMax - m_xMin, m_yMax - m_yMin); }

    // --- Conversions ---

    QPointF pixelToData(QPoint px) const;
    QPoint  dataToPixel(double x, double y) const;
    double  pixelToX(int px) const;
    int     xToPixel(double x) const;
    /** @return x mapped to [0, 1] across the visible range (may be outside for off-screen values). */
    double  normX(double x) const { return (x - m_xMin) / (m_xMax - m_xMin); }
    /** @return y mapped to [0, 1] across the visible range (may be outside for off-screen values). */
    double  normY(double y) const { return (y - m_yMin) / (m_yMax - m_yMin); }

    // --- Navigation ---

    /** @brief Zooms around a pixel anchor. factor < 1 zooms out, > 1 zooms in. */
    void zoom(double factor, QPoint anchor, Axes axes);
    /** @brief Moves the range so the content follows a mouse drag of deltaPx. */
    void pan(QPoint deltaPx, Axes axes);
    /** @brief Zooms to a rectangle given in widget pixels. */
    void zoomToPixelRect(const QRect& rect, Axes axes);

    /** @brief Widens [lo, hi] if it is narrower than the representable precision; swaps inverted bounds. */
    static void enforceMinSpan(double& lo, double& hi);

   private:
    QRect  m_area{0, 0, 1, 1};
    double m_xMin = 0.0, m_xMax = 1.0;
    double m_yMin = 0.0, m_yMax = 1.0;
};

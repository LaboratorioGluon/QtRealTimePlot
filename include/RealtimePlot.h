#pragma once

#include <QColor>
#include <QFont>
#include <QOpenGLWidget>
#include <QPoint>
#include <QRectF>
#include <QTimer>
#include <deque>
#include <memory>
#include <vector>

#include "PlotAxis.h"
#include "PlotCursor.h"
#include "PlotLegend.h"
#include "PlotRangeCursor.h"
#include "PlotSeries.h"
#include "PlotView.h"

// Deprecated names (RtpCursor, RtpRangeCursor, RtpLegend) kept for existing code.
#include "components/RtpCursor.h"
#include "components/RtpLegend.h"
#include "components/RtpRangeCursor.h"

namespace rtp
{
class PlotRenderer;
}

/**
 * @class RealtimePlot
 * @brief High-performance real-time plot widget using OpenGL.
 *
 * Features:
 * - Multiple series, each pushed from any thread
 * - Wheel zoom (X, Y, or both axes) - zooms toward cursor position
 * - Box-selection zoom (right-click + drag) - with Delta x / Delta y tooltip
 * - Left-click pan / middle-click pan; dragging an axis pans only that axis
 * - Double-click to auto-fit all visible data
 * - Keyboard shortcuts: F/Space=fit, X/Y/B=zoom mode
 * - Optional rolling window that follows the newest X value
 * - Configurable grid, legend, labels, margins, colors
 *
 * The widget only coordinates: PlotView holds the visible range and pixel
 * conversions, an internal renderer does the OpenGL work, and cursors and the
 * legend are PlotOverlay objects drawn on top.
 *
 * @code
 * auto* plot = new RealtimePlot(this);
 * auto  ch1  = plot->addSeries("ADC Ch1", Qt::cyan);
 * plot->setTitle("Real-time ADC");
 * plot->setXLabel("Time (s)");
 * plot->setYLabel("Voltage (V)");
 * plot->setRefreshRate(60);
 * plot->start();
 *
 * // Push from any thread (including ISR callback / QThread):
 * ch1->pushPoint(timestamp, voltage);
 * @endcode
 */
class RealtimePlot : public QOpenGLWidget
{
    Q_OBJECT

   public:
    // ------------------------------------------------------------------ types

    /** @brief Constraints applied to zoom and pan inside the plot area. */
    enum class ZoomMode {
        XY,    /**< Scale both X and Y axes simultaneously. */
        XOnly, /**< Lock Y axis and scale X axis exclusively. */
        YOnly  /**< Lock X axis and scale Y axis exclusively. */
    };

    /** @brief Visual properties of the plot grid background. */
    struct GridStyle {
        bool   enabled = true;               /**< Toggle to draw or hide grid lines. */
        QColor color   = QColor(55, 55, 65); /**< Color of the grid lines. */
    };

    /** @brief Space around the plot area, in logical pixels, for labels and ticks. */
    struct MarginPx {
        int left = 72, right = 18, top = 28, bottom = 52;
    };

    // ------------------------------------------------------------------ ctor

    explicit RealtimePlot(QWidget* parent = nullptr);
    ~RealtimePlot() override;

    // ------------------------------------------------------------------ series

    /**
     * @brief Creates and tracks a new series.
     * @param maxPoints Maximum number of samples kept; older samples are discarded. 0 means unlimited.
     */
    std::shared_ptr<PlotSeries> addSeries(const QString& name, QColor color = Qt::cyan, size_t maxPoints = 10000);

    /** @brief Adds an existing series. A series may be shown in several plots. */
    void addSeries(std::shared_ptr<PlotSeries> serie);
    void removeSeries(const std::shared_ptr<PlotSeries>& series);
    /** @brief Removes every series from the plot. */
    void clearSeries();
    /** @brief Clears the data of every series but keeps the series. */
    void clearSeriesData();

    // ------------------------------------------------------------------ cursors

    /** @brief Adds a cursor in the center of the current view. */
    void addCursor(PlotCursor::MarkerStyle style = PlotCursor::MarkerStyle::MARKER_SIMPLE);
    /** @brief Adds a cursor at xPos (data coordinates). */
    void addCursor(double xPos, PlotCursor::MarkerStyle style = PlotCursor::MarkerStyle::MARKER_SIMPLE);
    /** @brief Adds a cursor with an explicit color. An invalid or transparent color picks one from the palette. */
    void addCursor(double xPos, QColor color, PlotCursor::MarkerStyle style = PlotCursor::MarkerStyle::MARKER_SIMPLE);

    /** @param unit Unit appended to the values shown by full-style cursors (e.g. "V"). */
    void setCursorUnit(const QString& unit);

    void             setRangeCursorEnable(bool enable);
    void             setRangeCursorPos(double start, double end);
    void             enableRangeCursor(bool enable) { setRangeCursorEnable(enable); }
    /**
     * @return The range cursor.
     * @note Call update() after modifying it directly so the change is repainted.
     */
    PlotRangeCursor* getRangeCursor() { return &m_cursorRange; }

    // ------------------------------------------------------------------ view

    /** @brief Fits the view to all visible data. */
    void            autoFit();
    /** @brief Shows the last rolling-window width of data (see setAutoZoom()). */
    void            autoFitRolling();
    /** @brief Sets the visible range. Ranges narrower than the representable precision are widened. */
    void            setViewRange(double xMin, double xMax, double yMin, double yMax);
    /** @return x = xMin, y = yMin, width = X span, height = Y span. */
    QRectF          viewRange() const { return m_view.range(); }
    /** @return The view (range, plot rectangle and conversions). */
    const PlotView& view() const { return m_view; }

    // ------------------------------------------------------------------ config

    /** @param fps Maximum repaint frequency. */
    void setRefreshRate(int fps);
    /** @brief Starts the refresh clock. A frame is only repainted when a series changed. */
    void start();
    void stop();

    void     setZoomMode(ZoomMode mode) { m_zoomMode = mode; }
    ZoomMode zoomMode() const { return m_zoomMode; }

    void      setGrid(const GridStyle& g);
    GridStyle getGrid() const { return m_grid; }
    void      setMargins(const MarginPx& m);

    void setBackgroundColor(QColor c);
    void setAxisColor(QColor c);
    void setTextColor(QColor c);

    void setTitle(const QString& t);
    void setXLabel(const QString& l);
    void setYLabel(const QString& l);

    void setLegendVisible(bool v);

    /**
     * @param a Whether the view follows the data automatically.
     * @param window If greater than 0, the view shows only the last window X units.
     */
    void setAutoZoom(bool a, double window = 0.0);
    bool autoZoom() const { return m_zoomAuto; }

   signals:
    /** @brief Emitted whenever the visible range changes. */
    void viewChanged(double xMin, double xMax, double yMin, double yMax);
    /** @brief Emitted when a series was shown or hidden from the legend. */
    void visibilityChanged();

   protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void resizeEvent(QResizeEvent* event) override;

    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

   private:
    /** @brief The single mouse-interaction state. Only one gesture can be active at a time. */
    struct Interaction {
        enum class State {
            Idle,           /**< No button held. */
            Panning,        /**< Dragging the content. */
            BoxSelecting,   /**< Drawing a zoom rectangle. */
            DraggingOverlay /**< An overlay (cursor, legend) owns the drag. */
        };
        State          state   = State::Idle;
        PlotView::Axes panAxes = PlotView::AxisBoth; /**< Axes moved while Panning. */
        QPoint         lastPos;                      /**< Last mouse position while Panning. */
        QPoint         selStart, selEnd;             /**< Rectangle corners while BoxSelecting. */
        PlotOverlay*   overlay = nullptr;            /**< Owner of the drag while DraggingOverlay. */
    };

    QRect                     plotArea() const;
    void                      syncViewArea();
    PlotContext               overlayContext() const { return PlotContext{m_view, m_series}; }
    /** @return Overlays from bottom to top. */
    std::vector<PlotOverlay*> overlays();
    PlotView::Axes            zoomAxes() const;

    bool     dataBounds(double& xMin, double& xMax, double& yMin, double& yMax) const;
    uint64_t dataVersion() const;
    void     viewModified();
    void     paintLabels(QPainter& painter, const PlotAxis::TickSet& xTicks, const PlotAxis::TickSet& yTicks);
    void     paintSelection(QPainter& painter);
    void     cleanupGL();

    // --- Data and view ---
    std::vector<std::shared_ptr<PlotSeries>> m_series;
    PlotView                                 m_view;
    bool                                     m_viewInitialized = false;
    bool                                     m_inPaint         = false; /**< Suppresses update() while painting. */

    // --- Style ---
    QColor    m_bgColor{18, 18, 22};
    QColor    m_axisColor{120, 120, 130};
    QColor    m_textColor{200, 200, 210};
    GridStyle m_grid;
    MarginPx  m_margin;
    QFont     m_tickFont;
    QFont     m_labelFont;
    QString   m_title;
    PlotAxis  m_xAxis{80};
    PlotAxis  m_yAxis{50};

    // --- Behavior ---
    ZoomMode m_zoomMode   = ZoomMode::XY;
    bool     m_zoomAuto   = true;
    double   m_zoomWindow = 0.0; /**< Rolling window width in X units (0 = fit all data). */

    Interaction m_interaction;

    // --- Rendering ---
    QTimer*                            m_timer = nullptr; /**< Refresh clock; repaints only when data changed. */
    std::unique_ptr<rtp::PlotRenderer> m_renderer;
    uint64_t                           m_paintedVersion = ~uint64_t(0); /**< dataVersion() at the last paint. */

    // --- Overlays ---
    std::deque<PlotCursor> m_cursors; /**< A deque keeps references stable while adding. */
    PlotRangeCursor        m_cursorRange;
    QString                m_cursorUnit;
    PlotLegend             m_legend;

    static const std::vector<QColor> colorList; /**< Palette for cursors created without a color. */
};

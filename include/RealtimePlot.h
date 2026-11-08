#pragma once

#include <QColor>
#include <QFont>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QPoint>
#include <QRectF>
#include <QTimer>
#include <QVector>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

#include "PlotAxis.h"
#include "PlotSeries.h"
#include "components/RtpCursor.h"
#include "components/RtpLegend.h"
#include "components/RtpRangeCursor.h"

/**
 * @class RealtimePlot
 * @brief High-performance real-time plot widget using OpenGL.
 *
 * Features:
 * - Multiple series, each pushed from any thread
 * - Wheel zoom (X, Y, or both axes) - zooms toward cursor position
 * - Box-selection zoom (right-click + drag) - with Delta x / Delta y tooltip
 * - Left-click pan / middle-click pan
 * - Double-click to auto-fit all visible data
 * - Keyboard shortcuts: F/Space=fit, X/Y/B=zoom mode, S=auto-scroll
 * - Optional auto-scroll mode (follows newest X value)
 * - Configurable grid, legend, labels, margins, colors
 *
 * @code
 * auto* plot = new RealtimePlot(this);
 * auto  ch1  = plot->addSeries("ADC Ch1", Qt::cyan);
 * auto  ch2  = plot->addSeries("ADC Ch2", Qt::yellow);
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
class RealtimePlot : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

   public:
    // ------------------------------------------------------------------ types

    /**
     * @enum ZoomMode
     * @brief Constraints applied to zoom transformations.
     */
    enum class ZoomMode {
        XY,    /**< Scale both X and Y axes simultaneously. */
        XOnly, /**< Lock Y axis and scale X axis exclusively. */
        YOnly  /**< Lock X axis and scale Y axis exclusively. */
    };

    /**
     * @struct GridStyle
     * @brief Visual properties of the plot grid background.
     */
    struct GridStyle {
        bool   enabled = true;               /**< Toggle to draw or hide grid lines. */
        QColor color   = QColor(55, 55, 65); /**< RGBA color assigned to the grid lines. */
    };

    /**
     * @struct MarginPx
     * @brief Margin padding around the main viewport drawing area in screen pixels.
     */
    struct MarginPx {
        int left = 72, right = 18, top = 28, bottom = 52;
    };

    /**
     * @enum CursorType
     * @brief Identifiers for interactive cursors to manage focus and selection.
     */
    enum class CursorType {
        None, /**< No cursor is currently selected or hovered. */
        X1,   /**< Primary vertical cursor (X axis split). */
        X2,   /**< Secondary vertical cursor (X axis split). */
        Y1,   /**< Primary horizontal cursor (Y axis split). */
        Y2    /**< Secondary horizontal cursor (Y axis split). */
    };

    // ------------------------------------------------------------------ ctor

    /**
     * @brief Constructor for the high-performance OpenGL plot widget.
     * @param parent Pointer to parent Qt widget container.
     */
    explicit RealtimePlot(QWidget* parent = nullptr);

    /**
     * @brief Destructor. Safely deallocates GL shaders, objects, and timers.
     */
    ~RealtimePlot() override;

    // ------------------------------------------------------------------ series

    /**
     * @brief Dynamically creates and tracks a new plot data channel.
     * @param name Unique label identifier for the series.
     * @param color Default line render color profile (Default: Qt::cyan).
     * @param maxPoints Maximum number of samples kept; older samples are discarded. 0 means unlimited.
     * @return Shared pointer reference to the created PlotSeries instance.
     */
    std::shared_ptr<PlotSeries> addSeries(const QString& name, QColor color = Qt::cyan, size_t maxPoints = 10000);

    /**
     * @brief Adds a pre-existing externally built series into the rendering manager.
     * @param serie Shared pointer reference to the managed plot series. A series may be shown in several plots.
     */
    void addSeries(std::shared_ptr<PlotSeries> serie);

    /**
     * @brief Deletes a targeted channel from the workspace.
     * @param series Shared pointer reference to the target channel.
     */
    void removeSeries(const std::shared_ptr<PlotSeries>& series);

    /**
     * @brief Completely drops all tracking datasets from the widget memory space.
     */
    void clearSeries();

    /**
     * @brief Clear the series data but not the series themselves
     */
    void clearSeriesData();

    // ------------------------------------------------------------------ cursors

    /**
     * @brief Add cursor in the center of the current view.
     * @param style type of cursor to add.
     */
    void addCursor(RtpCursor::MarkerStyle style = RtpCursor::MarkerStyle::MARKER_SIMPLE);

    /**
     * @brief Add cursor in position.
     * @param xPos Position in data coordinates to place the cursor
     * @param style type of cursor to add.
     */
    void addCursor(double xPos, RtpCursor::MarkerStyle style = RtpCursor::MarkerStyle::MARKER_SIMPLE);

    /**
     * @brief Add cursor in position with an explicit color.
     * @param xPos Position in data coordinates to place the cursor.
     * @param color Cursor color. An invalid or transparent color picks one from the internal palette.
     * @param style type of cursor to add.
     */
    void addCursor(double xPos, QColor color, RtpCursor::MarkerStyle style = RtpCursor::MarkerStyle::MARKER_SIMPLE);

    /**
     * @brief Sets the unit appended to the values shown by full-style cursors (e.g. "V", "pA").
     * @param unit Unit text. Applies to existing and future cursors.
     */
    void setCursorUnit(const QString& unit);

    void setRangeCursorEnable(bool enable)
    {
        m_cursorRange.enable(enable);
        update();
    }
    void setRangeCursorPos(double start, double end)
    {
        m_cursorRange.setPosStart(start);
        m_cursorRange.setPosEnd(end);
        update();
    };

    // ------------------------------------------------------------------ view

    /**
     * @brief Computes data boundaries and updates viewport range parameters to fit content.
     */
    void autoFit();

    /**
     * @brief Shows the last rolling-window width of data (see setAutoZoom()).
     */
    void autoFitRolling();

    /**
     * @brief Explicitly overrides limits of the observable data window.
     * Ranges narrower than the representable precision are widened around their center.
     * @param xMin Lowest bounding value along the X scale.
     * @param xMax Highest bounding value along the X scale.
     * @param yMin Lowest bounding value along the Y scale.
     * @param yMax Highest bounding value along the Y scale.
     */
    void setViewRange(double xMin, double xMax, double yMin, double yMax);

    /**
     * @brief Retrieves a floating-point bounding rectangle representing current view extents.
     * @return QRectF containment envelope where width=X scope and height=Y scope.
     */
    QRectF viewRange() const;

    // ------------------------------------------------------------------ config

    /**
     * @brief Programs the maximum repaint frequency.
     * @param fps Frames Per Second target (e.g., 60).
     */
    void setRefreshRate(int fps);

    /**
     * @brief Starts the refresh clock. A frame is only repainted when a series changed.
     */
    void start();

    /**
     * @brief Suspends frame clock ticks to freeze screen evaluation updates.
     */
    void stop();

    /** @param mode Desired structural constraints for handling zooming gestures. */
    void     setZoomMode(ZoomMode mode) { m_zoomMode = mode; }
    /** @return Current behavioral zoom constraint mode active on the canvas. */
    ZoomMode zoomMode() const { return m_zoomMode; }

    /** @param g Custom style properties structure payload to write into background grid. */
    void setGrid(const GridStyle& g)
    {
        m_grid = g;
        update();
    }
    /** @return Current active style metadata copy assigned to the grid lines. */
    GridStyle getGrid() const { return m_grid; }
    /** @param m Custom pixel padding configuration metrics array to isolate axes. */
    void setMargins(const MarginPx& m)
    {
        m_margin = m;
        update();
    }

    /** @param c Solid background fill color applied during frame clearing pass. */
    void setBackgroundColor(QColor c)
    {
        m_bgColor = c;
        update();
    }
    /** @param c Color indicator mapped onto primary axis border rule boundaries. */
    void setAxisColor(QColor c)
    {
        m_axisColor = c;
        update();
    }
    /** @param c Master color context parameter applied onto drawing tick numerals. */
    void setTextColor(QColor c)
    {
        m_textColor = c;
        update();
    }

    /** @param t Descriptive text string printed globally at the widget crown. */
    void setTitle(const QString& t)
    {
        m_title = t;
        update();
    }
    /** @param l Unit/description identifier string centered under horizontal axis. */
    void setXLabel(const QString& l)
    {
        m_xLabel = l;
        update();
    }
    /** @param l Unit/description identifier string mapped near vertical layout line. */
    void setYLabel(const QString& l)
    {
        m_yLabel = l;
        update();
    }

    /** @param enable Toggles state of the specialized range cursor subcomponent. */
    void enableRangeCursor(bool enable) { setRangeCursorEnable(enable); }
    /**
     * @return Pointer to internal range cursor.
     * @note Call update() after modifying it directly so the change is repainted.
     */
    RtpRangeCursor* getRangeCursor() { return &m_cursorRange; }

    /** @param v Toggles visibility of the interactive serial descriptor panel. */
    void setLegendVisible(bool v)
    {
        m_legendVisible = v;
        update();
    }

    /** @param a Sets whether auto-zoom evaluation passes execute automatically.
     * @param window If greater than 0, this defines the rolling window width in X units.
     */
    void setAutoZoom(bool a, double window = 0.0)
    {
        m_zoomAuto   = a;
        m_zoomWindow = window > 0.0 ? window : 0.0;
        update();
    }
    bool autoZoom() const { return m_zoomAuto; }

   signals:
    /**
     * @brief Signal fired whenever visible data window view boundaries are adjusted.
     * @param xMin New absolute low limit parameter on X domain.
     * @param xMax New absolute high limit parameter on X domain.
     * @param yMin New absolute low limit parameter on Y domain.
     * @param yMax New absolute high limit parameter on Y domain.
     */
    void viewChanged(double xMin, double xMax, double yMin, double yMax);

    void visibilityChanged();

   protected:
    // --- Qt OpenGL Interface Overrides ---
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // --- Interactive User Input Overrides ---
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

   private:
    /**
     * @struct GpuBuffer
     * @brief VBO + VAO pair owned by this widget's GL context.
     */
    struct GpuBuffer {
        QOpenGLBuffer            vbo{QOpenGLBuffer::VertexBuffer};
        QOpenGLVertexArrayObject vao;
        int                      capacityBytes = 0;
    };

    // Coordinate helpers

    /**
     * @brief Transforms local widget integer window coordinates into absolute data unit values.
     * @param px Integer position relative to the local widget frame coordinate map.
     * @return Real calculated QPointF coordinate structure floating points.
     */
    QPointF pixelToData(QPoint px) const;

    /**
     * @brief Transforms absolute database values into target local integer frame layout coordinates.
     * @param x Absolute internal horizontal coordinate tracker trace.
     * @param y Absolute internal vertical coordinate tracker trace.
     * @return QPoint mapping position in widget screen coordinate pixel space.
     */
    QPoint dataToPixel(double x, double y) const;

    /**
     * @brief Extracts bounding geometry dimensions isolating the clean graph center core.
     * @return Screen coordinate integer geometry box wrapper.
     */
    QRect plotArea() const;

    /** @brief Union of the bounds of all visible series. Returns false if there is no data. */
    bool dataBounds(double& xMin, double& xMax, double& yMin, double& yMax) const;

    /** @brief Widens [lo, hi] if it is narrower than the representable precision. */
    static void enforceMinSpan(double& lo, double& hi);

    /** @brief Sum of the version counters of all series. */
    uint64_t dataVersion() const;

    // Draw passes
    void drawGrid(const QRect& area, const QVector<PlotAxis::Tick>& xTicks, const QVector<PlotAxis::Tick>& yTicks);
    void drawSeries(const QRect& area);
    void drawAxes(const QRect& area, const QVector<PlotAxis::Tick>& xTicks, const QVector<PlotAxis::Tick>& yTicks);

    // GL primitives (coordinates normalized to the plot area, [0, 1])
    void drawPrimitive(GpuBuffer& buf, const float* xy, int vertexCount, GLenum mode, QColor color, float widthPx,
                       const QRect& area);
    void uploadToBuffer(GpuBuffer& buf, const void* data, int bytes);
    GpuBuffer& gpuBufferFor(const PlotSeries* series);
    void cleanupGL();

    void applyZoom(double factor, QPoint anchor);
    void emitViewChanged();

    // --- Data ---
    std::vector<std::shared_ptr<PlotSeries>> m_series; /**< Master data collection vector of tracked display streams. */

    // --- View state ---
    double m_xMin = 0.0, m_xMax = 1.0; /**< Horizontal observation limit scope metrics. */
    double m_yMin = 0.0, m_yMax = 1.0; /**< Vertical observation limit scope metrics. */
    bool   m_viewInitialized = false;  /**< Safety control flag checking viewport validity. */
    bool   m_inPaint         = false;  /**< True while paintGL() runs; suppresses re-entrant update() calls. */

    // --- Style ---
    QColor    m_bgColor{18, 18, 22};       /**< Viewport baseline core wash clearing background fill color. */
    QColor    m_axisColor{120, 120, 130};  /**< Primary reference edge line boundary color mapping indicator. */
    QColor    m_textColor{200, 200, 210};  /**< Numeric label and heading string font canvas printing layout color. */
    GridStyle m_grid;                      /**< Grid operational style definition map metadata payload. */
    MarginPx  m_margin;                    /**< Functional margin offset allocation context matrix parameters. */
    QFont     m_tickFont;                  /**< Font profile styling context configuration applied on axis numerals. */
    QFont     m_labelFont;                 /**< Font profile styling context configuration applied on main titles. */
    QString   m_title, m_xLabel, m_yLabel; /**< Axis and plot header descriptive text label string records. */
    bool      m_legendVisible = true;      /**< Visibility override switch state assigned to control panel overlay. */

    // --- Behavior ---
    ZoomMode m_zoomMode   = ZoomMode::XY; /**< Positional scaling locking parameter selection profile. */
    bool     m_zoomAuto   = true;         /**< Automation tracking configuration state override directive. */
    double   m_zoomWindow = 0.0;          /**< Rolling window width in X units (0 = fit all data). */

    // --- Interaction ---
    bool   m_panning   = false; /**< Intermittent active click drag translation tracking state indicator. */
    ZoomMode m_panAxes = ZoomMode::XY; /**< Axes moved by the current pan (XOnly/YOnly when dragging an axis). */
    bool   m_selecting = false; /**< Intermittent active click drag window box boundary select zoom indicator. */
    QPoint m_lastMousePos;      /**< Intermediate tracking position coordinate of historical mouse move cursor event. */
    QPoint m_selStart, m_selEnd; /**< Screen space box boundaries tracking indices for selection boxes. */

    // --- GL ---
    QTimer*               m_timer  = nullptr; /**< Refresh clock; repaints only when data changed. */
    QOpenGLShaderProgram* m_shader = nullptr; /**< Compiled OpenGL program. */
    bool     m_shaderReady         = false;   /**< True once the shader program is linked. */
    float    m_maxLineWidth        = 1.0f;    /**< Widest line the driver can draw natively. */
    uint64_t m_paintedVersion      = ~uint64_t(0); /**< dataVersion() at the last paint. */
    std::unordered_map<const PlotSeries*, std::unique_ptr<GpuBuffer>> m_seriesGpu; /**< Per-series GPU buffers. */
    std::unique_ptr<GpuBuffer> m_overlayGpu;          /**< Buffer for grid, border and ticks. */
    std::vector<PlotSeries::NormalizedPoint> m_normScratch; /**< Reused per-frame vertex staging. */
    std::vector<float>                       m_triScratch;  /**< Reused wide-line triangle staging. */

    // --- Cursors ---
    std::deque<RtpCursor> m_cursors;     /**< User cursors. A deque keeps references stable while adding. */
    RtpRangeCursor        m_cursorRange; /**< Specialized range tracking cursor evaluation context module component. */
    QString               m_cursorUnit;  /**< Unit shown by full-style cursors. */

    // Drag management
    RtpCursor* m_activeCursorRef = nullptr; /**< Cursor currently being dragged. */
    const int  m_clickTolerancePixels = 7;  /**< Click bounding halo tolerance scope diameter context scale. */

    RtpLegend m_legend; /**< Secondary render presentation layer mapping data streams to labels. */

    static const std::vector<QColor>
        colorList; /**< Palette definition array used to automate color assignment layers. */
};

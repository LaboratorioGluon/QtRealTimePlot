#pragma once

#include <QColor>
#include <QFont>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QPoint>
#include <QRectF>
#include <QTimer>
#include <QVector>
#include <memory>
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
        bool   enabled = true; /**< Toggle to draw or hide grid lines. */
        QColor color =
            QColor(55, 55, 65); /**< RGBA color assigned to the grid lines. */
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
     * @param maxPoints Upper boundary capacity threshold for internal circular buffer.
     * @return Shared pointer reference to the created PlotSeries instance.
     */
    std::shared_ptr<PlotSeries> addSeries(const QString& name,
                                          QColor         color     = Qt::cyan,
                                          size_t         maxPoints = 10000);

    /**
     * @brief Adds a pre-existing externally built series into the rendering manager.
     * @param serie Shared pointer reference to the managed plot series.
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

    // ------------------------------------------------------------------ cursors

    void addCursor();
    void addCursor(double xPos);

    // ------------------------------------------------------------------ view

    /**
     * @brief Computes data boundaries and updates viewport range parameters to fit content.
     */
    void autoFit();

    /**
     * @brief Explicitly overrides limits of the observable data window.
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
     * @brief Programs the target periodic timer update frequency.
     * @param fps Frames Per Second target (e.g., 60).
     */
    void setRefreshRate(int fps);

    /**
     * @brief Activates the rendering frame clock loop to poll and repaint new data points.
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
    void      setGrid(const GridStyle& g) { m_grid = g; }
    /** @return Current active style metadata copy assigned to the grid lines. */
    GridStyle getGrid() { return m_grid; }
    /** @param m Custom pixel padding configuration metrics array to isolate axes. */
    void      setMargins(const MarginPx& m) { m_margin = m; }

    /** @param c Solid background fill color applied during frame clearing pass. */
    void setBackgroundColor(QColor c) { m_bgColor = c; }
    /** @param c Color indicator mapped onto primary axis border rule boundaries. */
    void setAxisColor(QColor c) { m_axisColor = c; }
    /** @param c Master color context parameter applied onto drawing tick numerals. */
    void setTextColor(QColor c) { m_textColor = c; }

    /** @param t Descriptive text string printed globally at the widget crown. */
    void setTitle(const QString& t) { m_title = t; }
    /** @param l Unit/description identifier string centered under horizontal axis. */
    void setXLabel(const QString& l) { m_xLabel = l; }
    /** @param l Unit/description identifier string mapped near vertical layout line. */
    void setYLabel(const QString& l) { m_yLabel = l; }

    /**
     * @brief Adds an analytical marker tracking point onto the plot workspace.
     * @param xValue Absolute coordinate localization target point value on X scale.
     * @param color Palette color tag assigned to distinguish marker line (Default: transparent).
     * @param style Shape layout decoration applied onto cursor anchor target points.
     */
    void addCursor(
        float xValue, QColor color = Qt::transparent,
        RtpCursor::MarkerStyle style = RtpCursor::MarkerStyle::MARKER_SIMPLE)
    {
        m_cursors.push_back(
            RtpCursor(m_cursors.size(),
                      color == Qt::transparent
                          ? colorList[m_cursors.size() % (colorList.size())]
                          : color,
                      style));
    };

    /** @param enable Toggles state of the specialized range cursor subcomponent. */
    void enableRangeCursor(bool enable) { m_cursorRange.enable(enable); }
    /** @return Pointer to internal range validation tracking subcomponent structure. */
    RtpRangeCursor* getRangeCursor() { return &m_cursorRange; }

    /** @param v Toggles visibility of the interactive serial descriptor panel. */
    void setLegendVisible(bool v) { m_legendVisible = v; }

    /** @param a Sets whether auto-zoom evaluation passes execute automatically. */
    void setAutoZoom(bool a) { m_zoomAuto = a; }

   signals:
    /**
     * @brief Signal fired whenever visible data window view boundaries are adjusted.
     * @param xMin New absolute low limit parameter on X domain.
     * @param xMax New absolute high limit parameter on X domain.
     * @param yMin New absolute low limit parameter on Y domain.
     * @param yMax New absolute high limit parameter on Y domain.
     */
    void viewChanged(double xMin, double xMax, double yMin, double yMax);

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

    // Draw passes
    void drawGrid(const QRect& area, const QVector<PlotAxis::Tick>& xTicks,
                  const QVector<PlotAxis::Tick>& yTicks);
    void drawSeries(const QRect& area);
    void drawAxes(const QRect& area, const QVector<PlotAxis::Tick>& xTicks,
                  const QVector<PlotAxis::Tick>& yTicks);

    // GL primitives (pixel-space coords)
    void drawLineStrip(const std::vector<float>& verts, QColor color,
                       float width);
    void drawLines(const std::vector<float>& verts, QColor color, float width);

    void applyZoom(double factor, QPoint anchor);
    void emitViewChanged();

    // --- Data ---
    std::vector<std::shared_ptr<PlotSeries>>
        m_series; /**< Master data collection vector of tracked display streams. */

    // --- View state ---
    double m_xMin = 0.0,
           m_xMax = 1.0; /**< Horizontal observation limit scope metrics. */
    double m_yMin = 0.0,
           m_yMax = 1.0; /**< Vertical observation limit scope metrics. */
    bool m_viewInitialized =
        false; /**< Safety control flag checking viewport validity. */

    // --- Style ---
    QColor m_bgColor{
        18, 18,
        22}; /**< Viewport baseline core wash clearing background fill color. */
    QColor m_axisColor{
        120, 120,
        130}; /**< Primary reference edge line boundary color mapping indicator. */
    QColor m_textColor{
        200, 200,
        210}; /**< Numeric label and heading string font canvas printing layout color. */
    GridStyle
        m_grid; /**< Grid operational style definition map metadata payload. */
    MarginPx
        m_margin; /**< Functional margin offset allocation context matrix parameters. */
    QFont
        m_tickFont; /**< Font profile styling context configuration applied on axis numerals. */
    QFont
        m_labelFont; /**< Font profile styling context configuration applied on main titles. */
    QString m_title, m_xLabel,
        m_yLabel; /**< Axis and plot header descriptive text label string records. */
    bool m_legendVisible =
        true; /**< Visibility override switch state assigned to control panel overlay. */

    // --- Behavior ---
    ZoomMode m_zoomMode = ZoomMode::
        XY; /**< Positional scaling locking parameter selection profile. */
    bool m_zoomAuto =
        true; /**< Automation tracking configuration state override directive. */

    // --- Interaction ---
    bool m_panning =
        false; /**< Intermittent active click drag translation tracking state indicator. */
    bool m_selecting =
        false; /**< Intermittent active click drag window box boundary select zoom indicator. */
    QPoint
        m_lastMousePos; /**< Intermediate tracking position coordinate of historical mouse move cursor event. */
    QPoint m_selStart,
        m_selEnd; /**< Screen space box boundaries tracking indices for selection boxes. */

    // --- GL ---
    QTimer* m_timer =
        nullptr; /**< Internal refresh loop triggering clock interval execution manager. */
    QOpenGLShaderProgram* m_shader =
        nullptr; /**< Compiled OpenGL program binding primitive geometry layout logic. */
    bool m_shaderReady =
        false; /**< Pipeline validation control flag ensuring compilation stability. */

    // --- Cursors ---
    std::vector<RtpCursor>
        m_cursors; /**< Extended sequence payload containing dynamic user cursor additions. */
    RtpRangeCursor
        m_cursorRange; /**< Specialized range tracking cursor evaluation context module component. */

    // Drag management
    /**< Active cursor selection tracking identifier context mode. */
    CursorType m_activeCursor = CursorType::None;
    RtpCursor* m_activeCursorRef =
        nullptr; /**< Direct memory reference address to dragged object context instance. */
    const int m_clickTolerancePixels =
        7; /**< Click bounding halo tolerance scope diameter context scale. */

    RtpLegend
        m_legend; /**< Secondary render presentation layer mapping data streams to labels. */

    static const std::vector<QColor>
        colorList; /**< Palette definition array used to automate color assignment layers. */
};
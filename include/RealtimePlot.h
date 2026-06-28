#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QPoint>
#include <QRectF>
#include <QColor>
#include <QFont>
#include <QVector>
#include <memory>
#include <vector>

#include "PlotSeries.h"
#include "PlotAxis.h"
#include "components/RtpLegend.h"
#include "components/RtpCursor.h"
#include "components/RtpRangeCursor.h"

/**
 * @brief High-performance real-time plot widget using OpenGL.
 *
 * Features:
 *   - Multiple series, each pushed from any thread
 *   - Wheel zoom (X, Y, or both axes) — zooms toward cursor position
 *   - Box-selection zoom (right-click + drag) — with Δx/Δy tooltip
 *   - Left-click pan / middle-click pan
 *   - Double-click to auto-fit all visible data
 *   - Keyboard shortcuts: F/Space=fit, X/Y/B=zoom mode, S=auto-scroll
 *   - Optional auto-scroll mode (follows newest X value)
 *   - Configurable grid, legend, labels, margins, colors
 *
 * @code
 *   auto* plot = new RealtimePlot(this);
 *   auto  ch1  = plot->addSeries("ADC Ch1", Qt::cyan);
 *   auto  ch2  = plot->addSeries("ADC Ch2", Qt::yellow);
 *   plot->setTitle("Real-time ADC");
 *   plot->setXLabel("Time (s)");
 *   plot->setYLabel("Voltage (V)");
 *   plot->setRefreshRate(60);
 *   plot->start();
 *
 *   // Push from any thread (including ISR callback / QThread):
 *   ch1->pushPoint(timestamp, voltage);
 * @endcode
 */
class RealtimePlot : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------ types
    enum class ZoomMode
    {
        XY,
        XOnly,
        YOnly
    };

    struct GridStyle
    {
        bool enabled = true;
        QColor color = QColor(55, 55, 65);
    };

    struct MarginPx
    {
        int left = 72, right = 18, top = 28, bottom = 52;
    };

    enum class CursorType
    {
        None,
        X1,
        X2,
        Y1,
        Y2
    };

    // ------------------------------------------------------------------ ctor
    explicit RealtimePlot(QWidget *parent = nullptr);
    ~RealtimePlot() override;

    // ------------------------------------------------------------------ series
    std::shared_ptr<PlotSeries> addSeries(const QString &name,
                                          QColor color = Qt::cyan,
                                          size_t maxPoints = 10000);
    void addSeries(std::shared_ptr<PlotSeries> serie);
    void removeSeries(const std::shared_ptr<PlotSeries> &series);
    void clearSeries();

    // ------------------------------------------------------------------ view
    void autoFit();
    void setViewRange(double xMin, double xMax, double yMin, double yMax);
    QRectF viewRange() const;

    // ------------------------------------------------------------------ config
    void setRefreshRate(int fps);
    void start();
    void stop();

    void setZoomMode(ZoomMode mode) { m_zoomMode = mode; }
    ZoomMode zoomMode() const { return m_zoomMode; }

    void setGrid(const GridStyle &g) { m_grid = g; }
    GridStyle getGrid() { return m_grid; }
    void setMargins(const MarginPx &m) { m_margin = m; }

    void setBackgroundColor(QColor c) { m_bgColor = c; }
    void setAxisColor(QColor c) { m_axisColor = c; }
    void setTextColor(QColor c) { m_textColor = c; }

    void setTitle(const QString &t) { m_title = t; }
    void setXLabel(const QString &l) { m_xLabel = l; }
    void setYLabel(const QString &l) { m_yLabel = l; }

    void enableCursors(bool enableCursor1, bool enableCursor2) { m_showXCursors = enableCursor1; }
    void enableRangeCursor(bool enable) { m_cursorRange.enable(enable); }
    RtpRangeCursor *getRangeCursor() { return &m_cursorRange; }

    void setLegendVisible(bool v) { m_legendVisible = v; }

    void setAutoZoom(bool a) { m_zoomAuto = a; }

signals:
    void viewChanged(double xMin, double xMax, double yMin, double yMax);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // Coordinate helpers
    QPointF pixelToData(QPoint px) const;
    QPoint dataToPixel(double x, double y) const;
    QRect plotArea() const;

    // Draw passes
    void drawGrid(const QRect &area,
                  const QVector<PlotAxis::Tick> &xTicks,
                  const QVector<PlotAxis::Tick> &yTicks);
    void drawSeries(const QRect &area);
    void drawAxes(const QRect &area,
                  const QVector<PlotAxis::Tick> &xTicks,
                  const QVector<PlotAxis::Tick> &yTicks);

    // GL primitives (pixel-space coords)
    void drawLineStrip(const std::vector<float> &verts, QColor color, float width);
    void drawLines(const std::vector<float> &verts, QColor color, float width);

    void applyZoom(double factor, QPoint anchor);
    void emitViewChanged();

    // --- Data ---
    std::vector<std::shared_ptr<PlotSeries>> m_series;

    // --- View state ---
    double m_xMin = 0.0, m_xMax = 1.0;
    double m_yMin = 0.0, m_yMax = 1.0;
    bool m_viewInitialized = false;

    // --- Style ---
    QColor m_bgColor{18, 18, 22};
    QColor m_axisColor{120, 120, 130};
    QColor m_textColor{200, 200, 210};
    GridStyle m_grid;
    MarginPx m_margin;
    QFont m_tickFont;
    QFont m_labelFont;
    QString m_title, m_xLabel, m_yLabel;
    bool m_legendVisible = true;

    // --- Behavior ---
    ZoomMode m_zoomMode = ZoomMode::XY;
    bool m_zoomAuto = true;

    // --- Interaction ---
    bool m_panning = false;
    bool m_selecting = false;
    QPoint m_lastMousePos;
    QPoint m_selStart, m_selEnd;

    // --- GL ---
    QTimer *m_timer = nullptr;
    QOpenGLShaderProgram *m_shader = nullptr;
    bool m_shaderReady = false;

    // --- Cursors ---

    RtpCursor m_cursorsX[2];
    RtpRangeCursor m_cursorRange;

    bool m_showXCursors = true;
    bool m_showYCursors = false;

    // Gestión del arrastre
    CursorType m_activeCursor = CursorType::None;
    RtpCursor *m_activeCursorRef = nullptr;
    const int m_clickTolerancePixels = 7;

    RtpLegend m_legend;
};

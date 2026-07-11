#ifndef RTPCURSOR_H__
#define RTPCURSOR_H__

#include <QMouseEvent>
#include <QPainter>
#include <QVector>
#include <QWheelEvent>

#include "PlotSeries.h"

/**
 * @class RtpCursor
 * @brief Manages a single interactive measurement cursor overlay.
 *
 * This class handles the rendering, positioning, and mouse interaction 
 * (hover, selection, dragging) for an individual vertical tracking cursor. 
 * It can render context labels showing raw data values intersecting at its current X coordinate.
 */
class RtpCursor
{
   public:
    /**
     * @enum MarkerStyle
     * @brief Visual rendering profiles for the cursor marker and its associated readout badge.
     */
    enum MarkerStyle {
        MARKER_FULL =
            0, /**< Draws a line stretching across the plot viewport accompanied by a descriptive label badge. */
        MARKER_SIMPLE /**< Draws an abbreviated minimalist handle indicator on the axis line. */
    };

    /**
     * @brief Constructs a single tracker cursor instance.
     * @param index Numeric order sequence tag tracking this identifier.
     * @param color Primary coloring profile configuration applied onto lines and icons.
     * @param style Presentation rendering geometry profile layout switch (Default: MARKER_FULL).
     */
    RtpCursor(int index, QColor color,
              MarkerStyle style = MarkerStyle::MARKER_FULL);

    /**
     * @brief Pairs the cursor context directly to a tracked sequence dataset list.
     * @param series Constant pointer targeting the shared data channels tracking array vector.
     */
    void setSeriesSource(const std::vector<std::shared_ptr<PlotSeries>>* series)
    {
        m_seriesRef = series;
    };

    /**
     * @brief Renders the cursor lines, intersection grips, and value tags onto the active view canvas.
     * @param painter Execution handle targeting active drawing canvas context surfaces.
     * @param plotArea Main viewport rectangle coordinate limits scaled in screen pixels.
     * @param calculatedPixelX Screen space pixel column position tracking cursor location.
     * @param mainLabel Custom text string to show inside the information tooltip bubble.
     * @param alignLeft Toggles justification layout alignment direction preferences for text tags.
     */
    void draw(QPainter& painter, const QRect& plotArea, int calculatedPixelX,
              const QString& mainLabel, bool alignLeft = true);

    /***** Styling *****/

    /**
     * @brief Updates typography configuration layouts applied onto numeric labels.
     * @param font QFont structure containing desired text rendering attributes.
     */
    void setFont(const QFont& font);

    /**
     * @brief Sets primary fill color parameters assigned to label text strings.
     * @param color QColor text drawing color.
     */
    void setTextColor(const QColor& color);

    /**
     * @brief Sets solid background shading profile colors masking text box plates.
     * @param bgColor QColor tooltips background layer wash color.
     */
    void setBackgroundColor(const QColor& bgColor);

    /**
     * @brief Prescribes internal edge padding boundary separation limits in screen pixels.
     * @param marginPixels Total pixel padding threshold space constraint value.
     */
    void setMargin(const int marginPixels);

    /***** INPUT Handling *****/

    /**
     * @brief Probes mouse down inputs to evaluate focus hits and lock drag lifecycle states.
     * @param event System mouse update data payload arguments forwarded by UI thread.
     * @param currentPixelX Extant horizontal position of cursor on screen.
     * @return True if selection focus hits interactive regions, triggering state capture lock.
     */
    bool mousePressEvent(QMouseEvent* event, int currentPixelX);

    /**
     * @brief Translates ongoing movement input tracking offsets to recompute absolute placements.
     * @param event System mouse update data payload arguments forwarded by UI thread.
     * @param plotArea Plot workspace pixel containment constraints structure box.
     * @param outNewPixelX Output column tracking parameter yielding modified layout location values.
     * @return True if a reposition sequence occurs requiring view updates.
     */
    bool mouseMoveEvent(QMouseEvent* event, const QRect& plotArea,
                        int& outNewPixelX);

    /**
     * @brief Breaks active pointer capture tracking loops, releasing global control lock status flags.
     * @param event System mouse update data payload arguments forwarded by UI thread.
     */
    void mouseReleaseEvent(QMouseEvent* event);

    /** @param pos Absolute internal data coordinate value location to map onto X axis. */
    void   setPos(double pos) { m_xPos = pos; }
    /** @return Current absolute position mapped value tracked by cursor on X scale. */
    double getPos() const { return m_xPos; }

    /** @return Computational processing loop permission status state verification flag. */
    bool enabled() const { return m_isEnabled; }
    /** @param e Toggle programmatic parsing and drawing evaluation authorization passes. */
    void enable(bool e) { m_isEnabled = e; }

    /** @return True if the cursor is currently being drawn on screen. */
    bool isVisible() { return m_isVisible; }

    /** @return Mutual exclusion lock tracking flag testing active navigation click drag sequences. */
    bool isDragging() const { return m_isDragging; }

    /**
     * @brief Hit-test verification evaluating coordinate intersection metrics across interaction bounds.
     * @param pos Intermittent screen pixel sample location targeted for layout mapping validation checks.
     * @return True if pointer sample context falls within focus collision geometries.
     */
    bool contains(const QPoint& pos) const;

   private:
    bool m_isEnabled =
        true; /**< Internal execution state controlling drawing passes. */
    double m_xPos =
        0; /**< Target coordinate placement metric tracking location across data domains. */

    bool m_isDragging =
        false; /**< Core tracking flag state tracking translation click drag cycles. */
    int m_dragOffset =
        0; /**< Variable preserving delta pixel offsets across movement loops. */

    int m_cursorIndex; /**< Positional tracking queue metadata order sequence tag. */
    bool m_isVisible =
        false; /**< Temporary runtime visibility evaluation storage state tracker. */
    QFont
        m_font; /**< Typography attribute profiles mapped onto label strings. */
    QColor
        m_color; /**< Base coloring applied across outline rules and vectors. */
    QColor
        m_bgColor; /**< Solid background plate color applied behind data popups. */

    QRect
        m_labelRect; /**< Interactive geometry container boundaries enclosing value tooltip cards. */
    QRect
        m_lineHitbox; /**< Oversized virtual selection envelope padding thin lines to simplify target grab clicks. */

    MarkerStyle
        m_style; /**< Active geometric decoration formatting behavior profile. */

    /** Shared repository address collection reference mapping active data vectors. */
    const std::vector<std::shared_ptr<PlotSeries>>* m_seriesRef = nullptr;

    /**
     * @brief Geometry drawer executing viewport line layouts with floating readout badges.
     */
    void drawFullMarker(QPainter& painter, const QRect& plotArea,
                        int calculatedPixelX, bool alignLeft);

    /**
     * @brief Geometry drawer rendering structural low-profile handle markers on axis frames.
     */
    void drawSimpleMarker(QPainter& painter, const QRect& plotArea,
                          int calculatedPixelX, bool alignLeft);
};

#endif  // RTPCURSOR_H__
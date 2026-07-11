#ifndef RTPRANGEDCURSOR_H__
#define RTPRANGEDCURSOR_H__

#include <QMouseEvent>
#include <QPainter>
#include <QVector>
#include <QWheelEvent>

#include "PlotSeries.h"
#include "components/RtpCursor.h"

/**
 * @class RtpRangeCursor
 * @brief Manages a pair of dual bounds cursors to measure mathematical delta ranges.
 *
 * This component coordinates a start boundary cursor and an end boundary cursor,
 * calculating metrics, statistics, and delta values across the bounded sub-interval.
 * It encapsulates mouse event handling for interactive tracking and movement.
 *
 * 
 */
class RtpRangeCursor
{
   public:
    /**
     * @brief Constructs a dual range selection cursor controller.
     * @param index Identifier tag tracking this composite asset component.
     * @param color Base rendering color layer mapped onto cursor boundary rules.
     * @param style Geometric shape decoration setting applied onto layout bounds (Default: MARKER_FULL).
     */
    RtpRangeCursor(
        int index, QColor color,
        RtpCursor::MarkerStyle style = RtpCursor::MarkerStyle::MARKER_FULL);

    /**
     * @brief Maps the reference data tracking dataset channel lookup collection list.
     * @param series Constant pointer targeting the multi-channel vector stack array.
     */
    void setSeriesSource(const std::vector<std::shared_ptr<PlotSeries>>* series)
    {
        m_seriesRef = series;
    };

    /**
     * @brief Adjusts primary text foreground overlay paintbrush color options.
     * @param color QColor description applied on textual outputs.
     */
    void setTextColor(const QColor& color);

    /**
     * @brief Adjusts back-face readout boundary summary box mask fill colors.
     * @param bgColor QColor applied to wash metric summary backgrounds.
     */
    void setBackgroundColor(const QColor& bgColor);

    /**
     * @brief Sets internal content margin spacing boundaries in screen pixel units.
     * @param marginPixels Absolute padding pixel threshold constraint value.
     */
    void setMargin(const int marginPixels);

    /**
     * @brief Renders lines, boundaries, highlights, and tooltips on the painting canvas.
     * @param painter Target QPainter render canvas execution surface context layer.
     * @param plotArea Main active data graphing rectangle geometry constraints in screen pixels.
     * @param calculatedPixelStart Screen pixel column tracking localization of start bounds.
     * @param calculatedPixelEnd Screen pixel column tracking localization of end bounds.
     */
    void draw(QPainter& painter, const QRect& plotArea,
              int calculatedPixelStart, int calculatedPixelEnd);

    /**
     * @brief Evaluates mouse down inputs to trigger active edge translation drag capture cycles.
     * @param event Native mouse press parameters forwarded by layout framework.
     * @param currentPixelStart Extant horizontal pixel location of start border mark.
     * @param currentPixelEnd Extant horizontal pixel location of end border mark.
     * @return Underlying target cursor subcomponent reference capturing drag state, or nullptr if click missed.
     */
    RtpCursor* mousePressEvent(QMouseEvent* event, int currentPixelStart,
                               int currentPixelEnd);

    /**
     * @brief Processes ongoing pointing translation offsets to recompute absolute boundary marks.
     * @param event Native mouse update path parameters forwarded by layout framework.
     * @param plotArea Reference canvas rectangle geometry constraints.
     * @param outNewPixelX Output tracking parameter yielding updated location details.
     * @return True if a modification pass took place requiring view update flags.
     */
    bool mouseMoveEvent(QMouseEvent* event, const QRect& plotArea,
                        int& outNewPixelX);

    /**
     * @brief Finalizes boundary adjustments, unlocking state control loops.
     * @param event Native mouse release parameters forwarded by layout framework.
     */
    void mouseReleaseEvent(QMouseEvent* event);

    /** @param pos Absolute data space value position coordinate assigned to start edge. */
    void   setPosStart(double pos) { m_cStart.setPos(pos); }
    /** @return Current absolute data space value placement of start boundary edge. */
    double getPosStart() const { return m_cStart.getPos(); }

    /** @param pos Absolute data space value position coordinate assigned to end edge. */
    void   setPosEnd(double pos) { m_cEnd.setPos(pos); }
    /** @return Current absolute data space value placement of end boundary edge. */
    double getPosEnd() const { return m_cEnd.getPos(); }

    /** @return Visibility and computation evaluation permission validation status state flag. */
    bool enabled() const { return m_isEnabled; }
    /** @param e Toggle visibility and algorithmic evaluation processing permissions. */
    void enable(bool e) { m_isEnabled = e; }

    /**
     * @brief Determines whether a pointer position lands inside the bounding footprint selection.
     * @param pos Intermittent screen space coordinate target evaluation candidate.
     * @return True if geometry intersections confirm pointing focus hits the element.
     */
    bool contains(const QPoint& pos) const;

    bool checkReorder();

   private:
    bool m_isEnabled =
        true; /**< Structural execution state controlling drawing passes. */
    double m_xPos =
        0; /**< Master anchor localization reference metric point tracker. */

    bool m_isDragging =
        false; /**< Mutual exclusion flag lock tracking interactive relocation inputs. */
    int m_dragOffset =
        0; /**< Offset value cache preserving layout adjustment tracking pixels. */

    int m_cursorIndex; /**< Identifying component order sequence metadata tracking token. */

    QColor
        m_color; /**< Primary outline and line rule display color definition. */
    QColor
        m_bgColor; /**< Readout info box opaque background plate fill color profile. */

    RtpCursor
        m_cStart; /**< Core tracking subcomponent instance representing left bound marker. */
    RtpCursor
        m_cEnd; /**< Core tracking subcomponent instance representing right bound marker. */

    RtpCursor::MarkerStyle
        m_style; /**< Geometric styling shape type rendering configuration mode. */

    /** Internal dataset lookup registry array mapping tracking lines. */
    const std::vector<std::shared_ptr<PlotSeries>>* m_seriesRef = nullptr;
};

#endif  // RTPRANGEDCURSOR_H__
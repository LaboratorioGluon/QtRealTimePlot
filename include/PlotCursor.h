#pragma once

#include <QColor>
#include <QFont>
#include <QRect>
#include <QString>

#include "PlotOverlay.h"

/**
 * @class PlotCursor
 * @brief Draggable vertical marker at an X position.
 *
 * The simple style draws a small "Mn" flag. The full style also shows, for every
 * series, the value of the sample closest to the cursor.
 */
class PlotCursor : public PlotOverlay
{
   public:
    /** @brief Visual style of the marker. */
    enum MarkerStyle {
        MARKER_FULL = 0, /**< Line plus a box with each series' value at the cursor. */
        MARKER_SIMPLE    /**< Line plus a small numbered flag. */
    };

    PlotCursor(int index, QColor color, MarkerStyle style = MarkerStyle::MARKER_FULL);

    void   setPos(double pos) { m_xPos = pos; }
    double getPos() const { return m_xPos; }

    void setFont(const QFont& font) { m_font = font; }
    void setBackgroundColor(const QColor& bgColor) { m_bgColor = bgColor; }
    /** @param unit Appended to the values in the full marker (e.g. "V"). Empty shows bare values. */
    void setUnit(const QString& unit) { m_unit = unit; }

    /** @return True if the cursor was inside the plot during the last draw. */
    bool isVisible() const { return m_isVisible; }
    bool isDragging() const { return m_isDragging; }
    /** @return True if pos is over the line or the label of the cursor. */
    bool contains(const QPoint& pos) const;

    // PlotOverlay
    void draw(QPainter& painter, const PlotContext& ctx) override;
    bool mousePress(QMouseEvent* event, const PlotContext& ctx) override;
    void mouseMove(QMouseEvent* event, const PlotContext& ctx) override;
    void mouseRelease(QMouseEvent* event, const PlotContext& ctx) override;
    bool hoverShape(QPoint pos, Qt::CursorShape& shape) const override;

   private:
    void drawFullMarker(QPainter& painter, const PlotContext& ctx, int pixelX);
    void drawSimpleMarker(QPainter& painter, const QRect& plotArea, int pixelX);

    int         m_cursorIndex;
    QColor      m_color;
    MarkerStyle m_style;
    QColor      m_bgColor{Qt::black};
    QFont       m_font;
    QString     m_unit;
    double      m_xPos       = 0.0;
    bool        m_isDragging = false;
    int         m_dragOffset = 0;
    bool        m_isVisible  = false;
    QRect       m_labelRect;  /**< Label hit area from the last draw. */
    QRect       m_lineHitbox; /**< Widened line hit area from the last draw. */
};

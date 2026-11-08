#pragma once

#include "PlotCursor.h"

/**
 * @class PlotRangeCursor
 * @brief Two cursors delimiting a shaded X interval. Start always ends up left of End.
 */
class PlotRangeCursor : public PlotOverlay
{
   public:
    PlotRangeCursor(int index, QColor color, PlotCursor::MarkerStyle style = PlotCursor::MarkerStyle::MARKER_FULL);

    void   setPosStart(double pos) { m_start.setPos(pos); }
    double getPosStart() const { return m_start.getPos(); }
    void   setPosEnd(double pos) { m_end.setPos(pos); }
    double getPosEnd() const { return m_end.getPos(); }

    /** @return True if pos is over either cursor. */
    bool contains(const QPoint& pos) const;
    /** @brief Swaps the cursors if Start is right of End. @return True if they were swapped. */
    bool checkReorder();

    // PlotOverlay
    void draw(QPainter& painter, const PlotContext& ctx) override;
    bool mousePress(QMouseEvent* event, const PlotContext& ctx) override;
    void mouseMove(QMouseEvent* event, const PlotContext& ctx) override;
    void mouseRelease(QMouseEvent* event, const PlotContext& ctx) override;
    bool hoverShape(QPoint pos, Qt::CursorShape& shape) const override;

   private:
    QColor      m_color;
    PlotCursor  m_start;
    PlotCursor  m_end;
    PlotCursor* m_active = nullptr; /**< Cursor being dragged. */
};

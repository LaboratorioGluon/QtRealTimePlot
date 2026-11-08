#pragma once

#include <QColor>
#include <QFont>
#include <QRect>
#include <functional>

#include "PlotOverlay.h"

/**
 * @class PlotLegend
 * @brief Box in the top-right corner listing the series. Clicking a row toggles that series.
 */
class PlotLegend : public PlotOverlay
{
   public:
    PlotLegend();

    void setFont(const QFont& font) { m_font = font; }
    void setBackgroundColor(const QColor& bgColor) { m_bgColor = bgColor; }
    void setMargin(int marginPixels) { m_padding = marginPixels; }

    /** @param cb Called with the series index after a click toggled its visibility. */
    void setOnToggled(std::function<void(int)> cb) { m_onToggled = std::move(cb); }

    /** @return Row index under mousePos, or -1. Uses the geometry of the last draw. */
    int   checkClick(const QPoint& mousePos, size_t seriesCount) const;
    QRect lastRenderedRect() const { return m_lastRect; }

    // PlotOverlay
    void draw(QPainter& painter, const PlotContext& ctx) override;
    bool mousePress(QMouseEvent* event, const PlotContext& ctx) override;
    bool hoverShape(QPoint pos, Qt::CursorShape& shape) const override;

   private:
    QFont                    m_font;
    QColor                   m_bgColor{18, 18, 22, 220};
    int                      m_padding = 6;
    QRect                    m_lastRect;
    std::function<void(int)> m_onToggled;
};

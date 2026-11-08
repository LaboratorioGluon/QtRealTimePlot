#pragma once

#include <QMouseEvent>
#include <QPainter>
#include <memory>
#include <vector>

#include "PlotSeries.h"
#include "PlotView.h"

/**
 * @struct PlotContext
 * @brief What an overlay can see while drawing or handling input.
 */
struct PlotContext {
    const PlotView&                                 view;   /**< Visible range and plot rectangle. */
    const std::vector<std::shared_ptr<PlotSeries>>& series; /**< Series shown by the plot. */
};

/**
 * @class PlotOverlay
 * @brief Interface for anything drawn with QPainter on top of the data: cursors, legend, ...
 *
 * The plot draws overlays in order and offers mouse presses to them from the
 * topmost down. An overlay that accepts a press receives the following move and
 * release events until the button is released.
 */
class PlotOverlay
{
   public:
    virtual ~PlotOverlay() = default;

    /** @brief Paints the overlay. Called once per frame. */
    virtual void draw(QPainter& painter, const PlotContext& ctx) = 0;

    /** @return True if the overlay takes the press (and the drag that follows). */
    virtual bool mousePress(QMouseEvent* /*event*/, const PlotContext& /*ctx*/) { return false; }
    /** @brief Mouse moved while this overlay owns the drag. */
    virtual void mouseMove(QMouseEvent* /*event*/, const PlotContext& /*ctx*/) {}
    /** @brief Button released while this overlay owns the drag. */
    virtual void mouseRelease(QMouseEvent* /*event*/, const PlotContext& /*ctx*/) {}

    /**
     * @brief Mouse cursor to show while hovering pos.
     * @return True if pos is over this overlay and shape was set.
     */
    virtual bool hoverShape(QPoint /*pos*/, Qt::CursorShape& /*shape*/) const { return false; }

    bool enabled() const { return m_enabled; }
    void enable(bool e) { m_enabled = e; }

   protected:
    bool m_enabled = true;
};

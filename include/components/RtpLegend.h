#ifndef RTPLEGEND_H__
#define RTPLEGEND_H__

#include <QPainter>
#include <QVector>
#include "PlotSeries.h"

/**
 * @class RtpLegend
 * @brief Manages the visual layout, rendering, and interaction of the chart legend.
 *
 * This class handles rendering the series names and color indicators on an overlay 
 * using QPainter. It also provides mouse click detection to allow toggling the 
 * visibility of individual plot lines.
 */
class RtpLegend
{
   public:
    /**
     * @brief Constructs a plot legend overlay manager instance.
     */
    RtpLegend();

    /**
     * @brief Sets the data source pointer containing the collection of plot series.
     * @param series Constant pointer to the std::vector containing shared pointers of PlotSeries.
     */
    void setSeriesSource(
        const std::vector<std::shared_ptr<PlotSeries>>* series);

    /**
     * @brief Configuration modifier to change the text label font settings.
     * @param font QFont styling properties to apply to the text items.
     */
    void setFont(const QFont& font);

    /**
     * @brief Adjusts the primary color used to draw text strings.
     * @param color QColor description matching the target text layout style.
     */
    void setTextColor(const QColor& color);

    /**
     * @brief Adjusts the background box solid mask color fill.
     * @param bgColor QColor applied to wash the background bounding box area.
     */
    void setBackgroundColor(const QColor& bgColor);

    /**
     * @brief Sets the outer spacing metric around text bounding masks in screen pixels.
     * @param marginPixels Target padding distance in screen pixels.
     */
    void setMargin(int marginPixels);

    /**
     * @brief Renders the legend window overlay on the provided painter canvas context.
     * @param painter QPainter surface canvas tracking rendering pipelines.
     * @param plotRect Bounding pixel constraints enclosing the active chart area widget.
     */
    void draw(QPainter& painter, const QRect& plotRect);

    /**
     * @brief Tests if a given screen mouse coordinate lands within a valid legend series item row.
     *
     * Used to implement hit-testing to see if a user clicked on a specific series item entry 
     * to perform runtime mutations (e.g., toggling that specific channel's visibility).
     *
     * @param mousePos Integer mouse coordinate position localized relative to the parent widget map.
     * @return Integer index of the matched plot channel entry row, or -1 if the click missed.
     */
    int checkClick(const QPoint& mousePos) const;

    /** * @brief Retrieves the boundary pixel constraints from the most recent rendering pass execution.
     * @return QRect representing the screen space layout geometry envelope occupied by the component.
     */
    QRect lastRenderedRect() const { return m_lastRect; }

   private:
    QFont
        m_font; /**< Font specification asset mapping descriptor settings onto label outputs. */
    QColor
        m_bgColor; /**< Solid clear back-face channel color metrics profile map configuration. */
    int m_padding; /**< Inner alignment border separation distance threshold scaled in screen pixels. */
    QRect
        m_lastRect; /**< Geometry box cache capturing metrics from the historical draw call operation. */

    /** Reference to the Plot Series, usually configured from the RealTimePlot. */
    const std::vector<std::shared_ptr<PlotSeries>>* m_seriesRef = nullptr;
};

#endif  // RTPLEGEND_H__
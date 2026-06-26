#ifndef RTPLEGEND_H__
#define RTPLEGEND_H__

#include <QPainter>
#include <QVector>

// Declaración hacia adelante (Forward declaration) de tu clase real
class PlotSeries;

class RtpLegend
{
public:
    RtpLegend();

    void setSeriesSource(const std::vector<std::shared_ptr<PlotSeries>> *series);

    void setFont(const QFont &font);
    void setTextColor(const QColor &color);
    void setBackgroundColor(const QColor &bgColor);
    void setMargin(int marginPixels);

    void draw(QPainter &painter, const QRect &plotRect);
    int checkClick(const QPoint &mousePos) const;

    QRect lastRenderedRect() const { return m_lastRect; }

private:
    QFont m_font;
    QColor m_textColor;
    QColor m_bgColor;
    int m_padding;
    QRect m_lastRect;

    // Apuntamos al contenedor original de RealtimePlot
    const std::vector<std::shared_ptr<PlotSeries>> *m_seriesRef = nullptr;
};

#endif // RTPLEGEND_H__
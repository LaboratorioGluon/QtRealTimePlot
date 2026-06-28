#ifndef RTPRANGEDCURSOR_H__
#define RTPRANGEDCURSOR_H__

#include <QPainter>
#include <QVector>
#include <QWheelEvent>
#include <QMouseEvent>

#include "PlotSeries.h"
#include "components/RtpCursor.h"

class RtpRangeCursor
{
public:
    RtpRangeCursor(int index, QColor color, RtpCursor::MarkerStyle style = RtpCursor::MarkerStyle::MARKER_FULL);

    void setSeriesSource(const std::vector<std::shared_ptr<PlotSeries>> *series) { m_seriesRef = series; };

    void setFont(const QFont &font);
    void setTextColor(const QColor &color);
    void setBackgroundColor(const QColor &bgColor);
    void setMargin(const int marginPixels);

    void draw(QPainter &painter,
              const QRect &plotArea,
              int calculatedPixelStart,
              int calculatedPixelEnd);

    RtpCursor *mousePressEvent(QMouseEvent *event, int currentPixelStart, int currentPixelEnd);
    bool mouseMoveEvent(QMouseEvent *event, const QRect &plotArea, int &outNewPixelX);
    void mouseReleaseEvent(QMouseEvent *event);

    QRect lastRenderedRect() const { return m_lastRect; }

    void setPosStart(double pos) { m_cStart.setPos(pos); }
    double getPosStart() const { return m_cStart.getPos(); }

    void setPosEnd(double pos) { m_cEnd.setPos(pos); }
    double getPosEnd() const { return m_cEnd.getPos(); }

    bool enabled() const { return m_isEnabled; }
    void enable(bool e) { m_isEnabled = e; }

    bool contains(const QPoint &pos) const;

private:
    bool m_isEnabled = true;
    double m_xPos = 0;

    bool m_isDragging = false;
    int m_dragOffset = 0;

    int m_cursorIndex;

    QFont m_font;
    QColor m_color;
    QColor m_textColor;
    QColor m_bgColor;
    int m_padding;
    QRect m_lastRect;

    RtpCursor m_cStart;
    RtpCursor m_cEnd;

    RtpCursor::MarkerStyle m_style;

    const std::vector<std::shared_ptr<PlotSeries>> *m_seriesRef = nullptr;
};

#endif // RTPRANGEDCURSOR_H__
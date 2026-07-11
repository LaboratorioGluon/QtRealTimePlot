#include "components/RtpRangeCursor.h"

#include <QPainterPath>

RtpRangeCursor::RtpRangeCursor(int index, QColor color,
                               RtpCursor::MarkerStyle style)
    : m_color(color),
      m_cursorIndex(index),
      m_style(style),
      m_bgColor(Qt::black),
      m_cStart(index, color, style),
      m_cEnd(index, color, style)
{}

void RtpRangeCursor::setBackgroundColor(const QColor& bgColor)
{
    m_bgColor = bgColor;
}

bool RtpRangeCursor::contains(const QPoint& pos) const
{
    if (!m_isEnabled)
        return false;
    return m_cStart.contains(pos) || m_cEnd.contains(pos);
}

RtpCursor* RtpRangeCursor::mousePressEvent(QMouseEvent* event,
                                           int          currentPixelStart,
                                           int          currentPixelEnd)
{
    if (!m_isEnabled)
        return nullptr;

    if (m_cStart.mousePressEvent(event, currentPixelStart))
    {

        qDebug() << "Activated start!";
        return &m_cStart;
    }
    if (m_cEnd.mousePressEvent(event, currentPixelEnd))
    {
        qDebug() << "Activated end!";
        return &m_cEnd;
    }

    return nullptr;
}

bool RtpRangeCursor::mouseMoveEvent(QMouseEvent* event, const QRect& plotArea,
                                    int& outNewPixelX)
{
    if (!m_isDragging)
        return false;

    if (m_cStart.isDragging())
    {
        return m_cStart.mouseMoveEvent(event, plotArea, outNewPixelX);
    }
    else if (m_cEnd.isDragging())
    {
        return m_cEnd.mouseMoveEvent(event, plotArea, outNewPixelX);
    }

    return true;
}

void RtpRangeCursor::mouseReleaseEvent(QMouseEvent* event)
{
    m_isDragging = false;
}

void RtpRangeCursor::draw(QPainter& painter, const QRect& plotArea,
                          int calculatedPixelStart, int calculatedPixelEnd)
{
    painter.save();

    QRect zone(QPoint(calculatedPixelStart, plotArea.top()),
               QPoint(calculatedPixelEnd, plotArea.bottom()));

    QColor bg = m_color.lighter(100);
    bg.setAlpha(20);
    if (m_cStart.isVisible() && m_cEnd.isVisible())
    {
        painter.setBrush(QBrush(bg));
        painter.setPen(Qt::PenStyle::NoPen);
        painter.drawRect(zone);
    }
    painter.restore();
    m_cStart.draw(painter, plotArea, calculatedPixelStart, "Start");
    m_cEnd.draw(painter, plotArea, calculatedPixelEnd, "End");
}

bool RtpRangeCursor::checkReorder()
{
    double prevStart = m_cStart.getPos();
    double prevEnd   = m_cEnd.getPos();
    if (prevStart > prevEnd)
    {
        m_cStart.setPos(prevEnd);
        m_cEnd.setPos(prevStart);
#ifdef RTP_ENABLE_DEBUG
        qDebug() << "Reordered";
#endif
        return true;
    }
    return false;
}
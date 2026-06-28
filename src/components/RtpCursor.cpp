#include "components/RtpCursor.h"

#include <QPainterPath>

RtpCursor::RtpCursor(int index, QColor color, MarkerStyle style)
    : m_color(color),
      m_cursorIndex(index),
      m_style(style),
      m_textColor(Qt::red),
      m_bgColor(Qt::black)
{}

void RtpCursor::setFont(const QFont& font)
{
    m_font = font;
}
void RtpCursor::setTextColor(const QColor& color)
{
    m_textColor = color;
}
void RtpCursor::setBackgroundColor(const QColor& bgColor)
{
    m_bgColor = bgColor;
}

bool RtpCursor::contains(const QPoint& pos) const
{
    if (!m_isEnabled)
        return false;
    return m_labelRect.contains(pos) || m_lineHitbox.contains(pos);
}

bool RtpCursor::mousePressEvent(QMouseEvent* event, int currentPixelX)
{
    if (!m_isEnabled)
        return false;

    QPoint pos = event->pos();
    if (m_labelRect.contains(pos) || m_lineHitbox.contains(pos))
    {
        m_isDragging = true;
        m_dragOffset =
            pos.x() -
            currentPixelX;  // Usamos el píxel que el plot sabe que tenemos
        return true;
    }
    return false;
}

bool RtpCursor::mouseMoveEvent(QMouseEvent* event, const QRect& plotArea,
                               int& outNewPixelX)
{
    if (!m_isDragging)
        return false;

    int newPixelX = event->pos().x() - m_dragOffset;
    if (newPixelX < plotArea.left())
        newPixelX = plotArea.left();
    if (newPixelX > plotArea.right())
        newPixelX = plotArea.right();

    qDebug() << "Movin cursor to " << newPixelX;
    outNewPixelX = newPixelX;  // Devolvemos el píxel calculado al plot
    return true;
}

void RtpCursor::mouseReleaseEvent(QMouseEvent* event)
{
    m_isDragging = false;
}

void RtpCursor::drawFullMarker(QPainter& painter, const QRect& plotArea,
                               int calculatedPixelX, bool alignLeft)
{
    QFontMetrics fm(m_font);
    int          lineHeight = fm.height();
    int          padding    = 4;

    int boxHeight = padding * 2 + lineHeight * (1 + m_seriesRef->size()) +
                    (m_seriesRef->size() * 2);
    int boxWidth  = 120;

    int boxX    = alignLeft ? (calculatedPixelX - boxWidth) : calculatedPixelX;
    m_labelRect = QRect(boxX, plotArea.top() + 5, boxWidth, boxHeight);

    painter.setBrush(QColor(10, 10, 10, m_isDragging ? 255 : 230));
    painter.setPen(QPen(m_color, 1, Qt::SolidLine));
    painter.drawRect(m_labelRect);

    // Textos
    painter.setPen(m_color);
    int  textX = alignLeft ? (calculatedPixelX - 4) : (calculatedPixelX + 4);
    auto alignment = alignLeft ? (Qt::AlignRight | Qt::AlignTop)
                               : (Qt::AlignLeft | Qt::AlignTop);
    painter.drawText(QRect(textX, plotArea.top() + 5 + padding,
                           alignLeft ? -boxWidth : boxWidth, lineHeight)
                         .normalized(),
                     alignment, QString("M%1").arg(m_cursorIndex));

    int index = 1;
    for (const auto& s : *m_seriesRef)
    {
        painter.setPen(s->color());
        painter.drawText(
            QRect(textX,
                  plotArea.top() + 5 + padding + (lineHeight + 2) * index++,
                  alignLeft ? -boxWidth : boxWidth, lineHeight)
                .normalized(),
            alignment,
            QString("%1 pA").arg(s->getClosestPointToX(m_xPos).y, 0, 'f', 2));
    }
}

void RtpCursor::drawSimpleMarker(QPainter& painter, const QRect& plotArea,
                                 int calculatedPixelX, bool alignLeft)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int rectWidth  = 18;
    const int rectHeight = 24;
    const int notchSize  = rectWidth / 2;

    QPainterPath path;
    path.moveTo(0, 0);

    path.lineTo(-notchSize, -notchSize);
    path.lineTo(-rectWidth / 2, -notchSize);

    path.lineTo(-rectWidth / 2, -(notchSize + rectHeight));
    path.lineTo(rectWidth / 2, -(notchSize + rectHeight));

    path.lineTo(rectWidth / 2, -notchSize);
    path.lineTo(notchSize, -notchSize);

    path.closeSubpath();

    painter.translate(calculatedPixelX,
                      plotArea.top() + rectHeight + notchSize);

    m_labelRect = QRect(calculatedPixelX - rectWidth / 2, plotArea.top(),
                        rectWidth, rectHeight + notchSize);

    painter.setPen(QPen(m_color, 2));
    painter.setBrush(m_bgColor);
    painter.drawPath(path);

    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);

    int textCenterX = 0;
    int textCenterY = -(notchSize + (rectHeight / 2));

    painter.translate(textCenterX, textCenterY);
    painter.rotate(-90);

    m_font.setBold(true);

    painter.setPen(m_color);

    QRect textRect(-rectWidth / 2, -rectHeight / 2, rectWidth, rectHeight);

    painter.drawText(textRect, Qt::AlignCenter,
                     QString("M%1").arg(m_cursorIndex));
    m_font.setBold(false);

    painter.restore();
}

void RtpCursor::draw(QPainter& painter, const QRect& plotArea,
                     int calculatedPixelX, const QString& mainLabel,
                     bool alignLeft)
{
    {
        if (!m_isEnabled || calculatedPixelX < plotArea.left() ||
            calculatedPixelX > plotArea.right())
        {
            m_isVisible  = false;
            m_labelRect  = QRect();
            m_lineHitbox = QRect();
            return;
        }
        m_isVisible = true;

        painter.save();
        painter.setFont(m_font);

        m_lineHitbox =
            QRect(calculatedPixelX - 4, plotArea.top(), 8, plotArea.height());

        painter.setPen(QPen(m_color, m_isDragging ? 2 : 1, Qt::SolidLine));
        painter.drawLine(calculatedPixelX, plotArea.top(), calculatedPixelX,
                         plotArea.bottom());

        if (m_style == MarkerStyle::MARKER_SIMPLE)
        {
            drawSimpleMarker(painter, plotArea, calculatedPixelX, alignLeft);
        }
        else
        {
            drawFullMarker(painter, plotArea, calculatedPixelX, alignLeft);
        }

        painter.restore();
    }
}

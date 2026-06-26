#include "components/RtpLegend.h"
#include "PlotSeries.h"

RtpLegend::RtpLegend()
    : m_textColor(Qt::black), m_bgColor(QColor(18, 18, 22, 220)), m_padding(6)
{
    m_font.setPointSize(9);
}

void RtpLegend::setSeriesSource(const std::vector<std::shared_ptr<PlotSeries>> *series) { m_seriesRef = series; }
void RtpLegend::setFont(const QFont &font) { m_font = font; }
void RtpLegend::setTextColor(const QColor &color) { m_textColor = color; }
void RtpLegend::setBackgroundColor(const QColor &bgColor) { m_bgColor = bgColor; }
void RtpLegend::setMargin(int marginPixels) { m_padding = marginPixels; }

void RtpLegend::draw(QPainter &painter, const QRect &plotRect)
{
    if (!m_seriesRef || m_seriesRef->empty())
    {
        m_lastRect = QRect();
        return;
    }

    painter.save();
    painter.setFont(m_font);

    QFontMetrics fm(m_font);
    int lineHeight = fm.height();
    int indicatorSize = lineHeight - 4;

    int maxTextWidth = 0;
    for (const auto &serie : *m_seriesRef)
    {
        int textW = fm.horizontalAdvance(serie->name());
        if (textW > maxTextWidth)
            maxTextWidth = textW;
    }

    int boxWidth = (m_padding * 2) + indicatorSize + 6 + maxTextWidth;
    int boxHeight = (m_padding * 2) + (m_seriesRef->size() * lineHeight) + ((m_seriesRef->size() - 1) * 2);

    int posX = plotRect.right() - boxWidth - 10;
    int posY = plotRect.top() + 10;
    m_lastRect = QRect(posX, posY, boxWidth, boxHeight);

    painter.setBrush(m_bgColor);
    painter.setPen(QPen(QColor(180, 180, 180), 1));
    painter.drawRect(m_lastRect);

    int currentY = posY + m_padding;
    for (const auto &serie : *m_seriesRef)
    {
        int currentX = posX + m_padding;
        bool isVisible = serie->visible();

        QRect indicatorRect(currentX, currentY + (lineHeight - indicatorSize) / 2, indicatorSize, indicatorSize);
        if (isVisible)
        {
            painter.setBrush(serie->color());
            painter.setPen(Qt::NoPen);
        }
        else
        {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(serie->color(), 1));
        }
        painter.drawRect(indicatorRect);

        QColor textCol = serie->color();
        if (!isVisible)
        {
            textCol.setAlpha(100);
        }
        painter.setPen(textCol);

        QRect textRect(currentX + indicatorSize + 6, currentY, maxTextWidth, lineHeight);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, serie->name());

        currentY += lineHeight + 2;
    }

    painter.restore();
}

int RtpLegend::checkClick(const QPoint &mousePos) const
{
    if (!m_seriesRef || m_lastRect.isEmpty() || !m_lastRect.contains(mousePos))
    {
        return -1;
    }

    QFontMetrics fm(m_font);
    int lineHeight = fm.height();
    int currentY = m_lastRect.top() + m_padding;

    for (int i = 0; i < m_seriesRef->size(); ++i)
    {
        QRect rowRect(m_lastRect.left() + m_padding, currentY, m_lastRect.width() - (m_padding * 2), lineHeight);

        if (rowRect.contains(mousePos))
        {
            return i;
        }
        currentY += lineHeight + 2;
    }

    return -1;
}
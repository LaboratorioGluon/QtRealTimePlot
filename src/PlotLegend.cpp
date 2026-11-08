#include "PlotLegend.h"

#include <QFontMetrics>
#include <algorithm>

PlotLegend::PlotLegend()
{
    m_font.setPointSize(9);
}

void PlotLegend::draw(QPainter& painter, const PlotContext& ctx)
{
    if (!m_enabled || ctx.series.empty())
    {
        m_lastRect = QRect();
        return;
    }

    painter.save();
    painter.setFont(m_font);

    QFontMetrics fm(m_font);
    const int    lineHeight    = fm.height();
    const int    indicatorSize = lineHeight - 4;
    const int    rows          = static_cast<int>(ctx.series.size());

    int maxTextWidth = 0;
    for (const auto& s : ctx.series)
        maxTextWidth = std::max(maxTextWidth, fm.horizontalAdvance(s->name()));

    const int   boxWidth  = (m_padding * 2) + indicatorSize + 6 + maxTextWidth + 4;
    const int   boxHeight = (m_padding * 2) + (rows * lineHeight) + ((rows - 1) * 2);
    const QRect plotRect  = ctx.view.area();
    const int   posX      = plotRect.right() - boxWidth - 10;
    const int   posY      = plotRect.top() + 10;
    m_lastRect            = QRect(posX, posY, boxWidth, boxHeight);

    painter.setBrush(m_bgColor);
    painter.setPen(QPen(QColor(180, 180, 180), 1));
    painter.drawRect(m_lastRect);

    int currentY = posY + m_padding;
    for (const auto& s : ctx.series)
    {
        const int  currentX  = posX + m_padding;
        const bool isVisible = s->visible();

        QRect indicatorRect(currentX, currentY + (lineHeight - indicatorSize) / 2, indicatorSize, indicatorSize);
        if (isVisible)
        {
            painter.setBrush(s->color());
            painter.setPen(Qt::NoPen);
        }
        else
        {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(s->color(), 1));
        }
        painter.drawRect(indicatorRect);

        QColor textCol = s->color();
        if (!isVisible)
            textCol.setAlpha(100);
        painter.setPen(textCol);
        painter.drawText(QRect(currentX + indicatorSize + 6, currentY, maxTextWidth + 4, lineHeight),
                         Qt::AlignLeft | Qt::AlignVCenter, s->name());

        currentY += lineHeight + 2;
    }

    painter.restore();
}

int PlotLegend::checkClick(const QPoint& mousePos, size_t seriesCount) const
{
    if (!m_enabled || m_lastRect.isEmpty() || !m_lastRect.contains(mousePos))
        return -1;

    QFontMetrics fm(m_font);
    const int    lineHeight = fm.height();
    int          currentY   = m_lastRect.top() + m_padding;

    for (size_t i = 0; i < seriesCount; ++i)
    {
        QRect rowRect(m_lastRect.left() + m_padding, currentY, m_lastRect.width() - (m_padding * 2), lineHeight);
        if (rowRect.contains(mousePos))
            return static_cast<int>(i);
        currentY += lineHeight + 2;
    }
    return -1;
}

bool PlotLegend::mousePress(QMouseEvent* event, const PlotContext& ctx)
{
    const int row = checkClick(event->pos(), ctx.series.size());
    if (row < 0)
        return false;

    auto& s = ctx.series[static_cast<size_t>(row)];
    s->setVisible(!s->visible());
    if (m_onToggled)
        m_onToggled(row);
    return true;
}

bool PlotLegend::hoverShape(QPoint pos, Qt::CursorShape& shape) const
{
    if (!m_enabled || !m_lastRect.contains(pos))
        return false;
    shape = Qt::PointingHandCursor;
    return true;
}

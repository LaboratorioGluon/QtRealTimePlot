#include "PlotCursor.h"

#include <QFontMetrics>
#include <QPainterPath>

#include <algorithm>
#include "PlotAxis.h"

PlotCursor::PlotCursor(int index, QColor color, MarkerStyle style)
    : m_cursorIndex(index), m_color(color), m_style(style)
{}

bool PlotCursor::contains(const QPoint& pos) const
{
    if (!m_enabled)
        return false;
    return m_labelRect.contains(pos) || m_lineHitbox.contains(pos);
}

bool PlotCursor::mousePress(QMouseEvent* event, const PlotContext& ctx)
{
    if (!contains(event->pos()))
        return false;
    m_isDragging = true;
    m_dragOffset = event->pos().x() - ctx.view.xToPixel(m_xPos);
    return true;
}

void PlotCursor::mouseMove(QMouseEvent* event, const PlotContext& ctx)
{
    if (!m_isDragging)
        return;
    const QRect area   = ctx.view.area();
    const int   pixelX = std::clamp(event->pos().x() - m_dragOffset, area.left(), area.right());
    m_xPos             = ctx.view.pixelToX(pixelX);
}

void PlotCursor::mouseRelease(QMouseEvent* /*event*/, const PlotContext& /*ctx*/)
{
    m_isDragging = false;
}

bool PlotCursor::hoverShape(QPoint pos, Qt::CursorShape& shape) const
{
    if (!contains(pos))
        return false;
    shape = Qt::SizeHorCursor;
    return true;
}

void PlotCursor::draw(QPainter& painter, const PlotContext& ctx)
{
    const QRect area   = ctx.view.area();
    const int   pixelX = ctx.view.xToPixel(m_xPos);

    if (!m_enabled || pixelX < area.left() || pixelX > area.right())
    {
        m_isVisible  = false;
        m_labelRect  = QRect();
        m_lineHitbox = QRect();
        return;
    }
    m_isVisible = true;

    painter.save();
    painter.setFont(m_font);

    m_lineHitbox = QRect(pixelX - 4, area.top(), 8, area.height());

    painter.setPen(QPen(m_color, m_isDragging ? 2 : 1, Qt::SolidLine));
    painter.drawLine(pixelX, area.top(), pixelX, area.bottom());

    if (m_style == MarkerStyle::MARKER_SIMPLE)
        drawSimpleMarker(painter, area, pixelX);
    else
        drawFullMarker(painter, ctx, pixelX);

    painter.restore();
}

void PlotCursor::drawFullMarker(QPainter& painter, const PlotContext& ctx, int pixelX)
{
    const QRect  area = ctx.view.area();
    QFontMetrics fm(m_font);
    const int    lineHeight  = fm.height();
    const int    padding     = 4;
    const int    seriesCount = static_cast<int>(ctx.series.size());
    const int    boxHeight   = padding * 2 + lineHeight * (1 + seriesCount) + (seriesCount * 2);
    const int    boxWidth    = 120;

    // The box sits left of the line.
    m_labelRect = QRect(pixelX - boxWidth, area.top() + 5, boxWidth, boxHeight);

    painter.setBrush(QColor(10, 10, 10, m_isDragging ? 255 : 230));
    painter.setPen(QPen(m_color, 1, Qt::SolidLine));
    painter.drawRect(m_labelRect);

    const int  textX     = pixelX - 4;
    const auto alignment = Qt::AlignRight | Qt::AlignTop;
    painter.setPen(m_color);
    painter.drawText(QRect(textX, area.top() + 5 + padding, -boxWidth, lineHeight).normalized(), alignment,
                     QString("M%1").arg(m_cursorIndex));

    const QString suffix = m_unit.isEmpty() ? QString() : QStringLiteral(" ") + m_unit;
    int           row    = 1;
    for (const auto& s : ctx.series)
    {
        painter.setPen(s->color());
        painter.drawText(
            QRect(textX, area.top() + 5 + padding + (lineHeight + 2) * row++, -boxWidth, lineHeight).normalized(),
            alignment, PlotAxis::formatValue(s->getClosestPointToX(m_xPos).y, 4) + suffix);
    }
}

void PlotCursor::drawSimpleMarker(QPainter& painter, const QRect& plotArea, int pixelX)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int rectWidth  = 18;
    const int rectHeight = 24;
    const int notchSize  = rectWidth / 2;

    // Flag with a pointing triangle at the bottom
    QPainterPath path;
    path.moveTo(0, 0);
    path.lineTo(-notchSize, -notchSize);
    path.lineTo(-rectWidth / 2, -notchSize);
    path.lineTo(-rectWidth / 2, -(notchSize + rectHeight));
    path.lineTo(rectWidth / 2, -(notchSize + rectHeight));
    path.lineTo(rectWidth / 2, -notchSize);
    path.lineTo(notchSize, -notchSize);
    path.closeSubpath();

    painter.translate(pixelX, plotArea.top() + rectHeight + notchSize);
    m_labelRect = QRect(pixelX - rectWidth / 2, plotArea.top(), rectWidth, rectHeight + notchSize);

    painter.setPen(QPen(m_color, 2));
    painter.setBrush(m_bgColor);
    painter.drawPath(path);

    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);

    painter.translate(0, -(notchSize + (rectHeight / 2)));
    painter.rotate(-90);
    painter.setPen(m_color);
    painter.drawText(QRect(-rectWidth / 2, -rectHeight / 2, rectWidth, rectHeight), Qt::AlignCenter,
                     QString("M%1").arg(m_cursorIndex));

    painter.restore();
}

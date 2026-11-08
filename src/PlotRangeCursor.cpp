#include "PlotRangeCursor.h"

PlotRangeCursor::PlotRangeCursor(int index, QColor color, PlotCursor::MarkerStyle style)
    : m_color(color), m_start(index, color, style), m_end(index, color, style)
{}

bool PlotRangeCursor::contains(const QPoint& pos) const
{
    return m_enabled && (m_start.contains(pos) || m_end.contains(pos));
}

bool PlotRangeCursor::mousePress(QMouseEvent* event, const PlotContext& ctx)
{
    if (!m_enabled)
        return false;
    if (m_start.mousePress(event, ctx))
        m_active = &m_start;
    else if (m_end.mousePress(event, ctx))
        m_active = &m_end;
    return m_active != nullptr;
}

void PlotRangeCursor::mouseMove(QMouseEvent* event, const PlotContext& ctx)
{
    if (m_active)
        m_active->mouseMove(event, ctx);
}

void PlotRangeCursor::mouseRelease(QMouseEvent* event, const PlotContext& ctx)
{
    if (m_active)
        m_active->mouseRelease(event, ctx);
    m_active = nullptr;
    checkReorder();
}

bool PlotRangeCursor::hoverShape(QPoint pos, Qt::CursorShape& shape) const
{
    return m_enabled && (m_start.hoverShape(pos, shape) || m_end.hoverShape(pos, shape));
}

void PlotRangeCursor::draw(QPainter& painter, const PlotContext& ctx)
{
    if (!m_enabled)
        return;

    const QRect area = ctx.view.area();
    const int   x0   = ctx.view.xToPixel(m_start.getPos());
    const int   x1   = ctx.view.xToPixel(m_end.getPos());

    if (m_start.isVisible() && m_end.isVisible())
    {
        QColor bg = m_color;
        bg.setAlpha(20);
        painter.save();
        painter.setBrush(QBrush(bg));
        painter.setPen(Qt::PenStyle::NoPen);
        painter.drawRect(QRect(QPoint(x0, area.top()), QPoint(x1, area.bottom())));
        painter.restore();
    }
    m_start.draw(painter, ctx);
    m_end.draw(painter, ctx);
}

bool PlotRangeCursor::checkReorder()
{
    const double start = m_start.getPos();
    const double end   = m_end.getPos();
    if (start <= end)
        return false;
    m_start.setPos(end);
    m_end.setPos(start);
    return true;
}

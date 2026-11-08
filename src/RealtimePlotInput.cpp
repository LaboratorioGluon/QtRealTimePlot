// Mouse and keyboard handling for RealtimePlot.
//
// A single Interaction state (Idle, Panning, BoxSelecting, DraggingOverlay)
// drives every gesture, so only one can be active at a time.

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include "RealtimePlot.h"

// ==========================================================================
//  Wheel zoom
// ==========================================================================
void RealtimePlot::wheelEvent(QWheelEvent* event)
{
    const QPoint pos = event->position().toPoint();

    PlotView::Axes axes;
    switch (m_view.regionAt(pos))
    {
        case PlotView::Region::XAxis:
            axes = PlotView::AxisX;
            break;
        case PlotView::Region::YAxis:
            axes = PlotView::AxisY;
            break;
        case PlotView::Region::Plot:
            axes = zoomAxes();
            break;
        default:
            event->ignore();
            return;
    }

    m_zoomAuto          = false;
    // Wheel up widens the range (zoom out), as before.
    const double factor = (event->angleDelta().y() > 0) ? 0.85 : 1.15;
    m_view.zoom(factor, pos, axes);
    viewModified();
    event->accept();
}

// ==========================================================================
//  Mouse press
// ==========================================================================
void RealtimePlot::mousePressEvent(QMouseEvent* event)
{
    if (m_interaction.state != Interaction::State::Idle)
    {
        event->accept();
        return;
    }

    const QPoint pos = event->pos();

    // Overlays get the first chance, topmost first.
    if (event->button() == Qt::LeftButton)
    {
        const PlotContext ctx  = overlayContext();
        auto              list = overlays();
        for (auto it = list.rbegin(); it != list.rend(); ++it)
        {
            if ((*it)->enabled() && (*it)->mousePress(event, ctx))
            {
                m_interaction.state   = Interaction::State::DraggingOverlay;
                m_interaction.overlay = *it;
                update();
                event->accept();
                return;
            }
        }
    }

    const PlotView::Region region = m_view.regionAt(pos);

    if (event->button() == Qt::LeftButton && region != PlotView::Region::Outside)
    {
        // Left drag pans. Dragging an axis moves only that axis.
        m_zoomAuto            = false;
        m_interaction.state   = Interaction::State::Panning;
        m_interaction.lastPos = pos;
        m_interaction.panAxes = region == PlotView::Region::XAxis   ? PlotView::AxisX
                                : region == PlotView::Region::YAxis ? PlotView::AxisY
                                                                    : zoomAxes();
        setCursor(region == PlotView::Region::XAxis   ? Qt::SizeHorCursor
                  : region == PlotView::Region::YAxis ? Qt::SizeVerCursor
                                                      : Qt::ClosedHandCursor);
    }
    else if (event->button() == Qt::RightButton && region == PlotView::Region::Plot)
    {
        // Right drag draws a zoom rectangle.
        m_zoomAuto             = false;
        m_interaction.state    = Interaction::State::BoxSelecting;
        m_interaction.selStart = pos;
        m_interaction.selEnd   = pos;
        setCursor(Qt::CrossCursor);
    }
    else if (event->button() == Qt::MiddleButton)
    {
        m_zoomAuto            = false;
        m_interaction.state   = Interaction::State::Panning;
        m_interaction.lastPos = pos;
        m_interaction.panAxes = zoomAxes();
        setCursor(Qt::ClosedHandCursor);
    }

    event->accept();
}

// ==========================================================================
//  Mouse move
// ==========================================================================
void RealtimePlot::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint pos = event->pos();

    switch (m_interaction.state)
    {
        case Interaction::State::DraggingOverlay:
            m_interaction.overlay->mouseMove(event, overlayContext());
            setCursor(Qt::SizeHorCursor);
            update();
            break;

        case Interaction::State::Panning:
            m_view.pan(pos - m_interaction.lastPos, m_interaction.panAxes);
            m_interaction.lastPos = pos;
            viewModified();
            break;

        case Interaction::State::BoxSelecting:
            m_interaction.selEnd = pos;
            update();
            break;

        case Interaction::State::Idle: {
            // Hover feedback only.
            Qt::CursorShape shape = Qt::ArrowCursor;
            auto            list  = overlays();
            for (auto it = list.rbegin(); it != list.rend(); ++it)
            {
                if ((*it)->enabled() && (*it)->hoverShape(pos, shape))
                {
                    setCursor(shape);
                    event->accept();
                    return;
                }
            }
            switch (m_view.regionAt(pos))
            {
                case PlotView::Region::XAxis:
                    setCursor(Qt::SizeHorCursor);
                    break;
                case PlotView::Region::YAxis:
                    setCursor(Qt::SizeVerCursor);
                    break;
                default:
                    unsetCursor();
                    break;
            }
            break;
        }
    }
    event->accept();
}

// ==========================================================================
//  Mouse release
// ==========================================================================
void RealtimePlot::mouseReleaseEvent(QMouseEvent* event)
{
    switch (m_interaction.state)
    {
        case Interaction::State::DraggingOverlay:
            m_interaction.overlay->mouseRelease(event, overlayContext());
            m_interaction.overlay = nullptr;
            m_interaction.state   = Interaction::State::Idle;
            unsetCursor();
            update();
            break;

        case Interaction::State::Panning:
            m_interaction.state = Interaction::State::Idle;
            unsetCursor();
            break;

        case Interaction::State::BoxSelecting: {
            if (event->button() != Qt::RightButton)
                break;
            m_interaction.state = Interaction::State::Idle;
            unsetCursor();

            const QRect sel = QRect(m_interaction.selStart, m_interaction.selEnd).normalized();
            // Ignore tiny rectangles (accidental clicks) and ones outside the plot.
            if (sel.width() >= 8 && sel.height() >= 8 && m_view.area().intersects(sel))
            {
                m_view.zoomToPixelRect(sel, zoomAxes());
                viewModified();
            }
            update();
            break;
        }

        case Interaction::State::Idle:
            break;
    }
    event->accept();
}

// ==========================================================================
//  Double-click and keyboard
// ==========================================================================
void RealtimePlot::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        autoFit();
    event->accept();
}

void RealtimePlot::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_F:
        case Qt::Key_Space:
            autoFit();
            break;
        case Qt::Key_X:
            setZoomMode(ZoomMode::XOnly);
            break;
        case Qt::Key_Y:
            setZoomMode(ZoomMode::YOnly);
            break;
        case Qt::Key_B:
            setZoomMode(ZoomMode::XY);
            break;
        default:
            QOpenGLWidget::keyPressEvent(event);
            return;
    }
    event->accept();
}

#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include "RealtimePlot.h"

// ==========================================================================
//  Wheel zoom
// ==========================================================================
void RealtimePlot::wheelEvent(QWheelEvent* event)
{
    QPoint pos      = event->position().toPoint();
    QRect  rectArea = plotArea();

    ZoomMode modoOriginal = m_zoomMode;
    m_zoomAuto            = false;

    if (pos.x() >= rectArea.left() && pos.x() <= rectArea.right() &&
        pos.y() > rectArea.bottom())
    {

        m_zoomMode = ZoomMode::XOnly;
    }
    else if (pos.x() < rectArea.left() && pos.y() >= rectArea.top() &&
             pos.y() <= rectArea.bottom())
    {

        m_zoomMode = ZoomMode::YOnly;
    }
    else if (rectArea.contains(pos))
    {

        m_zoomMode = modoOriginal;
    }
    else
    {

        event->ignore();
        return;
    }

    double angleDelta = event->angleDelta().y();
    double factor     = (angleDelta > 0) ? 0.85 : 1.15;

    applyZoom(factor, pos);

    m_zoomMode = modoOriginal;

    event->accept();
}

// --------------------------------------------------------------------------
void RealtimePlot::applyZoom(double factor, QPoint anchor)
{
    QPointF dataAnchor = pixelToData(anchor);

    if (m_zoomMode == ZoomMode::XY || m_zoomMode == ZoomMode::XOnly)
    {
        double distLeft  = dataAnchor.x() - m_xMin;
        double distRight = m_xMax - dataAnchor.x();

        m_xMin = dataAnchor.x() - distLeft / factor;
        m_xMax = dataAnchor.x() + distRight / factor;
    }

    if (m_zoomMode == ZoomMode::XY || m_zoomMode == ZoomMode::YOnly)
    {
        double distBottom = dataAnchor.y() - m_yMin;
        double distTop    = m_yMax - dataAnchor.y();

        m_yMin = dataAnchor.y() - distBottom / factor;
        m_yMax = dataAnchor.y() + distTop / factor;
    }

    emitViewChanged();
    update();
}

// ==========================================================================
//  Mouse press
// ==========================================================================
void RealtimePlot::mousePressEvent(QMouseEvent* event)
{
    QPoint      pos  = event->pos();
    const QRect area = plotArea();

    if (m_legendVisible)
    {
        int clickedIndex = m_legend.checkClick(event->pos());
        if (clickedIndex != -1)
        {
            bool newVisibility = !m_series[clickedIndex]->visible();
            m_series[clickedIndex]->setVisible(newVisibility);
            emit visibilityChanged();
            update();  // Forzar repintado
            event->accept();
            return;
        }
    }

    for (auto& c : m_cursors)
    {
        if (c.enabled())
        {
            int px1 = dataToPixel(c.getPos(), m_yMin).x();
            // int px2 = dataToPixel(m_cursorX2, m_yMin).x();

            if (c.mousePressEvent(event, px1))
            {
                m_activeCursorRef = &c;
                update();
                event->accept();
                return;
            }
        }
    }

    if (m_cursorRange.enabled())
    {
        int pxStart = dataToPixel(m_cursorRange.getPosStart(), m_yMin).x();
        int pxEnd   = dataToPixel(m_cursorRange.getPosEnd(), m_yMin).x();
        m_activeCursorRef =
            m_cursorRange.mousePressEvent(event, pxStart, pxEnd);
    }

    if (m_activeCursor != CursorType::None)
    {
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && area.contains(pos))
    {
        // Left drag = pan
        m_zoomAuto     = false;
        m_panning      = true;
        m_selecting    = false;
        m_lastMousePos = pos;
        setCursor(Qt::ClosedHandCursor);
    }
    else if (event->button() == Qt::RightButton && area.contains(pos))
    {
        // Right drag = box-zoom selection
        m_selecting = true;
        m_zoomAuto  = false;
        m_panning   = false;
        m_selStart  = pos;
        m_selEnd    = pos;
        setCursor(Qt::CrossCursor);
    }
    else if (event->button() == Qt::MiddleButton)
    {
        m_panning      = true;
        m_lastMousePos = pos;
        setCursor(Qt::ClosedHandCursor);
    }

    event->accept();
}

// ==========================================================================
//  Mouse move
// ==========================================================================
void RealtimePlot::mouseMoveEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();

    if (m_activeCursorRef)
    {
        int targetPixelX = 0;

        if (m_activeCursorRef->mouseMoveEvent(event, plotArea(), targetPixelX))
        {
            double newDataX = pixelToData(QPoint(targetPixelX, 0)).x();
            m_activeCursorRef->setPos(newDataX);

            setCursor(Qt::SizeHorCursor);

            update();
            event->accept();
            return;
        }
    }
    else if (m_panning)
    {
        QPoint delta   = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();

        const QRect area = plotArea();
        double      dx   = -delta.x() * (m_xMax - m_xMin) / area.width();
        double      dy   = delta.y() * (m_yMax - m_yMin) / area.height();

        if (m_zoomMode != ZoomMode::YOnly)
        {
            m_xMin += dx;
            m_xMax += dx;
        }
        if (m_zoomMode != ZoomMode::XOnly)
        {
            m_yMin += dy;
            m_yMax += dy;
        }

        emitViewChanged();
        update();
    }
    else if (m_selecting)
    {
        m_selEnd = event->pos();
        update();
    }
    else
    {
        for (const auto& c : m_cursors)
        {
            if (c.enabled())
            {
                if (c.contains(pos))
                {
                    setCursor(Qt::SizeHorCursor);
                    event->accept();
                    return;
                }
            }
        }

        if (m_legendVisible)
        {
            if (m_legend.lastRenderedRect().contains(event->pos()))
            {
                setCursor(Qt::PointingHandCursor);
                update();  // Forzar repintado
                event->accept();
                return;
            }
        }

        QRect rectArea = plotArea();
        if (pos.x() >= rectArea.left() && pos.x() <= rectArea.right() &&
            pos.y() > rectArea.bottom())
        {

            setCursor(Qt::SizeHorCursor);
            event->accept();
            return;
        }
        else if (pos.x() < rectArea.left() && pos.y() >= rectArea.top() &&
                 pos.y() <= rectArea.bottom())
        {

            setCursor(Qt::SizeVerCursor);
            event->accept();
            return;
        }
    }

    // If we arrive here, there is nothing special, reset cursor
    unsetCursor();

    event->accept();
}

// ==========================================================================
//  Mouse release
// ==========================================================================
void RealtimePlot::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_panning)
    {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
    }

    if (m_activeCursorRef)
    {
        m_cursorRange.checkReorder();
        m_activeCursorRef = nullptr;

        unsetCursor();
        event->accept();
        return;
    }

    if (m_selecting && event->button() == Qt::RightButton)
    {
        m_selecting = false;
        setCursor(Qt::ArrowCursor);

        QRect sel = QRect(m_selStart, m_selEnd).normalized();

        // Minimum selection size (avoid accidental micro-zooms)
        if (sel.width() < 8 || sel.height() < 8)
        {
            update();
            return;
        }

        // Only zoom to selection box if inside plot area
        if (plotArea().intersects(sel))
        {
            QPointF dataTL = pixelToData(sel.topLeft());
            QPointF dataBR = pixelToData(sel.bottomRight());

            double newXmin = std::min(dataTL.x(), dataBR.x());
            double newXmax = std::max(dataTL.x(), dataBR.x());
            double newYmin = std::min(dataTL.y(), dataBR.y());
            double newYmax = std::max(dataTL.y(), dataBR.y());

            if (m_zoomMode != ZoomMode::YOnly)
            {
                m_xMin = newXmin;
                m_xMax = newXmax;
            }
            if (m_zoomMode != ZoomMode::XOnly)
            {
                m_yMin = newYmin;
                m_yMax = newYmax;
            }

            emitViewChanged();
        }

        update();
    }

    event->accept();
}

// ==========================================================================
//  Double-click → auto-fit
// ==========================================================================
void RealtimePlot::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        autoFit();
        update();
    }
    event->accept();
}

// ==========================================================================
//  Keyboard shortcuts
// ==========================================================================
void RealtimePlot::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_F:
        case Qt::Key_Space:
            autoFit();
            update();
            break;
        case Qt::Key_X:
            setZoomMode(ZoomMode::XOnly);
            break;
        case Qt::Key_Y:
            setZoomMode(ZoomMode::YOnly);
            break;
        case Qt::Key_B:  // Both
            setZoomMode(ZoomMode::XY);
            break;

        default:
            QOpenGLWidget::keyPressEvent(event);
            return;
    }
    event->accept();
}

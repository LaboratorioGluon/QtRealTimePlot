#include "RealtimePlot.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <cmath>

// ==========================================================================
//  Wheel zoom
// ==========================================================================
void RealtimePlot::wheelEvent(QWheelEvent *event)
{
    // Zoom factor: 1.15 per 120-unit step (one notch)
    double delta = event->angleDelta().y();
    double factor = std::pow(1.15, delta / 120.0);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QPoint anchor = event->position().toPoint();
#else
    QPoint anchor = event->pos();
#endif

    // Only zoom if cursor is inside the plot area
    if (!plotArea().contains(anchor))
    {
        event->ignore();
        return;
    }

    applyZoom(factor, anchor);
    m_autoScroll = false; // disable auto-scroll when user zooms
    event->accept();
}

// --------------------------------------------------------------------------
void RealtimePlot::applyZoom(double factor, QPoint anchor)
{
    QPointF dataAnchor = pixelToData(anchor);

    if (m_zoomMode == ZoomMode::XY || m_zoomMode == ZoomMode::XOnly)
    {
        double distLeft = dataAnchor.x() - m_xMin;
        double distRight = m_xMax - dataAnchor.x();

        m_xMin = dataAnchor.x() - distLeft / factor;
        m_xMax = dataAnchor.x() + distRight / factor;
    }

    if (m_zoomMode == ZoomMode::XY || m_zoomMode == ZoomMode::YOnly)
    {
        double distBottom = dataAnchor.y() - m_yMin;
        double distTop = m_yMax - dataAnchor.y();

        m_yMin = dataAnchor.y() - distBottom / factor;
        m_yMax = dataAnchor.y() + distTop / factor;
    }

    emitViewChanged();
    update();
}

// ==========================================================================
//  Mouse press
// ==========================================================================
void RealtimePlot::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    const QRect area = plotArea();

    if (m_showXCursors)
    {
        int px1 = dataToPixel(m_cursorX1, m_yMin).x();
        int px2 = dataToPixel(m_cursorX2, m_yMin).x();

        // Comprobamos línea O caja de texto
        if (std::abs(pos.x() - px1) <= m_clickTolerancePixels || m_rectLabelX1.contains(pos))
        {
            m_activeCursor = CursorType::X1;
            m_cursorDeltaX = m_cursorX1 - pixelToData(pos).x();
        }
        else if (std::abs(pos.x() - px2) <= m_clickTolerancePixels || m_rectLabelX2.contains(pos))
        {
            m_activeCursor = CursorType::X2;
            m_cursorDeltaX = m_cursorX2 - pixelToData(pos).x();
        }
    }

    if (m_showYCursors && m_activeCursor == CursorType::None)
    {
        int py1 = dataToPixel(m_xMin, m_cursorY1).y();
        int py2 = dataToPixel(m_xMin, m_cursorY2).y();

        if (std::abs(pos.y() - py1) <= m_clickTolerancePixels || m_rectLabelY1.contains(pos))
        {
            m_activeCursor = CursorType::Y1;
        }
        else if (std::abs(pos.y() - py2) <= m_clickTolerancePixels || m_rectLabelY2.contains(pos))
        {
            m_activeCursor = CursorType::Y2;
        }
    }

    if (m_activeCursor != CursorType::None)
    {
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && area.contains(pos))
    {
        // Left drag = pan
        m_panning = true;
        m_selecting = false;
        m_lastMousePos = pos;
        setCursor(Qt::ClosedHandCursor);
    }
    else if (event->button() == Qt::RightButton && area.contains(pos))
    {
        // Right drag = box-zoom selection
        m_selecting = true;
        m_panning = false;
        m_selStart = pos;
        m_selEnd = pos;
        setCursor(Qt::CrossCursor);
    }
    else if (event->button() == Qt::MiddleButton)
    {
        m_panning = true;
        m_lastMousePos = pos;
        setCursor(Qt::ClosedHandCursor);
    }

    event->accept();
}

// ==========================================================================
//  Mouse move
// ==========================================================================
void RealtimePlot::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    bool overCursor = false;

    if (m_showXCursors)
    {
        int px1 = dataToPixel(m_cursorX1, m_yMin).x();
        int px2 = dataToPixel(m_cursorX2, m_yMin).x();

        if (std::abs(pos.x() - px1) <= m_clickTolerancePixels || m_rectLabelX1.contains(pos) ||
            std::abs(pos.x() - px2) <= m_clickTolerancePixels || m_rectLabelX2.contains(pos))
        {
            setCursor(Qt::SizeHorCursor);
            overCursor = true;
        }
    }

    if (m_showYCursors && !overCursor)
    {
        int py1 = dataToPixel(m_xMin, m_cursorY1).y();
        int py2 = dataToPixel(m_xMin, m_cursorY2).y();

        if (std::abs(pos.y() - py1) <= m_clickTolerancePixels || m_rectLabelY1.contains(pos) ||
            std::abs(pos.y() - py2) <= m_clickTolerancePixels || m_rectLabelY2.contains(pos))
        {
            setCursor(Qt::SizeVerCursor);
            overCursor = true;
        }
    }

    if (m_activeCursor != CursorType::None)
    {
        QPointF dataPos = pixelToData(pos);

        if (m_activeCursor == CursorType::X1)
            m_cursorX1 = dataPos.x() + m_cursorDeltaX;
        else if (m_activeCursor == CursorType::X2)
            m_cursorX2 = dataPos.x() + m_cursorDeltaX;
        else if (m_activeCursor == CursorType::Y1)
            m_cursorY1 = dataPos.y();
        else if (m_activeCursor == CursorType::Y2)
            m_cursorY2 = dataPos.y();

        update(); // Forzar repintado inmediato
        event->accept();
        return;
    }

    if (m_panning)
    {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();

        const QRect area = plotArea();
        double dx = -delta.x() * (m_xMax - m_xMin) / area.width();
        double dy = delta.y() * (m_yMax - m_yMin) / area.height();

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

        m_autoScroll = false;
        emitViewChanged();
        update();
    }
    else if (m_selecting)
    {
        m_selEnd = event->pos();
        update();
    }
    else if (!overCursor)
    {
        setCursor(Qt::ArrowCursor);
    }

    event->accept();
}

// ==========================================================================
//  Mouse release
// ==========================================================================
void RealtimePlot::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_panning)
    {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
    }

    if (m_activeCursor != CursorType::None)
    {
        m_activeCursor = CursorType::None;
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

            m_autoScroll = false;
            emitViewChanged();
        }

        update();
    }

    event->accept();
}

// ==========================================================================
//  Double-click → auto-fit
// ==========================================================================
void RealtimePlot::mouseDoubleClickEvent(QMouseEvent *event)
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
void RealtimePlot::keyPressEvent(QKeyEvent *event)
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
    case Qt::Key_B: // Both
        setZoomMode(ZoomMode::XY);
        break;
    case Qt::Key_S: // Toggle auto-scroll
        m_autoScroll = !m_autoScroll;
        break;
    default:
        QOpenGLWidget::keyPressEvent(event);
        return;
    }
    event->accept();
}

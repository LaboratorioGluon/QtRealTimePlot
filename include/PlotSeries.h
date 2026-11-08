#pragma once

#include <QColor>
#include <QString>

#include "SampleBuffer.h"

/**
 * @class PlotSeries
 * @brief A sample buffer plus how it is drawn: name, color, line width and visibility.
 *
 * Data methods (inherited from SampleBuffer) are thread-safe and may be called
 * from acquisition threads. The appearance methods declared here belong to the
 * GUI thread. Every appearance setter bumps version() so plots repaint.
 *
 * A series holds no GPU resources, so the same series can be shown in several plots.
 */
class PlotSeries : public SampleBuffer
{
   public:
    /**
     * @param name Identifying name of the series (e.g., "Channel 1").
     * @param color Line color (Default: Qt::cyan).
     * @param maxPoints Maximum number of raw samples kept. 0 means unlimited.
     */
    explicit PlotSeries(QString name, QColor color = Qt::cyan, size_t maxPoints = 0)
        : SampleBuffer(maxPoints), m_name(std::move(name)), m_color(color)
    {}

    /** @return The current name of the series. */
    const QString& name() const { return m_name; }
    /** @param n New name for the series. */
    void           setName(const QString& n)
    {
        m_name = n;
        bumpVersion();
    }

    /** @return The color assigned to this series. */
    QColor color() const { return m_color; }
    /** @param c New color for the series rendering. */
    void   setColor(QColor c)
    {
        m_color = c;
        bumpVersion();
    }

    /** @return True if the series is currently configured to be visible. */
    bool visible() const { return m_visible; }
    /** @param v Desired visibility state. */
    void setVisible(bool v)
    {
        m_visible = v;
        bumpVersion();
    }

    /** @return Line width in logical pixels. */
    float lineWidth() const { return m_lineWidth; }
    /** @param w New line width in logical pixels (e.g., 1.5f). */
    void  setLineWidth(float w)
    {
        m_lineWidth = w;
        bumpVersion();
    }

   private:
    QString m_name;
    QColor  m_color;
    bool    m_visible   = true;
    float   m_lineWidth = 1.5f;
};

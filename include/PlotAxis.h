#pragma once

#include <QString>
#include <QVector>

/**
 * @class PlotAxis
 * @brief One plot axis: its label and its tick marks.
 *
 * Ticks use the "nice numbers" algorithm (steps of 1, 2 or 5 times a power of ten).
 * Labels are kept short at any scale: very large or very small values are shown
 * with a shared multiplier (e.g. "×1e-21"), and a deep zoom far from zero is shown
 * relative to a shared offset (e.g. "+10.2"). The multiplier and offset are
 * returned once per axis in TickSet::scaleLabel.
 */
class PlotAxis
{
   public:
    /** @brief One tick mark: its data value and the text shown next to it. */
    struct Tick {
        double  value;
        QString label;
    };

    /**
     * @brief Ticks of one axis plus the text that completes their labels.
     *
     * A tick's value equals label × 10^exponent + offset.
     */
    struct TickSet {
        QVector<Tick> ticks;
        double        offset   = 0.0; /**< Added to every scaled label. */
        int           exponent = 0;   /**< Power of ten multiplying every label. */
        QString       scaleLabel;     /**< "×1eN" and/or "+offset" to show once on the axis. Empty if not needed. */
    };

    /**
     * @param pixelsPerTick Approximate spacing between ticks, in pixels.
     */
    explicit PlotAxis(int pixelsPerTick = 80) : m_pixelsPerTick(pixelsPerTick) {}

    /** @param label Text drawn along the axis (e.g. "Time (s)"). */
    void           setLabel(const QString& label) { m_label = label; }
    const QString& label() const { return m_label; }

    /** @param px Approximate spacing between ticks, in pixels. */
    void setPixelsPerTick(int px) { m_pixelsPerTick = px > 0 ? px : 1; }
    int  pixelsPerTick() const { return m_pixelsPerTick; }

    /**
     * @brief Ticks for the range [lo, hi] drawn over an axis lengthPx pixels long.
     * At least 10 ticks are requested, as before, so short axes still get labels.
     */
    TickSet ticks(double lo, double hi, int lengthPx) const
    {
        const int byLength = m_pixelsPerTick > 0 ? lengthPx / m_pixelsPerTick : 10;
        return computeTicks(lo, hi, byLength > 10 ? byLength : 10);
    }

    /** @brief Ticks for [lo, hi] targeting about maxTicks labels. */
    static TickSet computeTicks(double lo, double hi, int maxTicks = 8);

    /**
     * @brief Formats a single value with a given number of significant digits,
     * switching to exponent notation for very large or very small magnitudes.
     */
    static QString formatValue(double value, int significantDigits = 4);

   private:
    QString m_label;
    int     m_pixelsPerTick;
};

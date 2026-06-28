#pragma once

#include <QString>
#include <QVector>
#include <cmath>

/**
 * @brief Computes human-friendly tick marks for a given range.
 *
 * Uses the "nice numbers" algorithm to choose a step size that
 * produces 4–10 readable labels regardless of zoom level.
 */
class PlotAxis
{
   public:
    struct Tick {
        double  value;
        QString label;
    };

    // Recompute ticks for [lo, hi] targeting ~maxTicks labels
    static QVector<Tick> computeTicks(double lo, double hi, int maxTicks = 8);

    // Format a value with appropriate precision given the step size
    static QString formatValue(double value, double step);

   private:
    static double niceNum(double range, bool round);
};

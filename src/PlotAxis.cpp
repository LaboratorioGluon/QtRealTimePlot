#include "PlotAxis.h"
#include <cmath>

double PlotAxis::niceNum(double range, bool round)
{
    double exponent = std::floor(std::log10(range));
    double fraction = range / std::pow(10.0, exponent);
    double niceFraction;

    if (round) {
        if      (fraction < 1.5) niceFraction = 1.0;
        else if (fraction < 3.0) niceFraction = 2.0;
        else if (fraction < 7.0) niceFraction = 5.0;
        else                     niceFraction = 10.0;
    } else {
        if      (fraction <= 1.0) niceFraction = 1.0;
        else if (fraction <= 2.0) niceFraction = 2.0;
        else if (fraction <= 5.0) niceFraction = 5.0;
        else                      niceFraction = 10.0;
    }
    return niceFraction * std::pow(10.0, exponent);
}

QVector<PlotAxis::Tick> PlotAxis::computeTicks(double lo, double hi, int maxTicks)
{
    QVector<Tick> ticks;
    if (hi <= lo || !std::isfinite(lo) || !std::isfinite(hi))
        return ticks;

    double range  = niceNum(hi - lo, false);
    double step   = niceNum(range / (maxTicks - 1), true);
    double niceMin = std::floor(lo / step) * step;
    double niceMax = std::ceil (hi / step) * step;

    for (double v = niceMin; v <= niceMax + step * 0.5; v += step) {
        if (v >= lo - step * 0.01 && v <= hi + step * 0.01) {
            ticks.append({v, formatValue(v, step)});
        }
    }
    return ticks;
}

QString PlotAxis::formatValue(double value, double step)
{
    // Determine decimal places from step magnitude
    int decimals = 0;
    if (step < 1.0) {
        decimals = static_cast<int>(std::ceil(-std::log10(step)));
        decimals = std::min(decimals, 6);
    }
    // Use engineering notation for very large or very small values
    if (std::abs(value) >= 1e6 || (std::abs(value) < 1e-3 && value != 0.0))
        return QString::number(value, 'e', decimals > 0 ? decimals : 2);

    return QString::number(value, 'f', decimals);
}

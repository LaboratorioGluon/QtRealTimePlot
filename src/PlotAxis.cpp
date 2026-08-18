#include "PlotAxis.h"
#include <cmath>

double PlotAxis::niceNum(double range, bool round)
{
    double exponent = std::floor(std::log10(range));
    double fraction = range / std::pow(10.0, exponent);
    double niceFraction;

    if (round)
    {
        if (fraction < 1.5)
            niceFraction = 1.0;
        else if (fraction < 3.0)
            niceFraction = 2.0;
        else if (fraction < 7.0)
            niceFraction = 5.0;
        else
            niceFraction = 10.0;
    }
    else
    {
        if (fraction <= 1.0)
            niceFraction = 1.0;
        else if (fraction <= 2.0)
            niceFraction = 2.0;
        else if (fraction <= 5.0)
            niceFraction = 5.0;
        else
            niceFraction = 10.0;
    }
    return niceFraction * std::pow(10.0, exponent);
}

QVector<PlotAxis::Tick> PlotAxis::computeTicks(double lo, double hi,
                                               int maxTicks)
{
    QVector<Tick> ticks;
    if (hi <= lo || !std::isfinite(lo) || !std::isfinite(hi))
        return ticks;

    double range = hi - lo;
    if (range <= 0.0 || maxTicks <= 0)
        return ticks;

    double offset = lo;
    double relHi  = range;

    double rawStep  = range / maxTicks;
    double exponent = std::floor(std::log10(rawStep));
    double fraction = rawStep / std::pow(10.0, exponent);

    double step;
    if (fraction < 1.5)
        step = 1.0 * std::pow(10.0, exponent);
    else if (fraction < 3.0)
        step = 2.0 * std::pow(10.0, exponent);
    else if (fraction < 7.0)
        step = 5.0 * std::pow(10.0, exponent);
    else
        step = 10.0 * std::pow(10.0, exponent);

    double stepExponent = std::floor(std::log10(step));

    int precision =
        (stepExponent < 0.0) ? static_cast<int>(std::abs(stepExponent)) : 0;

    /*double range   = niceNum(hi - lo, false);
    double step    = niceNum(range / (maxTicks - 1), true);
    double niceMin = std::floor(lo / step) * step;
    double niceMax = std::ceil(hi / step) * step;*/

    double firstAbsTick = std::ceil(lo / step) * step;
    double firstRelTick = firstAbsTick - offset;

    int safetyCounter  = 0;
    int maxSafetyLimit = maxTicks * 2;

    for (double relVal = firstRelTick; relVal <= relHi + (step * 0.1);
         relVal += step)
    {
        if (++safetyCounter > maxSafetyLimit)
        {
            break;  // Evita colgar la app si algo falla matemáticamente
        }

        // Recuperamos el valor absoluto original únicamente para la etiqueta
        double absVal = offset + relVal;

        PlotAxis::Tick t;
        t.value = absVal;

        // Formateamos inteligentemente según el nivel de zoom
        t.label = QString::number(absVal, 'f', precision);

        ticks.append(t);
    }
    return ticks;
}

QString PlotAxis::formatValue(double value, double step)
{
    // Determine decimal places from step magnitude
    int decimals = 0;
    if (step < 1.0)
    {
        decimals = static_cast<int>(std::ceil(-std::log10(step)));
        decimals = std::min(decimals, 6);
    }
    // Use engineering notation for very large or very small values
    if (std::abs(value) >= 1e6 || (std::abs(value) < 1e-3 && value != 0.0))
        return QString::number(value, 'e', decimals > 0 ? decimals : 2);

    return QString::number(value, 'f', decimals);
}

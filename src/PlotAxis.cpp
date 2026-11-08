#include "PlotAxis.h"

#include <algorithm>
#include <cmath>

namespace
{

/** Step of 1, 2, 5 or 10 times a power of ten close to rawStep. */
double niceStep(double rawStep)
{
    const double exponent = std::floor(std::log10(rawStep));
    const double base     = std::pow(10.0, exponent);
    const double fraction = rawStep / base;
    if (fraction < 1.5)
        return 1.0 * base;
    if (fraction < 3.0)
        return 2.0 * base;
    if (fraction < 7.0)
        return 5.0 * base;
    return 10.0 * base;
}

/** Decimal places needed to show multiples of step exactly. */
int decimalsFor(double step)
{
    const int exp = static_cast<int>(std::floor(std::log10(step) + 1e-9));
    return exp < 0 ? -exp : 0;
}

/** Shortest plain representation of an offset, up to 12 significant digits. */
QString formatOffset(double offset)
{
    return QString::number(std::abs(offset), 'g', 12);
}

}  // namespace

PlotAxis::TickSet PlotAxis::computeTicks(double lo, double hi, int maxTicks)
{
    TickSet set;
    if (!(hi > lo) || !std::isfinite(lo) || !std::isfinite(hi) || maxTicks <= 0)
        return set;

    const double span = hi - lo;
    const double step = niceStep(span / maxTicks);
    if (!(step > 0.0) || !std::isfinite(step))
        return set;

    // Offset: when the span is tiny compared with the values themselves (a deep zoom
    // far from zero, or timestamps), label ticks relative to a round value instead.
    const double magnitude = std::max(std::abs(lo), std::abs(hi));
    if (magnitude > 0.0 && span * 1e4 < magnitude)
    {
        // A power of ten just above the span, so relative labels need few digits.
        const double unit = std::pow(10.0, std::floor(std::log10(span)) + 1.0);
        set.offset        = std::floor(lo / unit + 1e-9) * unit;  // epsilon: 10.2 / 1e-5 is 1019999.99...
    }

    // Tick values.
    const double first      = std::ceil(lo / step) * step;
    const int    safetyStop = maxTicks * 2 + 2;
    for (int i = 0; i < safetyStop; ++i)
    {
        const double value = first + i * step;
        if (value > hi + step * 1e-6)
            break;
        set.ticks.append(Tick{value, QString()});
    }
    if (set.ticks.isEmpty())
        return set;

    // Exponent: shared power of ten (a multiple of 3) for very large or very small labels.
    double maxRel = 0.0;
    for (const auto& t : set.ticks)
        maxRel = std::max(maxRel, std::abs(t.value - set.offset));
    if (maxRel > 0.0)
    {
        const int e = static_cast<int>(std::floor(std::log10(maxRel) + 1e-9));  // 1e-6 - ulp is still e = -6
        if (e >= 6 || e <= -4)
            set.exponent = static_cast<int>(std::floor(e / 3.0)) * 3;
    }

    // Labels.
    const double scale    = std::pow(10.0, -set.exponent);
    const int    decimals = decimalsFor(step * scale);
    for (auto& t : set.ticks)
    {
        double rel = (t.value - set.offset) * scale;
        if (std::abs(rel) < step * scale * 1e-6)
            rel = 0.0;  // avoid "-0.00"
        t.label = QString::number(rel, 'f', decimals);
    }

    // Text shown once on the axis.
    QStringList parts;
    if (set.exponent != 0)
        parts << QStringLiteral("×1e%1").arg(set.exponent);
    if (set.offset != 0.0)
        parts << (set.offset < 0 ? QStringLiteral("−") : QStringLiteral("+")) + formatOffset(set.offset);
    set.scaleLabel = parts.join(QStringLiteral("  "));

    return set;
}

QString PlotAxis::formatValue(double value, int significantDigits)
{
    if (!std::isfinite(value))
        return QString::number(value);
    if (value == 0.0)
        return QStringLiteral("0");

    const int e = static_cast<int>(std::floor(std::log10(std::abs(value))));
    if (e >= 6 || e <= -4)
        return QString::number(value, 'e', std::max(0, significantDigits - 1));

    // Plain notation with enough decimals for the requested significant digits.
    const int decimals = std::max(0, significantDigits - 1 - e);
    return QString::number(value, 'f', decimals);
}

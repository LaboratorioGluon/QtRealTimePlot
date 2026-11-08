// Checks that tick labels stay short, distinct and exact at every scale.

#include <QSet>
#include <cstdio>

#include "PlotAxis.h"
#include "TestUtil.h"

namespace
{

// Every label must be short, unique, and label * 10^exponent + offset must give the tick value.
void checkRange(double lo, double hi, const char* expectScale)
{
    const auto set = PlotAxis::computeTicks(lo, hi, 10);
    CHECK(set.ticks.size() >= 3);

    QSet<QString> seen;
    const double  step = set.ticks.size() > 1 ? set.ticks[1].value - set.ticks[0].value : hi - lo;
    for (const auto& t : set.ticks)
    {
        CHECK(t.value >= lo - step * 1e-6 && t.value <= hi + step * 1e-6);
        CHECK(t.label.size() <= 8);
        CHECK(!t.label.startsWith(QStringLiteral("-0")) || t.label.toDouble() != 0.0);
        CHECK(!seen.contains(t.label));
        seen.insert(t.label);

        bool         ok   = false;
        const double back = t.label.toDouble(&ok) * std::pow(10.0, set.exponent) + set.offset;
        CHECK(ok);
        CHECK(std::abs(back - t.value) <= step * 1e-3);
    }

    const QString scale = set.scaleLabel;
    if (expectScale)
        CHECK(scale.contains(QString::fromUtf8(expectScale)));
    else
        CHECK(scale.isEmpty());

    std::printf("  [%-22.15g, %-22.15g] scale '%s'  labels:", lo, hi, scale.toUtf8().constData());
    for (const auto& t : set.ticks)
        std::printf(" %s", t.label.toUtf8().constData());
    std::printf("\n");
}

}  // namespace

int main()
{
    // Ordinary ranges: plain labels, nothing shared.
    checkRange(0, 100, nullptr);
    checkRange(-1.1, 1.1, nullptr);
    checkRange(0.001, 0.009, nullptr);

    // Tiny values: shared multiplier.
    checkRange(-2.3e-20, 4.3e-20, "×1e-21");
    checkRange(1e-20, 2e-20, "×1e-21");
    checkRange(-5e-9, 5e-9, "×1e-9");

    // Large values: shared multiplier.
    checkRange(1e6, 5e6, "×1e6");
    checkRange(-3e15, 3e15, "×1e15");

    // Deep zoom far from zero, and timestamps: shared offset.
    checkRange(10.2, 10.2 + 1e-6, "×1e-6  +10.2");
    checkRange(1.7e9, 1.7e9 + 100, "+1700000000");
    checkRange(-50.000002, -50.000001, "−50.00001");
    checkRange(1e-20, 1e-20 + 1e-28, "×1e-30  +1e-20");

    // Single values with significant digits.
    CHECK(PlotAxis::formatValue(0.6634) == "0.6634");
    CHECK(PlotAxis::formatValue(3e-20) == "3.000e-20");
    CHECK(PlotAxis::formatValue(-1e6) == "-1.000e+06");
    CHECK(PlotAxis::formatValue(123.456) == "123.5");
    CHECK(PlotAxis::formatValue(0.0) == "0");

    return testResult("test_plot_axis");
}

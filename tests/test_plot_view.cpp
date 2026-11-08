// Checks PlotView conversions, zoom, pan, fit and region detection. No widget or GL needed.

#include <limits>

#include "PlotView.h"
#include "TestUtil.h"

int main()
{
    PlotView v;
    v.setArea(QRect(72, 28, 700, 400));  // left 72, top 28, right 771, bottom 427
    v.setRange(0.0, 100.0, -1.0, 1.0);

    // Conversions round-trip and Y grows upwards.
    CHECK(v.dataToPixel(0.0, -1.0) == QPoint(72, 427));
    CHECK(v.dataToPixel(100.0, 1.0).y() == 427 - 400);
    CHECK(nearlyEqual(v.pixelToData(QPoint(72 + 350, 28)).x(), 50.0));
    CHECK(nearlyEqual(v.pixelToData(QPoint(72, 28)).y(), 1.0));
    CHECK(nearlyEqual(v.pixelToX(v.xToPixel(25.0)), 25.0, 1e-2));
    CHECK(nearlyEqual(v.normX(50.0), 0.5));
    CHECK(nearlyEqual(v.normY(0.0), 0.5));
    // Far off-screen values must not overflow the int conversion.
    CHECK(v.dataToPixel(1e300, 0).x() > 0);

    // Regions.
    CHECK(v.regionAt(QPoint(400, 200)) == PlotView::Region::Plot);
    CHECK(v.regionAt(QPoint(400, 450)) == PlotView::Region::XAxis);
    CHECK(v.regionAt(QPoint(30, 200)) == PlotView::Region::YAxis);
    CHECK(v.regionAt(QPoint(30, 450)) == PlotView::Region::Outside);
    CHECK(v.regionAt(QPoint(790, 200)) == PlotView::Region::Outside);

    // Zoom keeps the anchor fixed and respects the axis selection.
    {
        PlotView     z      = v;
        const QPoint anchor = v.dataToPixel(25.0, 0.5);
        const auto   before = z.pixelToData(anchor);
        z.zoom(2.0, anchor, PlotView::AxisX);
        CHECK(nearlyEqual(z.xMax() - z.xMin(), 50.0));
        CHECK(nearlyEqual(z.yMin(), -1.0) && nearlyEqual(z.yMax(), 1.0));
        CHECK(nearlyEqual(z.pixelToData(anchor).x(), before.x(), 1e-9));

        z.zoom(0.5, anchor, PlotView::AxisBoth);
        CHECK(nearlyEqual(z.xMax() - z.xMin(), 100.0));
        CHECK(nearlyEqual(z.yMax() - z.yMin(), 4.0));
    }

    // Pan follows the mouse: dragging right by a quarter width moves the range left by a quarter span.
    {
        PlotView p = v;
        p.pan(QPoint(175, 0), PlotView::AxisBoth);
        CHECK(nearlyEqual(p.xMin(), -25.0) && nearlyEqual(p.xMax(), 75.0));
        p.pan(QPoint(0, 100), PlotView::AxisX);  // Y movement ignored when only X is selected
        CHECK(nearlyEqual(p.yMin(), -1.0));
        p.pan(QPoint(0, 100), PlotView::AxisY);  // dragging down moves content down: range goes up
        CHECK(nearlyEqual(p.yMin(), -0.5) && nearlyEqual(p.yMax(), 1.5));
    }

    // Box zoom.
    {
        PlotView b = v;
        b.zoomToPixelRect(QRect(v.dataToPixel(10, 0.5), v.dataToPixel(30, -0.5)), PlotView::AxisBoth);
        CHECK(nearlyEqual(b.xMin(), 10.0, 1e-2) && nearlyEqual(b.xMax(), 30.0, 1e-2));
        CHECK(nearlyEqual(b.yMin(), -0.5, 1e-2) && nearlyEqual(b.yMax(), 0.5, 1e-2));
    }

    // Fit pads by a fraction of the span, and by 5 % of the magnitude for zero spans.
    {
        PlotView f = v;
        f.fit(0, 100, 5, 5, 0.05, 0.05);
        CHECK(nearlyEqual(f.xMin(), -5.0) && nearlyEqual(f.xMax(), 105.0));
        CHECK(nearlyEqual(f.yMin(), 4.75) && nearlyEqual(f.yMax(), 5.25));
    }

    // A flat signal is padded relative to its magnitude, not by a fixed 0.5.
    {
        PlotView f = v;
        f.fit(0, 1, 1e-20, 1e-20, 0.05, 0.05);
        CHECK(f.yMin() < 1e-20 && f.yMax() > 1e-20);
        CHECK(f.yMax() - f.yMin() < 1e-20);
        f.fit(0, 1, 0, 0, 0.05, 0.05);
        CHECK(nearlyEqual(f.yMin(), -0.5) && nearlyEqual(f.yMax(), 0.5));
    }

    // Ranges never collapse, invert or go non-finite.
    {
        PlotView m = v;
        for (int i = 0; i < 200; ++i)
            m.zoom(10.0, QPoint(400, 200), PlotView::AxisBoth);
        CHECK(m.xMax() > m.xMin());
        CHECK(m.yMax() > m.yMin());

        m.setRange(5, 1, 3, 3);
        CHECK(m.xMin() == 1 && m.xMax() == 5);
        CHECK(m.yMax() > m.yMin());

        m.setRange(std::numeric_limits<double>::quiet_NaN(), 1, 0, 1);
        CHECK(m.xMin() == 0 && m.xMax() == 1);
    }

    return testResult("test_plot_view");
}

// Checks SampleBuffer storage, LOD tiers, statistics and decimation against brute force.

#include <atomic>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

#include "Decimation.h"
#include "SampleBuffer.h"
#include "TestUtil.h"

using Point = SampleBuffer::Point;

namespace
{

void checkAgainstReference(const SampleBuffer& buf, const std::vector<Point>& ref, std::mt19937& rng)
{
    {
        auto lk = buf.lock();
        CHECK(buf.points().size() == ref.size());
        if (!ref.empty())
        {
            double mn = 1e300, mx = -1e300;
            for (const auto& p : ref)
            {
                mn = std::min(mn, p.y);
                mx = std::max(mx, p.y);
            }
            CHECK(nearlyEqual(buf.yMin(), mn));
            CHECK(nearlyEqual(buf.yMax(), mx));
            CHECK(buf.xMin() == ref.front().x);
            CHECK(buf.xMax() == ref.back().x);
        }
    }
    if (ref.empty())
        return;

    std::uniform_real_distribution<double> dist(ref.front().x - 5, ref.back().x + 5);
    std::vector<Point>                     gathered, visible;
    std::vector<rtp::MinMaxBucket>         buckets;

    for (int k = 0; k < 100; ++k)
    {
        double a = dist(rng), b = dist(rng);
        if (a > b)
            std::swap(a, b);

        double mn = 1e300, mx = -1e300, sum = 0, sq = 0, lastX = -1e300;
        int    n = 0;
        for (const auto& p : ref)
        {
            if (p.x < a || p.x > b)
                continue;
            mn = std::min(mn, p.y);
            mx = std::max(mx, p.y);
            sum += p.y;
            sq += p.y * p.y;
            lastX = p.x;
            ++n;
        }

        const auto st = buf.calculateIntervalStats(a, b);
        CHECK(st.numSamples == n);
        if (n)
        {
            CHECK(nearlyEqual(st.min, mn));
            CHECK(nearlyEqual(st.max, mx));
            CHECK(nearlyEqual(st.mean, sum / n, 1e-6));
            CHECK(nearlyEqual(st.rms, std::sqrt(sq / n), 1e-6));
        }

        // Nearest samples just outside [a, b]; they must be returned so lines reach the edges.
        const Point* before = nullptr;
        const Point* after  = nullptr;
        for (const auto& p : ref)
        {
            if (p.x < a)
                before = &p;
            else if (p.x > b && !after)
                after = &p;
        }

        for (int width : {50, 400, 2000})
        {
            buf.copyVisibleRange(a, b, width, gathered);
            rtp::decimateMinMax(gathered, a, b, width, visible, buckets);

            for (size_t i = 1; i < visible.size(); ++i)
                CHECK(visible[i - 1].x <= visible[i].x);
            CHECK(visible.size() <= static_cast<size_t>(std::max(2 * width, n)) + 2);

            size_t first = 0, last = visible.size();
            if (before)
            {
                CHECK(!visible.empty() && visible.front().x == before->x && visible.front().y == before->y);
                ++first;
            }
            if (after)
            {
                CHECK(!visible.empty() && visible.back().x == after->x && visible.back().y == after->y);
                --last;
            }
            CHECK(visible.size() == (before ? 1u : 0u) + (after ? 1u : 0u) + (last - first));

            double vmn = 1e300, vmx = -1e300, vLastX = -1e300;
            for (size_t i = first; i < last; ++i)
            {
                CHECK(visible[i].x >= a && visible[i].x <= b);
                vmn    = std::min(vmn, visible[i].y);
                vmx    = std::max(vmx, visible[i].y);
                vLastX = visible[i].x;
            }
            CHECK((last > first) == (n > 0));
            if (n)
            {
                // Decimation must keep every peak, and the newest sample must not be lost.
                CHECK(nearlyEqual(vmn, mn));
                CHECK(nearlyEqual(vmx, mx));
                CHECK(vLastX >= lastX - (b - a) / width * 2);
            }
        }
    }

    const Point mid = ref[ref.size() / 2];
    CHECK(buf.getClosestPointToX(mid.x).x == mid.x);
}

}  // namespace

int main()
{
    std::mt19937                     rng(42);
    std::normal_distribution<double> noise(0, 1);

    // Unlimited buffers whose sizes cross every LOD boundary.
    for (size_t n : {0ul, 1ul, 99ul, 100ul, 1000ul, 12345ul, 300000ul})
    {
        SampleBuffer       buf;
        std::vector<Point> ref;
        for (size_t i = 0; i < n; ++i)
        {
            const Point p{i * 0.001, std::sin(i * 0.01) + 0.1 * noise(rng)};
            CHECK(buf.pushPoint(p));
            ref.push_back(p);
        }
        checkAgainstReference(buf, ref, rng);
    }

    // Deep zoom: a view between two samples must still return both of them.
    {
        SampleBuffer buf;
        for (int i = 0; i < 100; ++i)
            buf.pushPoint(i * 1.0, i % 2);
        std::vector<Point> out;

        buf.copyVisibleRange(10.2, 10.8, 800, out);
        CHECK(out.size() == 2 && out[0].x == 10.0 && out[1].x == 11.0);

        buf.copyVisibleRange(10.2, 11.5, 800, out);
        CHECK(out.size() == 3 && out[0].x == 10.0 && out[1].x == 11.0 && out[2].x == 12.0);

        // Before the first and after the last sample: only the one existing neighbour.
        buf.copyVisibleRange(-5.0, -1.0, 800, out);
        CHECK(out.size() == 1 && out[0].x == 0.0);
        buf.copyVisibleRange(200.0, 300.0, 800, out);
        CHECK(out.size() == 1 && out[0].x == 99.0);

        // Extreme zoom: a view a billion times narrower than the sample spacing.
        buf.copyVisibleRange(50.5, 50.5 + 1e-9, 800, out);
        CHECK(out.size() == 2 && out[0].x == 50.0 && out[1].x == 51.0);
    }

    // Coarse detail levels must also be framed by the raw neighbours.
    {
        SampleBuffer buf;
        for (int i = 0; i < 2000000; ++i)
            buf.pushPoint(i * 1.0, std::sin(i * 0.001));
        std::vector<Point> out;
        buf.copyVisibleRange(1000.5, 1999000.5, 100, out);  // ~20000 samples per pixel: coarsest level
        CHECK(out.size() > 2 && out.size() < 2000);
        CHECK(out.front().x == 1000.0 && out.back().x == 1999001.0);
    }

    // Bounded ring buffers.
    for (size_t cap : {1ul, 50ul, 150ul, 10000ul, 120000ul})
    {
        SampleBuffer       buf(cap);
        std::vector<Point> all;
        for (size_t i = 0; i < cap * 3 + 777; ++i)
        {
            const Point p{static_cast<double>(i), (i % 997) * 0.5 - 100 + noise(rng)};
            buf.pushPoint(p);
            all.push_back(p);
        }
        const std::vector<Point> ref(all.end() - std::min(cap, all.size()), all.end());
        checkAgainstReference(buf, ref, rng);

        auto lk = buf.lock();
        for (uint32_t lod = 1; lod < SampleBuffer::NUM_LOD_LEVELS; ++lod)
            if (!buf.getDataPoints(lod).empty())
                CHECK(buf.getDataPoints(lod).front().x >= ref.front().x);
    }

    // Shrinking the capacity later.
    {
        SampleBuffer       buf;
        std::vector<Point> ref;
        for (int i = 0; i < 50000; ++i)
        {
            const Point p{static_cast<double>(i), noise(rng)};
            buf.pushPoint(p);
            ref.push_back(p);
        }
        buf.setMaxPoints(3000);
        ref.erase(ref.begin(), ref.end() - 3000);
        checkAgainstReference(buf, ref, rng);
    }

    // Rejected samples, clear() and reuse.
    {
        SampleBuffer buf;
        const auto   v0 = buf.version();
        CHECK(buf.pushPoint(1, 1));
        CHECK(buf.version() != v0);
        CHECK(!buf.pushPoint(0.5, 1));
        CHECK(!buf.pushPoint(2, NAN));
        CHECK(!buf.pushPoint(INFINITY, 1));
        CHECK(buf.pushPoint(1, 2));
        CHECK(buf.droppedPoints() == 3);
        CHECK(buf.size() == 2);

        for (int i = 0; i < 5000; ++i)
            buf.pushPoint(2 + i, i);
        buf.clear();
        CHECK(buf.size() == 0);
        CHECK(buf.calculateIntervalStats(-1e9, 1e9).numSamples == 0);

        std::vector<Point> ref;
        for (int i = 0; i < 3000; ++i)
        {
            const Point p{static_cast<double>(i), static_cast<double>(-i)};
            buf.pushPoint(p);
            ref.push_back(p);
        }
        checkAgainstReference(buf, ref, rng);
    }

    // A producer thread pushing while this thread reads (run under TSan to check for races).
    {
        SampleBuffer       buf(200000);
        std::atomic<bool>  done{false};
        std::thread        producer([&] {
            double t = 0;
            for (int i = 0; i < 400000; ++i, t += 1e-4)
                buf.pushPoint(t, std::sin(t));
            done = true;
        });
        std::vector<Point> out;
        while (!done)
        {
            double xMax;
            {
                auto lk = buf.lock();
                xMax    = buf.xMax();
            }
            buf.copyVisibleRange(xMax - 10, xMax, 1920, out);
            buf.calculateIntervalStats(xMax - 5, xMax);
            buf.getClosestPointToX(xMax - 1);
            // Read like a render loop, not a busy spin: an unpaced reader can starve the producer.
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        producer.join();
        CHECK(buf.size() == 200000);
    }

    return testResult("test_sample_buffer");
}

# RealtimePlot — Qt/OpenGL real-time plotting library

A lightweight, header-small C++ library for high-rate data plotting inside a
Qt application. Uses OpenGL for fast line rendering with a QPainter overlay
for text, legend, and interactive selection.

---

## Features

| Feature | Details |
|---|---|
| **Wheel zoom** | Zooms toward the cursor; supports X-only, Y-only, or XY mode |
| **Box-zoom** | Right-click + drag draws a selection rectangle; release to zoom in |
| **Pan** | Left-click drag or middle-click drag |
| **Auto-fit** | Double-click or press `F`/`Space` |
| **Auto-scroll** | Follows the newest X value; press `S` to toggle |
| **Multi-series** | Any number of named, colored series |
| **Thread-safe** | `pushPoint()` can be called from any thread (ISR callback, QThread, …) |
| **Tick labels** | "Nice numbers" algorithm — always readable, adapts to zoom level |
| **Legend** | Floating overlay, auto-sized to visible series |

---

## Quick start

```cpp
#include "RealtimePlot.h"

// 1. Create widget
auto* plot = new RealtimePlot(parent);
plot->setTitle("ADC Monitor");
plot->setXLabel("Time (s)");
plot->setYLabel("Voltage (V)");
plot->setRefreshRate(60);          // FPS

// 2. Add series (returns a shared_ptr you keep)
auto ch1 = plot->addSeries("Channel 1", Qt::cyan,   10000);
auto ch2 = plot->addSeries("Channel 2", Qt::yellow, 10000);

// 3. (Optional) auto-scroll to follow live data
plot->setAutoScroll(true);
plot->setAutoScrollWindow(5.0);    // show last 5 s

// 4. Start rendering
plot->start();

// 5. Feed data from any thread
ch1->pushPoint(timestamp_s, voltage_v);
```

---

## Zoom modes

Set via `setZoomMode()` or keyboard:

| Key | Mode |
|-----|------|
| `B` | XY (default) |
| `X` | X axis only |
| `Y` | Y axis only |

---

## Zoom interactions

| Interaction | Effect |
|---|---|
| Mouse wheel | Zoom in/out centered on cursor |
| Right-click + drag | Draw selection box → zoom to region |
| Left-click + drag | Pan |
| Middle-click + drag | Pan |
| Double-click | Auto-fit to all data |
| `F` / `Space` | Auto-fit |
| `S` | Toggle auto-scroll |

---

## Build

Requires **Qt 5.9+** or **Qt 6**, and **OpenGL**.

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
# Run the demo:
./realtimeplot_demo
```

### Integrate into your own project

```cmake
add_subdirectory(realtimeplot)
target_link_libraries(your_target PRIVATE realtimeplot)
```

---

## API reference

### `RealtimePlot`

```cpp
// Series
std::shared_ptr<PlotSeries> addSeries(name, color, maxPoints=10000);
void removeSeries(series);
void clearSeries();

// View
void   autoFit();
void   setViewRange(xMin, xMax, yMin, yMax);
QRectF viewRange() const;

// Timer
void setRefreshRate(int fps);   // default 60
void start();
void stop();

// Zoom
void     setZoomMode(ZoomMode::XY | XOnly | YOnly);
ZoomMode zoomMode() const;

// Auto-scroll
void setAutoScroll(bool);
void setAutoScrollWindow(double dataUnits);

// Style
void setTitle/XLabel/YLabel(QString);
void setBackgroundColor/AxisColor/TextColor(QColor);
void setLegendVisible(bool);
void setGrid(GridStyle{enabled, color});
void setMargins(MarginPx{left, right, top, bottom});

// Signal
void viewChanged(double xMin, double xMax, double yMin, double yMax);
```

### `PlotSeries`

```cpp
// Thread-safe data ingestion
void pushPoint(double x, double y);
void clear();

// Appearance
void setColor(QColor);
void setLineWidth(float);
void setVisible(bool);
void setMaxPoints(size_t);   // circular buffer size

// Bounds (valid after at least one pushPoint)
double xMin(), xMax(), yMin(), yMax();
```

---

## Performance notes

- **CPU transform**: data→pixel conversion happens on the CPU before uploading
  vertices. For ≤50,000 visible points per series this is imperceptible at 60 Hz.
  For higher throughput, consider switching to a VBO + GPU-side transform using
  a `u_dataToNDC` uniform matrix.
- **Circular buffer**: `PlotSeries` uses `std::deque` with a configurable cap.
  At 1 kHz × 10 000 points = 10 s of history, which is comfortable for most
  embedded telemetry use cases.
- **Thread safety**: each series has its own `std::mutex`. The render thread
  acquires it briefly per frame; the data thread acquires it per `pushPoint`.
  Contention is negligible at typical embedded data rates (< 100 kHz).

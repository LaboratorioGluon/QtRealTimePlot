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



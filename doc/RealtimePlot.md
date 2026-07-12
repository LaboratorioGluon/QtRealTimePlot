{#realtimeplot-1}

# RealtimePlot

```cpp
#include <RealtimePlot.h>
```

```cpp
class RealtimePlot
```

Defined in include/RealtimePlot.h:49

> **Inherits:** `QOpenGLWidget`, `QOpenGLFunctions`

High-performance real-time plot widget using OpenGL.

Features:

* Multiple series, each pushed from any thread

* Wheel zoom (X, Y, or both axes) - zooms toward cursor position

* Box-selection zoom (right-click + drag) - with Delta x / Delta y tooltip

* Left-click pan / middle-click pan

* Double-click to auto-fit all visible data

* Keyboard shortcuts: F/Space=fit, X/Y/B=zoom mode, S=auto-scroll

* Optional auto-scroll mode (follows newest X value)

* Configurable grid, legend, labels, margins, colors

```cpp
auto* plot = new RealtimePlot(this);
auto  ch1  = plot->addSeries("ADC Ch1", Qt::cyan);
auto  ch2  = plot->addSeries("ADC Ch2", Qt::yellow);
plot->setTitle("Real-time ADC");
plot->setXLabel("Time (s)");
plot->setYLabel("Voltage (V)");
plot->setRefreshRate(60);
plot->start();

// Push from any thread (including ISR callback / QThread):
ch1->pushPoint(timestamp, voltage);
```

## List of all members

| Name | Kind | Owner |
|------|------|-------|
| [`RealtimePlot`](#realtimeplot-2) | `function` | Declared here |
| [`~RealtimePlot`](#realtimeplot-3) | `function` | Declared here |
| [`addSeries`](#addseries) | `function` | Declared here |
| [`addSeries`](#addseries-1) | `function` | Declared here |
| [`removeSeries`](#removeseries) | `function` | Declared here |
| [`clearSeries`](#clearseries) | `function` | Declared here |
| [`addCursor`](#addcursor) | `function` | Declared here |
| [`addCursor`](#addcursor-1) | `function` | Declared here |
| [`setRangeCursorEnable`](#setrangecursorenable) | `function` | Declared here |
| [`setRangeCursorPos`](#setrangecursorpos) | `function` | Declared here |
| [`autoFit`](#autofit) | `function` | Declared here |
| [`setViewRange`](#setviewrange) | `function` | Declared here |
| [`viewRange`](#viewrange) | `function` | Declared here |
| [`setRefreshRate`](#setrefreshrate) | `function` | Declared here |
| [`start`](#start) | `function` | Declared here |
| [`stop`](#stop) | `function` | Declared here |
| [`setZoomMode`](#setzoommode) | `function` | Declared here |
| [`zoomMode`](#zoommode) | `function` | Declared here |
| [`setGrid`](#setgrid) | `function` | Declared here |
| [`getGrid`](#getgrid) | `function` | Declared here |
| [`setMargins`](#setmargins) | `function` | Declared here |
| [`setBackgroundColor`](#setbackgroundcolor-2) | `function` | Declared here |
| [`setAxisColor`](#setaxiscolor) | `function` | Declared here |
| [`setTextColor`](#settextcolor-2) | `function` | Declared here |
| [`setTitle`](#settitle) | `function` | Declared here |
| [`setXLabel`](#setxlabel) | `function` | Declared here |
| [`setYLabel`](#setylabel) | `function` | Declared here |
| [`addCursor`](#addcursor-2) | `function` | Declared here |
| [`enableRangeCursor`](#enablerangecursor) | `function` | Declared here |
| [`getRangeCursor`](#getrangecursor) | `function` | Declared here |
| [`setLegendVisible`](#setlegendvisible) | `function` | Declared here |
| [`setAutoZoom`](#setautozoom) | `function` | Declared here |
| [`initializeGL`](#initializegl) | `function` | Declared here |
| [`resizeGL`](#resizegl) | `function` | Declared here |
| [`paintGL`](#paintgl) | `function` | Declared here |
| [`wheelEvent`](#wheelevent) | `function` | Declared here |
| [`mousePressEvent`](#mousepressevent-1) | `function` | Declared here |
| [`mouseMoveEvent`](#mousemoveevent-1) | `function` | Declared here |
| [`mouseReleaseEvent`](#mousereleaseevent-1) | `function` | Declared here |
| [`mouseDoubleClickEvent`](#mousedoubleclickevent) | `function` | Declared here |
| [`keyPressEvent`](#keypressevent) | `function` | Declared here |
| [`viewChanged`](#viewchanged) | `signal` | Declared here |
| [`ZoomMode`](#zoommode-1) | `enum` | Declared here |
| [`CursorType`](#cursortype) | `enum` | Declared here |
| [`m_series`](#m_series) | `variable` | Declared here |
| [`m_xMin`](#m_xmin-1) | `variable` | Declared here |
| [`m_xMax`](#m_xmax-1) | `variable` | Declared here |
| [`m_yMin`](#m_ymin-1) | `variable` | Declared here |
| [`m_yMax`](#m_ymax-1) | `variable` | Declared here |
| [`m_viewInitialized`](#m_viewinitialized) | `variable` | Declared here |
| [`m_bgColor`](#m_bgcolor-2) | `variable` | Declared here |
| [`m_axisColor`](#m_axiscolor) | `variable` | Declared here |
| [`m_textColor`](#m_textcolor) | `variable` | Declared here |
| [`m_grid`](#m_grid) | `variable` | Declared here |
| [`m_margin`](#m_margin) | `variable` | Declared here |
| [`m_tickFont`](#m_tickfont) | `variable` | Declared here |
| [`m_labelFont`](#m_labelfont) | `variable` | Declared here |
| [`m_title`](#m_title) | `variable` | Declared here |
| [`m_xLabel`](#m_xlabel) | `variable` | Declared here |
| [`m_yLabel`](#m_ylabel) | `variable` | Declared here |
| [`m_legendVisible`](#m_legendvisible) | `variable` | Declared here |
| [`m_zoomMode`](#m_zoommode) | `variable` | Declared here |
| [`m_zoomAuto`](#m_zoomauto) | `variable` | Declared here |
| [`m_panning`](#m_panning) | `variable` | Declared here |
| [`m_selecting`](#m_selecting) | `variable` | Declared here |
| [`m_lastMousePos`](#m_lastmousepos) | `variable` | Declared here |
| [`m_selStart`](#m_selstart) | `variable` | Declared here |
| [`m_selEnd`](#m_selend) | `variable` | Declared here |
| [`m_timer`](#m_timer) | `variable` | Declared here |
| [`m_shader`](#m_shader) | `variable` | Declared here |
| [`m_shaderReady`](#m_shaderready) | `variable` | Declared here |
| [`m_cursors`](#m_cursors) | `variable` | Declared here |
| [`m_cursorRange`](#m_cursorrange) | `variable` | Declared here |
| [`m_activeCursor`](#m_activecursor) | `variable` | Declared here |
| [`m_activeCursorRef`](#m_activecursorref) | `variable` | Declared here |
| [`m_clickTolerancePixels`](#m_clicktolerancepixels) | `variable` | Declared here |
| [`m_legend`](#m_legend) | `variable` | Declared here |
| [`pixelToData`](#pixeltodata) | `function` | Declared here |
| [`dataToPixel`](#datatopixel) | `function` | Declared here |
| [`plotArea`](#plotarea) | `function` | Declared here |
| [`drawGrid`](#drawgrid) | `function` | Declared here |
| [`drawSeries`](#drawseries) | `function` | Declared here |
| [`drawAxes`](#drawaxes) | `function` | Declared here |
| [`drawLineStrip`](#drawlinestrip) | `function` | Declared here |
| [`drawLines`](#drawlines) | `function` | Declared here |
| [`applyZoom`](#applyzoom) | `function` | Declared here |
| [`emitViewChanged`](#emitviewchanged) | `function` | Declared here |
| [`colorList`](#colorlist) | `variable` | Declared here |

## Public Methods

| Return | Name | Description |
|--------|------|-------------|
|  | [`RealtimePlot`](#realtimeplot-2) `explicit` | Constructor for the high-performance OpenGL plot widget. |
|  | [`~RealtimePlot`](#realtimeplot-3) `override` | Destructor. Safely deallocates GL shaders, objects, and timers. |
| `std::shared_ptr< PlotSeries >` | [`addSeries`](#addseries)  | Dynamically creates and tracks a new plot data channel. |
| `void` | [`addSeries`](#addseries-1)  | Adds a pre-existing externally built series into the rendering manager. |
| `void` | [`removeSeries`](#removeseries)  | Deletes a targeted channel from the workspace. |
| `void` | [`clearSeries`](#clearseries)  | Completely drops all tracking datasets from the widget memory space. |
| `void` | [`addCursor`](#addcursor)  | Add cursor in the the center of the current view. |
| `void` | [`addCursor`](#addcursor-1)  | Add cursor in position. |
| `void` | [`setRangeCursorEnable`](#setrangecursorenable) `inline` |  |
| `void` | [`setRangeCursorPos`](#setrangecursorpos) `inline` |  |
| `void` | [`autoFit`](#autofit)  | Computes data boundaries and updates viewport range parameters to fit content. |
| `void` | [`setViewRange`](#setviewrange)  | Explicitly overrides limits of the observable data window. |
| `QRectF` | [`viewRange`](#viewrange) `const` | Retrieves a floating-point bounding rectangle representing current view extents. |
| `void` | [`setRefreshRate`](#setrefreshrate)  | Programs the target periodic timer update frequency. |
| `void` | [`start`](#start)  | Activates the rendering frame clock loop to poll and repaint new data points. |
| `void` | [`stop`](#stop)  | Suspends frame clock ticks to freeze screen evaluation updates. |
| `void` | [`setZoomMode`](#setzoommode) `inline` |  |
| `ZoomMode` | [`zoomMode`](#zoommode) `const` `inline` |  |
| `void` | [`setGrid`](#setgrid) `inline` |  |
| `GridStyle` | [`getGrid`](#getgrid) `inline` |  |
| `void` | [`setMargins`](#setmargins) `inline` |  |
| `void` | [`setBackgroundColor`](#setbackgroundcolor-2) `inline` |  |
| `void` | [`setAxisColor`](#setaxiscolor) `inline` |  |
| `void` | [`setTextColor`](#settextcolor-2) `inline` |  |
| `void` | [`setTitle`](#settitle) `inline` |  |
| `void` | [`setXLabel`](#setxlabel) `inline` |  |
| `void` | [`setYLabel`](#setylabel) `inline` |  |
| `void` | [`addCursor`](#addcursor-2) `inline` | Adds an analytical marker tracking point onto the plot workspace. |
| `void` | [`enableRangeCursor`](#enablerangecursor) `inline` |  |
| `RtpRangeCursor *` | [`getRangeCursor`](#getrangecursor) `inline` |  |
| `void` | [`setLegendVisible`](#setlegendvisible) `inline` |  |
| `void` | [`setAutoZoom`](#setautozoom) `inline` |  |

---

{#realtimeplot-2}

### RealtimePlot

`explicit`

```cpp
explicit RealtimePlot(QWidget * parent = nullptr)
```

Defined in include/RealtimePlot.h:102

Constructor for the high-performance OpenGL plot widget.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `parent` | `QWidget *` | Pointer to parent Qt widget container. |

---

{#realtimeplot-3}

### ~RealtimePlot

`override`

```cpp
~RealtimePlot() override
```

Defined in include/RealtimePlot.h:107

Destructor. Safely deallocates GL shaders, objects, and timers.

---

{#addseries}

### addSeries

```cpp
std::shared_ptr< PlotSeries > addSeries(const QString & name, QColor color = Qt::cyan, size_t maxPoints = 10000)
```

Defined in include/RealtimePlot.h:118

Dynamically creates and tracks a new plot data channel.

#### Returns
Shared pointer reference to the created [PlotSeries](PlotSeries.md#plotseries) instance.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `const QString &` | Unique label identifier for the series. |
| `color` | `QColor` | Default line render color profile (Default: Qt::cyan). |
| `maxPoints` | `size_t` | Upper boundary capacity threshold for internal circular buffer. |

---

{#addseries-1}

### addSeries

```cpp
void addSeries(std::shared_ptr< PlotSeries > serie)
```

Defined in include/RealtimePlot.h:126

Adds a pre-existing externally built series into the rendering manager.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `serie` | `std::shared_ptr< [PlotSeries](PlotSeries.md#plotseries) >` | Shared pointer reference to the managed plot series. |

---

{#removeseries}

### removeSeries

```cpp
void removeSeries(const std::shared_ptr< PlotSeries > & series)
```

Defined in include/RealtimePlot.h:132

Deletes a targeted channel from the workspace.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `series` | `const std::shared_ptr< [PlotSeries](PlotSeries.md#plotseries) > &` | Shared pointer reference to the target channel. |

---

{#clearseries}

### clearSeries

```cpp
void clearSeries()
```

Defined in include/RealtimePlot.h:137

Completely drops all tracking datasets from the widget memory space.

---

{#addcursor}

### addCursor

```cpp
void addCursor()
```

Defined in include/RealtimePlot.h:144

Add cursor in the the center of the current view.

---

{#addcursor-1}

### addCursor

```cpp
void addCursor(double xPos)
```

Defined in include/RealtimePlot.h:150

Add cursor in position.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `xPos` | `double` | Position in data coordinates to place the cursor |

---

{#setrangecursorenable}

### setRangeCursorEnable

`inline`

```cpp
inline void setRangeCursorEnable(bool enable)
```

Defined in include/RealtimePlot.h:152

---

{#setrangecursorpos}

### setRangeCursorPos

`inline`

```cpp
inline void setRangeCursorPos(double start, double end)
```

Defined in include/RealtimePlot.h:153

---

{#autofit}

### autoFit

```cpp
void autoFit()
```

Defined in include/RealtimePlot.h:164

Computes data boundaries and updates viewport range parameters to fit content.

---

{#setviewrange}

### setViewRange

```cpp
void setViewRange(double xMin, double xMax, double yMin, double yMax)
```

Defined in include/RealtimePlot.h:173

Explicitly overrides limits of the observable data window.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `xMin` | `double` | Lowest bounding value along the X scale. |
| `xMax` | `double` | Highest bounding value along the X scale. |
| `yMin` | `double` | Lowest bounding value along the Y scale. |
| `yMax` | `double` | Highest bounding value along the Y scale. |

---

{#viewrange}

### viewRange

`const`

```cpp
QRectF viewRange() const
```

Defined in include/RealtimePlot.h:179

Retrieves a floating-point bounding rectangle representing current view extents.

#### Returns
QRectF containment envelope where width=X scope and height=Y scope.

---

{#setrefreshrate}

### setRefreshRate

```cpp
void setRefreshRate(int fps)
```

Defined in include/RealtimePlot.h:187

Programs the target periodic timer update frequency.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `fps` | `int` | Frames Per Second target (e.g., 60). |

---

{#start}

### start

```cpp
void start()
```

Defined in include/RealtimePlot.h:192

Activates the rendering frame clock loop to poll and repaint new data points.

---

{#stop}

### stop

```cpp
void stop()
```

Defined in include/RealtimePlot.h:197

Suspends frame clock ticks to freeze screen evaluation updates.

---

{#setzoommode}

### setZoomMode

`inline`

```cpp
inline void setZoomMode(ZoomMode mode)
```

Defined in include/RealtimePlot.h:200

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `mode` | `[ZoomMode](#zoommode-1)` | Desired structural constraints for handling zooming gestures. |

---

{#zoommode}

### zoomMode

`const` `inline`

```cpp
inline ZoomMode zoomMode() const
```

Defined in include/RealtimePlot.h:202

#### Returns
Current behavioral zoom constraint mode active on the canvas.

---

{#setgrid}

### setGrid

`inline`

```cpp
inline void setGrid(const GridStyle & g)
```

Defined in include/RealtimePlot.h:205

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `g` | `const [GridStyle](RealtimePlot-GridStyle.md#gridstyle) &` | Custom style properties structure payload to write into background grid. |

---

{#getgrid}

### getGrid

`inline`

```cpp
inline GridStyle getGrid()
```

Defined in include/RealtimePlot.h:207

#### Returns
Current active style metadata copy assigned to the grid lines.

---

{#setmargins}

### setMargins

`inline`

```cpp
inline void setMargins(const MarginPx & m)
```

Defined in include/RealtimePlot.h:209

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `m` | `const [MarginPx](RealtimePlot-MarginPx.md#marginpx) &` | Custom pixel padding configuration metrics array to isolate axes. |

---

{#setbackgroundcolor-2}

### setBackgroundColor

`inline`

```cpp
inline void setBackgroundColor(QColor c)
```

Defined in include/RealtimePlot.h:212

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `c` | `QColor` | Solid background fill color applied during frame clearing pass. |

---

{#setaxiscolor}

### setAxisColor

`inline`

```cpp
inline void setAxisColor(QColor c)
```

Defined in include/RealtimePlot.h:214

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `c` | `QColor` | Color indicator mapped onto primary axis border rule boundaries. |

---

{#settextcolor-2}

### setTextColor

`inline`

```cpp
inline void setTextColor(QColor c)
```

Defined in include/RealtimePlot.h:216

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `c` | `QColor` | Master color context parameter applied onto drawing tick numerals. |

---

{#settitle}

### setTitle

`inline`

```cpp
inline void setTitle(const QString & t)
```

Defined in include/RealtimePlot.h:219

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `t` | `const QString &` | Descriptive text string printed globally at the widget crown. |

---

{#setxlabel}

### setXLabel

`inline`

```cpp
inline void setXLabel(const QString & l)
```

Defined in include/RealtimePlot.h:221

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `l` | `const QString &` | Unit/description identifier string centered under horizontal axis. |

---

{#setylabel}

### setYLabel

`inline`

```cpp
inline void setYLabel(const QString & l)
```

Defined in include/RealtimePlot.h:223

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `l` | `const QString &` | Unit/description identifier string mapped near vertical layout line. |

---

{#addcursor-2}

### addCursor

`inline`

```cpp
inline void addCursor(float xValue, QColor color = Qt::transparent, RtpCursor::MarkerStyle style = RtpCursor::MarkerStyle::MARKER_SIMPLE)
```

Defined in include/RealtimePlot.h:231

Adds an analytical marker tracking point onto the plot workspace.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `xValue` | `float` | Absolute coordinate localization target point value on X scale. |
| `color` | `QColor` | Palette color tag assigned to distinguish marker line (Default: transparent). |
| `style` | `[RtpCursor::MarkerStyle](RtpCursor.md#markerstyle)` | Shape layout decoration applied onto cursor anchor target points. |

---

{#enablerangecursor}

### enableRangeCursor

`inline`

```cpp
inline void enableRangeCursor(bool enable)
```

Defined in include/RealtimePlot.h:244

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `enable` | `bool` | Toggles state of the specialized range cursor subcomponent. |

---

{#getrangecursor}

### getRangeCursor

`inline`

```cpp
inline RtpRangeCursor * getRangeCursor()
```

Defined in include/RealtimePlot.h:246

#### Returns
Pointer to internal range validation tracking subcomponent structure.

---

{#setlegendvisible}

### setLegendVisible

`inline`

```cpp
inline void setLegendVisible(bool v)
```

Defined in include/RealtimePlot.h:249

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `v` | `bool` | Toggles visibility of the interactive serial descriptor panel. |

---

{#setautozoom}

### setAutoZoom

`inline`

```cpp
inline void setAutoZoom(bool a)
```

Defined in include/RealtimePlot.h:252

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `a` | `bool` | Sets whether auto-zoom evaluation passes execute automatically. |

## Protected Methods

| Return | Name | Description |
|--------|------|-------------|
| `void` | [`initializeGL`](#initializegl) `override` |  |
| `void` | [`resizeGL`](#resizegl) `override` |  |
| `void` | [`paintGL`](#paintgl) `override` |  |
| `void` | [`wheelEvent`](#wheelevent) `override` |  |
| `void` | [`mousePressEvent`](#mousepressevent-1) `override` |  |
| `void` | [`mouseMoveEvent`](#mousemoveevent-1) `override` |  |
| `void` | [`mouseReleaseEvent`](#mousereleaseevent-1) `override` |  |
| `void` | [`mouseDoubleClickEvent`](#mousedoubleclickevent) `override` |  |
| `void` | [`keyPressEvent`](#keypressevent) `override` |  |

---

{#initializegl}

### initializeGL

`override`

```cpp
void initializeGL() override
```

Defined in include/RealtimePlot.h:266

---

{#resizegl}

### resizeGL

`override`

```cpp
void resizeGL(int w, int h) override
```

Defined in include/RealtimePlot.h:267

---

{#paintgl}

### paintGL

`override`

```cpp
void paintGL() override
```

Defined in include/RealtimePlot.h:268

---

{#wheelevent}

### wheelEvent

`override`

```cpp
void wheelEvent(QWheelEvent * event) override
```

Defined in include/RealtimePlot.h:271

---

{#mousepressevent-1}

### mousePressEvent

`override`

```cpp
void mousePressEvent(QMouseEvent * event) override
```

Defined in include/RealtimePlot.h:272

---

{#mousemoveevent-1}

### mouseMoveEvent

`override`

```cpp
void mouseMoveEvent(QMouseEvent * event) override
```

Defined in include/RealtimePlot.h:273

---

{#mousereleaseevent-1}

### mouseReleaseEvent

`override`

```cpp
void mouseReleaseEvent(QMouseEvent * event) override
```

Defined in include/RealtimePlot.h:274

---

{#mousedoubleclickevent}

### mouseDoubleClickEvent

`override`

```cpp
void mouseDoubleClickEvent(QMouseEvent * event) override
```

Defined in include/RealtimePlot.h:275

---

{#keypressevent}

### keyPressEvent

`override`

```cpp
void keyPressEvent(QKeyEvent * event) override
```

Defined in include/RealtimePlot.h:276

## Signals

| Return | Name | Description |
|--------|------|-------------|
| `void` | [`viewChanged`](#viewchanged)  | Signal fired whenever visible data window view boundaries are adjusted. |

---

{#viewchanged}

### viewChanged

```cpp
void viewChanged(double xMin, double xMax, double yMin, double yMax)
```

Defined in include/RealtimePlot.h:262

Signal fired whenever visible data window view boundaries are adjusted.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `xMin` | `double` | New absolute low limit parameter on X domain. |
| `xMax` | `double` | New absolute high limit parameter on X domain. |
| `yMin` | `double` | New absolute low limit parameter on Y domain. |
| `yMax` | `double` | New absolute high limit parameter on Y domain. |

## Public Types

| Name | Description |
|------|-------------|
| [`ZoomMode`](#zoommode-1)  | Constraints applied to zoom transformations. |
| [`CursorType`](#cursortype)  | Identifiers for interactive cursors to manage focus and selection. |

---

{#zoommode-1}

### ZoomMode

```cpp
enum ZoomMode
```

Defined in include/RealtimePlot.h:60

Constraints applied to zoom transformations.

| Value | Description |
|-------|-------------|
| `XY` | Scale both X and Y axes simultaneously. |
| `XOnly` | Lock Y axis and scale X axis exclusively. |
| `YOnly` | Lock X axis and scale Y axis exclusively. |

---

{#cursortype}

### CursorType

```cpp
enum CursorType
```

Defined in include/RealtimePlot.h:88

Identifiers for interactive cursors to manage focus and selection.

| Value | Description |
|-------|-------------|
| `None` | No cursor is currently selected or hovered. |
| `X1` | Primary vertical cursor (X axis split). |
| `X2` | Secondary vertical cursor (X axis split). |
| `Y1` | Primary horizontal cursor (Y axis split). |
| `Y2` | Secondary horizontal cursor (Y axis split). |

## Private Attributes

| Return | Name | Description |
|--------|------|-------------|
| `std::vector< std::shared_ptr< PlotSeries > >` | [`m_series`](#m_series)  | Master data collection vector of tracked display streams. |
| `double` | [`m_xMin`](#m_xmin-1)  |  |
| `double` | [`m_xMax`](#m_xmax-1)  | Horizontal observation limit scope metrics. |
| `double` | [`m_yMin`](#m_ymin-1)  |  |
| `double` | [`m_yMax`](#m_ymax-1)  | Vertical observation limit scope metrics. |
| `bool` | [`m_viewInitialized`](#m_viewinitialized)  | Safety control flag checking viewport validity. |
| `QColor` | [`m_bgColor`](#m_bgcolor-2)  | Viewport baseline core wash clearing background fill color. |
| `QColor` | [`m_axisColor`](#m_axiscolor)  | Primary reference edge line boundary color mapping indicator. |
| `QColor` | [`m_textColor`](#m_textcolor)  | Numeric label and heading string font canvas printing layout color. |
| `GridStyle` | [`m_grid`](#m_grid)  | Grid operational style definition map metadata payload. |
| `MarginPx` | [`m_margin`](#m_margin)  | Functional margin offset allocation context matrix parameters. |
| `QFont` | [`m_tickFont`](#m_tickfont)  | Font profile styling context configuration applied on axis numerals. |
| `QFont` | [`m_labelFont`](#m_labelfont)  | Font profile styling context configuration applied on main titles. |
| `QString` | [`m_title`](#m_title)  |  |
| `QString` | [`m_xLabel`](#m_xlabel)  |  |
| `QString` | [`m_yLabel`](#m_ylabel)  | Axis and plot header descriptive text label string records. |
| `bool` | [`m_legendVisible`](#m_legendvisible)  | Visibility override switch state assigned to control panel overlay. |
| `ZoomMode` | [`m_zoomMode`](#m_zoommode)  | Positional scaling locking parameter selection profile. |
| `bool` | [`m_zoomAuto`](#m_zoomauto)  | Automation tracking configuration state override directive. |
| `bool` | [`m_panning`](#m_panning)  | Intermittent active click drag translation tracking state indicator. |
| `bool` | [`m_selecting`](#m_selecting)  | Intermittent active click drag window box boundary select zoom indicator. |
| `QPoint` | [`m_lastMousePos`](#m_lastmousepos)  | Intermediate tracking position coordinate of historical mouse move cursor event. |
| `QPoint` | [`m_selStart`](#m_selstart)  |  |
| `QPoint` | [`m_selEnd`](#m_selend)  | Screen space box boundaries tracking indices for selection boxes. |
| `QTimer *` | [`m_timer`](#m_timer)  | Internal refresh loop triggering clock interval execution manager. |
| `QOpenGLShaderProgram *` | [`m_shader`](#m_shader)  | Compiled OpenGL program binding primitive geometry layout logic. |
| `bool` | [`m_shaderReady`](#m_shaderready)  | Pipeline validation control flag ensuring compilation stability. |
| `std::vector< RtpCursor >` | [`m_cursors`](#m_cursors)  | Extended sequence payload containing dynamic user cursor additions. |
| `RtpRangeCursor` | [`m_cursorRange`](#m_cursorrange)  | Specialized range tracking cursor evaluation context module component. |
| `CursorType` | [`m_activeCursor`](#m_activecursor)  | < Active cursor selection tracking identifier context mode. |
| `RtpCursor *` | [`m_activeCursorRef`](#m_activecursorref)  | Direct memory reference address to dragged object context instance. |
| `const int` | [`m_clickTolerancePixels`](#m_clicktolerancepixels)  | Click bounding halo tolerance scope diameter context scale. |
| `RtpLegend` | [`m_legend`](#m_legend)  | Secondary render presentation layer mapping data streams to labels. |

---

{#m_series}

### m_series

```cpp
std::vector< std::shared_ptr< PlotSeries > > m_series
```

Defined in include/RealtimePlot.h:319

Master data collection vector of tracked display streams.

---

{#m_xmin-1}

### m_xMin

```cpp
double m_xMin = 0.0
```

Defined in include/RealtimePlot.h:322

---

{#m_xmax-1}

### m_xMax

```cpp
double m_xMax = 1.0
```

Defined in include/RealtimePlot.h:323

Horizontal observation limit scope metrics.

---

{#m_ymin-1}

### m_yMin

```cpp
double m_yMin = 0.0
```

Defined in include/RealtimePlot.h:324

---

{#m_ymax-1}

### m_yMax

```cpp
double m_yMax = 1.0
```

Defined in include/RealtimePlot.h:325

Vertical observation limit scope metrics.

---

{#m_viewinitialized}

### m_viewInitialized

```cpp
bool m_viewInitialized =
        false
```

Defined in include/RealtimePlot.h:326

Safety control flag checking viewport validity.

---

{#m_bgcolor-2}

### m_bgColor

```cpp
QColor m_bgColor {
        18, 18,
        22}
```

Defined in include/RealtimePlot.h:330

Viewport baseline core wash clearing background fill color.

---

{#m_axiscolor}

### m_axisColor

```cpp
QColor m_axisColor {
        120, 120,
        130}
```

Defined in include/RealtimePlot.h:333

Primary reference edge line boundary color mapping indicator.

---

{#m_textcolor}

### m_textColor

```cpp
QColor m_textColor {
        200, 200,
        210}
```

Defined in include/RealtimePlot.h:336

Numeric label and heading string font canvas printing layout color.

---

{#m_grid}

### m_grid

```cpp
GridStyle m_grid
```

Defined in include/RealtimePlot.h:340

Grid operational style definition map metadata payload.

---

{#m_margin}

### m_margin

```cpp
MarginPx m_margin
```

Defined in include/RealtimePlot.h:342

Functional margin offset allocation context matrix parameters.

---

{#m_tickfont}

### m_tickFont

```cpp
QFont m_tickFont
```

Defined in include/RealtimePlot.h:344

Font profile styling context configuration applied on axis numerals.

---

{#m_labelfont}

### m_labelFont

```cpp
QFont m_labelFont
```

Defined in include/RealtimePlot.h:346

Font profile styling context configuration applied on main titles.

---

{#m_title}

### m_title

```cpp
QString m_title
```

Defined in include/RealtimePlot.h:347

---

{#m_xlabel}

### m_xLabel

```cpp
QString m_xLabel
```

Defined in include/RealtimePlot.h:347

---

{#m_ylabel}

### m_yLabel

```cpp
QString m_yLabel
```

Defined in include/RealtimePlot.h:348

Axis and plot header descriptive text label string records.

---

{#m_legendvisible}

### m_legendVisible

```cpp
bool m_legendVisible =
        true
```

Defined in include/RealtimePlot.h:349

Visibility override switch state assigned to control panel overlay.

---

{#m_zoommode}

### m_zoomMode

```cpp
ZoomMode m_zoomMode = ZoomMode::
        
```

Defined in include/RealtimePlot.h:353

Positional scaling locking parameter selection profile.

---

{#m_zoomauto}

### m_zoomAuto

```cpp
bool m_zoomAuto =
        true
```

Defined in include/RealtimePlot.h:355

Automation tracking configuration state override directive.

---

{#m_panning}

### m_panning

```cpp
bool m_panning =
        false
```

Defined in include/RealtimePlot.h:359

Intermittent active click drag translation tracking state indicator.

---

{#m_selecting}

### m_selecting

```cpp
bool m_selecting =
        false
```

Defined in include/RealtimePlot.h:361

Intermittent active click drag window box boundary select zoom indicator.

---

{#m_lastmousepos}

### m_lastMousePos

```cpp
QPoint m_lastMousePos
```

Defined in include/RealtimePlot.h:364

Intermediate tracking position coordinate of historical mouse move cursor event.

---

{#m_selstart}

### m_selStart

```cpp
QPoint m_selStart
```

Defined in include/RealtimePlot.h:365

---

{#m_selend}

### m_selEnd

```cpp
QPoint m_selEnd
```

Defined in include/RealtimePlot.h:366

Screen space box boundaries tracking indices for selection boxes.

---

{#m_timer}

### m_timer

```cpp
QTimer * m_timer =
        nullptr
```

Defined in include/RealtimePlot.h:369

Internal refresh loop triggering clock interval execution manager.

---

{#m_shader}

### m_shader

```cpp
QOpenGLShaderProgram * m_shader =
        nullptr
```

Defined in include/RealtimePlot.h:371

Compiled OpenGL program binding primitive geometry layout logic.

---

{#m_shaderready}

### m_shaderReady

```cpp
bool m_shaderReady =
        false
```

Defined in include/RealtimePlot.h:373

Pipeline validation control flag ensuring compilation stability.

---

{#m_cursors}

### m_cursors

```cpp
std::vector< RtpCursor > m_cursors
```

Defined in include/RealtimePlot.h:378

Extended sequence payload containing dynamic user cursor additions.

---

{#m_cursorrange}

### m_cursorRange

```cpp
RtpRangeCursor m_cursorRange
```

Defined in include/RealtimePlot.h:380

Specialized range tracking cursor evaluation context module component.

---

{#m_activecursor}

### m_activeCursor

```cpp
CursorType m_activeCursor = 
```

Defined in include/RealtimePlot.h:384

< Active cursor selection tracking identifier context mode.

---

{#m_activecursorref}

### m_activeCursorRef

```cpp
RtpCursor * m_activeCursorRef =
        nullptr
```

Defined in include/RealtimePlot.h:385

Direct memory reference address to dragged object context instance.

---

{#m_clicktolerancepixels}

### m_clickTolerancePixels

```cpp
const int m_clickTolerancePixels =
        7
```

Defined in include/RealtimePlot.h:387

Click bounding halo tolerance scope diameter context scale.

---

{#m_legend}

### m_legend

```cpp
RtpLegend m_legend
```

Defined in include/RealtimePlot.h:391

Secondary render presentation layer mapping data streams to labels.

## Private Methods

| Return | Name | Description |
|--------|------|-------------|
| `QPointF` | [`pixelToData`](#pixeltodata) `const` | Transforms local widget integer window coordinates into absolute data unit values. |
| `QPoint` | [`dataToPixel`](#datatopixel) `const` | Transforms absolute database values into target local integer frame layout coordinates. |
| `QRect` | [`plotArea`](#plotarea) `const` | Extracts bounding geometry dimensions isolating the clean graph center core. |
| `void` | [`drawGrid`](#drawgrid)  |  |
| `void` | [`drawSeries`](#drawseries)  |  |
| `void` | [`drawAxes`](#drawaxes)  |  |
| `void` | [`drawLineStrip`](#drawlinestrip)  |  |
| `void` | [`drawLines`](#drawlines)  |  |
| `void` | [`applyZoom`](#applyzoom)  |  |
| `void` | [`emitViewChanged`](#emitviewchanged)  |  |

---

{#pixeltodata}

### pixelToData

`const`

```cpp
QPointF pixelToData(QPoint px) const
```

Defined in include/RealtimePlot.h:286

Transforms local widget integer window coordinates into absolute data unit values.

#### Returns
Real calculated QPointF coordinate structure floating points.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `px` | `QPoint` | Integer position relative to the local widget frame coordinate map. |

---

{#datatopixel}

### dataToPixel

`const`

```cpp
QPoint dataToPixel(double x, double y) const
```

Defined in include/RealtimePlot.h:294

Transforms absolute database values into target local integer frame layout coordinates.

#### Returns
QPoint mapping position in widget screen coordinate pixel space.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `x` | `double` | Absolute internal horizontal coordinate tracker trace. |
| `y` | `double` | Absolute internal vertical coordinate tracker trace. |

---

{#plotarea}

### plotArea

`const`

```cpp
QRect plotArea() const
```

Defined in include/RealtimePlot.h:300

Extracts bounding geometry dimensions isolating the clean graph center core.

#### Returns
Screen coordinate integer geometry box wrapper.

---

{#drawgrid}

### drawGrid

```cpp
void drawGrid(const QRect & area, const QVector< PlotAxis::Tick > & xTicks, const QVector< PlotAxis::Tick > & yTicks)
```

Defined in include/RealtimePlot.h:303

---

{#drawseries}

### drawSeries

```cpp
void drawSeries(const QRect & area)
```

Defined in include/RealtimePlot.h:305

---

{#drawaxes}

### drawAxes

```cpp
void drawAxes(const QRect & area, const QVector< PlotAxis::Tick > & xTicks, const QVector< PlotAxis::Tick > & yTicks)
```

Defined in include/RealtimePlot.h:306

---

{#drawlinestrip}

### drawLineStrip

```cpp
void drawLineStrip(const std::vector< float > & verts, QColor color, float width)
```

Defined in include/RealtimePlot.h:310

---

{#drawlines}

### drawLines

```cpp
void drawLines(const std::vector< float > & verts, QColor color, float width)
```

Defined in include/RealtimePlot.h:312

---

{#applyzoom}

### applyZoom

```cpp
void applyZoom(double factor, QPoint anchor)
```

Defined in include/RealtimePlot.h:314

---

{#emitviewchanged}

### emitViewChanged

```cpp
void emitViewChanged()
```

Defined in include/RealtimePlot.h:315

## Private Static Attributes

| Return | Name | Description |
|--------|------|-------------|
| `const std::vector< QColor >` | [`colorList`](#colorlist) `static` | Palette definition array used to automate color assignment layers. |

---

{#colorlist}

### colorList

`static`

```cpp
const std::vector< QColor > colorList = {Qt::red, Qt::yellow,
                                                     Qt::green}
```

Defined in include/RealtimePlot.h:394

Palette definition array used to automate color assignment layers.


{#rtpcursor}

# RtpCursor

```cpp
#include <RtpCursor.h>
```

```cpp
class RtpCursor
```

Defined in include/components/RtpCursor.h:19

Manages a single interactive measurement cursor overlay.

This class handles the rendering, positioning, and mouse interaction (hover, selection, dragging) for an individual vertical tracking cursor. It can render context labels showing raw data values intersecting at its current X coordinate.

## List of all members

| Name | Kind | Owner |
|------|------|-------|
| [`RtpCursor`](#rtpcursor-1) | `function` | Declared here |
| [`setSeriesSource`](#setseriessource) | `function` | Declared here |
| [`draw`](#draw) | `function` | Declared here |
| [`setFont`](#setfont) | `function` | Declared here |
| [`setTextColor`](#settextcolor) | `function` | Declared here |
| [`setBackgroundColor`](#setbackgroundcolor) | `function` | Declared here |
| [`setMargin`](#setmargin) | `function` | Declared here |
| [`mousePressEvent`](#mousepressevent) | `function` | Declared here |
| [`mouseMoveEvent`](#mousemoveevent) | `function` | Declared here |
| [`mouseReleaseEvent`](#mousereleaseevent) | `function` | Declared here |
| [`setPos`](#setpos) | `function` | Declared here |
| [`getPos`](#getpos) | `function` | Declared here |
| [`enabled`](#enabled) | `function` | Declared here |
| [`enable`](#enable) | `function` | Declared here |
| [`isVisible`](#isvisible) | `function` | Declared here |
| [`isDragging`](#isdragging) | `function` | Declared here |
| [`contains`](#contains) | `function` | Declared here |
| [`MarkerStyle`](#markerstyle) | `enum` | Declared here |
| [`m_isEnabled`](#m_isenabled) | `variable` | Declared here |
| [`m_xPos`](#m_xpos) | `variable` | Declared here |
| [`m_isDragging`](#m_isdragging) | `variable` | Declared here |
| [`m_dragOffset`](#m_dragoffset) | `variable` | Declared here |
| [`m_cursorIndex`](#m_cursorindex) | `variable` | Declared here |
| [`m_isVisible`](#m_isvisible) | `variable` | Declared here |
| [`m_font`](#m_font) | `variable` | Declared here |
| [`m_color`](#m_color) | `variable` | Declared here |
| [`m_bgColor`](#m_bgcolor) | `variable` | Declared here |
| [`m_labelRect`](#m_labelrect) | `variable` | Declared here |
| [`m_lineHitbox`](#m_linehitbox) | `variable` | Declared here |
| [`m_style`](#m_style) | `variable` | Declared here |
| [`m_seriesRef`](#m_seriesref) | `variable` | Declared here |
| [`drawFullMarker`](#drawfullmarker) | `function` | Declared here |
| [`drawSimpleMarker`](#drawsimplemarker) | `function` | Declared here |

## Public Methods

| Return | Name | Description |
|--------|------|-------------|
|  | [`RtpCursor`](#rtpcursor-1)  | Constructs a single tracker cursor instance. |
| `void` | [`setSeriesSource`](#setseriessource) `inline` | Pairs the cursor context directly to a tracked sequence dataset list. |
| `void` | [`draw`](#draw)  | Renders the cursor lines, intersection grips, and value tags onto the active view canvas. |
| `void` | [`setFont`](#setfont)  | Updates typography configuration layouts applied onto numeric labels. |
| `void` | [`setTextColor`](#settextcolor)  | Sets primary fill color parameters assigned to label text strings. |
| `void` | [`setBackgroundColor`](#setbackgroundcolor)  | Sets solid background shading profile colors masking text box plates. |
| `void` | [`setMargin`](#setmargin)  | Prescribes internal edge padding boundary separation limits in screen pixels. |
| `bool` | [`mousePressEvent`](#mousepressevent)  | Probes mouse down inputs to evaluate focus hits and lock drag lifecycle states. |
| `bool` | [`mouseMoveEvent`](#mousemoveevent)  | Translates ongoing movement input tracking offsets to recompute absolute placements. |
| `void` | [`mouseReleaseEvent`](#mousereleaseevent)  | Breaks active pointer capture tracking loops, releasing global control lock status flags. |
| `void` | [`setPos`](#setpos) `inline` |  |
| `double` | [`getPos`](#getpos) `const` `inline` |  |
| `bool` | [`enabled`](#enabled) `const` `inline` |  |
| `void` | [`enable`](#enable) `inline` |  |
| `bool` | [`isVisible`](#isvisible) `inline` |  |
| `bool` | [`isDragging`](#isdragging) `const` `inline` |  |
| `bool` | [`contains`](#contains) `const` | Hit-test verification evaluating coordinate intersection metrics across interaction bounds. |

---

{#rtpcursor-1}

### RtpCursor

```cpp
RtpCursor(int index, QColor color, MarkerStyle style = MarkerStyle::MARKER_FULL)
```

Defined in include/components/RtpCursor.h:38

Constructs a single tracker cursor instance.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `index` | `int` | Numeric order sequence tag tracking this identifier. |
| `color` | `QColor` | Primary coloring profile configuration applied onto lines and icons. |
| `style` | `[MarkerStyle](#markerstyle)` | Presentation rendering geometry profile layout switch (Default: MARKER_FULL). |

---

{#setseriessource}

### setSeriesSource

`inline`

```cpp
inline void setSeriesSource(const std::vector< std::shared_ptr< PlotSeries > > * series)
```

Defined in include/components/RtpCursor.h:45

Pairs the cursor context directly to a tracked sequence dataset list.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `series` | `const std::vector< std::shared_ptr< [PlotSeries](PlotSeries.md#plotseries) > > *` | Constant pointer targeting the shared data channels tracking array vector. |

---

{#draw}

### draw

```cpp
void draw(QPainter & painter, const QRect & plotArea, int calculatedPixelX, const QString & mainLabel, bool alignLeft = true)
```

Defined in include/components/RtpCursor.h:58

Renders the cursor lines, intersection grips, and value tags onto the active view canvas.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `painter` | `QPainter &` | Execution handle targeting active drawing canvas context surfaces. |
| `plotArea` | `const QRect &` | Main viewport rectangle coordinate limits scaled in screen pixels. |
| `calculatedPixelX` | `int` | Screen space pixel column position tracking cursor location. |
| `mainLabel` | `const QString &` | Custom text string to show inside the information tooltip bubble. |
| `alignLeft` | `bool` | Toggles justification layout alignment direction preferences for text tags. |

---

{#setfont}

### setFont

```cpp
void setFont(const QFont & font)
```

Defined in include/components/RtpCursor.h:67

Updates typography configuration layouts applied onto numeric labels.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `font` | `const QFont &` | QFont structure containing desired text rendering attributes. |

---

{#settextcolor}

### setTextColor

```cpp
void setTextColor(const QColor & color)
```

Defined in include/components/RtpCursor.h:73

Sets primary fill color parameters assigned to label text strings.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `color` | `const QColor &` | QColor text drawing color. |

---

{#setbackgroundcolor}

### setBackgroundColor

```cpp
void setBackgroundColor(const QColor & bgColor)
```

Defined in include/components/RtpCursor.h:79

Sets solid background shading profile colors masking text box plates.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `bgColor` | `const QColor &` | QColor tooltips background layer wash color. |

---

{#setmargin}

### setMargin

```cpp
void setMargin(const int marginPixels)
```

Defined in include/components/RtpCursor.h:85

Prescribes internal edge padding boundary separation limits in screen pixels.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `marginPixels` | `const int` | Total pixel padding threshold space constraint value. |

---

{#mousepressevent}

### mousePressEvent

```cpp
bool mousePressEvent(QMouseEvent * event, int currentPixelX)
```

Defined in include/components/RtpCursor.h:95

Probes mouse down inputs to evaluate focus hits and lock drag lifecycle states.

#### Returns
True if selection focus hits interactive regions, triggering state capture lock.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `event` | `QMouseEvent *` | System mouse update data payload arguments forwarded by UI thread. |
| `currentPixelX` | `int` | Extant horizontal position of cursor on screen. |

---

{#mousemoveevent}

### mouseMoveEvent

```cpp
bool mouseMoveEvent(QMouseEvent * event, const QRect & plotArea, int & outNewPixelX)
```

Defined in include/components/RtpCursor.h:104

Translates ongoing movement input tracking offsets to recompute absolute placements.

#### Returns
True if a reposition sequence occurs requiring view updates.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `event` | `QMouseEvent *` | System mouse update data payload arguments forwarded by UI thread. |
| `plotArea` | `const QRect &` | Plot workspace pixel containment constraints structure box. |
| `outNewPixelX` | `int &` | Output column tracking parameter yielding modified layout location values. |

---

{#mousereleaseevent}

### mouseReleaseEvent

```cpp
void mouseReleaseEvent(QMouseEvent * event)
```

Defined in include/components/RtpCursor.h:111

Breaks active pointer capture tracking loops, releasing global control lock status flags.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `event` | `QMouseEvent *` | System mouse update data payload arguments forwarded by UI thread. |

---

{#setpos}

### setPos

`inline`

```cpp
inline void setPos(double pos)
```

Defined in include/components/RtpCursor.h:114

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pos` | `double` | Absolute internal data coordinate value location to map onto X axis. |

---

{#getpos}

### getPos

`const` `inline`

```cpp
inline double getPos() const
```

Defined in include/components/RtpCursor.h:116

#### Returns
Current absolute position mapped value tracked by cursor on X scale.

---

{#enabled}

### enabled

`const` `inline`

```cpp
inline bool enabled() const
```

Defined in include/components/RtpCursor.h:119

#### Returns
Computational processing loop permission status state verification flag.

---

{#enable}

### enable

`inline`

```cpp
inline void enable(bool e)
```

Defined in include/components/RtpCursor.h:121

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `e` | `bool` | Toggle programmatic parsing and drawing evaluation authorization passes. |

---

{#isvisible}

### isVisible

`inline`

```cpp
inline bool isVisible()
```

Defined in include/components/RtpCursor.h:124

#### Returns
True if the cursor is currently being drawn on screen.

---

{#isdragging}

### isDragging

`const` `inline`

```cpp
inline bool isDragging() const
```

Defined in include/components/RtpCursor.h:127

#### Returns
Mutual exclusion lock tracking flag testing active navigation click drag sequences.

---

{#contains}

### contains

`const`

```cpp
bool contains(const QPoint & pos) const
```

Defined in include/components/RtpCursor.h:134

Hit-test verification evaluating coordinate intersection metrics across interaction bounds.

#### Returns
True if pointer sample context falls within focus collision geometries.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pos` | `const QPoint &` | Intermittent screen pixel sample location targeted for layout mapping validation checks. |

## Public Types

| Name | Description |
|------|-------------|
| [`MarkerStyle`](#markerstyle)  | Visual rendering profiles for the cursor marker and its associated readout badge. |

---

{#markerstyle}

### MarkerStyle

```cpp
enum MarkerStyle
```

Defined in include/components/RtpCursor.h:26

Visual rendering profiles for the cursor marker and its associated readout badge.

| Value | Description |
|-------|-------------|
| `MARKER_FULL` | Draws a line stretching across the plot viewport accompanied by a descriptive label badge. |
| `MARKER_SIMPLE` | Draws an abbreviated minimalist handle indicator on the axis line. |

## Private Attributes

| Return | Name | Description |
|--------|------|-------------|
| `bool` | [`m_isEnabled`](#m_isenabled)  | Internal execution state controlling drawing passes. |
| `double` | [`m_xPos`](#m_xpos)  | Target coordinate placement metric tracking location across data domains. |
| `bool` | [`m_isDragging`](#m_isdragging)  | Core tracking flag state tracking translation click drag cycles. |
| `int` | [`m_dragOffset`](#m_dragoffset)  | Variable preserving delta pixel offsets across movement loops. |
| `int` | [`m_cursorIndex`](#m_cursorindex)  | Positional tracking queue metadata order sequence tag. |
| `bool` | [`m_isVisible`](#m_isvisible)  | Temporary runtime visibility evaluation storage state tracker. |
| `QFont` | [`m_font`](#m_font)  | Typography attribute profiles mapped onto label strings. |
| `QColor` | [`m_color`](#m_color)  | Base coloring applied across outline rules and vectors. |
| `QColor` | [`m_bgColor`](#m_bgcolor)  | Solid background plate color applied behind data popups. |
| `QRect` | [`m_labelRect`](#m_labelrect)  | Interactive geometry container boundaries enclosing value tooltip cards. |
| `QRect` | [`m_lineHitbox`](#m_linehitbox)  | Oversized virtual selection envelope padding thin lines to simplify target grab clicks. |
| `MarkerStyle` | [`m_style`](#m_style)  | Active geometric decoration formatting behavior profile. |
| `const std::vector< std::shared_ptr< PlotSeries > > *` | [`m_seriesRef`](#m_seriesref)  | Shared repository address collection reference mapping active data vectors. |

---

{#m_isenabled}

### m_isEnabled

```cpp
bool m_isEnabled =
        true
```

Defined in include/components/RtpCursor.h:137

Internal execution state controlling drawing passes.

---

{#m_xpos}

### m_xPos

```cpp
double m_xPos =
        0
```

Defined in include/components/RtpCursor.h:139

Target coordinate placement metric tracking location across data domains.

---

{#m_isdragging}

### m_isDragging

```cpp
bool m_isDragging =
        false
```

Defined in include/components/RtpCursor.h:142

Core tracking flag state tracking translation click drag cycles.

---

{#m_dragoffset}

### m_dragOffset

```cpp
int m_dragOffset =
        0
```

Defined in include/components/RtpCursor.h:144

Variable preserving delta pixel offsets across movement loops.

---

{#m_cursorindex}

### m_cursorIndex

```cpp
int m_cursorIndex
```

Defined in include/components/RtpCursor.h:147

Positional tracking queue metadata order sequence tag.

---

{#m_isvisible}

### m_isVisible

```cpp
bool m_isVisible =
        false
```

Defined in include/components/RtpCursor.h:148

Temporary runtime visibility evaluation storage state tracker.

---

{#m_font}

### m_font

```cpp
QFont m_font
```

Defined in include/components/RtpCursor.h:151

Typography attribute profiles mapped onto label strings.

---

{#m_color}

### m_color

```cpp
QColor m_color
```

Defined in include/components/RtpCursor.h:153

Base coloring applied across outline rules and vectors.

---

{#m_bgcolor}

### m_bgColor

```cpp
QColor m_bgColor
```

Defined in include/components/RtpCursor.h:155

Solid background plate color applied behind data popups.

---

{#m_labelrect}

### m_labelRect

```cpp
QRect m_labelRect
```

Defined in include/components/RtpCursor.h:158

Interactive geometry container boundaries enclosing value tooltip cards.

---

{#m_linehitbox}

### m_lineHitbox

```cpp
QRect m_lineHitbox
```

Defined in include/components/RtpCursor.h:160

Oversized virtual selection envelope padding thin lines to simplify target grab clicks.

---

{#m_style}

### m_style

```cpp
MarkerStyle m_style
```

Defined in include/components/RtpCursor.h:163

Active geometric decoration formatting behavior profile.

---

{#m_seriesref}

### m_seriesRef

```cpp
const std::vector< std::shared_ptr< PlotSeries > > * m_seriesRef = nullptr
```

Defined in include/components/RtpCursor.h:166

Shared repository address collection reference mapping active data vectors.

## Private Methods

| Return | Name | Description |
|--------|------|-------------|
| `void` | [`drawFullMarker`](#drawfullmarker)  | Geometry drawer executing viewport line layouts with floating readout badges. |
| `void` | [`drawSimpleMarker`](#drawsimplemarker)  | Geometry drawer rendering structural low-profile handle markers on axis frames. |

---

{#drawfullmarker}

### drawFullMarker

```cpp
void drawFullMarker(QPainter & painter, const QRect & plotArea, int calculatedPixelX, bool alignLeft)
```

Defined in include/components/RtpCursor.h:171

Geometry drawer executing viewport line layouts with floating readout badges.

---

{#drawsimplemarker}

### drawSimpleMarker

```cpp
void drawSimpleMarker(QPainter & painter, const QRect & plotArea, int calculatedPixelX, bool alignLeft)
```

Defined in include/components/RtpCursor.h:177

Geometry drawer rendering structural low-profile handle markers on axis frames.


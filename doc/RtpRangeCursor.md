

# RtpRangeCursor

```cpp
#include <RtpRangeCursor.h>
```

```cpp
class RtpRangeCursor
```

Defined in include/components/RtpRangeCursor.h:22

Manages a pair of dual bounds cursors to measure mathematical delta ranges.

This component coordinates a start boundary cursor and an end boundary cursor, calculating metrics, statistics, and delta values across the bounded sub-interval. It encapsulates mouse event handling for interactive tracking and movement.

## List of all members

| Name | Kind | Owner |
|------|------|-------|
| [`RtpRangeCursor`](#rtprangecursor-1) | `function` | Declared here |
| [`setSeriesSource`](#setseriessource-2) | `function` | Declared here |
| [`setTextColor`](#settextcolor-3) | `function` | Declared here |
| [`setBackgroundColor`](#setbackgroundcolor-3) | `function` | Declared here |
| [`setMargin`](#setmargin-2) | `function` | Declared here |
| [`draw`](#draw-2) | `function` | Declared here |
| [`mousePressEvent`](#mousepressevent-2) | `function` | Declared here |
| [`mouseMoveEvent`](#mousemoveevent-2) | `function` | Declared here |
| [`mouseReleaseEvent`](#mousereleaseevent-2) | `function` | Declared here |
| [`setPosStart`](#setposstart) | `function` | Declared here |
| [`getPosStart`](#getposstart) | `function` | Declared here |
| [`setPosEnd`](#setposend) | `function` | Declared here |
| [`getPosEnd`](#getposend) | `function` | Declared here |
| [`enabled`](#enabled-2) | `function` | Declared here |
| [`enable`](#enable-1) | `function` | Declared here |
| [`contains`](#contains-1) | `function` | Declared here |
| [`checkReorder`](#checkreorder) | `function` | Declared here |
| [`m_isEnabled`](#m_isenabled-1) | `variable` | Declared here |
| [`m_xPos`](#m_xpos-1) | `variable` | Declared here |
| [`m_isDragging`](#m_isdragging-1) | `variable` | Declared here |
| [`m_dragOffset`](#m_dragoffset-1) | `variable` | Declared here |
| [`m_cursorIndex`](#m_cursorindex-1) | `variable` | Declared here |
| [`m_color`](#m_color-2) | `variable` | Declared here |
| [`m_bgColor`](#m_bgcolor-3) | `variable` | Declared here |
| [`m_cStart`](#m_cstart) | `variable` | Declared here |
| [`m_cEnd`](#m_cend) | `variable` | Declared here |
| [`m_style`](#m_style-1) | `variable` | Declared here |
| [`m_seriesRef`](#m_seriesref-2) | `variable` | Declared here |

## Public Methods

| Return | Name | Description |
|--------|------|-------------|
|  | [`RtpRangeCursor`](#rtprangecursor-1)  | Constructs a dual range selection cursor controller. |
| `void` | [`setSeriesSource`](#setseriessource-2) `inline` | Maps the reference data tracking dataset channel lookup collection list. |
| `void` | [`setTextColor`](#settextcolor-3)  | Adjusts primary text foreground overlay paintbrush color options. |
| `void` | [`setBackgroundColor`](#setbackgroundcolor-3)  | Adjusts back-face readout boundary summary box mask fill colors. |
| `void` | [`setMargin`](#setmargin-2)  | Sets internal content margin spacing boundaries in screen pixel units. |
| `void` | [`draw`](#draw-2)  | Renders lines, boundaries, highlights, and tooltips on the painting canvas. |
| `RtpCursor *` | [`mousePressEvent`](#mousepressevent-2)  | Evaluates mouse down inputs to trigger active edge translation drag capture cycles. |
| `bool` | [`mouseMoveEvent`](#mousemoveevent-2)  | Processes ongoing pointing translation offsets to recompute absolute boundary marks. |
| `void` | [`mouseReleaseEvent`](#mousereleaseevent-2)  | Finalizes boundary adjustments, unlocking state control loops. |
| `void` | [`setPosStart`](#setposstart) `inline` |  |
| `double` | [`getPosStart`](#getposstart) `const` `inline` |  |
| `void` | [`setPosEnd`](#setposend) `inline` |  |
| `double` | [`getPosEnd`](#getposend) `const` `inline` |  |
| `bool` | [`enabled`](#enabled-2) `const` `inline` |  |
| `void` | [`enable`](#enable-1) `inline` |  |
| `bool` | [`contains`](#contains-1) `const` | Determines whether a pointer position lands inside the bounding footprint selection. |
| `bool` | [`checkReorder`](#checkreorder)  |  |

---

### RtpRangeCursor

```cpp
RtpRangeCursor(int index, QColor color, RtpCursor::MarkerStyle style = RtpCursor::MarkerStyle::MARKER_FULL)
```

Defined in include/components/RtpRangeCursor.h:31

Constructs a dual range selection cursor controller.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `index` | `int` | Identifier tag tracking this composite asset component. |
| `color` | `QColor` | Base rendering color layer mapped onto cursor boundary rules. |
| `style` | `[RtpCursor::MarkerStyle](RtpCursor.md#markerstyle)` | Geometric shape decoration setting applied onto layout bounds (Default: MARKER_FULL). |

---

### setSeriesSource

`inline`

```cpp
inline void setSeriesSource(const std::vector< std::shared_ptr< PlotSeries > > * series)
```

Defined in include/components/RtpRangeCursor.h:39

Maps the reference data tracking dataset channel lookup collection list.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `series` | `const std::vector< std::shared_ptr< [PlotSeries](PlotSeries.md#plotseries) > > *` | Constant pointer targeting the multi-channel vector stack array. |

---

### setTextColor

```cpp
void setTextColor(const QColor & color)
```

Defined in include/components/RtpRangeCursor.h:48

Adjusts primary text foreground overlay paintbrush color options.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `color` | `const QColor &` | QColor description applied on textual outputs. |

---

### setBackgroundColor

```cpp
void setBackgroundColor(const QColor & bgColor)
```

Defined in include/components/RtpRangeCursor.h:54

Adjusts back-face readout boundary summary box mask fill colors.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `bgColor` | `const QColor &` | QColor applied to wash metric summary backgrounds. |

---

### setMargin

```cpp
void setMargin(const int marginPixels)
```

Defined in include/components/RtpRangeCursor.h:60

Sets internal content margin spacing boundaries in screen pixel units.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `marginPixels` | `const int` | Absolute padding pixel threshold constraint value. |

---

### draw

```cpp
void draw(QPainter & painter, const QRect & plotArea, int calculatedPixelStart, int calculatedPixelEnd)
```

Defined in include/components/RtpRangeCursor.h:69

Renders lines, boundaries, highlights, and tooltips on the painting canvas.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `painter` | `QPainter &` | Target QPainter render canvas execution surface context layer. |
| `plotArea` | `const QRect &` | Main active data graphing rectangle geometry constraints in screen pixels. |
| `calculatedPixelStart` | `int` | Screen pixel column tracking localization of start bounds. |
| `calculatedPixelEnd` | `int` | Screen pixel column tracking localization of end bounds. |

---

### mousePressEvent

```cpp
RtpCursor * mousePressEvent(QMouseEvent * event, int currentPixelStart, int currentPixelEnd)
```

Defined in include/components/RtpRangeCursor.h:79

Evaluates mouse down inputs to trigger active edge translation drag capture cycles.

#### Returns
Underlying target cursor subcomponent reference capturing drag state, or nullptr if click missed.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `event` | `QMouseEvent *` | Native mouse press parameters forwarded by layout framework. |
| `currentPixelStart` | `int` | Extant horizontal pixel location of start border mark. |
| `currentPixelEnd` | `int` | Extant horizontal pixel location of end border mark. |

---

### mouseMoveEvent

```cpp
bool mouseMoveEvent(QMouseEvent * event, const QRect & plotArea, int & outNewPixelX)
```

Defined in include/components/RtpRangeCursor.h:89

Processes ongoing pointing translation offsets to recompute absolute boundary marks.

#### Returns
True if a modification pass took place requiring view update flags.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `event` | `QMouseEvent *` | Native mouse update path parameters forwarded by layout framework. |
| `plotArea` | `const QRect &` | Reference canvas rectangle geometry constraints. |
| `outNewPixelX` | `int &` | Output tracking parameter yielding updated location details. |

---

### mouseReleaseEvent

```cpp
void mouseReleaseEvent(QMouseEvent * event)
```

Defined in include/components/RtpRangeCursor.h:96

Finalizes boundary adjustments, unlocking state control loops.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `event` | `QMouseEvent *` | Native mouse release parameters forwarded by layout framework. |

---

### setPosStart

`inline`

```cpp
inline void setPosStart(double pos)
```

Defined in include/components/RtpRangeCursor.h:99

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pos` | `double` | Absolute data space value position coordinate assigned to start edge. |

---

### getPosStart

`const` `inline`

```cpp
inline double getPosStart() const
```

Defined in include/components/RtpRangeCursor.h:101

#### Returns
Current absolute data space value placement of start boundary edge.

---

### setPosEnd

`inline`

```cpp
inline void setPosEnd(double pos)
```

Defined in include/components/RtpRangeCursor.h:104

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pos` | `double` | Absolute data space value position coordinate assigned to end edge. |

---

### getPosEnd

`const` `inline`

```cpp
inline double getPosEnd() const
```

Defined in include/components/RtpRangeCursor.h:106

#### Returns
Current absolute data space value placement of end boundary edge.

---

### enabled

`const` `inline`

```cpp
inline bool enabled() const
```

Defined in include/components/RtpRangeCursor.h:109

#### Returns
Visibility and computation evaluation permission validation status state flag.

---

### enable

`inline`

```cpp
inline void enable(bool e)
```

Defined in include/components/RtpRangeCursor.h:111

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `e` | `bool` | Toggle visibility and algorithmic evaluation processing permissions. |

---

### contains

`const`

```cpp
bool contains(const QPoint & pos) const
```

Defined in include/components/RtpRangeCursor.h:118

Determines whether a pointer position lands inside the bounding footprint selection.

#### Returns
True if geometry intersections confirm pointing focus hits the element.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `pos` | `const QPoint &` | Intermittent screen space coordinate target evaluation candidate. |

---

### checkReorder

```cpp
bool checkReorder()
```

Defined in include/components/RtpRangeCursor.h:120

## Private Attributes

| Return | Name | Description |
|--------|------|-------------|
| `bool` | [`m_isEnabled`](#m_isenabled-1)  | Structural execution state controlling drawing passes. |
| `double` | [`m_xPos`](#m_xpos-1)  | Master anchor localization reference metric point tracker. |
| `bool` | [`m_isDragging`](#m_isdragging-1)  | Mutual exclusion flag lock tracking interactive relocation inputs. |
| `int` | [`m_dragOffset`](#m_dragoffset-1)  | Offset value cache preserving layout adjustment tracking pixels. |
| `int` | [`m_cursorIndex`](#m_cursorindex-1)  | Identifying component order sequence metadata tracking token. |
| `QColor` | [`m_color`](#m_color-2)  | Primary outline and line rule display color definition. |
| `QColor` | [`m_bgColor`](#m_bgcolor-3)  | Readout info box opaque background plate fill color profile. |
| `RtpCursor` | [`m_cStart`](#m_cstart)  | Core tracking subcomponent instance representing left bound marker. |
| `RtpCursor` | [`m_cEnd`](#m_cend)  | Core tracking subcomponent instance representing right bound marker. |
| `RtpCursor::MarkerStyle` | [`m_style`](#m_style-1)  | Geometric styling shape type rendering configuration mode. |
| `const std::vector< std::shared_ptr< PlotSeries > > *` | [`m_seriesRef`](#m_seriesref-2)  | Internal dataset lookup registry array mapping tracking lines. |

---

### m_isEnabled

```cpp
bool m_isEnabled =
        true
```

Defined in include/components/RtpRangeCursor.h:123

Structural execution state controlling drawing passes.

---

### m_xPos

```cpp
double m_xPos =
        0
```

Defined in include/components/RtpRangeCursor.h:125

Master anchor localization reference metric point tracker.

---

### m_isDragging

```cpp
bool m_isDragging =
        false
```

Defined in include/components/RtpRangeCursor.h:128

Mutual exclusion flag lock tracking interactive relocation inputs.

---

### m_dragOffset

```cpp
int m_dragOffset =
        0
```

Defined in include/components/RtpRangeCursor.h:130

Offset value cache preserving layout adjustment tracking pixels.

---

### m_cursorIndex

```cpp
int m_cursorIndex
```

Defined in include/components/RtpRangeCursor.h:133

Identifying component order sequence metadata tracking token.

---

### m_color

```cpp
QColor m_color
```

Defined in include/components/RtpRangeCursor.h:136

Primary outline and line rule display color definition.

---

### m_bgColor

```cpp
QColor m_bgColor
```

Defined in include/components/RtpRangeCursor.h:138

Readout info box opaque background plate fill color profile.

---

### m_cStart

```cpp
RtpCursor m_cStart
```

Defined in include/components/RtpRangeCursor.h:141

Core tracking subcomponent instance representing left bound marker.

---

### m_cEnd

```cpp
RtpCursor m_cEnd
```

Defined in include/components/RtpRangeCursor.h:143

Core tracking subcomponent instance representing right bound marker.

---

### m_style

```cpp
RtpCursor::MarkerStyle m_style
```

Defined in include/components/RtpRangeCursor.h:146

Geometric styling shape type rendering configuration mode.

---

### m_seriesRef

```cpp
const std::vector< std::shared_ptr< PlotSeries > > * m_seriesRef = nullptr
```

Defined in include/components/RtpRangeCursor.h:149

Internal dataset lookup registry array mapping tracking lines.


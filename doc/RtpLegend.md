{#rtplegend}

# RtpLegend

```cpp
#include <RtpLegend.h>
```

```cpp
class RtpLegend
```

Defined in include/components/RtpLegend.h:17

Manages the visual layout, rendering, and interaction of the chart legend.

This class handles rendering the series names and color indicators on an overlay using QPainter. It also provides mouse click detection to allow toggling the visibility of individual plot lines.

## List of all members

| Name | Kind | Owner |
|------|------|-------|
| [`RtpLegend`](#rtplegend-1) | `function` | Declared here |
| [`setSeriesSource`](#setseriessource-1) | `function` | Declared here |
| [`setFont`](#setfont-1) | `function` | Declared here |
| [`setTextColor`](#settextcolor-1) | `function` | Declared here |
| [`setBackgroundColor`](#setbackgroundcolor-1) | `function` | Declared here |
| [`setMargin`](#setmargin-1) | `function` | Declared here |
| [`draw`](#draw-1) | `function` | Declared here |
| [`checkClick`](#checkclick) | `function` | Declared here |
| [`lastRenderedRect`](#lastrenderedrect) | `function` | Declared here |
| [`m_font`](#m_font-1) | `variable` | Declared here |
| [`m_bgColor`](#m_bgcolor-1) | `variable` | Declared here |
| [`m_padding`](#m_padding) | `variable` | Declared here |
| [`m_lastRect`](#m_lastrect) | `variable` | Declared here |
| [`m_seriesRef`](#m_seriesref-1) | `variable` | Declared here |

## Public Methods

| Return | Name | Description |
|--------|------|-------------|
|  | [`RtpLegend`](#rtplegend-1)  | Constructs a plot legend overlay manager instance. |
| `void` | [`setSeriesSource`](#setseriessource-1)  | Sets the data source pointer containing the collection of plot series. |
| `void` | [`setFont`](#setfont-1)  | Configuration modifier to change the text label font settings. |
| `void` | [`setTextColor`](#settextcolor-1)  | Adjusts the primary color used to draw text strings. |
| `void` | [`setBackgroundColor`](#setbackgroundcolor-1)  | Adjusts the background box solid mask color fill. |
| `void` | [`setMargin`](#setmargin-1)  | Sets the outer spacing metric around text bounding masks in screen pixels. |
| `void` | [`draw`](#draw-1)  | Renders the legend window overlay on the provided painter canvas context. |
| `int` | [`checkClick`](#checkclick) `const` | Tests if a given screen mouse coordinate lands within a valid legend series item row. |
| `QRect` | [`lastRenderedRect`](#lastrenderedrect) `const` `inline` | Retrieves the boundary pixel constraints from the most recent rendering pass execution. |

---

{#rtplegend-1}

### RtpLegend

```cpp
RtpLegend()
```

Defined in include/components/RtpLegend.h:23

Constructs a plot legend overlay manager instance.

---

{#setseriessource-1}

### setSeriesSource

```cpp
void setSeriesSource(const std::vector< std::shared_ptr< PlotSeries > > * series)
```

Defined in include/components/RtpLegend.h:29

Sets the data source pointer containing the collection of plot series.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `series` | `const std::vector< std::shared_ptr< [PlotSeries](PlotSeries.md#plotseries) > > *` | Constant pointer to the std::vector containing shared pointers of [PlotSeries](PlotSeries.md#plotseries). |

---

{#setfont-1}

### setFont

```cpp
void setFont(const QFont & font)
```

Defined in include/components/RtpLegend.h:36

Configuration modifier to change the text label font settings.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `font` | `const QFont &` | QFont styling properties to apply to the text items. |

---

{#settextcolor-1}

### setTextColor

```cpp
void setTextColor(const QColor & color)
```

Defined in include/components/RtpLegend.h:42

Adjusts the primary color used to draw text strings.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `color` | `const QColor &` | QColor description matching the target text layout style. |

---

{#setbackgroundcolor-1}

### setBackgroundColor

```cpp
void setBackgroundColor(const QColor & bgColor)
```

Defined in include/components/RtpLegend.h:48

Adjusts the background box solid mask color fill.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `bgColor` | `const QColor &` | QColor applied to wash the background bounding box area. |

---

{#setmargin-1}

### setMargin

```cpp
void setMargin(int marginPixels)
```

Defined in include/components/RtpLegend.h:54

Sets the outer spacing metric around text bounding masks in screen pixels.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `marginPixels` | `int` | Target padding distance in screen pixels. |

---

{#draw-1}

### draw

```cpp
void draw(QPainter & painter, const QRect & plotRect)
```

Defined in include/components/RtpLegend.h:61

Renders the legend window overlay on the provided painter canvas context.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `painter` | `QPainter &` | QPainter surface canvas tracking rendering pipelines. |
| `plotRect` | `const QRect &` | Bounding pixel constraints enclosing the active chart area widget. |

---

{#checkclick}

### checkClick

`const`

```cpp
int checkClick(const QPoint & mousePos) const
```

Defined in include/components/RtpLegend.h:72

Tests if a given screen mouse coordinate lands within a valid legend series item row.

Used to implement hit-testing to see if a user clicked on a specific series item entry to perform runtime mutations (e.g., toggling that specific channel's visibility).

#### Returns
Integer index of the matched plot channel entry row, or -1 if the click missed.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `mousePos` | `const QPoint &` | Integer mouse coordinate position localized relative to the parent widget map. |

---

{#lastrenderedrect}

### lastRenderedRect

`const` `inline`

```cpp
inline QRect lastRenderedRect() const
```

Defined in include/components/RtpLegend.h:77

Retrieves the boundary pixel constraints from the most recent rendering pass execution.

*
#### Returns
QRect representing the screen space layout geometry envelope occupied by the component.

## Private Attributes

| Return | Name | Description |
|--------|------|-------------|
| `QFont` | [`m_font`](#m_font-1)  | Font specification asset mapping descriptor settings onto label outputs. |
| `QColor` | [`m_bgColor`](#m_bgcolor-1)  | Solid clear back-face channel color metrics profile map configuration. |
| `int` | [`m_padding`](#m_padding)  | Inner alignment border separation distance threshold scaled in screen pixels. |
| `QRect` | [`m_lastRect`](#m_lastrect)  | Geometry box cache capturing metrics from the historical draw call operation. |
| `const std::vector< std::shared_ptr< PlotSeries > > *` | [`m_seriesRef`](#m_seriesref-1)  | Reference to the Plot Series, usually configured from the RealTimePlot. |

---

{#m_font-1}

### m_font

```cpp
QFont m_font
```

Defined in include/components/RtpLegend.h:81

Font specification asset mapping descriptor settings onto label outputs.

---

{#m_bgcolor-1}

### m_bgColor

```cpp
QColor m_bgColor
```

Defined in include/components/RtpLegend.h:83

Solid clear back-face channel color metrics profile map configuration.

---

{#m_padding}

### m_padding

```cpp
int m_padding
```

Defined in include/components/RtpLegend.h:84

Inner alignment border separation distance threshold scaled in screen pixels.

---

{#m_lastrect}

### m_lastRect

```cpp
QRect m_lastRect
```

Defined in include/components/RtpLegend.h:86

Geometry box cache capturing metrics from the historical draw call operation.

---

{#m_seriesref-1}

### m_seriesRef

```cpp
const std::vector< std::shared_ptr< PlotSeries > > * m_seriesRef = nullptr
```

Defined in include/components/RtpLegend.h:89

Reference to the Plot Series, usually configured from the RealTimePlot.


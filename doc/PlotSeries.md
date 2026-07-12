

# PlotSeries

```cpp
#include <PlotSeries.h>
```

```cpp
class PlotSeries
```

Defined in include/PlotSeries.h:24

> **Inherits:** `QOpenGLFunctions`

Thread-safe circular buffer for a single plot series.

Stores (x, y) coordinate pairs with support for multi-level Level of Detail (LOD) and real-time statistics calculation. Data can be pushed from any thread (e.g., data acquisition threads), while the rendering loop reads data safely using mutual exclusion locks (mutex).

## List of all members

| Name | Kind | Owner |
|------|------|-------|
| [`RealtimePlot`](#realtimeplot) | `friend` | Declared here |
| [`PlotSeries`](#plotseries-1) | `function` | Declared here |
| [`pushPoint`](#pushpoint) | `function` | Declared here |
| [`pushPoint`](#pushpoint-1) | `function` | Declared here |
| [`clear`](#clear) | `function` | Declared here |
| [`lock`](#lock) | `function` | Declared here |
| [`points`](#points) | `function` | Declared here |
| [`name`](#name) | `function` | Declared here |
| [`color`](#color) | `function` | Declared here |
| [`setColor`](#setcolor) | `function` | Declared here |
| [`setName`](#setname) | `function` | Declared here |
| [`visible`](#visible) | `function` | Declared here |
| [`setVisible`](#setvisible) | `function` | Declared here |
| [`lineWidth`](#linewidth) | `function` | Declared here |
| [`setLineWidth`](#setlinewidth) | `function` | Declared here |
| [`xMin`](#xmin) | `function` | Declared here |
| [`xMax`](#xmax) | `function` | Declared here |
| [`yMin`](#ymin) | `function` | Declared here |
| [`yMax`](#ymax) | `function` | Declared here |
| [`getVisiblePoints`](#getvisiblepoints) | `function` | Declared here |
| [`getDataPoints`](#getdatapoints) | `function` | Declared here |
| [`size`](#size) | `function` | Declared here |
| [`getClosestPointToX`](#getclosestpointtox) | `function` | Declared here |
| [`calculateIntervalStats`](#calculateintervalstats) | `function` | Declared here |
| [`m_mutex`](#m_mutex) | `variable` | Declared here |
| [`m_points`](#m_points) | `variable` | Declared here |
| [`m_visibleBuffer`](#m_visiblebuffer) | `variable` | Declared here |
| [`m_lodLevels`](#m_lodlevels) | `variable` | Declared here |
| [`m_name`](#m_name) | `variable` | Declared here |
| [`m_color`](#m_color-1) | `variable` | Declared here |
| [`m_visible`](#m_visible) | `variable` | Declared here |
| [`m_lineWidth`](#m_linewidth) | `variable` | Declared here |
| [`m_xMin`](#m_xmin) | `variable` | Declared here |
| [`m_xMax`](#m_xmax) | `variable` | Declared here |
| [`m_yMin`](#m_ymin) | `variable` | Declared here |
| [`m_yMax`](#m_ymax) | `variable` | Declared here |
| [`m_vbo`](#m_vbo) | `variable` | Declared here |
| [`m_vao`](#m_vao) | `variable` | Declared here |
| [`m_glInitialized`](#m_glinitialized) | `variable` | Declared here |
| [`m_pointsInGPU`](#m_pointsingpu) | `variable` | Declared here |
| [`updateBounds`](#updatebounds) | `function` | Declared here |
| [`recomputeBounds`](#recomputebounds) | `function` | Declared here |
| [`updateLodLevels`](#updatelodlevels) | `function` | Declared here |
| [`initGLBuffers`](#initglbuffers) | `function` | Declared here |
| [`destroyGLBuffers`](#destroyglbuffers) | `function` | Declared here |
| [`updateVBO`](#updatevbo) | `function` | Declared here |
| [`vao`](#vao) | `function` | Declared here |
| [`vertexCount`](#vertexcount) | `function` | Declared here |
| [`vbo`](#vbo) | `function` | Declared here |
| [`syncWithGPU`](#syncwithgpu) | `function` | Declared here |
| [`uploadVisiblePoints`](#uploadvisiblepoints) | `function` | Declared here |

## Friends

| Name | Description |
|------|-------------|
| [`RealtimePlot`](#realtimeplot)  |  |

---

### RealtimePlot

```cpp
friend class RealtimePlot
```

Defined in include/PlotSeries.h:26

## Public Methods

| Return | Name | Description |
|--------|------|-------------|
|  | [`PlotSeries`](#plotseries-1) `explicit` | Constructs the plot data series. |
| `void` | [`pushPoint`](#pushpoint)  | Pushes a new point into the series using explicit X and Y components. |
| `void` | [`pushPoint`](#pushpoint-1)  | Pushes a structured point into the series. |
| `void` | [`clear`](#clear)  | Completely flushes the data series, resetting bounding boxes and LOD tiers. |
| `std::unique_lock< std::mutex >` | [`lock`](#lock) `const` | Acquires the internal mutex lock for external safe operations. |
| `const std::vector< Point > &` | [`points`](#points) `const` `inline` | Retrieves a constant reference to the raw points vector. |
| `const QString &` | [`name`](#name) `const` `inline` |  |
| `QColor` | [`color`](#color) `const` `inline` |  |
| `void` | [`setColor`](#setcolor) `inline` |  |
| `void` | [`setName`](#setname) `inline` |  |
| `bool` | [`visible`](#visible) `const` `inline` |  |
| `void` | [`setVisible`](#setvisible) `inline` |  |
| `float` | [`lineWidth`](#linewidth) `const` `inline` |  |
| `void` | [`setLineWidth`](#setlinewidth) `inline` |  |
| `double` | [`xMin`](#xmin) `const` `inline` |  |
| `double` | [`xMax`](#xmax) `const` `inline` |  |
| `double` | [`yMin`](#ymin) `const` `inline` |  |
| `double` | [`yMax`](#ymax) `const` `inline` |  |
| `std::vector< PlotSeries::Point > &` | [`getVisiblePoints`](#getvisiblepoints)  | Extracts a subset of points optimized for a target horizontal pixel resolution. |
| `std::vector< PlotSeries::Point > &` | [`getDataPoints`](#getdatapoints) `inline` | Gets raw points stored in a specific Level of Detail (LOD) tier. |
| `int` | [`size`](#size) `inline` |  |
| `PlotSeries::Point` | [`getClosestPointToX`](#getclosestpointtox)  | Safely searches for the closest point relative to a given X value. |
| `PlotSeries::IntervalStats` | [`calculateIntervalStats`](#calculateintervalstats)  | Computes advanced metrics (min, max, mean, RMS) within a specified X interval. |

---

### PlotSeries

`explicit`

```cpp
explicit PlotSeries(QString name, QColor color = Qt::cyan)
```

Defined in include/PlotSeries.h:79

Constructs the plot data series.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `QString` | Identifying name of the series (e.g., "Channel 1"). |
| `color` | `QColor` | Color used to render the series by default (Default: Qt::cyan). |

---

### pushPoint

```cpp
void pushPoint(double x, double y)
```

Defined in include/PlotSeries.h:89

Pushes a new point into the series using explicit X and Y components.

:::note
This method is thread-safe. 

:::

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `x` | `double` | X coordinate of the new point. |
| `y` | `double` | Y coordinate of the new point. |

---

### pushPoint

```cpp
void pushPoint(const Point & p)
```

Defined in include/PlotSeries.h:96

Pushes a structured point into the series.

:::note
This method is thread-safe. 

:::

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `p` | `const [Point](PlotSeries-Point.md#point) &` | [Point](PlotSeries-Point.md#point) structure containing the data to append. |

---

### clear

```cpp
void clear()
```

Defined in include/PlotSeries.h:102

Completely flushes the data series, resetting bounding boxes and LOD tiers.

:::note
This method is thread-safe. 

:::

---

### lock

`const`

```cpp
std::unique_lock< std::mutex > lock() const
```

Defined in include/PlotSeries.h:110

Acquires the internal mutex lock for external safe operations.

#### Returns
A std::unique_lock object managing the instance's mutex.

---

### points

`const` `inline`

```cpp
inline const std::vector< Point > & points() const
```

Defined in include/PlotSeries.h:117

Retrieves a constant reference to the raw points vector.

:::warning
Must be called under [lock()](#lock) if accessed from a non-rendering thread. 

:::

#### Returns
Constant reference to the underlying std::vector of raw points.

---

### name

`const` `inline`

```cpp
inline const QString & name() const
```

Defined in include/PlotSeries.h:122

#### Returns
The current name of the series.

---

### color

`const` `inline`

```cpp
inline QColor color() const
```

Defined in include/PlotSeries.h:125

#### Returns
The color assigned to this series.

---

### setColor

`inline`

```cpp
inline void setColor(QColor c)
```

Defined in include/PlotSeries.h:128

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `c` | `QColor` | New color for the series rendering. |

---

### setName

`inline`

```cpp
inline void setName(const QString & n)
```

Defined in include/PlotSeries.h:131

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `n` | `const QString &` | New name for the series. |

---

### visible

`const` `inline`

```cpp
inline bool visible() const
```

Defined in include/PlotSeries.h:134

#### Returns
True if the series is currently configured to be visible.

---

### setVisible

`inline`

```cpp
inline void setVisible(bool v)
```

Defined in include/PlotSeries.h:137

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `v` | `bool` | Desired visibility state. |

---

### lineWidth

`const` `inline`

```cpp
inline float lineWidth() const
```

Defined in include/PlotSeries.h:140

#### Returns
Line width in pixels.

---

### setLineWidth

`inline`

```cpp
inline void setLineWidth(float w)
```

Defined in include/PlotSeries.h:143

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `w` | `float` | New line width in pixels (e.g., 1.5f). |

---

### xMin

`const` `inline`

```cpp
inline double xMin() const
```

Defined in include/PlotSeries.h:148

#### Returns
Current minimum absolute value along the X-axis.

---

### xMax

`const` `inline`

```cpp
inline double xMax() const
```

Defined in include/PlotSeries.h:150

#### Returns
Current maximum absolute value along the X-axis.

---

### yMin

`const` `inline`

```cpp
inline double yMin() const
```

Defined in include/PlotSeries.h:152

#### Returns
Current minimum absolute value along the Y-axis.

---

### yMax

`const` `inline`

```cpp
inline double yMax() const
```

Defined in include/PlotSeries.h:154

#### Returns
Current maximum absolute value along the Y-axis.

---

### getVisiblePoints

```cpp
std::vector< PlotSeries::Point > & getVisiblePoints(double xMin, double xMax, int targetWidth)
```

Defined in include/PlotSeries.h:167

Extracts a subset of points optimized for a target horizontal pixel resolution.

Filters or decimates points based on the visible viewport boundary (xMin to xMax) and the pixel width of the widget container to prevent performance-killing overdraw.

#### Returns
Reference to an internal vector containing ready-to-render points.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `xMin` | `double` | Lower visible threshold of the X-axis. |
| `xMax` | `double` | Upper visible threshold of the X-axis. |
| `targetWidth` | `int` | Width in pixels of the drawing area on the screen. |

---

### getDataPoints

`inline`

```cpp
inline std::vector< PlotSeries::Point > & getDataPoints(uint32_t lod = 0)
```

Defined in include/PlotSeries.h:175

Gets raw points stored in a specific Level of Detail (LOD) tier.

#### Returns
Reference to the selected LOD tier point vector.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `lod` | `uint32_t` | Index of the target LOD tier (defaults to 0, maximum resolution). |

---

### size

`inline`

```cpp
inline int size()
```

Defined in include/PlotSeries.h:181

#### Returns
Total number of entries in the main series buffer.

---

### getClosestPointToX

```cpp
PlotSeries::Point getClosestPointToX(double xValue)
```

Defined in include/PlotSeries.h:188

Safely searches for the closest point relative to a given X value.

#### Returns
The [Point](PlotSeries-Point.md#point) structure sitting closest to the searched value.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `xValue` | `double` | Reference lookup value along the X-axis. |

---

### calculateIntervalStats

```cpp
PlotSeries::IntervalStats calculateIntervalStats(double xMin, double xMax)
```

Defined in include/PlotSeries.h:196

Computes advanced metrics (min, max, mean, RMS) within a specified X interval.

#### Returns
An [IntervalStats](PlotSeries-IntervalStats.md#intervalstats) structure populated with the mathematical findings.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `xMin` | `double` | Starting edge of the analysis window. |
| `xMax` | `double` | Ending edge of the analysis window. |

## Protected Attributes

| Return | Name | Description |
|--------|------|-------------|
| `std::mutex` | [`m_mutex`](#m_mutex)  | Mutex to ensure mutual exclusion across multi-threaded operations. |
| `std::vector< Point >` | [`m_points`](#m_points)  | Primary storage containing raw signal data. |
| `std::vector< Point >` | [`m_visibleBuffer`](#m_visiblebuffer)  | Scratchpad buffer used for exchanging and filtering displayable points. |
| `std::vector< LodLevel >` | [`m_lodLevels`](#m_lodlevels)  | Dynamic array holding discrete LOD data structures. |
| `QString` | [`m_name`](#m_name)  | Name identifier of the series. |
| `QColor` | [`m_color`](#m_color-1)  | Rendering color profile. |
| `bool` | [`m_visible`](#m_visible)  | Control flag to skip or commit execution of the render pipeline. |
| `float` | [`m_lineWidth`](#m_linewidth)  | Line thickness configuration for rasterized rendering commands. |
| `double` | [`m_xMin`](#m_xmin)  | Cached absolute minimum X boundary. |
| `double` | [`m_xMax`](#m_xmax)  | Cached absolute maximum X boundary. |
| `double` | [`m_yMin`](#m_ymin)  | Cached absolute minimum Y boundary. |
| `double` | [`m_yMax`](#m_ymax)  | Cached absolute maximum Y boundary. |
| `QOpenGLBuffer` | [`m_vbo`](#m_vbo)  | Qt abstraction wrapper for the Vertex Buffer Object. |
| `QOpenGLVertexArrayObject` | [`m_vao`](#m_vao)  | Qt abstraction wrapper for the Vertex Array Object. |
| `bool` | [`m_glInitialized`](#m_glinitialized)  | Confirms whether initGLBuffers was run successfully. |
| `size_t` | [`m_pointsInGPU`](#m_pointsingpu)  | Total count of vertices currently allocated on VRAM. |

---

### m_mutex

```cpp
std::mutex m_mutex
```

Defined in include/PlotSeries.h:227

Mutex to ensure mutual exclusion across multi-threaded operations.

---

### m_points

```cpp
std::vector< Point > m_points
```

Defined in include/PlotSeries.h:229

Primary storage containing raw signal data.

---

### m_visibleBuffer

```cpp
std::vector< Point > m_visibleBuffer
```

Defined in include/PlotSeries.h:231

Scratchpad buffer used for exchanging and filtering displayable points.

---

### m_lodLevels

```cpp
std::vector< LodLevel > m_lodLevels
```

Defined in include/PlotSeries.h:234

Dynamic array holding discrete LOD data structures.

---

### m_name

```cpp
QString m_name
```

Defined in include/PlotSeries.h:245

Name identifier of the series.

---

### m_color

```cpp
QColor m_color
```

Defined in include/PlotSeries.h:246

Rendering color profile.

---

### m_visible

```cpp
bool m_visible =
        true
```

Defined in include/PlotSeries.h:247

Control flag to skip or commit execution of the render pipeline.

---

### m_lineWidth

```cpp
float m_lineWidth =
        1.5f
```

Defined in include/PlotSeries.h:249

Line thickness configuration for rasterized rendering commands.

---

### m_xMin

```cpp
double m_xMin = std::numeric_limits<
        double>::max()
```

Defined in include/PlotSeries.h:252

Cached absolute minimum X boundary.

---

### m_xMax

```cpp
double m_xMax = -std::numeric_limits<
        double>::max()
```

Defined in include/PlotSeries.h:254

Cached absolute maximum X boundary.

---

### m_yMin

```cpp
double m_yMin = std::numeric_limits<
        double>::max()
```

Defined in include/PlotSeries.h:256

Cached absolute minimum Y boundary.

---

### m_yMax

```cpp
double m_yMax = -std::numeric_limits<
        double>::max()
```

Defined in include/PlotSeries.h:258

Cached absolute maximum Y boundary.

---

### m_vbo

```cpp
QOpenGLBuffer m_vbo {
        QOpenGLBuffer::
            VertexBuffer}
```

Defined in include/PlotSeries.h:261

Qt abstraction wrapper for the Vertex Buffer Object.

---

### m_vao

```cpp
QOpenGLVertexArrayObject m_vao
```

Defined in include/PlotSeries.h:265

Qt abstraction wrapper for the Vertex Array Object.

---

### m_glInitialized

```cpp
bool m_glInitialized =
        false
```

Defined in include/PlotSeries.h:266

Confirms whether initGLBuffers was run successfully.

---

### m_pointsInGPU

```cpp
size_t m_pointsInGPU =
        0
```

Defined in include/PlotSeries.h:268

Total count of vertices currently allocated on VRAM.

## Protected Methods

| Return | Name | Description |
|--------|------|-------------|
| `void` | [`updateBounds`](#updatebounds)  | Dynamically expands the current bounding box metrics to encompass a new point. |
| `void` | [`recomputeBounds`](#recomputebounds)  | Linearly recomputes absolute boundaries (mins and maxes) by scanning the entire series. |
| `void` | [`updateLodLevels`](#updatelodlevels)  | Increments or completely updates the LOD data arrays using newly injected points. |
| `void` | [`initGLBuffers`](#initglbuffers)  | Initializes OpenGL objects (VBO and VAO) in the current context. |
| `void` | [`destroyGLBuffers`](#destroyglbuffers)  | Releases and destroys the OpenGL buffers linked to this series. |
| `void` | [`updateVBO`](#updatevbo)  | Updates the Vertex Buffer Object (VBO) content with current RAM data. |
| `QOpenGLVertexArrayObject *` | [`vao`](#vao) `inline` | Gets the Vertex Array Object managed by this series. |
| `size_t` | [`vertexCount`](#vertexcount) `const` `inline` |  |
| `QOpenGLBuffer &` | [`vbo`](#vbo) `inline` | Gets a reference to the underlying Vertex Buffer Object (VBO). |
| `void` | [`syncWithGPU`](#syncwithgpu)  | Synchronizes CPU memory data over to the GPU efficiently. |
| `void` | [`uploadVisiblePoints`](#uploadvisiblepoints)  | Uploads an external buffer of pre-filtered visible points straight to the GPU VBO. |

---

### updateBounds

```cpp
void updateBounds(const Point & p)
```

Defined in include/PlotSeries.h:219

Dynamically expands the current bounding box metrics to encompass a new point.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `p` | `const [Point](PlotSeries-Point.md#point) &` | [Point](PlotSeries-Point.md#point) structure to evaluate. |

---

### recomputeBounds

```cpp
void recomputeBounds()
```

Defined in include/PlotSeries.h:224

Linearly recomputes absolute boundaries (mins and maxes) by scanning the entire series.

---

### updateLodLevels

```cpp
void updateLodLevels()
```

Defined in include/PlotSeries.h:243

Increments or completely updates the LOD data arrays using newly injected points.

---

### initGLBuffers

```cpp
void initGLBuffers()
```

Defined in include/PlotSeries.h:277

Initializes OpenGL objects (VBO and VAO) in the current context.

:::note
Must be called from the thread that owns the OpenGL context (Render Thread). 

:::

---

### destroyGLBuffers

```cpp
void destroyGLBuffers()
```

Defined in include/PlotSeries.h:282

Releases and destroys the OpenGL buffers linked to this series.

---

### updateVBO

```cpp
void updateVBO()
```

Defined in include/PlotSeries.h:287

Updates the Vertex Buffer Object (VBO) content with current RAM data.

---

### vao

`inline`

```cpp
inline QOpenGLVertexArrayObject * vao()
```

Defined in include/PlotSeries.h:293

Gets the Vertex Array Object managed by this series.

#### Returns
Pointer to the QOpenGLVertexArrayObject instance.

---

### vertexCount

`const` `inline`

```cpp
inline size_t vertexCount() const
```

Defined in include/PlotSeries.h:296

#### Returns
Total vertex count currently stored in RAM.

---

### vbo

`inline`

```cpp
inline QOpenGLBuffer & vbo()
```

Defined in include/PlotSeries.h:302

Gets a reference to the underlying Vertex Buffer Object (VBO).

#### Returns
Reference to the VertexBuffer-type QOpenGLBuffer.

---

### syncWithGPU

```cpp
void syncWithGPU()
```

Defined in include/PlotSeries.h:307

Synchronizes CPU memory data over to the GPU efficiently.

---

### uploadVisiblePoints

```cpp
void uploadVisiblePoints(const std::vector< Point > & visiblePts)
```

Defined in include/PlotSeries.h:313

Uploads an external buffer of pre-filtered visible points straight to the GPU VBO.

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `visiblePts` | `const std::vector< [Point](PlotSeries-Point.md#point) > &` | Vector containing the pre-processed points to be written to VRAM. |


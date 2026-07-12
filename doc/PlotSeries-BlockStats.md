{#blockstats}

# BlockStats

```cpp
#include <PlotSeries.h>
```

```cpp
struct BlockStats
```

Defined in include/PlotSeries.h:45

Accumulated statistics for a specific block of data.

Used to optimize metric calculations over data subsets and for point reduction routines within Level of Detail (LOD) generation.

## List of all members

| Name | Kind | Owner |
|------|------|-------|
| [`xStart`](#xstart) | `variable` | Declared here |
| [`xEnd`](#xend) | `variable` | Declared here |
| [`min`](#min) | `variable` | Declared here |
| [`max`](#max) | `variable` | Declared here |
| [`sum`](#sum) | `variable` | Declared here |
| [`sumSq`](#sumsq) | `variable` | Declared here |
| [`count`](#count) | `variable` | Declared here |

## Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `double` | [`xStart`](#xstart)  | Starting X value of this data block. |
| `double` | [`xEnd`](#xend)  | Ending X value of this data block. |
| `double` | [`min`](#min)  | Minimum Y value found within this block. |
| `double` | [`max`](#max)  | Maximum Y value found within this block. |
| `double` | [`sum`](#sum)  | Accumulated sum of Y values. Useful for fast mean calculation (mean = sum / count). |
| `double` | [`sumSq`](#sumsq)  | Sum of squares of Y values. Useful for Root Mean Square (RMS) or standard deviation. |
| `size_t` | [`count`](#count)  | Total number of samples processed in this block. |

---

{#xstart}

### xStart

```cpp
double xStart
```

Defined in include/PlotSeries.h:46

Starting X value of this data block.

---

{#xend}

### xEnd

```cpp
double xEnd
```

Defined in include/PlotSeries.h:47

Ending X value of this data block.

---

{#min}

### min

```cpp
double min
```

Defined in include/PlotSeries.h:48

Minimum Y value found within this block.

---

{#max}

### max

```cpp
double max
```

Defined in include/PlotSeries.h:49

Maximum Y value found within this block.

---

{#sum}

### sum

```cpp
double sum
```

Defined in include/PlotSeries.h:51

Accumulated sum of Y values. Useful for fast mean calculation (mean = sum / count).

---

{#sumsq}

### sumSq

```cpp
double sumSq
```

Defined in include/PlotSeries.h:53

Sum of squares of Y values. Useful for Root Mean Square (RMS) or standard deviation.

---

{#count}

### count

```cpp
size_t count
```

Defined in include/PlotSeries.h:54

Total number of samples processed in this block.


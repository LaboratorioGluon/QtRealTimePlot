

# IntervalStats

```cpp
#include <PlotSeries.h>
```

```cpp
struct IntervalStats
```

Defined in include/PlotSeries.h:61

Statistical metrics calculated over a specific chart window.

## List of all members

| Name | Kind | Owner |
|------|------|-------|
| [`min`](#min-1) | `variable` | Declared here |
| [`max`](#max-1) | `variable` | Declared here |
| [`mean`](#mean) | `variable` | Declared here |
| [`rms`](#rms) | `variable` | Declared here |
| [`numSamples`](#numsamples) | `variable` | Declared here |
| [`timeDiff`](#timediff) | `variable` | Declared here |

## Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `double` | [`min`](#min-1)  | Minimum value in the interval. |
| `double` | [`max`](#max-1)  | Maximum value in the interval. |
| `double` | [`mean`](#mean)  | Mean (average) value in the interval. |
| `double` | [`rms`](#rms)  | Root Mean Square (RMS) value in the interval. |
| `int` | [`numSamples`](#numsamples)  | Total number of samples contained in the interval. |
| `double` | [`timeDiff`](#timediff)  | Time delta (X_max - X_min) of the analyzed interval. |

---

### min

```cpp
double min = std::numeric_limits<
            double>()
```

Defined in include/PlotSeries.h:62

Minimum value in the interval.

---

### max

```cpp
double max = -std::numeric_limits<
            double>::max()
```

Defined in include/PlotSeries.h:64

Maximum value in the interval.

---

### mean

```cpp
double mean = 0.0
```

Defined in include/PlotSeries.h:66

Mean (average) value in the interval.

---

### rms

```cpp
double rms = 0.0
```

Defined in include/PlotSeries.h:67

Root Mean Square (RMS) value in the interval.

---

### numSamples

```cpp
int numSamples =
            0
```

Defined in include/PlotSeries.h:68

Total number of samples contained in the interval.

---

### timeDiff

```cpp
double timeDiff =
            0.0
```

Defined in include/PlotSeries.h:70

Time delta (X_max - X_min) of the analyzed interval.


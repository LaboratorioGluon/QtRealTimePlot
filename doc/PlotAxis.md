

# PlotAxis

```cpp
#include <PlotAxis.h>
```

```cpp
class PlotAxis
```

Defined in include/PlotAxis.h:13

Computes human-friendly tick marks for a given range.

Uses the "nice numbers" algorithm to choose a step size that produces 4–10 readable labels regardless of zoom level.

## List of all members

| Name | Kind | Owner |
|------|------|-------|
| [`computeTicks`](#computeticks) | `function` | Declared here |
| [`formatValue`](#formatvalue) | `function` | Declared here |
| [`niceNum`](#nicenum) | `function` | Declared here |

## Public Static Methods

| Return | Name | Description |
|--------|------|-------------|
| `QVector< Tick >` | [`computeTicks`](#computeticks) `static` |  |
| `QString` | [`formatValue`](#formatvalue) `static` |  |

---

### computeTicks

`static`

```cpp
static QVector< Tick > computeTicks(double lo, double hi, int maxTicks = 8)
```

Defined in include/PlotAxis.h:22

---

### formatValue

`static`

```cpp
static QString formatValue(double value, double step)
```

Defined in include/PlotAxis.h:25

## Private Static Methods

| Return | Name | Description |
|--------|------|-------------|
| `double` | [`niceNum`](#nicenum) `static` |  |

---

### niceNum

`static`

```cpp
static double niceNum(double range, bool round)
```

Defined in include/PlotAxis.h:28


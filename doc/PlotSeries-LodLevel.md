{#lodlevel}

# LodLevel

```cpp
#include <PlotSeries.h>
```

```cpp
struct LodLevel
```

Defined in include/PlotSeries.h:206

Container for a specific Level of Detail (LOD) tier.

Stores decimated or simplified versions of the original signal to accelerate rendering when viewing large time windows.

## List of all members

| Name | Kind | Owner |
|------|------|-------|
| [`points`](#points-1) | `variable` | Declared here |
| [`stats`](#stats) | `variable` | Declared here |
| [`lastProcessedCrudeSize`](#lastprocessedcrudesize) | `variable` | Declared here |

## Public Attributes

| Return | Name | Description |
|--------|------|-------------|
| `std::vector< Point >` | [`points`](#points-1)  | Decimated points corresponding to this LOD level. |
| `std::vector< BlockStats >` | [`stats`](#stats)  | Statistics of the blocks that make up this level. |
| `size_t` | [`lastProcessedCrudeSize`](#lastprocessedcrudesize)  | Amount of raw elements already processed for this level. |

---

{#points-1}

### points

```cpp
std::vector< Point > points
```

Defined in include/PlotSeries.h:208

Decimated points corresponding to this LOD level.

---

{#stats}

### stats

```cpp
std::vector< BlockStats > stats
```

Defined in include/PlotSeries.h:210

Statistics of the blocks that make up this level.

---

{#lastprocessedcrudesize}

### lastProcessedCrudeSize

```cpp
size_t lastProcessedCrudeSize =
            0
```

Defined in include/PlotSeries.h:211

Amount of raw elements already processed for this level.


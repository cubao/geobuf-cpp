# pybind11_geobuf — Python User Guide

`pybind11_geobuf` is a C++ implementation of [Geobuf](https://github.com/mapbox/geobuf) (compact binary GeoJSON via Protocol Buffers) with full Python bindings. It provides:

- Fast GeoJSON ↔ Geobuf (`.pbf`) encoding and decoding
- First-class GeoJSON object types (Feature, FeatureCollection, geometries)
- Spatial indexing with random access to individual features in large files
- In-memory feature collections with R-tree queries and polygon clipping
- Coordinate transforms (WGS84 ↔ ENU, affine, rotate, translate)
- JSON normalization utilities

```bash
pip install pybind11_geobuf
```

```python
import pybind11_geobuf as gb
from pybind11_geobuf import geojson
```

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [The `__call__()` Pattern — Critical Reading](#the-__call__-pattern--critical-reading)
3. [GeoJSON Object Model](#geojson-object-model)
4. [Working with Features](#working-with-features)
5. [Working with FeatureCollections](#working-with-featurecollections)
6. [Geometry Types](#geometry-types)
7. [Encoder](#encoder)
8. [Decoder](#decoder)
9. [GeobufIndex — Large File Random Access](#geobufindex--large-file-random-access)
10. [Planet — In-Memory Spatial Index](#planet--in-memory-spatial-index)
11. [rapidjson](#rapidjson)
12. [Coordinate Transforms](#coordinate-transforms)
13. [JSON Normalization](#json-normalization)
14. [CLI](#cli)
15. [Custom Properties](#custom-properties)

---

## Quick Start

### Encode GeoJSON to Geobuf

```python
import pybind11_geobuf as gb
from pybind11_geobuf import geojson

# Build a feature in Python
pt = geojson.Point(116.3, 39.9)          # lon, lat
f = geojson.Feature()
f.geometry(pt).properties({"name": "Beijing", "pop": 21_500_000}).id("BJ")

# Encode to protobuf bytes
enc = gb.Encoder()
pbf_bytes = enc.encode(f)

# Decode back
dec = gb.Decoder()
fc_json = dec.decode_to_geojson(pbf_bytes)
print(fc_json.as_feature()())            # -> {"type": "Feature", ...}
```

### File-level API

```python
# Encode a .geojson file to .pbf
enc = gb.Encoder(max_precision=10**7)
enc.encode(geojson="cities.geojson", geobuf="cities.pbf")

# Decode a .pbf file to .geojson
dec = gb.Decoder()
dec.decode(geobuf="cities.pbf", geojson="cities.geojson", indent=True)

# Or use the load/dump shortcut on GeoJSON objects
fc = geojson.FeatureCollection().load("cities.pbf")   # auto-detects .pbf
fc.dump("cities_copy.geojson", indent=True)           # auto-detects .geojson
```

---

## The `__call__()` Pattern — Critical Reading

**This is the most non-obvious aspect of the library.** Every C++ GeoJSON object and `rapidjson` value can be converted to a plain Python object by calling it like a function — `obj()`. Without this call you get a C++ wrapper, not a Python dict/list.

```python
pt = geojson.Point(1.0, 2.0, 3.0)

# This is a C++ Point wrapper:
print(pt)        # <pybind11_geobuf._core.geojson.Point object>

# This is a Python list:
print(pt())      # [1.0, 2.0, 3.0]
```

The same applies to every type:

```python
f = geojson.Feature({"type": "Feature",
                     "geometry": {"type": "Point", "coordinates": [1, 2]},
                     "properties": {"k": 42}})

f()              # -> {"type": "Feature", "geometry": {...}, "properties": {"k": 42}}
f.geometry()()   # -> {"type": "Point", "coordinates": [1.0, 2.0, 0.0]}

props = f.properties()          # C++ PropertyMap (unordered_map<string, value>)
props()                         # NOT available on PropertyMap — see next section
props["k"]                      # C++ geojson.value wrapper
props["k"]()                    # -> 42  (Python int)
```

### The `properties()()` Double Call

`f.properties()` returns the internal C++ property map. Each value in that map is a `geojson.value` object (not a Python object). You must call `()` again on each value to get a Python object:

```python
f = geojson.Feature({"type": "Feature",
                     "geometry": None,
                     "properties": {"name": "Alice", "score": 9.5, "tags": ["a", "b"]}})

props = f.properties()          # C++ PropertyMap

# Wrong — these are still C++ objects:
name = props["name"]            # geojson.value wrapper
score = props["score"]          # geojson.value wrapper

# Correct — call () to get Python values:
name = props["name"]()          # -> "Alice"   (str)
score = props["score"]()        # -> 9.5       (float)
tags = props["tags"]()          # -> ["a", "b"] (list)
```

To get **all** properties as a Python dict at once, convert via `to_rapidjson()` and then call `()`:

```python
props_dict = f.properties().to_rapidjson()()   # -> {"name": "Alice", "score": 9.5, ...}
```

Or use `f()` to get the entire feature as a dict (includes geometry and properties):

```python
feature_dict = f()              # -> {"type": "Feature", "geometry": ..., "properties": {...}}
props_dict = feature_dict["properties"]   # plain Python dict
```

### Summary Table

| Expression | Type | Description |
|---|---|---|
| `f` | `geojson.Feature` | C++ wrapper |
| `f()` | `dict` | Full feature as Python dict |
| `f.geometry()` | `geojson.Geometry` | C++ Geometry wrapper |
| `f.geometry()()` | `dict` | Geometry as Python dict |
| `f.properties()` | `value.object_type` | C++ property map |
| `f.properties()["k"]` | `geojson.value` | C++ value wrapper |
| `f.properties()["k"]()` | Python object | Actual Python value |
| `rj` | `rapidjson` | C++ RapidJSON wrapper |
| `rj["key"]` | `rapidjson` | C++ sub-value |
| `rj["key"]()` | Python object | Python value |
| `rj()` | `dict` / `list` | Full conversion |

---

## GeoJSON Object Model

The library models GeoJSON using the following C++ type hierarchy, exposed to Python under `pybind11_geobuf.geojson`:

```
geojson.GeoJSON         — variant: wraps one of Geometry / Feature / FeatureCollection
  geojson.Geometry      — variant: wraps one of the concrete geometry types below
    geojson.Point
    geojson.MultiPoint
    geojson.LineString
    geojson.MultiLineString
    geojson.LinearRing
    geojson.Polygon
    geojson.MultiPolygon
    geojson.GeometryCollection
  geojson.Feature
  geojson.FeatureCollection
  geojson.value         — variant: null / bool / int / float / str / list / dict
```

You can construct any type directly, from a Python dict, or from a raw JSON string. The library never forces you to build raw dicts — use the typed objects instead.

```python
from pybind11_geobuf import geojson

# Direct construction
pt = geojson.Point(116.3, 39.9, 50.0)         # lon, lat, alt

# From Python dict
ls = geojson.LineString([                       # or as_numpy / from_numpy
    [0.0, 0.0, 0.0],
    [1.0, 1.0, 0.0],
])
# Equivalent:
import numpy as np
ls = geojson.LineString()
ls.from_numpy(np.array([[0, 0, 0], [1, 1, 0]], dtype=float))

# From a JSON file (auto-detects .pbf or .geojson)
fc = geojson.FeatureCollection().load("data.geojson")
fc2 = geojson.FeatureCollection().load("data.pbf")

# GeoJSON variant — used when you don't know the type in advance
g = geojson.GeoJSON().load("data.geojson")
if g.is_feature_collection():
    fc = g.as_feature_collection()
elif g.is_feature():
    f = g.as_feature()
elif g.is_geometry():
    geom = g.as_geometry()
```

---

## Working with Features

### Constructing Features

```python
from pybind11_geobuf import geojson

# Empty feature, then set each part
f = geojson.Feature()
f.geometry(geojson.Point(116.3, 39.9))        # set geometry from a Point
f.properties({"name": "Beijing", "pop": 21_500_000})  # replace all properties
f.id("BJ")                                    # set feature ID

# Fluent chaining (each setter returns self)
f = (geojson.Feature()
     .geometry(geojson.Point(116.3, 39.9))
     .properties({"name": "Beijing"})
     .id("BJ"))

# From a Python dict
f = geojson.Feature({
    "type": "Feature",
    "geometry": {"type": "Point", "coordinates": [116.3, 39.9]},
    "properties": {"name": "Beijing"},
})
```

### Reading Feature Data

```python
# Geometry
geom = f.geometry()                  # geojson.Geometry wrapper
geom_dict = f.geometry()()           # Python dict

# Check geometry type
if f.geometry().is_point():
    pt = f.geometry().as_point()
    x, y, z = pt.x, pt.y, pt.z

# Properties — remember the double-call rule
props = f.properties()               # C++ PropertyMap
for key in props.keys():
    value = props[key]()             # () to get Python value
    print(key, value)

# Get a single property value
name = f.properties("name")          # returns geojson.value | None
if name is not None:
    name = name()                    # -> "Beijing"

# Set a single property
f.properties("population", 21_500_000)
f.properties("deleted_key", None)    # None deletes the key

# Feature ID
fid = f.id()                        # int | float | str | None
```

### Serialization

```python
# To Python dict
d = f()

# To JSON string
import json
json_str = json.dumps(f())

# To Geobuf bytes
pbf = f.to_geobuf(precision=8, only_xy=False)

# To file (auto-detects extension)
f.dump("feature.geojson", indent=True)
f.dump("feature.pbf")
```

### Bounding Box and Numpy Access

```python
# Bounding box [min_lon, min_lat, max_lon, max_lat]
bbox = f.bbox()                      # np.ndarray([4])

# With Z: [min_lon, min_lat, min_alt, max_lon, max_lat, max_alt]
bbox6 = f.bbox(with_z=True)          # np.ndarray([6])

# Mutable numpy view of coordinates (modifying this changes the C++ data)
coords = f.as_numpy()               # np.ndarray shape (N, 3), writable
coords[:, 2] += 10.0                # shift all Z values in-place

# Copy of coordinates (safe to modify without affecting C++ data)
coords_copy = f.to_numpy()
```

---

## Working with FeatureCollections

### Construction and Access

```python
from pybind11_geobuf import geojson
import numpy as np

fc = geojson.FeatureCollection()

# Append features
f1 = geojson.Feature().geometry(geojson.Point(0, 0)).properties({"id": 1})
f2 = geojson.Feature().geometry(geojson.Point(1, 1)).properties({"id": 2})
fc.append(f1)
fc.append(f2)

# Index and iteration
print(len(fc))               # 2
first = fc[0]                # geojson.Feature
for f in fc:
    props = f.properties()
    print(props["id"]())     # remember () to get Python value

# Slicing returns a new FeatureCollection
sub = fc[0:5]
del fc[:2]                   # delete first two features
```

### Loading and Saving

```python
fc = geojson.FeatureCollection().load("input.geojson")
fc = geojson.FeatureCollection().load("input.pbf")    # from Geobuf

fc.dump("output.geojson", indent=True, sort_keys=True)
fc.dump("output.pbf", precision=8)

# Convert to Python dict
fc_dict = fc()               # {"type": "FeatureCollection", "features": [...]}
```

### Bulk Operations

```python
# Get all feature IDs (reading properties)
ids = [f.id() for f in fc]

# Filter by property value
results = geojson.FeatureCollection()
for f in fc:
    props = f.properties()
    if "type" in props and props["type"]() == "road":
        results.append(f)

# Collect property values into a list
names = [f.properties()["name"]() for f in fc if "name" in f.properties()]
```

---

## Geometry Types

All geometry types live in `pybind11_geobuf.geojson`. They share a common interface: `__call__()`, `as_numpy()`, `to_numpy()`, `from_numpy()`, transform methods, `bbox()`, `clone()`, and `__eq__`.

### Point

```python
from pybind11_geobuf import geojson

pt = geojson.Point(116.3, 39.9)          # Z defaults to 0
pt = geojson.Point(116.3, 39.9, 50.0)   # with altitude

pt.x, pt.y, pt.z                         # read coordinates
pt.x = 116.4                             # write coordinates
pt[0], pt[1], pt[2]                      # index access

pt()                                     # -> [116.3, 39.9, 50.0]

arr = pt.to_numpy()                      # shape (3,), copy
view = pt.as_numpy()                     # shape (3,), mutable view
```

### LineString and MultiPoint

These share the same interface — a sequence of Points.

```python
ls = geojson.LineString()
ls.push_back(geojson.Point(0, 0))
ls.push_back(geojson.Point(1, 1))
ls.push_back([2.0, 2.0, 0.0])           # array-like also accepted

len(ls)                                  # 3
ls[0]                                    # geojson.Point
ls[0]()                                  # [0.0, 0.0, 0.0]

ls()                    # -> [[0.0, 0.0, 0.0], [1.0, 1.0, 0.0], [2.0, 2.0, 0.0]]

# Efficient numpy access
import numpy as np
coords = np.array([[0, 0, 0], [1, 1, 0], [2, 2, 0]], dtype=float)
ls = geojson.LineString()
ls.from_numpy(coords)

view = ls.as_numpy()       # shape (N, 3), mutable — changes reflect in C++
copy = ls.to_numpy()       # shape (N, 3), safe copy
view[0, 2] = 100.0         # this modifies the C++ LineString directly
```

### Polygon

```python
ring = np.array([[0,0,0], [1,0,0], [1,1,0], [0,1,0], [0,0,0]], dtype=float)
poly = geojson.Polygon()
poly.push_back(ring)       # exterior ring
poly.push_back(hole)       # interior ring (hole)

poly[0]                    # geojson.LinearRing (first ring)
poly[0].as_numpy()         # Nx3 mutable view of ring coordinates
```

### Checking and Unwrapping via `Geometry`

`geojson.Geometry` is a variant — it can hold any of the concrete types.

```python
g = geojson.Geometry(geojson.Point(1, 2))

g.type()                  # "Point"
g.is_point()              # True
g.is_line_string()        # False

pt = g.as_point()         # geojson.Point

# From dict
g = geojson.Geometry({"type": "LineString", "coordinates": [[0,0], [1,1]]})
ls = g.as_line_string()
```

### Deduplication and `bbox`

```python
ls.deduplicate_xyz()       # removes consecutive duplicate points, returns bool (modified?)
ls.bbox()                  # [min_lon, min_lat, max_lon, max_lat]
ls.bbox(with_z=True)       # [min_lon, min_lat, min_alt, max_lon, max_lat, max_alt]
```

---

## Encoder

`Encoder` converts GeoJSON objects or files to compact Geobuf (Protocol Buffer) binary format.

### Configuration

```python
import pybind11_geobuf as gb

enc = gb.Encoder(
    max_precision=10**6,    # default: 10^6 → 6 decimal places for lon/lat
    only_xy=False,          # if True, drop Z coordinate entirely
    round_z=None,           # round Z to N decimal places before encoding (e.g. round_z=2)
)
```

`max_precision` is a multiplier, not a digit count:
- `10**6` → 6 decimal places (default, ~0.1 m precision)
- `10**7` → 7 decimal places (~1 cm precision)
- `10**8` → 8 decimal places (~1 mm precision)

### Encoding

```python
from pybind11_geobuf import geojson

fc = geojson.FeatureCollection().load("data.geojson")

# Encode a FeatureCollection
pbf_bytes: bytes = enc.encode(fc)

# Encode a single Feature
f = fc[0]
pbf_bytes = enc.encode(f)

# Encode a Geometry
geom = f.geometry()
pbf_bytes = enc.encode(geom)

# Encode from a Python dict (will be converted internally)
pbf_bytes = enc.encode({"type": "FeatureCollection", "features": []})

# File to file (most efficient for large data)
enc.encode(geojson="input.geojson", geobuf="output.pbf")
```

### Inspecting After Encode

```python
enc.dim()            # 2 or 3 (detected from data)
enc.e()              # actual precision multiplier used
enc.max_precision()  # the configured max_precision
enc.keys()           # dict[str, int] — property key to index mapping
```

---

## Decoder

`Decoder` converts Geobuf bytes back to GeoJSON objects, strings, or files. It also supports partial decoding.

### Basic Decoding

```python
import pybind11_geobuf as gb

dec = gb.Decoder()

# Decode to a GeoJSON variant object (recommended)
g = dec.decode_to_geojson(pbf_bytes)        # geojson.GeoJSON
if g.is_feature_collection():
    fc = g.as_feature_collection()

# Decode to a JSON string
json_str = dec.decode(pbf_bytes)
json_str = dec.decode(pbf_bytes, indent=True, sort_keys=True)

# File to file
dec.decode(geobuf="input.pbf", geojson="output.geojson", indent=True)

# Decode to rapidjson object
rj = dec.decode_to_rapidjson(pbf_bytes)
rj()                                        # -> Python dict
```

### Inspecting the Decoded State

After any decode call, the decoder retains state about the file:

```python
dec.precision()       # int — coordinate precision (e.g. 6 means 10^6)
dec.dim()             # 2 or 3 — coordinate dimension
dec.keys()            # list[str] — property key names in index order
dec.offsets()         # list[int] — byte offsets for each feature in the stream
```

### Partial Decoding

You can decode a raw byte slice for a single feature (useful when combined with `GeobufIndex`):

```python
# Decode only header (populates keys, dim, precision)
dec.decode_header(pbf_bytes)

# Decode a single feature from raw bytes
f = dec.decode_feature(feature_bytes)
f = dec.decode_feature(feature_bytes, only_geometry=True)    # skip properties
f = dec.decode_feature(feature_bytes, only_properties=True)  # skip geometry

# Decode non-feature data (FeatureCollection-level custom properties)
v = dec.decode_non_features(trailing_bytes)
v()                   # -> Python dict
```

---

## GeobufIndex — Large File Random Access

For large `.pbf` files with millions of features, loading the entire file is impractical. `GeobufIndex` enables:
- Memory-mapped access (no full file load)
- Random access to individual features by integer index or string ID
- Spatial queries via a packed R-tree

### Building an Index

First, create an index file from a geobuf file:

```python
import pybind11_geobuf as gb

# Build index alongside the geobuf file
gb.GeobufIndex.indexing(
    "cities.pbf",              # input geobuf
    "cities.pbf.index",        # output index file
    feature_id="@",            # "@" = auto-detect: uses feature.id, or falls back to
                               #   "id" / "feature_id" / "fid" property keys
    packed_rtree="@",          # "@" = build R-tree (one AABB per feature)
                               # "per_line_segment" = one AABB per line segment
)
```

The index file is small compared to the geobuf — it contains offsets, feature IDs, and the packed R-tree. Store it next to the `.pbf` file.

### Opening and Querying

```python
idx = gb.GeobufIndex()

# Two-step: load index, then memory-map the geobuf file
idx.mmap_init("cities.pbf.index", "cities.pbf")

# Inspect the index
idx.num_features          # total feature count
idx.header_size           # bytes in PBF header
idx.offsets               # list of byte offsets (length = num_features + 2)
idx.ids                   # dict[str, int] | None — feature_id → feature index
```

### Random Access by Index

```python
# Decode a single feature (loads only that slice of the file)
f = idx.decode_feature(42)                           # feature at index 42
f = idx.decode_feature(42, only_geometry=True)       # skip properties
f = idx.decode_feature(42, only_properties=True)     # skip geometry

# Decode a batch of features
fc = idx.decode_features([0, 5, 100, 999])           # geojson.FeatureCollection
```

### Random Access by Feature ID

If you built the index with `feature_id="@"`, you can look up features by their string ID:

```python
f = idx.decode_feature_of_id("BJ")                  # geojson.Feature | None
f = idx.decode_feature_of_id("BJ", only_geometry=True)
```

### Spatial Query

Returns a set of feature indices whose bounding boxes overlap the query box:

```python
import numpy as np

# Query features in a bounding box [lon_min, lat_min] to [lon_max, lat_max]
indices = idx.query(
    np.array([116.0, 39.5]),    # min corner (lon, lat)
    np.array([117.0, 40.5]),    # max corner (lon, lat)
)
# indices is a set[int]

# Decode only those features
fc = idx.decode_features(sorted(indices))
```

### FeatureCollection-level Custom Properties

```python
# Decode non-feature data (trailing bytes after all features)
meta = idx.decode_non_features()   # geojson.value
meta()                             # -> Python dict (e.g. {"source": "OSM", "date": "2024"})
```

### Typical Workflow

```python
# One-time: build index
gb.GeobufIndex.indexing("large.pbf", "large.pbf.index")

# At query time
idx = gb.GeobufIndex()
idx.mmap_init("large.pbf.index", "large.pbf")

# Get features in a region
hits = idx.query(np.array([min_lon, min_lat]), np.array([max_lon, max_lat]))
fc = idx.decode_features(sorted(hits))
print(len(fc), "features found")
```

---

## Planet — In-Memory Spatial Index

`Planet` wraps a `FeatureCollection` with a built-in packed R-tree. Use it when the entire dataset fits in memory but you need repeated spatial queries.

```python
import pybind11_geobuf as gb
from pybind11_geobuf import geojson
import numpy as np

# Load
fc = geojson.FeatureCollection().load("roads.pbf")
planet = gb.Planet(fc)

# Build R-tree (lazy — also triggered automatically on first query)
planet.build()
planet.build(per_line_segment=True)  # separate AABB per line segment (for long roads)

# Spatial query — returns int32 numpy array of feature indices
indices = planet.query(
    np.array([116.0, 39.5]),   # min corner (lon, lat)
    np.array([117.0, 40.5]),   # max corner (lon, lat)
)

# Extract matching features
result_fc = planet.copy(indices)          # geojson.FeatureCollection
```

### Polygon Clipping

```python
import numpy as np

# Define a clipping polygon (Nx2, lon/lat)
polygon = np.array([
    [116.0, 39.5],
    [117.0, 39.5],
    [117.0, 40.5],
    [116.0, 40.5],
    [116.0, 39.5],
], dtype=float)

clipped = planet.crop(
    polygon,
    clipping_mode="longest",     # keep only the longest segment of each clipped linestring
    strip_properties=False,      # if True, replace properties with {"index": original_index}
    is_wgs84=True,               # use geodesic calculations
)
# clipped is a geojson.FeatureCollection

# Clipping modes:
# "longest" — for each feature, keep only the longest clipped segment
# "first"   — keep only the first clipped segment
# "all"     — return all segments as separate features
# "whole"   — keep entire feature if its centroid is inside the polygon
```

### Accessing and Replacing Features

```python
# Access the underlying FeatureCollection
fc = planet.features()           # reference to internal FC — modifying it modifies Planet

# Replace with a new FeatureCollection (resets R-tree)
planet.features(new_fc)
planet.build(force=True)         # rebuild after replacement
```

---

## rapidjson

`pybind11_geobuf.rapidjson` is a Python binding for the [RapidJSON](https://rapidjson.org/) C++ value type. It provides a fast, dict/list-compatible JSON value that interoperates with all GeoJSON types.

You'll encounter it when you need to:
- Load/save raw JSON without going through the GeoJSON type system
- Normalize or round JSON values in-place
- Use `to_rapidjson()` / `from_rapidjson()` bridges on GeoJSON objects

### Basic Usage

```python
from pybind11_geobuf import rapidjson

# Create from Python object
rj = rapidjson({"type": "FeatureCollection", "features": []})
rj = rapidjson([1, 2, 3])
rj = rapidjson("hello")

# The () operator converts to Python
rj()                    # -> Python dict/list/primitive

# Load from file / string
rj = rapidjson().load("data.json")
rj = rapidjson().loads('{"x": 1}')

# Save
rj.dump("out.json", indent=True, sort_keys=True)
s = rj.dumps(indent=True)
```

### Dict/Array Interface

```python
rj = rapidjson({"a": 1, "b": [10, 20, 30]})

# Read
rj["a"]            # rapidjson wrapper
rj["a"]()          # -> 1 (Python int)
rj["b"][0]()       # -> 10
"a" in rj          # True

# Write
rj["c"] = 42
rj["b"].push_back(40)
del rj["a"]

# Keys and values
rj.keys()          # list[str]
rj.values()        # list[rapidjson]

# Get returns None if missing (won't raise KeyError)
rj.get("missing")  # -> None
```

### Normalization and Rounding

`rapidjson` exposes in-place normalization methods that are commonly used to clean up GeoJSON before encoding or comparing:

```python
rj = rapidjson().load("data.geojson")

# Sort all object keys recursively (for deterministic output)
rj.sort_keys()

# Round all coordinates: [lon_precision, lat_precision, alt_precision]
rj.round_geojson_geometry(precision=[8, 8, 3])

# Round non-geometry numeric values in GeoJSON (e.g. speed, score)
rj.round_geojson_non_geometry(precision=3)

# Convert float values that are integers (e.g. 1.0 → 1)
rj.denoise_double_0()

# Remove Z=0 from 2D-only coordinates
rj.strip_geometry_z_0()

# All of the above in one call
rj.normalize(
    sort_keys=True,
    round_geojson_geometry=[8, 8, 3],
    round_geojson_non_geometry=3,
    denoise_double_0=True,
    strip_geometry_z_0=True,
)

# Check for NaN/Inf (returns path string like "features[0].geometry.coordinates[2]", or None)
path = rj.locate_nan_inf()

# Subset check
is_subset = rj.is_subset_of(other_rj)
```

### Bridge with GeoJSON Types

All GeoJSON objects have `to_rapidjson()` and `from_rapidjson()`:

```python
fc = geojson.FeatureCollection().load("data.geojson")

# Convert to rapidjson for manipulation
rj = fc.to_rapidjson()
rj.normalize()
rj.dump("normalized.geojson", indent=True)

# Or convert back to typed objects
fc2 = geojson.FeatureCollection()
fc2.from_rapidjson(rj)
```

---

## Coordinate Transforms

All GeoJSON objects support in-place coordinate transforms. Every transform method returns `self` (fluent interface).

### Built-in Transforms

```python
import numpy as np
from pybind11_geobuf import geojson

anchor = np.array([116.3, 39.9, 0.0])   # anchor point in WGS84 (lon, lat, alt)

fc = geojson.FeatureCollection().load("data.geojson")

# WGS84 → local ENU (meters, East/North/Up from anchor)
fc.to_enu(anchor)
fc.to_enu(anchor, cheap_ruler=True)      # fast approximation (default)
fc.to_enu(anchor, cheap_ruler=False)     # rigorous ECEF transform

# ENU → WGS84
fc.to_wgs84(anchor)

# Affine transform (4x4 matrix)
T = np.eye(4)
fc.affine(T)

# Translate / scale / rotate
fc.translate(np.array([100.0, 0.0, 0.0]))
fc.scale(np.array([2.0, 2.0, 1.0]))

R = np.eye(3)  # 3x3 rotation matrix
fc.rotate(R)

# Round coordinates
fc.round(lon=8, lat=8, alt=3)

# Remove consecutive duplicate points
fc.deduplicate_xyz()
```

These methods are available on `GeoJSON`, `Geometry`, all concrete geometry types, `Feature`, and `FeatureCollection`.

### Custom Transform Function

Use `transform(fn)` to apply any custom per-feature coordinate transformation:

```python
def my_transform(coords: np.ndarray) -> np.ndarray | None:
    # coords is an Nx3 numpy array (read-write view)
    # Option 1: modify in-place and return None
    coords[:, 2] += 100.0
    # Option 2: return a new array
    # return coords + 1.0

fc.transform(my_transform)
```

### The `tf` Submodule

Low-level coordinate conversion functions are available in `pybind11_geobuf.tf`:

```python
from pybind11_geobuf import tf
import numpy as np

anchor_lla = np.array([116.3, 39.9, 50.0])   # lon, lat, alt

# Single point: LLA → ENU
enu = tf.lla2enu(np.array([[116.4, 40.0, 50.0]]), anchor_lla=anchor_lla)
# enu.shape == (1, 3)

# Batch: ENU → LLA
llas = tf.enu2lla(enu_array, anchor_lla=anchor_lla)

# ECEF conversions
ecef = tf.lla2ecef(116.3, 39.9, 0.0)          # single point
ecef = tf.lla2ecef(lla_array)                  # batch
lla  = tf.ecef2lla(ecef_array)                 # batch

# Rotation matrix: ECEF ↔ ENU at given lon/lat
R = tf.R_ecef_enu(116.3, 39.9)               # 3x3 numpy array

# 4×4 homogeneous transform ECEF ↔ ENU
T = tf.T_ecef_enu(116.3, 39.9, 0.0)         # 4x4 numpy array

# cheap_ruler factor at a latitude (for fast lat/lon → meters conversion)
k = tf.cheap_ruler_k(39.9)                   # shape (3,): [meters_per_lon, meters_per_lat, 1.0]
dlon = 1.0 / k[0]                            # degrees of longitude per meter at this latitude
dlat = 1.0 / k[1]                            # degrees of latitude per meter

# Apply a 4x4 transform to an Nx3 array
out = tf.apply_transform(T, coords)
tf.apply_transform_inplace(T, coords, batch_size=1000)
```

---

## JSON Normalization

`normalize_json` standardizes GeoJSON or plain JSON for consistent serialization and comparison.

### File-Level Normalization

```python
import pybind11_geobuf as gb

# Normalize in place (read → transform → write)
gb.normalize_json(
    "input.geojson",
    "output.geojson",
    indent=True,                         # pretty-print output
    sort_keys=True,                      # sort object keys recursively
    denoise_double_0=True,               # 1.0 → 1, 0.0 → 0
    strip_geometry_z_0=True,             # [lon, lat, 0.0] → [lon, lat]
    round_non_geojson=3,                 # round non-GeoJSON numbers to 3 dp
    round_geojson_non_geometry=3,        # round non-coordinate GeoJSON numbers to 3 dp
    round_geojson_geometry=[8, 8, 3],    # round [lon, lat, alt] to [8, 8, 3] dp
)
```

### In-Memory Normalization

```python
rj = gb.rapidjson().load("data.geojson")

gb.normalize_json(
    rj,
    sort_keys=True,
    round_geojson_geometry=[8, 8, 3],
)
rj.dump("normalized.geojson", indent=True)
```

### Subset Check

```python
# Check if all keys/values in file1 also exist with same values in file2
result = gb.is_subset_of("subset.geojson", "superset.geojson")   # bool
```

### PBF Debug Inspection

```python
raw = open("data.pbf", "rb").read()
print(gb.pbf_decode(raw, indent="  "))   # human-readable protobuf representation
```

---

## CLI

```bash
# GeoJSON → Geobuf
python -m pybind11_geobuf json2geobuf input.geojson output.pbf
python -m pybind11_geobuf json2geobuf input.geojson output.pbf --precision=7 --only-xy

# Geobuf → GeoJSON
python -m pybind11_geobuf geobuf2json input.pbf output.geojson --indent --sort-keys

# Normalize a GeoJSON file
python -m pybind11_geobuf normalize_json input.geojson output.geojson
python -m pybind11_geobuf normalize_json input.geojson input.geojson  # in-place

# Normalize a Geobuf file (decode → normalize → re-encode)
python -m pybind11_geobuf normalize_geobuf input.pbf --precision=8

# Inspect raw protobuf bytes
python -m pybind11_geobuf pbf_decode data.pbf

# Check if one JSON file is a subset of another
python -m pybind11_geobuf is_subset_of a.geojson b.geojson

# Roundtrip test (encode → decode, compare result)
python -m pybind11_geobuf round_trip data.geojson --precision=8

# Build a spatial index for a geobuf file
python -m pybind11_geobuf index_geobuf input.pbf output.pbf.index
python -m pybind11_geobuf index_geobuf input.pbf output.pbf.index --feature-id=@ --packed-rtree=@
```

---

## Custom Properties

Custom properties are metadata attached to geometry objects and FeatureCollections (not the same as feature `properties`). They survive encoding/decoding and are stored in the protobuf.

```python
from pybind11_geobuf import geojson

# On a FeatureCollection
fc = geojson.FeatureCollection().load("data.geojson")
cp = fc.custom_properties()     # geojson.value.object_type (C++ map)
cp["source"] = "OSM"            # set via [] operator
cp["date"] = "2024-01"
cp["source"]()                  # -> "OSM"

# Equivalent via __getitem__/__setitem__ on the FeatureCollection directly
fc["source"] = "OSM"
val = fc["source"]()            # -> "OSM"

# On a Geometry
geom = geojson.Geometry(geojson.LineString())
geom["road_type"] = "highway"   # stored in custom_properties
geom["road_type"]()             # -> "highway"

# Note: geometry custom_properties ≠ feature properties
# f.properties() is for GeoJSON-spec Feature properties (in "properties" key)
# f.custom_properties() is for extra metadata outside the GeoJSON spec
```

### Difference: `feature.properties()` vs `feature.custom_properties()`

| | `feature.properties()` | `feature.custom_properties()` |
|---|---|---|
| GeoJSON spec | Yes — appears under `"properties"` key | No — stored outside the spec |
| Encoded in PBF | Yes | Yes |
| Accessed via `f[key]` | No — `f[key]` goes to custom_properties | Yes |
| Typical use | Feature attributes (name, id, type...) | Internal metadata (source, version...) |

```python
# properties() — the standard GeoJSON properties dict
f.properties()["name"]()         # -> "Beijing"

# custom_properties() — additional metadata, accessed via f[key]
f["internal_id"] = 42
f["internal_id"]()               # -> 42
```

---

## Quick Reference

### Common Patterns

```python
# Load any GeoJSON or Geobuf file
fc = geojson.FeatureCollection().load("data.pbf")       # or .geojson

# Iterate features, access properties safely
for f in fc:
    props = f.properties()
    if "name" not in props:
        continue
    name = props["name"]()                   # () is mandatory!
    fid = f.id()                             # int | float | str | None

# Encode to Geobuf
enc = gb.Encoder(max_precision=10**7)
pbf = enc.encode(fc)
with open("out.pbf", "wb") as fh:
    fh.write(pbf)

# Spatial query (large file)
idx = gb.GeobufIndex()
idx.mmap_init("large.pbf.index", "large.pbf")
hits = idx.query(np.array([lon0, lat0]), np.array([lon1, lat1]))
fc = idx.decode_features(sorted(hits))

# Spatial query (in-memory)
planet = gb.Planet(fc)
indices = planet.query(np.array([lon0, lat0]), np.array([lon1, lat1]))
subset = planet.copy(indices)

# Coordinate transform
anchor = np.array([116.3, 39.9, 0.0])
fc_enu = fc.clone().to_enu(anchor)           # clone to avoid modifying original

# Normalize and save
rj = fc.to_rapidjson()
rj.normalize().dump("normalized.geojson", indent=True)
```

### Converting C++ Objects to Python

```python
# Always use () to get a plain Python object:
pt()                          # Point → list
ls()                          # LineString → list of lists
poly()                        # Polygon → list of list of lists
f()                           # Feature → dict
fc()                          # FeatureCollection → dict
geom()                        # Geometry → dict
g()                           # GeoJSON variant → dict

# For individual property values:
f.properties()["key"]()       # geojson.value → Python scalar/list/dict

# For the whole properties dict:
f.properties().to_rapidjson()()   # -> Python dict
# or simply:
f()["properties"]                 # -> Python dict (from the full feature dict)
```

# geobuf

<!--intro-start-->

C++ port of <https://github.com/mapbox/geobuf>,
and with python binding.

## Python binding

Install

```bash
# from pypi
pip install -U pybind11_geobuf

# from source
git clone --recursive https://github.com/cubao/geobuf-cpp
pip install ./geobuf-cpp

# or just
pip install git+https://github.com/cubao/geobuf-cpp.git
```

(you can build wheels for later reuse by ` pip wheel git+https://github.com/cubao/geobuf-cpp.git`)

See `tests/test_geobuf.py` for usage.

## Dependencies

All dependencies are header-only, including:

-   [`rapidjson`](https://github.com/Tencent/rapidjson) for JSON read/write
-   [`geojson-cpp`](https://github.com/district10/geojson-cpp) for GeoJSON representation
    -   dependencies
        -   [`variant`](https://github.com/mapbox/variant)
        -   [`geometry.hpp`](https://github.com/district10/geometry.hpp)
    -   forked from mapbox, with some modifications to `geojson-cpp` and `geometry.hpp`
        -   added `z` to mapbox::geojson::point
        -   added `custom_properties` to geometry/feature/feature_collection
-   [`protozero`](https://github.com/mapbox/protozero) for protobuf encoding/decoding

*[`dbg-macro`](https://github.com/sharkdp/dbg-macro) and [`doctest`](https://github.com/onqtam/doctest) are dev dependencies.*

Simple roundtrip tests pass, have identical results to JS implementation.

<!--intro-end-->

## Development

pull all code:

```bash
git submodule update --init --recursive
```

compile & test:

```bash
make build
make test_all

make roundtrip_test_js
make roundtrip_test_cpp
```

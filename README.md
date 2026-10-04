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

See [introduction.md](introduction.md) for a user guide and `tests/test_geobuf.py` for usage.

### in the browser (pyodide / wasm)

A pyodide wheel is built by the `Wheel on pyodide` job in
[`.github/workflows/wheels.yml`](.github/workflows/wheels.yml) and published to
PyPI together with the other wheels. Inside pyodide:

```js
const pyodide = await loadPyodide();
await pyodide.loadPackage(["numpy", "micropip"]);
const micropip = pyodide.pyimport("micropip");
await micropip.install("pybind11-geobuf"); // or a local wheel: "./pybind11_geobuf-...wasm32.whl"
const geobuf = pyodide.pyimport("pybind11_geobuf");
```

To build and test locally:

```bash
make pyodide_install   # pip install pyodide-build
make pyodide_wheel     # -> dist/*wasm32.whl
make pyodide_web       # builds, writes tests/pyodide/wheels.json, serves :8123
# open http://localhost:8123/tests/pyodide/index.html
```

Each wheel is ABI-tagged for one pyodide runtime (`pyemscripten_2024_0_wasm32` …)
and the page's pyodide version has to match that tag. CI builds one wheel per
supported pyodide version; locally `pyodide build` picks the xbuildenv whose
CPython matches your host interpreter, so a Python 3.12 host gets pyodide 0.27.x
(ABI 2024_0). Use `pyodide xbuildenv install <version> --force` to target
another version — `gen_wheels_json.py` then writes it into `wheels.json`, and
`?pyodide_version=` overrides it in the browser.

[`tests/pyodide/index.html`](tests/pyodide/index.html) loads pyodide (by default
from the jsdelivr CDN), installs the wheel from `dist/` and runs `tests/` in the
browser (the test files and `data/` are fetched into the pyodide filesystem). If
the CDN is slow, mirror the runtime once and the page picks it up automatically:

```bash
python tests/pyodide/fetch_pyodide_dist.py --version 0.27.8
```

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

install deps:

```
npm i -g geobuf
python3 -m pip install geobuf
```

compile & test:

```bash
make build
make test_all

make roundtrip_test_js
make roundtrip_test_cpp
```

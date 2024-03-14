# Release Notes

---

## Upgrading

To upgrade `pybind11-geobuf` to the latest version, use pip:

```bash
pip install -U pybind11-geobuf
```

## Version 0.2.1 (2024-03-14)

*   Export PackedRTree

## Version 0.2.0 (2023-11-18)

*   Indexing geobuf in protobuf format; Spec: geobuf_index.proto

## Version 0.1.9 (2023-11-15)

*   Indexing geobuf (like flatgeobuf, but more general), making it random accessible

## Version 0.1.8 (2023-11-11)

*   Fix readthedocs
*   Full support of geobuf Feature id
*   Fix geojson value uint64_t/int64_t
*   Misc updates

## Version 0.1.7 (2023-11-11)

*   Integrate PackedRTree (rbush)

## Version 0.1.6 (2023-07-02)

*   Crop geojson features by polygon (alpha release)

## Version 0.1.5 (2023-06-02)

*   Add `round_non_geojson` to `normalize_json`
*   Handle NaN, Inf in json, add `locate_nan_inf`

## Version 0.1.4 (2023-04-15)

*   More options to normalize_json
*   Add `is_subset_of(json1, json2)`

## Version 0.1.3 (2023-04-11)

*   Fix round geojson-non-geometry

## Version 0.1.2 (2023-04-10)

*   Fix round geojson-non-geometry, should be recursive

## Version 0.1.1 (2023-04-09)

*   benchmark (speed: cpp > js > python)
*   normalize json round geometry/non-geometry parts
*   sort keys in encoding geobuf

## Version 0.1.0 (2023-04-01)

*   round z in geobuf encoding
*   round rapidjson

## Version 0.0.9 (2023-03-30)

*   Export to geobuf with only xy (no z), `only_xy=False`
*   Geobuf property keys are sorted now!

## Version 0.0.8 (2023-03-28)

*   Integrate geobuf into geojson!

## Version 0.0.7 (2023-03-26)

*   More bindings, more tests

## Version 0.0.6 (2023-03-25)

*   More bindings, more tests
*   Some fixes

## Version 0.0.5 (2023-03-08)

*   Add windows version
*   Add cli interface

## Version 0.0.4 (2023-03-04)

*   Use GitHub workflow to release to pypi
*   Setup CI

## Version 0.0.3 (2023-02-02)

*   Export `normalize_json`

## Version 0.0.2 (2023-01-24)

*   More bindings, more tests

## Version 0.0.1 (2023-01-14)

*   Add python binding, release to pypi

---

You can also checkout releases on:

-   GitHub: <https://github.com/cubao/geobuf-cpp/releases>
-   PyPi: <https://pypi.org/project/pybind11-geobuf>

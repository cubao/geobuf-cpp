# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++ port of [mapbox/geobuf](https://github.com/mapbox/geobuf) with Python bindings via pybind11. Geobuf is a compact binary encoding for GeoJSON using Protocol Buffers.

## Build Commands

```bash
# Initialize submodules (required first time)
git submodule update --init --recursive

# Build Python extension (editable install)
make build

# Run all C++ tests
make test_all

# Run Python tests
make pytest
# Or directly: pytest tests/test_basic.py

# Lint code
make lint

# Roundtrip tests (compare C++ vs JS implementations)
make roundtrip_test_cpp
make roundtrip_test_js

# CLI tests
make cli_test
```

## Architecture

### Core C++ Library (`src/geobuf/`)

- **geobuf.hpp/cpp**: Main `Encoder` and `Decoder` classes for GeoJSON ↔ Geobuf (protobuf) conversion
- **geobuf_index.hpp**: `GeobufIndex` class for spatial indexing and random access to features in large Geobuf files using memory-mapped I/O and packed R-tree
- **planet.hpp**: `Planet` class wrapping feature collections with spatial query support via `PackedRTree`
- **geojson_helpers.hpp**: JSON normalization utilities (rounding, sorting keys, denoising)
- **rapidjson_helpers.hpp**: RapidJSON wrapper utilities

### Python Bindings (`src/`)

- **main.cpp**: pybind11 module definition exposing `Encoder`, `Decoder`, `GeobufIndex`, `Planet`, `PackedRTree`
- **pybind11_geojson.cpp**: Bindings for mapbox::geojson types (Point, LineString, Polygon, Feature, FeatureCollection)
- **pybind11_rapidjson.cpp**: Bindings for RapidJSON value types

### Key Dependencies (all header-only, in submodules)

- `rapidjson`: JSON parsing/serialization
- `geojson-cpp` (forked): GeoJSON representation with Z-coordinate and custom_properties support
- `protozero`: Protocol Buffer encoding/decoding
- `geometry.hpp` (forked): Geometry types

### Python Package (`src/pybind11_geobuf/`)

- CLI via `python -m pybind11_geobuf` with commands: `json2geobuf`, `geobuf2json`, `pbf_decode`, `normalize_json`, `round_trip`, `is_subset_of`

## Key Classes

- `mapbox::geobuf::Encoder`: Encodes GeoJSON to compact Geobuf format with configurable precision
- `mapbox::geobuf::Decoder`: Decodes Geobuf back to GeoJSON, supports partial decoding (header, individual features)
- `cubao::GeobufIndex`: Enables random access to features in large Geobuf files without loading entire file
- `cubao::Planet`: Feature collection with built-in spatial index for bounding box queries

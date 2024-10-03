#include <mapbox/geojson.hpp>

#include <pybind11/eigen.h>
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "geobuf/geojson_cropping.hpp"
#include "geobuf/geojson_helpers.hpp"
#include "geobuf/pybind11_helpers.hpp"
#include "geobuf/rapidjson_helpers.hpp"

// #define PYBIND11_GEOJSON_WITH_GEOBUF
// #if PYBIND11_GEOJSON_WITH_GEOBUF
// #endif
#include "geobuf.hpp"

#include <fstream>
#include <iostream>

// https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html?highlight=stl#making-opaque-types
PYBIND11_MAKE_OPAQUE(mapbox::geojson::geometry_collection::container_type);
PYBIND11_MAKE_OPAQUE(mapbox::geojson::multi_line_string::container_type);
PYBIND11_MAKE_OPAQUE(mapbox::geojson::multi_point::container_type);
PYBIND11_MAKE_OPAQUE(mapbox::geojson::multi_polygon::container_type);
PYBIND11_MAKE_OPAQUE(mapbox::geojson::polygon::container_type);
PYBIND11_MAKE_OPAQUE(mapbox::geojson::value::array_type);
PYBIND11_MAKE_OPAQUE(mapbox::geojson::value::object_type);
PYBIND11_MAKE_OPAQUE(std::vector<mapbox::geojson::feature>);

namespace cubao
{
namespace py = pybind11;
using namespace pybind11::literals;
using rvp = py::return_value_policy;

using PropertyMap = mapbox::geojson::value::object_type;

template <typename T> Eigen::VectorXd geom2bbox(const T &t, bool with_z)
{
    auto bbox = mapbox::geometry::envelope(t);
    if (with_z) {
        return (Eigen::VectorXd(6) << //
                    bbox.min.x,
                bbox.min.y, bbox.min.z, //
                bbox.max.x, bbox.max.y, bbox.max.z)
            .finished();
    } else {
        return (Eigen::VectorXd(4) << //
                    bbox.min.x,
                bbox.min.y, //
                bbox.max.x, bbox.max.y)
            .finished();
    }
}

inline bool endswith(const std::string &text, const std::string &suffix)
{
    if (text.length() < suffix.length()) {
        return false;
    }
    return 0 == text.compare(text.length() - suffix.length(), suffix.length(),
                             suffix);
}

void bind_geojson(py::module &geojson)
{
#define is_geojson_type(geojson_type)                                          \
    .def(                                                                      \
        "is_" #geojson_type,                                                   \
        [](const mapbox::geojson::geojson &self) {                             \
            return self.is<mapbox::geojson::geojson_type>();                   \
        },                                                                     \
        "Check if this GeoJSON object is of type " #geojson_type)

#define as_geojson_type(geojson_type)                                          \
    .def(                                                                      \
        "as_" #geojson_type,                                                   \
        [](mapbox::geojson::geojson &self)                                     \
            -> mapbox::geojson::geojson_type & {                               \
            return self.get<mapbox::geojson::geojson_type>();                  \
        },                                                                     \
        rvp::reference_internal,                                               \
        "Get this GeoJSON object as a " #geojson_type " (if it is one)")

#define GEOMETRY_DEDUPLICATE_XYZ(geom_type)                                    \
    .def(                                                                      \
        "deduplicate_xyz",                                                     \
        [](mapbox::geojson::geom_type &self) {                                 \
            return deduplicate_xyz(self);                                      \
        },                                                                     \
        "Remove duplicate consecutive points based on their XYZ coordinates")

#define copy_deepcopy_clone(Type)                                              \
    .def(                                                                      \
        "__copy__", [](const Type &self, py::dict) -> Type { return self; },   \
        "Create a shallow copy of the object")                                 \
        .def(                                                                  \
            "__deepcopy__",                                                    \
            [](const Type &self, py::dict) -> Type { return self; }, "memo"_a, \
            "Create a deep copy of the object")                                \
        .def(                                                                  \
            "clone", [](const Type &self) -> Type { return self; },            \
            "Create a clone of the object")

    py::class_<mapbox::geojson::geojson>(geojson, "GeoJSON", py::module_local())
        is_geojson_type(geometry)           //
        is_geojson_type(feature)            //
        is_geojson_type(feature_collection) //
        as_geojson_type(geometry)           //
        as_geojson_type(feature)            //
        as_geojson_type(feature_collection) //
                                            //
            .def(py::self == py::self, "Check if two GeoJSON objects are equal")
            .def(py::self != py::self,
                 "Check if two GeoJSON objects are not equal")
            .def(py::init<>(), "Create an empty GeoJSON object")
            .def(py::init([](const mapbox::geojson::geometry &g) { return g; }),
                 "Create a GeoJSON object from a geometry")
            .def(py::init([](const mapbox::geojson::feature &g) { return g; }),
                 "Create a GeoJSON object from a feature")
            .def(py::init([](const mapbox::geojson::feature_collection &g) {
                     return g;
                 }),
                 "Create a GeoJSON object from a feature collection")
            .def(
                "round",
                [](mapbox::geojson::geojson &self, int lon, int lat,
                   int alt) -> mapbox::geojson::geojson & {
                    self.match(
                        [&](mapbox::geojson::geometry &g) {
                            round_coords(g, lon, lat, alt);
                        },
                        [&](mapbox::geojson::feature &f) {
                            round_coords(f.geometry, lon, lat, alt);
                        },
                        [&](mapbox::geojson::feature_collection &fc) {
                            for (auto &f : fc) {
                                round_coords(f.geometry, lon, lat, alt);
                            }
                        },
                        [](auto &) {});
                    return self;
                },
                py::kw_only(), "lon"_a = 8, "lat"_a = 8, "alt"_a = 3,
                rvp::reference_internal,
                "Round coordinates to specified decimal places") //
        GEOMETRY_DEDUPLICATE_XYZ(geojson)                        //
            .def(
                "from_rapidjson",
                [](mapbox::geojson::geojson &self,
                   const RapidjsonValue &json) -> mapbox::geojson::geojson & {
                    self = mapbox::geojson::convert(json);
                    return self;
                },
                rvp::reference_internal,
                "Convert a RapidJSON value to a GeoJSON object")
            .def(
                "to_rapidjson",
                [](const mapbox::geojson::geojson &self) {
                    RapidjsonAllocator allocator;
                    auto json = mapbox::geojson::convert(self, allocator);
                    return json;
                },
                "Convert the GeoJSON object to a RapidJSON value")
            .def(
                "from_geobuf",
                [](mapbox::geojson::geojson &self,
                   const std::string &bytes) -> mapbox::geojson::geojson & {
                    self = mapbox::geobuf::Decoder().decode(bytes);
                    return self;
                },
                rvp::reference_internal,
                "Decode a Geobuf byte string into a GeoJSON object")
            .def(
                "to_geobuf",
                [](const mapbox::geojson::geojson &self, //
                   int precision, bool only_xy, std::optional<int> round_z) {
                    auto bytes = mapbox::geobuf::Encoder(
                                     std::pow(10, precision), only_xy, round_z)
                                     .encode(self);
                    return py::bytes(bytes);
                },
                py::kw_only(),       //
                "precision"_a = 8,   //
                "only_xy"_a = false, //
                "round_z"_a = std::nullopt,
                "Encode the GeoJSON object to a Geobuf byte string")
            //
            .def(
                "crop",
                [](mapbox::geojson::geojson &self, const RowVectors &polygon,
                   const std::string &clipping_mode,
                   std::optional<double> max_z_offset)
                    -> mapbox::geojson::feature_collection {
                    return cubao::geojson_cropping(self,          //
                                                   polygon,       //
                                                   clipping_mode, //
                                                   max_z_offset);
                },
                "polygon"_a, py::kw_only(),    //
                "clipping_mode"_a = "longest", //
                "max_z_offset"_a = std::nullopt,
                "Crop the GeoJSON object using a polygon")
            //
            .def(
                "load",
                [](mapbox::geojson::geojson &self,
                   const std::string &path) -> mapbox::geojson::geojson & {
                    if (endswith(path, ".pbf")) {
                        auto bytes = mapbox::geobuf::load_bytes(path);
                        self = mapbox::geobuf::Decoder().decode(bytes);
                        return self;
                    }
                    auto json = load_json(path);
                    self = mapbox::geojson::convert(json);
                    return self;
                },
                rvp::reference_internal, "Load a GeoJSON object from a file")
            .def(
                "dump",
                [](const mapbox::geojson::geojson &self, //
                   const std::string &path,              //
                   bool indent,                          //
                   bool sort_keys,                       //
                   int precision,                        //
                   bool only_xy) {
                    if (endswith(path, ".pbf")) {
                        auto bytes = mapbox::geobuf::Encoder(
                                         std::pow(10, precision), only_xy)
                                         .encode(self);
                        return mapbox::geobuf::dump_bytes(path, bytes);
                    }
                    RapidjsonAllocator allocator;
                    auto json = mapbox::geojson::convert(self, allocator);
                    sort_keys_inplace(json);
                    return dump_json(path, json, indent);
                },
                "path"_a, py::kw_only(), //
                "indent"_a = false,      //
                "sort_keys"_a = false,   //
                "precision"_a = 8,       //
                "only_xy"_a = false,     //
                "Dump the GeoJSON object to a file")
                copy_deepcopy_clone(mapbox::geojson::geojson)
            .def(
                "__call__",
                [](const mapbox::geojson::geojson &self) {
                    return self.match(
                        [&](const mapbox::geojson::geometry &g) {
                            return to_python(g);
                        },
                        [&](const mapbox::geojson::feature &f) {
                            return to_python(f);
                        },
                        [&](const mapbox::geojson::feature_collection &fc) {
                            return to_python(fc);
                        },
                        [](const auto &) -> py::object { return py::none(); });
                },
                "Convert the GeoJSON object to a Python dictionary")
        //
        ;

#define GEOMETRY_ROUND_COORDS(geom_type)                                       \
    .def(                                                                      \
        "round",                                                               \
        [](mapbox::geojson::geom_type &self, int lon, int lat,                 \
           int alt) -> mapbox::geojson::geom_type & {                          \
            round_coords(self, lon, lat, alt);                                 \
            return self;                                                       \
        },                                                                     \
        py::kw_only(), "lon"_a = 8, "lat"_a = 8, "alt"_a = 3,                  \
        rvp::reference_internal,                                               \
        "Round coordinates to specified decimal places")

    py::bind_vector<mapbox::geojson::multi_point::container_type>(geojson,
                                                                  "coordinates")
        .def(
            "as_numpy",
            [](std::vector<mapbox::geojson::point> &self)
                -> Eigen::Map<RowVectors> {
                return Eigen::Map<RowVectors>(&self[0].x, //
                                              self.size(), 3);
            },
            rvp::reference_internal, "Get a numpy view of the coordinates")
        .def(
            "to_numpy",
            [](const std::vector<mapbox::geojson::point> &self) -> RowVectors {
                return Eigen::Map<const RowVectors>(&self[0].x, //
                                                    self.size(), 3);
            },
            "Convert coordinates to a numpy array") //
        .def(
            "from_numpy",
            [](std::vector<mapbox::geojson::point> &self,
               const Eigen::Ref<const MatrixXdRowMajor> &points)
                -> std::vector<mapbox::geojson::point> & {
                eigen2geom(points, self);
                return self;
            },
            rvp::reference_internal, "Set coordinates from a numpy array")
        //
        ;

#define is_geometry_type(geom_type)                                            \
    .def(                                                                      \
        "is_" #geom_type,                                                      \
        [](const mapbox::geojson::geometry &self) {                            \
            return self.is<mapbox::geojson::geom_type>();                      \
        },                                                                     \
        "Check if this geometry is of type " #geom_type)

#define as_geometry_type(geom_type)                                            \
    .def(                                                                      \
        "as_" #geom_type,                                                      \
        [](mapbox::geojson::geometry &self) -> mapbox::geojson::geom_type & {  \
            return self.get<mapbox::geojson::geom_type>();                     \
        },                                                                     \
        rvp::reference_internal,                                               \
        "Get this geometry as a " #geom_type " (if it is one)")

    using GeometryBase = mapbox::geometry::geometry_base<double, std::vector>;
    py::class_<GeometryBase>(geojson, "GeometryBase", py::module_local());
    py::class_<mapbox::geojson::geometry, GeometryBase>(geojson, "Geometry",
                                                        py::module_local())
        .def(py::init<>(), "Initialize an empty geometry")
        .def(py::init([](const mapbox::geojson::point &g) { return g; }),
             "Initialize from a Point")
        .def(py::init([](const mapbox::geojson::multi_point &g) { return g; }),
             "Initialize from a MultiPoint")
        .def(py::init([](const mapbox::geojson::line_string &g) { return g; }),
             "Initialize from a LineString")
        .def(py::init(
            [](const mapbox::geojson::multi_line_string &g) { return g; }),
             "Initialize from a MultiLineString")
        .def(py::init([](const mapbox::geojson::polygon &g) { return g; }),
             "Initialize from a Polygon")
        .def(
            py::init([](const mapbox::geojson::multi_polygon &g) { return g; }),
             "Initialize from a MultiPolygon")
        .def(py::init(
            [](const mapbox::geojson::geometry_collection &g) { return g; }),
             "Initialize from a GeometryCollection")
        .def(py::init([](const mapbox::geojson::geometry &g) { return g; }),
             "Initialize from another Geometry")
        .def(py::init(
            [](const mapbox::geojson::geometry_collection &g) { return g; }),
             "Initialize from a GeometryCollection")
        .def(py::init(
            [](const RapidjsonValue &g) {
                return mapbox::geojson::convert<mapbox::geojson::geometry>(g);
            }),
             "Initialize from a RapidJSON value")
        .def(py::init(
            [](const py::dict &g) {
                auto json = to_rapidjson(g);
                return mapbox::geojson::convert<mapbox::geojson::geometry>(json);
            }),
             "Initialize from a Python dictionary")
        // check geometry type
        is_geometry_type(empty)               //
        is_geometry_type(point)               //
        is_geometry_type(line_string)         //
        is_geometry_type(polygon)             //
        is_geometry_type(multi_point)         //
        is_geometry_type(multi_line_string)   //
        is_geometry_type(multi_polygon)       //
        is_geometry_type(geometry_collection) //
        // convert geometry type
        as_geometry_type(point)               //
        as_geometry_type(line_string)         //
        as_geometry_type(polygon)             //
        as_geometry_type(multi_point)         //
        as_geometry_type(multi_line_string)   //
        as_geometry_type(multi_polygon)       //
        as_geometry_type(geometry_collection) //
        .def(
            "__getitem__",
            [](mapbox::geojson::geometry &self,
               const std::string &key) -> mapbox::geojson::value & {
                return self.custom_properties.at(key);
            },
            "key"_a, rvp::reference_internal,
            "Get a custom property value") //
        .def(
            "get",
            [](mapbox::geojson::geometry &self,
               const std::string &key) -> mapbox::geojson::value * {
                auto &props = self.custom_properties;
                auto itr = props.find(key);
                if (itr == props.end()) {
                    return nullptr;
                }
                return &itr->second;
            },
            "key"_a, rvp::reference_internal,
            "Get a custom property value, returns None if not found")
        .def("__setitem__",
             [](mapbox::geojson::geometry &self, const std::string &key,
                const py::object &value) {
                 if (key == "type" || key == "coordinates" || key == "geometries") {
                     throw pybind11::key_error(key);
                 }
                 self.custom_properties[key] = to_geojson_value(value);
                 return value;
             },
             "Set a custom property value")
        .def("__delitem__",
             [](mapbox::geojson::geometry &self, const std::string &key) {
                 return self.custom_properties.erase(key);
             },
             "Delete a custom property")
        .def("__len__",
             [](mapbox::geojson::geometry &self) { return __len__(self); },
             "Get the number of coordinates or sub-geometries")
        .def(
            "push_back",
            [](mapbox::geojson::geometry &self,
               const mapbox::geojson::point &point)
                -> mapbox::geojson::geometry & {
                geometry_push_back(self, point);
                return self;
            },
            rvp::reference_internal,
            "Add a point to the geometry")
        .def(
            "push_back",
            [](mapbox::geojson::geometry &self,
               const Eigen::VectorXd &point) -> mapbox::geojson::geometry & {
                geometry_push_back(self, point);
                return self;
            },
            rvp::reference_internal,
            "Add a point to the geometry")
        .def(
            "push_back",
            [](mapbox::geojson::geometry &self,
               const Eigen::Ref<const MatrixXdRowMajor> &points)
                -> mapbox::geojson::geometry & {
                geometry_push_back(self, points);
                return self;
            },
            rvp::reference_internal,
            "Add multiple points to the geometry")
        .def(
            "push_back",
            [](mapbox::geojson::geometry &self,
               const mapbox::geojson::geometry &geom)
                -> mapbox::geojson::geometry & {
                if (self.is<mapbox::geojson::multi_polygon>() && geom.is<mapbox::geojson::polygon>()) {
                    self.get<mapbox::geojson::multi_polygon>().push_back(geom.get<mapbox::geojson::polygon>());
                } else {
                    geometry_push_back(self, geom);
                }
                return self;
            },
            rvp::reference_internal,
            "Add a sub-geometry to the geometry")
        .def(
            "push_back",
            [](mapbox::geojson::geometry &self,
               const mapbox::geojson::polygon &geom)
                -> mapbox::geojson::geometry & {
                    if (self.is<mapbox::geojson::multi_polygon>()) {
                        self.get<mapbox::geojson::multi_polygon>().push_back(geom);
                    } else {
                        std::cerr << "can only push_back Polygon to MultiPolygon, current type: " << geometry_type(self) << std::endl;
                    }
                return self;
            },
            rvp::reference_internal,
            "Add a polygon to a multi-polygon geometry")
        .def(
            "push_back",
            [](mapbox::geojson::geometry &self,
               const mapbox::geojson::line_string &geom)
                -> mapbox::geojson::geometry & {
                    if (self.is<mapbox::geojson::multi_line_string>()) {
                        self.get<mapbox::geojson::multi_line_string>().push_back(geom);
                    } else {
                        std::cerr << "can only push_back LineString to MultiLineString, current type: " << geometry_type(self) << std::endl;
                    }
                return self;
            },
            rvp::reference_internal,
            "Add a line string to a multi-line string geometry")
        .def(
            "pop_back",
            [](mapbox::geojson::geometry &self) -> mapbox::geojson::geometry & {
                geometry_pop_back(self);
                return self;
            },
            rvp::reference_internal,
            "Remove the last point or sub-geometry")
        .def(
            "resize",
            [](mapbox::geojson::geometry &self, int size) -> mapbox::geojson::geometry & {
                geometry_resize(self, size);
                return self;
            },
            rvp::reference_internal,
            "Resize the geometry")
        .def(
            "clear",
            [](mapbox::geojson::geometry &self) -> mapbox::geojson::geometry & {
                geometry_clear(self);
                self.custom_properties.clear();
                return self;
            },
            rvp::reference_internal,
            "Clear the geometry and custom properties")
        .def("type",
             [](const mapbox::geojson::geometry &self) {
                 return geometry_type(self);
             },
             "Get the type of the geometry")
        //
        .def(
            "as_numpy",
            [](mapbox::geojson::geometry &self) -> Eigen::Map<RowVectors> {
                return as_row_vectors(self);
            },
            rvp::reference_internal,
            "Get a numpy view of the geometry coordinates")
        .def("to_numpy",
             [](const mapbox::geojson::geometry &self) -> RowVectors {
                 return as_row_vectors(self);
             },
             "Convert geometry coordinates to a numpy array") //
        .def(
            "from_numpy",
            [](mapbox::geojson::geometry &self,
               const Eigen::Ref<const MatrixXdRowMajor> &points)
                -> mapbox::geojson::geometry & {
                eigen2geom(points, self);
                return self;
            },
            rvp::reference_internal,
            "Set geometry coordinates from a numpy array") //
        // dumps, loads, from/to rapidjson
        copy_deepcopy_clone(mapbox::geojson::geometry)
        .def(py::pickle(
            [](const mapbox::geojson::geometry &self) {
                return to_python(self);
            },
            [](py::object o) -> mapbox::geojson::geometry {
                auto json = to_rapidjson(o);
                return mapbox::geojson::convert<mapbox::geojson::geometry>(
                    json);
            }),
            "Pickle support for Geometry objects")
        // custom_properties
        BIND_PY_FLUENT_ATTRIBUTE(mapbox::geojson::geometry,  //
                                 PropertyMap,               //
                                 custom_properties)         //
            .def("keys",
                 [](mapbox::geojson::geometry &self) {
                     return py::make_key_iterator(self.custom_properties);
                 }, py::keep_alive<0, 1>(),
                 "Get an iterator over the custom property keys")
            .def("values",
                 [](mapbox::geojson::geometry &self) {
                     return py::make_value_iterator(self.custom_properties);
                 }, py::keep_alive<0, 1>(),
                 "Get an iterator over the custom property values")
            .def("items",
                 [](mapbox::geojson::geometry &self) {
                     return py::make_iterator(self.custom_properties);
                 },
                 py::keep_alive<0, 1>(),
                 "Get an iterator over the custom property items")

        .def(
            "__iter__",
            [](mapbox::geojson::geometry &self) {
                return py::make_key_iterator(self.custom_properties);
            },
            py::keep_alive<0, 1>(),
            "Get an iterator over the custom property keys")
        GEOMETRY_ROUND_COORDS(geometry)
        GEOMETRY_DEDUPLICATE_XYZ(geometry)
        .def_property_readonly(
            "__geo_interface__",
            [](const mapbox::geojson::geometry &self) -> py::object {
                return to_python(self);
            })
        .def(
            "from_rapidjson",
            [](mapbox::geojson::geometry &self,
               const RapidjsonValue &json) -> mapbox::geojson::geometry & {
                self =
                    mapbox::geojson::convert<mapbox::geojson::geometry>(json);
                return self;
            },
            rvp::reference_internal,
            "Convert a RapidJSON value to a geometry")
        .def("to_rapidjson",
             [](const mapbox::geojson::geometry &self) {
                 RapidjsonAllocator allocator;
                 auto json = mapbox::geojson::convert(self, allocator);
                 return json;
             },
             "Convert the geometry to a RapidJSON value")
        .def(
            "from_geobuf",
            [](mapbox::geojson::geometry &self,
                const std::string &bytes) -> mapbox::geojson::geometry & {
                auto geojson = mapbox::geobuf::Decoder().decode(bytes);
                self = std::move(geojson.get<mapbox::geojson::geometry>());
                return self;
            },
            rvp::reference_internal,
            "Decode a Geobuf byte string into a geometry")
        .def("to_geobuf",
            [](const mapbox::geojson::geometry &self, //
            int precision, bool only_xy, std::optional<int> round_z) {
            auto bytes =
                mapbox::geobuf::Encoder(std::pow(10, precision), only_xy, round_z)
                    .encode(self);
            return py::bytes(bytes);
            }, py::kw_only(), //
            "precision"_a = 8, //
            "only_xy"_a = false, //
            "round_z"_a = std::nullopt,
            "Encode the geometry to a Geobuf byte string")
        .def("load",
            [](mapbox::geojson::geometry &self, const std::string &path) -> mapbox::geojson::geometry & {
                if (endswith(path, ".pbf")) {
                    auto bytes = mapbox::geobuf::load_bytes(path);
                    auto geojson = mapbox::geobuf::Decoder().decode(bytes);
                    self = std::move(geojson.get<mapbox::geojson::geometry>());
                    return self;
                }
                auto json = load_json(path);
                self =
                    mapbox::geojson::convert<mapbox::geojson::geometry>(json);
                return self;
            }, rvp::reference_internal,
            "Load a geometry from a file")
        .def("dump",
            [](const mapbox::geojson::geometry &self, //
                const std::string &path, //
                bool indent, //
                bool sort_keys, //
                int precision, //
                bool only_xy) {
                if (endswith(path, ".pbf")) {
                    auto bytes = mapbox::geobuf::Encoder(std::pow(10, precision), only_xy).encode(self);
                    return mapbox::geobuf::dump_bytes(path, bytes);
                }
                RapidjsonAllocator allocator;
                auto json = mapbox::geojson::convert(self, allocator);
                sort_keys_inplace(json);
                return dump_json(path, json, indent);
            }, "path"_a, py::kw_only(), //
            "indent"_a = false, //
            "sort_keys"_a = false, //
            "precision"_a = 8, //
            "only_xy"_a = false,
            "Dump the geometry to a file")
        .def("bbox", [](const mapbox::geojson::geometry &self, bool with_z) -> Eigen::VectorXd {
            return geom2bbox(self, with_z);
        }, py::kw_only(), "with_z"_a = false,
        "Get the bounding box of the geometry")
        .def(py::self == py::self, "Check if two geometries are equal")
        .def(py::self != py::self, "Check if two geometries are not equal")

        .def("__call__",
             [](const mapbox::geojson::geometry &self) {
                 return to_python(self);
             },
             "Convert the geometry to a Python dictionary")
        //
        ;

    py::class_<mapbox::geojson::point>(geojson, "Point", py::module_local())
        .def(py::init<>(), "Initialize an empty Point")
        .def(py::init<double, double, double>(), "x"_a, "y"_a, "z"_a = 0.0,
             "Initialize a Point with coordinates (x, y, z)")
        .def(py::init([](const Eigen::VectorXd &p) {
                 return mapbox::geojson::point(p[0], p[1],
                                               p.size() > 2 ? p[2] : 0.0);
             }),
             "Initialize a Point from a numpy array or vector")
        .def(
            "as_numpy",
            [](mapbox::geojson::point &self) -> Eigen::Map<Eigen::Vector3d> {
                return Eigen::Vector3d::Map(&self.x);
            },
            rvp::reference_internal,
            "Get a numpy view of the point coordinates")
        .def(
            "to_numpy",
            [](const mapbox::geojson::point &self) -> Eigen::Vector3d {
                return Eigen::Vector3d::Map(&self.x);
            },
            "Convert point coordinates to a numpy array")
        .def(
            "from_numpy",
            [](mapbox::geojson::point &self,
               const Eigen::VectorXd &p) -> mapbox::geojson::point & {
                self.x = p[0];
                self.y = p[1];
                self.z = p.size() > 2 ? p[2] : 0.0;
                return self;
            },
            rvp::reference_internal, "Set point coordinates from a numpy array")
        .def_property(
            "x", [](const mapbox::geojson::point &self) { return self.x; },
            [](mapbox::geojson::point &self, double value) { self.x = value; },
            "Get or set the x-coordinate of the point")
        .def_property(
            "y", [](const mapbox::geojson::point &self) { return self.y; },
            [](mapbox::geojson::point &self, double value) { self.y = value; },
            "Get or set the y-coordinate of the point")
        .def_property(
            "z", [](const mapbox::geojson::point &self) { return self.z; },
            [](mapbox::geojson::point &self, double value) { self.z = value; },
            "Get or set the z-coordinate of the point")
        .def(
            "__getitem__",
            [](mapbox::geojson::point &self, int index) -> double {
                return *(&self.x + (index >= 0 ? index : index + 3));
            },
            "index"_a,
            "Get the coordinate value at the specified index (0: x, 1: y, 2: "
            "z)")
        .def(
            "__setitem__",
            [](mapbox::geojson::point &self, int index, double v) {
                *(&self.x + (index >= 0 ? index : index + 3)) = v;
                return v;
            },
            "index"_a, "value"_a,
            "Set the coordinate value at the specified index (0: x, 1: y, 2: "
            "z)") //
        .def(
            "__len__",
            [](const mapbox::geojson::point &self) -> int { return 3; },
            "Return the number of coordinates (always 3)")
        .def(
            "__iter__",
            [](mapbox::geojson::point &self) {
                return py::make_iterator(&self.x, &self.x + 3);
            },
            py::keep_alive<0, 1>(),
            "Return an iterator over the point's coordinates")
        //
        copy_deepcopy_clone(mapbox::geojson::point)
        .def(py::pickle(
                 [](const mapbox::geojson::point &self) {
                     return to_python(mapbox::geojson::geometry(self));
                 },
                 [](py::object o) -> mapbox::geojson::point {
                     auto json = to_rapidjson(o);
                     return mapbox::geojson::convert<mapbox::geojson::geometry>(
                                json)
                         .get<mapbox::geojson::point>();
                 }),
             "Enable pickling support for Point objects") //
        GEOMETRY_ROUND_COORDS(point)                      //
        GEOMETRY_DEDUPLICATE_XYZ(point)                   //
        .def_property_readonly(
            "__geo_interface__",
            [](const mapbox::geojson::point &self) -> py::object {
                return to_python(self);
            },
            "Return the __geo_interface__ representation of the point")
        .def(
            "from_rapidjson",
            [](mapbox::geojson::point &self,
               const RapidjsonValue &json) -> mapbox::geojson::point & {
                self = mapbox::geojson::convert<mapbox::geojson::geometry>(json)
                           .get<mapbox::geojson::point>();
                return self;
            },
            rvp::reference_internal, "Create a Point from a RapidJSON value")
        .def(
            "to_rapidjson",
            [](const mapbox::geojson::point &self) {
                RapidjsonAllocator allocator;
                auto json = mapbox::geojson::convert(
                    mapbox::geojson::geometry{self}, allocator);
                return json;
            },
            "Convert the Point to a RapidJSON value")
        .def(
            "bbox",
            [](const mapbox::geojson::point &self, bool with_z)
                -> Eigen::VectorXd { return geom2bbox(self, with_z); },
            py::kw_only(), "with_z"_a = false,
            "Get the bounding box of the point")
        .def(py::self == py::self, "Check if two Points are equal")     //
        .def(py::self != py::self, "Check if two Points are not equal") //
        .def(
            "clear",
            [](mapbox::geojson::point &self)
                -> mapbox::geojson::point & { // more like "reset"
                self.x = 0.0;
                self.y = 0.0;
                self.z = 0.0;
                return self;
            },
            rvp::reference_internal,
            "Reset all coordinates of the point to 0.0")
        .def(
            "__call__",
            [](const mapbox::geojson::point &self) { return to_python(self); },
            "Convert the Point to a Python dictionary")
        //
        ;

#define BIND_FOR_VECTOR_POINT_TYPE_PURE(geom_type)                             \
    .def(                                                                      \
        "__call__",                                                            \
        [](const mapbox::geojson::geom_type &self) {                           \
            return to_python(self);                                            \
        },                                                                     \
        "Convert the geometry to a Python dictionary")                         \
        .def(                                                                  \
            "__getitem__",                                                     \
            [](mapbox::geojson::geom_type &self,                               \
               int index) -> mapbox::geojson::point & {                        \
                return self[index >= 0 ? index : index + (int)self.size()];    \
            },                                                                 \
            rvp::reference_internal, "Get a point from the geometry by index") \
        .def(                                                                  \
            "__setitem__",                                                     \
            [](mapbox::geojson::geom_type &self, int index,                    \
               const mapbox::geojson::point &p) {                              \
                self[index >= 0 ? index : index + (int)self.size()] = p;       \
                return p;                                                      \
            },                                                                 \
            "Set a point in the geometry by index")                            \
        .def(                                                                  \
            "__setitem__",                                                     \
            [](mapbox::geojson::geom_type &self, int index,                    \
               const Eigen::VectorXd &p) {                                     \
                index = index >= 0 ? index : index + (int)self.size();         \
                self[index].x = p[0];                                          \
                self[index].y = p[1];                                          \
                self[index].z = p.size() > 2 ? p[2] : 0.0;                     \
                return p;                                                      \
            },                                                                 \
            "Set a point in the geometry by index using a vector")             \
        .def(                                                                  \
            "__len__",                                                         \
            [](const mapbox::geojson::geom_type &self) -> int {                \
                return self.size();                                            \
            },                                                                 \
            "Get the number of points in the geometry")                        \
        .def(                                                                  \
            "__iter__",                                                        \
            [](mapbox::geojson::geom_type &self) {                             \
                return py::make_iterator(self.begin(), self.end());            \
            },                                                                 \
            py::keep_alive<0, 1>(), "Iterate over the points in the geometry") \
        .def(                                                                  \
            "clear",                                                           \
            [](mapbox::geojson::geom_type &self)                               \
                -> mapbox::geojson::geom_type & {                              \
                self.clear();                                                  \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal, "Clear all points from the geometry")     \
        .def(                                                                  \
            "pop_back",                                                        \
            [](mapbox::geojson::geom_type &self)                               \
                -> mapbox::geojson::geom_type & {                              \
                self.pop_back();                                               \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Remove the last point from the geometry")                         \
        .def(                                                                  \
            "push_back",                                                       \
            [](mapbox::geojson::geom_type &self,                               \
               const mapbox::geojson::point &point)                            \
                -> mapbox::geojson::geom_type & {                              \
                self.push_back(point);                                         \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal, "Add a point to the end of the geometry") \
        .def(                                                                  \
            "push_back",                                                       \
            [](mapbox::geojson::geom_type &self,                               \
               const Eigen::VectorXd &xyz) -> mapbox::geojson::geom_type & {   \
                self.emplace_back(xyz[0], xyz[1],                              \
                                  xyz.size() > 2 ? xyz[2] : 0.0);              \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Add a point to the end of the geometry using a vector")           \
        .def(                                                                  \
            "as_numpy",                                                        \
            [](mapbox::geojson::geom_type &self) -> Eigen::Map<RowVectors> {   \
                return Eigen::Map<RowVectors>(&self[0].x, self.size(), 3);     \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Get a numpy view of the geometry points")                         \
        .def(                                                                  \
            "to_numpy",                                                        \
            [](const mapbox::geojson::geom_type &self) -> RowVectors {         \
                return Eigen::Map<const RowVectors>(&self[0].x, self.size(),   \
                                                    3);                        \
            },                                                                 \
            "Convert the geometry points to a numpy array")                    \
        .def(                                                                  \
            "from_numpy",                                                      \
            [](mapbox::geojson::geom_type &self,                               \
               const Eigen::Ref<const MatrixXdRowMajor> &points)               \
                -> mapbox::geojson::geom_type & {                              \
                eigen2geom(points, self);                                      \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Set the geometry points from a numpy array")                      \
            copy_deepcopy_clone(mapbox::geojson::geom_type) //

#define BIND_FOR_VECTOR_POINT_TYPE(geom_type)                                  \
    BIND_FOR_VECTOR_POINT_TYPE_PURE(geom_type)                                 \
        .def(py::init([](const Eigen::Ref<const MatrixXdRowMajor> &points) {   \
                 mapbox::geojson::geom_type self;                              \
                 eigen2geom(points, self);                                     \
                 return self;                                                  \
             }),                                                               \
             "Initialize from a numpy array of points")                        \
        .def(                                                                  \
            "resize",                                                          \
            [](mapbox::geojson::geom_type &self,                               \
               int size) -> mapbox::geojson::geom_type & {                     \
                self.resize(size);                                             \
                return self;                                                   \
            },                                                                 \
            "Resize the geometry to the specified size")                       \
        .def(py::pickle(                                                       \
                 [](const mapbox::geojson::geom_type &self) {                  \
                     return to_python(mapbox::geojson::geometry{self});        \
                 },                                                            \
                 [](py::object o) -> mapbox::geojson::geom_type {              \
                     auto json = to_rapidjson(o);                              \
                     return mapbox::geojson::convert<                          \
                                mapbox::geojson::geometry>(json)               \
                         .get<mapbox::geojson::geom_type>();                   \
                 }),                                                           \
             "Pickle support for serialization")                               \
            GEOMETRY_ROUND_COORDS(geom_type)                                   \
                GEOMETRY_DEDUPLICATE_XYZ(geom_type)                            \
        .def_property_readonly(                                                \
            "__geo_interface__",                                               \
            [](const mapbox::geojson::geom_type &self) -> py::object {         \
                return to_python(mapbox::geojson::geometry(self));             \
            },                                                                 \
            "Return the __geo_interface__ representation")                     \
        .def(                                                                  \
            "from_rapidjson",                                                  \
            [](mapbox::geojson::geom_type &self,                               \
               const RapidjsonValue &json) -> mapbox::geojson::geom_type & {   \
                self =                                                         \
                    mapbox::geojson::convert<mapbox::geojson::geometry>(json)  \
                        .get<mapbox::geojson::geom_type>();                    \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal, "Initialize from a RapidJSON value")      \
        .def(                                                                  \
            "to_rapidjson",                                                    \
            [](const mapbox::geojson::geom_type &self) {                       \
                RapidjsonAllocator allocator;                                  \
                auto json = mapbox::geojson::convert(                          \
                    mapbox::geojson::geometry{self}, allocator);               \
                return json;                                                   \
            },                                                                 \
            "Convert to a RapidJSON value")                                    \
        .def(                                                                  \
            "bbox",                                                            \
            [](const mapbox::geojson::geom_type &self, bool with_z)            \
                -> Eigen::VectorXd { return geom2bbox(self, with_z); },        \
            py::kw_only(), "with_z"_a = false,                                 \
            "Compute the bounding box of the geometry")

    py::class_<mapbox::geojson::multi_point,
               std::vector<mapbox::geojson::point>>(geojson, "MultiPoint",
                                                    py::module_local())
        .def(py::init<>(), "Default constructor for MultiPoint")
            BIND_FOR_VECTOR_POINT_TYPE(multi_point)
        .def(py::self == py::self, "Check if two MultiPoints are equal")
        .def(py::self != py::self, "Check if two MultiPoints are not equal");

    py::class_<mapbox::geojson::line_string,
               std::vector<mapbox::geojson::point>>(geojson, "LineString",
                                                    py::module_local())
        .def(py::init<>(), "Default constructor for LineString")
            BIND_FOR_VECTOR_POINT_TYPE(line_string)
        .def(py::self == py::self, "Check if two LineStrings are equal")
        .def(py::self != py::self, "Check if two LineStrings are not equal")
        .def(
            "deduplicate_xyz",
            [](mapbox::geojson::line_string &self) {
                return deduplicate_xyz(self);
            },
            "Remove duplicate consecutive points based on their XYZ "
            "coordinates");

#define BIND_FOR_VECTOR_LINEAR_RING_TYPE(geom_type)                            \
    .def(py::init([](const Eigen::Ref<const MatrixXdRowMajor> &points) {       \
             mapbox::geojson::geom_type self;                                  \
             eigen2geom(points, self);                                         \
             return self;                                                      \
         }),                                                                   \
         "Initialize from a numpy array of points")                            \
        .def(                                                                  \
            "__call__",                                                        \
            [](const mapbox::geojson::geom_type &self) {                       \
                return to_python(self);                                        \
            },                                                                 \
            "Convert the geometry to a Python dictionary")                     \
        .def(                                                                  \
            "__len__",                                                         \
            [](const mapbox::geojson::geom_type &self) -> int {                \
                return self.size();                                            \
            },                                                                 \
            "Return the number of linear rings in the geometry")               \
        .def(                                                                  \
            "__iter__",                                                        \
            [](mapbox::geojson::geom_type &self) {                             \
                return py::make_iterator(self.begin(), self.end());            \
            },                                                                 \
            py::keep_alive<0, 1>(),                                            \
            "Return an iterator over the linear rings in the geometry")        \
        .def(                                                                  \
            "__getitem__",                                                     \
            [](mapbox::geojson::geom_type &self,                               \
               int index) -> decltype(self[0]) & {                             \
                return self[index >= 0 ? index : index + (int)self.size()];    \
            },                                                                 \
            rvp::reference_internal, "Get a linear ring by index")             \
        .def(                                                                  \
            "__setitem__",                                                     \
            [](mapbox::geojson::geom_type &self, int index,                    \
               const Eigen::Ref<const MatrixXdRowMajor> &points) {             \
                auto &g = self[index >= 0 ? index : index + (int)self.size()]; \
                eigen2geom(points, g);                                         \
                return points;                                                 \
            },                                                                 \
            "Set a linear ring by index using a numpy array of points")        \
        .def(                                                                  \
            "clear",                                                           \
            [](mapbox::geojson::geom_type &self)                               \
                -> mapbox::geojson::geom_type & {                              \
                self.clear();                                                  \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Clear all linear rings from the geometry")                        \
        .def(                                                                  \
            "pop_back",                                                        \
            [](mapbox::geojson::geom_type &self)                               \
                -> mapbox::geojson::geom_type & {                              \
                self.back().pop_back();                                        \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Remove the last point from the last linear ring")                 \
        .def(                                                                  \
            "push_back",                                                       \
            [](mapbox::geojson::geom_type &self,                               \
               const Eigen::Ref<const MatrixXdRowMajor> &points)               \
                -> mapbox::geojson::geom_type & {                              \
                mapbox::geojson::geom_type::container_type::value_type ls;     \
                eigen2geom(points, ls);                                        \
                self.push_back(ls);                                            \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Add a new linear ring from a numpy array of points")              \
        .def(                                                                  \
            "push_back",                                                       \
            [](mapbox::geojson::geom_type &self,                               \
               const mapbox::geojson::geom_type::container_type::value_type    \
                   &g) -> mapbox::geojson::geom_type & {                       \
                self.push_back(g);                                             \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal, "Add a new linear ring")                  \
        .def(                                                                  \
            "as_numpy",                                                        \
            [](mapbox::geojson::geom_type &self) -> Eigen::Map<RowVectors> {   \
                return as_row_vectors(self);                                   \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Return a numpy view of the geometry's points")                    \
        .def(                                                                  \
            "to_numpy",                                                        \
            [](const mapbox::geojson::geom_type &self) -> RowVectors {         \
                return as_row_vectors(self);                                   \
            },                                                                 \
            "Convert the geometry to a numpy array")                           \
        .def(                                                                  \
            "from_numpy",                                                      \
            [](mapbox::geojson::geom_type &self,                               \
               const Eigen::Ref<const MatrixXdRowMajor> &points)               \
                -> mapbox::geojson::geom_type & {                              \
                eigen2geom(points, self);                                      \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Set the geometry from a numpy array of points")                   \
            copy_deepcopy_clone(mapbox::geojson::geom_type)                    \
        .def(py::pickle(                                                       \
                 [](const mapbox::geojson::geom_type &self) {                  \
                     return to_python(mapbox::geojson::geometry{self});        \
                 },                                                            \
                 [](py::object o) -> mapbox::geojson::geom_type {              \
                     auto json = to_rapidjson(o);                              \
                     return mapbox::geojson::convert<                          \
                                mapbox::geojson::geometry>(json)               \
                         .get<mapbox::geojson::geom_type>();                   \
                 }),                                                           \
             "Pickle support for the geometry")                                \
        .def_property_readonly(                                                \
            "__geo_interface__",                                               \
            [](const mapbox::geojson::geom_type &self) -> py::object {         \
                return to_python(mapbox::geojson::geometry(self));             \
            },                                                                 \
            "Return the __geo_interface__ representation of the geometry")     \
        .def(                                                                  \
            "from_rapidjson",                                                  \
            [](mapbox::geojson::geom_type &self,                               \
               const RapidjsonValue &json) -> mapbox::geojson::geom_type & {   \
                self =                                                         \
                    mapbox::geojson::convert<mapbox::geojson::geometry>(json)  \
                        .get<mapbox::geojson::geom_type>();                    \
                return self;                                                   \
            },                                                                 \
            rvp::reference_internal,                                           \
            "Initialize the geometry from a RapidJSON value")                  \
        .def(                                                                  \
            "to_rapidjson",                                                    \
            [](const mapbox::geojson::geom_type &self) {                       \
                RapidjsonAllocator allocator;                                  \
                auto json = mapbox::geojson::convert(                          \
                    mapbox::geojson::geometry{self}, allocator);               \
                return json;                                                   \
            },                                                                 \
            "Convert the geometry to a RapidJSON value")                       \
        .def(                                                                  \
            "round",                                                           \
            [](mapbox::geojson::geom_type &self, int lon, int lat,             \
               int alt) -> mapbox::geojson::geom_type & {                      \
                for (auto &g : self) {                                         \
                    round_coords(g, lon, lat, alt);                            \
                }                                                              \
                return self;                                                   \
            },                                                                 \
            py::kw_only(), "lon"_a = 8, "lat"_a = 8, "alt"_a = 3,              \
            rvp::reference_internal, "Round the coordinates of the geometry")  \
            GEOMETRY_DEDUPLICATE_XYZ(geom_type)                                \
        .def(                                                                  \
            "bbox",                                                            \
            [](const mapbox::geojson::geom_type &self, bool with_z)            \
                -> Eigen::VectorXd { return geom2bbox(self, with_z); },        \
            py::kw_only(), "with_z"_a = false,                                 \
            "Compute the bounding box of the geometry")

    py::class_<mapbox::geojson::linear_ring,
               mapbox::geojson::linear_ring::container_type>(
        geojson, "LinearRing", py::module_local())               //
        .def(py::init<>(), "Default constructor for LinearRing") //
        BIND_FOR_VECTOR_POINT_TYPE_PURE(linear_ring)
        .def(py::self == py::self, "Check if two LinearRings are equal")     //
        .def(py::self != py::self, "Check if two LinearRings are not equal") //
        //
        ;

    py::bind_vector<mapbox::geojson::multi_line_string::container_type>(
        geojson, "LineStringList", py::module_local(), "A list of LineStrings");

    py::class_<mapbox::geojson::multi_line_string,
               mapbox::geojson::multi_line_string::container_type>(
        geojson, "MultiLineString", py::module_local()) //
        .def(py::init<>(), "Default constructor for MultiLineString")
        .def(py::init<mapbox::geojson::multi_line_string::container_type>(),
             "Construct MultiLineString from a container of LineStrings")
        .def(py::init([](std::vector<mapbox::geojson::point> line_string) {
                 return mapbox::geojson::multi_line_string(
                     {std::move(line_string)});
             }),
             "Construct MultiLineString from a single LineString") //
        //
        BIND_FOR_VECTOR_LINEAR_RING_TYPE(multi_line_string)                   //
        .def(py::self == py::self, "Check if two MultiLineStrings are equal") //
        .def(py::self != py::self,
             "Check if two MultiLineStrings are not equal") //
        //
        ;

    py::bind_vector<mapbox::geojson::polygon::container_type>(
        geojson, "LinearRingList", py::module_local(), "A list of LinearRings");
    py::class_<mapbox::geojson::polygon,
               mapbox::geojson::polygon::container_type>(geojson, "Polygon",
                                                         py::module_local()) //
        .def(py::init<>(), "Default constructor for Polygon")
        .def(py::init<mapbox::geojson::polygon::container_type>(),
             "Construct Polygon from a container of LinearRings")
        .def(py::init([](std::vector<mapbox::geojson::point> shell) {
                 return mapbox::geojson::polygon({std::move(shell)});
             }),
             "Construct Polygon from a single LinearRing (shell)") //
        //
        BIND_FOR_VECTOR_LINEAR_RING_TYPE(polygon)                         //
        .def(py::self == py::self, "Check if two Polygons are equal")     //
        .def(py::self != py::self, "Check if two Polygons are not equal") //
        //
        ;

    py::bind_vector<mapbox::geojson::multi_polygon::container_type>(
        geojson, "PolygonList");
    py::class_<mapbox::geojson::multi_polygon,
               mapbox::geojson::multi_polygon::container_type>(
        geojson, "MultiPolygon", py::module_local()) //
        .def(py::init<>(), "Default constructor for MultiPolygon")
        .def(py::init<mapbox::geojson::multi_polygon>(),
             "Copy constructor for MultiPolygon")
        .def(py::init<mapbox::geojson::multi_polygon::container_type>(),
             "Construct MultiPolygon from a container of Polygons")
        .def(py::init([](const Eigen::Ref<const MatrixXdRowMajor> &points) {
                 mapbox::geojson::multi_polygon self;
                 eigen2geom(points, self);
                 return self;
             }),
             "Construct MultiPolygon from a numpy array of points")
        .def(
            "as_numpy",
            [](mapbox::geojson::multi_polygon &self) -> Eigen::Map<RowVectors> {
                return as_row_vectors(self);
            },
            rvp::reference_internal,
            "Return a numpy view of the MultiPolygon coordinates")
        .def(
            "to_numpy",
            [](const mapbox::geojson::polygon &self) -> RowVectors {
                return as_row_vectors(self);
            },
            "Convert MultiPolygon to a numpy array") //
        .def(
            "from_numpy",
            [](mapbox::geojson::multi_polygon &self,
               const Eigen::Ref<const MatrixXdRowMajor> &points)
                -> mapbox::geojson::multi_polygon & {
                eigen2geom(points, self);
                return self;
            },
            rvp::reference_internal,
            "Set MultiPolygon coordinates from a numpy array") //
                                                               //
        .def(
            "__call__",
            [](const mapbox::geojson::multi_polygon &self) {
                return to_python(self);
            },
            "Convert MultiPolygon to a Python object")
        .def(
            "__len__",
            [](const mapbox::geojson::multi_polygon &self) -> int {
                return self.size();
            },
            "Return the number of Polygons in the MultiPolygon")
        .def(
            "__iter__",
            [](mapbox::geojson::multi_polygon &self) {
                return py::make_iterator(self.begin(), self.end());
            },
            py::keep_alive<0, 1>(),
            "Return an iterator over the Polygons in the MultiPolygon")
        .def(
            "__getitem__",
            [](mapbox::geojson::multi_polygon &self,
               int index) -> mapbox::geojson::polygon & {
                return self[index >= 0 ? index : index + (int)self.size()];
            },
            rvp::reference_internal,
            "Get a Polygon from the MultiPolygon by index")
        .def(
            "__setitem__",
            [](mapbox::geojson::multi_polygon &self, int index,
               const mapbox::geojson::polygon &polygon) {
                self[index >= 0 ? index : index + (int)self.size()] = polygon;
                return polygon;
            },
            "Set a Polygon in the MultiPolygon by index")
        .def(
            "__setitem__",
            [](mapbox::geojson::multi_polygon &self, int index,
               const Eigen::Ref<const MatrixXdRowMajor> &points) {
                auto &polygon =
                    self[index >= 0 ? index : index + (int)self.size()];
                eigen2geom(points, polygon);
                return polygon;
            },
            "Set a Polygon in the MultiPolygon by index using a numpy array")
        .def(
            "clear",
            [](mapbox::geojson::multi_polygon &self)
                -> mapbox::geojson::multi_polygon & {
                self.clear();
                return self;
            },
            rvp::reference_internal, "Clear all Polygons from the MultiPolygon")
        .def(
            "pop_back",
            [](mapbox::geojson::multi_polygon &self)
                -> mapbox::geojson::multi_polygon & {
                self.pop_back();
                return self;
            },
            rvp::reference_internal,
            "Remove the last Polygon from the MultiPolygon")
        .def(
            "push_back",
            [](mapbox::geojson::multi_polygon &self,
               const Eigen::Ref<const MatrixXdRowMajor> &points)
                -> mapbox::geojson::multi_polygon & {
                mapbox::geojson::polygon polygon;
                eigen2geom(points, polygon);
                self.push_back(std::move(polygon));
                return self;
            },
            rvp::reference_internal,
            "Add a new Polygon to the MultiPolygon from a numpy array")
        .def(
            "push_back",
            [](mapbox::geojson::multi_polygon &self,
               const mapbox::geojson::polygon &polygon)
                -> mapbox::geojson::multi_polygon & {
                self.push_back(polygon);
                return self;
            },
            rvp::reference_internal, "Add a new Polygon to the MultiPolygon")
            copy_deepcopy_clone(mapbox::geojson::multi_polygon)
        .def(py::pickle(
                 [](const mapbox::geojson::multi_polygon &self) {
                     return to_python(mapbox::geojson::geometry{self});
                 },
                 [](py::object o) -> mapbox::geojson::multi_polygon {
                     auto json = to_rapidjson(o);
                     return mapbox::geojson::convert<mapbox::geojson::geometry>(
                                json)
                         .get<mapbox::geojson::multi_polygon>();
                 }),
             "Pickle support for MultiPolygon")
        .def(
            "round",
            [](mapbox::geojson::multi_polygon &self, int lon, int lat,
               int alt) -> mapbox::geojson::multi_polygon & {
                for (auto &gg : self) {
                    for (auto &g : gg) {
                        round_coords(g, lon, lat, alt);
                    }
                }
                return self;
            },
            py::kw_only(), "lon"_a = 8, "lat"_a = 8, "alt"_a = 3,
            rvp::reference_internal,
            "Round the coordinates of the MultiPolygon")
        .def_property_readonly(
            "__geo_interface__",
            [](const mapbox::geojson::multi_polygon &self) -> py::object {
                return to_python(mapbox::geojson::geometry(self));
            },
            "Return the __geo_interface__ representation of the MultiPolygon")
        .def(
            "from_rapidjson",
            [](mapbox::geojson::multi_polygon &self,
               const RapidjsonValue &json) -> mapbox::geojson::multi_polygon & {
                self = mapbox::geojson::convert<mapbox::geojson::geometry>(json)
                           .get<mapbox::geojson::multi_polygon>();
                return self;
            },
            rvp::reference_internal,
            "Set the MultiPolygon from a RapidJSON value")
        .def(
            "to_rapidjson",
            [](const mapbox::geojson::multi_polygon &self) {
                RapidjsonAllocator allocator;
                auto json = mapbox::geojson::convert(
                    mapbox::geojson::geometry{self}, allocator);
                return json;
            },
            "Convert the MultiPolygon to a RapidJSON value")
        .def(
            "bbox",
            [](const mapbox::geojson::multi_polygon &self, bool with_z)
                -> Eigen::VectorXd { return geom2bbox(self, with_z); },
            py::kw_only(), "with_z"_a = false,
            "Compute the bounding box of the MultiPolygon")
        .def(py::self == py::self, "Check if two MultiPolygons are equal") //
        .def(py::self != py::self,
             "Check if two MultiPolygons are not equal") //
        //
        ;

    py::bind_vector<mapbox::geojson::geometry_collection::container_type>(
        geojson, "GeometryList", py::module_local());
    py::class_<mapbox::geojson::geometry_collection,
               mapbox::geojson::geometry_collection::container_type>(
        geojson, "GeometryCollection", py::module_local()) //
        .def(py::init<>(), "Default constructor for GeometryCollection")
        .def(py::init([](const mapbox::geojson::geometry_collection &g) {
                 return g;
             }),
             "Copy constructor for GeometryCollection")
        .def(py::init(
                 [](int N) { return mapbox::geojson::geometry_collection(N); }),
             "N"_a, "Construct a GeometryCollection with N empty geometries")
        .def(
            "resize",
            [](mapbox::geojson::geometry_collection &self,
               int N) -> mapbox::geojson::geometry_collection & {
                self.resize(N);
                return self;
            },
            rvp::reference_internal,
            "Resize the GeometryCollection to contain N geometries")
#define SETITEM_FOR_TYPE(geom_type)                                            \
    .def(                                                                      \
        "__setitem__",                                                         \
        [](mapbox::geojson::geometry_collection &self, int index,              \
           const mapbox::geojson::geom_type &g) {                              \
            self[index >= 0 ? index : index + (int)self.size()] = g;           \
            return self;                                                       \
        },                                                                     \
        rvp::reference_internal,                                               \
        "Set a geometry in the GeometryCollection by index")
        //
        SETITEM_FOR_TYPE(geometry)            //
        SETITEM_FOR_TYPE(point)               //
        SETITEM_FOR_TYPE(multi_point)         //
        SETITEM_FOR_TYPE(line_string)         //
        SETITEM_FOR_TYPE(multi_line_string)   //
        SETITEM_FOR_TYPE(polygon)             //
        SETITEM_FOR_TYPE(multi_polygon)       //
        SETITEM_FOR_TYPE(geometry_collection) //
#undef SETITEM_FOR_TYPE
        .def(
            "clear",
            [](mapbox::geojson::geometry_collection &self)
                -> mapbox::geojson::geometry_collection & {
                self.clear();
                return self;
            },
            rvp::reference_internal,
            "Clear all geometries from the GeometryCollection")
        .def(
            "pop_back",
            [](mapbox::geojson::geometry_collection &self)
                -> mapbox::geojson::geometry_collection & {
                self.pop_back();
                return self;
            },
            rvp::reference_internal,
            "Remove the last geometry from the GeometryCollection")
#define PUSH_BACK_FOR_TYPE(geom_type)                                          \
    .def(                                                                      \
        "push_back",                                                           \
        [](mapbox::geojson::geometry_collection &self,                         \
           const mapbox::geojson::geom_type &g) {                              \
            self.push_back(g);                                                 \
            return self;                                                       \
        },                                                                     \
        rvp::reference_internal,                                               \
        "Add a new geometry to the GeometryCollection")
        //
        PUSH_BACK_FOR_TYPE(geometry)            //
        PUSH_BACK_FOR_TYPE(point)               //
        PUSH_BACK_FOR_TYPE(multi_point)         //
        PUSH_BACK_FOR_TYPE(line_string)         //
        PUSH_BACK_FOR_TYPE(multi_line_string)   //
        PUSH_BACK_FOR_TYPE(polygon)             //
        PUSH_BACK_FOR_TYPE(multi_polygon)       //
        PUSH_BACK_FOR_TYPE(geometry_collection) //
#undef PUSH_BACK_FOR_TYPE
        .def(py::pickle(
                 [](const mapbox::geojson::geometry_collection &self) {
                     return to_python(mapbox::geojson::geometry{self});
                 },
                 [](py::object o) -> mapbox::geojson::geometry_collection {
                     auto json = to_rapidjson(o);
                     return mapbox::geojson::convert<mapbox::geojson::geometry>(
                                json)
                         .get<mapbox::geojson::geometry_collection>();
                 }),
             "Pickle support for GeometryCollection")
        .def(
            "round",
            [](mapbox::geojson::geometry_collection &self, int lon, int lat,
               int alt) -> mapbox::geojson::geometry_collection & {
                for (auto &g : self) {
                    round_coords(g, lon, lat, alt);
                }
                return self;
            },
            py::kw_only(), "lon"_a = 8, "lat"_a = 8, "alt"_a = 3,
            rvp::reference_internal,
            "Round the coordinates of all geometries in the GeometryCollection")
            GEOMETRY_DEDUPLICATE_XYZ(geometry_collection)
        .def_property_readonly(
            "__geo_interface__",
            [](const mapbox::geojson::geometry_collection &self) -> py::object {
                return to_python(mapbox::geojson::geometry(self));
            },
            "Return the __geo_interface__ representation of the "
            "GeometryCollection")
        .def(
            "from_rapidjson",
            [](mapbox::geojson::geometry_collection &self,
               const RapidjsonValue &json)
                -> mapbox::geojson::geometry_collection & {
                self = mapbox::geojson::convert<mapbox::geojson::geometry>(json)
                           .get<mapbox::geojson::geometry_collection>();
                return self;
            },
            rvp::reference_internal,
            "Set the GeometryCollection from a RapidJSON value")
        .def(
            "to_rapidjson",
            [](const mapbox::geojson::geometry_collection &self) {
                RapidjsonAllocator allocator;
                auto json = mapbox::geojson::convert(
                    mapbox::geojson::geometry{self}, allocator);
                return json;
            },
            "Convert the GeometryCollection to a RapidJSON value")
        .def(
            "__call__",
            [](const mapbox::geojson::geometry_collection &self) {
                return to_python(self);
            },
            "Convert the GeometryCollection to a Python object")
        .def(py::self == py::self,
             "Check if two GeometryCollections are equal") //
        .def(py::self != py::self,
             "Check if two GeometryCollections are not equal") //
        ;

    auto geojson_value =
        py::class_<mapbox::geojson::value>(geojson, "value", py::module_local())
            .def(py::init<>(), "Default constructor for GeoJSON value")
            .def(
                py::init([](const py::object &obj) { return to_geojson_value(obj); }),
                "Construct a GeoJSON value from a Python object")
            .def("GetType",
                 [](const mapbox::geojson::value &self) {
                    return get_type(self);
                 },
                 "Get the type of the GeoJSON value")
            .def("__call__",
                 [](const mapbox::geojson::value &self) {
                     return to_python(self);
                 },
                 "Convert the GeoJSON value to a Python object")
            .def("Get",
                 [](const mapbox::geojson::value &self) {
                     return to_python(self);
                 },
                 "Get the GeoJSON value as a Python object")
            .def("GetBool",
                 [](mapbox::geojson::value &self) { return self.get<bool>(); },
                 "Get the GeoJSON value as a boolean")
            .def("GetUint64",
                 [](mapbox::geojson::value &self) {
                     return self.get<uint64_t>();
                 },
                 "Get the GeoJSON value as an unsigned 64-bit integer")
            .def("GetInt64",
                 [](mapbox::geojson::value &self) {
                     return self.get<int64_t>();
                 },
                 "Get the GeoJSON value as a signed 64-bit integer")
            .def("GetDouble",
                 [](mapbox::geojson::value &self) -> double & {
                     return self.get<double>();
                 },
                 "Get the GeoJSON value as a double")
            .def("GetString",
                 [](mapbox::geojson::value &self) -> std::string & {
                     return self.get<std::string>();
                 },
                 "Get the GeoJSON value as a string")
            // casters
            .def("is_object",
                 [](const mapbox::geojson::value &self) {
                     return self.is<mapbox::geojson::value::object_type>();
                 },
                 "Check if the GeoJSON value is an object")
            .def("as_object",
                 [](mapbox::geojson::value &self)
                     -> mapbox::geojson::value::object_type & {
                         return self.get<mapbox::geojson::value::object_type>();
                     },
                 rvp::reference_internal,
                 "Get the GeoJSON value as an object")
            .def("is_array",
                 [](const mapbox::geojson::value &self) {
                     return self.is<mapbox::geojson::value::array_type>();
                 },
                 "Check if the GeoJSON value is an array")
            .def("as_array",
                 [](mapbox::geojson::value &self)
                     -> mapbox::geojson::value::array_type & {
                         return self.get<mapbox::geojson::value::array_type>();
                     },
                 rvp::reference_internal,
                 "Get the GeoJSON value as an array")
            .def("__getitem__",
                 [](mapbox::geojson::value &self,
                    int index) -> mapbox::geojson::value & {
                     auto &arr = self.get<mapbox::geojson::value::array_type>();
                     return arr[index >= 0 ? index : index + (int)arr.size()];
                 },
                 rvp::reference_internal,
                 "Get an item from the GeoJSON array by index")
            .def("__getitem__",
                 [](mapbox::geojson::value &self,
                    const std::string &key) -> mapbox::geojson::value & {
                     auto &obj =
                         self.get<mapbox::geojson::value::object_type>();
                     return obj.at(key);
                 },
                 rvp::reference_internal,
                 "Get an item from the GeoJSON object by key")                         //
            .def(
                "get", // get by key
                [](mapbox::geojson::value &self,
                    const std::string &key) -> mapbox::geojson::value * {
                     auto &obj =
                         self.get<mapbox::geojson::value::object_type>();
                    auto itr = obj.find(key);
                    if (itr == obj.end()) {
                        return nullptr;
                    }
                    return &itr->second;
                },
                "key"_a, rvp::reference_internal,
                "Get an item from the GeoJSON object by key, returning None if not found")
            .def("set", // set value
                 [](mapbox::geojson::value &self,
                    const py::object &obj) -> mapbox::geojson::value & {
                     self = to_geojson_value(obj);
                     return self;
                 },
                 rvp::reference_internal,
                 "Set the GeoJSON value from a Python object")
            .def("__setitem__",
                 [](mapbox::geojson::value &self, const std::string &key,
                    const py::object &value) {
                     auto &obj =
                         self.get<mapbox::geojson::value::object_type>();
                     obj[key] = to_geojson_value(value);
                     return value;
                 },
                 "Set an item in the GeoJSON object by key")
            .def("__setitem__",
                 [](mapbox::geojson::value &self, int index,
                    const py::object &value) {
                     auto &arr = self.get<mapbox::geojson::value::array_type>();
                     arr[index >= 0 ? index : index + (int)arr.size()] =
                         to_geojson_value(value);
                     return value;
                 },
                 "Set an item in the GeoJSON array by index")
            .def("keys",
                 [](mapbox::geojson::value &self) {
                     std::vector<std::string> keys;
                     auto &obj =
                         self.get<mapbox::geojson::value::object_type>();
                     return py::make_key_iterator(obj);
                 }, py::keep_alive<0, 1>(),
                 "Get an iterator over the keys of the GeoJSON object")
            .def("values",
                 [](mapbox::geojson::value &self) {
                     std::vector<std::string> keys;
                     auto &obj =
                         self.get<mapbox::geojson::value::object_type>();
                     return py::make_value_iterator(obj);
                 }, py::keep_alive<0, 1>(),
                 "Get an iterator over the values of the GeoJSON object")
            .def("items",
                 [](mapbox::geojson::value &self) {
                     auto &obj =
                         self.get<mapbox::geojson::value::object_type>();
                     return py::make_iterator(obj.begin(), obj.end());
                 },
                 py::keep_alive<0, 1>(),
                 "Get an iterator over the items of the GeoJSON object")

            .def("__delitem__",
                 [](mapbox::geojson::value &self, const std::string &key) {
                     auto &obj =
                         self.get<mapbox::geojson::value::object_type>();
                     return obj.erase(key);
                 },
                 "Delete an item from the GeoJSON object by key")
            .def("__delitem__",
                 [](mapbox::geojson::value &self, int index) {
                     auto &arr = self.get<mapbox::geojson::value::array_type>();
                     arr.erase(arr.begin() +
                               (index >= 0 ? index : index + (int)arr.size()));
                 },
                 "Delete an item from the GeoJSON array by index")
            .def("clear",
                 [](mapbox::geojson::value &self) -> mapbox::geojson::value & {
                     geojson_value_clear(self);
                     return self;
                 },
                 rvp::reference_internal,
                 "Clear the GeoJSON value")
            .def("push_back",
                 [](mapbox::geojson::value &self,
                    const py::object &value) -> mapbox::geojson::value & {
                     auto &arr = self.get<mapbox::geojson::value::array_type>();
                     arr.push_back(to_geojson_value(value));
                     return self;
                 },
                 rvp::reference_internal,
                 "Add a value to the end of the GeoJSON array")
            .def("pop_back",
                 [](mapbox::geojson::value &self) -> mapbox::geojson::value & {
                     auto &arr = self.get<mapbox::geojson::value::array_type>();
                     arr.pop_back();
                     return self;
                 },
                 rvp::reference_internal,
                 "Remove the last value from the GeoJSON array")
            .def("__len__",
                 [](const mapbox::geojson::value &self) -> int {
                    return __len__(self);
                 },
                 "Get the length of the GeoJSON value")
            .def("__bool__",
                 [](const mapbox::geojson::value &self) -> bool {
                    return __bool__(self);
                 },
                 "Check if the GeoJSON value is truthy")
            .def(
                "from_rapidjson",
                [](mapbox::geojson::value &self, const RapidjsonValue &json) -> mapbox::geojson::value & {
                    self = mapbox::geojson::convert<mapbox::geojson::value>(json);
                    return self;
                },
                rvp::reference_internal,
                "Set the GeoJSON value from a RapidJSON value")
            .def("to_rapidjson",
                [](const mapbox::geojson::value &self) {
                    return to_rapidjson(self);
                },
                "Convert the GeoJSON value to a RapidJSON value")
        //
        //
        ;

    py::bind_vector<mapbox::geojson::value::array_type>(
        geojson_value, "array_type", py::module_local())
        .def(py::init<>(), "Default constructor for GeoJSON array")
        .def(py::init([](const py::handle &arr) {
                 return to_geojson_value(arr)
                     .get<mapbox::geojson::value::array_type>();
             }),
             "Construct a GeoJSON array from a Python iterable")
        .def(
            "clear",
            [](mapbox::geojson::value::array_type &self)
                -> mapbox::geojson::value::array_type & {
                self.clear();
                return self;
            },
            rvp::reference_internal, "Clear the GeoJSON array")
        .def(
            "__getitem__",
            [](mapbox::geojson::value::array_type &self,
               int index) -> mapbox::geojson::value & {
                return self[index >= 0 ? index : index + (int)self.size()];
            },
            rvp::reference_internal,
            "Get an item from the GeoJSON array by index")
        .def(
            "__setitem__",
            [](mapbox::geojson::value::array_type &self, int index,
               const py::object &obj) {
                index = index < 0 ? index + (int)self.size() : index;
                self[index] = to_geojson_value(obj);
                return self[index]; // why not return obj?
            },
            rvp::reference_internal,
            "Set an item in the GeoJSON array by index")
        .def(
            "from_rapidjson",
            [](mapbox::geojson::value::array_type &self,
               const RapidjsonValue &json)
                -> mapbox::geojson::value::array_type & {
                self = mapbox::geojson::convert<mapbox::geojson::value>(json)
                           .get<mapbox::geojson::value::array_type>();
                return self;
            },
            rvp::reference_internal,
            "Set the GeoJSON array from a RapidJSON value")
        .def(
            "to_rapidjson",
            [](const mapbox::geojson::value::array_type &self) {
                return to_rapidjson(self);
            },
            "Convert the GeoJSON array to a RapidJSON value")
        .def(
            "__call__",
            [](const mapbox::geojson::value::array_type &self) {
                return to_python(self);
            },
            "Convert the GeoJSON array to a Python list")
        //
        ;

    py::bind_map<mapbox::geojson::value::object_type>(
        geojson_value, "object_type", py::module_local())
        .def(py::init<>(), "Default constructor for GeoJSON object")
        .def(py::init([](const py::object &obj) {
                 return to_geojson_value(obj)
                     .get<mapbox::geojson::value::object_type>();
             }),
             "Construct a GeoJSON object from a Python dict")
        .def(
            "clear",
            [](mapbox::geojson::value::object_type &self)
                -> mapbox::geojson::value::object_type & {
                self.clear();
                return self;
            },
            rvp::reference_internal, "Clear the GeoJSON object")
        .def(
            "__setitem__",
            [](mapbox::geojson::value::object_type &self,
               const std::string &key, const py::object &obj) {
                self[key] = to_geojson_value(obj);
                return self[key];
            },
            rvp::reference_internal, "Set an item in the GeoJSON object by key")
        .def(
            "keys",
            [](const mapbox::geojson::value::object_type &self) {
                return py::make_key_iterator(self.begin(), self.end());
            },
            py::keep_alive<0, 1>(),
            "Get an iterator over the keys of the GeoJSON object")
        .def(
            "values",
            [](const mapbox::geojson::value::object_type &self) {
                return py::make_value_iterator(self.begin(), self.end());
            },
            py::keep_alive<0, 1>(),
            "Get an iterator over the values of the GeoJSON object")
        .def(
            "items",
            [](mapbox::geojson::value::object_type &self) {
                return py::make_iterator(self.begin(), self.end());
            },
            py::keep_alive<0, 1>(),
            "Get an iterator over the items (key-value pairs) of the GeoJSON "
            "object")
        .def(
            "from_rapidjson",
            [](mapbox::geojson::value::object_type &self,
               const RapidjsonValue &json)
                -> mapbox::geojson::value::object_type & {
                self = mapbox::geojson::convert<
                    mapbox::geojson::value::object_type>(json);
                return self;
            },
            rvp::reference_internal,
            "Convert a RapidJSON value to a GeoJSON object")
        .def(
            "to_rapidjson",
            [](const mapbox::geojson::value::object_type &self) {
                return to_rapidjson(self);
            },
            "Convert the GeoJSON object to a RapidJSON value")
        .def(
            "__call__",
            [](const mapbox::geojson::value::object_type &self) {
                return to_python(self);
            },
            "Convert the GeoJSON object to a Python dict")
        //
        ;

    py::class_<mapbox::geojson::feature>(geojson, "Feature", py::module_local())
        .def(py::init<>(), "Default constructor for GeoJSON Feature")
        .def(py::init(
                 [](const mapbox::geojson::feature &other) { return other; }),
             "Copy constructor for GeoJSON Feature")
        .def(py::init([](const RapidjsonValue &feature) {
                 return mapbox::geojson::convert<mapbox::geojson::feature>(
                     feature);
             }),
             "Construct a GeoJSON Feature from a RapidJSON value")
        .def(py::init([](const py::dict &feature) {
                 auto json = to_rapidjson(feature);
                 return mapbox::geojson::convert<mapbox::geojson::feature>(
                     json);
             }),
             "Construct a GeoJSON Feature from a Python dict")
        //
        BIND_PY_FLUENT_ATTRIBUTE(mapbox::geojson::feature,  //
                                 mapbox::geojson::geometry, //
                                 geometry)                  //
        BIND_PY_FLUENT_ATTRIBUTE(mapbox::geojson::feature,  //
                                 PropertyMap,               //
                                 properties)                //
        BIND_PY_FLUENT_ATTRIBUTE(mapbox::geojson::feature,  //
                                 PropertyMap,               //
                                 custom_properties)         //

// geometry from point, mulipoint, etc
#define GeometryFromType(geom_type)                                            \
    .def(                                                                      \
        "geometry",                                                            \
        [](mapbox::geojson::feature &self,                                     \
           const mapbox::geojson::geom_type &geometry)                         \
            -> mapbox::geojson::feature & {                                    \
            self.geometry = geometry;                                          \
            return self;                                                       \
        },                                                                     \
        #geom_type##_a, rvp::reference_internal,                               \
        "Set the geometry of the feature to the given geometry object") //
                                                                        //
        GeometryFromType(point)                                         //
        GeometryFromType(multi_point)                                   //
        GeometryFromType(line_string)                                   //
        GeometryFromType(multi_line_string)                             //
        GeometryFromType(polygon)                                       //
        GeometryFromType(multi_polygon)                                 //
#undef GeometryFromType

        .def(
            "geometry",
            [](mapbox::geojson::feature &self,
               const py::object &obj) -> mapbox::geojson::feature & {
                auto json = to_rapidjson(obj);
                self.geometry =
                    mapbox::geojson::convert<mapbox::geojson::geometry>(json);
                return self;
            },
            rvp::reference_internal,
            "Set the geometry of the feature from a Python object")
        .def(
            "properties",
            [](mapbox::geojson::feature &self,
               const py::object &obj) -> mapbox::geojson::feature & {
                auto json = to_rapidjson(obj);
                self.properties =
                    mapbox::geojson::convert<mapbox::feature::property_map>(
                        json);
                return self;
            },
            rvp::reference_internal,
            "Set the properties of the feature from a Python object")
        .def(
            "properties",
            [](mapbox::geojson::feature &self,
               const std::string &key) -> mapbox::geojson::value * {
                auto &props = self.properties;
                auto itr = props.find(key);
                if (itr == props.end()) {
                    return nullptr;
                }
                return &itr->second;
            },
            rvp::reference_internal, "Get a property value by key")
        .def(
            "properties",
            [](mapbox::geojson::feature &self, const std::string &key,
               const py::object &value) -> mapbox::geojson::feature & {
                if (value.ptr() == nullptr || value.is_none()) {
                    self.properties.erase(key);
                } else {
                    self.properties[key] = to_geojson_value(value);
                }
                return self;
            },
            rvp::reference_internal, "Set a property value by key")
        .def(
            "id",
            [](mapbox::geojson::feature &self) { return to_python(self.id); },
            "Get the feature ID")
        .def(
            "id",
            [](mapbox::geojson::feature &self,
               const py::object &value) -> mapbox::geojson::feature & {
                self.id = to_feature_id(value);
                return self;
            },
            rvp::reference_internal, "Set the feature ID")
        //
        .def(
            "__getitem__",
            [](mapbox::geojson::feature &self,
               const std::string &key) -> mapbox::geojson::value * {
                // don't try "type", "geometry", "properties", "id"
                auto &props = self.custom_properties;
                auto itr = props.find(key);
                if (itr == props.end()) {
                    return nullptr;
                }
                return &itr->second;
            },
            rvp::reference_internal, "Get a custom property value by key")
        .def(
            "__setitem__",
            [](mapbox::geojson::feature &self, const std::string &key,
               const py::object &value) {
                if (key == "type" || key == "geometry" || key == "properties" ||
                    key == "id") {
                    throw pybind11::key_error(key);
                }
                self.custom_properties[key] = to_geojson_value(value);
                return value;
            },
            "Set a custom property value by key")
        .def(
            "__delitem__",
            [](mapbox::geojson::feature &self, const std::string &key) {
                return self.custom_properties.erase(key);
            },
            "Delete a custom property by key")
        .def(
            "keys",
            [](mapbox::geojson::feature &self) {
                return py::make_key_iterator(self.custom_properties);
            },
            py::keep_alive<0, 1>(), "Get an iterator over custom property keys")
        .def(
            "items",
            [](mapbox::geojson::feature &self) {
                return py::make_iterator(self.custom_properties.begin(),
                                         self.custom_properties.end());
            },
            py::keep_alive<0, 1>(),
            "Get an iterator over custom property items")
        .def(
            "clear",
            [](mapbox::geojson::feature &self) -> mapbox::geojson::feature & {
                geometry_clear(self.geometry);
                self.geometry.custom_properties.clear();
                self.properties.clear();
                self.id = mapbox::geojson::null_value_t();
                self.custom_properties.clear();
                return self;
            },
            rvp::reference_internal,
            "Clear all properties and geometry of the feature")
        .def(
            "bbox",
            [](const mapbox::geojson::feature &self, bool with_z)
                -> Eigen::VectorXd { return geom2bbox(self.geometry, with_z); },
            py::kw_only(), "with_z"_a = false,
            "Compute the bounding box of the feature")
        .def(py::self == py::self, "Check if two features are equal")     //
        .def(py::self != py::self, "Check if two features are not equal") //
        //
        .def(
            "as_numpy",
            [](mapbox::geojson::feature &self) -> Eigen::Map<RowVectors> {
                return as_row_vectors(self.geometry);
            },
            rvp::reference_internal, "Get a numpy view of the feature geometry")
        .def(
            "to_numpy",
            [](const mapbox::geojson::feature &self) -> RowVectors {
                return as_row_vectors(self.geometry);
            },
            "Convert the feature geometry to a numpy array") //
        .def(
            "__call__",
            [](const mapbox::geojson::feature &self) {
                return to_python(self);
            },
            "Convert the feature to a Python dict")
        .def(
            "from_rapidjson",
            [](mapbox::geojson::feature &self,
               const RapidjsonValue &json) -> mapbox::geojson::feature & {
                self = mapbox::geojson::convert<mapbox::geojson::feature>(json);
                return self;
            },
            rvp::reference_internal,
            "Initialize the feature from a RapidJSON value")
        .def(
            "from_rapidjson",
            [](mapbox::geojson::feature &self,
               const py::object &feature) -> mapbox::geojson::feature & {
                self = mapbox::geojson::convert<mapbox::geojson::feature>(
                    to_rapidjson(feature));
                return self;
            },
            rvp::reference_internal,
            "Initialize the feature from a Python object")
        .def(
            "to_rapidjson",
            [](const mapbox::geojson::feature &self) {
                RapidjsonAllocator allocator;
                return mapbox::geojson::convert(self, allocator);
            },
            "Convert the feature to a RapidJSON value")
        .def(
            "from_geobuf",
            [](mapbox::geojson::feature &self,
               const std::string &bytes) -> mapbox::geojson::feature & {
                auto geojson = mapbox::geobuf::Decoder().decode(bytes);
                self = std::move(geojson.get<mapbox::geojson::feature>());
                return self;
            },
            rvp::reference_internal, "Initialize the feature from Geobuf bytes")
        .def(
            "to_geobuf",
            [](const mapbox::geojson::feature &self, //
               int precision, bool only_xy, std::optional<int> round_z) {
                auto bytes = mapbox::geobuf::Encoder(std::pow(10, precision),
                                                     only_xy, round_z)
                                 .encode(self);
                return py::bytes(bytes);
            },
            py::kw_only(),       //
            "precision"_a = 8,   //
            "only_xy"_a = false, //
            "round_z"_a = std::nullopt, "Convert the feature to Geobuf bytes")
        .def(
            "load",
            [](mapbox::geojson::feature &self,
               const std::string &path) -> mapbox::geojson::feature & {
                if (endswith(path, ".pbf")) {
                    auto bytes = mapbox::geobuf::load_bytes(path);
                    auto geojson = mapbox::geobuf::Decoder().decode(bytes);
                    self = std::move(geojson.get<mapbox::geojson::feature>());
                    return self;
                }
                auto json = load_json(path);
                self = mapbox::geojson::convert<mapbox::geojson::feature>(json);
                return self;
            },
            rvp::reference_internal,
            "Load a feature from a file (GeoJSON or Geobuf)")
        .def(
            "dump",
            [](const mapbox::geojson::feature &self, //
               const std::string &path,              //
               bool indent,                          //
               bool sort_keys,                       //
               int precision,                        //
               bool only_xy) {
                if (endswith(path, ".pbf")) {
                    auto bytes = mapbox::geobuf::Encoder(
                                     std::pow(10, precision), only_xy)
                                     .encode(self);
                    return mapbox::geobuf::dump_bytes(path, bytes);
                }
                RapidjsonAllocator allocator;
                auto json = mapbox::geojson::convert(self, allocator);
                sort_keys_inplace(json);
                return dump_json(path, json, indent);
            },
            "path"_a, py::kw_only(), //
            "indent"_a = false,      //
            "sort_keys"_a = false,   //
            "precision"_a = 8,       //
            "only_xy"_a = false,
            "Dump the feature to a file (GeoJSON or Geobuf)")
        //
        copy_deepcopy_clone(mapbox::geojson::feature)
        //
        .def(
            "round",
            [](mapbox::geojson::feature &self, int lon, int lat,
               int alt) -> mapbox::geojson::feature & {
                round_coords(self.geometry, lon, lat, alt);
                return self;
            },
            py::kw_only(), "lon"_a = 8, "lat"_a = 8, "alt"_a = 3,
            rvp::reference_internal,
            "Round the coordinates of the feature geometry") //
        GEOMETRY_DEDUPLICATE_XYZ(feature)
        //
        ;

    py::bind_vector<std::vector<mapbox::geojson::feature>>(
        geojson, "FeatureList", py::module_local())
        //
        .def("__call__", [](const std::vector<mapbox::geojson::feature> &self) {
            return to_python(self);
        });
    auto fc_binding = py::class_<mapbox::geojson::feature_collection,
                                 std::vector<mapbox::geojson::feature>>(
        geojson, "FeatureCollection", py::module_local());
    fc_binding.def(py::init<>(), "Initialize an empty FeatureCollection")
        .def(py::init([](const mapbox::geojson::feature_collection &g) {
                 return g;
             }),
             "Initialize a FeatureCollection from another FeatureCollection")
        .def(py::init([](int N) {
                 mapbox::geojson::feature_collection fc;
                 fc.resize(N);
                 return fc;
             }),
             "N"_a, "Initialize a FeatureCollection with N empty features")
        .def(
            "resize",
            [](mapbox::geojson::feature_collection &self,
               int N) -> mapbox::geojson::feature_collection & {
                self.resize(N);
                return self;
            },
            rvp::reference_internal,
            "Resize the FeatureCollection to contain N features")
        //
        .def(
            "__call__",
            [](const mapbox::geojson::feature_collection &self) {
                return to_python(self);
            },
            "Convert the FeatureCollection to a Python dictionary") //
        .def(
            "round",
            [](mapbox::geojson::feature_collection &self, int lon, int lat,
               int alt) -> mapbox::geojson::feature_collection & {
                for (auto &f : self) {
                    round_coords(f.geometry, lon, lat, alt);
                }
                return self;
            },
            py::kw_only(), "lon"_a = 8, "lat"_a = 8, "alt"_a = 3,
            rvp::reference_internal,
            "Round the coordinates of all features in the collection")
            GEOMETRY_DEDUPLICATE_XYZ(feature_collection)
        // round
        //
        .def(
            "from_rapidjson",
            [](mapbox::geojson::feature_collection &self,
               const RapidjsonValue &json)
                -> mapbox::geojson::feature_collection & {
                self =
                    std::move(mapbox::geojson::convert(json)
                                  .get<mapbox::geojson::feature_collection>());
                return self;
            },
            rvp::reference_internal,
            "Load the FeatureCollection from a RapidJSON value")
        .def(
            "to_rapidjson",
            [](const mapbox::geojson::feature_collection &self) {
                RapidjsonAllocator allocator;
                auto json = mapbox::geojson::convert(self, allocator);
                return json;
            },
            "Convert the FeatureCollection to a RapidJSON value")
        //
        .def(
            "from_geobuf",
            [](mapbox::geojson::feature_collection &self,
               const std::string &bytes)
                -> mapbox::geojson::feature_collection & {
                auto geojson = mapbox::geobuf::Decoder().decode(bytes);
                self = std::move(
                    geojson.get<mapbox::geojson::feature_collection>());
                return self;
            },
            rvp::reference_internal,
            "Load the FeatureCollection from Geobuf bytes")
        .def(
            "to_geobuf",
            [](const mapbox::geojson::feature_collection &self, //
               int precision, bool only_xy, std::optional<int> round_z) {
                auto bytes = mapbox::geobuf::Encoder(std::pow(10, precision),
                                                     only_xy, round_z)
                                 .encode(self);
                return py::bytes(bytes);
            },
            py::kw_only(),       //
            "precision"_a = 8,   //
            "only_xy"_a = false, //
            "round_z"_a = std::nullopt,
            "Convert the FeatureCollection to Geobuf bytes")
        .def(
            "load",
            [](mapbox::geojson::feature_collection &self,
               const std::string &path)
                -> mapbox::geojson::feature_collection & {
                if (endswith(path, ".pbf")) {
                    auto bytes = mapbox::geobuf::load_bytes(path);
                    auto geojson = mapbox::geobuf::Decoder().decode(bytes);
                    self = std::move(
                        geojson.get<mapbox::geojson::feature_collection>());
                    return self;
                }
                auto json = load_json(path);
                self =
                    std::move(mapbox::geojson::convert(json)
                                  .get<mapbox::geojson::feature_collection>());
                return self;
            },
            rvp::reference_internal,
            "Load the FeatureCollection from a file (GeoJSON or Geobuf)")
        .def(
            "dump",
            [](const mapbox::geojson::feature_collection &self,
               const std::string &path, //
               bool indent,             //
               bool sort_keys,          //
               int precision,           //
               bool only_xy) {
                if (endswith(path, ".pbf")) {
                    auto bytes = mapbox::geobuf::Encoder(
                                     std::pow(10, precision), only_xy)
                                     .encode(self);
                    return mapbox::geobuf::dump_bytes(path, bytes);
                }
                RapidjsonAllocator allocator;
                auto json = mapbox::geojson::convert(self, allocator);
                sort_keys_inplace(json);
                return dump_json(path, json, indent);
            },
            "path"_a, py::kw_only(), //
            "indent"_a = false,      //
            "sort_keys"_a = false,   //
            "precision"_a = 8,       //
            "only_xy"_a = false,
            "Dump the FeatureCollection to a file (GeoJSON or Geobuf)")
        //
        copy_deepcopy_clone(mapbox::geojson::feature_collection)
        //
        BIND_PY_FLUENT_ATTRIBUTE(mapbox::geojson::feature_collection, //
                                 PropertyMap,                         //
                                 custom_properties)                   //
        .def(
            "keys",
            [](mapbox::geojson::feature_collection &self) {
                return py::make_key_iterator(self.custom_properties);
            },
            py::keep_alive<0, 1>(),
            "Return an iterator over the keys of custom properties")
        .def(
            "values",
            [](mapbox::geojson::feature_collection &self) {
                return py::make_value_iterator(self.custom_properties);
            },
            py::keep_alive<0, 1>(),
            "Return an iterator over the values of custom properties")
        .def(
            "items",
            [](mapbox::geojson::feature_collection &self) {
                return py::make_iterator(self.custom_properties);
            },
            py::keep_alive<0, 1>(),
            "Return an iterator over the items of custom properties")
        .def(
            "__getitem__",
            [](mapbox::geojson::feature_collection &self,
               const std::string &key) -> mapbox::geojson::value * {
                // don't try "type", "features"
                auto &props = self.custom_properties;
                auto itr = props.find(key);
                if (itr == props.end()) {
                    return nullptr;
                }
                return &itr->second;
            },
            rvp::reference_internal, "Get a custom property by key")
        .def(
            "__setitem__",
            [](mapbox::geojson::feature_collection &self,
               const std::string &key, const py::object &value) {
                if (key == "type" || key == "features") {
                    throw pybind11::key_error(key);
                }
                self.custom_properties[key] = to_geojson_value(value);
                return value;
            },
            "Set a custom property")
        .def(
            "__delitem__",
            [](mapbox::geojson::feature_collection &self,
               const std::string &key) {
                return self.custom_properties.erase(key);
            },
            "Delete a custom property");
    //
    ;

    // copied from stl_bind.h
    using Vector = mapbox::geojson::feature_collection;
    using T = typename Vector::value_type;
    using SizeType = typename Vector::size_type;
    using DiffType = typename Vector::difference_type;

    auto wrap_i = [](DiffType i, SizeType n) {
        if (i < 0) {
            i += n;
        }
        if (i < 0 || (SizeType)i >= n) {
            throw py::index_error();
        }
        return i;
    };

    auto cl = fc_binding;
    cl.def(
        "__getitem__",
        [wrap_i](Vector &v, DiffType i) -> T & {
            i = wrap_i(i, v.size());
            return v[(SizeType)i];
        },
        rvp::reference_internal,
        "Get a feature from the collection by index"); // ref + keepalive
    cl.def(
        "__setitem__",
        [wrap_i](Vector &v, DiffType i, const T &t) {
            i = wrap_i(i, v.size());
            v[(SizeType)i] = t;
        },
        "Set a feature in the collection at the specified index");
    cl.def(
        "__getitem__",
        [](const Vector &v, const py::slice &slice) -> Vector * {
            size_t start = 0, stop = 0, step = 0, slicelength = 0;

            if (!slice.compute(v.size(), &start, &stop, &step, &slicelength)) {
                throw py::error_already_set();
            }

            auto *seq = new Vector();
            seq->reserve((size_t)slicelength);

            for (size_t i = 0; i < slicelength; ++i) {
                seq->push_back(v[start]);
                start += step;
            }
            return seq;
        },
        py::arg("s"), "Retrieve list elements using a slice object");

    fc_binding.def(
        "__setitem__",
        [](Vector &v, const py::slice &slice, const Vector &value) {
            size_t start = 0, stop = 0, step = 0, slicelength = 0;
            if (!slice.compute(v.size(), &start, &stop, &step, &slicelength)) {
                throw py::error_already_set();
            }

            if (slicelength != value.size()) {
                throw std::runtime_error("Left and right hand size of slice "
                                         "assignment have different sizes!");
            }

            for (size_t i = 0; i < slicelength; ++i) {
                v[start] = value[i];
                start += step;
            }
        },
        "Assign list elements using a slice object");

    cl.def(
        "__delitem__",
        [wrap_i](Vector &v, DiffType i) {
            i = wrap_i(i, v.size());
            v.erase(v.begin() + i);
        },
        "Delete the list elements at index ``i``");

    cl.def(
        "__delitem__",
        [](Vector &v, const py::slice &slice) {
            size_t start = 0, stop = 0, step = 0, slicelength = 0;

            if (!slice.compute(v.size(), &start, &stop, &step, &slicelength)) {
                throw py::error_already_set();
            }

            if (step == 1 && false) {
                v.erase(v.begin() + (DiffType)start,
                        v.begin() + DiffType(start + slicelength));
            } else {
                for (size_t i = 0; i < slicelength; ++i) {
                    v.erase(v.begin() + DiffType(start));
                    start += step - 1;
                }
            }
        },
        "Delete list elements using a slice object");
}
} // namespace cubao

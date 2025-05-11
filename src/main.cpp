// https://github.com/microsoft/vscode-cpptools/issues/9692
#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include <pybind11/eigen.h>
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "geobuf/geobuf.hpp"
#include "geobuf/geobuf_index.hpp"
#include "geobuf/planet.hpp"
#include "geobuf/pybind11_helpers.hpp"

#include <optional>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
using namespace pybind11::literals;
using rvp = py::return_value_policy;

#define CUBAO_ARGV_DEFAULT_NONE(argv) py::arg_v(#argv, std::nullopt, "None")

namespace cubao
{
void bind_rapidjson(py::module &m);
void bind_geojson(py::module &m);
void bind_crs_transform(py::module &m);
} // namespace cubao

PYBIND11_MODULE(_core, m)
{
    using namespace mapbox::geobuf;

    cubao::bind_rapidjson(m);
    auto geojson = m.def_submodule("geojson");
    cubao::bind_geojson(geojson);
    auto tf = m.def_submodule("tf");
    cubao::bind_crs_transform(tf);

    m.def(
         "normalize_json",
         [](const std::string &input, const std::string &output, bool indent,
            bool sort_keys, bool denoise_double_0, bool strip_geometry_z_0,
            std::optional<int> round_non_geojson,
            std::optional<int> round_geojson_non_geometry,
            const std::optional<std::array<int, 3>> &round_geojson_geometry) {
             auto json = mapbox::geobuf::load_json(input);
             cubao::normalize_json(json,                       //
                                   sort_keys,                  //
                                   round_geojson_non_geometry, //
                                   round_geojson_geometry,     //
                                   round_non_geojson,          //
                                   denoise_double_0,           //
                                   strip_geometry_z_0);
             return mapbox::geobuf::dump_json(output, json, indent);
         },
         "input_path"_a, "output_path"_a,    //
         py::kw_only(),                      //
         "indent"_a = true,                  //
         "sort_keys"_a = true,               //
         "denoise_double_0"_a = true,        //
         "strip_geometry_z_0"_a = true,      //
         "round_non_geojson"_a = 3,          //
         "round_geojson_non_geometry"_a = 3, //
         "round_geojson_geometry"_a = std::array<int, 3>{8, 8, 3},
         R"docstring(
         Normalize JSON file.

         Args:
             input_path (str): Path to input JSON file.
             output_path (str): Path to output normalized JSON file.
             indent (bool, optional): Whether to indent the output JSON. Defaults to True.
             sort_keys (bool, optional): Whether to sort object keys. Defaults to True.
             denoise_double_0 (bool, optional): Whether to remove trailing zeros from doubles. Defaults to True.
             strip_geometry_z_0 (bool, optional): Whether to strip Z coordinate if it's 0. Defaults to True.
             round_non_geojson (int, optional): Number of decimal places to round non-GeoJSON numbers. Defaults to 3.
             round_geojson_non_geometry (int, optional): Number of decimal places to round GeoJSON non-geometry numbers. Defaults to 3.
             round_geojson_geometry (array of 3 ints, optional): Number of decimal places to round GeoJSON geometry coordinates. Defaults to [8, 8, 3].

         Returns:
             None
         )docstring")
        .def(
            "normalize_json",
            [](RapidjsonValue &json, bool sort_keys, bool denoise_double_0,
               bool strip_geometry_z_0, std::optional<int> round_non_geojson,
               std::optional<int> round_geojson_non_geometry,
               const std::optional<std::array<int, 3>> &round_geojson_geometry)
                -> RapidjsonValue & {
                cubao::normalize_json(json,                       //
                                      sort_keys,                  //
                                      round_geojson_non_geometry, //
                                      round_geojson_geometry,     //
                                      round_non_geojson,          //
                                      denoise_double_0,           //
                                      strip_geometry_z_0);
                return json;
            },
            "json"_a,
            py::kw_only(),                 //
            "sort_keys"_a = true,          //
            "denoise_double_0"_a = true,   //
            "strip_geometry_z_0"_a = true, //
            "round_non_geojson"_a = 3,     //
            "round_geojson_non_geometry"_a = 3,
            "round_geojson_geometry"_a = std::array<int, 3>{8, 8, 3},
            rvp::reference_internal,
            R"docstring(
            Normalize JSON object in-place.

            Args:
                json (RapidjsonValue): JSON object to normalize.
                sort_keys (bool, optional): Whether to sort object keys. Defaults to True.
                denoise_double_0 (bool, optional): Whether to remove trailing zeros from doubles. Defaults to True.
                strip_geometry_z_0 (bool, optional): Whether to strip Z coordinate if it's 0. Defaults to True.
                round_non_geojson (int, optional): Number of decimal places to round non-GeoJSON numbers. Defaults to 3.
                round_geojson_non_geometry (int, optional): Number of decimal places to round GeoJSON non-geometry numbers. Defaults to 3.
                round_geojson_geometry (array of 3 ints, optional): Number of decimal places to round GeoJSON geometry coordinates. Defaults to [8, 8, 3].

            Returns:
                RapidjsonValue: Reference to the normalized JSON object.
            )docstring");

    m.def(
        "is_subset_of",
        [](const std::string &path1, const std::string &path2) {
            auto json1 = mapbox::geobuf::load_json(path1);
            auto json2 = mapbox::geobuf::load_json(path2);
            return cubao::is_subset_of(json1, json2);
        },
        "path1"_a, "path2"_a,
        R"docstring(
        Check if the JSON at path1 is a subset of the JSON at path2.

        Args:
            path1 (str): Path to the first JSON file.
            path2 (str): Path to the second JSON file.

        Returns:
            bool: True if the first JSON is a subset of the second, False otherwise.
        )docstring");

    m.def(
        "str2json2str",
        [](const std::string &json_string, //
           bool indent,                    //
           bool sort_keys) -> std::optional<std::string> {
            auto json = mapbox::geobuf::parse(json_string);
            if (json.IsNull()) {
                return {};
            }
            if (sort_keys) {
                mapbox::geobuf::sort_keys_inplace(json);
            }
            return mapbox::geobuf::dump(json, indent);
        },
        "json_string"_a,    //
        py::kw_only(),      //
        "indent"_a = false, //
        "sort_keys"_a = false,
        R"docstring(
        Convert JSON string to JSON object and back to string.

        Args:
            json_string (str): Input JSON string.
            indent (bool, optional): Whether to indent the output JSON. Defaults to False.
            sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

        Returns:
            Optional[str]: Converted JSON string, or None if input is invalid.
        )docstring");

    m.def(
        "str2geojson2str",
        [](const std::string &json_string, //
           bool indent,                    //
           bool sort_keys) -> std::optional<std::string> {
            auto json = mapbox::geobuf::parse(json_string);
            if (json.IsNull()) {
                return {};
            }
            auto geojson = mapbox::geobuf::json2geojson(json);
            auto json_output = mapbox::geobuf::geojson2json(geojson);
            if (sort_keys) {
                mapbox::geobuf::sort_keys_inplace(json_output);
            }
            return mapbox::geobuf::dump(json_output, indent);
        },
        "json_string"_a,    //
        py::kw_only(),      //
        "indent"_a = false, //
        "sort_keys"_a = false,
        R"docstring(
        Convert JSON string to GeoJSON object and back to JSON string.

        Args:
            json_string (str): Input JSON string.
            indent (bool, optional): Whether to indent the output JSON. Defaults to False.
            sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

        Returns:
            Optional[str]: Converted GeoJSON string, or None if input is invalid.
        )docstring");

    m.def(
        "pbf_decode",
        [](const std::string &pbf_bytes, const std::string &indent)
            -> std::string { return Decoder::to_printable(pbf_bytes, indent); },
        "pbf_bytes"_a, //
        py::kw_only(), //
        "indent"_a = "",
        R"docstring(
        Decode Protocol Buffer (PBF) bytes to a printable string.

        Args:
            pbf_bytes (str): Input PBF bytes.
            indent (str, optional): Indentation string. Defaults to "".

        Returns:
            str: Decoded and formatted PBF content as a string.
        )docstring");

    py::class_<Encoder>(m, "Encoder", py::module_local(), py::dynamic_attr()) //
        .def(py::init<uint32_t, bool, std::optional<int>>(),                  //
             py::kw_only(),
             "max_precision"_a = static_cast<uint32_t>(
                 std::pow(10, MAPBOX_GEOBUF_DEFAULT_PRECISION)),
             "only_xy"_a = false, //
             "round_z"_a = std::nullopt,
             R"docstring(
             Initialize an Encoder object.

             Args:
                 max_precision (int): Maximum precision for coordinate encoding. Default is 10^8.
                 only_xy (bool): If True, only encode X and Y coordinates. Default is False.
                 round_z (Optional[int]): Number of decimal places to round Z coordinates. Default is None.
             )docstring")
        //
        .def("max_precision", &Encoder::__maxPrecision,
             "Get the maximum precision used for coordinate encoding.")
        .def("only_xy", &Encoder::__onlyXY,
             "Check if only X and Y coordinates are being encoded.")
        .def(
            "round_z", &Encoder::__roundZ,
            "Get the number of decimal places used for rounding Z coordinates.")
        .def("dim", &Encoder::__dim,
             "Get the dimension of the encoded coordinates (2 or 3).")
        .def("e", &Encoder::__e,
             "Get the encoding factor used for coordinate precision.")
        .def("keys", &Encoder::__keys, "Get keys used in the encoded data.")
        .def(
            "encode",
            [](Encoder &self, const mapbox::geojson::geojson &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "geojson"_a,
            R"docstring(
            Encode GeoJSON to Protocol Buffer (PBF) bytes.

            Args:
                geojson (mapbox::geojson::geojson): Input GeoJSON object.

            Returns:
                bytes: Encoded PBF bytes.
            )docstring")
        .def(
            "encode",
            [](Encoder &self,
               const mapbox::geojson::feature_collection &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "features"_a,
            R"docstring(
            Encode GeoJSON FeatureCollection to Protocol Buffer (PBF) bytes.

            Args:
                features (mapbox::geojson::feature_collection): Input GeoJSON FeatureCollection.

            Returns:
                bytes: Encoded PBF bytes.
            )docstring")
        .def(
            "encode",
            [](Encoder &self, const mapbox::geojson::feature &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "feature"_a,
            R"docstring(
            Encode GeoJSON Feature to Protocol Buffer (PBF) bytes.

            Args:
                feature (mapbox::geojson::feature): Input GeoJSON Feature.

            Returns:
                bytes: Encoded PBF bytes.
            )docstring")
        .def(
            "encode",
            [](Encoder &self, const mapbox::geojson::geometry &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "geometry"_a,
            R"docstring(
            Encode GeoJSON Geometry to Protocol Buffer (PBF) bytes.

            Args:
                geometry (mapbox::geojson::geometry): Input GeoJSON Geometry.

            Returns:
                bytes: Encoded PBF bytes.
            )docstring")
        .def(
            "encode",
            [](Encoder &self, const RapidjsonValue &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "geojson"_a,
            R"docstring(
            Encode RapidjsonValue GeoJSON to Protocol Buffer (PBF) bytes.

            Args:
                geojson (RapidjsonValue): Input RapidjsonValue GeoJSON object.

            Returns:
                bytes: Encoded PBF bytes.
            )docstring")
        .def(
            "encode",
            [](Encoder &self, const py::object &geojson) {
                if (py::isinstance<py::str>(geojson)) {
                    auto str = geojson.cast<std::string>();
                    return py::bytes(self.encode(str));
                }
                return py::bytes(self.encode(cubao::to_rapidjson(geojson)));
            },
            "geojson"_a,
            R"docstring(
            Encode Python object GeoJSON to Protocol Buffer (PBF) bytes.

            Args:
                geojson (object): Input Python object representing GeoJSON.

            Returns:
                bytes: Encoded PBF bytes.
            )docstring")
        .def("encode",
             py::overload_cast<const std::string &, const std::string &>(
                 &Encoder::encode),
             py::kw_only(), "geojson"_a, "geobuf"_a,
             R"docstring(
             Encode GeoJSON file to Protocol Buffer (PBF) file.

             Args:
                 geojson (str): Path to input GeoJSON file.
                 geobuf (str): Path to output PBF file.

             Returns:
                 Bool: succ or not.
             )docstring")
        //
        ;

    py::class_<Decoder>(m, "Decoder", py::module_local(), py::dynamic_attr()) //
        .def(py::init<>(),
             R"docstring(
             Initialize a Decoder object.
             )docstring")
        //
        .def("precision", &Decoder::precision,
             R"docstring(
             Get the precision used in the decoding process.

             Returns:
                 int: The precision value.
             )docstring")
        .def("dim", &Decoder::__dim,
             R"docstring(
             Get the dimension of the coordinates in the decoded data.

             Returns:
                 int: The dimension value (2 for 2D, 3 for 3D).
             )docstring")
        .def(
            "decode",
            [](Decoder &self, const std::string &geobuf, bool indent,
               bool sort_keys) {
                return mapbox::geobuf::dump(self.decode(geobuf), indent,
                                            sort_keys);
            },
            "geobuf"_a, py::kw_only(), "indent"_a = false,
            "sort_keys"_a = false,
            R"docstring(
            Decode Protocol Buffer (PBF) bytes to GeoJSON string.

            Args:
                geobuf (str): Input PBF bytes.
                indent (bool, optional): Whether to indent the output JSON. Defaults to False.
                sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

            Returns:
                str: Decoded GeoJSON as a string.
            )docstring")
        .def(
            "decode_to_rapidjson",
            [](Decoder &self, const std::string &geobuf, bool sort_keys) {
                auto json = geojson2json(self.decode(geobuf));
                if (sort_keys) {
                    sort_keys_inplace(json);
                }
                return json;
            },
            "geobuf"_a, py::kw_only(), "sort_keys"_a = false,
            R"docstring(
            Decode Protocol Buffer (PBF) bytes to RapidjsonValue GeoJSON.

            Args:
                geobuf (str): Input PBF bytes.
                sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

            Returns:
                RapidjsonValue: Decoded GeoJSON as a RapidjsonValue object.
            )docstring")
        .def(
            "decode_to_geojson",
            [](Decoder &self, const std::string &geobuf) {
                return self.decode(geobuf);
            },
            "geobuf"_a,
            R"docstring(
            Decode Protocol Buffer (PBF) bytes to mapbox::geojson::geojson object.

            Args:
                geobuf (str): Input PBF bytes.

            Returns:
                mapbox::geojson::geojson: Decoded GeoJSON object.
            )docstring")
        .def(
            "decode",
            [](Decoder &self,              //
               const std::string &geobuf,  //
               const std::string &geojson, //
               bool indent,                //
               bool sort_keys) {
                return self.decode(geobuf, geojson, indent, sort_keys);
            },
            py::kw_only(),      //
            "geobuf"_a,         //
            "geojson"_a,        //
            "indent"_a = false, //
            "sort_keys"_a = false,
            R"docstring(
            Decode Protocol Buffer (PBF) file to GeoJSON file.

            Args:
                geobuf (str): Path to input PBF file.
                geojson (str): Path to output GeoJSON file.
                indent (bool, optional): Whether to indent the output JSON. Defaults to False.
                sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

            Returns:
                None
            )docstring")
        .def("keys", &Decoder::__keys,
             R"docstring(
             Get the keys of the decoded Protocol Buffer (PBF) data.

             Returns:
                 list: A list of strings representing the keys in the decoded PBF data.
             )docstring")
        //
        .def("decode_header",
             py::overload_cast<const std::string &>(&Decoder::decode_header),
             "bytes"_a,
             R"docstring(
             Decode Protocol Buffer (PBF) header.

             Args:
                 bytes (str): Input PBF bytes.

             Returns:
                 dict: Decoded header information.
             )docstring")
        .def("decode_feature",
             py::overload_cast<const std::string &, bool, bool>(
                 &Decoder::decode_feature),
             "bytes"_a, "only_geometry"_a = false, "only_properties"_a = false,
             R"docstring(
             Decode Protocol Buffer (PBF) feature.

             Args:
                 bytes (str): Input PBF bytes.
                 only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
                 only_properties (bool, optional): Whether to decode only properties. Defaults to False.

             Returns:
                 mapbox::geojson::feature: Decoded GeoJSON feature.
             )docstring")
        .def("decode_non_features",
             py::overload_cast<const std::string &>(
                 &Decoder::decode_non_features),
             R"docstring(
             Decode non-feature elements from Protocol Buffer (PBF) bytes.

             Args:
                 bytes (str): Input PBF bytes.

             Returns:
                 dict: Decoded non-feature elements.
             )docstring")
        .def("offsets", &Decoder::__offsets,
             R"docstring(
             Get the offsets of features in the Protocol Buffer (PBF) file.

             Returns:
                 list: A list of integer offsets representing the starting positions of features in the PBF file.
             )docstring")
        //
        ;

    using namespace FlatGeobuf;
    py::class_<NodeItem>(m, "NodeItem", py::module_local())
        .def_property_readonly(
            "min_x", [](const NodeItem &self) { return self.minX; },
            "Get the minimum X coordinate of the node")
        .def_property_readonly(
            "min_y", [](const NodeItem &self) { return self.minY; },
            "Get the minimum Y coordinate of the node")
        .def_property_readonly(
            "max_x", [](const NodeItem &self) { return self.maxX; },
            "Get the maximum X coordinate of the node")
        .def_property_readonly(
            "max_y", [](const NodeItem &self) { return self.maxY; },
            "Get the maximum Y coordinate of the node")
        .def_property_readonly(
            "offset", [](const NodeItem &self) { return self.offset; },
            "Get the offset of the node")
        .def_property_readonly(
            "width", [](const NodeItem &self) { return self.width(); },
            "Get the width of the node's bounding box")
        .def_property_readonly(
            "height", [](const NodeItem &self) { return self.height(); },
            "Get the height of the node's bounding box")
        //
        .def("expand", &NodeItem::expand, "other"_a,
             "Expand the node's bounding box to include another node")
        .def("intersects", &NodeItem::intersects, "other"_a,
             "Check if this node's bounding box intersects with another node's "
             "bounding box")
        .def(py::self == py::self, "Check if two nodes are equal")
        .def(py::self != py::self, "Check if two nodes are not equal")
        .def(
            "to_numpy",
            [](const NodeItem &self) -> Eigen::Vector4d {
                return {self.minX, self.minY, self.maxX, self.maxY};
            },
            "Convert the node's bounding box to a numpy array [minX, minY, "
            "maxX, maxY]")
        //
        ;

    using PackedRTree = FlatGeobuf::PackedRTree;
    py::class_<PackedRTree>(m, "PackedRTree", py::module_local(),
                            py::dynamic_attr())
        .def(
            "search",
            [](const PackedRTree &self, double minX, double minY, double maxX,
               double maxY) {
                auto hits = self.search(minX, minY, maxX, maxY);
                std::vector<size_t> ret;
                ret.reserve(hits.size());
                for (auto &h : hits) {
                    ret.push_back(h.offset);
                }
                return ret;
            },
            "min_x"_a, "min_y"_a, "max_x"_a, "max_y"_a,
            R"docstring(
            Search for items within the given bounding box.

            Args:
                min_x (float): Minimum X coordinate of the bounding box.
                min_y (float): Minimum Y coordinate of the bounding box.
                max_x (float): Maximum X coordinate of the bounding box.
                max_y (float): Maximum Y coordinate of the bounding box.

            Returns:
                list: List of offsets of items within the bounding box.
            )docstring")
        .def_property_readonly(
            "size", [](const PackedRTree &self) { return self.size(); })
        .def_property_readonly("extent",
                               [](const PackedRTree &self) {
                                   auto bbox = self.getExtent();
                                   return Eigen::Vector4d(bbox.minX, bbox.minY,
                                                          bbox.maxX, bbox.maxY);
                               })
        .def_property_readonly(
            "num_items",
            [](const PackedRTree &self) { return self.getNumItems(); })
        .def_property_readonly(
            "num_nodes",
            [](const PackedRTree &self) { return self.getNumNodes(); })
        .def_property_readonly(
            "node_size",
            [](const PackedRTree &self) { return self.getNodeSize(); })
        //
        ;

    using Planet = cubao::Planet;
    py::class_<Planet>(m, "Planet", py::module_local(), py::dynamic_attr())
        .def(py::init<>(), R"docstring(
            Initialize an empty Planet object.
        )docstring")
        .def(py::init<const mapbox::geojson::feature_collection &>(),
             R"docstring(
            Initialize a Planet object with a feature collection.

            Args:
                feature_collection (mapbox::geojson::feature_collection): The feature collection to initialize with.
        )docstring")
        .def("features", py::overload_cast<>(&Planet::features, py::const_),
             rvp::reference_internal, R"docstring(
            Get the features of the Planet object.

            Returns:
                mapbox::geojson::feature_collection: The features of the Planet object.
        )docstring")
        .def("features",
             py::overload_cast<const mapbox::geojson::feature_collection &>(
                 &Planet::features),
             R"docstring(
            Set the features of the Planet object.

            Args:
                feature_collection (mapbox::geojson::feature_collection): The new feature collection to set.
        )docstring")
        .def("build", &Planet::build, py::kw_only(),
             "per_line_segment"_a = false, "force"_a = false,
             R"docstring(
             Build the spatial index for the features.

             Args:
                 per_line_segment (bool, optional): Whether to index each line segment separately. Defaults to False.
                 force (bool, optional): Whether to force rebuilding the index. Defaults to False.

             Returns:
                 None
             )docstring")
        .def("query", &Planet::query, "min"_a, "max"_a,
             R"docstring(
             Query features within the given bounding box.

             Args:
                 min (array-like): Minimum coordinates of the bounding box.
                 max (array-like): Maximum coordinates of the bounding box.

             Returns:
                 list: List of features within the bounding box.
             )docstring")
        .def("packed_rtree", &Planet::packed_rtree, rvp::reference_internal,
             R"docstring(
             Get the packed R-tree of the Planet object.

             Returns:
                 FlatGeobuf::PackedRTree: The packed R-tree of the Planet object.
             )docstring")
        .def("copy", &Planet::copy,
             R"docstring(
             Create a deep copy of the Planet object.

             Returns:
                 Planet: A new Planet object that is a deep copy of the current one.
             )docstring")
        .def("crop", &Planet::crop, "polygon"_a, py::kw_only(),
             "clipping_mode"_a = "longest", //
             "strip_properties"_a = false,  //
             "is_wgs84"_a = true,
             R"docstring(
             Crop features using a polygon.

             Args:
                 polygon (mapbox::geojson::polygon): Polygon to crop with.
                 clipping_mode (str, optional): Clipping mode. Defaults to "longest".
                 strip_properties (bool, optional): Whether to strip properties from cropped features. Defaults to False.
                 is_wgs84 (bool, optional): Whether the coordinates are in WGS84. Defaults to True.

             Returns:
                 Planet: New Planet object with cropped features.
             )docstring")
        //
        ;

    using GeobufIndex = cubao::GeobufIndex;
    py::class_<GeobufIndex>(m, "GeobufIndex", py::module_local(),
                            py::dynamic_attr()) //
        .def(py::init<>(), R"docstring(
            Default constructor for GeobufIndex.
        )docstring")
        // attrs
        .def_property_readonly(
            "header_size",
            [](const GeobufIndex &self) { return self.header_size; },
            R"docstring(
            Get the size of the header in bytes.

            Returns:
                int: The size of the header in bytes.
            )docstring")
        .def_property_readonly(
            "num_features",
            [](const GeobufIndex &self) { return self.num_features; },
            R"docstring(
            Get the number of features in the index.

            Returns:
                int: The number of features.
            )docstring")
        .def_property_readonly(
            "offsets", [](const GeobufIndex &self) { return self.offsets; },
            R"docstring(
            Get the offsets of features in the Geobuf file.

            Returns:
                list: A list of offsets for each feature.
            )docstring")
        .def_property_readonly(
            "ids", [](const GeobufIndex &self) { return self.ids; },
            R"docstring(
            Get the IDs of features in the index.

            Returns:
                list: A list of feature IDs.
            )docstring")
        .def_property_readonly(
            "packed_rtree",
            [](const GeobufIndex &self) -> const FlatGeobuf::PackedRTree * {
                if (!self.packed_rtree) {
                    return nullptr;
                }
                return &*self.packed_rtree;
            },
            rvp::reference_internal,
            R"docstring(
            Get the packed R-tree of the index.

            Returns:
                FlatGeobuf.PackedRTree: The packed R-tree of the index, or None if not available.
            )docstring")
        //
        .def("init", py::overload_cast<const std::string &>(&GeobufIndex::init),
             "index_bytes"_a,
             R"docstring(
             Initialize the GeobufIndex from index bytes.

             Args:
                 index_bytes (str): Bytes containing the index information.

             Returns:
                 None
             )docstring")
        //
        .def("mmap_init",
             py::overload_cast<const std::string &, const std::string &>(
                 &GeobufIndex::mmap_init),
             "index_path"_a, "geobuf_path"_a,
             R"docstring(
             Initialize the GeobufIndex using memory-mapped files.

             Args:
                 index_path (str): Path to the index file.
                 geobuf_path (str): Path to the Geobuf file.

             Returns:
                 None
             )docstring")
        .def("mmap_init",
             py::overload_cast<const std::string &>(&GeobufIndex::mmap_init),
             "geobuf_path"_a,
             R"docstring(
             Initialize the GeobufIndex using a memory-mapped Geobuf file.

             Args:
                 geobuf_path (str): Path to the Geobuf file.

             Returns:
                 None
             )docstring")
        //
        .def(
            "mmap_bytes",
            [](const GeobufIndex &self, size_t offset,
               size_t length) -> std::optional<py::bytes> {
                auto bytes = self.mmap_bytes(offset, length);
                if (!bytes) {
                    return {};
                }
                return py::bytes(*bytes);
            },
            "offset"_a, "length"_a,
            R"docstring(
            Read bytes from the memory-mapped file.

            Args:
                offset (int): Offset in the file.
                length (int): Number of bytes to read.

            Returns:
                Optional[bytes]: Read bytes, or None if reading failed.
            )docstring")
        //
        .def("decode_feature",
             py::overload_cast<uint32_t, bool, bool>(
                 &GeobufIndex::decode_feature),
             "index"_a, py::kw_only(), "only_geometry"_a = false,
             "only_properties"_a = false,
             R"docstring(
             Decode a feature from the Geobuf file.

             Args:
                 index (int): Index of the feature to decode.
                 only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
                 only_properties (bool, optional): Whether to decode only properties. Defaults to False.

             Returns:
                 mapbox::geojson::feature: Decoded feature.
             )docstring")
        .def("decode_feature",
             py::overload_cast<const std::string &, bool, bool>(
                 &GeobufIndex::decode_feature),
             "bytes"_a, py::kw_only(), "only_geometry"_a = false,
             "only_properties"_a = false,
             R"docstring(
             Decode a feature from bytes.

             Args:
                 bytes (str): Bytes containing the feature data.
                 only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
                 only_properties (bool, optional): Whether to decode only properties. Defaults to False.

             Returns:
                 mapbox::geojson::feature: Decoded feature.
             )docstring")
        .def("decode_feature_of_id",
             py::overload_cast<const std::string &, bool, bool>(
                 &GeobufIndex::decode_feature),
             "id"_a, py::kw_only(), "only_geometry"_a = false,
             "only_properties"_a = false,
             R"docstring(
             Decode a feature by its ID.

             Args:
                 id (str): ID of the feature to decode.
                 only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
                 only_properties (bool, optional): Whether to decode only properties. Defaults to False.

             Returns:
                 mapbox::geojson::feature: Decoded feature.
             )docstring")
        .def("decode_features",
             py::overload_cast<const std::vector<int> &, bool, bool>(
                 &GeobufIndex::decode_features),
             "index"_a, py::kw_only(), "only_geometry"_a = false,
             "only_properties"_a = false,
             R"docstring(
             Decode multiple features from the Geobuf file.

             Args:
                 index (List[int]): List of indices of the features to decode.
                 only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
                 only_properties (bool, optional): Whether to decode only properties. Defaults to False.

             Returns:
                 List[mapbox::geojson::feature]: List of decoded features.
             )docstring")
        //
        .def("decode_non_features",
             py::overload_cast<const std::string &>(
                 &GeobufIndex::decode_non_features),
             "bytes"_a,
             R"docstring(
             Decode non-feature data from bytes.

             Args:
                 bytes (str): Bytes containing the non-feature data.

             Returns:
                 Dict: Decoded non-feature data.
             )docstring")
        .def("decode_non_features",
             py::overload_cast<>(&GeobufIndex::decode_non_features),
             R"docstring(
             Decode non-feature data from the Geobuf file.

             Returns:
                 Dict: Decoded non-feature data.
             )docstring")
        .def(
            "query",
            py::overload_cast<const Eigen::Vector2d &, const Eigen::Vector2d &>(
                &GeobufIndex::query, py::const_),
            R"docstring(
            Query features within a bounding box.

            Args:
                min_corner (Eigen::Vector2d): Minimum corner of the bounding box.
                max_corner (Eigen::Vector2d): Maximum corner of the bounding box.

            Returns:
                List[int]: List of indices of features within the bounding box.
            )docstring")
        //
        .def_static("indexing", &GeobufIndex::indexing, //
                    "input_geobuf_path"_a,              //
                    "output_index_path"_a,              //
                    py::kw_only(),                      //
                    "feature_id"_a = "@",               //
                    "packed_rtree"_a = "@",             //
                    R"docstring(
                    Create an index for a Geobuf file.

                    Args:
                        input_geobuf_path (str): Path to the input Geobuf file.
                        output_index_path (str): Path to save the output index file.
                        feature_id (str, optional): Feature ID field. Defaults to "@".
                        packed_rtree (str, optional): Packed R-tree option. Defaults to "@".

                    Returns:
                        None
                    )docstring")
        //
        ;

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}

#define CUBAO_STATIC_LIBRARY
#include "cubao/pybind11_crs_transform.hpp"

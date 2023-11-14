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
#include "geobuf/geobuf_plus.hpp"
#include "geobuf/planet.hpp"
#include "geobuf/pybind11_helpers.hpp"

#include <optional>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
using namespace pybind11::literals;
using rvp = py::return_value_policy;

namespace cubao
{
void bind_geojson(py::module &m);
void bind_rapidjson(py::module &m);
void bind_crs_transform(py::module &m);
} // namespace cubao

PYBIND11_MODULE(_pybind11_geobuf, m)
{
    using namespace mapbox::geobuf;

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
         "round_geojson_geometry"_a = std::array<int, 3>{8, 8, 3})
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
            rvp::reference_internal);

    m.def(
        "is_subset_of",
        [](const std::string &path1, const std::string &path2) {
            auto json1 = mapbox::geobuf::load_json(path1);
            auto json2 = mapbox::geobuf::load_json(path2);
            return cubao::is_subset_of(json1, json2);
        },
        "path1"_a, "path2"_a);

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
        "sort_keys"_a = false);

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
        "sort_keys"_a = false);

    m.def(
        "pbf_decode",
        [](const std::string &pbf_bytes, const std::string &indent)
            -> std::string { return Decoder::to_printable(pbf_bytes, indent); },
        "pbf_bytes"_a, //
        py::kw_only(), //
        "indent"_a = "");

    py::class_<Encoder>(m, "Encoder", py::module_local())    //
        .def(py::init<uint32_t, bool, std::optional<int>>(), //
             py::kw_only(),
             "max_precision"_a = static_cast<uint32_t>(
                 std::pow(10, MAPBOX_GEOBUF_DEFAULT_PRECISION)),
             "only_xy"_a = false, //
             "round_z"_a = std::nullopt)
        //
        .def("max_precision", &Encoder::__maxPrecision)
        .def("only_xy", &Encoder::__onlyXY)
        .def("round_z", &Encoder::__roundZ)
        .def("dim", &Encoder::__dim)
        .def("e", &Encoder::__e)
        .def("keys", &Encoder::__keys)
        .def(
            "encode",
            [](Encoder &self, const mapbox::geojson::geojson &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "geojson"_a)
        .def(
            "encode",
            [](Encoder &self,
               const mapbox::geojson::feature_collection &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "features"_a)
        .def(
            "encode",
            [](Encoder &self, const mapbox::geojson::feature &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "feature"_a)
        .def(
            "encode",
            [](Encoder &self, const mapbox::geojson::geometry &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "geometry"_a)
        .def(
            "encode",
            [](Encoder &self, const RapidjsonValue &geojson) {
                return py::bytes(self.encode(geojson));
            },
            "geojson"_a)
        .def(
            "encode",
            [](Encoder &self, const py::object &geojson) {
                if (py::isinstance<py::str>(geojson)) {
                    auto str = geojson.cast<std::string>();
                    return py::bytes(self.encode(str));
                }
                return py::bytes(self.encode(cubao::to_rapidjson(geojson)));
            },
            "geojson"_a)
        .def("encode",
             py::overload_cast<const std::string &, const std::string &>(
                 &Encoder::encode),
             py::kw_only(), "geojson"_a, "geobuf"_a)
        .def("keys", &Encoder::__keys)
        //
        ;

    py::class_<Decoder>(m, "Decoder", py::module_local()) //
        .def(py::init<>())
        //
        .def("precision", &Decoder::precision)
        .def("dim", &Decoder::__dim)
        .def(
            "decode",
            [](Decoder &self, const std::string &geobuf, bool indent,
               bool sort_keys) {
                return mapbox::geobuf::dump(self.decode(geobuf), indent,
                                            sort_keys);
            },
            "geobuf"_a, py::kw_only(), "indent"_a = false,
            "sort_keys"_a = false)
        .def(
            "decode_to_rapidjson",
            [](Decoder &self, const std::string &geobuf, bool sort_keys) {
                auto json = geojson2json(self.decode(geobuf));
                if (sort_keys) {
                    sort_keys_inplace(json);
                }
                return json;
            },
            "geobuf"_a, py::kw_only(), "sort_keys"_a = false)
        .def(
            "decode_to_geojson",
            [](Decoder &self, const std::string &geobuf) {
                return self.decode(geobuf);
            },
            "geobuf"_a)
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
            "sort_keys"_a = false)
        .def("keys", &Decoder::__keys)
        //
        ;

    auto geojson = m.def_submodule("geojson");
    cubao::bind_geojson(geojson);

    using Planet = cubao::Planet;
    py::class_<Planet>(m, "Planet", py::module_local())
        .def(py::init<>())
        .def(py::init<const mapbox::geojson::feature_collection &>())
        .def("features", py::overload_cast<>(&Planet::features, py::const_),
             rvp::reference_internal)
        .def("features",
             py::overload_cast<const mapbox::geojson::feature_collection &>(
                 &Planet::features))
        .def("build", &Planet::build, py::kw_only(),
             "per_line_segment"_a = false, "force"_a = false)
        .def("query", &Planet::query, "min"_a, "max"_a)
        .def("copy", &Planet::copy)
        .def("crop", &Planet::crop, "polygon"_a, py::kw_only(),
             "clipping_mode"_a = "longest", //
             "strip_properties"_a = false,  //
             "is_wgs84"_a = true)
        //
        ;

    using GeobufPlus = cubao::GeobufPlus;
    py::class_<GeobufPlus>(m, "GeobufPlus", py::module_local()) //
        .def(py::init<>())
        .def("mmap_init", &GeobufPlus::mmap_init, "index_path"_a,
             "geobuf_path"_a)
        .def("init_index", &GeobufPlus::init_index, "index_bytes"_a)
        .def("decode_feature",
             py::overload_cast<int>(&GeobufPlus::decode_feature), "index"_a)
        .def(
            "decode_feature",
            py::overload_cast<const std::string &>(&GeobufPlus::decode_feature),
            "bytes"_a)
        //
        .def_static(
            "encode",
            [](const std::string &ipath, const std::string &oidx,
               const std::string &opbf, uint8_t precision, bool only_xy,
               std::optional<int> round_z, int N) {
                auto features = GeobufPlus::encode(ipath, oidx, opbf, precision,
                                                   only_xy, round_z, N);
                std::vector<py::bytes> bytes_list;
                bytes_list.reserve(features.size());
                for (auto &f : features) {
                    bytes_list.push_back(py::bytes(f));
                }
                return bytes_list;
                // return py::bytes(features[idx]);
            },
            "input_geojson_path"_a, "output_index_path"_a,
            "output_geobuf_path"_a, py::kw_only(), "precision"_a = 8,
            "only_xy"_a = false, "round_z"_a = 3, "N"_a = 0)
        //
        ;

    cubao::bind_rapidjson(m);

    auto tf = m.def_submodule("tf");
    cubao::bind_crs_transform(tf);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}

#define CUBAO_STATIC_LIBRARY
#ifndef CUBAO_ARGV_DEFAULT_NONE
#define CUBAO_ARGV_DEFAULT_NONE(argv) py::arg_v(#argv, std::nullopt, "None")
#endif

#include "cubao/pybind11_crs_transform.hpp"

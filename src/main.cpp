// https://github.com/microsoft/vscode-cpptools/issues/9692
#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "geobuf/geobuf.hpp"
#include "geobuf/pybind11_helpers.hpp"

#include <optional>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
using namespace pybind11::literals;

namespace cubao
{
void bind_geojson(py::module &m);
void bind_rapidjson(py::module &m);
} // namespace cubao

PYBIND11_MODULE(_pybind11_geobuf, m)
{
    using namespace mapbox::geobuf;

    m.def(
        "normalize_json",
        [](const std::string &input, const std::string &output, bool indent,
           bool sort_keys) {
            auto json = mapbox::geobuf::load_json(input);
            if (sort_keys) {
                mapbox::geobuf::sort_keys_inplace(json);
            }
            return mapbox::geobuf::dump_json(output, json, indent);
        },
        "input_path"_a, "output_path"_a, //
        py::kw_only(), "indent"_a = true, "sort_keys"_a = true);

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

    py::class_<Encoder>(m, "Encoder", py::module_local()) //
        .def(py::init<uint32_t>(),                        //
             py::kw_only(),
             "max_precision"_a = std::pow(10, MAPBOX_GEOBUF_DEFAULT_PRECISION))
        //
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
        //
        ;

    auto geojson = m.def_submodule("geojson");
    cubao::bind_geojson(geojson);

    cubao::bind_rapidjson(m);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}

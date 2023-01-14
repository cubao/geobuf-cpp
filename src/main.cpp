// https://github.com/microsoft/vscode-cpptools/issues/9692
#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include <pybind11/eigen.h>
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>

#include "geobuf/geobuf.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(pybind11_geobuf, m)
{
    using Encoder = mapbox::geobuf::Encoder;
    py::class_<Encoder>(m, "Encoder", py::module_local()) //
        .def(py::init<uint32_t>(),                        //
             py::kw_only(),
             "max_precision"_a = std::pow(10, MAPBOX_GEOBUF_DEFAULT_PRECISION))
        //
        .def(
            "encode",
            [](Encoder &self, const std::string &geojson) {
                return py::bytes(self.encode(geojson));
            },
            py::kw_only(), "geojson"_a)
        .def("encode",
             py::overload_cast<const std::string &, const std::string &>(
                 &Encoder::encode),
             py::kw_only(), "geojson"_a, "geobuf"_a)
        //
        ;

    using Decoder = mapbox::geobuf::Decoder;
    py::class_<Decoder>(m, "Decoder", py::module_local()) //
        .def(py::init<>())
        //
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
            "decode",
            [](Decoder &self,              //
               const std::string &geobuf,  //
               const std::string &geojson, //
               bool indent,                //
               bool sort_keys) {
                return self.decode(geobuf, geojson, indent, sort_keys);
            },
            py::kw_only(), "geobuf"_a, "geojson"_a, //
            "indent"_a = false, "sort_keys"_a = false)
        //
        ;

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}

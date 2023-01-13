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

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}

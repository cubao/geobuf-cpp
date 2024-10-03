#pragma once

#include <mapbox/geojson.hpp>
#include <mapbox/geojson/rapidjson.hpp>

#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <fstream>
#include <iostream>

#include "geobuf/pybind11_helpers.hpp"
#include "geobuf/rapidjson_helpers.hpp"

namespace cubao
{
namespace py = pybind11;
using namespace pybind11::literals;
using rvp = py::return_value_policy;

using RapidjsonValue = mapbox::geojson::rapidjson_value;
using RapidjsonAllocator = mapbox::geojson::rapidjson_allocator;
using RapidjsonDocument = mapbox::geojson::rapidjson_document;

void bind_rapidjson(py::module &m)
{
    auto rj =
        py::class_<RapidjsonValue>(m, "rapidjson") //
            .def(py::init<>(), "Initialize an empty RapidJSON value")
            .def(py::init(
                [](const py::object &obj) { return to_rapidjson(obj); }),
                "Initialize a RapidJSON value from a Python object")
            // type checks
            .def("GetType", &RapidjsonValue::GetType, "Get the type of the value")   //
            .def("IsNull", &RapidjsonValue::IsNull, "Check if the value is null")     //
            .def("IsFalse", &RapidjsonValue::IsFalse, "Check if the value is false")   //
            .def("IsTrue", &RapidjsonValue::IsTrue, "Check if the value is true")     //
            .def("IsBool", &RapidjsonValue::IsBool, "Check if the value is a boolean")     //
            .def("IsObject", &RapidjsonValue::IsObject, "Check if the value is an object") //
            .def("IsArray", &RapidjsonValue::IsArray, "Check if the value is an array")   //
            .def("IsNumber", &RapidjsonValue::IsNumber, "Check if the value is a number") //
            .def("IsInt", &RapidjsonValue::IsInt, "Check if the value is an integer")       //
            .def("IsUint", &RapidjsonValue::IsUint, "Check if the value is an unsigned integer")     //
            .def("IsInt64", &RapidjsonValue::IsInt64, "Check if the value is a 64-bit integer")   //
            .def("IsUint64", &RapidjsonValue::IsUint64, "Check if the value is a 64-bit unsigned integer") //
            .def("IsDouble", &RapidjsonValue::IsDouble, "Check if the value is a double") //
            .def("IsFloat", &RapidjsonValue::IsFloat, "Check if the value is a float")   //
            .def("IsString", &RapidjsonValue::IsString, "Check if the value is a string") //
            //
            .def("IsLosslessDouble", &RapidjsonValue::IsLosslessDouble, "Check if the value can be losslessly converted to double") //
            .def("IsLosslessFloat", &RapidjsonValue::IsLosslessFloat, "Check if the value can be losslessly converted to float")   //
            //
            .def("SetNull", &RapidjsonValue::SetNull, "Set the value to null")     //
            .def("SetObject", &RapidjsonValue::SetObject, "Set the value to an empty object") //
            .def("SetArray", &RapidjsonValue::SetArray, "Set the value to an empty array")   //
            .def("SetInt", &RapidjsonValue::SetInt, "Set the value to an integer")       //
            .def("SetUint", &RapidjsonValue::SetUint, "Set the value to an unsigned integer")     //
            .def("SetInt64", &RapidjsonValue::SetInt64, "Set the value to a 64-bit integer")   //
            .def("SetUint64", &RapidjsonValue::SetUint64, "Set the value to a 64-bit unsigned integer") //
            .def("SetDouble", &RapidjsonValue::SetDouble, "Set the value to a double") //
            .def("SetFloat", &RapidjsonValue::SetFloat, "Set the value to a float")   //
            // setstring
            // get string
            //
            .def("Empty",
                 [](const RapidjsonValue &self) { return !__bool__(self); },
                 "Check if the value is empty")
            .def("__bool__",
                 [](const RapidjsonValue &self) { return __bool__(self); },
                 "Check if the value is truthy")
            .def(
                "Size",
                [](const RapidjsonValue &self) -> int { return __len__(self); },
                "Get the size of the value (for arrays and objects)")
            .def(
                "__len__",
                [](const RapidjsonValue &self) -> int { return __len__(self); },
                "Get the size of the value (for arrays and objects)")
            .def("HasMember",
                 [](const RapidjsonValue &self, const std::string &key) {
                     return self.HasMember(key.c_str());
                 },
                 "Check if the object has a member with the given key")
            .def("__contains__",
                 [](const RapidjsonValue &self, const std::string &key) {
                     return self.HasMember(key.c_str());
                 },
                 "Check if the object has a member with the given key")
            .def("keys",
                 [](const RapidjsonValue &self) {
                     std::vector<std::string> keys;
                     if (self.IsObject()) {
                         keys.reserve(self.MemberCount());
                         for (auto &m : self.GetObject()) {
                             keys.emplace_back(m.name.GetString(),
                                               m.name.GetStringLength());
                         }
                     }
                     return keys;
                 },
                 "Get a list of keys for an object")
            .def(
                "values",
                [](RapidjsonValue &self) {
                    std::vector<RapidjsonValue *> values;
                    if (self.IsObject()) {
                        values.reserve(self.MemberCount());
                        for (auto &m : self.GetObject()) {
                            values.push_back(&m.value);
                        }
                    }
                    return values;
                },
                rvp::reference_internal,
                "Get a list of values for an object")
            //
            .def("is_subset_of", [](const RapidjsonValue &self, const RapidjsonValue &other) -> bool {
                return is_subset_of(self, other);
            }, "other"_a, "Check if this value is a subset of another value")
            // load/dump file
            .def(
                "load",
                [](RapidjsonValue &self,
                   const std::string &path) -> RapidjsonValue & {
                    self = load_json(path);
                    return self;
                },
                rvp::reference_internal,
                "Load JSON from a file")
            .def(
                "dump",
                [](const RapidjsonValue &self, const std::string &path,
                   bool indent, bool sort_keys) -> bool {
                    return dump_json(path, self, indent, sort_keys);
                },
                "path"_a, py::kw_only(), "indent"_a = false, "sort_keys"_a = false,
                "Dump JSON to a file")
            // loads/dumps string
            .def(
                "loads",
                [](RapidjsonValue &self,
                   const std::string &json) -> RapidjsonValue & {
                    self = loads(json);
                    return self;
                },
                rvp::reference_internal,
                "Load JSON from a string")
            .def(
                "dumps",
                [](const RapidjsonValue &self, bool indent, bool sort_keys) -> std::string {
                    return dumps(self, indent, sort_keys);
                },
                py::kw_only(), "indent"_a = false, "sort_keys"_a = false,
                "Dump JSON to a string")
            // sort_keys
            .def("sort_keys", [](RapidjsonValue &self) -> RapidjsonValue & {
                sort_keys_inplace(self);
                return self;
            }, rvp::reference_internal, "Sort keys of objects recursively")
            // locate_nan_inf
            .def("locate_nan_inf", [](const RapidjsonValue &self) -> std::optional<std::string> {
                return locate_nan_inf(self);
            }, "Locate NaN or Inf values in the JSON")
            .def("round", [](RapidjsonValue &self, double precision, int depth, //
                const std::vector<std::string> &skip_keys) -> RapidjsonValue & {
                    round_rapidjson(self, std::pow(10, precision), depth, skip_keys);
                return self;
            }, rvp::reference_internal, py::kw_only(), //
                "precision"_a = 3, //
                "depth"_a = 32, //
                "skip_keys"_a = std::vector<std::string>{},
                "Round numeric values in the JSON")
            .def("round_geojson_non_geometry", [](RapidjsonValue &self, int precision) -> RapidjsonValue & {
                round_geojson_non_geometry(self, std::pow(10, precision));
                return self;
            }, rvp::reference_internal, py::kw_only(), "precision"_a = 3,
               "Round non-geometry numeric values in GeoJSON")
            .def("round_geojson_geometry", [](RapidjsonValue &self, const std::array<int, 3> &precision) -> RapidjsonValue & {
                round_geojson_geometry(self, {
                    std::pow(10, precision[0]),
                    std::pow(10, precision[1]),
                    std::pow(10, precision[2])});
                return self;
            }, rvp::reference_internal, py::kw_only(), "precision"_a = std::array<int, 3>{8, 8, 3},
               "Round geometry coordinates in GeoJSON")
            .def("strip_geometry_z_0", [](RapidjsonValue &self) -> RapidjsonValue & {
                strip_geometry_z_0(self);
                return self;
            }, rvp::reference_internal, "Strip zero Z values from GeoJSON geometries")
            .def("denoise_double_0", [](RapidjsonValue &self) -> RapidjsonValue & {
                denoise_double_0_rapidjson(self);
                return self;
            }, rvp::reference_internal, "Denoise double values that are close to zero")
            .def("normalize", [](RapidjsonValue &self,
                    bool sort_keys,
                    bool strip_geometry_z_0,
                    std::optional<int> round_geojson_non_geometry,
                    const std::optional<std::array<int, 3>> &round_geojson_geometry,
                    bool denoise_double_0) -> RapidjsonValue & {
                        normalize_json(self,
                            sort_keys,
                            round_geojson_non_geometry,
                            round_geojson_geometry,
                            strip_geometry_z_0,
                            denoise_double_0);
                return self;
            }, py::kw_only(), //
                "sort_keys"_a = true, //
                "strip_geometry_z_0"_a = true,
                "round_geojson_non_geometry"_a = 3,
                "round_geojson_geometry"_a = std::array<int, 3>{8, 8, 3},
                "denoise_double_0"_a = true,
                "Normalize JSON by applying multiple transformations")
            .def(
                "get",
                [](RapidjsonValue &self,
                   const std::string &key) -> RapidjsonValue * {
                    auto itr = self.FindMember(key.c_str());
                    if (itr == self.MemberEnd()) {
                        return nullptr;
                    } else {
                        return &itr->value;
                    }
                },
                "key"_a, rvp::reference_internal,
                "Get a member value by key, returns None if not found")
            .def(
                "__getitem__",
                [](RapidjsonValue &self,
                   const std::string &key) -> RapidjsonValue * {
                    auto itr = self.FindMember(key.c_str());
                    if (itr == self.MemberEnd()) {
                        throw pybind11::key_error(key);
                    }
                    return &itr->value;
                },
                rvp::reference_internal,
                "Get a member value by key")
            .def(
                "__getitem__",
                [](RapidjsonValue &self, int index) -> RapidjsonValue & {
                    return self[index >= 0 ? index : index + (int)self.Size()];
                },
                rvp::reference_internal,
                "Get an array element by index")
            .def("__delitem__",
                 [](RapidjsonValue &self, const std::string &key) {
                     return self.EraseMember(key.c_str());
                 },
                 "Delete a member by key")
            .def("__delitem__",
                 [](RapidjsonValue &self, int index) {
                     self.Erase(
                         self.Begin() +
                         (index >= 0 ? index : index + (int)self.Size()));
                 },
                 "Delete an array element by index")
            .def("clear",
                 [](RapidjsonValue &self) -> RapidjsonValue & {
                     if (self.IsObject()) {
                         self.RemoveAllMembers();
                     } else if (self.IsArray()) {
                         self.Clear();
                     }
                     return self;
                 }, rvp::reference_internal,
                 "Clear all members of an object or array")
            // get (python copy)
            .def("GetBool", &RapidjsonValue::GetBool, "Get boolean value")
            .def("GetInt", &RapidjsonValue::GetInt, "Get integer value")
            .def("GetUint", &RapidjsonValue::GetUint, "Get unsigned integer value")
            .def("GetInt64", &RapidjsonValue::GetInt64, "Get 64-bit integer value")
            .def("GetUInt64", &RapidjsonValue::GetUint64, "Get 64-bit unsigned integer value")
            .def("GetFloat", &RapidjsonValue::GetFloat, "Get float value")
            .def("GetDouble", &RapidjsonValue::GetDouble, "Get double value")
            .def("GetString",
                 [](const RapidjsonValue &self) {
                     return std::string{self.GetString(),
                                        self.GetStringLength()};
                 },
                 "Get string value")
            .def("GetStringLength", &RapidjsonValue::GetStringLength, "Get length of string value")
            // https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=MemoryView#memory-view
            .def("GetRawString", [](const RapidjsonValue &self) {
                return py::memoryview::from_memory(
                    self.GetString(),
                    self.GetStringLength()
                );
            }, rvp::reference_internal, "Get raw string as memory view")
            .def("Get",
                 [](const RapidjsonValue &self) { return to_python(self); },
                 "Convert RapidJSON value to Python object")
            .def("__call__",
                 [](const RapidjsonValue &self) { return to_python(self); },
                 "Convert RapidJSON value to Python object")
            // set
            .def(
                "set",
                [](RapidjsonValue &self,
                   const py::object &obj) -> RapidjsonValue & {
                    self = to_rapidjson(obj);
                    return self;
                },
                rvp::reference_internal,
                "Set value from Python object")
            .def(
                "set",
                [](RapidjsonValue &self,
                   const RapidjsonValue &obj) -> RapidjsonValue & {
                    self = deepcopy(obj);
                    return self;
                },
                rvp::reference_internal,
                "Set value from another RapidJSON value")
            .def( // same as set
                "copy_from",
                [](RapidjsonValue &self,
                   const RapidjsonValue &obj) -> RapidjsonValue & {
                    self = deepcopy(obj);
                    return self;
                },
                rvp::reference_internal,
                "Copy value from another RapidJSON value")
            .def(
                "__setitem__",
                [](RapidjsonValue &self, int index, const py::object &obj) {
                    self[index >= 0 ? index : index + (int)self.Size()] =
                        to_rapidjson(obj);
                    return obj;
                },
                "index"_a, "value"_a, rvp::reference_internal,
                "Set array element by index")
            .def(
                "__setitem__",
                [](RapidjsonValue &self, const std::string &key,
                   const py::object &obj) {
                    auto itr = self.FindMember(key.c_str());
                    if (itr == self.MemberEnd()) {
                        RapidjsonAllocator allocator;
                        self.AddMember(
                            RapidjsonValue(key.data(), key.size(), allocator),
                            to_rapidjson(obj, allocator), allocator);
                    } else {
                        RapidjsonAllocator allocator;
                        itr->value = to_rapidjson(obj, allocator);
                    }
                    return obj;
                },
                rvp::reference_internal,
                "Set object member by key")
            .def(
                "push_back",
                [](RapidjsonValue &self,
                   const py::object &obj) -> RapidjsonValue & {
                    RapidjsonAllocator allocator;
                    self.PushBack(to_rapidjson(obj), allocator);
                    return self;
                },
                rvp::reference_internal,
                "Append value to array")
            //
            .def(
                "pop_back",
                [](RapidjsonValue &self) -> RapidjsonValue
                                             & {
                                                 self.PopBack();
                                                 return self;
                                             },
                rvp::reference_internal,
                "Remove and return the last element of the array")
            // https://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=__deepcopy__#deepcopy-support
            .def("__copy__",
                 [](const RapidjsonValue &self, py::dict) -> RapidjsonValue {
                     return deepcopy(self);
                 },
                 "Create a shallow copy of the value")
            .def(
                "__deepcopy__",
                [](const RapidjsonValue &self, py::dict) -> RapidjsonValue {
                    return deepcopy(self);
                },
                "memo"_a,
                "Create a deep copy of the value")
            .def("clone",
                 [](const RapidjsonValue &self) -> RapidjsonValue {
                     return deepcopy(self);
                 },
                 "Create a deep copy of the value")
            // https://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=pickle#pickling-support
            .def(py::pickle(
                [](const RapidjsonValue &self) { return to_python(self); },
                [](py::object o) -> RapidjsonValue { return to_rapidjson(o); }),
                "Enable pickling support")
            .def(py::self == py::self, "Compare two RapidJSON values for equality")
            .def(py::self != py::self, "Compare two RapidJSON values for inequality")
        //
        ;
    py::enum_<rapidjson::Type>(rj, "type")
        .value("kNullType", rapidjson::kNullType, "Null type")
        .value("kFalseType", rapidjson::kFalseType, "False type")
        .value("kTrueType", rapidjson::kTrueType, "True type")
        .value("kObjectType", rapidjson::kObjectType, "Object type")
        .value("kArrayType", rapidjson::kArrayType, "Array type")
        .value("kStringType", rapidjson::kStringType, "String type")
        .value("kNumberType", rapidjson::kNumberType, "Number type")
        .export_values();
}
} // namespace cubao

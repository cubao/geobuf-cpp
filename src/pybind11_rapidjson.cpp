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
            .def(py::init<>())
            .def(py::init(
                [](const py::object &obj) { return to_rapidjson(obj); }))
            // type checks
            .def("GetType", &RapidjsonValue::GetType)   //
            .def("IsNull", &RapidjsonValue::IsNull)     //
            .def("IsFalse", &RapidjsonValue::IsFalse)   //
            .def("IsTrue", &RapidjsonValue::IsTrue)     //
            .def("IsBool", &RapidjsonValue::IsBool)     //
            .def("IsObject", &RapidjsonValue::IsObject) //
            .def("IsArray", &RapidjsonValue::IsArray)   //
            .def("IsNumber", &RapidjsonValue::IsNumber) //
            .def("IsInt", &RapidjsonValue::IsInt)       //
            .def("IsUint", &RapidjsonValue::IsUint)     //
            .def("IsInt64", &RapidjsonValue::IsInt64)   //
            .def("IsUint64", &RapidjsonValue::IsUint64) //
            .def("IsDouble", &RapidjsonValue::IsDouble) //
            .def("IsFloat", &RapidjsonValue::IsFloat)   //
            .def("IsString", &RapidjsonValue::IsString) //
            //
            .def("IsLosslessDouble", &RapidjsonValue::IsLosslessDouble) //
            .def("IsLosslessFloat", &RapidjsonValue::IsLosslessFloat)   //
            //
            .def("SetNull", &RapidjsonValue::SetNull)     //
            .def("SetObject", &RapidjsonValue::SetObject) //
            .def("SetArray", &RapidjsonValue::SetArray)   //
            .def("SetInt", &RapidjsonValue::SetInt)       //
            .def("SetUint", &RapidjsonValue::SetUint)     //
            .def("SetInt64", &RapidjsonValue::SetInt64)   //
            .def("SetUint64", &RapidjsonValue::SetUint64) //
            .def("SetDouble", &RapidjsonValue::SetDouble) //
            .def("SetFloat", &RapidjsonValue::SetFloat)   //
            // setstring
            // get string
            //
            .def("Empty",
                 [](const RapidjsonValue &self) { return !__bool__(self); })
            .def("__bool__",
                 [](const RapidjsonValue &self) { return __bool__(self); })
            .def(
                "Size",
                [](const RapidjsonValue &self) -> int { return __len__(self); })
            .def(
                "__len__",
                [](const RapidjsonValue &self) -> int { return __len__(self); })
            .def("HasMember",
                 [](const RapidjsonValue &self, const std::string &key) {
                     return self.HasMember(key.c_str());
                 })
            .def("__contains__",
                 [](const RapidjsonValue &self, const std::string &key) {
                     return self.HasMember(key.c_str());
                 })
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
                 })
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
                rvp::reference_internal)
            // load/dump file
            .def(
                "load",
                [](RapidjsonValue &self,
                   const std::string &path) -> RapidjsonValue & {
                    self = load_json(path);
                    return self;
                },
                rvp::reference_internal)
            .def(
                "dump",
                [](const RapidjsonValue &self, const std::string &path,
                   bool indent, bool sort_keys) -> bool {
                    return dump_json(path, self, indent, sort_keys);
                },
                "path"_a, py::kw_only(), "indent"_a = false, "sort_keys"_a = false)
            // loads/dumps string
            .def(
                "loads",
                [](RapidjsonValue &self,
                   const std::string &json) -> RapidjsonValue & {
                    self = loads(json);
                    return self;
                },
                rvp::reference_internal)
            .def(
                "dumps",
                [](const RapidjsonValue &self, bool indent, bool sort_keys) -> std::string {
                    return dumps(self, indent, sort_keys);
                },
                py::kw_only(), "indent"_a = false, "sort_keys"_a = false)
            // sort_keys
            .def("sort_keys", [](RapidjsonValue &self) -> RapidjsonValue & {
                sort_keys_inplace(self);
                return self;
            }, rvp::reference_internal)
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
                "key"_a, rvp::reference_internal)
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
                rvp::reference_internal)
            .def(
                "__getitem__",
                [](RapidjsonValue &self, int index) -> RapidjsonValue & {
                    return self[index >= 0 ? index : index + (int)self.Size()];
                },
                rvp::reference_internal)
            .def("__delitem__",
                 [](RapidjsonValue &self, const std::string &key) {
                     return self.EraseMember(key.c_str());
                 })
            .def("__delitem__",
                 [](RapidjsonValue &self, int index) {
                     self.Erase(
                         self.Begin() +
                         (index >= 0 ? index : index + (int)self.Size()));
                 })
            .def("clear",
                 [](RapidjsonValue &self) -> RapidjsonValue & {
                     if (self.IsObject()) {
                         self.RemoveAllMembers();
                     } else if (self.IsArray()) {
                         self.Clear();
                     }
                     return self;
                 }, rvp::reference_internal)
            // get (python copy)
            .def("GetBool", &RapidjsonValue::GetBool)
            .def("GetInt", &RapidjsonValue::GetInt)
            .def("GetUint", &RapidjsonValue::GetUint)
            .def("GetInt64", &RapidjsonValue::GetInt64)
            .def("GetUInt64", &RapidjsonValue::GetUint64)
            .def("GetFloat", &RapidjsonValue::GetFloat)
            .def("GetDouble", &RapidjsonValue::GetDouble)
            .def("GetString",
                 [](const RapidjsonValue &self) {
                     return std::string{self.GetString(),
                                        self.GetStringLength()};
                 })
            .def("GetStringLength", &RapidjsonValue::GetStringLength)
            // https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=MemoryView#memory-view
            .def("GetRawString", [](const RapidjsonValue &self) {
                return py::memoryview::from_memory(
                    self.GetString(),
                    self.GetStringLength()
                );
            }, rvp::reference_internal)
            .def("Get",
                 [](const RapidjsonValue &self) { return to_python(self); })
            .def("__call__",
                 [](const RapidjsonValue &self) { return to_python(self); })
            // set
            .def(
                "set",
                [](RapidjsonValue &self,
                   const py::object &obj) -> RapidjsonValue & {
                    self = to_rapidjson(obj);
                    return self;
                },
                rvp::reference_internal)
            .def(
                "set",
                [](RapidjsonValue &self,
                   const RapidjsonValue &obj) -> RapidjsonValue & {
                    self = deepcopy(obj);
                    return self;
                },
                rvp::reference_internal)
            .def( // same as set
                "copy_from",
                [](RapidjsonValue &self,
                   const RapidjsonValue &obj) -> RapidjsonValue & {
                    self = deepcopy(obj);
                    return self;
                },
                rvp::reference_internal)
            .def(
                "__setitem__",
                [](RapidjsonValue &self, int index, const py::object &obj) {
                    self[index >= 0 ? index : index + (int)self.Size()] =
                        to_rapidjson(obj);
                    return obj;
                },
                "index"_a, "value"_a, rvp::reference_internal)
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
                rvp::reference_internal)
            .def(
                "push_back",
                [](RapidjsonValue &self,
                   const py::object &obj) -> RapidjsonValue & {
                    RapidjsonAllocator allocator;
                    self.PushBack(to_rapidjson(obj), allocator);
                    return self;
                },
                rvp::reference_internal)
            //
            .def(
                "pop_back",
                [](RapidjsonValue &self) -> RapidjsonValue
                                             & {
                                                 self.PopBack();
                                                 return self;
                                             },
                rvp::reference_internal)
            // https://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=__deepcopy__#deepcopy-support
            .def("__copy__",
                 [](const RapidjsonValue &self, py::dict) -> RapidjsonValue {
                     return deepcopy(self);
                 })
            .def(
                "__deepcopy__",
                [](const RapidjsonValue &self, py::dict) -> RapidjsonValue {
                    return deepcopy(self);
                },
                "memo"_a)
            .def("clone",
                 [](const RapidjsonValue &self) -> RapidjsonValue {
                     return deepcopy(self);
                 })
            // https://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=pickle#pickling-support
            .def(py::pickle(
                [](const RapidjsonValue &self) { return to_python(self); },
                [](py::object o) -> RapidjsonValue { return to_rapidjson(o); }))
            .def(py::self == py::self)
            .def(py::self != py::self)
        //
        ;
    py::enum_<rapidjson::Type>(rj, "type")
        .value("kNullType", rapidjson::kNullType)
        .value("kFalseType", rapidjson::kFalseType)
        .value("kTrueType", rapidjson::kTrueType)
        .value("kObjectType", rapidjson::kObjectType)
        .value("kArrayType", rapidjson::kArrayType)
        .value("kStringType", rapidjson::kStringType)
        .value("kNumberType", rapidjson::kNumberType)
        .export_values();
}
} // namespace cubao

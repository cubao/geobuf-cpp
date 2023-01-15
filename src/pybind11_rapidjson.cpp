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

constexpr const auto RJFLAGS = rapidjson::kParseDefaultFlags |      //
                               rapidjson::kParseCommentsFlag |      //
                               rapidjson::kParseFullPrecisionFlag | //
                               rapidjson::kParseTrailingCommasFlag;

namespace py = pybind11;
using namespace pybind11::literals;
using rvp = py::return_value_policy;

using RapidjsonValue = mapbox::geojson::rapidjson_value;
using RapidjsonAllocator = mapbox::geojson::rapidjson_allocator;
using RapidjsonDocument = mapbox::geojson::rapidjson_document;

inline RapidjsonValue deepcopy(const RapidjsonValue &json, RapidjsonAllocator &allocator)
{
    RapidjsonValue copy;
    copy.CopyFrom(json, allocator);
    return copy;
}
inline RapidjsonValue deepcopy(const RapidjsonValue &json) {
    RapidjsonAllocator allocator;
    return deepcopy(json, allocator);
}

template <typename T> RapidjsonValue int_to_rapidjson(T const &num)
{
    if (sizeof(T) < sizeof(int64_t)) {
        return std::is_signed<T>::value
                   ? RapidjsonValue(static_cast<int32_t>(num))
                   : RapidjsonValue(static_cast<uint32_t>(num));
    } else {
        return std::is_signed<T>::value
                   ? RapidjsonValue(static_cast<int64_t>(num))
                   : RapidjsonValue(static_cast<uint64_t>(num));
    }
}

inline RapidjsonValue py_int_to_rapidjson(const py::handle &obj)
{
    try {
        auto num = obj.cast<int64_t>();
        if (py::int_(num).equal(obj)) {
            return RapidjsonValue(num);
        }
    } catch (...) {
    }
    try {
        auto num = obj.cast<uint64_t>();
        if (py::int_(num).equal(obj)) {
            return RapidjsonValue(num);
        }
    } catch (...) {
    }
    throw std::runtime_error(
        "failed to convert to rapidjson, invalid integer: " +
        py::repr(obj).cast<std::string>());
}

inline RapidjsonValue to_rapidjson(const py::handle &obj,
                                   RapidjsonAllocator &allocator)
{
    if (obj.ptr() == nullptr || obj.is_none()) {
        return {};
    }
    if (py::isinstance<py::bool_>(obj)) {
        return RapidjsonValue(obj.cast<bool>());
    }
    if (py::isinstance<py::int_>(obj)) {
        return py_int_to_rapidjson(obj);
    }
    if (py::isinstance<py::float_>(obj)) {
        return RapidjsonValue(obj.cast<double>());
    }
    if (py::isinstance<py::bytes>(obj)) {
        // https://github.com/pybind/pybind11_json/blob/master/include/pybind11_json/pybind11_json.hpp#L112
        py::module base64 = py::module::import("base64");
        auto str = base64.attr("b64encode")(obj)
                       .attr("decode")("utf-8")
                       .cast<std::string>();
        return RapidjsonValue(str.data(), str.size(), allocator);
    }
    if (py::isinstance<py::str>(obj)) {
        auto str = obj.cast<std::string>();
        return RapidjsonValue(str.data(), str.size(), allocator);
    }
    if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj)) {
        RapidjsonValue arr(rapidjson::kArrayType);
        for (const py::handle &value : obj) {
            arr.PushBack(to_rapidjson(value, allocator), allocator);
        }
        return arr;
    }
    if (py::isinstance<py::dict>(obj)) {
        RapidjsonValue kv(rapidjson::kObjectType);
        for (const py::handle &key : obj) {
            auto k = py::str(key).cast<std::string>();
            kv.AddMember(RapidjsonValue(k.data(), k.size(), allocator),
                         to_rapidjson(obj[key], allocator), allocator);
        }
        return kv;
    }
    if (py::isinstance<RapidjsonValue>(obj)) {
        auto ptr = py::cast<const RapidjsonValue *>(obj);
        return deepcopy(*ptr, allocator);
    }
    throw std::runtime_error(
        "to_rapidjson not implemented for this type of object: " +
        py::repr(obj).cast<std::string>());
}

inline RapidjsonValue to_rapidjson(const py::handle &obj)
{
    RapidjsonAllocator allocator;
    return to_rapidjson(obj, allocator);
}

inline py::object to_python(const RapidjsonValue &j)
{
    if (j.IsNull()) {
        return py::none();
    } else if (j.IsBool()) {
        return py::bool_(j.GetBool());
    } else if (j.IsNumber()) {
        if (j.IsUint64()) {
            return py::int_(j.GetUint64());
        } else if (j.IsInt64()) {
            return py::int_(j.GetInt64());
        } else {
            return py::float_(j.GetDouble());
        }
    } else if (j.IsString()) {
        return py::str(std::string{j.GetString(), j.GetStringLength()});
    } else if (j.IsArray()) {
        py::list obj;
        for (const auto &el : j.GetArray()) {
            obj.append(to_python(el));
        }
        return obj;
    } else {
        py::dict obj;
        for (auto &m : j.GetObject()) {
            obj[py::str(m.name.GetString(), m.name.GetStringLength())] =
                to_python(m.value);
        }
        return obj;
    }
}

inline RapidjsonValue load_json(const std::string &path)
{
    FILE *fp = fopen(path.c_str(), "rb");
    if (!fp) {
        throw std::runtime_error("can't open for reading: " + path);
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    RapidjsonDocument d;
    d.ParseStream<RJFLAGS>(is);
    fclose(fp);
    return RapidjsonValue{std::move(d.Move())};
}
inline bool dump_json(const std::string &path, const RapidjsonValue &json,
                      bool indent = false)
{
    FILE *fp = fopen(path.c_str(), "wb");
    if (!fp) {
        std::cerr << "can't open for writing: " + path << std::endl;
        return false;
    }
    using namespace rapidjson;
    char writeBuffer[65536];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    if (indent) {
        PrettyWriter<FileWriteStream> writer(os);
        json.Accept(writer);
    } else {
        Writer<FileWriteStream> writer(os);
        json.Accept(writer);
    }
    fclose(fp);
    return true;
}

inline RapidjsonValue loads(const std::string &json)
{
    RapidjsonDocument d;
    rapidjson::StringStream ss(json.data());
    d.ParseStream<RJFLAGS>(ss);
    if (d.HasParseError()) {
        throw std::invalid_argument(
            "invalid json, offset: " + std::to_string(d.GetErrorOffset()) +
            ", error: " + rapidjson::GetParseError_En(d.GetParseError()));
    }
    return RapidjsonValue{std::move(d.Move())};
}
inline std::string dumps(const RapidjsonValue &json, bool indent = false)
{
    rapidjson::StringBuffer buffer;
    if (indent) {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        json.Accept(writer);
    } else {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        json.Accept(writer);
    }
    return buffer.GetString();
}

inline bool __bool__(const RapidjsonValue &self)
{
    if (self.IsArray()) {
        return !self.Empty();
    } else if (self.IsObject()) {
        return !self.ObjectEmpty();
    } else if (self.IsString()) {
        return self.GetStringLength() != 0u;
    } else if (self.IsBool()) {
        return self.GetBool();
    } else if (self.IsNumber()) {
        if (self.IsUint64()) {
            return self.GetUint64() != 0;
        } else if (self.IsInt64()) {
            return self.GetInt64() != 0;
        } else {
            return self.GetDouble() != 0.0;
        }
    }
    return !self.IsNull();
}

inline int __len__(const RapidjsonValue &self)
{
    if (self.IsArray()) {
        return self.Size();
    } else if (self.IsObject()) {
        return self.MemberCount();
    }
    return 0;
}

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
                 [](const RapidjsonValue &self) { return __bool__(self); })
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
                   bool indent) -> bool {
                    return dump_json(path, self, indent);
                },
                "path"_a, py::kw_only(), "indent"_a = false)
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
                [](const RapidjsonValue &self, bool indent) -> std::string {
                    return dumps(self, indent);
                },
                py::kw_only(), "indent"_a = false)
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
                 [](RapidjsonValue &self) {
                     if (self.IsObject()) {
                         self.RemoveAllMembers();
                     } else if (self.IsArray()) {
                         self.Clear();
                     }
                 })
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
                 [](const RapidjsonValue &self) { return ::to_python(self); })
            .def("__call__",
                 [](const RapidjsonValue &self) { return ::to_python(self); })
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

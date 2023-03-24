#ifndef CUBAO_RAPIDJSON_HELPERS_HPP
#define CUBAO_RAPIDJSON_HELPERS_HPP

#include <mapbox/geojson.hpp>
#include <mapbox/geojson/rapidjson.hpp>
#include <mapbox/geojson/value.hpp>

#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <fstream>
#include <iostream>

namespace cubao
{
constexpr const auto RJFLAGS = rapidjson::kParseDefaultFlags |      //
                               rapidjson::kParseCommentsFlag |      //
                               rapidjson::kParseFullPrecisionFlag | //
                               rapidjson::kParseTrailingCommasFlag;

using RapidjsonValue = mapbox::geojson::rapidjson_value;
using RapidjsonAllocator = mapbox::geojson::rapidjson_allocator;
using RapidjsonDocument = mapbox::geojson::rapidjson_document;

inline RapidjsonValue deepcopy(const RapidjsonValue &json,
                               RapidjsonAllocator &allocator)
{
    RapidjsonValue copy;
    copy.CopyFrom(json, allocator);
    return copy;
}
inline RapidjsonValue deepcopy(const RapidjsonValue &json)
{
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

inline void sort_keys_inplace(RapidjsonValue &json)
{
    if (json.IsArray()) {
        for (auto &e : json.GetArray()) {
            sort_keys_inplace(e);
        }
    } else if (json.IsObject()) {
        auto obj = json.GetObject();
        // https://rapidjson.docsforge.com/master/sortkeys.cpp/
        std::sort(obj.MemberBegin(), obj.MemberEnd(), [](auto &lhs, auto &rhs) {
            return strcmp(lhs.name.GetString(), rhs.name.GetString()) < 0;
        });
        for (auto &kv : obj) {
            sort_keys_inplace(kv.value);
        }
    }
}

inline RapidjsonValue sort_keys(const RapidjsonValue &json)
{
    RapidjsonAllocator allocator;
    RapidjsonValue copy;
    copy.CopyFrom(json, allocator);
    sort_keys_inplace(copy);
    return copy;
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
                      bool indent = false, bool sort_keys = false)
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
        if (sort_keys) {
            cubao::sort_keys(json).Accept(writer);
        } else {
            json.Accept(writer);
        }
    } else {
        Writer<FileWriteStream> writer(os);
        if (sort_keys) {
            cubao::sort_keys(json).Accept(writer);
        } else {
            json.Accept(writer);
        }
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
inline std::string dumps(const RapidjsonValue &json, bool indent = false,
                         bool sort_keys = false)
{
    if (sort_keys) {
        return dumps(cubao::sort_keys(json), indent, !sort_keys);
    }
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

struct to_value
{
    RapidjsonAllocator &allocator;

    RapidjsonValue operator()(mapbox::geojson::null_value_t)
    {
        RapidjsonValue result;
        result.SetNull();
        return result;
    }

    RapidjsonValue operator()(bool t)
    {
        RapidjsonValue result;
        result.SetBool(t);
        return result;
    }

    RapidjsonValue operator()(int64_t t)
    {
        RapidjsonValue result;
        result.SetInt64(t);
        return result;
    }

    RapidjsonValue operator()(uint64_t t)
    {
        RapidjsonValue result;
        result.SetUint64(t);
        return result;
    }

    RapidjsonValue operator()(double t)
    {
        RapidjsonValue result;
        result.SetDouble(t);
        return result;
    }

    RapidjsonValue operator()(const std::string &t)
    {
        RapidjsonValue result;
        result.SetString(t.data(), rapidjson::SizeType(t.size()), allocator);
        return result;
    }

    RapidjsonValue operator()(const std::vector<mapbox::geojson::value> &array)
    {
        RapidjsonValue result;
        result.SetArray();
        for (const auto &item : array) {
            result.PushBack(mapbox::geojson::value::visit(item, *this),
                            allocator);
        }
        return result;
    }

    RapidjsonValue operator()(
        const std::unordered_map<std::string, mapbox::geojson::value> &map)
    {
        RapidjsonValue result;
        result.SetObject();
        for (const auto &property : map) {
            result.AddMember(
                RapidjsonValue(property.first.data(),
                               rapidjson::SizeType(property.first.size()),
                               allocator),
                mapbox::geojson::value::visit(property.second, *this),
                allocator);
        }
        return result;
    }
};

inline RapidjsonValue to_rapidjson(const mapbox::geojson::value &json,
                                   RapidjsonAllocator &allocator)
{
    return mapbox::geojson::value::visit(json, cubao::to_value{allocator});
}

inline RapidjsonValue to_rapidjson(const mapbox::geojson::value &json)
{
    RapidjsonAllocator allocator;
    return to_rapidjson(json, allocator);
}

// inline mapbox::geojson::value to_geojson_value(const RapidjsonValue &json)
// {
//     return mapbox::geojson::convert<mapbox::geojson::value>(json);
// }

} // namespace cubao

#endif

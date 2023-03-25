#pragma once

#include <cmath>
#include <mapbox/geojson.hpp>
#include <mapbox/geojson/rapidjson.hpp>
#include <protozero/pbf_builder.hpp>
#include <protozero/pbf_reader.hpp>

#include <map>
#include <unordered_map>

#define MAPBOX_GEOBUF_DEFAULT_PRECISION 6
#define MAPBOX_GEOBUF_DEFAULT_DIM 2

namespace mapbox
{
namespace geobuf
{
using PointsType = mapbox::geojson::multi_point::container_type;
using LinesType = mapbox::geojson::multi_line_string::container_type;
using PolygonsType = mapbox::geojson::multi_polygon::container_type;

using RapidjsonValue = mapbox::geojson::rapidjson_value;
using RapidjsonAllocator = mapbox::geojson::rapidjson_allocator;

RapidjsonValue load_json(const std::string &path);
RapidjsonValue load_json(); // read from stdin
bool dump_json(const std::string &path, const RapidjsonValue &json,
               bool indent = false, bool sort_keys = false);
bool dump_json(const RapidjsonValue &json, //
               bool indent = false,
               bool sort_keys = false); // write to stdout

std::string load_bytes(const std::string &path);
std::string load_bytes(); // read from stdin
bool dump_bytes(const std::string &path, const std::string &bytes);

RapidjsonValue geojson2json(const mapbox::geojson::value &geojson,
                            bool sort_keys = false);
RapidjsonValue geojson2json(const mapbox::geojson::geojson &geojson,
                            bool sort_keys = false);
mapbox::geojson::value json2geojson(const RapidjsonValue &json);

RapidjsonValue parse(const std::string &json, bool raise_error = false);
std::string dump(const RapidjsonValue &json, //
                 bool indent = false, bool sort_keys = false);
std::string dump(const mapbox::geojson::value &geojson, //
                 bool indent = false, bool sort_keys = false);
std::string dump(const mapbox::geojson::geojson &geojson, //
                 bool indent = false, bool sort_keys = false);

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

struct Encoder
{
    using Pbf = protozero::pbf_writer;
    Encoder(uint32_t maxPrecision = std::pow(10,
                                             MAPBOX_GEOBUF_DEFAULT_PRECISION))
        : maxPrecision(maxPrecision)
    {
    }
    std::string encode(const mapbox::geojson::geojson &geojson);
    std::string encode(const mapbox::geojson::feature_collection &features)
    {
        return encode(mapbox::geojson::geojson{features});
    }
    std::string encode(const mapbox::geojson::feature &feature)
    {
        return encode(mapbox::geojson::geojson{feature});
    }
    std::string encode(const mapbox::geojson::geometry &geometry)
    {
        return encode(mapbox::geojson::geojson{geometry});
    }

    std::string encode(const std::string &geojson);
    std::string encode(const RapidjsonValue &json);
    bool encode(const std::string &input_path, const std::string &output_path);

    std::map<std::string, std::uint32_t> __keys() const
    {
        return std::map<std::string, std::uint32_t>(keys.begin(), keys.end());
    }

  private:
    void analyze(const mapbox::geojson::geojson &geojson);
    void analyzeGeometry(const mapbox::geojson::geometry &geometry);
    void analyzeMultiLine(const LinesType &lines);
    void analyzePoints(const PointsType &points);
    void analyzePoint(const mapbox::geojson::point &point);
    void saveKey(const std::string &key);
    void saveKey(const mapbox::feature::property_map &props);

    // Yeah, I know. In c++, we can use overloading...
    // Just make it identical to the JS implementation
    void
    writeFeatureCollection(const mapbox::geojson::feature_collection &geojson,
                           Pbf &pbf);
    void writeFeature(const mapbox::geojson::feature &geojson, Pbf &pbf);
    void writeGeometry(const mapbox::geojson::geometry &geojson, Pbf &pbf);
    // in mapbox geojson, there is no custom properties
    void writeProps(const mapbox::feature::property_map &props, Pbf &pbf,
                    int tag);
    void writeValue(const mapbox::feature::value &value, Pbf &pbf);
    void writePoint(const mapbox::geojson::point &point, Pbf &pbf);
    void writeLine(const PointsType &line, Pbf &pbf);
    // implict close=false is JS, we don't do that
    void writeMultiLine(const LinesType &lines, Pbf &pbf, bool closed);
    void writeMultiPolygon(const PolygonsType &polygons, Pbf &pbf);
    std::vector<int64_t> populateLine(const PointsType &line, bool closed);
    void populateLine(std::vector<int64_t> &coords, //
                      const PointsType &line,       //
                      bool closed);

    const uint32_t maxPrecision;
    uint32_t dim = MAPBOX_GEOBUF_DEFAULT_DIM;
    uint32_t e = 1;
    std::unordered_map<std::string, std::uint32_t> keys;
};

struct Decoder
{
    using Pbf = protozero::pbf_reader;
    Decoder() {}
    static std::string to_printable(const std::string &pbf_bytes,
                                    const std::string &indent = "");
    mapbox::geojson::geojson decode(const std::string &pbf_bytes);
    bool decode(const std::string &input_path, const std::string &output_path,
                bool indent = false, bool sort_keys = false);
    int precision() const { return std::log10(e); }

  private:
    mapbox::geojson::feature_collection readFeatureCollection(Pbf &pbf);
    mapbox::geojson::feature readFeature(Pbf &pbf);
    mapbox::geojson::geometry readGeometry(Pbf &pbf);
    mapbox::geojson::value readValue(Pbf &pbf);

    uint32_t dim = MAPBOX_GEOBUF_DEFAULT_DIM;
    uint32_t e = std::pow(10, MAPBOX_GEOBUF_DEFAULT_PRECISION);
    std::vector<std::string> keys;
};

} // namespace geobuf
} // namespace mapbox

#pragma once
#include "geobuf.hpp"
#include "planet.hpp"

namespace cubao
{

struct GeobufPlus
{

    static std::string encode(const Planet &planet) { return ""; }

    GeobufPlus() = default;

    void init(const std::string &header_bytes) {
    }
    mapbox::geojson::feature_collection decode(const std::string &bytes) const
    {
        return {};
    }

    mapbox::geojson::feature decode_one_feature(const char *ptr) const
    {
        return mapbox::geojson::feature{};
    }

    mapbox::geojson::feature_collection
    decode(const void *data, const std::vector<int> &offsets) const
    {
        return {};
    }

    mapbox::feature::property_map decode_non_features(const char *tail_start, const char *tail_end) const {
    }

    static std::string encode(const Planet &planet) { return ""; }
};
} // namespace cubao
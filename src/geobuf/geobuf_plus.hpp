#pragma once
#include "geobuf.hpp"
#include "planet.hpp"

namespace cubao
{

struct GeobufPlus
{

    static std::string encode(const Planet &planet) { return ""; }

    GeobufPlus(const std::string &header) {}

    mapbox::geojson::feature_collection decode(const std::string &bytes) const
    {
    }

    mapbox::geojson::feature_collection
    decode(const void *data, const std::vector<int> &offsets) const
    {
    }

    static std::string encode(const Planet &planet) { return ""; }
};
} // namespace cubao
#ifndef CUBAO_PLANET_HPP
#define CUBAO_PLANET_HPP

// https://github.com/microsoft/vscode-cpptools/issues/9692
#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include "geojson_cropping.hpp"
#include "packedrtree.hpp"

namespace cubao
{
struct Planet
{
    using FeatureCollection = mapbox::geojson::feature_collection;

    const FeatureCollection &features() const { return features_; }
    Planet &features(const FeatureCollection &features) { features_ = features; return *this; }

  private:
    FeatureCollection features_;
};
} // namespace cubao

#endif

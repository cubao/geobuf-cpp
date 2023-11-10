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
    Planet &features(const FeatureCollection &features)
    {
        features_ = features;
        return *this;
    }

  private:
    FeatureCollection features_;

    mutable std::optional<FlatGeobuf::PackedRTree> rtree_;

    template <typename G>
    static FlatGeobuf::NodeItem envelope_2d(G const &geometry, uint64_t index)
    {
        // mapbox/geometry/envelope.hpp
        using limits = std::numeric_limits<double>;
        constexpr double min_t =
            limits::has_infinity ? -limits::infinity() : limits::min();
        constexpr double max_t =
            limits::has_infinity ? limits::infinity() : limits::max();
        double min_x = max_t;
        double min_y = max_t;
        double max_x = min_t;
        double max_y = min_t;
        mapbox::geometry::for_each_point(
            geometry, [&](mapbox::geojson::point const &point) {
                if (min_x > point.x)
                    min_x = point.x;
                if (min_y > point.y)
                    min_y = point.y;
                if (max_x < point.x)
                    max_x = point.x;
                if (max_y < point.y)
                    max_y = point.y;
            });
        return {min_x, min_y, max_x, max_y, index};
    }

    FlatGeobuf::PackedRTree &rtree() const
    {
        if (rtree_) {
            return *rtree_;
        }
        auto nodes = std::vector<FlatGeobuf::NodeItem>{};
        nodes.reserve(features_.size());
        uint64_t index{0};
        for (auto &feature : features_) {
            nodes.emplace_back(envelope_2d(feature.geometry, index++));
        }
        auto extent = calcExtent(nodes);
        hilbertSort(nodes, extent);
        rtree_ = FlatGeobuf::PackedRTree(nodes, extent);
        return *rtree_;
    }
};
} // namespace cubao

#endif

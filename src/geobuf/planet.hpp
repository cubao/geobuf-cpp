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

    void build() const { this->rtree(); }

    Eigen::VectorXi query(const Eigen::Vector2d &min,
                          const Eigen::Vector2d &max) const
    {
        auto &tree = this->rtree();
        auto hits = tree.search(min[0], min[1], max[0], max[1]);
        Eigen::VectorXi index(hits.size());
        for (int i = 0, N = hits.size(); i < N; ++i) {
            index[i] = hits[i].offset;
        }
        return index;
    }
    FeatureCollection select(const Eigen::VectorXi &index) const
    {
        auto fc = FeatureCollection();
        fc.reserve(index.size());
        for (int i = 0, N = index.size(); i < N; ++i) {
            fc.push_back(features_[index[i]]);
        }
        return fc;
    }

    FeatureCollection crop(const RowVectorsNx2 &polygon,
                           const std::string &clipping_mode = "longest",
                           bool strip_properties = false) const
    {
        auto bbox = row_vectors_to_bbox(polygon);
        auto hits =
            this->query({bbox.min.x, bbox.min.y}, {bbox.max.x, bbox.max.y});
        auto fc = FeatureCollection();
        fc.reserve(hits.size());
        for (auto idx : hits) {
            auto &feature = features_[idx];
            if (!feature.geometry.is<mapbox::geojson::line_string>() ||
                clipping_mode == "whole") {
                // only check centroid
                auto centroid = geometry_to_centroid(feature.geometry);
                auto mask = point_in_polygon(
                    Eigen::Map<const RowVectorsNx2>(&centroid.x, 1, 2),
                    polygon);
                if (mask[0]) {
                    fc.emplace_back(
                        feature.geometry,
                        strip_properties
                            ? mapbox::feature::property_map{{"index", idx}}
                            : feature.properties,
                        feature.id);
                }
                continue;
            }
            auto &line_string =
                feature.geometry.get<mapbox::geojson::line_string>();
            auto polyline = Eigen::Map<const RowVectors>(&line_string[0].x,
                                                         line_string.size(), 3);
            auto segs = polyline_in_polygon(polyline, polygon);
            if (segs.empty()) {
                continue;
            }
            if (clipping_mode == "first") {
                auto &coords = segs.begin()->second;
                auto geom = mapbox::geojson::line_string();
                geom.resize(coords.rows());
                as_row_vectors(geom) = coords;
                fc.emplace_back(
                    geom,
                    strip_properties
                        ? mapbox::feature::property_map{{"index", idx}}
                        : feature.properties,
                    feature.id);
                continue;
            }
            // longest or all
            std::vector<PolylineChunks::key_type> keys;
            keys.reserve(segs.size());
            for (auto &pair : segs) {
                keys.push_back(pair.first);
            }
            if (clipping_mode == "longest") { // else assume all
                auto itr = std::max_element(
                    keys.begin(), keys.end(),
                    [](const auto &k1, const auto &k2) {
                        double len1 = std::get<5>(k1) - std::get<2>(k1);
                        double len2 = std::get<5>(k2) - std::get<2>(k2);
                        return len1 < len2;
                    });
                keys = {*itr}; // pick longest
            }
            for (auto &key : keys) {
                auto &coords = segs[key];
                auto geom = mapbox::geojson::line_string();
                geom.resize(coords.rows());
                as_row_vectors(geom) = coords;
                fc.emplace_back(
                    geom,
                    strip_properties
                        ? mapbox::feature::property_map{{"index", idx}}
                        : feature.properties,
                    feature.id);
            }
        }
        return fc;
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

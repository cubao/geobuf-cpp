#ifndef CUBAO_GEOJSON_CROPPING_HPP
#define CUBAO_GEOJSON_CROPPING_HPP

// https://github.com/microsoft/vscode-cpptools/issues/9692
#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include "cubao/point_in_polygon.hpp"
#include "cubao/polyline_in_polygon.hpp"
#include "geojson_helpers.hpp"
#include <Eigen/Core>
#include <limits>
#include <mapbox/geojson.hpp>
#include <mapbox/geometry.hpp>
#include <mapbox/geometry/envelope.hpp>
#include <optional>

namespace cubao
{
using RowVectors = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;
using RowVectorsNx2 = Eigen::Matrix<double, Eigen::Dynamic, 2, Eigen::RowMajor>;
using BboxType = mapbox::geometry::box<double>;

inline BboxType row_vectors_to_bbox(const RowVectors &coords)
{
    using limits = std::numeric_limits<double>;
    double min_t = limits::has_infinity ? -limits::infinity() : limits::min();
    double max_t = limits::has_infinity ? limits::infinity() : limits::max();
    auto bbox = BboxType({max_t, max_t, max_t}, {min_t, min_t, min_t});
    auto &min = bbox.min;
    auto &max = bbox.max;
    for (int i = 0, N = coords.rows(); i < N; ++i) {
        double x = coords(i, 0);
        double y = coords(i, 1);
        double z = coords(i, 2);
        if (min.x > x)
            min.x = x;
        if (min.y > y)
            min.y = y;
        if (min.z > z)
            min.z = z;
        if (max.x < x)
            max.x = x;
        if (max.y < y)
            max.y = y;
        if (max.z < z)
            max.z = z;
    }
    return bbox;
}

inline BboxType
row_vectors_to_bbox(const Eigen::Ref<const RowVectorsNx2> &coords)
{
    using limits = std::numeric_limits<double>;
    double min_t = limits::has_infinity ? -limits::infinity() : limits::min();
    double max_t = limits::has_infinity ? limits::infinity() : limits::max();
    auto bbox = BboxType({max_t, max_t, 0.0}, {min_t, min_t, 0.0});
    auto &min = bbox.min;
    auto &max = bbox.max;
    for (int i = 0, N = coords.rows(); i < N; ++i) {
        double x = coords(i, 0);
        double y = coords(i, 1);
        if (min.x > x)
            min.x = x;
        if (min.y > y)
            min.y = y;
        if (max.x < x)
            max.x = x;
        if (max.y < y)
            max.y = y;
    }
    return bbox;
}

inline RowVectors bbox2row_vectors(const BboxType &bbox)
{
    auto coords = RowVectors(5, 3);
    coords << bbox.min.x, bbox.min.y, 0.0, //
        bbox.max.x, bbox.min.y, 0.0,       //
        bbox.max.x, bbox.max.y, 0.0,       //
        bbox.min.x, bbox.max.y, 0.0,       //
        bbox.min.x, bbox.min.y, 0.0;
    return coords;
}

inline RowVectors bbox2row_vectors(const Eigen::Vector4d &bbox)
{
    return bbox2row_vectors(BboxType({bbox[0], bbox[1]}, {bbox[2], bbox[3]}));
}

inline mapbox::geojson::point
geometry_to_centroid(const mapbox::geojson::geometry &geom)
{
    auto centroid = mapbox::geojson::point(0, 0, 0);
    int N = 0;
    mapbox::geometry::for_each_point(geom, [&](auto &point) {
        centroid.x += point.x;
        centroid.y += point.y;
        centroid.z += point.z;
        ++N;
    });
    centroid.x /= N;
    centroid.y /= N;
    centroid.z /= N;
    return centroid;
}

inline bool bbox_overlap(const BboxType &bbox1, const BboxType &bbox2,
                         bool check_z = false)
{
    if (check_z && (bbox1.max.z < bbox2.min.z || bbox2.min.z > bbox2.max.z)) {
        return false;
    }
    if (bbox1.max.x < bbox2.min.x || bbox1.min.x > bbox2.max.x) {
        return false;
    }
    if (bbox1.max.y < bbox2.min.y || bbox1.min.y > bbox2.max.y) {
        return false;
    }
    return true;
}

/*
there is a difference between
-   "cropping" (more like in a bigger picture) and
-   "clipping" (more like focused onto something specific, like in tunnel
vision).

clipping_mode
    -   longest
    -   first
    -   all
    -   whole
*/

inline int geojson_cropping(const mapbox::geojson::feature &feature,
                            mapbox::geojson::feature_collection &output,
                            const RowVectors &polygon,
                            std::optional<BboxType> bbox,
                            const std::string &clipping_mode = "longest",
                            const std::optional<double> max_z_offset = {})
{
    if (!bbox) {
        bbox = row_vectors_to_bbox(polygon);
    }
    if (max_z_offset) {
        bbox->min.z -= *max_z_offset;
        bbox->max.z += *max_z_offset;
    }

    if (!bbox_overlap(mapbox::geometry::envelope(feature.geometry), //
                      *bbox,                                        //
                      (bool)max_z_offset)                           // check_z?
    ) {
        return 0;
    }
    if (!feature.geometry.is<mapbox::geojson::line_string>() ||
        clipping_mode == "whole") {
        // only check centroid
        auto centroid = geometry_to_centroid(feature.geometry);
        auto mask =
            point_in_polygon(Eigen::Map<const RowVectorsNx2>(&centroid.x, 1, 2),
                             polygon.leftCols<2>());
        if (mask[0]) {
            output.push_back(feature);
            return 1;
        } else {
            return 0;
        }
    }
    auto &line_string = feature.geometry.get<mapbox::geojson::line_string>();
    auto polyline =
        Eigen::Map<const RowVectors>(&line_string[0].x, line_string.size(), 3);
    auto segs = polyline_in_polygon(polyline, polygon.leftCols<2>());
    if (segs.empty()) {
        return 0;
    }
    if (clipping_mode == "first") {
        auto &coords = segs.begin()->second;
        auto geom = mapbox::geojson::line_string();
        geom.resize(coords.rows());
        as_row_vectors(geom) = coords;
        auto f = feature;
        f.geometry = geom;
        output.push_back(std::move(f));
        return 1;
    }
    // longest or all
    std::vector<PolylineChunks::key_type> keys;
    keys.reserve(segs.size());
    for (auto &pair : segs) {
        keys.push_back(pair.first);
    }
    if (clipping_mode == "longest") { // else assume all
        auto itr = std::max_element(
            keys.begin(), keys.end(), [](const auto &k1, const auto &k2) {
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
        auto f = feature;
        f.geometry = geom;
        output.push_back(std::move(f));
    }
    return keys.size();
}

inline mapbox::geojson::feature_collection
geojson_cropping(const mapbox::geojson::feature_collection &features,
                 const RowVectors &polygon,
                 const std::string &clipping_mode = "longest",
                 const std::optional<double> max_z_offset = {})
{
    auto bbox = row_vectors_to_bbox(polygon);
    bbox.min.z = bbox.max.z = (bbox.min.z + bbox.max.z) / 2.0;
    mapbox::geojson::feature_collection output;
    for (auto &f : features) {
        geojson_cropping(f, output, polygon, bbox, clipping_mode, max_z_offset);
    }
    return output;
}

inline mapbox::geojson::feature_collection
geojson_cropping(const mapbox::geojson::geojson &geojson,
                 const RowVectors &polygon,
                 const std::string &clipping_mode = "longest",
                 const std::optional<double> max_z_offset = {})
{
    return geojson.match(
        [&](const mapbox::geojson::geometry &g) {
            return geojson_cropping(
                mapbox::geojson::feature_collection{
                    mapbox::geojson::feature_collection{g}},
                polygon, clipping_mode, max_z_offset);
        },
        [&](const mapbox::geojson::feature &f) {
            return geojson_cropping(mapbox::geojson::feature_collection{f},
                                    polygon, clipping_mode, max_z_offset);
        },
        [&](const mapbox::geojson::feature_collection &fc) {
            return geojson_cropping(fc, polygon, clipping_mode, max_z_offset);
        });
}

} // namespace cubao

#endif

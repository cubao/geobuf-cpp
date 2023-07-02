#ifndef CUBAO_GEOJSON_CROPPING_HPP
#define CUBAO_GEOJSON_CROPPING_HPP

// https://github.com/microsoft/vscode-cpptools/issues/9692
#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include "cubao/point_in_polygon.hpp"
#include <Eigen/Core>
#include <limits>
#include <mapbox/geojson.hpp>
#include <mapbox/geometry.hpp>
#include <mapbox/geometry/envelope.hpp>
#include <optional>

namespace cubao
{
using RowVectors = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;
using BboxType = mapbox::geometry::box<double>;

inline BboxType row_vectors_to_bbox(const RowVectors &coords)
{
    using limits = std::numeric_limits<double>;
    double min_t = limits::has_infinity ? -limits::infinity() : limits::min();
    double max_t = limits::has_infinity ? limits::infinity() : limits::max();
    auto bbox = BboxType({max_t, max_t, max_t}, {min_t, min_t, min_t});
    auto &min = bbox.min;
    auto &max = bbox.max;
    for (int i = 1, N = coords.rows(); i < N; ++i) {
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
        return false;
    }
    if (!feature.geometry.is<mapbox::geojson::line_string>()) {
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
    return 0;
}

inline mapbox::geojson::feature_collection
geojson_cropping(const mapbox::geojson::feature_collection &features,
                 const RowVectors &polygon,
                 const std::string &clipping_mode = "longest",
                 const std::optional<double> max_z_offset = {})
{
    return mapbox::geojson::feature_collection{};
}

inline mapbox::geojson::feature_collection
geojson_cropping(const mapbox::geojson::geojson &geojson,
                 const RowVectors &polygon,
                 const std::string &clipping_mode = "longest",
                 const std::optional<double> = {})
{
    return mapbox::geojson::feature_collection{};
}

} // namespace cubao

#endif

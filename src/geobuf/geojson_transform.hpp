#pragma once

// https://github.com/microsoft/vscode-cpptools/issues/9692
#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include "geojson_helpers.hpp"
#include <cubao/crs_transform.hpp>
#include <functional>

namespace cubao
{

// Matrix transform function type
using MatrixTransformFn = std::function<void(Eigen::Ref<RowVectors>)>;

// Forward declarations
inline void transform_coords(std::vector<mapbox::geojson::point> &coords,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::point &pt,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::multi_point &geom,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::line_string &geom,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::linear_ring &geom,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::multi_line_string &geom,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::polygon &geom,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::multi_polygon &geom,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::geometry_collection &gc,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::geometry &g,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::feature &f,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::feature_collection &fc,
                             const MatrixTransformFn &fn);
inline void transform_coords(mapbox::geojson::geojson &geojson,
                             const MatrixTransformFn &fn);

// Implementation for vector of points
inline void transform_coords(std::vector<mapbox::geojson::point> &coords,
                             const MatrixTransformFn &fn)
{
    if (coords.empty()) {
        return;
    }
    auto matrix = as_row_vectors(coords);
    fn(matrix);
}

// Implementation for single point
inline void transform_coords(mapbox::geojson::point &pt,
                             const MatrixTransformFn &fn)
{
    auto matrix = as_row_vectors(pt);
    fn(matrix);
}

// Implementation for multi_point (inherits from vector<point>)
inline void transform_coords(mapbox::geojson::multi_point &geom,
                             const MatrixTransformFn &fn)
{
    transform_coords(static_cast<std::vector<mapbox::geojson::point> &>(geom),
                     fn);
}

// Implementation for line_string (inherits from vector<point>)
inline void transform_coords(mapbox::geojson::line_string &geom,
                             const MatrixTransformFn &fn)
{
    transform_coords(static_cast<std::vector<mapbox::geojson::point> &>(geom),
                     fn);
}

// Implementation for linear_ring (inherits from vector<point>)
inline void transform_coords(mapbox::geojson::linear_ring &geom,
                             const MatrixTransformFn &fn)
{
    transform_coords(static_cast<std::vector<mapbox::geojson::point> &>(geom),
                     fn);
}

// Implementation for multi_line_string
inline void transform_coords(mapbox::geojson::multi_line_string &geom,
                             const MatrixTransformFn &fn)
{
    for (auto &ls : geom) {
        transform_coords(ls, fn);
    }
}

// Implementation for polygon
inline void transform_coords(mapbox::geojson::polygon &geom,
                             const MatrixTransformFn &fn)
{
    for (auto &ring : geom) {
        transform_coords(ring, fn);
    }
}

// Implementation for multi_polygon
inline void transform_coords(mapbox::geojson::multi_polygon &geom,
                             const MatrixTransformFn &fn)
{
    for (auto &poly : geom) {
        for (auto &ring : poly) {
            transform_coords(ring, fn);
        }
    }
}

// Implementation for geometry_collection
inline void transform_coords(mapbox::geojson::geometry_collection &gc,
                             const MatrixTransformFn &fn)
{
    for (auto &g : gc) {
        transform_coords(g, fn);
    }
}

// Implementation for geometry (recursive traversal using match)
inline void transform_coords(mapbox::geojson::geometry &g,
                             const MatrixTransformFn &fn)
{
    g.match([&](mapbox::geojson::point &pt) { transform_coords(pt, fn); },
            [&](mapbox::geojson::multi_point &mp) { transform_coords(mp, fn); },
            [&](mapbox::geojson::line_string &ls) { transform_coords(ls, fn); },
            [&](mapbox::geojson::linear_ring &lr) { transform_coords(lr, fn); },
            [&](mapbox::geojson::multi_line_string &mls) {
                transform_coords(mls, fn);
            },
            [&](mapbox::geojson::polygon &poly) { transform_coords(poly, fn); },
            [&](mapbox::geojson::multi_polygon &mpoly) {
                transform_coords(mpoly, fn);
            },
            [&](mapbox::geojson::geometry_collection &gc) {
                transform_coords(gc, fn);
            },
            [](auto &) {});
}

// Implementation for feature
inline void transform_coords(mapbox::geojson::feature &f,
                             const MatrixTransformFn &fn)
{
    transform_coords(f.geometry, fn);
}

// Implementation for feature collection
inline void transform_coords(mapbox::geojson::feature_collection &fc,
                             const MatrixTransformFn &fn)
{
    for (auto &f : fc) {
        transform_coords(f, fn);
    }
}

// Implementation for geojson variant
inline void transform_coords(mapbox::geojson::geojson &geojson,
                             const MatrixTransformFn &fn)
{
    geojson.match(
        [&](mapbox::geojson::geometry &g) { transform_coords(g, fn); },
        [&](mapbox::geojson::feature &f) { transform_coords(f, fn); },
        [&](mapbox::geojson::feature_collection &fc) {
            transform_coords(fc, fn);
        },
        [](auto &) {});
}

// Preset transform function objects

// WGS84 to ENU transform
struct Wgs84ToEnu
{
    Eigen::Vector3d anchor_lla;
    bool cheap_ruler = true;

    void operator()(Eigen::Ref<RowVectors> coords) const
    {
        if (coords.rows() == 0) {
            return;
        }
        coords = lla2enu(coords, anchor_lla, cheap_ruler);
    }
};

// ENU to WGS84 transform
struct EnuToWgs84
{
    Eigen::Vector3d anchor_lla;
    bool cheap_ruler = true;

    void operator()(Eigen::Ref<RowVectors> coords) const
    {
        if (coords.rows() == 0) {
            return;
        }
        coords = enu2lla(coords, anchor_lla, cheap_ruler);
    }
};

// Affine transform (4x4 matrix)
struct AffineTransform
{
    Eigen::Matrix4d T;

    void operator()(Eigen::Ref<RowVectors> coords) const
    {
        if (coords.rows() == 0) {
            return;
        }
        apply_transform_inplace(T, coords);
    }
};

// 3D rotation transform
struct Rotation3D
{
    Eigen::Matrix3d R;

    void operator()(Eigen::Ref<RowVectors> coords) const
    {
        if (coords.rows() == 0) {
            return;
        }
        coords = (R * coords.transpose()).transpose();
    }
};

// 3D translation transform
struct Translation3D
{
    Eigen::Vector3d offset;

    void operator()(Eigen::Ref<RowVectors> coords) const
    {
        if (coords.rows() == 0) {
            return;
        }
        coords.rowwise() += offset.transpose();
    }
};

// 3D scale transform
struct Scale3D
{
    Eigen::Vector3d scale;

    void operator()(Eigen::Ref<RowVectors> coords) const
    {
        if (coords.rows() == 0) {
            return;
        }
        coords.col(0) *= scale[0];
        coords.col(1) *= scale[1];
        coords.col(2) *= scale[2];
    }
};

} // namespace cubao

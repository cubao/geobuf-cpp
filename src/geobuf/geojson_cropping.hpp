#ifndef CUBAO_GEOJSON_CROPPING_HPP
#define CUBAO_GEOJSON_CROPPING_HPP

// https://github.com/microsoft/vscode-cpptools/issues/9692
#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include <Eigen/Core>
#include <mapbox/geojson.hpp>
#include <optional>

namespace cubao
{
using RowVectors = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;

inline bool geojson_cropping(const mapbox::geojson::feature &feature,
                             const RowVectors &polygon,
                             const std::optional<Eigen::Vector4d> &bbox,
                             const std::string &clipping_mode = "longest",
                             const std::optional<double> = {})
{
    return false;
}

inline mapbox::geojson::feature_collection
geojson_cropping(const mapbox::geojson::feature_collection &features,
                 const RowVectors &polygon,
                 const std::string &clipping_mode = "longest",
                 const std::optional<double> = {})
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

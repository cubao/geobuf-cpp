#ifndef CUBAO_GEOJSON_HELPERS_HPP
#define CUBAO_GEOJSON_HELPERS_HPP

// https://github.com/microsoft/vscode-cpptools/issues/9692
#if __INTELLISENSE__
#undef __ARM_NEON
#undef __ARM_NEON__
#endif

#include <Eigen/Core>
#include <iostream>
#include <mapbox/geojson.hpp>

static_assert(sizeof(mapbox::geojson::point) == sizeof(Eigen::Vector3d),
              "mapbox::geojson::point should be double*3 (xyz instead of xy)");

namespace cubao
{
using RowVectors = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;

// non-const version
inline Eigen::Map<RowVectors> as_row_vectors(double *data, int N)
{
    return Eigen::Map<RowVectors>(data, N, 3);
}
inline Eigen::Map<RowVectors>
as_row_vectors(std::vector<std::array<double, 3>> &data)
{
    return Eigen::Map<RowVectors>(&data[0][0], data.size(), 3);
}
inline Eigen::Map<RowVectors>
as_row_vectors(std::vector<mapbox::geojson::point> &data)
{
    return Eigen::Map<RowVectors>(&data[0].x, data.size(), 3);
}
inline Eigen::Map<RowVectors> as_row_vectors(mapbox::geojson::point &geom)
{
    return as_row_vectors(&geom.x, 1);
}
inline Eigen::Map<RowVectors> as_row_vectors(mapbox::geojson::multi_point &geom)
{
    return as_row_vectors(&geom[0].x, geom.size());
}
inline Eigen::Map<RowVectors> as_row_vectors(mapbox::geojson::line_string &geom)
{
    return as_row_vectors(&geom[0].x, geom.size());
}
inline Eigen::Map<RowVectors> as_row_vectors(mapbox::geojson::linear_ring &geom)
{
    return as_row_vectors(&geom[0].x, geom.size());
}
inline Eigen::Map<RowVectors>
as_row_vectors(mapbox::geojson::multi_line_string &geom)
{
    return as_row_vectors(geom[0]);
}
inline Eigen::Map<RowVectors> as_row_vectors(mapbox::geojson::polygon &geom)
{
    return as_row_vectors(&geom[0][0].x, geom[0].size());
}
inline Eigen::Map<RowVectors>
as_row_vectors(mapbox::geojson::multi_polygon &geom)
{
    auto &shell = geom[0];
    return as_row_vectors(shell);
}
inline Eigen::Map<RowVectors> as_row_vectors(mapbox::geojson::geometry &geom)
{
    return geom.match(
        [](mapbox::geojson::point &g) { return as_row_vectors(g); },
        [](mapbox::geojson::multi_point &g) { return as_row_vectors(g); },
        [](mapbox::geojson::line_string &g) { return as_row_vectors(g); },
        [](mapbox::geojson::linear_ring &g) { return as_row_vectors(g); },
        [](mapbox::geojson::multi_line_string &g) { return as_row_vectors(g); },
        [](mapbox::geojson::polygon &g) { return as_row_vectors(g); },
        [](mapbox::geojson::multi_polygon &g) { return as_row_vectors(g); },
        [](auto &) -> Eigen::Map<RowVectors> {
            return as_row_vectors((double *)0, 0);
        });
}

// const version
inline Eigen::Map<const RowVectors> as_row_vectors(const double *data, int N)
{
    return Eigen::Map<const RowVectors>(data, N, 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const std::vector<std::array<double, 3>> &data)
{
    return Eigen::Map<const RowVectors>(&data[0][0], data.size(), 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const std::vector<mapbox::geojson::point> &data)
{
    return Eigen::Map<const RowVectors>(&data[0].x, data.size(), 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::point &geom)
{
    return Eigen::Map<const RowVectors>(&geom.x, 1, 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::multi_point &geom)
{
    return Eigen::Map<const RowVectors>(&geom[0].x, geom.size(), 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::line_string &geom)
{
    return Eigen::Map<const RowVectors>(&geom[0].x, geom.size(), 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::linear_ring &geom)
{
    return Eigen::Map<const RowVectors>(&geom[0].x, geom.size(), 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::multi_line_string &geom)
{
    auto &ls = geom[0];
    return Eigen::Map<const RowVectors>(&ls[0].x, ls.size(), 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::polygon &geom)
{
    auto &shell = geom[0];
    return Eigen::Map<const RowVectors>(&shell[0].x, shell.size(), 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::multi_polygon &geom)
{
    auto &poly = geom[0];
    return as_row_vectors(poly);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::geometry &geom)
{
    return geom.match(
        [](const mapbox::geojson::point &g) { return as_row_vectors(g); },
        [](const mapbox::geojson::multi_point &g) { return as_row_vectors(g); },
        [](const mapbox::geojson::line_string &g) { return as_row_vectors(g); },
        [](const mapbox::geojson::linear_ring &g) { return as_row_vectors(g); },
        [](const mapbox::geojson::multi_line_string &g) {
            return as_row_vectors(g);
        },
        [](const mapbox::geojson::polygon &g) { return as_row_vectors(g); },
        [](const mapbox::geojson::multi_polygon &g) {
            return as_row_vectors(g);
        },
        [](const auto &) -> Eigen::Map<const RowVectors> {
            return as_row_vectors((const double *)0, 0);
        });
}

inline std::string geometry_type(const mapbox::geojson::geometry &self)
{
    return self.match(
        [](const mapbox::geojson::point &g) { return "Point"; },
        [](const mapbox::geojson::line_string &g) { return "LineString"; },
        [](const mapbox::geojson::polygon &g) { return "Polygon"; },
        [](const mapbox::geojson::multi_point &g) { return "MultiPoint"; },
        [](const mapbox::geojson::multi_line_string &g) {
            return "MultiLineString";
        },
        [](const mapbox::geojson::multi_polygon &g) { return "MultiPolygon"; },
        [](const mapbox::geojson::geometry_collection &g) {
            return "GeometryCollection";
        },
        [](const auto &g) -> std::string { return "None"; });
}

inline void eigen2geom(const Eigen::MatrixXd &mat,
                       std::vector<mapbox::geojson::point> &points)
{
    if (mat.rows() == 0) {
        points.clear();
        return;
    }
    if (mat.cols() != 2 && mat.cols() != 3) {
        return;
    }
    points.resize(mat.rows());
    Eigen::Map<RowVectors> M(&points[0].x, points.size(), 3);
    M.leftCols(mat.cols()) = mat;
}

using MatrixXdRowMajor =
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

inline void eigen2geom(Eigen::Ref<const MatrixXdRowMajor> mat,
                       std::vector<mapbox::geojson::point> &points)
{
    if (mat.rows() == 0) {
        points.clear();
        return;
    }
    if (mat.cols() != 2 && mat.cols() != 3) {
        return;
    }
    points.resize(mat.rows());
    Eigen::Map<RowVectors> M(&points[0].x, points.size(), 3);
    M.leftCols(mat.cols()) = mat;
}

inline void eigen2geom(Eigen::Ref<const MatrixXdRowMajor> points,
                       mapbox::geojson::point &g)
{
    Eigen::Vector3d::Map(&g.x) = points.row(0).transpose();
}

inline void eigen2geom(Eigen::Ref<const MatrixXdRowMajor> points,
                       mapbox::geojson::multi_line_string &g)
{
    g.resize(1);
    eigen2geom(points, g[0]);
}

inline void eigen2geom(Eigen::Ref<const MatrixXdRowMajor> points,
                       mapbox::geojson::polygon &g)
{
    g.resize(1);
    eigen2geom(points, g[0]);
}

inline void eigen2geom(Eigen::Ref<const MatrixXdRowMajor> points,
                       mapbox::geojson::multi_polygon &g)
{
    g.resize(1);
    g[0].resize(1);
    eigen2geom(points, g[0][0]);
}

inline void eigen2geom(Eigen::Ref<const MatrixXdRowMajor> points,
                       mapbox::geojson::geometry &geom)
{
    geom.match(
        [&](mapbox::geojson::point &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::line_string &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::polygon &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::multi_point &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::multi_line_string &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::multi_polygon &g) { eigen2geom(points, g); },
        [&](auto &g) -> void {
            std::cerr << "eigen2geom not handled for this type: "
                      << geometry_type(g) << std::endl;
        });
}

inline std::string get_type(const mapbox::geojson::value &self)
{
    return self.match(
        [](const mapbox::geojson::value::array_type &) { return "array"; },
        [](const mapbox::geojson::value::object_type &) { return "object"; },
        [](const bool &) { return "bool"; },
        [](const uint64_t &) { return "uint64_t"; },
        [](const int64_t &) { return "int64_t"; },
        [](const double &) { return "double"; },
        [](const std::string &) { return "string"; },
        [](const auto &) -> std::string { return "null"; });
}

inline void geometry_push_back(mapbox::geojson::geometry &self,
                               const mapbox::geojson::point &point)
{
    self.match([&](mapbox::geojson::multi_point &g) { g.push_back(point); },
               [&](mapbox::geojson::line_string &g) { g.push_back(point); },
               [&](auto &) {
                   // TODO, log
               });
}

inline void geometry_push_back(mapbox::geojson::geometry &self,
                               const Eigen::VectorXd &point)
{
    auto geom = mapbox::geojson::point(point[0], point[1],
                                       point.size() > 2 ? point[2] : 0.0);
    geometry_push_back(self, geom);
}

inline void geometry_push_back(mapbox::geojson::geometry &self,
                               const mapbox::geojson::geometry &geom)
{
    self.match(
        [&](mapbox::geojson::geometry_collection &g) { g.push_back(geom); },
        [&](auto &) {});
}

inline void geometry_pop_back(mapbox::geojson::geometry &self)
{
    // TODO
}
inline void geometry_clear(mapbox::geojson::geometry &self)
{
    // TODO
}

inline void clear_geojson_value(mapbox::geojson::value &self)
{
    self.match([](mapbox::geojson::value::array_type &arr) { arr.clear(); },
               [](mapbox::geojson::value::object_type &obj) { obj.clear(); },
               [](bool &b) { b = false; }, [](uint64_t &i) { i = 0UL; },
               [](int64_t &i) { i = 0; }, [](double &d) { d = 0.0; },
               [](std::string &str) { str.clear(); }, [](auto &) -> void {});
}
} // namespace cubao

#endif

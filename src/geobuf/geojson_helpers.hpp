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
    if (geom.empty()) {
        return Eigen::Map<RowVectors>((double *)0, 0, 3);
    }
    return as_row_vectors(geom[0]);
}
inline Eigen::Map<RowVectors> as_row_vectors(mapbox::geojson::polygon &geom)
{
    if (geom.empty() || geom.front().empty()) {
        return Eigen::Map<RowVectors>((double *)0, 0, 3);
    }
    return as_row_vectors(&geom[0][0].x, geom[0].size());
}
inline Eigen::Map<RowVectors>
as_row_vectors(mapbox::geojson::multi_polygon &geom)
{
    if (geom.empty()) {
        return Eigen::Map<RowVectors>((double *)0, 0, 3);
    }
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
    if (geom.empty()) {
        return Eigen::Map<const RowVectors>((const double *)0, 0, 3);
    }
    auto &ls = geom[0];
    return Eigen::Map<const RowVectors>(&ls[0].x, ls.size(), 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::polygon &geom)
{
    if (geom.empty() || geom.front().empty()) {
        return Eigen::Map<const RowVectors>((const double *)0, 0, 3);
    }
    auto &shell = geom[0];
    return Eigen::Map<const RowVectors>(&shell[0].x, shell.size(), 3);
}
inline Eigen::Map<const RowVectors>
as_row_vectors(const mapbox::geojson::multi_polygon &geom)
{
    if (geom.empty()) {
        return Eigen::Map<const RowVectors>((const double *)0, 0, 3);
    }
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

using MatrixXdRowMajor =
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

inline void eigen2geom(const Eigen::Ref<const MatrixXdRowMajor> &mat,
                       std::vector<mapbox::geojson::point> &points)
{
    if (mat.rows() == 0) {
        points.clear();
        return;
    }
    const int cols = mat.cols();
    if (cols != 2 && cols != 3) {
        throw std::invalid_argument(
            "matrix shape expected to be Nx2 or Nx3, actual=" +
            std::to_string(mat.rows()) + "x" + std::to_string(cols));
        return;
    }
    points.resize(mat.rows());
    Eigen::Map<RowVectors> M(&points[0].x, points.size(), 3);
    M.leftCols(mat.cols()) = mat;
    if (cols == 2) {
        M.col(2).setZero();
    }
}

inline mapbox::geojson::point eigen2geom(const Eigen::VectorXd &xyz)
{
    return {xyz[0], xyz[1], xyz.size() > 2 ? xyz[2] : 0.0};
}

inline void eigen2geom(const Eigen::Ref<const MatrixXdRowMajor> &points,
                       mapbox::geojson::point &g)
{
    g.x = points(0, 0);
    g.y = points(0, 1);
    g.z = points.cols() > 2 ? points(0, 2) : 0.0;
}

inline void eigen2geom(const Eigen::Ref<const MatrixXdRowMajor> &points,
                       mapbox::geojson::multi_line_string &g)
{
    g.resize(1);
    eigen2geom(points, g[0]);
}

inline void eigen2geom(const Eigen::Ref<const MatrixXdRowMajor> &points,
                       mapbox::geojson::polygon &g)
{
    g.resize(1);
    eigen2geom(points, g[0]);
}

inline void eigen2geom(const Eigen::Ref<const MatrixXdRowMajor> &points,
                       mapbox::geojson::multi_polygon &g)
{
    g.resize(1);
    g[0].resize(1);
    eigen2geom(points, g[0][0]);
}

inline void eigen2geom(const Eigen::Ref<const MatrixXdRowMajor> &points,
                       mapbox::geojson::geometry &geom)
{
    geom.match(
        [&](mapbox::geojson::point &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::line_string &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::polygon &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::multi_point &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::multi_line_string &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::multi_polygon &g) { eigen2geom(points, g); },
        [&](mapbox::geojson::geometry_collection &g) {
            g.resize(1);
            eigen2geom(points, g[0]);
        },
        [&](auto &g) -> void {
            std::cerr << "eigen2geom not handled for this type: "
                      << geometry_type(g) << std::endl;
        });
}

inline void eigen2geom(const Eigen::Ref<const MatrixXdRowMajor> &points,
                       mapbox::geojson::geometry_collection &g)
{
    g.resize(1);
    eigen2geom(points, g[0]);
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
               [&](mapbox::geojson::multi_line_string &g) {
                   g.back().push_back(point);
               },
               [&](mapbox::geojson::polygon &g) { g.back().push_back(point); },
               [&](mapbox::geojson::multi_polygon &g) {
                   g.back().back().push_back(point);
               },
               [&](auto &g) {
                   std::cerr << "geometry_push_back not handled for this type: "
                             << geometry_type(g) << std::endl;
               });
}

inline void geometry_push_back(mapbox::geojson::geometry &self,
                               const Eigen::VectorXd &point)
{
    geometry_push_back(self, eigen2geom(point));
}

inline void geometry_push_back(mapbox::geojson::geometry &self,
                               const Eigen::Ref<const MatrixXdRowMajor> &points)
{
    self.match(
        [&](mapbox::geojson::multi_line_string &g) {
            g.push_back({});
            eigen2geom(points, g.back());
        },
        [&](mapbox::geojson::polygon &g) {
            g.push_back({});
            eigen2geom(points, g.back());
        },
        [&](auto &g) {
            std::cerr << "geometry_push_back not handled for this type: "
                      << geometry_type(g) << std::endl;
        });
}

inline void geometry_push_back(mapbox::geojson::geometry &self,
                               const mapbox::geojson::geometry &geom)
{
    self.match(
        [&](mapbox::geojson::geometry_collection &g) { g.push_back(geom); },
        [&](auto &g) {
            std::cerr << "geometry_push_back(geom) not handled for this type: "
                      << geometry_type(g) << std::endl;
        });
}

inline void geometry_pop_back(mapbox::geojson::geometry &self)
{
    self.match([&](mapbox::geojson::multi_point &g) { g.pop_back(); },
               [&](mapbox::geojson::line_string &g) { g.pop_back(); },
               [&](mapbox::geojson::multi_line_string &g) {
                   g.back().pop_back();
                   // not g.pop_back()
               },
               [&](mapbox::geojson::polygon &g) {
                   g.back().pop_back();
                   // not g.pop_back()
               },
               [&](mapbox::geojson::multi_polygon &g) {
                   g.back().back().pop_back();
                   // not g.pop_back()
               },
               [&](auto &g) {
                   std::cerr
                       << "geometry_pop_back() not handled for this type: "
                       << geometry_type(g) << std::endl;
               });
}
inline void geometry_clear(mapbox::geojson::geometry &self)
{
    self.match([&](mapbox::geojson::multi_point &g) { g.clear(); },
               [&](mapbox::geojson::line_string &g) { g.clear(); },
               [&](mapbox::geojson::multi_line_string &g) {
                   g.clear();
                   // not g.back().clear();
               },
               [&](mapbox::geojson::polygon &g) {
                   g.clear();
                   // not g.back().clear();
               },
               [&](mapbox::geojson::multi_polygon &g) {
                   g.clear();
                   // not g.back().back().clear();
               },
               [&](mapbox::geojson::point &g) { g.x = g.y = g.z = 0.0; },
               [&](mapbox::geojson::geometry_collection &g) { g.clear(); },
               [&](auto &g) {
                   std::cerr << "geometry_clear() not handled for this type: "
                             << geometry_type(g) << std::endl;
               });
}

inline void geojson_value_clear(mapbox::geojson::value &self)
{
    self.match([](mapbox::geojson::value::array_type &arr) { arr.clear(); },
               [](mapbox::geojson::value::object_type &obj) { obj.clear(); },
               [](bool &b) { b = false; }, [](uint64_t &i) { i = 0UL; },
               [](int64_t &i) { i = 0; }, [](double &d) { d = 0.0; },
               [](std::string &str) { str.clear(); }, [](auto &) -> void {});
}

inline bool __bool__(const mapbox::geojson::value &self)
{
    return self.match(
        [](const mapbox::geojson::value::object_type &obj) {
            return !obj.empty();
        },
        [](const mapbox::geojson::value::array_type &arr) {
            return !arr.empty();
        },
        [](const bool &b) { return b; },
        [](const uint64_t &i) { return i != 0; },
        [](const int64_t &i) { return i != 0; },
        [](const double &d) { return d != 0; },
        [](const std::string &s) { return !s.empty(); },
        [](const mapbox::geojson::null_value_t &) { return false; },
        [](auto &v) -> bool { return false; });
}

inline int __len__(const mapbox::geojson::value &self)
{
    return self.match(
        [](const mapbox::geojson::value::array_type &arr) {
            return arr.size();
        },
        [](const mapbox::geojson::value::object_type &obj) {
            return obj.size();
        },
        [](auto &) -> int { return 0; });
}
inline int __len__(mapbox::geojson::geometry &self)
{
    return self.match(
        [](mapbox::geojson::point &g) { return 3; },
        [](mapbox::geojson::multi_point &g) { return g.size(); },
        [](mapbox::geojson::line_string &g) { return g.size(); },
        [](mapbox::geojson::linear_ring &g) { return g.size(); },
        [](mapbox::geojson::multi_line_string &g) { return g.size(); },
        [](mapbox::geojson::polygon &g) { return g.size(); },
        [](mapbox::geojson::multi_polygon &g) { return g.size(); },
        [](mapbox::geojson::geometry_collection &g) { return g.size(); },
        [](auto &) -> int { return 0; });
}

} // namespace cubao

#endif

#include "geobuf/geobuf.hpp"
#include "geobuf/geojson_helpers.hpp"
#include "geobuf/rapidjson_helpers.hpp"

#define DBG_MACRO_NO_WARNING
#include "dbg.h"

#include <iostream>

int main(int argc, char *argv[])
{
    {
        mapbox::geojson::geometry g;
        dbg(mapbox::geojson::stringify(g));
    }
    {
        mapbox::geojson::geometry g = mapbox::geojson::point(1, 2, 3);
        dbg(mapbox::geojson::stringify(g));
    }
    {
        mapbox::geojson::geometry g =
            mapbox::geojson::multi_point{{1, 2, 3}, {4, 5, 6}};
        dbg(mapbox::geojson::stringify(g));
    }
    {
        auto g1 = mapbox::geojson::point();
        auto g2 = mapbox::geojson::multi_point{{1, 2, 3}, {4, 5, 6}};
        auto g3 = mapbox::geojson::polygon{
            {{1, 2, 3}, {4, 5, 6}, {7, 8, 0}, {1, 2, 3}}};
        auto g = mapbox::geojson::geometry_collection{g1, g2, g3};
        // dbg(mapbox::geojson::stringify(g));
        auto gg = mapbox::geojson::geometry(g);
        dbg(mapbox::geojson::stringify(gg));
    }
    return 0;
}

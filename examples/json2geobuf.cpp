#include "geobuf/geobuf.hpp"
#include "geobuf/rapidjson_helpers.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    // usage:
    //      cat input.json | json2geobuf > output.pbf
    //      json2geobuf input.json > output.pbf
    //      json2geobuf input.json output.pbf
    const char *precision = std::getenv("GEOBUF_PRECISION"); // default 6
    auto encoder =
        precision ? mapbox::geobuf::Encoder(std::pow(10, std::atoi(precision)))
                  : mapbox::geobuf::Encoder();
    auto json = argc > 1 ? cubao::load_json(argv[1]) : cubao::load_json();
    auto geojson = mapbox::geojson::convert(json);
    auto pbf = encoder.encode(geojson);
    if (argc > 2) {
        return mapbox::geobuf::dump_bytes(argv[2], pbf) ? 0 : -1;
    }
    std::cout << pbf;
    return 0;
}

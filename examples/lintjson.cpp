#include "geobuf/geobuf.hpp"
#include "geobuf/rapidjson_helpers.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    // usage:
    //      cat input.json | lintjson > output.json
    //      lintjson input.json > output.json
    //      lintjson input.json output.json
    auto json = argc > 1 ? cubao::load_json(argv[1]) : cubao::load_json();
    return (argc > 2 ? mapbox::geobuf::dump_json(argv[2], json, true)
                     : mapbox::geobuf::dump_json(json, true))
               ? 0
               : -1;
}

#include "geobuf/geobuf.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    // usage:
    //      cat input.json | normalize_json > output.json
    //      normalize_json input.json > output.json
    //      normalize_json input.json output.json
    auto json = argc > 1 ? mapbox::geobuf::load_json(argv[1])
                         : mapbox::geobuf::load_json();
    mapbox::geobuf::sort_keys_inplace(json);
    return (argc > 2 ? mapbox::geobuf::dump_json(argv[2], json, true)
                     : mapbox::geobuf::dump_json(json, true))
               ? 0
               : -1;
}

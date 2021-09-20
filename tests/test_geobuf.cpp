#include "geobuf/geobuf.hpp"
#include "geobuf/version.h"

#define DBG_MACRO_NO_WARNING
#include <dbg.h>

// https://github.com/onqtam/doctest
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

std::string FIXTURES_DIR = PROJECT_SOURCE_DIR "/geobuf/test/fixtures";
std::vector<std::string> FIXTURES = {
    "issue55.json", //
    // "issue62.json",          //
    // "issue90.json",          //
    // "precision.json",        //
    // "props.json",            //
    // "single-multipoly.json", //
};

void roundtripTest(const std::string &fixture_basename)
{
    auto input =
        dbg(std::string{FIXTURES_DIR + std::string("/") + fixture_basename});
    auto output = dbg(std::string{PROJECT_BINARY_DIR + std::string("/") +
                                  fixture_basename + ".pbf"});

    // js version
    // npm install -g geobuf
    // json2geobuf input.json > output.pbf

    auto geojson = mapbox::geojson::convert(mapbox::geobuf::load_json(input));
    auto encoder = mapbox::geobuf::Encoder();
    auto pbf = encoder.encode(geojson);
    dbg(pbf.size());
    mapbox::geobuf::dump_bytes(output, pbf);
    dbg(mapbox::geobuf::Decoder::to_printable(pbf));
    dbg("done");

    // build/bin/pbf_decoder build/issue55.json.pbf
}

TEST_CASE("read write json")
{
    for (auto &basename : FIXTURES) {
        auto input =
            dbg(std::string{FIXTURES_DIR + std::string("/") + basename});
        auto output =
            dbg(std::string{PROJECT_BINARY_DIR + std::string("/") + basename});
        auto json = mapbox::geobuf::load_json(input);
        CHECK(mapbox::geobuf::dump_json(output, json));

        auto bytes = mapbox::geobuf::load_bytes(output);
        CHECK(bytes == mapbox::geobuf::dump(json));
    }
}

TEST_CASE("load configs")
{
    dbg(FIXTURES_DIR);
    dbg(FIXTURES);
    for (auto &path : FIXTURES) {
        roundtripTest(path);
    }
}

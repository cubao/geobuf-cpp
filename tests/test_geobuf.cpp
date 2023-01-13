#include "geobuf/geobuf.hpp"
#include "geobuf/version.h"

#define DBG_MACRO_NO_WARNING
#include <dbg.h>

// https://github.com/onqtam/doctest
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

std::string FIXTURES_DIR = PROJECT_SOURCE_DIR "/geobuf/test/fixtures";
std::vector<std::string> FIXTURES = {
    "issue55.json",          //
    "issue62.json",          //
    "issue90.json",          //
    "precision.json",        //
    "props.json",            //
    "single-multipoly.json", //
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

    auto decoder = mapbox::geobuf::Decoder();
    auto geojson_back = decoder.decode(pbf);
    dbg(mapbox::geobuf::dump(geojson_back, false));
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

TEST_CASE("decoding test")
{
    auto path = std::string(PROJECT_BINARY_DIR "/js/sample1.json.pbf");
    auto decoder = mapbox::geobuf::Decoder();
    auto geojson = decoder.decode(mapbox::geobuf::load_bytes(path));
    dbg(mapbox::geojson::stringify(geojson));

    double lon = 119.88281249999999;
    dbg(lon);
    dbg(lon * 1e6);
    dbg(lon * 1000000);
    dbg(std::round(lon * 1e6));
    std::cout << std::setprecision(10)                                //
              << ", lon:" << lon                                      //
              << ", lon*1e6:" << lon * 1e6                            //
              << ", lon*1000000:" << lon * 1000000                    //
              << ", round(lon*1e6):" << std::round(lon * 1e6)         //
              << ", round(lon*1000000):" << std::round(lon * 1000000) //
              << std::endl;
    //      119.88281249999999,
    // js:  119.882812,
    // cxx: 119.882813,
}

TEST_CASE("custom properties test")
{
    {
        mapbox::geojson::feature_collection fc;
        fc.custom_properties["answer"] = 42;
        dbg(mapbox::geobuf::dump(fc.custom_properties));
        auto geojson = mapbox::geojson::geojson{fc};
        dbg(mapbox::geobuf::dump(
            geojson.get<mapbox::geojson::feature_collection>()
                .custom_properties));
        CHECK(geojson.get<mapbox::geojson::feature_collection>()
                  .custom_properties.size() == 1);

        mapbox::geojson::geojson geojson2 = fc;
        dbg(fc.custom_properties.size());
        dbg(mapbox::geobuf::dump(
            geojson2.get<mapbox::geojson::feature_collection>()
                .custom_properties));
        geojson2.get<mapbox::geojson::feature_collection>().custom_properties =
            fc.custom_properties;
        dbg(mapbox::geobuf::dump(
            geojson2.get<mapbox::geojson::feature_collection>()
                .custom_properties));
        CHECK(geojson2.get<mapbox::geojson::feature_collection>()
                  .custom_properties.size() == 1);

        mapbox::geojson::feature_collection geojson3 = fc;
        dbg(mapbox::geobuf::dump(geojson3.custom_properties));
        dbg(mapbox::geobuf::dump(fc.custom_properties));
        CHECK(geojson3.custom_properties.size() == 1);
    }

    mapbox::geojson::feature feature;
    dbg(feature.custom_properties.size());
    auto path = std::string(PROJECT_SOURCE_DIR "/data/sample1.json");
    auto json = mapbox::geobuf::load_json(path);
    auto geojson = mapbox::geojson::convert(json);
    auto &fc = geojson.get<mapbox::geojson::feature_collection>();
    dbg(fc.size());
    dbg(fc.custom_properties.size());
    dbg(mapbox::geobuf::dump(fc.custom_properties));
    auto &f = fc[0];
    dbg(f.custom_properties.size());
    dbg(mapbox::geobuf::dump(f.custom_properties));
    CHECK(fc.custom_properties.size() == 2);
    CHECK(f.custom_properties.size() == 3);
}

TEST_CASE("roundtrip")
{
    for (auto &basename : FIXTURES) {
        roundtripTest(basename);
    }
}

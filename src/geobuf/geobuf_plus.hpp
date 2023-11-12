#pragma once
#include "geobuf.hpp"
#include "planet.hpp"
#include <spdlog/spdlog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mio/mio.hpp>

namespace cubao
{

struct GeobufPlus
{
    GeobufPlus() = default;
    mio::shared_ummap_source mmap;

    bool mmap_init(const std::string &path) {
        mmap = std::make_shared<mio::ummap_source>(path);
        int cursor = 10;
        if (std::string((const char *)mmap.data(), cursor) != "GeobufPlus") {
            spdlog::error("invalid geobuf plus file, wrong magic");
            return false;
        }
        auto xx =  mmap[cursor];
        int num_features = *(const int*)(mmap.data() + cursor);
        spdlog::info("#features: {}", num_features);
        return true;
    }

    void init(const std::string &header_bytes) {}
    mapbox::geojson::feature_collection decode(const std::string &bytes) const
    {
        return {};
    }

    mapbox::geojson::feature decode_one_feature(const char *ptr) const
    {
        return mapbox::geojson::feature{};
    }

    mapbox::geojson::feature_collection
    decode(const void *data, const std::vector<int> &offsets) const
    {
        return {};
    }

    mapbox::feature::property_map
    decode_non_features(const char *tail_start, const char *tail_end) const
    {
        return {};
    }

    static std::string encode(const Planet &planet) { return ""; }

    static bool encode(const std::string &input_geojson_path,
                       const std::string &output_geobuf_plus_path,
                       uint8_t precision = 8, bool only_xy = false,
                       std::optional<int> round_z = 3)
    {
        spdlog::info("Loading {} ...", input_geojson_path);
        auto json = mapbox::geobuf::load_json(input_geojson_path);
        if (json.IsNull()) {
            spdlog::error("Invalid input. Abort");
            return false;
        }
        auto geojson = mapbox::geojson::convert(json);
        if (geojson.is<mapbox::geojson::geometry>()) {
            geojson =
                mapbox::geojson::feature_collection{mapbox::geojson::feature{
                    geojson.get<mapbox::geojson::geometry>()}};
        } else if (geojson.is<mapbox::geojson::feature>()) {
            geojson = mapbox::geojson::feature_collection{
                geojson.get<mapbox::geojson::feature>()};
        }
        auto &fc = geojson.get<mapbox::geojson::feature_collection>();
        spdlog::info("encoding {} features...", fc.size());

        auto planet = Planet(fc);
        FILE *fp = fopen(output_geobuf_plus_path.c_str(), "wb");
        if (!fp) {
            spdlog::error("failed to open {} for writing",
                          output_geobuf_plus_path);
            return false;
        }
        // magic
        fwrite("GeobufPlus", 10, 1, fp);
        int num_features = fc.size();
        // #features
        fwrite(&num_features, sizeof(num_features), 1, fp);

        auto &rtree = planet.packed_rtree();
        auto extent = rtree.getExtent();
        // spatial
        //      extent/bbox
        fwrite(&extent, sizeof(extent), 1, fp);
        //      num_items
        int num_items = rtree.getNumItems();
        fwrite(&num_items, sizeof(num_items), 1, fp);
        //      num_nodes
        int num_nodes = rtree.getNumNodes();
        fwrite(&num_nodes, sizeof(num_nodes), 1, fp);
        //      node_size
        int node_size = rtree.getNodeSize();
        fwrite(&node_size, sizeof(node_size), 1, fp);
        //      tree_size
        int tree_size = rtree.size();
        fwrite(&tree_size, sizeof(tree_size), 1, fp);
        //      tree_bytes
        rtree.streamWrite([&](const uint8_t *data, size_t size) {
            fwrite(data, 1, size, fp);
        });

        const int padding = 0;
        fwrite(&padding, sizeof(padding), 1, fp);

        mapbox::geobuf::Encoder encoder(std::pow(10, precision), only_xy,
                                        round_z);
        auto bytes = encoder.encode(geojson);
        std::vector<int> offsets = encoder.__offsets();
        int num_offsets = offsets.size();
        fwrite(&num_offsets, sizeof(num_offsets), 1, fp);
        fwrite(offsets.data(), sizeof(offsets[0]), offsets.size(), fp);
        fwrite(&padding, sizeof(padding), 1, fp);
        fwrite(bytes.data(), 1, bytes.size(), fp);

        fclose(fp);
        return true;
        // write magic, geobuf_plus
        // write num_features, bbox
        // write num_tree_bytes, tree
        // write num_offsets, tree
        // geobuf
    }
};
} // namespace cubao

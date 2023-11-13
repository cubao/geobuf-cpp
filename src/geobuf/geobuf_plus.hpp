#pragma once
#include "geobuf.hpp"
#include "planet.hpp"
#include <spdlog/spdlog.h>

#include <fcntl.h>
#include <mio/mio.hpp>
#include <sys/stat.h>
#include <sys/types.h>

namespace cubao
{

struct GeobufPlus
{
    GeobufPlus() = default;
    mio::shared_ummap_source mmap;
    int num_features;
    FlatGeobuf::PackedRTree rtree;

    bool mmap_init(const std::string &path)
    {
        mmap = std::make_shared<mio::ummap_source>(path);
        int cursor = 10;
        if (std::string((const char *)mmap.data(), cursor) != "GeobufPlus") {
            spdlog::error("invalid geobuf plus file, wrong magic");
            return false;
        }
        auto xx = mmap[cursor];
        int num_features = *(const int *)(mmap.data() + cursor);
        spdlog::info("#features: {}", num_features);

        FlatGeobuf::NodeItem extent;
        int num_items;
        int num_nodes;
        int node_size;

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
                       const std::string &output_index_path,
                       const std::string &output_geobuf_path,
                       uint8_t precision = 8, bool only_xy = false,
                       std::optional<int> round_z = 3)
    {
        spdlog::info("loading {} ...", input_geojson_path);
        auto json = mapbox::geobuf::load_json(input_geojson_path);
        if (json.IsNull()) {
            spdlog::error("invalid input, abort");
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
        FILE *fp = fopen(output_index_path.c_str(), "wb");
        if (!fp) {
            spdlog::error("failed to open {} for writing",
                          output_index_path);
            return false;
        }
        // magic
        fwrite("GeobufPlus", 10, 1, fp);
        int num_features = fc.size();
        // #features
        fwrite(&num_features, sizeof(num_features), 1, fp);

        auto &rtree = planet.packed_rtree();
        auto extent = rtree.getExtent();
        // extent/bbox
        fwrite(&extent, sizeof(extent), 1, fp);
        // num_items
        int num_items = rtree.getNumItems();
        fwrite(&num_items, sizeof(num_items), 1, fp);
        // num_nodes
        int num_nodes = rtree.getNumNodes();
        fwrite(&num_nodes, sizeof(num_nodes), 1, fp);
        // node_size
        int node_size = rtree.getNodeSize();
        fwrite(&node_size, sizeof(node_size), 1, fp);
        // tree_size
        int tree_size = rtree.size();
        fwrite(&tree_size, sizeof(tree_size), 1, fp);
        // tree_bytes
        rtree.streamWrite([&](const uint8_t *data, size_t size) {
            fwrite(data, 1, size, fp);
        });

        const int padding = 930604; // geobuf
        fwrite(&padding, sizeof(padding), 1, fp);

        auto encoder = mapbox::geobuf::Encoder(std::pow(10, precision), only_xy, round_z);
        auto bytes = encoder.encode(geojson);
        std::vector<int> offsets = encoder.__offsets();
        int num_offsets = offsets.size();
        fwrite(&num_offsets, sizeof(num_offsets), 1, fp);
        fwrite(offsets.data(), sizeof(offsets[0]), offsets.size(), fp);
        fwrite(&padding, sizeof(padding), 1, fp);
        fclose(fp);
        spdlog::info("wrote index to {}", output_index_path);

        if (mapbox::geobuf::dump_bytes(output_geobuf_path, bytes)) {
            spdlog::error("wrote geobuf to {}", output_geobuf_path);
            return true;
        } else {
            spdlog::error("failed to write to {}", output_geobuf_path);
            return false;
        }
    }
};
} // namespace cubao

#pragma once
#include "geobuf.hpp"
#include "planet.hpp"
#include <spdlog/spdlog.h>

#include <fcntl.h>
#include <mio/mio.hpp>
#include <sys/stat.h>
#include <sys/types.h>

#include "dbg.h"

namespace cubao
{
struct GeobufPlus
{
    GeobufPlus() = default;
    int num_features;
    FlatGeobuf::PackedRTree rtree;
    std::vector<int> offsets;
    mio::shared_ummap_source mmap;
    mapbox::geobuf::Decoder decoder;

    bool mmap_init(const std::string &index_path,
                   const std::string &geobuf_path)
    {
        auto bytes = mapbox::geobuf::load_bytes(index_path);
        int cursor = 10;
        if (bytes.substr(0, cursor) != "GeobufIdx0") {
            spdlog::error("invalid geobuf index file: {}", index_path);
            return false;
        }
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.data());
        num_features = *reinterpret_cast<const int *>(data + cursor);
        cursor += sizeof(num_features);
        spdlog::info("#features: {}", num_features);

        FlatGeobuf::NodeItem extent;
        memcpy((void *)&extent.minX, data + cursor, sizeof(extent));
        cursor += sizeof(extent);
        spdlog::info("extent: {},{},{},{}", extent.minX, extent.minY,
                     extent.maxX, extent.maxY);

        int num_items{0};
        num_items = *reinterpret_cast<const int *>(data + cursor);
        cursor += sizeof(num_items);
        spdlog::info("num_items: {}", num_items);

        int num_nodes{0};
        num_nodes = *reinterpret_cast<const int *>(data + cursor);
        cursor += sizeof(num_nodes);
        spdlog::info("num_nodes: {}", num_nodes);

        int node_size{0};
        node_size = *reinterpret_cast<const int *>(data + cursor);
        cursor += sizeof(node_size);
        spdlog::info("node_size: {}", node_size);

        int tree_size{0};
        tree_size = *reinterpret_cast<const int *>(data + cursor);
        cursor += sizeof(tree_size);
        spdlog::info("tree_size: {}", tree_size);

        rtree = FlatGeobuf::PackedRTree(data + cursor, num_items, node_size);
        if (rtree.getNumNodes() != num_nodes || rtree.getExtent() != extent) {
            spdlog::error("invalid rtree, #nodes:{} != {} (expected)",
                          rtree.getNumNodes(), num_nodes);
            return false;
        }
        cursor += tree_size;

        int padding{0};
        padding = *reinterpret_cast<const int *>(data + cursor);
        cursor += sizeof(padding);
        if (padding != 930604) {
            spdlog::error("invalid padding: {} != 930604 (geobuf)", padding);
            return false;
        }

        int num_offsets{0};
        num_offsets = *reinterpret_cast<const int *>(data + cursor);
        cursor += sizeof(num_offsets);
        spdlog::info("num_offsets: {}", num_offsets);

        offsets.resize(num_offsets);
        memcpy(reinterpret_cast<void *>(offsets.data()), data + cursor,
               sizeof(offsets[0]) * num_offsets);
        cursor += sizeof(offsets[0]) * num_offsets;
        spdlog::info("offsets: [{}, ..., {}]", offsets.front(), offsets.back());

        padding = *reinterpret_cast<const int *>(data + cursor);
        cursor += sizeof(padding);
        if (padding != 930604) {
            spdlog::error("invalid padding: {} != 930604 (geobuf)", padding);
            return false;
        }

        spdlog::info("decoding geobuf...");
        mmap = std::make_shared<mio::ummap_source>(geobuf_path);
        decoder.decode_header(mmap.data(), offsets[0]);
        spdlog::info("decoded geobuf header, #keys={}, dim={}, precision: {}",
                     decoder.__keys().size(), decoder.__dim(),
                     decoder.precision());

        return true;
    }

    void init_index(const std::string &index_bytes) {}

    mapbox::geojson::feature_collection decode(const std::string &bytes) const
    {
        return {};
    }

    mapbox::geojson::feature decode_feature(int index)
    {
        return decode_feature(mmap.data() + offsets[index],
                              offsets[index + 1] - offsets[index]);
    }

    mapbox::geojson::feature decode_feature(const uint8_t *data, size_t size)
    {
        return decoder.decode_feature(data, size);
    }

    mapbox::geojson::feature_collection
    decode_feature(const uint8_t *data,
                   const std::vector<std::array<int, 2>> &index)
    {
        mapbox::geojson::feature_collection fc;
        fc.reserve(index.size());
        for (auto &pair : index) {
            fc.push_back(decode_feature(data + pair[0], pair[1]));
        }
        return fc;
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
            spdlog::error("failed to open {} for writing", output_index_path);
            return false;
        }
        // magic
        fwrite("GeobufIdx0", 10, 1, fp);
        int num_features = fc.size();
        // #features
        fwrite(&num_features, sizeof(num_features), 1, fp);

        auto rtree = planet.packed_rtree();
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

        auto encoder =
            mapbox::geobuf::Encoder(std::pow(10, precision), only_xy, round_z);
        auto bytes = encoder.encode(geojson);
        std::vector<int> offsets = encoder.__offsets();
        int num_offsets = offsets.size();
        fwrite(&num_offsets, sizeof(num_offsets), 1, fp);
        fwrite(offsets.data(), sizeof(offsets[0]), offsets.size(), fp);
        fwrite(&padding, sizeof(padding), 1, fp);
        fclose(fp);
        spdlog::info("wrote index to {}", output_index_path);

        if (mapbox::geobuf::dump_bytes(output_geobuf_path, bytes)) {
            spdlog::info("wrote geobuf to {}", output_geobuf_path);
            return true;
        } else {
            spdlog::error("failed to write to {}", output_geobuf_path);
            return false;
        }
    }
};
} // namespace cubao

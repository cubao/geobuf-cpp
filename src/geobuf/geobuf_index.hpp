#pragma once
#include "geobuf.hpp"
#include "planet.hpp"

#include <spdlog/spdlog.h>
// fix exposed macro 'GetObject' from wingdi.h (included by spdlog.h) under
// windows, see https://github.com/Tencent/rapidjson/issues/1448
#ifdef GetObject
#undef GetObject
#endif

#include <fcntl.h>
#include <mio/mio.hpp>
#include <sys/stat.h>
#include <sys/types.h>

#include <iomanip>
#include <sstream>

namespace cubao
{
inline std::string to_hex(const std::string &s, bool upper_case = true)
{
    std::ostringstream ret;

    for (std::string::size_type i = 0; i < s.length(); ++i) {
        int z = s[i] & 0xff;
        ret << std::hex << std::setfill('0') << std::setw(2)
            << (upper_case ? std::uppercase : std::nouppercase) << z;
    }

    return ret.str();
}

struct GeobufIndex
{
    GeobufIndex() = default;
    int num_features = -1;
    std::vector<int> offsets;
    FlatGeobuf::PackedRTree rtree;
    mio::shared_ummap_source mmap;
    mapbox::geobuf::Decoder decoder;

    bool init(const std::string &bytes)
    {
        int cursor = 10;
        if (bytes.substr(0, cursor) != "GeobufIdx0") {
            spdlog::error("invalid geobuf index");
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
        return true;
    }

    bool mmap_init(const std::string &index_path,
                   const std::string &geobuf_path)
    {
        spdlog::info("initiating geobuf index from {}", index_path);
        auto bytes = mapbox::geobuf::load_bytes(index_path);
        if (!init(bytes)) {
            return false;
        }
        return mmap_init(geobuf_path);
    }
    bool mmap_init(const std::string &geobuf_path)
    {
        if (num_features < 0 || offsets.empty()) {
            throw std::invalid_argument("should init index first!!!");
        }
        spdlog::info("lazy decoding geobuf with mmap");
        mmap = std::make_shared<mio::ummap_source>(geobuf_path);
        decoder.decode_header(mmap.data(), offsets[0]);
        spdlog::info("decoded geobuf header, #keys={}, dim={}, precision: {}",
                     decoder.__keys().size(), decoder.__dim(),
                     decoder.precision());
        return true;
    }

    std::optional<std::string> mmap_bytes(size_t offset, size_t length) const
    {
        if (mmap.is_open() && offset + length < mmap.size()) {
            return std::string((const char *)mmap.data() + offset, length);
        }
        return {};
    }

    std::optional<mapbox::geojson::feature>
    decode_feature(const uint8_t *data, size_t size, bool only_geometry = false,
                   bool only_properties = false)
    {
        return decoder.decode_feature(data, size, only_geometry,
                                      only_properties);
    }
    std::optional<mapbox::geojson::feature>
    decode_feature(const std::string &bytes, bool only_geometry,
                   bool only_properties)
    {
        return decode_feature((const uint8_t *)bytes.data(), bytes.size(),
                              only_geometry, only_properties);
    }
    std::optional<mapbox::geojson::feature>
    decode_feature(int index, bool only_geometry = false,
                   bool only_properties = false)
    {
        bool valid_index =
            0 <= index && index < num_features && index + 1 < offsets.size();
        if (!valid_index) {
            return {};
        }
        if (!mmap.is_open()) {
            return {};
        }
        try {
            int cursor = offsets[index];
            int length = offsets[index + 1] - cursor;
            return decode_feature(mmap.data() + cursor, length, only_geometry,
                                  only_properties);
        } catch (...) {
        }
        return {};
    }

    mapbox::geojson::feature_collection
    decode_features(const uint8_t *data,
                    const std::vector<std::array<int, 2>> &index,
                    bool only_geometry = false, bool only_properties = false)
    {
        auto fc = mapbox::geojson::feature_collection{};
        fc.reserve(index.size());
        for (auto &pair : index) {
            auto f = decode_feature(data + pair[0], pair[1], only_geometry,
                                    only_properties);
            if (f) {
                fc.push_back(std::move(*f));
            }
        }
        return fc;
    }
    mapbox::geojson::feature_collection
    decode_features(const std::vector<int> &index, bool only_geometry = false,
                    bool only_properties = false)
    {
        auto fc = mapbox::geojson::feature_collection{};
        fc.reserve(index.size());
        for (auto &idx : index) {
            auto f = decode_feature(idx, only_geometry, only_properties);
            if (f) {
                fc.push_back(std::move(*f));
            }
        }
        return fc;
    }

    mapbox::feature::value decode_non_features(const uint8_t *data, size_t size)
    {
        return decoder.decode_non_features(data, size);
    }
    mapbox::feature::value decode_non_features(const std::string &bytes)
    {
        return decode_non_features((const uint8_t *)bytes.data(), bytes.size());
    }
    mapbox::feature::value decode_non_features()
    {
        if (num_features <= 0 || offsets.size() < num_features + 2) {
            return {};
        }
        try {
            int cursor = offsets[num_features];
            int length = offsets[num_features + 1] - cursor;
            if (length <= 0 || !mmap.is_open()) {
                return {};
            }
            return decode_non_features(mmap.data() + cursor, length);
        } catch (...) {
        }
        return {};
    }

    static bool indexing(const std::string &input_geobuf_path,
                         const std::string &output_index_path)
    {
        spdlog::info("indexing {} ...", input_geobuf_path);
        auto decoder = mapbox::geobuf::Decoder();
        auto geojson = decoder.decode_file(input_geobuf_path);
        if (!geojson.is<mapbox::geojson::feature_collection>()) {
            throw std::invalid_argument(
                "invalid GeoJSON type, should be FeatureCollection");
        }
        auto &fc = geojson.get<mapbox::geojson::feature_collection>();

        FILE *fp = fopen(output_index_path.c_str(), "wb");
        if (!fp) {
            spdlog::error("failed to open {} for writing", output_index_path);
            return {};
        }
        auto planet = Planet(fc);
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

        std::vector<int> offsets = decoder.__offsets();
        int num_offsets = offsets.size();
        fwrite(&num_offsets, sizeof(num_offsets), 1, fp);
        fwrite(offsets.data(), sizeof(offsets[0]), offsets.size(), fp);
        fwrite(&padding, sizeof(padding), 1, fp);
        fclose(fp);
        spdlog::info("wrote index to {}", output_index_path);
        return true;
    }
};
} // namespace cubao

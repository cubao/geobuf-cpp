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
inline std::optional<std::string>
value2string(const mapbox::geojson::value &value)
{
    return value.match(
        [](uint64_t v) { return std::to_string(v); },
        [](int64_t v) { return std::to_string(v); },
        [](const std::string &v) { return v; },
        [](const auto &) -> std::optional<std::string> { return {}; });
}

inline std::optional<std::string>
fn_feature_id_default(const mapbox::geojson::feature &feature)
{
    if (!feature.id.is<mapbox::feature::null_value_t>()) {
        return feature.id.match(
            [](uint64_t v) { return std::to_string(v); },
            [](int64_t v) { return std::to_string(v); },
            [](const std::string &v) { return v; },
            [](const auto &) -> std::optional<std::string> { return {}; });
    }
    for (const auto &key : {"id", "feature_id", "fid"}) {
        auto itr = feature.properties.find(key);
        if (itr == feature.properties.end()) {
            continue;
        }
        auto id = value2string(itr->second);
        if (id) {
            return id;
        }
    }
    return {};
}

struct GeobufIndex
{
    GeobufIndex() = default;

    uint32_t header_size = 0;
    uint32_t num_features = 0;
    std::vector<uint64_t> offsets;
    std::optional<std::unordered_map<std::string, uint32_t>> ids;
    std::optional<FlatGeobuf::PackedRTree> packed_rtree;

    using Encoder = mapbox::geobuf::Encoder;
    using Decoder = mapbox::geobuf::Decoder;
    Decoder decoder;
    mio::shared_ummap_source mmap;

    bool init(const uint8_t *data, size_t size)
    {
        auto pbf = protozero::pbf_reader{(const char *)data, size};
        std::vector<std::string> fids;
        std::vector<uint32_t> idxs;
        while (pbf.next()) {
            const auto tag = pbf.tag();
            if (tag == 1) {
                header_size = pbf.get_uint32();
                spdlog::info("header_size: {}", header_size);
            } else if (tag == 2) {
                num_features = pbf.get_uint32();
                spdlog::info("num_features: {}", num_features);
            } else if (tag == 3) {
                auto iter = pbf.get_packed_uint64();
                offsets = std::vector<uint64_t>(iter.begin(), iter.end());
                if (offsets.size() == num_features + 2u) {
                    spdlog::info("#offsets: {}, values: [{},{}, ..., {}, {}]",
                                 offsets.size(), offsets[0], offsets[1],
                                 offsets[num_features],
                                 offsets[num_features + 1]);
                } else {
                    spdlog::error("#offsets:{} != 2 + num_features:{}",
                                  offsets.size(), num_features);
                }
            } else if (tag == 4) {
                fids.push_back(pbf.get_string());
            } else if (tag == 5) {
                auto iter = pbf.get_packed_uint32();
                idxs = std::vector<uint32_t>(iter.begin(), iter.end());
            } else if (tag == 8) {
                FlatGeobuf::NodeItem extent;
                uint32_t num_items{0};
                uint32_t num_nodes{0};
                uint32_t node_size{0};
                std::string rtree_bytes;
                protozero::pbf_reader pbf_rtree = pbf.get_message();
                while (pbf_rtree.next()) {
                    const auto tag = pbf_rtree.tag();
                    if (tag == 1) {
                        extent.minX = pbf_rtree.get_double();
                    } else if (tag == 2) {
                        extent.minY = pbf_rtree.get_double();
                    } else if (tag == 3) {
                        extent.maxX = pbf_rtree.get_double();
                    } else if (tag == 4) {
                        extent.maxY = pbf_rtree.get_double();
                    } else if (tag == 5) {
                        num_items = pbf_rtree.get_uint32();
                    } else if (tag == 6) {
                        num_nodes = pbf_rtree.get_uint32();
                    } else if (tag == 7) {
                        node_size = pbf_rtree.get_uint32();
                    } else if (tag == 8) {
                        rtree_bytes = pbf_rtree.get_bytes();
                    } else {
                        pbf_rtree.skip();
                    }
                }
                spdlog::info("PackedRTree num_items={}, num_nodes={}, "
                             "node_size={}, bbox=[{},{},{},{}], #bytes={}",
                             num_items, num_nodes, node_size, extent.minX,
                             extent.minY, extent.maxX, extent.maxY,
                             rtree_bytes.size());
                if (num_items > 0 && num_nodes > 0 && node_size > 0 &&
                    extent.width() >= 0 && extent.height() >= 0 &&
                    rtree_bytes.size() ==
                        num_nodes * sizeof(FlatGeobuf::NodeItem)) {
                    packed_rtree = FlatGeobuf::PackedRTree(
                        (const uint8_t *)rtree_bytes.data(), num_items,
                        node_size);
                    if (packed_rtree->getExtent() != extent) {
                        extent = packed_rtree->getExtent();
                        spdlog::error(
                            "extent mismatch, from RTree: {},{},{},{}",
                            extent.minX, extent.minY, extent.maxX, extent.maxY);
                    }
                } else {
                    spdlog::error("invalid PackedRTree");
                }
            } else {
                pbf.skip();
            }
        }
        if (fids.size() != idxs.size()) {
            spdlog::error("bad feature ids, #fids:{} != #idxs:{}", fids.size(),
                          idxs.size());
        } else {
            ids = std::unordered_map<std::string, uint32_t>{};
            for (size_t i = 0, N = idxs.size(); i < N; ++i) {
                ids->emplace(fids[i], idxs[i]);
            }
            spdlog::info("#feature_ids: {}", ids->size());
        }
        return true;
    }

    bool init(const std::string &bytes)
    {
        return init((const uint8_t *)bytes.data(), bytes.size());
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
        if (offsets.size() != num_features + 2u || header_size == 0) {
            throw std::invalid_argument("should init index first!!!");
        }
        spdlog::info(
            "decoding geobuf with mmap, only parse {} bytes header for now",
            header_size);
        mmap = std::make_shared<mio::ummap_source>(geobuf_path);
        decoder.decode_header(mmap.data(), header_size);
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
    decode_feature(uint32_t index, bool only_geometry = false,
                   bool only_properties = false)
    {
        bool valid_index = index < num_features && index + 1 < offsets.size();
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

    std::optional<mapbox::geojson::feature>
    decode_feature_of_id(const std::string &id, bool only_geometry = false,
                         bool only_properties = false)
    {
        if (!ids) {
            return {};
        }
        auto itr = ids->find(id);
        if (itr == ids->end()) {
            return {};
        }
        return decode_feature(itr->second, only_geometry, only_properties);
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
        if (num_features <= 0 || offsets.size() < num_features + 2u) {
            return {};
        }
        try {
            size_t begin = offsets[num_features];
            size_t end = offsets[num_features + 1u];
            if (begin >= end || !mmap.is_open()) {
                return {};
            }
            return decode_non_features(mmap.data() + begin, end - begin);
        } catch (...) {
        }
        return {};
    }

    std::set<uint32_t> query(const Eigen::Vector2d &min,
                             const Eigen::Vector2d &max) const
    {
        if (!packed_rtree) {
            return {};
        }
        std::set<uint32_t> hits;
        for (auto h : packed_rtree->search(min[0], min[1], max[0], max[1])) {
            hits.insert(h.offset);
        }
        return hits;
    }

    static bool indexing(const std::string &input_geobuf_path,
                         const std::string &output_index_path,
                         const std::optional<std::string> feature_id = "@",
                         const std::optional<std::string> packed_rtree = "@")
    {
        spdlog::info("indexing {} ...", input_geobuf_path);
        auto decoder = mapbox::geobuf::Decoder();
        auto geojson = decoder.decode_file(input_geobuf_path);
        if (!geojson.is<mapbox::geojson::feature_collection>()) {
            throw std::invalid_argument(
                "invalid GeoJSON type, should be FeatureCollection");
        }
        auto &fc = geojson.get<mapbox::geojson::feature_collection>();
        auto header_size = decoder.__header_size();
        auto offsets = decoder.__offsets();
        spdlog::info("#features: {}", fc.size());
        spdlog::info("header_size: {}", header_size);
        if (offsets.size() == fc.size() + 2u) {
            spdlog::info("#offsets: {}, values: [{},{}, ..., {}, {}]",
                         offsets.size(), offsets[0], offsets[1],
                         offsets[fc.size()], offsets[fc.size() + 1]);
        } else {
            spdlog::error("#offsets:{} != 2 + num_features:{}", offsets.size(),
                          fc.size());
        }

        std::string data;
        Encoder::Pbf pbf{data};
        pbf.add_uint32(1, header_size);
        pbf.add_uint32(2, fc.size());
        pbf.add_packed_uint64(3, offsets.begin(), offsets.end());

        if (feature_id) {
            std::map<std::string, uint32_t> ids;
            for (size_t i = 0; i < fc.size(); ++i) {
                auto id = fn_feature_id_default(fc[i]);
                if (id) {
                    ids.emplace(*id, static_cast<uint32_t>(i));
                }
            }
            if (!ids.empty()) {
                spdlog::info("#feature_ids: {}", ids.size());
                std::vector<uint32_t> idxs;
                idxs.reserve(ids.size());
                for (auto &pair : ids) {
                    pbf.add_string(4, pair.first);
                    idxs.push_back(pair.second);
                }
                pbf.add_packed_uint32(5, idxs.begin(), idxs.end());
            }
        }

        if (packed_rtree) {
            auto planet = Planet(fc);
            if (*packed_rtree == "per_line_segment") {
                planet.build(true);
            }
            auto rtree = planet.packed_rtree();
            auto extent = rtree.getExtent();
            spdlog::info("PackedRTree num_items={}, num_nodes={}, "
                         "node_size={}, bbox=[{},{},{},{}], #bytes={}",
                         rtree.getNumItems(), rtree.getNumNodes(),
                         rtree.getNodeSize(), extent.minX, extent.minY,
                         extent.maxX, extent.maxY, rtree.size());
            if (extent.width() >= 0 && extent.height() >= 0) {
                protozero::pbf_writer pbf_rtree{pbf, 8};
                pbf_rtree.add_double(1, extent.minX);
                pbf_rtree.add_double(2, extent.minY);
                pbf_rtree.add_double(3, extent.maxX);
                pbf_rtree.add_double(4, extent.maxY);
                pbf_rtree.add_uint32(5, rtree.getNumItems());
                pbf_rtree.add_uint32(6, rtree.getNumNodes());
                pbf_rtree.add_uint32(7, rtree.getNodeSize());
                rtree.streamWrite([&](const uint8_t *data, size_t size) {
                    pbf_rtree.add_bytes(8, (const char *)data, size);
                });
            } else {
                spdlog::error("invalid PackedRTree");
            }
        }
        return mapbox::geobuf::dump_bytes(output_index_path, data);
    }
};
} // namespace cubao

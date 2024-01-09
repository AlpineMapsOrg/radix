/*****************************************************************************
 * Alpine Terrain Builder
 * Copyright (C) 2022 Adam Celarek
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#pragma once

#include <cstddef>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include "hasher.h"
#include "tile.h"

class TileHeights {
public:
    //    using KeyType = std::tuple<unsigned, unsigned, unsigned>;
    using KeyType = uint64_t;
    using ValueType = std::pair<float, float>;

private:
    //    std::unordered_map<KeyType, ValueType, hasher::for_tuple<unsigned, unsigned, unsigned>> m_data;
    std::unordered_map<KeyType, ValueType> m_data;
    unsigned m_max_zoom_level = 0;

public:
    TileHeights();
    void emplace(const tile::Id& tile_id, const std::pair<float, float>& min_max);
    [[nodiscard]] ValueType query(tile::Id tile_id) const;
    void write_to(const std::filesystem::path& path) const;
    [[nodiscard]] static TileHeights read_from(const std::filesystem::path& path);
    [[nodiscard]] std::vector<std::byte> serialise() const;

    template <typename VectorOfBytes>
    [[nodiscard]] static TileHeights deserialise(const VectorOfBytes& bytes)
    {
        using vector_size_t = decltype(bytes.size());
        auto size = uint64_t(-1);
        std::copy_n(reinterpret_cast<const std::byte*>(bytes.data()), sizeof(size), reinterpret_cast<std::byte*>(&size));
        if (size > uint64_t(1024 * 1024 * 50))
            return {};

        static_assert(sizeof(std::pair<glm::uvec3, ValueType>) == 5lu * 4u);
        std::vector<std::pair<glm::uvec3, ValueType>> vector_data;
        const auto data_size_in_bytes = size * sizeof(decltype(vector_data.front()));

        if (size_t(bytes.size()) != sizeof(size) + data_size_in_bytes)
            return {};

        vector_data.resize(vector_size_t(size));
        std::copy_n(reinterpret_cast<const std::byte*>(bytes.data()) + sizeof(size), data_size_in_bytes, reinterpret_cast<std::byte*>(vector_data.data()));

        TileHeights new_heights;
        for (const auto& entry : vector_data) {
            const auto tile_id = tile::Id { entry.first.x, { entry.first.y, entry.first.z } };
            new_heights.emplace(tile_id, entry.second);
            //            new_heights.m_max_zoom_level = std::max(new_heights.m_max_zoom_level, entry.first.x);
            //            new_heights.m_data[std::make_tuple(entry.first.x, entry.first.y, entry.first.z)] = entry.second;
        }
        return new_heights;
    }
};

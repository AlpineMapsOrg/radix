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
    using KeyType = std::tuple<unsigned, unsigned, unsigned>;
    using ValueType = std::pair<float, float>;
    std::unordered_map<KeyType, ValueType, hasher::for_tuple<unsigned, unsigned, unsigned>> m_data;
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
        uint64_t size = -1;
        std::copy_n(reinterpret_cast<const std::byte*>(bytes.data()), sizeof(size), reinterpret_cast<std::byte*>(&size));
        if (size > uint64_t(1024 * 1024 * 50))
            return {};

        static_assert(sizeof(std::pair<KeyType, ValueType>) == 5 * 4);
        std::vector<std::pair<glm::uvec3, ValueType>> vector_data;
        const auto data_size_in_bytes = size * sizeof(decltype(vector_data.front()));

        if (bytes.size() != sizeof(size) + data_size_in_bytes)
            return {};

        vector_data.resize(size);
        std::copy_n(reinterpret_cast<const std::byte*>(bytes.data()) + sizeof(size), data_size_in_bytes, reinterpret_cast<std::byte*>(vector_data.data()));

        TileHeights new_heights;
        for (const auto& entry : vector_data) {
            new_heights.m_data[std::make_tuple(entry.first.x, entry.first.y, entry.first.z)] = entry.second;
        }
        return new_heights;
    }
};


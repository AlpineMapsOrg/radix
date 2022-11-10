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

#include <fstream>
#include <iterator>

#include "TileHeights.h"

namespace {
auto key(const tile::Id& tile_id)
{
    const auto id = tile_id.to(tile::Scheme::Tms);
    return std::make_tuple(id.zoom_level, id.coords.x, id.coords.y);
}
}

TileHeights::TileHeights() = default;

void TileHeights::emplace(const tile::Id& tile_id, const std::pair<float, float>& min_max)
{
    m_data[key(tile_id)] = min_max;
}

TileHeights::ValueType TileHeights::query(tile::Id tile_id) const
{
    auto iter = m_data.find(key(tile_id));
    while (iter == m_data.end()) {
        tile_id = tile_id.parent();
        iter = m_data.find(key(tile_id));
        if (tile_id.zoom_level == unsigned(-1)) {
            assert(false);
            return { 0, 9000 }; // mount everest is a bit under 9km, but there should always be a root tile.
        }
    }
    return iter->second;
}

void TileHeights::write_to(const std::filesystem::path& path) const
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::binary);

    const auto bytes = serialise();
    static_assert(sizeof(decltype(bytes.front())) == sizeof(char));
    file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

TileHeights TileHeights::read_from(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();

    if (size > std::streamsize(1024 * 1024 * 200))
        return {};

    file.seekg(0, std::ios::beg);

    std::vector<std::byte> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        return {};

    return deserialise(buffer);
}

std::vector<std::byte> TileHeights::serialise() const
{
    std::vector<std::pair<KeyType, ValueType>> vector_data;
    vector_data.reserve(m_data.size());
    std::copy(m_data.cbegin(), m_data.cend(), std::back_inserter(vector_data));
    const uint64_t size = vector_data.size();

    const auto data_size_in_bytes = size * sizeof(decltype(vector_data.front()));

    std::vector<std::byte> bytes;
    bytes.reserve(sizeof(size) + data_size_in_bytes);
    std::copy_n(reinterpret_cast<const std::byte*>(&size), sizeof(size), std::back_inserter(bytes));
    std::copy_n(reinterpret_cast<std::byte*>(vector_data.data()), data_size_in_bytes, std::back_inserter(bytes));
    return bytes;
}

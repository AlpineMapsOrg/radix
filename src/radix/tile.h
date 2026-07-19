/*****************************************************************************
 * Alpine Terrain Builder
 * Copyright (C) 2022 alpinemaps.org
 * Copyright (C) 2022 Adam Celarek <family name at cg tuwien ac at>
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

#include "geometry.h"
#include "hasher.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/vector_relational.hpp>
#include <iostream>
#include <tuple>
#include <unordered_set>

namespace radix::tile {
using SrsBounds = geometry::Aabb2<double>;
using SrsAndHeightBounds = geometry::Aabb<3, double>;

enum class Border {
    Yes = 1,
    No = 0
};

enum class QuadPosition : unsigned { TopLeft = 0, TopRight, BottomLeft, BottomRight };

/// A Google/Mapbox/XYZ tile identifier. The origin is north-west, x grows east, and y grows south.
struct Id {
    unsigned zoom_level = unsigned(-1);
    glm::uvec2 coords = {};

    [[nodiscard]] Id parent() const { return { zoom_level - 1, coords / 2u }; }
    /// returns the children in an unspecified order
    [[nodiscard]] std::array<Id, 4> children() const
    {
        // clang-format off
        return {
            Id { zoom_level + 1, coords * 2u + glm::uvec2(0, 0) },
            Id { zoom_level + 1, coords * 2u + glm::uvec2(1, 0) },
            Id { zoom_level + 1, coords * 2u + glm::uvec2(0, 1) },
            Id { zoom_level + 1, coords * 2u + glm::uvec2(1, 1) }
        };
        // clang-format on
    }
    bool operator==(const Id& other) const = default;
    bool operator<(const Id& other) const
    {
        return std::tie(zoom_level, coords.x, coords.y) < std::tie(other.zoom_level, other.coords.x, other.coords.y);
    }
    operator std::tuple<unsigned, unsigned, unsigned>() const
    {
        return std::make_tuple(zoom_level, coords.x, coords.y);
    }

    using Hasher = typename hasher::for_tuple<unsigned, unsigned, unsigned>;
};

/// gives the index in the children array of the parent, i.e., in the order top left, top right, bottom left, bottom right
inline QuadPosition quad_position(const Id& id)
{
    const auto x_comp = id.coords.x % 2;

    const auto y_comp = id.coords.y % 2;

    return QuadPosition(2 * y_comp + x_comp);
}

using IdSet = std::unordered_set<tile::Id, tile::Id::Hasher>;
template <typename T> using IdMap = std::unordered_map<tile::Id, T, tile::Id::Hasher>;

inline std::string to_string(const Id& value)
{
    return std::to_string(value.zoom_level) + "/" + std::to_string(value.coords.x) + "/" + std::to_string(value.coords.y);
}

// helper for catch2
inline std::ostream& operator<<(std::ostream& os, const Id& value)
{
    os << "{"
       << value.zoom_level << ", {" << value.coords.x << ", " << value.coords.y << "}"
       << "}";
    return os;
}

struct Descriptor {
    // used to generate file name
    tile::Id id;

    // srsBounds are the bounds of the tile including the border pixel.
    SrsBounds srsBounds;
    int srs_epsg = -1;

    // some tiling schemes require a border (e.g. cesium heightmap https://github.com/CesiumGS/cesium/wiki/heightmap-1%2E0).
    // grid bounds does not contain that border (e.g. 64 width)
    // tile bounds contains that border (e.g. 65 width)
    unsigned gridSize = unsigned(-1);
    unsigned tileSize = unsigned(-1);
};

} // namespace radix::tile

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

#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include <tuple>

namespace tile {
/// A representation of an extent
template <unsigned n_dims, class T>
class Aabb {
    using Vec = glm::vec<n_dims, T>;
public:
    Vec min = {};
    Vec max = {};

    bool operator==(const Aabb<n_dims, T>& other) const = default;

    [[nodiscard]] T width() const { return max.x - min.x; }
    [[nodiscard]] T height() const { return max.y - min.y; }
};
using SrsBounds = Aabb<2, double>;

template <typename T>
bool intersect(const Aabb<2, T>& a, const Aabb<2, T>& b)
{
    // http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
    return a.min.x <= b.max.x && b.min.x <= a.max.x && a.min.y <= b.max.y && b.min.y <= a.max.y;
}

template <typename T>
bool intersect(const Aabb<3, T>& a, const Aabb<3, T>& b)
{
    // http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
    return a.min.x <= b.max.x
        && b.min.x <= a.max.x
        && a.min.y <= b.max.y
        && b.min.y <= a.max.y
        && a.min.z <= b.max.z
        && b.min.z <= a.max.z;
}

template <typename T>
Aabb<2, T> intersection(const Aabb<2, T>& a, const Aabb<2, T>& b)
{
    Aabb<2, T> r;
    r.min.x = std::max(a.min.x, b.min.x);
    r.min.y = std::max(a.min.y, b.min.y);
    r.max.x = std::min(a.max.x, b.max.x);
    r.max.y = std::min(a.max.y, b.max.y);
    return r;
}

template <typename T>
Aabb<3, T> intersection(const Aabb<3, T>& a, const Aabb<3, T>& b)
{
    Aabb<3, T> r;
    r.min.x = std::max(a.min.x, b.min.x);
    r.min.y = std::max(a.min.y, b.min.y);
    r.min.z = std::max(a.min.z, b.min.z);

    r.max.x = std::min(a.max.x, b.max.x);
    r.max.y = std::min(a.max.y, b.max.y);
    r.max.z = std::min(a.max.z, b.max.z);
    return r;
}

enum class Border {
    Yes = 1,
    No = 0
};

// The difference between TMS and slippyMap is whether y starts counting from the bottom (south) or top (north).
// https://www.maptiler.com/google-maps-coordinates-tile-bounds-projection/#1/-16.88/79.02
//
enum class Scheme {
    Tms, // southern most tile is y = 0
    SlippyMap // aka Google, XYZ, webmap tiles; northern most tile is y = 0
};

struct Id {
    unsigned zoom_level = unsigned(-1);
    glm::uvec2 coords;
    Scheme scheme = Scheme::Tms;

    [[nodiscard]] Id to(Scheme new_scheme) const
    {
        if (scheme == new_scheme)
            return *this;

        const auto n_y_tiles = (1u << zoom_level);
        const auto coord_y = n_y_tiles - coords.y - 1;
        return { zoom_level, { coords.x, coord_y }, new_scheme };
    }
    [[nodiscard]] Id parent() const { return { zoom_level - 1, coords / 2u, scheme }; }
    [[nodiscard]] std::array<Id, 4> children() const
    {
        return {
            Id { zoom_level + 1, coords * 2u + glm::uvec2(0, scheme != Scheme::Tms), scheme },
            Id { zoom_level + 1, coords * 2u + glm::uvec2(1, scheme != Scheme::Tms), scheme },
            Id { zoom_level + 1, coords * 2u + glm::uvec2(0, scheme == Scheme::Tms), scheme },
            Id { zoom_level + 1, coords * 2u + glm::uvec2(1, scheme == Scheme::Tms), scheme }
        };
    }
    bool operator==(const Id& other) const { return other.coords == coords && other.scheme == scheme && other.zoom_level == zoom_level; };
    bool operator<(const Id& other) const { return std::tie(zoom_level, coords.x, coords.y, scheme) < std::tie(other.zoom_level, other.coords.x, other.coords.y, other.scheme); };
};

struct Descriptor {
    // used to generate file name
    tile::Id id;

    // srsBounds are the bounds of the tile including the border pixel.
    SrsBounds srsBounds;
    int srs_epsg;

    // some tiling schemes require a border (e.g. cesium heightmap https://github.com/CesiumGS/cesium/wiki/heightmap-1%2E0).
    // grid bounds does not contain that border (e.g. 64 width)
    // tile bounds contains that border (e.g. 65 width)
    unsigned gridSize;
    unsigned tileSize;
};
}

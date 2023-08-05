/*****************************************************************************
 * Alpine Terrain Renderer
 * Copyright (C) 2023 Adam Celarek
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

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

namespace sherpa::height_encoding {

constexpr float min_height = 0.f;
constexpr float max_height = 8191.875f;
static_assert(max_height > min_height);

inline glm::u8vec3 convert(float height)
{
    height -= min_height;
    constexpr float scaling_factor = 65535.f / (max_height - min_height);

    uint scaled = uint(height * scaling_factor + 0.5f);
    const auto r = (scaled >> 8u) & 255u;
    const auto g = scaled & 255u;
    return {glm::u8(r), glm::u8(g), 0};
}

inline float convert(glm::u8vec3 v)
{
    constexpr float scaling_factor = (max_height - min_height) / 65535.f;
    return float(v.x << 8 | v.y) * scaling_factor + min_height;
}

} // namespace sherpa::height_encoding

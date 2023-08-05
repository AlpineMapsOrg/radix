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

inline glm::u8vec3 convert(float height)
{
    const auto r = std::clamp(int(height / 32.0f), 0, 255);
    const auto g = std::clamp(int(std::fmod(height, 32.0f) * 8), 0, 255);

    return {glm::u8(r), glm::u8(g), 0};
}

inline float convert(glm::u8vec3 v)
{
    return float(v.x << 8 | v.y) * 8191.875f / 65535.f;
}

} // namespace sherpa::height_encoding

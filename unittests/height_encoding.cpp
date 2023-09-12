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

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <radix/height_encoding.h>

TEST_CASE("radix/height encoding float to rgb")
{
    using namespace sherpa::height_encoding;
    const auto one_red = 32.0f;
    const auto one_green = 32.000000001f / 256;
    const auto eps = 0.000000001f;

    CHECK(to_rgb(0) == glm::u8vec3(0, 0, 0));

    CHECK(to_rgb(one_red + eps) == glm::u8vec3(1, 0, 0));
    CHECK(to_rgb(one_green + eps) == glm::u8vec3(0, 1, 0));

    CHECK(to_rgb(42 * one_red + eps) == glm::u8vec3(42, 0, 0));
    CHECK(to_rgb(200 * one_green + eps) == glm::u8vec3(0, 200, 0));

    CHECK(to_rgb(255 * one_red + eps) == glm::u8vec3(255, 0, 0));
    CHECK(to_rgb(253 * one_green + eps) == glm::u8vec3(0, 253, 0));
    CHECK(to_rgb(254 * one_green + eps) == glm::u8vec3(0, 254, 0));
    CHECK(to_rgb(255 * one_green + eps) == glm::u8vec3(0, 255, 0));
    CHECK(to_rgb(256 * one_green + eps) == glm::u8vec3(1, 0, 0));
    CHECK(to_rgb(257 * one_green + eps) == glm::u8vec3(1, 1, 0));
    CHECK(to_rgb(258 * one_green + eps) == glm::u8vec3(1, 2, 0));

    CHECK(to_rgb(240 * one_red + 195 * one_green + eps) == glm::u8vec3(240, 195, 0));
    CHECK(to_rgb(64 * one_red + 255 * one_green + eps) == glm::u8vec3(64, 255, 0));
    CHECK(to_rgb(255 * one_red + 128 * one_green + eps) == glm::u8vec3(255, 128, 0));
    CHECK(to_rgb(255 * one_red + 255 * one_green + eps) == glm::u8vec3(255, 255, 0));

    CHECK(to_rgb(123 * one_red + 250 * one_green + eps) == glm::u8vec3(123, 250, 0));
    CHECK(to_rgb(140 * one_red + 255 * one_green + eps) == glm::u8vec3(140, 255, 0));
    CHECK(to_rgb(141 * one_red + 0 * one_green + eps) == glm::u8vec3(141, 0, 0));
    CHECK(to_rgb(141 * one_red + 1 * one_green + eps) == glm::u8vec3(141, 1, 0));
}

TEST_CASE("radix/height encoding round trip")
{
    using namespace sherpa::height_encoding;

    const auto test_cases = std::vector({
        0.0f,
        1.0f,
        50.f,
        122.f,
        1234.f,
        2495.f,
        3798.f,
        8191.25f,
    });
    for (auto v : test_cases)
        CHECK(to_float(to_rgb(v)) == Catch::Approx(v));
}

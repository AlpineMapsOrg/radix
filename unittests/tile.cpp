/*****************************************************************************
 * Alpine Radix
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

#include <catch2/catch_test_macros.hpp>

#include <radix/tile.h>

using namespace radix;

TEST_CASE("radix/tile::Id scheme conversion")
{
    SECTION("tms -> slippy map")
    {
        CHECK(tile::Id{ 0, { 0, 0 }, tile::Scheme::Tms }.to(tile::Scheme::SlippyMap) == tile::Id{ 0, { 0, 0 }, tile::Scheme::SlippyMap });
        CHECK(tile::Id{ 1, { 0, 0 }, tile::Scheme::Tms }.to(tile::Scheme::SlippyMap) == tile::Id{ 1, { 0, 1 }, tile::Scheme::SlippyMap });
        CHECK(tile::Id{ 1, { 1, 1 }, tile::Scheme::Tms }.to(tile::Scheme::SlippyMap) == tile::Id{ 1, { 1, 0 }, tile::Scheme::SlippyMap });
        CHECK(tile::Id{ 2, { 2, 3 }, tile::Scheme::Tms }.to(tile::Scheme::SlippyMap) == tile::Id{ 2, { 2, 0 }, tile::Scheme::SlippyMap });
        CHECK(tile::Id{ 2, { 3, 1 }, tile::Scheme::Tms }.to(tile::Scheme::SlippyMap) == tile::Id{ 2, { 3, 2 }, tile::Scheme::SlippyMap });
    }
    SECTION("slippy map -> tms")
    {
        CHECK(tile::Id{ 0, { 0, 0 }, tile::Scheme::SlippyMap }.to(tile::Scheme::Tms) == tile::Id{ 0, { 0, 0 }, tile::Scheme::Tms });
        CHECK(tile::Id{ 1, { 0, 0 }, tile::Scheme::SlippyMap }.to(tile::Scheme::Tms) == tile::Id{ 1, { 0, 1 }, tile::Scheme::Tms });
        CHECK(tile::Id{ 1, { 1, 1 }, tile::Scheme::SlippyMap }.to(tile::Scheme::Tms) == tile::Id{ 1, { 1, 0 }, tile::Scheme::Tms });
        CHECK(tile::Id{ 2, { 2, 3 }, tile::Scheme::SlippyMap }.to(tile::Scheme::Tms) == tile::Id{ 2, { 2, 0 }, tile::Scheme::Tms });
        CHECK(tile::Id{ 2, { 3, 1 }, tile::Scheme::SlippyMap }.to(tile::Scheme::Tms) == tile::Id{ 2, { 3, 2 }, tile::Scheme::Tms });
    }
    SECTION("tms -> tms (no op)")
    {
        CHECK(tile::Id{ 0, { 0, 0 }, tile::Scheme::Tms }.to(tile::Scheme::Tms) == tile::Id{ 0, { 0, 0 }, tile::Scheme::Tms });
        CHECK(tile::Id{ 1, { 0, 0 }, tile::Scheme::Tms }.to(tile::Scheme::Tms) == tile::Id{ 1, { 0, 0 }, tile::Scheme::Tms });
        CHECK(tile::Id{ 1, { 1, 1 }, tile::Scheme::Tms }.to(tile::Scheme::Tms) == tile::Id{ 1, { 1, 1 }, tile::Scheme::Tms });
        CHECK(tile::Id{ 2, { 2, 3 }, tile::Scheme::Tms }.to(tile::Scheme::Tms) == tile::Id{ 2, { 2, 3 }, tile::Scheme::Tms });
        CHECK(tile::Id{ 2, { 3, 1 }, tile::Scheme::Tms }.to(tile::Scheme::Tms) == tile::Id{ 2, { 3, 1 }, tile::Scheme::Tms });
    }
    SECTION("slippy map -> slippy map (no op)")
    {
        CHECK(tile::Id{ 0, { 0, 0 }, tile::Scheme::SlippyMap }.to(tile::Scheme::SlippyMap) == tile::Id{ 0, { 0, 0 }, tile::Scheme::SlippyMap });
        CHECK(tile::Id{ 1, { 0, 0 }, tile::Scheme::SlippyMap }.to(tile::Scheme::SlippyMap) == tile::Id{ 1, { 0, 0 }, tile::Scheme::SlippyMap });
        CHECK(tile::Id{ 1, { 1, 1 }, tile::Scheme::SlippyMap }.to(tile::Scheme::SlippyMap) == tile::Id{ 1, { 1, 1 }, tile::Scheme::SlippyMap });
        CHECK(tile::Id{ 2, { 2, 3 }, tile::Scheme::SlippyMap }.to(tile::Scheme::SlippyMap) == tile::Id{ 2, { 2, 3 }, tile::Scheme::SlippyMap });
        CHECK(tile::Id{ 2, { 3, 1 }, tile::Scheme::SlippyMap }.to(tile::Scheme::SlippyMap) == tile::Id{ 2, { 3, 1 }, tile::Scheme::SlippyMap });
    }
}

TEST_CASE("radix/tile::Id/parent")
{
    CHECK(tile::Id { 1, { 0, 1 } }.parent() == tile::Id { 0, { 0, 0 } });
    CHECK(tile::Id { 2, { 2, 1 } }.parent() == tile::Id { 1, { 1, 0 } });
    CHECK(tile::Id { 2, { 0, 0 } }.parent() == tile::Id { 1, { 0, 0 } });
    CHECK(tile::Id { 2, { 3, 3 } }.parent() == tile::Id { 1, { 1, 1 } });
}

TEST_CASE("radix/tile::Id/children")
{
    SECTION("child index tms (y pointing up)")
    {
        CHECK(tile::quad_position({ 1, { 0, 0 } }) == tile::QuadPosition::BottomLeft);
        CHECK(tile::quad_position({ 1, { 0, 1 } }) == tile::QuadPosition::TopLeft);
        CHECK(tile::quad_position({ 1, { 1, 0 } }) == tile::QuadPosition::BottomRight);
        CHECK(tile::quad_position({ 1, { 1, 1 } }) == tile::QuadPosition::TopRight);

        CHECK(tile::quad_position({ 1, { 0, 0 }, tile::Scheme::Tms }) == tile::QuadPosition::BottomLeft);
        CHECK(tile::quad_position({ 1, { 0, 1 }, tile::Scheme::Tms }) == tile::QuadPosition::TopLeft);
        CHECK(tile::quad_position({ 1, { 1, 0 }, tile::Scheme::Tms }) == tile::QuadPosition::BottomRight);
        CHECK(tile::quad_position({ 1, { 1, 1 }, tile::Scheme::Tms }) == tile::QuadPosition::TopRight);
    }

    SECTION("child index slippy map (y pointing down)")
    {
        CHECK(tile::quad_position({ 1, { 0, 1 }, tile::Scheme::SlippyMap }) == tile::QuadPosition::BottomLeft);
        CHECK(tile::quad_position({ 1, { 0, 0 }, tile::Scheme::SlippyMap }) == tile::QuadPosition::TopLeft);
        CHECK(tile::quad_position({ 1, { 1, 1 }, tile::Scheme::SlippyMap }) == tile::QuadPosition::BottomRight);
        CHECK(tile::quad_position({ 1, { 1, 0 }, tile::Scheme::SlippyMap }) == tile::QuadPosition::TopRight);
    }

    const auto array_contains = [](const std::array<tile::Id, 4>& array, const tile::Id& id) {
        for (const auto& e : array) {
            if (e == id)
                return true;
        }
        return false;
    };

    SECTION("mercator tms (y point up)")
    {
        {
            const auto tiles = tile::Id{0, {0, 0}}.children();
            REQUIRE(tiles.size() == 4);
            CHECK(array_contains(tiles, { 1, { 0, 0 } }));
            CHECK(array_contains(tiles, { 1, { 1, 0 } }));
            CHECK(array_contains(tiles, { 1, { 0, 1 } }));
            CHECK(array_contains(tiles, { 1, { 1, 1 } }));
        }
        {
            const auto tiles = tile::Id{1, {1, 1}}.children();
            REQUIRE(tiles.size() == 4);
            CHECK(array_contains(tiles, { 2, { 2, 2 } }));
            CHECK(array_contains(tiles, { 2, { 3, 2 } }));
            CHECK(array_contains(tiles, { 2, { 2, 3 } }));
            CHECK(array_contains(tiles, { 2, { 3, 3 } }));
        }
    }
    SECTION("mercator SlippyMap (y points down)")
    {
        {
            const auto tiles = tile::Id{0, {0, 0}, tile::Scheme::SlippyMap}.children();
            REQUIRE(tiles.size() == 4);
            CHECK(array_contains(tiles, { 1, { 0, 0 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 1, { 1, 0 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 1, { 0, 1 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 1, { 1, 1 }, tile::Scheme::SlippyMap }));
        }
        {
            const auto tiles = tile::Id{1, {1, 0}, tile::Scheme::SlippyMap}.children();
            REQUIRE(tiles.size() == 4);
            CHECK(array_contains(tiles, { 2, { 2, 1 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 2, { 3, 1 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 2, { 2, 0 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 2, { 3, 0 }, tile::Scheme::SlippyMap }));
        }
    }

    SECTION("geodetic tms (y point up)")
    {

        {
            const auto tiles = tile::Id{0, {1, 0}, tile::Scheme::Tms}.children();
            REQUIRE(tiles.size() == 4);
            CHECK(array_contains(tiles, { 1, { 2, 1 } }));
            CHECK(array_contains(tiles, { 1, { 3, 1 } }));
            CHECK(array_contains(tiles, { 1, { 2, 0 } }));
            CHECK(array_contains(tiles, { 1, { 3, 0 } }));
        }
        {
            const auto tiles = tile::Id{1, {3, 0}, tile::Scheme::Tms}.children();
            REQUIRE(tiles.size() == 4);
            CHECK(array_contains(tiles, { 2, { 6, 0 } }));
            CHECK(array_contains(tiles, { 2, { 7, 0 } }));
            CHECK(array_contains(tiles, { 2, { 6, 1 } }));
            CHECK(array_contains(tiles, { 2, { 7, 1 } }));
        }
    }

    SECTION("geodetic SlippyMap (y points down)")
    {
        {
            const auto tiles = tile::Id{0, {1, 0}, tile::Scheme::SlippyMap}.children();
            REQUIRE(tiles.size() == 4);
            CHECK(array_contains(tiles, { 1, { 2, 1 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 1, { 3, 1 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 1, { 2, 0 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 1, { 3, 0 }, tile::Scheme::SlippyMap }));
        }
        {
            const auto tiles = tile::Id{1, {3, 1}, tile::Scheme::SlippyMap}.children();
            REQUIRE(tiles.size() == 4);
            CHECK(array_contains(tiles, { 2, { 6, 3 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 2, { 7, 3 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 2, { 6, 2 }, tile::Scheme::SlippyMap }));
            CHECK(array_contains(tiles, { 2, { 7, 2 }, tile::Scheme::SlippyMap }));
        }
    }
}

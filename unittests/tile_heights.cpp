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

#include <filesystem>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include "TileHeights.h"
#include "quad_tree.h"

TEST_CASE("sherpa/TileHeights emplace and query")
{
    TileHeights d;
    SECTION("basic in and out")
    {
        d.emplace(tile::Id { 0, { 0, 0 } }, { 0.f, 100.f });
        {
            auto [min, max] = d.query(tile::Id { 0, { 0, 0 } });
            CHECK(min == 0.f);
            CHECK(max == 100.f);
        }

        d.emplace(tile::Id { 1, { 0, 0 } }, { 20.f, 50.f });
        {
            auto [min, max] = d.query(tile::Id { 1, { 0, 0 } });
            CHECK(min == 20.f);
            CHECK(max == 50.f);
        }
    }

    SECTION("defer to parent tiles, if unavailable")
    {
        d.emplace(tile::Id { 0, { 0, 0 } }, { 0.f, 100.f });
        {
            auto [min, max] = d.query(tile::Id { 1, { 0, 0 } });
            CHECK(min == 0.f);
            CHECK(max == 100.f);
        }
        {
            auto [min, max] = d.query(tile::Id { 2, { 0, 2 } });
            CHECK(min == 0.f);
            CHECK(max == 100.f);
        }
        d.emplace(tile::Id { 1, { 0, 0 } }, { 10.f, 20.f });
        d.emplace(tile::Id { 1, { 1, 1 } }, { 20.f, 40.f });
        {
            auto [min, max] = d.query(tile::Id { 4, { 0, 0 } });
            CHECK(min == 10.f);
            CHECK(max == 20.f);
        }
        {
            auto [min, max] = d.query(tile::Id { 4, { 14, 15 } });
            CHECK(min == 20.f);
            CHECK(max == 40.f);
        }
        {
            auto [min, max] = d.query(tile::Id { 4, { 3, 15 } });
            CHECK(min == 0.f);
            CHECK(max == 100.f);
        }
    }
}
TEST_CASE("sherpa/TileHeights query performance")
{
    TileHeights tile_heights;
    std::vector<tile::Id> ids;
    quad_tree::onTheFlyTraverse(
        tile::Id { 0, { 0, 0 } },
        [](const tile::Id& v) { return v.zoom_level < 14 && v.coords.x < 100 && v.coords.y < 40; },
        [&](const tile::Id& v) {
            tile_heights.emplace(v, { 0.f, 1.f });
            return v.children();
        });
    quad_tree::onTheFlyTraverse(
        tile::Id { 0, { 0, 0 } },
        [](const tile::Id& v) { return v.zoom_level < 18 && v.coords.x < 100 && v.coords.y < 40; },
        [&](const tile::Id& v) {
            ids.emplace_back(v);
            return v.children();
        });

    BENCHMARK("TileHeights::query()")
    {
        float retval = 0;
        for (const auto& id : ids) {
            retval += tile_heights.query(id).second;
        }
        return retval;
    };
}

TEST_CASE("sherpa/TileHeights serialisation")
{
    TileHeights d1;
    d1.emplace(tile::Id { 0, { 0, 0 } }, { 0.f, 100.f });
    d1.emplace(tile::Id { 1, { 0, 0 } }, { 10.f, 20.f });
    d1.emplace(tile::Id { 1, { 1, 1 } }, { 20.f, 40.f });

    const auto d1_bytes = d1.serialise();

    const auto d2 = TileHeights::deserialise(d1_bytes);
    {
        auto [min, max] = d2.query(tile::Id { 0, { 0, 0 } });
        CHECK(min == 0.f);
        CHECK(max == 100.f);
    }
    {
        auto [min, max] = d2.query(tile::Id { 1, { 0, 0 } });
        CHECK(min == 10.f);
        CHECK(max == 20.f);
    }
    {
        auto [min, max] = d2.query(tile::Id { 1, { 1, 1 } });
        CHECK(min == 20.f);
        CHECK(max == 40.f);
    }
}

TEST_CASE("sherpa/TileHeights io")
{
    const auto base_path = std::filesystem::path("./unittest_tile_heights");
    constexpr auto file_name = "height_data.atb";
    std::filesystem::remove_all(base_path);

    TileHeights d1;
    d1.emplace(tile::Id { 0, { 0, 0 } }, { 0.f, 100.f });
    d1.emplace(tile::Id { 1, { 0, 0 } }, { 10.f, 20.f });
    d1.emplace(tile::Id { 1, { 1, 1 } }, { 20.f, 40.f });
    d1.write_to(base_path / file_name);

    const auto d2 = TileHeights::read_from(base_path / file_name);
    {
        auto [min, max] = d2.query(tile::Id { 0, { 0, 0 } });
        CHECK(min == 0.f);
        CHECK(max == 100.f);
    }
    {
        auto [min, max] = d2.query(tile::Id { 1, { 0, 0 } });
        CHECK(min == 10.f);
        CHECK(max == 20.f);
    }
    {
        auto [min, max] = d2.query(tile::Id { 1, { 1, 1 } });
        CHECK(min == 20.f);
        CHECK(max == 40.f);
    }
}

/*****************************************************************************
 * Alpine Terrain Builder
 * Copyright (C) 2022 alpinemaps.org
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

#include "geometry.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

namespace {
template <int n_dims>
bool equals(const glm::vec<n_dims, double>& a, const glm::vec<n_dims, double>& b, double scale = 1)
{
    const auto delta = glm::length(a - b);
    return delta == Approx(0).scale(scale);
}

bool test_contains(const geometry::Triangle<3, double>& triangle, const glm::dvec3& b, double scale = 1)
{
    return ((glm::length(triangle[0] - b) == Approx(0).scale(scale)
        || glm::length(triangle[1] - b) == Approx(0).scale(scale)
        || glm::length(triangle[2] - b) == Approx(0).scale(scale)));
}
} // namespace

TEST_CASE("sherpa/geometry::Aabb intersect")
{
    CHECK(intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{0, 0}, {1, 1}}));
    CHECK(intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{-1, -1}, {2, 2}}));
    CHECK(intersect(geometry::Aabb2d{{-1, -1}, {2, 2}}, geometry::Aabb2d{{0, 0}, {1, 1}}));
    CHECK(intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{-1, 0.5}, {2, 0.8}}));
    CHECK(intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{-0.5, 0.5}, {2, 0.8}}));
    CHECK(intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{0.5, -0.5}, {2, 0.8}}));
    CHECK(intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{-0.5, -0.5}, {2.5, 0.5}}));
    CHECK(intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{-0.5, -0.5}, {0.5, 2.0}}));
    CHECK(intersect(geometry::Aabb<3, double>{{0, 0, 0}, {1, 1, 1}}, geometry::Aabb<3, double>{{-0.5, -0.5, -0.5}, {0.5, 2.0, 3.0}}));


    CHECK(!intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{-0.5, -0.5}, {-0.2, 0.2}}));
    CHECK(!intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{-0.5, -0.5}, {0.2, -0.2}}));
    CHECK(!intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{1.5, 0.5}, {1.7, 0.8}}));
    CHECK(!intersect(geometry::Aabb2d{{0, 0}, {1, 1}}, geometry::Aabb2d{{0.5, 1.5}, {0.7, 1.8}}));
    CHECK(!intersect(geometry::Aabb<3, double>{{0, 0, 0}, {1, 1, 1}}, geometry::Aabb<3, double>{{-0.5, -0.5, 1.5}, {0.5, 2.0, 3.0}}));
}

TEST_CASE("sherpa/geometry::Aabb::contains")
{
    CHECK(geometry::Aabb<3, double>{{0, 0, 0}, {1, 1, 1}}.contains({0.5, 0.5, 0.5}));
    CHECK(geometry::Aabb<3, double>{{0, 0, 0}, {1, 1, 1}}.contains({0.0, 0.0, 0.0}));
    CHECK(!geometry::Aabb<3, double>{{0, 0, 0}, {1, 1, 1}}.contains({1.0, 1.0, 1.0}));
    CHECK(!geometry::Aabb<3, double>{{0, 0, 0}, {1, 1, 1}}.contains({1.5, 0.5, 0.5}));
}


TEST_CASE("sherpa/geometry")
{
    SECTION("aabb")
    {
        const auto box = geometry::Aabb<3, double> { .min = { 0.0, 1.0, 2.0 }, .max = { 10.0, 11.0, 12.0 } };
        CHECK(geometry::inside(glm::dvec3(5, 5, 5), box));
        CHECK(!geometry::inside(glm::dvec3(5, 15, 5), box));
        CHECK(equals(geometry::centroid(box), { 5.0, 6.0, 7.0 }));

        CHECK(geometry::distance(geometry::Aabb<3, double> { .min = { 0.0, 0.0, 0.0 }, .max = { 1.0, 1.0, 1.0 } }, glm::dvec3(0, 0, 0)) == Approx(0));
        CHECK(geometry::distance(geometry::Aabb<3, double> { .min = { 0.0, 0.0, 0.0 }, .max = { 1.0, 1.0, 1.0 } }, glm::dvec3(-1, 0, 0)) == Approx(1));
        CHECK(geometry::distance(geometry::Aabb<3, double> { .min = { 0.0, 1.0, 0.0 }, .max = { 1.0, 2.0, 1.0 } }, glm::dvec3(-1, 0, 0)) == Approx(std::sqrt(2)));
        CHECK(geometry::distance(geometry::Aabb<3, double> { .min = { 0.0, 1.0, 2.0 }, .max = { 1.0, 2.0, 1.0 } }, glm::dvec3(-1, 0, 1)) == Approx(std::sqrt(3)));
        CHECK(geometry::distance(geometry::Aabb<3, double> { .min = { 0.0, 0.0, 0.0 }, .max = { 1.0, 1.0, 1.0 } }, glm::dvec3(2, 0, 0)) == Approx(1));
        CHECK(geometry::distance(geometry::Aabb<3, double> { .min = { 10.0, 10.0, 10.0 }, .max = { 22.0, 22.0, 22.0 } }, glm::dvec3(0, 0, 0)) == Approx(std::sqrt(300)));
        CHECK(geometry::distance(geometry::Aabb<3, double> { .min = { 10.0, 10.0, 10.0 }, .max = { 22.0, 22.0, 22.0 } }, glm::dvec3(23, 23, 23)) == Approx(std::sqrt(3)));
        CHECK(geometry::distance(geometry::Aabb<3, double> { .min = { 10.0, 10.0, 10.0 }, .max = { 22.0, 22.0, 22.0 } }, glm::dvec3(23, 22, 22)) == Approx(1));
    }
    SECTION("aabb size")
    {
        CHECK(geometry::Aabb3d { { -0.5, -0.5, -0.5 }, { 0.5, 0.5, 0.5 } }.size().x == Approx(1.0));
        CHECK(geometry::Aabb3d { { -0.5, -0.5, -0.5 }, { 0.5, 0.5, 0.5 } }.size().y == Approx(1.0));
        CHECK(geometry::Aabb3d { { -0.5, -0.5, -0.5 }, { 0.5, 0.5, 0.5 } }.size().z == Approx(1.0));
        CHECK(glm::length(geometry::Aabb3d { { -0.5, -0.5, -0.5 }, { 0.5, 0.5, 0.5 } }.size()) == Approx(std::sqrt(3)));
    }

    SECTION("triangle normal")
    {
        const auto tri = geometry::Triangle<3, double> { { { 1.0, 0.0, 0.0 }, { 2.0, 0.0, 0.0 }, { 0.0, 5.0, 0.0 } } };
        CHECK(equals(geometry::normal(tri), { 0.0, 0.0, 1.0 }));
    }

    SECTION("aabb to triangles")
    {
        const auto box = geometry::Aabb<3, double> { .min = { 0.0, 1.0, 2.0 }, .max = { 10.0, 11.0, 12.0 } };
        const auto centroid = geometry::centroid(box);
        const auto triangles = geometry::triangulise(box);
        unsigned cnt_minx = 0;
        unsigned cnt_miny = 0;
        unsigned cnt_minz = 0;
        unsigned cnt_maxx = 0;
        unsigned cnt_maxy = 0;
        unsigned cnt_maxz = 0;
        REQUIRE(triangles.size() == 6u * 2u);
        for (const auto& tri : triangles) {
            for (unsigned i = 0; i < 3; ++i) {
                CHECK(((tri[i].x == 0.0 || tri[i].x == 10.0) && (tri[i].y == 1.0 || tri[i].y == 11.0) && (tri[i].z == 2.0 || tri[i].z == 12.0)));
                if (tri[i].x == 0.0)
                    cnt_minx++;
                if (tri[i].x == 10.0)
                    cnt_maxx++;
                if (tri[i].y == 1.0)
                    cnt_miny++;
                if (tri[i].y == 11.0)
                    cnt_maxy++;
                if (tri[i].z == 2.0)
                    cnt_minz++;
                if (tri[i].z == 12.0)
                    cnt_maxz++;
            }
            const auto normal = geometry::normal(tri);
            const auto centroid_a = glm::normalize(tri[0] - centroid);
            const auto centroid_b = glm::normalize(tri[1] - centroid);
            const auto centroid_c = glm::normalize(tri[2] - centroid);

            CHECK(glm::dot(normal, centroid_a) > 0);
            CHECK(glm::dot(normal, centroid_b) > 0);
            CHECK(glm::dot(normal, centroid_c) > 0);
        }
        CHECK(cnt_minx == 3 + 3 + 3 * 4);
        CHECK(cnt_miny == 3 + 3 + 3 * 4);
        CHECK(cnt_minz == 3 + 3 + 3 * 4);

        CHECK(cnt_maxx == 3 + 3 + 3 * 4);
        CHECK(cnt_maxy == 3 + 3 + 3 * 4);
        CHECK(cnt_maxz == 3 + 3 + 3 * 4);
    }

    SECTION("plane")
    {
        const auto plane = geometry::Plane<double> { glm::normalize(glm::dvec3 { 1.0, 1.0, 1.0 }), -std::sqrt(3) };
        CHECK(geometry::distance(plane, { 1.0, 1.0, 1.0 }) == Approx(0.0).scale(1.0));
        CHECK(geometry::distance(plane, { 2.0, 2.0, 2.0 }) == Approx(std::sqrt(3)).scale(1.0));
        CHECK(geometry::distance(plane, { 0.0, 0.0, 0.0 }) == Approx(-std::sqrt(3)).scale(1.0));
        {
            const auto intersection = geometry::intersection(geometry::Edge<3, double>{ glm::dvec3 { 0.0, 0.0, 0.0 }, glm::dvec3 { 2.0, 2.0, 2.0 } }, plane);
            CHECK(intersection.x == Approx(1.0));
            CHECK(intersection.y == Approx(1.0));
            CHECK(intersection.z == Approx(1.0));
        }
        {
            const auto intersection = geometry::intersection(geometry::Edge<3, double>{ glm::dvec3 { 4.0, 4.0, 4.0 }, glm::dvec3 { -1.0, -1.0, -1.0 } }, plane);
            CHECK(intersection.x == Approx(1.0));
            CHECK(intersection.y == Approx(1.0));
            CHECK(intersection.z == Approx(1.0));
        }
    }

    SECTION("plane plane intersection") {
        CHECK(!geometry::intersection(geometry::Plane<double> {glm::dvec3 { 1.0, 0.0, 0.0 }, 1 }, geometry::Plane<double> {glm::dvec3 { 1.0, 0.0, 0.0 }, -1}));

        {
            const auto i = geometry::intersection(geometry::Plane<double> {glm::dvec3 { 1.0, 0.0, 0.0 }, -1 }, geometry::Plane<double> {glm::dvec3 { 0.0, -1.0, 0.0 }, -2});
            REQUIRE(i);
            CHECK(equals(i.value().direction, glm::dvec3{0, 0, -1}));
            CHECK(equals(i.value().point, glm::dvec3{1, -2, 0}));
        }
        {
            const auto i = geometry::intersection(geometry::Plane<double> {glm::dvec3 { 0.0, 0.0, 1.0 }, 5 }, geometry::Plane<double> {glm::dvec3 { 0.0, 1.0, 0.0 }, -1});
            REQUIRE(i);
            CHECK(equals(i.value().direction, glm::dvec3{-1, 0, 0}));
            CHECK(equals(i.value().point, glm::dvec3{0, 1, -5}));
        }
    }

    SECTION("line plane intersection") {
        CHECK(!geometry::intersection(geometry::Line<3, double> {glm::dvec3 { 0.0, 0.0, 0.0 }, glm::dvec3 { 0.0, 1.0, 0.0 } }, geometry::Plane<double> {glm::dvec3 { 0.0, 0.0, 1.0 }, -1}));

        {
            const auto i = geometry::intersection(geometry::Line<3, double> {.point = glm::dvec3 { 0.0, 0.0, 0.0 }, .direction = glm::dvec3 { 0.0, 0.0, 1.0 } },
                                                  geometry::Plane<double> {glm::dvec3 { 0.0, 0.0, 1.0 }, -1});
            REQUIRE(i);
            CHECK(equals(i.value(), glm::dvec3{0, 0, 1}));
        }
        {
            const auto i = geometry::intersection(geometry::Line<3, double> {.point = glm::dvec3 { 2.0, 2.0, 2.0 }, .direction = glm::normalize(glm::dvec3 { 1.0, 1.0, 1.0 }) },
                                                  geometry::Plane<double> {glm::normalize(glm::dvec3 { 1.0, 2.0, 3.0 }), 0});
            REQUIRE(i);
            CHECK(equals(i.value(), glm::dvec3{0, 0, 0}));
        }
    }

    SECTION("clip triangle with plane")
    {
        const auto t0 = glm::dvec3(0.0, 0.0, 0.0);
        const auto t1 = glm::dvec3(10.0, 0.0, 0.0);
        const auto t2 = glm::dvec3(0.0, 10.0, 0.0);
        { // completely behind the plane
            const auto plane = geometry::Plane<double> { glm::normalize(glm::dvec3 { -1.0, -1.0, -1.0 }), -std::sqrt(3) };
            const auto triangle = geometry::Triangle<3, double> { t0, t1, t2 };
            const auto clipped = geometry::clip(triangle, plane);
            CHECK(clipped.empty());
        }
        { // completely in front of the plane
            const auto plane = geometry::Plane<double> { glm::normalize(glm::dvec3 { -1.0, -1.0, -1.0 }), 100 };
            const auto triangle = geometry::Triangle<3, double> { t0, t1, t2 };
            const auto clipped = geometry::clip(triangle, plane);
            REQUIRE(clipped.size() == 1);
            CHECK(clipped[0] == triangle);
        }
        { // one vertex in front
            const auto plane = geometry::Plane<double> { glm::normalize(glm::dvec3 { -1.0, -1.0, -1.0 }), std::sqrt(3) };
            const auto check_fun = [&plane](const geometry::Triangle<3, double>& triangle) {
                const auto clipped = geometry::clip(triangle, plane);
                REQUIRE(clipped.size() == 1);
                CHECK(equals(geometry::normal(triangle), geometry::normal(clipped[0])));
                CHECK(test_contains(clipped[0], { 0.0, 0.0, 0.0 }));
                CHECK(test_contains(clipped[0], { 3.0, 0.0, 0.0 }));
                CHECK(test_contains(clipped[0], { 0.0, 3.0, 0.0 }));
            };
            check_fun({ t0, t1, t2 });
            check_fun({ t1, t2, t0 });
            check_fun({ t2, t0, t1 });
        }
        { // two vertices in front
            const auto plane = geometry::Plane<double> { glm::normalize(glm::dvec3 { 1.0, 1.0, 1.0 }), -std::sqrt(3) };
            const auto check_fun = [&plane](const geometry::Triangle<3, double>& triangle) {
                const auto clipped = geometry::clip(triangle, plane);
                REQUIRE(clipped.size() == 2);
                CHECK(equals(geometry::normal(triangle), geometry::normal(clipped[0])));
                CHECK(equals(geometry::normal(triangle), geometry::normal(clipped[1])));
                unsigned found_vertices = 0;
                found_vertices += test_contains(clipped[0], { 3.0, 0.0, 0.0 });
                found_vertices += test_contains(clipped[1], { 3.0, 0.0, 0.0 });
                found_vertices += test_contains(clipped[0], { 0.0, 3.0, 0.0 });
                found_vertices += test_contains(clipped[1], { 0.0, 3.0, 0.0 });
                found_vertices += test_contains(clipped[0], { 10.0, 0.0, 0.0 });
                found_vertices += test_contains(clipped[1], { 10.0, 0.0, 0.0 });
                found_vertices += test_contains(clipped[0], { 0.0, 10.0, 0.0 });
                found_vertices += test_contains(clipped[1], { 0.0, 10.0, 0.0 });
                CHECK(found_vertices == 6);
            };
            check_fun({ t0, t1, t2 });
            check_fun({ t1, t2, t0 });
            check_fun({ t2, t0, t1 });
        }
    }

    SECTION("clip several triangles at once")
    {
        const auto plane = geometry::Plane<double> { glm::normalize(glm::dvec3 { 1.0, 1.0, 1.0 }), -std::sqrt(3) };
        const auto box = geometry::Aabb<3, double> { .min = { 0.0, -1.0, -2.0 }, .max = { 10.0, 11.0, 12.0 } };
        const auto triangles = geometry::triangulise(box);
        const auto clipped_triangles = geometry::clip(triangles, plane);
        CHECK(!clipped_triangles.empty());
    }
}

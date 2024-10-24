/*****************************************************************************
 * Alpine Terrain Builder
 * Copyright (C) 2022 Adam Celarek
 * Copyright (c) 2015 Wenzel Jakob (for AABB to point distance)
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
#include <array>
#include <iterator>
#include <optional>
#include <vector>

#include <glm/glm.hpp>

namespace radix::geometry {

template <typename T = double>
    requires std::is_floating_point_v<T>
inline constexpr T epsilon = 0.0000001;

template <>
inline constexpr float epsilon<float> = 0.0001f;

template <glm::length_t n_dims, class T>
class Aabb {
    using Vec = glm::vec<n_dims, T>;

public:
    Vec min = {};
    Vec max = {};

    bool operator==(const Aabb<n_dims, T>& other) const = default;

    [[nodiscard]] glm::vec<n_dims, T> size() const { return max - min; }
    [[nodiscard]] bool contains(const Vec& point) const
    {
        return glm::all(glm::lessThanEqual(min, point)) && glm::all(glm::greaterThan(max, point));
    }
    [[nodiscard]] bool contains_inclusive(const Vec &point) const {
        return glm::all(glm::lessThanEqual(min, point)) && glm::all(glm::greaterThanEqual(max, point));
    }
    [[nodiscard]] bool contains_exclusive(const Vec &point) const {
        return glm::all(glm::lessThan(min, point)) && glm::all(glm::greaterThan(max, point));
    }
    [[nodiscard]] glm::vec<n_dims, T> centre() const { return min + size() * T(0.5); }

    void expand_by(const Vec &point) {
        this->min = glm::min(this->min, point);
        this->max = glm::max(this->max, point);
    }
    void expand_by(const Aabb<n_dims, T> &other) {
        this->min = glm::min(this->min, other.min);
        this->max = glm::max(this->max, other.max);
    }
};

template <class T>
class Aabb2 : public Aabb<2, T> {
    using Base = Aabb<2, T>;
    using Vec = glm::vec<2, T>;
public:
    using Base::min;
    using Base::max;
    Aabb2() = default;
    Aabb2(const Vec& min, const Vec& max) : Base{min, max} {}
    Aabb2(const geometry::Aabb<3, T>& other) : Base{Vec(other.min), Vec(other.max)} {}

    [[nodiscard]] T width() const { return max.x - min.x; }
    [[nodiscard]] T height() const { return max.y - min.y; }
};

using Aabb2d = Aabb2<double>;
using Aabb2f = Aabb2<float>;
using Aabb2i = Aabb2<int>;
using Aabb2ui = Aabb2<unsigned int>;
using Aabb2i64 = Aabb2<glm::int64>;


using Aabb3d = Aabb<3, double>;
using Aabb3f = Aabb<3, float>;
using Aabb3i = Aabb<3, int>;
using Aabb3ui = Aabb<3, unsigned int>;
using Aabb3i64 = Aabb<3, glm::int64>;


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

template <glm::length_t n_dimensions, typename T>
using Triangle = std::array<glm::vec<n_dimensions, T>, 3>;

/// defined by two points, i.e, an edge is finite
template <glm::length_t n_dimensions, typename T>
using Edge = std::array<glm::vec<n_dimensions, T>, 2>;

/// defined by a point and an direction, it is inifinite in both directions
template <glm::length_t n_dimensions, typename T>
struct Line {
    glm::vec<n_dimensions, T> point;
    glm::vec<n_dimensions, T> direction;
};
// deduction guide:
template <glm::length_t n_dimensions, typename T>
Line(glm::vec<n_dimensions, T>, glm::vec<n_dimensions, T>) -> Line<n_dimensions, T>;

template <typename T>
struct Plane {
    glm::vec<3, T> normal;
    T distance;
};
// deduction guide:
template <typename T>
Plane(glm::vec<3, T>, T) -> Plane<T>;

// functions
template <glm::length_t n_dimensions, typename T>
bool inside(const glm::vec<n_dimensions, T>& point, const Aabb<n_dimensions, T>& box)
{
    return box.contains(point);
}

template <typename T>
glm::tvec3<T> normal(const Triangle<3, T>& triangle)
{
    const auto ab = triangle[1] - triangle[0];
    const auto ac = triangle[2] - triangle[0];
    return glm::normalize(glm::cross(glm::normalize(ab), glm::normalize(ac)));
}

template <glm::length_t n_dimensions, typename T>
glm::vec<n_dimensions, T> centroid(const Aabb<n_dimensions, T>& box)
{
    return (box.max + box.min) * T(0.5);
}

template <glm::length_t n_dimensions, typename T>
T distance(const Aabb<n_dimensions, T>& box, const glm::vec<n_dimensions, T>& point)
{
    T distance_squared = 0;
    for (int i = 0; i < n_dimensions; ++i) {
        T value = 0;
        if (point[i] < box.min[i])
            value = box.min[i] - point[i];
        else if (point[i] > box.max[i])
            value = point[i] - box.max[i];
        distance_squared += value * value;
    }
    return std::sqrt(distance_squared);
}

template <glm::length_t n_dimensions, typename T>
T distance(const glm::vec<n_dimensions, T>& a, const glm::vec<n_dimensions, T>& b)
{
    return glm::distance(a, b);
}

template <typename T>
T distance(const Plane<T>& plane, const glm::tvec3<T>& point)
{
    // from https://gabrielgambetta.com/computer-graphics-from-scratch/11-clipping.html
    return glm::dot(plane.normal, point) + plane.distance;
}

template <typename T>
std::optional<glm::tvec3<T>> intersection(const Line<3, T>& line, const Plane<T>& plane)
{
    // adapted from https://gabrielgambetta.com/computer-graphics-from-scratch/11-clipping.html
    const auto dot = glm::dot(plane.normal, line.direction);
    if (std::abs(dot) < epsilon<T>)
        return {};
    T t = (-plane.distance - glm::dot(plane.normal, line.point)) / dot;
    return line.point + t * line.direction;
}

template <typename T>
glm::tvec3<T> intersection(const Edge<3, T>& line, const Plane<T>& plane)
{
    const auto direction = line[1] - line[0];
    return intersection(Line{line[0], direction}, plane).value();
}


template <typename T>
std::optional<Line<3, T>> intersection(const Plane<T>& p1, const Plane<T>& p2)
{
    // adapted from https://stackoverflow.com/questions/6408670/line-of-intersection-between-two-planes

    // logically the 3rd plane, but we only use the normal component.
    const auto p3_normal = glm::cross(p1.normal, p2.normal);
    const auto det = glm::dot(p3_normal, p3_normal);

    // If the determinant is 0, that means parallel planes, no intersection.
    if (std::abs(det) <= epsilon<T>)
        return {};

    // calculate the final (point, normal)
    const auto r_point = ((glm::cross(p3_normal, p2.normal) * p1.distance) +
                          (glm::cross(p1.normal, p3_normal) * p2.distance)) / det;
    return Line<3, T>{r_point, p3_normal};
}

template <typename T>
std::vector<Triangle<3, T>> clip(const Triangle<3, T>& triangle, const Plane<T>& plane)
{
    using Tri = Triangle<3, T>;
    const auto triangles_2in = [&plane](const glm::tvec3<T>& inside_a, const glm::tvec3<T>& inside_b, const glm::tvec3<T>& outside_c) {
        const auto a_prime = intersection(geometry::Edge<3, T>{ inside_a, outside_c }, plane);
        const auto b_prime = intersection(geometry::Edge<3, T>{ inside_b, outside_c }, plane);
        return std::vector<Tri>({ Tri { inside_a, inside_b, b_prime }, Tri { inside_a, b_prime, a_prime } });
    };

    const auto b0 = distance(plane, triangle[0]) > 0;
    const auto b1 = distance(plane, triangle[1]) > 0;
    const auto b2 = distance(plane, triangle[2]) > 0;
    const auto s = b0 + b1 + b2;
    if (s == 3)
        return { triangle };
    if (s == 2) {
        if (!b0)
            return triangles_2in(triangle[1], triangle[2], triangle[0]);
        if (!b1)
            return triangles_2in(triangle[2], triangle[0], triangle[1]);
        if (!b2)
            return triangles_2in(triangle[0], triangle[1], triangle[2]);
        assert(false);
    }
    if (s == 1) {
        if (b0)
            return { { triangle[0], intersection(geometry::Edge<3, T>{ triangle[0], triangle[1] }, plane), intersection(geometry::Edge<3, T>{ triangle[0], triangle[2] }, plane) } };
        if (b1)
            return { { intersection(geometry::Edge<3, T>{ triangle[0], triangle[1] }, plane), triangle[1], intersection(geometry::Edge<3, T>{ triangle[1], triangle[2] }, plane) } };
        if (b2)
            return { { intersection(geometry::Edge<3, T>{ triangle[0], triangle[2] }, plane), intersection(geometry::Edge<3, T>{ triangle[1], triangle[2] }, plane), triangle[2] } };
        assert(false);
    }
    return {};
}

template <typename T>
std::vector<Triangle<3, T>> clip(const std::vector<Triangle<3, T>>& triangles, const Plane<T>& plane)
{
    std::vector<Triangle<3, T>> ret;
    ret.reserve(triangles.size() * 2);
    for (const auto& tri : triangles) {
        const auto clipped = clip(tri, plane);
        std::copy(clipped.begin(), clipped.end(), std::back_inserter(ret));
    }
    return ret;
}

template <typename T, typename PlaneContainer>
std::vector<Triangle<3, T>> clip(std::vector<Triangle<3, T>> triangles, const PlaneContainer& planes)
{
    for (const auto& plane : planes)
        triangles = geometry::clip(triangles, plane);
    return triangles;
}

template <typename T>
std::vector<Triangle<3, T>> triangulise(const Aabb<3, T>& box)
{
    using Tri = Triangle<3, T>;
    using Vert = glm::vec<3, T>;

    // counter clockwise top face
    const auto a = Vert { box.min.x, box.min.y, box.max.z };
    const auto b = Vert { box.max.x, box.min.y, box.max.z };
    const auto c = Vert { box.max.x, box.max.y, box.max.z };
    const auto d = Vert { box.min.x, box.max.y, box.max.z };
    // ccw bottom face, normal points up
    const auto e = Vert { box.min.x, box.min.y, box.min.z };
    const auto f = Vert { box.max.x, box.min.y, box.min.z };
    const auto g = Vert { box.max.x, box.max.y, box.min.z };
    const auto h = Vert { box.min.x, box.max.y, box.min.z };

    return std::vector<Tri>({ { a, b, c }, { a, c, d },
        { a, e, b }, { b, e, f },
        { b, f, c }, { c, f, g },
        { c, g, d }, { d, g, h },
        { d, h, a }, { a, h, e },
        { e, g, f }, { e, h, g } });
}

template<typename T>
std::array<glm::vec<3, T>, 8> corners(const Aabb<3, T> &box)
{
    using Vert = glm::vec<3, T>;

    // counter clockwise top face
    const auto a = Vert{box.min.x, box.min.y, box.max.z};
    const auto b = Vert{box.max.x, box.min.y, box.max.z};
    const auto c = Vert{box.max.x, box.max.y, box.max.z};
    const auto d = Vert{box.min.x, box.max.y, box.max.z};
    // ccw bottom face, normal points up
    const auto e = Vert{box.min.x, box.min.y, box.min.z};
    const auto f = Vert{box.max.x, box.min.y, box.min.z};
    const auto g = Vert{box.max.x, box.max.y, box.min.z};
    const auto h = Vert{box.min.x, box.max.y, box.min.z};

    return {a, b, c, d, e, f, g, h};
}
} // namespace radix::geometry

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

#include <radix/iterator.h>

#include <unordered_set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("radix/iterator: unordered_inserter")
{
    std::unordered_set<int> set;
    std::vector<int> l = { 0, 1, 2, 3, 4, 5 };
    std::copy(l.begin(), l.end(), radix::unordered_inserter(set));
    CHECK(set.size() == 6);
    CHECK(set.contains(0));
    CHECK(set.contains(1));
    CHECK(set.contains(2));
    CHECK(set.contains(3));
    CHECK(set.contains(4));
    CHECK(set.contains(5));
}

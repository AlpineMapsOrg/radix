/*****************************************************************************
 * Alpine Renderer
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

#pragma once

#include <iterator>

namespace sherpa {

// from https://stackoverflow.com/questions/27429705/std-insert-iterator-for-unordered-sets-or-maps
template <typename Container>
class unordered_insert_iterator {
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using reference_type = void;
    using difference_type = void;
    using pointer = void;
    using reference = void;
    using container_type = Container;

    unordered_insert_iterator& operator++() { return *this; } // no-op
    unordered_insert_iterator& operator++(int) { return *this; } // no-op
    [[nodiscard]] unordered_insert_iterator& operator*() { return *this; } // no-op
    constexpr unordered_insert_iterator& operator=(const typename Container::value_type& value)
    {
        container.insert(value);
        return *this;
    }
    constexpr unordered_insert_iterator& operator=(typename Container::value_type&& value)
    {
        container.insert(std::move(value));
        return *this;
    }
    unordered_insert_iterator(Container& container)
        : container(container)
    {
    }

private:
    Container& container;
};

template <typename Container>
[[nodiscard]] unordered_insert_iterator<Container> unordered_inserter(Container& container) // usually I would prefer Container*, but let's stay compatible with the stl here (yes/no?)
{
    return unordered_insert_iterator<Container>(container);
}

}

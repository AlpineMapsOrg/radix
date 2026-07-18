/*****************************************************************************
 * AlpineMaps.org
 * Copyright (C) 2022 Adam Celarek
 * Copyright (C) 2024 Gerald Kimmersdorfer
 * Copyright (C) 2024 Lucas Dworschak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

namespace radix {

enum class RasterError {
    IncompatibleDimensions,
    InvalidMipmapDimensions,
};

template <typename T>
class Raster;

namespace raster {
    template <typename T>
    [[nodiscard]] std::expected<void, RasterError> append_vertically(Raster<T>& destination, const Raster<T>& source);
}

template <typename T>
class Raster {
    static_assert(!std::same_as<std::remove_cv_t<T>, bool>,
        "Raster<bool> is not supported because std::vector<bool> is "
        "not contiguous; use RasterMask instead.");

public:
    Raster() = default;

    explicit Raster(const unsigned square_side_length)
        : Raster(glm::uvec2(square_side_length))
    {
    }

    Raster(const unsigned square_side_length, std::vector<T>&& data)
        : Raster(glm::uvec2(square_side_length), std::move(data))
    {
    }

    explicit Raster(const glm::uvec2 size)
    {
        const auto count = checked_element_count(size);
        if (!count.has_value()) {
            assert(false && "Raster dimensions exceed the backing vector's capacity");
            return;
        }
        m_size = size;
        m_data.resize(*count);
    }

    Raster(const glm::uvec2 size, const T& fill_value)
    {
        const auto count = checked_element_count(size);
        if (!count.has_value()) {
            assert(false && "Raster dimensions exceed the backing vector's capacity");
            return;
        }
        m_size = size;
        m_data.assign(*count, fill_value);
    }

    Raster(const glm::uvec2 size, std::vector<T>&& data)
    {
        const auto count = checked_element_count(size);
        if (!count.has_value() || *count != data.size()) {
            assert(false && "Raster dimensions do not match the supplied vector");
            return;
        }
        m_size = size;
        m_data = std::move(data);
    }

    [[nodiscard]] std::span<const T> buffer() const noexcept { return m_data; }
    [[nodiscard]] std::span<T> buffer() noexcept { return m_data; }
    [[nodiscard]] unsigned width() const noexcept { return m_size.x; }
    [[nodiscard]] unsigned height() const noexcept { return m_size.y; }
    [[nodiscard]] glm::uvec2 size() const noexcept { return m_size; }
    [[nodiscard]] std::size_t size_in_bytes() const noexcept { return bytes().size(); }
    [[nodiscard]] std::size_t size_per_line() const noexcept { return static_cast<std::size_t>(m_size.x) * sizeof(T); }
    [[nodiscard]] std::size_t buffer_length() const noexcept { return m_data.size(); }

    [[nodiscard]] const T& pixel(const glm::uvec2 position) const noexcept
    {
        if (position.x >= m_size.x || position.y >= m_size.y) {
            assert(false && "Raster pixel coordinates are out of bounds");
            return dummy_pixel();
        }
        return m_data[index(position)];
    }

    [[nodiscard]] T& pixel(const glm::uvec2 position) noexcept
    {
        if (position.x >= m_size.x || position.y >= m_size.y) {
            assert(false && "Raster pixel coordinates are out of bounds");
            return dummy_pixel();
        }
        return m_data[index(position)];
    }

    [[nodiscard]] std::span<const std::byte> bytes() const noexcept { return std::as_bytes(buffer()); }
    [[nodiscard]] std::span<std::byte> bytes() noexcept { return std::as_writable_bytes(buffer()); }

    void fill(const T& value) { std::ranges::fill(m_data, value); }

    auto begin() noexcept { return m_data.begin(); }
    auto end() noexcept { return m_data.end(); }
    auto begin() const noexcept { return m_data.begin(); }
    auto end() const noexcept { return m_data.end(); }
    auto cbegin() const noexcept { return m_data.cbegin(); }
    auto cend() const noexcept { return m_data.cend(); }

    const T* data() const noexcept { return m_data.data(); }
    T* data() noexcept { return m_data.data(); }

private:
    [[nodiscard]] static std::expected<std::size_t, RasterError> checked_element_count(const glm::uvec2 size) noexcept
    {
        constexpr auto max_size = std::numeric_limits<std::size_t>::max();
        if (size.x != 0 && static_cast<std::size_t>(size.y) > max_size / size.x)
            return std::unexpected(RasterError::IncompatibleDimensions);

        const auto count = static_cast<std::size_t>(size.x) * size.y;
        if (count > std::vector<T>().max_size())
            return std::unexpected(RasterError::IncompatibleDimensions);
        return count;
    }

    [[nodiscard]] std::size_t index(const glm::uvec2 position) const noexcept { return static_cast<std::size_t>(position.y) * m_size.x + position.x; }

    [[nodiscard]] static T& dummy_pixel() noexcept
    {
        static thread_local T dummy {};
        return dummy;
    }

    template <typename U>
    friend std::expected<void, RasterError> raster::append_vertically(Raster<U>&, const Raster<U>&);

    glm::uvec2 m_size { 0, 0 };
    std::vector<T> m_data;
};

// std::vector<bool> does not provide contiguous storage and therefore cannot
// expose the same span-based API as Raster<T>. A byte mask is predictable,
// writable, and interoperable with native and GPU APIs.
using RasterMask = Raster<std::uint8_t>;

namespace raster {

    template <typename T>
    [[nodiscard]] std::expected<void, RasterError> append_vertically(Raster<T>& destination, const Raster<T>& source)
    {
        if (destination.width() != source.width())
            return std::unexpected(RasterError::IncompatibleDimensions);

        const auto new_height = static_cast<std::uint64_t>(destination.height()) + source.height();
        if (new_height > std::numeric_limits<unsigned>::max())
            return std::unexpected(RasterError::IncompatibleDimensions);

        const auto destination_length = destination.buffer_length();
        const auto source_length = source.buffer_length();
        if (source_length > destination.m_data.max_size() - destination_length)
            return std::unexpected(RasterError::IncompatibleDimensions);

        destination.m_data.reserve(destination_length + source_length);
        if (&destination == &source) {
            destination.m_data.resize(destination_length + source_length);
            std::copy_n(destination.m_data.begin(), source_length, destination.m_data.begin() + destination_length);
        } else {
            destination.m_data.insert(destination.m_data.end(), source.begin(), source.end());
        }
        destination.m_size.y = static_cast<unsigned>(new_height);
        return {};
    }

    namespace detail {

        template <std::integral T>
        [[nodiscard]] constexpr T average_four(const T a, const T b, const T c, const T d)
        {
            static_assert(sizeof(T) < sizeof(std::uint64_t),
                "64-bit integral mipmap pixels require a dedicated "
                "overflow-safe average");
            using Accumulator = std::conditional_t<std::is_signed_v<T>, std::int64_t, std::uint64_t>;
            return static_cast<T>((static_cast<Accumulator>(a) + static_cast<Accumulator>(b) + static_cast<Accumulator>(c) + static_cast<Accumulator>(d)) / 4);
        }

        template <std::floating_point T>
        [[nodiscard]] constexpr T average_four(const T a, const T b, const T c, const T d)
        {
            return (a + b + c + d) / static_cast<T>(4);
        }

        template <glm::length_t Length, typename T, glm::qualifier Qualifier>
        [[nodiscard]] constexpr glm::vec<Length, T, Qualifier> average_four(const glm::vec<Length, T, Qualifier>& a,
            const glm::vec<Length, T, Qualifier>& b,
            const glm::vec<Length, T, Qualifier>& c,
            const glm::vec<Length, T, Qualifier>& d)
        {
            glm::vec<Length, T, Qualifier> result {};
            for (glm::length_t i = 0; i < Length; ++i)
                result[i] = average_four(a[i], b[i], c[i], d[i]);
            return result;
        }

    } // namespace detail

    template <typename T>
    [[nodiscard]] std::expected<std::vector<Raster<T>>, RasterError> generate_mipmap(Raster<T> raster)
    {
        if (raster.width() != raster.height() || raster.width() <= 1 || (raster.width() & (raster.width() - 1)) != 0)
            return std::unexpected(RasterError::InvalidMipmapDimensions);

        auto resolution = raster.size();
        std::vector<Raster<T>> mipmap;
        std::size_t level_count = 0;
        for (auto side = resolution.x; side != 0; side >>= 1)
            ++level_count;
        mipmap.reserve(level_count);

        mipmap.push_back(std::move(raster));
        while (resolution.x > 1) {
            resolution /= 2u;
            const Raster<T>& upper = mipmap.back();
            Raster<T> lower(resolution);
            for (unsigned y = 0; y < resolution.y; ++y) {
                for (unsigned x = 0; x < resolution.x; ++x) {
                    lower.pixel({ x, y }) = detail::average_four(upper.pixel({ x * 2, y * 2 }),
                        upper.pixel({ x * 2, y * 2 + 1 }),
                        upper.pixel({ x * 2 + 1, y * 2 }),
                        upper.pixel({ x * 2 + 1, y * 2 + 1 }));
                }
            }
            mipmap.push_back(std::move(lower));
        }
        return mipmap;
    }

    template <typename T>
    [[nodiscard]] Raster<T> resize(const Raster<T>& input, const glm::uvec2 new_size, const T& fill_value)
    {
        Raster<T> output(new_size, fill_value);
        for (unsigned y = 0; y < std::min(input.height(), new_size.y); ++y) {
            for (unsigned x = 0; x < std::min(input.width(), new_size.x); ++x)
                output.pixel({ x, y }) = input.pixel({ x, y });
        }
        return output;
    }

    template <typename T>
    [[nodiscard]] std::expected<Raster<T>, RasterError> concatenate_horizontally(const Raster<T>& left, const Raster<T>& right)
    {
        if (left.height() != right.height())
            return std::unexpected(RasterError::IncompatibleDimensions);

        const auto new_width = static_cast<std::uint64_t>(left.width()) + right.width();
        if (new_width > std::numeric_limits<unsigned>::max())
            return std::unexpected(RasterError::IncompatibleDimensions);

        const glm::uvec2 output_size(static_cast<unsigned>(new_width), left.height());
        Raster<T> output(output_size);
        if (output.size() != output_size)
            return std::unexpected(RasterError::IncompatibleDimensions);

        for (unsigned y = 0; y < left.height(); ++y) {
            auto destination = output.buffer().begin() + static_cast<std::size_t>(y) * output.width();
            destination = std::copy_n(left.buffer().begin() + static_cast<std::size_t>(y) * left.width(), left.width(), destination);
            std::copy_n(right.buffer().begin() + static_cast<std::size_t>(y) * right.width(), right.width(), destination);
        }
        return output;
    }

    template <typename Input, typename Function>
    requires std::invocable<Function, const Input&>
    [[nodiscard]] auto transform(const Raster<Input>& input, Function&& function)
    {
        using Output = std::remove_cvref_t<std::invoke_result_t<Function, const Input&>>;
        static_assert(!std::same_as<Output, bool>, "Use radix::raster::transform_mask for boolean predicates");
        Raster<Output> output(input.size());
        std::ranges::transform(input, output.begin(), std::forward<Function>(function));
        return output;
    }

    template <typename Input, typename Function>
    requires std::invocable<Function, const Input&, glm::uvec2> && (!std::invocable<Function, const Input&>)
    [[nodiscard]] auto transform(const Raster<Input>& input, Function&& function)
    {
        using Output = std::remove_cvref_t<std::invoke_result_t<Function, const Input&, glm::uvec2>>;
        static_assert(!std::same_as<Output, bool>, "Use radix::raster::transform_mask for boolean predicates");
        Raster<Output> output(input.size());
        for (unsigned y = 0; y < input.height(); ++y) {
            for (unsigned x = 0; x < input.width(); ++x) {
                const glm::uvec2 position(x, y);
                output.pixel(position) = std::invoke(function, input.pixel(position), position);
            }
        }
        return output;
    }

    template <typename Input, typename Predicate>
    requires std::predicate<Predicate, const Input&>
    [[nodiscard]] RasterMask transform_mask(const Raster<Input>& input, Predicate&& predicate)
    {
        RasterMask output(input.size());
        std::ranges::transform(input, output.begin(), [&](const Input& value) { return static_cast<std::uint8_t>(std::invoke(predicate, value)); });
        return output;
    }

    template <typename Input, typename Predicate>
    requires std::predicate<Predicate, const Input&, glm::uvec2> && (!std::predicate<Predicate, const Input&>)
    [[nodiscard]] RasterMask transform_mask(const Raster<Input>& input, Predicate&& predicate)
    {
        RasterMask output(input.size());
        for (unsigned y = 0; y < input.height(); ++y) {
            for (unsigned x = 0; x < input.width(); ++x) {
                const glm::uvec2 position(x, y);
                output.pixel(position) = static_cast<std::uint8_t>(std::invoke(predicate, input.pixel(position), position));
            }
        }
        return output;
    }

    template <typename Input, typename Function>
    requires std::invocable<Function, const Input&>
    [[nodiscard]] auto transform(const Raster<Input>& input, const RasterMask& mask, Function&& function)
        -> std::expected<Raster<std::remove_cvref_t<std::invoke_result_t<Function, const Input&>>>, RasterError>
    {
        using Output = std::remove_cvref_t<std::invoke_result_t<Function, const Input&>>;
        static_assert(!std::same_as<Output, bool>, "Use radix::raster::transform_mask for boolean predicates");
        if (input.size() != mask.size())
            return std::unexpected(RasterError::IncompatibleDimensions);

        Raster<Output> output(input.size());
        for (unsigned y = 0; y < input.height(); ++y) {
            for (unsigned x = 0; x < input.width(); ++x) {
                const glm::uvec2 position(x, y);
                if (mask.pixel(position) != 0)
                    output.pixel(position) = std::invoke(function, input.pixel(position));
            }
        }
        return output;
    }

    template <typename Input, typename Function>
    requires std::invocable<Function, const Input&, glm::uvec2> && (!std::invocable<Function, const Input&>)
    [[nodiscard]] auto transform(const Raster<Input>& input, const RasterMask& mask, Function&& function)
        -> std::expected<Raster<std::remove_cvref_t<std::invoke_result_t<Function, const Input&, glm::uvec2>>>, RasterError>
    {
        using Output = std::remove_cvref_t<std::invoke_result_t<Function, const Input&, glm::uvec2>>;
        static_assert(!std::same_as<Output, bool>, "Use radix::raster::transform_mask for boolean predicates");
        if (input.size() != mask.size())
            return std::unexpected(RasterError::IncompatibleDimensions);

        Raster<Output> output(input.size());
        for (unsigned y = 0; y < input.height(); ++y) {
            for (unsigned x = 0; x < input.width(); ++x) {
                const glm::uvec2 position(x, y);
                if (mask.pixel(position) != 0)
                    output.pixel(position) = std::invoke(function, input.pixel(position), position);
            }
        }
        return output;
    }

} // namespace raster
} // namespace radix

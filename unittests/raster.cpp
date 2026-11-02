/*****************************************************************************
 * AlpineMaps.org
 * Copyright (C) 2022 alpinemaps.org
 * Copyright (C) 2022 Adam Celarek
 * Copyright (C) 2024 Gerald Kimmersdorfer
 * Copyright (C) 2024 Lucas Dworschak
 * Copyright (C) 2025 Martin Braunsperger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *****************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <type_traits>

#include <glm/glm.hpp>

#include <radix/raster.h>

namespace {
radix::Raster<int> make_raster(const glm::uvec2 size, const std::initializer_list<int> values)
{
    radix::Raster<int> raster(size, radix::uninitialised);
    std::copy(values.begin(), values.end(), raster.begin());
    return raster;
}
} // namespace

TEST_CASE("radix raster empty, square, and fill interface")
{
    const radix::Raster<std::uint16_t> empty;
    CHECK(empty.buffer().empty());
    CHECK(empty.size() == glm::uvec2(0));
    CHECK(empty.begin() == empty.end());

    radix::Raster<int> square(4);
    square.fill(17);
    CHECK(square.size() == glm::uvec2(4));
    CHECK(std::ranges::all_of(square, [](const int value) { return value == 17; }));
}

TEST_CASE("radix raster allocates uninitialised storage")
{
    const radix::Raster<glm::u8vec3> raster({ 3, 2 }, radix::uninitialised);
    CHECK(raster.size() == glm::uvec2(3, 2));
    CHECK(raster.buffer_length() == 6);
    CHECK(raster.size_in_bytes() == 6 * sizeof(glm::u8vec3));
}

TEST_CASE("radix raster value-initialises sized storage")
{
    static_assert(radix::detail::zero_bytes_are_value_initialised<glm::u8vec3>);
    const radix::Raster<glm::u8vec3> colours({ 5, 3 });
    CHECK(std::ranges::all_of(colours, [](const glm::u8vec3 value) { return value == glm::u8vec3(0); }));

    const radix::Raster<float> heights({ 4, 4 });
    CHECK(std::ranges::all_of(heights, [](const float value) { return value == 0.0f; }));

    using Block = std::array<std::uint8_t, 8>;
    static_assert(radix::detail::zero_bytes_are_value_initialised<Block>);
    const radix::Raster<Block> blocks({ 2, 2 });
    CHECK(std::ranges::all_of(blocks, [](const Block& block) { return block == Block {}; }));

    // Move-only: takes the in-place fallback.
    struct OwningPixel {
        int value;
        std::unique_ptr<int> owner;
    };
    static_assert(!std::is_copy_constructible_v<OwningPixel>);
    const radix::Raster<OwningPixel> owning({ 3, 3 });
    CHECK(owning.buffer_length() == 9);
    CHECK(std::ranges::all_of(owning, [](const OwningPixel& pixel) { return pixel.value == 0 && pixel.owner == nullptr; }));

    // Trivial, but all-zero bytes are not a null data member pointer on the Itanium C++ ABI.
    struct Member {
        int value;
    };
    struct MemberPointerPixel {
        int Member::* member;
    };
    static_assert(std::is_trivially_default_constructible_v<MemberPointerPixel> && std::is_trivially_copyable_v<MemberPointerPixel>);
    const radix::Raster<MemberPointerPixel> member_pointers({ 2, 2 });
    CHECK(std::ranges::all_of(member_pointers, [](const MemberPointerPixel& pixel) { return pixel.member == nullptr; }));

    // Not assignable: needs construction in place.
    struct ConstantPixel {
        const int value = 7;
    };
    static_assert(!std::is_copy_assignable_v<ConstantPixel>);
    const radix::Raster<ConstantPixel> constants({ 3, 2 });
    CHECK(std::ranges::all_of(constants, [](const ConstantPixel& pixel) { return pixel.value == 7; }));
}

TEST_CASE("radix raster stores rectangular row-major data")
{
    radix::Raster<int> raster({ 3, 2 });
    std::iota(raster.begin(), raster.end(), 0);

    CHECK(raster.size() == glm::uvec2(3, 2));
    CHECK(raster.buffer_length() == 6);
    CHECK(raster.pixel({ 0, 0 }) == 0);
    CHECK(raster.pixel({ 2, 0 }) == 2);
    CHECK(raster.pixel({ 0, 1 }) == 3);
    CHECK(raster.pixel({ 2, 1 }) == 5);
}

TEST_CASE("radix raster exposes bounded element and byte spans")
{
    radix::Raster<std::uint16_t> raster({ 2, 3 }, 0);
    static_assert(std::same_as<decltype(raster.buffer()), std::span<std::uint16_t>>);
    static_assert(std::same_as<decltype(std::as_const(raster).buffer()), std::span<const std::uint16_t>>);
    static_assert(std::same_as<decltype(raster.bytes()), std::span<std::byte>>);
    static_assert(std::same_as<decltype(std::as_const(raster).bytes()), std::span<const std::byte>>);

    raster.buffer()[4] = 42;
    CHECK(raster.pixel({ 0, 2 }) == 42);
    CHECK(raster.bytes().size() == raster.buffer_length() * sizeof(std::uint16_t));
    CHECK(raster.size_in_bytes() == raster.bytes().size());
}

TEST_CASE("radix raster resizes by cropping and padding")
{
    radix::Raster<int> input({ 2, 2 });
    std::iota(input.begin(), input.end(), 1);

    const auto enlarged = radix::raster::resize(input, { 3, 4 }, -1);
    CHECK(enlarged.pixel({ 0, 0 }) == 1);
    CHECK(enlarged.pixel({ 1, 1 }) == 4);
    CHECK(enlarged.pixel({ 2, 0 }) == -1);
    CHECK(enlarged.pixel({ 0, 3 }) == -1);

    const auto cropped = radix::raster::resize(input, { 1, 1 }, -1);
    CHECK(cropped.size() == glm::uvec2(1, 1));
    CHECK(cropped.pixel({ 0, 0 }) == 1);
}

TEST_CASE("radix raster concatenates without exposing its vector")
{
    const radix::Raster<int> top({ 2, 1 }, 1);
    const radix::Raster<int> bottom({ 2, 2 }, 2);
    const auto vertical_result = radix::raster::concatenate_vertically(top, bottom);
    REQUIRE(vertical_result.has_value());
    CHECK(vertical_result->size() == glm::uvec2(2, 3));
    CHECK(vertical_result->pixel({ 1, 0 }) == 1);
    CHECK(vertical_result->pixel({ 1, 2 }) == 2);

    const radix::Raster<int> right({ 1, 3 }, 3);
    const auto combined_result = radix::raster::concatenate_horizontally(*vertical_result, right);
    REQUIRE(combined_result.has_value());
    const auto& combined = *combined_result;
    CHECK(combined.size() == glm::uvec2(3, 3));
    CHECK(combined.pixel({ 1, 2 }) == 2);
    CHECK(combined.pixel({ 2, 2 }) == 3);
}

TEST_CASE("radix raster concatenation reports incompatible dimensions")
{
    const radix::Raster<int> raster({ 2, 2 }, 1);
    const radix::Raster<int> wrong_width({ 3, 1 }, 2);
    const auto vertical_result = radix::raster::concatenate_vertically(raster, wrong_width);
    CHECK_FALSE(vertical_result.has_value());
    CHECK(vertical_result.error() == radix::RasterError::IncompatibleDimensions);

    const std::array incompatible_range { raster, wrong_width };
    const auto range_result = radix::raster::concatenate_vertically(std::span<const radix::Raster<int>>(incompatible_range));
    CHECK_FALSE(range_result.has_value());
    CHECK(range_result.error() == radix::RasterError::IncompatibleDimensions);

    const radix::Raster<int> wrong_height({ 1, 3 }, 2);
    const auto horizontal_result = radix::raster::concatenate_horizontally(raster, wrong_height);
    CHECK_FALSE(horizontal_result.has_value());
    CHECK(horizontal_result.error() == radix::RasterError::IncompatibleDimensions);
}

TEST_CASE("radix raster vertically concatenates a range")
{
    const std::array rasters {
        make_raster({ 2, 1 }, { 1, 2 }),
        make_raster({ 2, 2 }, { 3, 4, 5, 6 }),
        make_raster({ 2, 1 }, { 7, 8 }),
    };

    const auto result = radix::raster::concatenate_vertically(std::span<const radix::Raster<int>>(rasters));

    REQUIRE(result.has_value());
    CHECK(result->size() == glm::uvec2(2, 4));
    CHECK(std::ranges::equal(result->buffer(), std::array { 1, 2, 3, 4, 5, 6, 7, 8 }));

    const std::span<const radix::Raster<int>> empty;
    const auto empty_result = radix::raster::concatenate_vertically(empty);
    REQUIRE(empty_result.has_value());
    CHECK(empty_result->size() == glm::uvec2(0, 0));

    const std::array excessive_height {
        radix::Raster<int>({ 0, std::numeric_limits<unsigned>::max() }),
        radix::Raster<int>({ 0, 1 }),
    };
    const auto excessive_height_result = radix::raster::concatenate_vertically(std::span<const radix::Raster<int>>(excessive_height));
    CHECK_FALSE(excessive_height_result.has_value());
    CHECK(excessive_height_result.error() == radix::RasterError::IncompatibleDimensions);
}

TEST_CASE("radix raster generates overflow-safe integral mipmaps")
{
    radix::Raster<glm::u16vec2> input({ 2, 2 }, glm::u16vec2(65535, 60000));
    const auto result = radix::raster::generate_mipmap(input);
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 2);
    CHECK(result->at(0).size() == glm::uvec2(2, 2));
    CHECK(result->at(1).size() == glm::uvec2(1, 1));
    CHECK(result->at(1).pixel({ 0, 0 }) == glm::u16vec2(65535, 60000));
}

TEST_CASE("radix raster generates component-wise mipmaps")
{
    radix::Raster<glm::u8vec4> input({ 4, 4 }, glm::u8vec4(0));
    input.pixel({ 0, 0 }) = { 100, 0, 100, 0 };
    input.pixel({ 0, 1 }) = { 100, 0, 100, 150 };
    input.pixel({ 1, 0 }) = { 200, 200, 100, 250 };
    input.pixel({ 1, 1 }) = { 0, 200, 100, 0 };

    input.pixel({ 0, 2 }) = input.pixel({ 0, 3 }) = input.pixel({ 1, 2 }) = input.pixel({ 1, 3 }) = { 10, 20, 30, 40 };
    input.pixel({ 2, 0 }) = input.pixel({ 2, 1 }) = input.pixel({ 3, 0 }) = input.pixel({ 3, 1 }) = { 200, 250, 0, 255 };
    input.pixel({ 2, 2 }) = { 0, 0, 0, 1 };
    input.pixel({ 2, 3 }) = { 3, 0, 0, 1 };
    input.pixel({ 3, 2 }) = { 0, 0, 1, 1 };
    input.pixel({ 3, 3 }) = { 0, 1, 2, 1 };

    const auto result = radix::raster::generate_mipmap(input);
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 3);
    CHECK(result->at(1).pixel({ 0, 0 }) == glm::u8vec4(100, 100, 100, 100));
    CHECK(result->at(1).pixel({ 0, 1 }) == glm::u8vec4(10, 20, 30, 40));
    CHECK(result->at(1).pixel({ 1, 0 }) == glm::u8vec4(200, 250, 0, 255));
    CHECK(result->at(1).pixel({ 1, 1 }) == glm::u8vec4(0, 0, 0, 1));
    CHECK(result->at(2).pixel({ 0, 0 }) == glm::u8vec4(77, 92, 32, 99));
}

TEST_CASE("radix raster rejects unsupported mipmap dimensions")
{
    const auto rectangular = radix::raster::generate_mipmap(radix::Raster<int>({ 4, 2 }));
    CHECK_FALSE(rectangular.has_value());
    CHECK(rectangular.error() == radix::RasterError::InvalidMipmapDimensions);

    const auto non_power_of_two = radix::raster::generate_mipmap(radix::Raster<int>({ 3, 3 }));
    CHECK_FALSE(non_power_of_two.has_value());
}

TEST_CASE("radix raster transforms values with coordinates")
{
    radix::Raster<int> input({ 3, 2 }, 10);
    const auto output
        = radix::raster::transform(input, [](const int value, const glm::uvec2 position) { return value + static_cast<int>(position.x + position.y * 10); });

    CHECK(output.pixel({ 0, 0 }) == 10);
    CHECK(output.pixel({ 2, 1 }) == 22);
}

TEST_CASE("radix RasterMask uses normalized contiguous bytes")
{
    static_assert(std::same_as<radix::RasterMask, radix::Raster<std::uint8_t>>);
    radix::Raster<int> input({ 3, 1 });
    input.buffer()[0] = -1;
    input.buffer()[1] = 0;
    input.buffer()[2] = 4;

    const auto mask = radix::raster::transform(input, [](const int value) { return value > 0; });
    static_assert(std::same_as<decltype(mask), const radix::RasterMask>);
    CHECK(mask.buffer()[0] == 0);
    CHECK(mask.buffer()[1] == 0);
    CHECK(mask.buffer()[2] == 1);
    CHECK(mask.bytes().size() == 3);

    const auto values = radix::raster::transform(input, [](const int value) { return value; });
    static_assert(std::same_as<decltype(values), const radix::Raster<int>>);
    CHECK(std::ranges::equal(values.buffer(), input.buffer()));

    const auto coordinate_mask = radix::raster::transform(input, [](const int, const glm::uvec2 position) { return position.x == 1; });
    static_assert(std::same_as<decltype(coordinate_mask), const radix::RasterMask>);
    CHECK(std::ranges::equal(coordinate_mask.buffer(), std::array<std::uint8_t, 3> { 0, 1, 0 }));
}

TEST_CASE("radix raster masked transform validates dimensions")
{
    const radix::Raster<int> input({ 2, 2 }, 4);
    const radix::RasterMask wrong_mask({ 1, 2 }, 1);
    const auto invalid = radix::raster::transform(input, wrong_mask, [](const int value) { return value * 2; });
    CHECK_FALSE(invalid.has_value());
    CHECK(invalid.error() == radix::RasterError::IncompatibleDimensions);

    radix::RasterMask mask({ 2, 2 }, 0);
    mask.pixel({ 1, 0 }) = 1;
    const auto valid = radix::raster::transform(input, mask, [](const int value) { return value * 2; });
    REQUIRE(valid.has_value());
    CHECK(valid->pixel({ 0, 0 }) == 0);
    CHECK(valid->pixel({ 1, 0 }) == 8);

    const auto boolean = radix::raster::transform(input, mask, [](const int value) { return value > 0; });
    static_assert(std::same_as<decltype(boolean), const std::expected<radix::RasterMask, radix::RasterError>>);
    REQUIRE(boolean.has_value());
    CHECK(boolean->pixel({ 0, 0 }) == 0);
    CHECK(boolean->pixel({ 1, 0 }) == 1);
}

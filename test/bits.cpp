/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2020, Kuba Sejdak <kuba.sejdak@gmail.com>
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// 1. Redistributions of source code must retain the above copyright notice, this
///    list of conditions and the following disclaimer.
///
/// 2. Redistributions in binary form must reproduce the above copyright notice,
///    this list of conditions and the following disclaimer in the documentation
///    and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
/// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
/// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
/////////////////////////////////////////////////////////////////////////////////////

#include <utils/bits.hpp>

#include <catch2/catch.hpp>

#include <cmath>
#include <limits>

TEST_CASE("Check power of 2 detection", "[unit][bits]")
{
    constexpr std::uint32_t cIterationsCount = 1'000;
    for (std::uint32_t i = 0; i < cIterationsCount; ++i) {
        std::uint32_t valueA = std::log2(i);
        double valueB = std::pow(2, valueA);
        bool powerOf2 = double(i) == valueB;

        REQUIRE(utils::isPowerOf2(i) == powerOf2);
    }
}

TEST_CASE("Check conversions to little endian", "[unit][bits]")
{
    SECTION("16bit conversions")
    {
        for (std::uint16_t i = 0; i < std::numeric_limits<std::uint16_t>::max(); ++i)
            REQUIRE(utils::toLittleEndian(i) == i);
    }

    SECTION("32bit conversions")
    {
        constexpr std::uint32_t cIterationsCount = 1'000;
        for (std::uint32_t i = 0; i < cIterationsCount; ++i)
            REQUIRE(utils::toLittleEndian(i) == i);
    }

    SECTION("64bit conversions")
    {
        constexpr std::uint64_t cIterationsCount = 1'000;
        for (std::uint64_t i = 0; i < cIterationsCount; ++i)
            REQUIRE(utils::toLittleEndian(i) == i);
    }
}

TEST_CASE("Check conversions to big endian", "[unit][bits]")
{
    SECTION("16bit conversions")
    {
        for (std::uint16_t i = 0; i < std::numeric_limits<std::uint16_t>::max(); ++i) {
            std::uint16_t expected = ((i & 0x00ff) << 8) | ((i & 0xff00) >> 8); // NOLINT
            REQUIRE(utils::toBigEndian(i) == expected);
        }
    }

    SECTION("32bit conversions")
    {
        constexpr std::uint32_t cIterationsCount = 1'000;
        for (std::uint32_t i = 0; i < cIterationsCount; ++i) {
            // clang-format off
            std::uint32_t expected = ((i & 0x000000ff) << 24)  // NOLINT
                                   | ((i & 0x0000ff00) << 8)   // NOLINT
                                   | ((i & 0x00ff0000) >> 8)   // NOLINT
                                   | ((i & 0xff000000) >> 24); // NOLINT
            // clang-format on
            REQUIRE(utils::toBigEndian(i) == expected);
        }
    }

    SECTION("64bit conversions")
    {
        constexpr std::uint64_t cIterationsCount = 1'000;
        for (std::uint64_t i = 0; i < cIterationsCount; ++i) {
            // clang-format off
            std::uint64_t expected = ((i & 0x00000000000000ffUL) << 56)  // NOLINT
                                   | ((i & 0x000000000000ff00UL) << 40)  // NOLINT
                                   | ((i & 0x0000000000ff0000UL) << 24)  // NOLINT
                                   | ((i & 0x00000000ff000000UL) << 8)   // NOLINT
                                   | ((i & 0x000000ff00000000UL) >> 8)   // NOLINT
                                   | ((i & 0x0000ff0000000000UL) >> 24)  // NOLINT
                                   | ((i & 0x00ff000000000000UL) >> 40)  // NOLINT
                                   | ((i & 0xff00000000000000UL) >> 56); // NOLINT
            // clang-format on
            REQUIRE(utils::toBigEndian(i) == expected);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2023, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <utils/bits/endianness.hpp>
#include <utils/bits/numerics.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>
#include <limits>

TEST_CASE("1. Check power of 2 detection", "[unit][bits]")
{
    constexpr std::uint32_t cIterationsCount = 1'000;
    for (std::uint32_t i = 1; i < cIterationsCount; ++i) {
        auto valueA = std::uint32_t(std::log2(i));
        double valueB = std::pow(2, valueA);
        bool powerOf2 = double(i) == valueB;

        REQUIRE(utils::bits::isPowerOf2(i) == powerOf2);
    }
}

TEST_CASE("2. Check conversions to little endian", "[unit][bits]")
{
    SECTION("2.1. 16bit conversions")
    {
        for (std::uint16_t i = 0; i < std::numeric_limits<std::uint16_t>::max(); ++i)
            REQUIRE(utils::bits::toLittleEndian(i) == i);
    }

    SECTION("2.2. 32bit conversions")
    {
        constexpr std::uint32_t cIterationsCount = 1'000;
        for (std::uint32_t i = 0; i < cIterationsCount; ++i)
            REQUIRE(utils::bits::toLittleEndian(i) == i);
    }

    SECTION("2.3. 64bit conversions")
    {
        constexpr std::uint64_t cIterationsCount = 1'000;
        for (std::uint64_t i = 0; i < cIterationsCount; ++i)
            REQUIRE(utils::bits::toLittleEndian(i) == i);
    }
}

TEST_CASE("3. Check conversions to big endian", "[unit][bits]")
{
    SECTION("3.1. 16bit conversions")
    {
        for (std::uint16_t i = 0; i < std::numeric_limits<std::uint16_t>::max(); ++i) {
            std::uint16_t expected = ((i & 0x00ff) << 8) | ((i & 0xff00) >> 8); // NOLINT
            REQUIRE(utils::bits::toBigEndian(i) == expected);
        }
    }

    SECTION("3.2. 32bit conversions")
    {
        constexpr std::uint32_t cIterationsCount = 1'000;
        for (std::uint32_t i = 0; i < cIterationsCount; ++i) {
            std::uint32_t expected = ((i & 0x000000ff) << 24)  // NOLINT
                                   | ((i & 0x0000ff00) << 8)   // NOLINT
                                   | ((i & 0x00ff0000) >> 8)   // NOLINT
                                   | ((i & 0xff000000) >> 24); // NOLINT
            REQUIRE(utils::bits::toBigEndian(i) == expected);
        }
    }

    SECTION("3.3 64bit conversions")
    {
        constexpr std::uint64_t cIterationsCount = 1'000;
        for (std::uint64_t i = 0; i < cIterationsCount; ++i) {
            std::uint64_t expected = ((i & 0x00000000000000ffUL) << 56)  // NOLINT
                                   | ((i & 0x000000000000ff00UL) << 40)  // NOLINT
                                   | ((i & 0x0000000000ff0000UL) << 24)  // NOLINT
                                   | ((i & 0x00000000ff000000UL) << 8)   // NOLINT
                                   | ((i & 0x000000ff00000000UL) >> 8)   // NOLINT
                                   | ((i & 0x0000ff0000000000UL) >> 24)  // NOLINT
                                   | ((i & 0x00ff000000000000UL) >> 40)  // NOLINT
                                   | ((i & 0xff00000000000000UL) >> 56); // NOLINT
            REQUIRE(utils::bits::toBigEndian(i) == expected);
        }
    }
}

TEST_CASE("4. Conversions from integral to byte array", "[unit][bits]")
{
    SECTION("4.1. std::uint8_t")
    {
        for (std::uint8_t value = 0; value < std::numeric_limits<std::uint8_t>::max(); ++value) {
            auto array = utils::bits::toBytesArray(value);
            REQUIRE(array.size() == sizeof(value));
            REQUIRE(array[0] == value);
        }
    }

    SECTION("4.2. std::uint16_t")
    {
        for (std::uint16_t value = 0; value < std::numeric_limits<std::uint16_t>::max(); ++value) {
            auto array = utils::bits::toBytesArray(value);
            REQUIRE(array.size() == sizeof(value));
            REQUIRE(array[0] == (value & 0x00ff));         // NOLINT
            REQUIRE(array[1] == ((value & 0xff00) >> 8U)); // NOLINT
        }
    }

    SECTION("4.3. std::uint32_t")
    {
        constexpr std::uint32_t cIterationsCount = 1'000;
        for (std::uint32_t value = 0; value < cIterationsCount; ++value) {
            auto array = utils::bits::toBytesArray(value);
            REQUIRE(array.size() == sizeof(value));
            REQUIRE(array[0] == (value & 0x000000ff));          // NOLINT
            REQUIRE(array[1] == ((value & 0x0000ff00) >> 8U));  // NOLINT
            REQUIRE(array[2] == ((value & 0x00ff0000) >> 16U)); // NOLINT
            REQUIRE(array[3] == ((value & 0xff000000) >> 24U)); // NOLINT
        }
    }

    SECTION("4.4. std::uint64_t")
    {
        constexpr std::uint64_t cIterationsCount = 1'000;
        for (std::uint64_t value = 0; value < cIterationsCount; ++value) {
            auto array = utils::bits::toBytesArray(value);
            REQUIRE(array.size() == sizeof(value));
            REQUIRE(array[0] == (value & 0x00000000000000ff));          // NOLINT
            REQUIRE(array[1] == ((value & 0x000000000000ff00) >> 8U));  // NOLINT
            REQUIRE(array[2] == ((value & 0x0000000000ff0000) >> 16U)); // NOLINT
            REQUIRE(array[3] == ((value & 0x00000000ff000000) >> 24U)); // NOLINT
            REQUIRE(array[4] == ((value & 0x000000ff00000000) >> 32U)); // NOLINT
            REQUIRE(array[5] == ((value & 0x0000ff0000000000) >> 40U)); // NOLINT
            REQUIRE(array[6] == ((value & 0x00ff000000000000) >> 48U)); // NOLINT
            REQUIRE(array[7] == ((value & 0xff00000000000000) >> 56U)); // NOLINT
        }
    }
}

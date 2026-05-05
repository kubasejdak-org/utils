/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2020 Kuba Sejdak (kuba.sejdak@gmail.com)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///
/////////////////////////////////////////////////////////////////////////////////////

#include <utils/bits/endianness.hpp>
#include <utils/bits/numerics.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

TEMPLATE_TEST_CASE("1. Check power of 2 detection",
                   "[unit][bits]",
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t)
{
    CHECK_FALSE(utils::bits::isPowerOf2(TestType{0}));

    constexpr auto cIterationsCount
        = static_cast<TestType>(std::min<std::uint64_t>(std::numeric_limits<TestType>::max(), 1000));
    for (TestType i = 1; i < cIterationsCount; ++i) {
        auto valueA = TestType(std::log2(i));
        double valueB = std::pow(2, valueA);
        bool powerOf2 = double(i) == valueB;
        CHECK(utils::bits::isPowerOf2(i) == powerOf2);
    }
}

TEST_CASE("2. Check conversions to little endian", "[unit][bits]")
{
    SECTION("2.1. 16bit conversions")
    {
        for (std::uint16_t i = 0; i < std::numeric_limits<std::uint16_t>::max(); ++i)
            CHECK(utils::bits::toLittleEndian(i) == i);
    }

    SECTION("2.2. 32bit conversions")
    {
        constexpr std::uint32_t cIterationsCount = 1000;
        for (std::uint32_t i = 0; i < cIterationsCount; ++i)
            CHECK(utils::bits::toLittleEndian(i) == i);
    }

    SECTION("2.3. 64bit conversions")
    {
        constexpr std::uint64_t cIterationsCount = 1000;
        for (std::uint64_t i = 0; i < cIterationsCount; ++i)
            CHECK(utils::bits::toLittleEndian(i) == i);
    }
}

TEST_CASE("3. Check conversions to big endian", "[unit][bits]")
{
    SECTION("3.1. 16bit conversions")
    {
        for (std::uint16_t i = 0; i < std::numeric_limits<std::uint16_t>::max(); ++i) {
            std::uint16_t expected = ((i & 0x00ff) << 8) | ((i & 0xff00) >> 8);
            CHECK(utils::bits::toBigEndian(i) == expected);
        }
    }

    SECTION("3.2. 32bit conversions")
    {
        constexpr std::uint32_t cIterationsCount = 1000;
        for (std::uint32_t i = 0; i < cIterationsCount; ++i) {
            std::uint32_t expected = ((i & 0x000000ff) << 24) | ((i & 0x0000ff00) << 8) | ((i & 0x00ff0000) >> 8)
                                   | ((i & 0xff000000) >> 24);
            CHECK(utils::bits::toBigEndian(i) == expected);
        }
    }

    SECTION("3.3 64bit conversions")
    {
        constexpr std::uint64_t cIterationsCount = 1000;
        for (std::uint64_t i = 0; i < cIterationsCount; ++i) {
            std::uint64_t expected = ((i & 0x00000000000000ffUL) << 56) | ((i & 0x000000000000ff00UL) << 40)
                                   | ((i & 0x0000000000ff0000UL) << 24) | ((i & 0x00000000ff000000UL) << 8)
                                   | ((i & 0x000000ff00000000UL) >> 8) | ((i & 0x0000ff0000000000UL) >> 24)
                                   | ((i & 0x00ff000000000000UL) >> 40) | ((i & 0xff00000000000000UL) >> 56);
            CHECK(utils::bits::toBigEndian(i) == expected);
        }
    }
}

TEST_CASE("4. Conversions from integral to byte array", "[unit][bits]")
{
    SECTION("4.1. std::uint8_t")
    {
        for (std::uint8_t value = 0; value < std::numeric_limits<std::uint8_t>::max(); ++value) {
            auto array = utils::bits::toBytesArray(value);
            CHECK(array.size() == sizeof(value));
            CHECK(array[0] == value);
        }
    }

    SECTION("4.2. std::uint16_t")
    {
        for (std::uint16_t value = 0; value < std::numeric_limits<std::uint16_t>::max(); ++value) {
            auto array = utils::bits::toBytesArray(value);
            CHECK(array.size() == sizeof(value));
            CHECK(array[0] == (value & 0x00ff));
            CHECK(array[1] == ((value & 0xff00) >> 8U));
        }
    }

    SECTION("4.3. std::uint32_t")
    {
        constexpr std::uint32_t cIterationsCount = 1000;
        for (std::uint32_t value = 0; value < cIterationsCount; ++value) {
            auto array = utils::bits::toBytesArray(value);
            CHECK(array.size() == sizeof(value));
            CHECK(array[0] == (value & 0x000000ff));
            CHECK(array[1] == ((value & 0x0000ff00) >> 8U));
            CHECK(array[2] == ((value & 0x00ff0000) >> 16U));
            CHECK(array[3] == ((value & 0xff000000) >> 24U));
        }
    }

    SECTION("4.4. std::uint64_t")
    {
        constexpr std::uint64_t cIterationsCount = 1000;
        for (std::uint64_t value = 0; value < cIterationsCount; ++value) {
            auto array = utils::bits::toBytesArray(value);
            CHECK(array.size() == sizeof(value));
            CHECK(array[0] == (value & 0x00000000000000ff));
            CHECK(array[1] == ((value & 0x000000000000ff00) >> 8U));
            CHECK(array[2] == ((value & 0x0000000000ff0000) >> 16U));
            CHECK(array[3] == ((value & 0x00000000ff000000) >> 24U));
            CHECK(array[4] == ((value & 0x000000ff00000000) >> 32U));
            CHECK(array[5] == ((value & 0x0000ff0000000000) >> 40U));
            CHECK(array[6] == ((value & 0x00ff000000000000) >> 48U));
            CHECK(array[7] == ((value & 0xff00000000000000) >> 56U));
        }
    }
}

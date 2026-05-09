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

#pragma once

#include <array>
#include <bit>
#include <concepts>
#include <cstdint>

namespace utils::bits {

/// Checks if current system uses big endian notation.
/// @return Flag indicating if current system uses big endian notation.
/// @retval true        Current system uses big endian notation.
/// @retval false       Current system uses little endian notation.
constexpr bool isBigEndian()
{
    return std::endian::native == std::endian::big;
}

/// Changes order of bytes in given integer value to the opposite endianness.
/// @tparam T           Type of the value to be changed.
/// @param value        Value to be changed.
/// @return Value with the opposite endianness.
template <std::integral T>
constexpr T changeEndianness(T value)
{
    return std::byteswap(value);
}

/// Changes order of bytes in given integer value to big endian.
/// @tparam T           Type of the value to be changed.
/// @param value        Value to be changed.
/// @return Value with the big endian notation.
template <std::integral T>
constexpr auto toBigEndian(T value)
{
    return isBigEndian() ? value : changeEndianness(value);
}

/// Changes order of bytes in given integer value to little endian.
/// @tparam T           Type of the value to be changed.
/// @param value        Value to be changed.
/// @return Value with the little endian notation.
template <std::integral T>
constexpr auto toLittleEndian(T value)
{
    return isBigEndian() ? changeEndianness(value) : value;
}

/// Converts given integral value into array of bytes.
/// @tparam T           Type of the value to be converted.
/// @param value        Value to be converted.
/// @return Array of bytes created from the given integral value.
template <std::integral T>
constexpr auto toBytesArray(T value)
{
    return std::bit_cast<std::array<std::uint8_t, sizeof(T)>>(value);
}

} // namespace utils::bits

/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2021, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace utils {

/// Checks if the given value is a power of 2.
/// @param value        Value to be checked.
/// @return Flag indicating if given value is a power of 2.
/// @retval true        Given value is a power of 2.
/// @retval false       Given value is not a power of 2.
constexpr inline bool isPowerOf2(std::uint32_t value)
{
    return (value > 0 && ((value & (value - 1)) == 0));
}

/// Checks if current system uses big endian notation.
/// @return Flag indicating if current system uses big endian notation.
/// @retval true        Current system uses big endian notation.
/// @retval false       Current system uses little endian notation.
inline bool isBigEndian()
{
    constexpr std::uint32_t cValue = 0x01020304;

    union {
        std::uint32_t integer;
        std::array<std::uint8_t, 4> bytes{};
    } helper = {cValue};

    return helper.bytes[0] == 1; // NOLINT(cppcoreguidelines-pro-type-union-access)
}

/// Changes order of bytes in given integer value to the opposite endianness.
/// @tparam T           Type of the value to be changed.
/// @param value        Value to be changed.
/// @return Value with the opposite endianness.
template <typename T>
constexpr inline auto changeEndianness(T value)
{
    static_assert(std::is_integral_v<T>, "T is not integral");

    constexpr auto cSize = sizeof(T);

    if constexpr (cSize == 2)
        return __builtin_bswap16(value);

    if constexpr (sizeof(value) == 4)
        return __builtin_bswap32(value);

    if constexpr (sizeof(value) == 8) // NOLINT
        return __builtin_bswap64(value);

    return value;
}

/// Changes order of bytes in given integer value to big endian.
/// @tparam T           Type of the value to be changed.
/// @param value        Value to be changed.
/// @return Value with the big endian notation.
template <typename T>
inline auto toBigEndian(T value)
{
    return isBigEndian() ? value : changeEndianness(value);
}

/// Changes order of bytes in given integer value to little endian.
/// @tparam T           Type of the value to be changed.
/// @param value        Value to be changed.
/// @return Value with the little endian notation.
template <typename T>
inline auto toLittleEndian(T value)
{
    return isBigEndian() ? changeEndianness(value) : value;
}

/// Converts given integral value into array of bytes.
/// @tparam T           Type of the value to be converted.
/// @param value        Value to be converted.
/// @return Array of bytes created from the given integral value.
template <typename T>
constexpr inline auto toBytesArray(T value)
{
    static_assert(std::is_integral_v<T>, "T is not integral");

    std::array<std::uint8_t, sizeof(T)> bytes{};
    std::memcpy(bytes.data(), &value, sizeof(T));
    return bytes;
}

} // namespace utils

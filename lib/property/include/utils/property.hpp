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

#pragma once

namespace utils {

/// Base property type.
/// @tparam Ts              Type tags used to specialize generic Property type.
/// @note This empty type is used only for user specialization.
template <typename... Ts>
struct Property {};

/// Helper alias for user-defined property type.
/// @tparam Ts              Type tags used to specialize generic Property type.
template <typename... Ts>
using PropertyType = typename Property<Ts...>::type;

/// Helper constant representing user-defined property value.
/// @tparam Ts              Type tags used to specialize generic Property type.
template <typename... Ts>
inline constexpr auto cPropertyValue = Property<Ts...>::value;

} // namespace utils

/// Registers user-defined type property which depends on T1 type.
/// @tparam T1              First type on which depends given type property.
/// @tparam property        Type of the user-defined type property.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_PROPERTY_TYPE(T1, property)                                                                                \
    struct T1;                                                                                                         \
    struct property;                                                                                                   \
    template <>                                                                                                        \
    struct utils::Property<T1> {                                                                                       \
        using type = property;                                                                                         \
    }

/// Registers user-defined type property which depends on T1 and T2 types.
/// @tparam T1              First type on which depends given type property.
/// @tparam T2              Second type on which depends given type property.
/// @tparam property        Type of the user-defined type property.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_PROPERTY_TYPE_2(T1, T2, property)                                                                          \
    struct T1;                                                                                                         \
    struct T2;                                                                                                         \
    struct property;                                                                                                   \
    template <>                                                                                                        \
    struct utils::Property<T1, T2> {                                                                                   \
        using type = property;                                                                                         \
    }

/// Registers user-defined type property which depends on T1, T2 and T3 types.
/// @tparam T1              First type on which depends given type property.
/// @tparam T2              Second type on which depends given type property.
/// @tparam T3              Third type on which depends given type property.
/// @tparam property        Type of the user-defined type property.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_PROPERTY_TYPE_3(T1, T2, T3, property)                                                                      \
    struct T1;                                                                                                         \
    struct T2;                                                                                                         \
    struct T3;                                                                                                         \
    struct property;                                                                                                   \
    template <>                                                                                                        \
    struct utils::Property<T1, T2, T3> {                                                                               \
        using type = property;                                                                                         \
    }

/// Registers user-defined value property which depends on T1 type.
/// @tparam T1              First type on which depends given value property.
/// @param property         Value of the user-defined value property.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_PROPERTY(T1, property)                                                                                     \
    struct T1;                                                                                                         \
    template <>                                                                                                        \
    struct utils::Property<T1> {                                                                                       \
        static constexpr decltype(property) value = property;                                                          \
    }

/// Registers user-defined value property which depends on T1 and T2 types.
/// @tparam T1              First type on which depends given value property.
/// @tparam T2              Second type on which depends given value property.
/// @param property         Value of the user-defined value property.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_PROPERTY_2(T1, T2, property)                                                                               \
    struct T1;                                                                                                         \
    struct T2;                                                                                                         \
    template <>                                                                                                        \
    struct utils::Property<T1, T2> {                                                                                   \
        static constexpr decltype(property) value = property;                                                          \
    }

/// Registers user-defined type property which depends on T1, T2 and T3 types.
/// @tparam T1              First type on which depends given value property.
/// @tparam T2              Second type on which depends given value property.
/// @tparam T3              Third type on which depends given value property.
/// @param property         Value of the user-defined value property.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_PROPERTY_3(T1, T2, T3, property)                                                                           \
    struct T1;                                                                                                         \
    struct T2;                                                                                                         \
    struct T3;                                                                                                         \
    template <>                                                                                                        \
    struct utils::Property<T1, T2, T3> {                                                                               \
        static constexpr decltype(property) value = property;                                                          \
    }

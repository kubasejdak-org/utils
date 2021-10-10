/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2021-2021, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <cassert>
#include <optional>
#include <system_error>
#include <type_traits>
#include <utility>

namespace utils::types {

template <typename T>
class Result {
public:
    Result() = default;

    Result(const T& value, std::error_code error = {}) // NOLINT
        : m_value(value)
        , m_error(error)
    {}

    template <typename ErrorEnum>
    Result(ErrorEnum error) // NOLINT
        : Result(std::error_code{error})
    {
        static_assert(std::is_error_code_enum_v<ErrorEnum>, "ErrorEnum is not an error code enum");
    }

    Result(T&& value, std::error_code error = {}) // NOLINT
        : m_value(std::move(value))
        , m_error(error)
    {}

    Result(std::error_code error) // NOLINT
        : m_error(error)
    {}

    template <typename OtherT>
    Result(const Result<OtherT>& other) // NOLINT
        : m_error(other.error())
    {
        assert(!other);
    }

    Result(const Result& other) = default;

    Result(Result&& other) noexcept = default;

    ~Result() = default;

    Result& operator=(const Result& other) = default;

    Result& operator=(Result&& other) noexcept
    {
        if (&other != this) {
            m_value = std::move(other.m_value);
            m_error = std::exchange(other.m_error, {});
        }

        return *this;
    }

    void setValue(const T& value) { m_value = value; }

    template <typename ErrorEnum>
    void setError(ErrorEnum error)
    {
        static_assert(std::is_error_code_enum_v<ErrorEnum>, "ErrorEnum is not an error code enum");
        m_error = error;
    }

    void setError(std::error_code error) { m_error = error; }

    [[nodiscard]] T value() const
    {
        assert(m_value.has_value());
        return *m_value;
    }

    [[nodiscard]] T valueOr(const T& fallbackValue) const { return m_value.value_or(fallbackValue); }

    [[nodiscard]] std::optional<T> optionalValue() const { return m_value; }

    [[nodiscard]] std::error_code error() const { return m_error; }

    T operator*() const { return value(); }

    operator T() const { return value(); } // NOLINT

    operator std::optional<T>() const { return optionalValue(); } // NOLINT

    operator std::error_code() const { return error(); } // NOLINT

    explicit operator bool() const { return m_value.has_value(); }

    template <std::size_t cIndex>
    std::tuple_element_t<cIndex, Result> get() const
    {
        if constexpr (cIndex == 0)
            return optionalValue();

        if constexpr (cIndex == 1)
            return error();
    }

private:
    std::optional<T> m_value;
    std::error_code m_error;
};

} // namespace utils::types

template <typename T>
using Result = utils::types::Result<T>;

namespace std {

template <typename T>
struct tuple_size<Result<T>> : integral_constant<size_t, 2> {};

template <typename T>
struct tuple_element<0, Result<T>> {
    using type = std::optional<T>; // NOLINT(readability-identifier-naming)
};

template <typename T>
struct tuple_element<1, Result<T>> {
    using type = std::error_code; // NOLINT(readability-identifier-naming)
};

} // namespace std

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

/// Helper type, that acts as a holder of either value or error code. The main use case for such dual nature of this
/// type is to be a result of an operation (e.g. function). Thanks to multiple convenience accessors, constructors and
/// conversion operators it is easy to return both value and error from function using `return` keyword.
/// @tparam T                   Type of the value, that is returned if there was no error.
template <typename T>
class Result {
public:
    /// Default constructor. Creates a result.
    /// Such result has empty value and error code indicating success.
    Result() = default;

    /// Constructor that creates Result initialized with given value and error code. By default, error code is set to
    /// "0", so Result will represent a successful operation result.
    /// @param value            Value to be stored.
    /// @param error            Error code to be stored (defaults to 0, which is success).
    /// @note This overload takes value by lvalue reference (values is copied into the Result).
    Result(const T& value, std::error_code error = {}) // NOLINT
        : m_value(value)
        , m_error(error)
    {}

    /// Constructor that creates Result initialized with given value and error code. By default, error code is set to
    /// "0", so Result will represent a successful operation result.
    /// @param value            Value to be stored.
    /// @param error            Error code to be stored (defaults to 0, which is success).
    /// @note This overload takes value by rvalue reference (value is moved into the Result).
    Result(T&& value, std::error_code error = {}) // NOLINT
        : m_value(std::move(value))
        , m_error(error)
    {}

    /// Constructor that creates Result initialized with given error enum.
    /// @tparam ErrorEnum       Type representing error enum to be stored in given Result.
    /// @param error            Error value to be stored.
    template <typename ErrorEnum>
    Result(ErrorEnum error) // NOLINT
        : Result(std::error_code{error})
    {
        static_assert(std::is_error_code_enum_v<ErrorEnum>, "ErrorEnum is not an error code enum");
    }

    /// Constructor that creates Result initialized with given error code.
    /// @param error            Error code to be stored.
    Result(std::error_code error) // NOLINT
        : m_error(error)
    {}

    /// Constructor that creates Result out of Result with different value type. Value in other object is ignored,
    /// only the error is copied into given Result. This way, Result with error can be propagated through chain of
    /// function calls even if they return different value types.
    /// @tparam OtherValue      Type of value from other Result to be initialized from.
    /// @param other            Object to be initialized from.
    template <typename OtherValue>
    Result(const Result<OtherValue>& other) // NOLINT
        : m_error(other.error())
    {
        assert(!other);
    }

    /// Copy constructor.
    Result(const Result&) = default;

    /// Move constructor.
    /// @param other            Result object to be moved into current instance.
    Result(Result&& other) noexcept
        : m_value(std::exchange(other.m_value, {}))
        , m_error(std::exchange(other.m_error, {}))
    {}

    /// Default destructor.
    ~Result() = default;

    /// Copy assignment operator.
    /// @return Reference to self.
    Result& operator=(const Result&) = default;

    /// Move assignment operator.
    /// @param other            Result object to be moved into current instance.
    /// @return Reference to self.
    Result& operator=(Result&& other) noexcept
    {
        if (&other != this) {
            m_value = std::exchange(other.m_value, {});
            m_error = std::exchange(other.m_error, {});
        }

        return *this;
    }

    /// Sets given value into current Result object.
    /// @param value            Value to be stored.
    /// @note This overload takes value by lvalue reference (values is copied into the Result).
    void setValue(const T& value) { m_value = value; }

    /// Sets given value into current Result object.
    /// @param value            Value to be stored.
    /// @note This overload takes value by rvalue reference (values is moved into the Result).
    void setValue(T&& value) { m_value = std::move(value); }

    /// Sets given error enum into current Result object.
    /// @tparam ErrorEnum       Type representing error enum to be stored in given Result.
    /// @param error            Error value to be stored.
    template <typename ErrorEnum>
    void setError(ErrorEnum error)
    {
        static_assert(std::is_error_code_enum_v<ErrorEnum>, "ErrorEnum is not an error code enum");
        m_error = error;
    }

    /// Sets given error code into current Result object.
    /// @param error            Error code to be stored.
    void setError(std::error_code error) { m_error = error; }

    /// Returns currently stored value.
    /// @return Currently stored value.
    /// @note If current object doesn't store any value (internal optional is empty), then an assert should happen as
    ///       this is an invalid condition to call this method.
    [[nodiscard]] T value() const
    {
        assert(m_value.has_value());
        return *m_value;
    }

    /// Returns currently stored value if exists or given fallback value if internal optional is empty.
    /// @param fallbackValue    Fallback value to be returned when there is no internal value.
    /// @return Currently stored value if exists or given fallback value if internal optional is empty.
    [[nodiscard]] T valueOr(const T& fallbackValue) const { return m_value.value_or(fallbackValue); }

    /// Returns optional with currently stored value.
    /// @return Optional with currently stored value.
    [[nodiscard]] std::optional<T> optionalValue() const { return m_value; }

    /// Returns currently stored error code.
    /// @return Currently stored error code.
    [[nodiscard]] std::error_code error() const { return m_error; }

    /// Returns currently stored value.
    /// @return Currently stored value.
    /// @note If current object doesn't store any value (internal optional is empty), then an assert should happen as
    ///       this is an invalid condition to call this operator.
    T operator*() const { return value(); }

    /// Conversion operator to T (value type).
    /// @return Currently stored value.
    /// @note If current object doesn't store any value (internal optional is empty), then an assert should happen as
    ///       this is an invalid condition to call this operator.
    explicit operator T() const { return value(); } // NOLINT

    /// Conversion operator to std::optional<T> (value type).
    /// @return Optional with currently stored value.
    operator std::optional<T>() const { return optionalValue(); } // NOLINT

    /// Conversion operator to std::error_code.
    /// @return Currently stored error code.
    operator std::error_code() const { return error(); } // NOLINT

    /// Conversion operator to bool.
    /// @return Flag indicating if underlying optional contains a valid value.
    /// @retval true            Result contains a valid value.
    /// @retval false           Result doesn't contain a valid value (optional is empty).
    explicit operator bool() const { return m_value.has_value(); }

    /// Returns N-th element in the structured binding support.
    /// @tparam cIndex          Index of the element to be returned.
    /// @returnN-th element in the structured binding support.
    /// @note This method is part of structured binding support for Result.
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

#ifndef UTILS_NO_INLINE_RESULT
/// Helper alias that brings Result into global namespace.
/// @tparam T                   Type of the value, that is returned if there was no error.
template <typename T>
using Result = utils::types::Result<T>;
#endif

namespace std {

/// Specialization of type returning number of elements used for structured binding.
/// @tparam T                   Type of the value, that is returned if there was no error.
/// @note This type is part of structured binding support for Result.
template <typename T>
struct tuple_size<utils::types::Result<T>> : integral_constant<size_t, 2> {};

/// Specialization of type returning type of first element used for structured binding.
/// @tparam T                   Type of the value, that is returned if there was no error.
/// @note This type is part of structured binding support for Result.
template <typename T>
struct tuple_element<0, utils::types::Result<T>> {
    using type = std::optional<T>; // NOLINT(readability-identifier-naming)
};

/// Specialization of type returning type of second element used for structured binding.
/// @tparam T                   Type of the value, that is returned if there was no error.
/// @note This type is part of structured binding support for Result.
template <typename T>
struct tuple_element<1, utils::types::Result<T>> {
    using type = std::error_code; // NOLINT(readability-identifier-naming)
};

} // namespace std

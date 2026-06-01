/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2026 Kuba Sejdak (kuba.sejdak@gmail.com)
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

#include <string>
#include <system_error>
#include <type_traits>

namespace utils::registry {

/// Errors definition.
enum class Error {
    eAlreadyInitialized = 1,
    eDuplicateId
};

/// Error category required by std::error_code.
struct ErrorCategory : std::error_category {
    [[nodiscard]] const char* name() const noexcept override { return "utils::registry"; }

    [[nodiscard]] std::string message(int value) const override
    {
        switch (static_cast<Error>(value)) {
            case Error::eAlreadyInitialized: return "already initialized";
            case Error::eDuplicateId:        return "duplicate id";
            default:                         return std::string(name()) + ": unrecognized error";
        }
    }
};

/// Creates error code value for Error enum.
/// @return std::error_code value created from Error enum.
inline std::error_code make_error_code(Error error) // NOLINT(readability-identifier-naming)
{
    static const ErrorCategory cCategory{};
    return {static_cast<int>(error), cCategory};
}

} // namespace utils::registry

namespace std {

template <>
struct is_error_code_enum<utils::registry::Error> : true_type {};

} // namespace std

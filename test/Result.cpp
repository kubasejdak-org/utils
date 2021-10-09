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

#include <utils/types/Result.hpp>

#include <catch2/catch.hpp>
#include <fmt/printf.h>

#include <string>
#include <system_error>
#include <type_traits>

enum class Error { eOk, eInvalidArgument };

struct ErrorCategory : std::error_category {
    [[nodiscard]] const char* name() const noexcept override;
    [[nodiscard]] std::string message(int value) const override;
};

const char* ErrorCategory::name() const noexcept
{
    return "test";
}

std::string ErrorCategory::message(int value) const
{
    switch (static_cast<Error>(value)) {
        case Error::eOk: return "eOk";
        case Error::eInvalidArgument: return "eInvalidArgument";
        default: return "(unrecognized error)";
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
std::error_code make_error_code(Error error)
{
    static const ErrorCategory cErrorCategory{};
    return {static_cast<int>(error), cErrorCategory};
}

namespace std {

template <>
struct is_error_code_enum<Error> : true_type {};

} // namespace std

Result<int> func(int value)
{
    if (value < 0)
        return Error::eInvalidArgument;

    return value * 2;
}

Result<int> func2(Result<int> value)
{
    if (value)
        return *value * 2;

    return value;
}

Result<std::string> func3(Result<int> value)
{
    if (value)
        return "<" + std::to_string(*value) + ">";

    return value.error();
}

TEST_CASE("1. Basic tests", "[unit][Result]")
{
    constexpr int cValue = 5;
    auto result1 = func(cValue);
    REQUIRE(result1);
    REQUIRE(*result1 == (2 * cValue));
    REQUIRE(result1.error().message() == "Success");

    int extractedValue1 = result1;
    std::error_code extractedError1 = result1;
    REQUIRE(extractedValue1 == (2 * cValue));
    REQUIRE(extractedError1.message() == "Success");

    auto [extractedValue2, extractedError2] = result1;
    REQUIRE(*extractedValue2 == (2 * cValue));
    REQUIRE(extractedError2.message() == "Success");

    auto result2 = func(-1);
    REQUIRE(!result2);
    REQUIRE(result2.error().message() == "eInvalidArgument");

    auto result3 = func2(func(cValue));
    REQUIRE(result3);
    REQUIRE(*result3 == (4 * cValue));
    REQUIRE(result3.error().message() == "Success");

    auto result4 = func2(func(-1));
    REQUIRE(!result4);
    REQUIRE(result4.error().message() == "eInvalidArgument");

    auto result5 = func3(func2(func(2)));
    REQUIRE(result5);
    REQUIRE(*result5 == "<8>");
    REQUIRE(result5.error().message() == "Success");

    auto result6 = func3(func2(func(-1)));
    REQUIRE(!result6);
    REQUIRE(result6.error().message() == "eInvalidArgument");
}

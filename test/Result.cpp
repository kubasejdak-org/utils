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

TEST_CASE("1. Manually constructing result object", "[unit][Result]")
{
    constexpr int cValue = 132;

    SECTION("1.1. Empty result")
    {
        Result<int> result;
        REQUIRE(!result.optionalValue());
        REQUIRE(!result.error());
    }

    SECTION("1.2. Setting value and default error code via constructor")
    {
        Result<int> result(cValue);
        REQUIRE(result.value() == cValue);
        REQUIRE(!result.error());
    }

    SECTION("1.3. Setting value and error enum via constructor")
    {
        Result<int> result(cValue, Error::eOk);
        REQUIRE(result.value() == cValue);
        REQUIRE(!result.error());
    }

    SECTION("1.4. Setting value and error code via constructor")
    {
        std::error_code error = Error::eOk;
        Result<int> result(cValue, error);
        REQUIRE(result.value() == cValue);
        REQUIRE(!result.error());
    }

    SECTION("1.5. Setting error enum via constructor")
    {
        Result<int> result(Error::eInvalidArgument);
        REQUIRE(!result.optionalValue());
        REQUIRE(result.error() == Error::eInvalidArgument);
    }

    SECTION("1.6. Setting error code via constructor")
    {
        std::error_code error = Error::eInvalidArgument;
        Result<int> result(error);
        REQUIRE(!result.optionalValue());
        REQUIRE(result.error() == Error::eInvalidArgument);
    }

    SECTION("1.7. Setting value and error enum via dedicated setters")
    {
        Result<int> result;
        result.setValue(cValue);
        result.setError(Error::eInvalidArgument);
        REQUIRE(result.value() == cValue);
        REQUIRE(result.error() == Error::eInvalidArgument);
    }

    SECTION("1.8. Setting value and error code via dedicated setters")
    {
        Result<int> result;
        result.setValue(cValue);
        std::error_code error = Error::eInvalidArgument;
        result.setError(error);
        REQUIRE(result.value() == cValue);
        REQUIRE(result.error() == Error::eInvalidArgument);
    }
}

TEST_CASE("2. Copy constructing result", "[unit][Result]")
{
    constexpr int cValue = 132;

    SECTION("2.1. Copy construct from empty result")
    {
        Result<int> result1 = {};
        REQUIRE(!result1.optionalValue());
        REQUIRE(!result1.error());

        Result<int> result2 = Result<int>{};
        REQUIRE(!result2.optionalValue());
        REQUIRE(!result2.error());
    }

    SECTION("2.2. Copy construct from value")
    {
        Result<int> result1 = cValue;
        REQUIRE(result1.value() == cValue);
        REQUIRE(!result1.error());

        Result<int> result2 = Result<int>{cValue};
        REQUIRE(result2.value() == cValue);
        REQUIRE(!result2.error());
    }

    SECTION("2.3. Copy construct from value and error enum")
    {
        Result<int> result1 = {cValue, Error::eInvalidArgument};
        REQUIRE(result1.value() == cValue);
        REQUIRE(result1.error() == Error::eInvalidArgument);

        Result<int> result2 = Result<int>{cValue, Error::eInvalidArgument};
        REQUIRE(result2.value() == cValue);
        REQUIRE(result2.error() == Error::eInvalidArgument);
    }

    SECTION("2.4. Copy construct from value and error code")
    {
        std::error_code error = Error::eInvalidArgument;
        Result<int> result1 = {cValue, error};
        REQUIRE(result1.value() == cValue);
        REQUIRE(result1.error() == Error::eInvalidArgument);

        Result<int> result2 = Result<int>{cValue, error};
        REQUIRE(result2.value() == cValue);
        REQUIRE(result2.error() == Error::eInvalidArgument);
    }

    SECTION("2.5. Copy construct from value and error code")
    {
        std::error_code error = Error::eInvalidArgument;
        Result<int> result1 = {cValue, error};
        REQUIRE(result1.value() == cValue);
        REQUIRE(result1.error() == Error::eInvalidArgument);

        Result<int> result2 = Result<int>{cValue, error};
        REQUIRE(result2.value() == cValue);
        REQUIRE(result2.error() == Error::eInvalidArgument);
    }

    SECTION("2.6. Copy construct from result with other value type")
    {
        Result<int> resultInt = Error::eInvalidArgument;
        REQUIRE(!resultInt.optionalValue());
        REQUIRE(resultInt.error() == Error::eInvalidArgument);

        Result<std::string> resultString = resultInt;
        REQUIRE(!resultString.optionalValue());
        REQUIRE(resultString.error() == Error::eInvalidArgument);
    }
}

TEST_CASE("3. Moving result around", "[unit][Result]")
{}

TEST_CASE("4. Basic tests", "[unit][Result]")
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

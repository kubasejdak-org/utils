/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2021-2023, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <string>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>

enum class Error {
    eOk,
    eInvalidArgument
};

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

constexpr int cValue = 132;

TEST_CASE("1. Manually constructing result object", "[unit][Result]")
{
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

        Result<int> result2 = Error::eOk;
        REQUIRE(!result2);
        REQUIRE(!result2.optionalValue());
        REQUIRE(result2.error() == Error::eOk);
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
        auto value = cValue;

        Result<int> result;
        result.setValue(std::move(value)); // NOLINT
        std::error_code error = Error::eInvalidArgument;
        result.setError(error);
        REQUIRE(result.value() == cValue);
        REQUIRE(result.error() == Error::eInvalidArgument);
    }
}

TEST_CASE("2. Copy constructing result", "[unit][Result]")
{
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

TEST_CASE("3. Moving result around via move constructor", "[unit][Result]")
{
    SECTION("3.1. Moving empty result")
    {
        Result<int> result;
        auto newResult = std::move(result);
        REQUIRE(!result.optionalValue()); // NOLINT
        REQUIRE(!result.error());
        REQUIRE(!newResult.optionalValue());
        REQUIRE(!newResult.error());
    }

    SECTION("3.2. Moving result initialized with value")
    {
        Result<int> result = cValue;
        auto newResult = std::move(result);
        REQUIRE(!result.optionalValue()); // NOLINT
        REQUIRE(!result.error());
        REQUIRE(newResult.value() == cValue);
        REQUIRE(!newResult.error());
    }

    SECTION("3.3. Moving result initialized with value and error")
    {
        Result<int> result = {cValue, Error::eInvalidArgument};
        auto newResult = std::move(result);
        REQUIRE(!result.optionalValue()); // NOLINT
        REQUIRE(!result.error());
        REQUIRE(newResult.value() == cValue);
        REQUIRE(newResult.error() == Error::eInvalidArgument);
    }

    SECTION("3.4. Moving result initialized with error")
    {
        Result<int> result = Error::eInvalidArgument;
        auto newResult = std::move(result);
        REQUIRE(!result.optionalValue()); // NOLINT
        REQUIRE(!result.error());
        REQUIRE(!newResult.optionalValue());
        REQUIRE(newResult.error() == Error::eInvalidArgument);
    }
}

TEST_CASE("4. Copying result via copy assignment", "[unit][Result]")
{
    SECTION("4.1. Copying empty result")
    {
        Result<int> result;
        Result<int> newResult;
        newResult = result;
        REQUIRE(!result.optionalValue());
        REQUIRE(!result.error());
        REQUIRE(!newResult.optionalValue());
        REQUIRE(!newResult.error());
    }

    SECTION("4.2. Copying result initialized with value")
    {
        Result<int> result = cValue;
        Result<int> newResult;
        newResult = result;
        REQUIRE(result.optionalValue());
        REQUIRE(!result.error());
        REQUIRE(newResult.value() == cValue);
        REQUIRE(!newResult.error());
    }

    SECTION("4.3. Copying result initialized with value and error")
    {
        Result<int> result = {cValue, Error::eInvalidArgument};
        Result<int> newResult;
        newResult = result;
        REQUIRE(result.value() == cValue);
        REQUIRE(result.error() == Error::eInvalidArgument);
        REQUIRE(newResult.value() == cValue);
        REQUIRE(newResult.error() == Error::eInvalidArgument);
    }

    SECTION("4.4. Copying result initialized with error")
    {
        Result<int> result = Error::eInvalidArgument;
        Result<int> newResult;
        newResult = result;
        REQUIRE(!result.optionalValue());
        REQUIRE(result.error() == Error::eInvalidArgument);
        REQUIRE(!newResult.optionalValue());
        REQUIRE(newResult.error() == Error::eInvalidArgument);
    }
}

TEST_CASE("5. Moving result around via move assignment", "[unit][Result]")
{
    SECTION("5.1. Moving empty result")
    {
        Result<int> result;
        Result<int> newResult;
        newResult = std::move(result);
        REQUIRE(!result.optionalValue()); // NOLINT
        REQUIRE(!result.error());
        REQUIRE(!newResult.optionalValue());
        REQUIRE(!newResult.error());
    }

    SECTION("5.2. Moving result initialized with value")
    {
        Result<int> result = cValue;
        Result<int> newResult;
        newResult = std::move(result);
        REQUIRE(!result.optionalValue()); // NOLINT
        REQUIRE(!result.error());
        REQUIRE(newResult.value() == cValue);
        REQUIRE(!newResult.error());
    }

    SECTION("5.3. Moving result initialized with value and error")
    {
        Result<int> result = {cValue, Error::eInvalidArgument};
        Result<int> newResult;
        newResult = std::move(result);
        REQUIRE(!result.optionalValue()); // NOLINT
        REQUIRE(!result.error());
        REQUIRE(newResult.value() == cValue);
        REQUIRE(newResult.error() == Error::eInvalidArgument);
    }

    SECTION("5.4. Moving result initialized with error")
    {
        Result<int> result = Error::eInvalidArgument;
        Result<int> newResult;
        newResult = std::move(result);
        REQUIRE(!result.optionalValue()); // NOLINT
        REQUIRE(!result.error());
        REQUIRE(!newResult.optionalValue());
        REQUIRE(newResult.error() == Error::eInvalidArgument);
    }
}

TEST_CASE("6. Manually setting ang getting values and errors", "[unit][Result]")
{
    SECTION("6.1. Setting and getting value")
    {
        Result<int> result;
        result.setValue(cValue);
        REQUIRE(result.value() == cValue);
        REQUIRE(result.valueOr(2 * cValue) == cValue);
        REQUIRE(result.optionalValue().has_value());
        REQUIRE(result.optionalValue().value() == cValue);
        REQUIRE(!result.error());
    }

    SECTION("6.2. Setting and getting error enum")
    {
        Result<int> result;
        result.setError(Error::eInvalidArgument);
        REQUIRE(result.valueOr(2 * cValue) == (2 * cValue));
        REQUIRE(!result.optionalValue().has_value());
        REQUIRE(result.error() == Error::eInvalidArgument);
    }

    SECTION("6.3. Setting and getting error code")
    {
        Result<int> result;
        std::error_code error = Error::eInvalidArgument;
        result.setError(error);
        REQUIRE(result.valueOr(2 * cValue) == (2 * cValue));
        REQUIRE(!result.optionalValue().has_value());
        REQUIRE(result.error() == error);
    }
}

TEST_CASE("7. Conversion operators", "[unit][Result]")
{
    Result<int> result = {cValue, Error::eInvalidArgument};
    Result<int> result2 = Error::eInvalidArgument;

    SECTION("7.1. Dereferencing result")
    {
        REQUIRE(*result == cValue);
    }

    SECTION("7.2. Casting to optional value")
    {
        std::optional<int> value = result;
        REQUIRE(*value == cValue);

        std::optional<int> value2 = result2;
        REQUIRE(!value2);
    }

    SECTION("7.3. Casting to error code")
    {
        std::error_code error = result;
        REQUIRE(error == Error::eInvalidArgument);

        std::error_code error2 = result2;
        REQUIRE(error2 == Error::eInvalidArgument);
    }

    SECTION("7.4. Casting to bool")
    {
        REQUIRE(result);
        REQUIRE(!result2);
    }
}

TEST_CASE("8. Structured binding", "[unit][Result]")
{
    SECTION("8.1. Result initialized with value")
    {
        Result<int> result = cValue;
        auto [value, error] = result;
        REQUIRE(*value == cValue);
        REQUIRE(!error);
    }

    SECTION("8.2. Result initialized with error")
    {
        Result<int> result = Error::eInvalidArgument;
        auto [value, error] = result;
        REQUIRE(!value);
        REQUIRE(error == Error::eInvalidArgument);
    }

    SECTION("8.3. Result initialized with value and error")
    {
        Result<int> result = {cValue, Error::eInvalidArgument};
        auto [value, error] = result;
        REQUIRE(*value == cValue);
        REQUIRE(error == Error::eInvalidArgument);
    }
}

TEST_CASE("9. std::tie() support", "[unit][Result]")
{
    std::optional<int> value;
    std::error_code error;

    SECTION("9.1. Result initialized with value")
    {
        Result<int> result = cValue;
        std::tie(value, error) = result.toTuple();
        REQUIRE(*value == cValue);
        REQUIRE(!error);
    }

    SECTION("9.2. Result initialized with error")
    {
        Result<int> result = Error::eInvalidArgument;
        std::tie(value, error) = result.toTuple();
        REQUIRE(!value);
        REQUIRE(error == Error::eInvalidArgument);
    }

    SECTION("9.3. Result initialized with value and error")
    {
        Result<int> result = {cValue, Error::eInvalidArgument};
        std::tie(value, error) = result.toTuple();
        REQUIRE(*value == cValue);
        REQUIRE(error == Error::eInvalidArgument);
    }
}

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

Result<std::string> func3(Result<int> value) // NOLINT
{
    if (value)
        return "<" + std::to_string(*value) + ">";

    return value;
}

TEST_CASE("10. Checking result of a function", "[unit][Result]")
{
    SECTION("10.1. Returning correct value")
    {
        auto result = func(cValue);
        REQUIRE(result);
        REQUIRE(*result == (2 * cValue));
        REQUIRE(!result.error());
        REQUIRE(result.error().message() == "Success");
    }

    SECTION("10.2. Returning error")
    {
        auto result = func(-1);
        REQUIRE(!result);
        REQUIRE(result.error() == Error::eInvalidArgument);
        REQUIRE(result.error().message() == "eInvalidArgument");
    }

    SECTION("10.3. Passing 'value' result from one function to another")
    {
        auto result = func2(func(cValue));
        REQUIRE(result);
        REQUIRE(*result == (4 * cValue));
        REQUIRE(!result.error());
        REQUIRE(result.error().message() == "Success");
    }

    SECTION("10.4. Passing 'error' result from one function to another")
    {
        auto result = func2(func(-1));
        REQUIRE(!result);
        REQUIRE(result.error() == Error::eInvalidArgument);
        REQUIRE(result.error().message() == "eInvalidArgument");
    }

    SECTION("10.5. Passing 'value' result from one function to another with value type change")
    {
        auto result = func3(func2(func(2)));
        REQUIRE(result);
        REQUIRE(*result == "<8>");
        REQUIRE(!result.error());
        REQUIRE(result.error().message() == "Success");
    }

    SECTION("10.6. Passing 'error' result from one function to another with value type change")
    {
        auto result = func3(func2(func(-1)));
        REQUIRE(!result);
        REQUIRE(result.error() == Error::eInvalidArgument);
        REQUIRE(result.error().message() == "eInvalidArgument");
    }
}

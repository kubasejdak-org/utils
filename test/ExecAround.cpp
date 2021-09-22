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

#include <utils/functional/ExecAround.hpp>

#include <catch2/catch.hpp>

struct TestType {
    int i{};
    double d{};

    void func1() { ++i; }
    void func2() { ++d; }
};

TEST_CASE("1. Simple wrapper around custom type", "[unit][ExecAround]")
{
    bool preCall{};
    bool postCall{};
    utils::functional::ExecAround wrapper([&] { preCall = true; }, [&] { postCall = true; }, TestType{});
    REQUIRE(wrapper->i == 0);
    REQUIRE(wrapper->d == 0.0);

    wrapper->func1();
    REQUIRE(wrapper->i == 1);
    REQUIRE(wrapper->d == 0.0);
    REQUIRE(preCall);
    REQUIRE(postCall);

    preCall = false;
    postCall = false;

    wrapper->func2();
    REQUIRE(wrapper->i == 1);
    REQUIRE(wrapper->d == 1.0);
    REQUIRE(preCall);
    REQUIRE(postCall);
}

TEST_CASE("2. Moving ExecAround around", "[unit][ExecAround]")
{
    SECTION("2.1. Copy construction") {}

    SECTION("2.2. Move construction") {}

    SECTION("2.3. Copy assignment") {}

    SECTION("2.4. Move assignment") {}
}

TEST_CASE("3. Passing wrapped object in different ways", "[unit][ExecAround]")
{
    SECTION("3.1. Copy") {}

    SECTION("3.2. Move") {}

    SECTION("3.3. Reference") {}

    SECTION("3.4. Reference wrapper") {}
}

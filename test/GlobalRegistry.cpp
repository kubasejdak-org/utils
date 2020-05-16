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

#include <utils/GlobalRegistry.hpp>

#include <catch2/catch.hpp>

#include <memory>
#include <type_traits>

TEST_CASE("All instances are correctly stored in GlobalRegistry with custom id type", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(int a)
            : value(a)
        {}

        int value{};
    };

    using TestRegistry = utils::GlobalRegistry<Test, int>;
    std::size_t instancesCount{};

    SECTION("1 instance")
    {
        TestRegistry::init({{0, Test(0)}});
        instancesCount = 1;
    }

    SECTION("4 instances")
    {
        TestRegistry::init({{0, Test(0)}, {1, Test(1)}, {2, Test(2)}, {3, Test(3)}});
        instancesCount = 4;
    }

    auto size = TestRegistry::size();
    REQUIRE(size == instancesCount);
    for (std::size_t i = 0; i < size; ++i) {
        auto instance = TestRegistry::get(i);
        REQUIRE(std::is_same_v<decltype(instance), std::shared_ptr<Test>>);
        REQUIRE(instance->value == static_cast<int>(i));
    }

    constexpr int cInvalidId = 100;
    auto nullInstance = TestRegistry::get(cInvalidId);
    REQUIRE(!nullInstance);

    TestRegistry::clear();
    size = TestRegistry::size();
    REQUIRE(size == 0);
}

TEST_CASE("Move only types can be stored in GlobalRegistry with default id type", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(int a)
            : value(a)
        {}

        [[maybe_unused]] Test(const Test&) = delete;
        Test(Test&& other) noexcept
        {
            value = other.value;
            other.value = -1;
            moved = true;
        }
        ~Test() = default;
        Test& operator=(const Test&) = delete;
        Test& operator=(Test&&) = delete;

        int value{};
        bool moved{false};
    };

    using TestRegistry = utils::GlobalRegistry<Test>;
    std::size_t instancesCount{};

    SECTION("1 instance")
    {
        TestRegistry::init({{"0", Test(0)}});
        instancesCount = 1;
    }

    SECTION("4 instances")
    {
        TestRegistry::init({{"0", Test(0)}, {"1", Test(1)}, {"2", Test(2)}, {"3", Test(3)}});
        instancesCount = 4;
    }

    auto size = TestRegistry::size();
    REQUIRE(size == instancesCount);
    for (std::size_t i = 0; i < size; ++i) {
        auto instance = TestRegistry::get(std::to_string(i));
        REQUIRE(std::is_same_v<decltype(instance), std::shared_ptr<Test>>);
        REQUIRE(instance->value == static_cast<int>(i));
        REQUIRE(instance->moved);
    }

    auto nullInstance = TestRegistry::get("100");
    REQUIRE(!nullInstance);

    TestRegistry::clear();
    size = TestRegistry::size();
    REQUIRE(size == 0);
}

TEST_CASE("GlobalRegistry can hold derived types with abstract interface", "[unit][GlobalRegistry]")
{
    struct IBase {
        [[maybe_unused]] virtual void func() = 0;
    };
    struct Derived : IBase {
        void func() override {}
    };

    using BaseRegistry = utils::GlobalRegistry<IBase>;

    BaseRegistry::init({{"instance0", Derived()}, {"instance1", Derived()}, {"instance2", Derived()}});
    auto size = BaseRegistry::size();
    REQUIRE(size == 3);

    BaseRegistry::clear();
    size = BaseRegistry::size();
    REQUIRE(size == 0);
}

/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2020 Kuba Sejdak (kuba.sejdak@gmail.com)
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

#include <utils/registry/GlobalRegistry.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <format>
#include <memory>
#include <string>
#include <type_traits>

TEST_CASE("1. All instances are correctly stored in GlobalRegistry with custom id type", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(unsigned int a)
            : value(a)
        {}

        unsigned int value{};
    };

    using TestRegistry = utils::registry::GlobalRegistry<Test, unsigned int>;
    std::size_t instancesCount{};

    SECTION("1.1. 1 instance")
    {
        TestRegistry::init({
            {0, Test(0)}
        });
        instancesCount = 1;
    }

    SECTION("1.2. 4 instances")
    {
        TestRegistry::init({
            {0, Test(0)},
            {1, Test(1)},
            {2, Test(2)},
            {3, Test(3)}
        });
        instancesCount = 4;
    }

    auto size = TestRegistry::size();
    REQUIRE(size == instancesCount);
    for (unsigned int i = 0; i < size; ++i) {
        auto instance = TestRegistry::get(i);
        static_assert(std::is_same_v<decltype(instance), std::shared_ptr<Test>>);
        REQUIRE(instance != nullptr);
        CHECK(instance->value == i);
    }

    constexpr int cInvalidId = 100;
    auto nullInstance = TestRegistry::get(cInvalidId);
    CHECK(nullInstance == nullptr);

    TestRegistry::clear();
    size = TestRegistry::size();
    CHECK(size == 0);
}

TEST_CASE("2. Move only types can be stored in GlobalRegistry with default id type", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(unsigned int a)
            : value(a)
        {}

        [[maybe_unused]] Test(const Test&) = delete;

        Test(Test&& other) noexcept
            : value(other.value)
            , moved(true)
        {
            other.value = -1;
        }

        ~Test() = default;
        Test& operator=(const Test&) = delete;
        Test& operator=(Test&&) = delete;

        unsigned int value{};
        bool moved{false};
    };

    using TestRegistry = utils::registry::GlobalRegistry<Test>;
    std::size_t instancesCount{};

    SECTION("2.1. 1 instance")
    {
        TestRegistry::init({
            {"0", Test(0)}
        });
        instancesCount = 1;
    }

    SECTION("2.2. 4 instances")
    {
        TestRegistry::init({
            {"0", Test(0)},
            {"1", Test(1)},
            {"2", Test(2)},
            {"3", Test(3)}
        });
        instancesCount = 4;
    }

    auto size = TestRegistry::size();
    REQUIRE(size == instancesCount);
    for (unsigned int i = 0; i < size; ++i) {
        auto instance = TestRegistry::get(std::to_string(i));
        static_assert(std::is_same_v<decltype(instance), std::shared_ptr<Test>>);
        REQUIRE(instance != nullptr);
        CHECK(instance->value == i);
        CHECK(instance->moved);
    }

    auto nullInstance = TestRegistry::get("100");
    CHECK(nullInstance == nullptr);

    TestRegistry::clear();
    size = TestRegistry::size();
    CHECK(size == 0);
}

TEST_CASE("3. GlobalRegistry can hold derived types with abstract interface", "[unit][GlobalRegistry]")
{
    struct IBase {
        IBase() = default;
        [[maybe_unused]] IBase(const IBase&) = delete;
        [[maybe_unused]] IBase(IBase&&) noexcept = default;

        virtual ~IBase() = default;
        IBase& operator=(const IBase&) = delete;
        IBase& operator=(IBase&&) = delete;
        [[maybe_unused]] virtual void func() = 0;
    };

    struct Derived : IBase {
        void func() override {}
    };

    using BaseRegistry = utils::registry::GlobalRegistry<IBase>;

    BaseRegistry::init({
        {"instance0", Derived()},
        {"instance1", Derived()},
        {"instance2", Derived()}
    });

    auto size = BaseRegistry::size();
    REQUIRE(size == 3);
    for (std::size_t i = 0; i < size; ++i) {
        auto instance = BaseRegistry::get(std::format("instance{}", i));
        REQUIRE(instance != nullptr);
        CHECK(dynamic_cast<Derived*>(instance.get()) != nullptr);
    }

    BaseRegistry::clear();
    size = BaseRegistry::size();
    CHECK(size == 0);
}

TEST_CASE("4. Instances can be appended to GlobalRegistry with append()", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(int a)
            : value(a)
        {}

        int value{};
    };

    using TestRegistry = utils::registry::GlobalRegistry<Test, int>;

    TestRegistry::init({
        {0, Test(0)},
        {1, Test(1)}
    });
    CHECK(TestRegistry::size() == 2);

    TestRegistry::append({
        {2, Test(2)},
        {3, Test(3)}
    });
    CHECK(TestRegistry::size() == 4);

    for (int i = 0; i < 4; ++i) {
        auto instance = TestRegistry::get(i);
        REQUIRE(instance != nullptr);
        CHECK(instance->value == i);
    }

    TestRegistry::clear();
    CHECK(TestRegistry::size() == 0);
}

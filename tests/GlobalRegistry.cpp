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

#include <utils/registry/Error.hpp>
#include <utils/registry/GlobalRegistry.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <vector>

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
        auto error = TestRegistry::init({
            {0, Test(0)}
        });
        REQUIRE(!error);
        instancesCount = 1;
    }

    SECTION("1.2. 4 instances")
    {
        auto error = TestRegistry::init({
            {0, Test(0)},
            {1, Test(1)},
            {2, Test(2)},
            {3, Test(3)}
        });
        REQUIRE(!error);
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
        auto error = TestRegistry::init({
            {"0", Test(0)}
        });
        REQUIRE(!error);
        instancesCount = 1;
    }

    SECTION("2.2. 4 instances")
    {
        auto error = TestRegistry::init({
            {"0", Test(0)},
            {"1", Test(1)},
            {"2", Test(2)},
            {"3", Test(3)}
        });
        REQUIRE(!error);
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

    auto error = BaseRegistry::init({
        {"instance0", Derived()},
        {"instance1", Derived()},
        {"instance2", Derived()}
    });
    REQUIRE(!error);

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

    auto initError = TestRegistry::init({
        {0, Test(0)},
        {1, Test(1)}
    });
    REQUIRE(!initError);
    CHECK(TestRegistry::size() == 2);

    auto appendError = TestRegistry::append({
        {2, Test(2)},
        {3, Test(3)}
    });
    REQUIRE(!appendError);
    CHECK(TestRegistry::size() == 4);

    for (int i = 0; i < 4; ++i) {
        auto instance = TestRegistry::get(i);
        REQUIRE(instance != nullptr);
        CHECK(instance->value == i);
    }

    TestRegistry::clear();
    CHECK(TestRegistry::size() == 0);
}

TEST_CASE("5. init() fails when called on a non-empty registry", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(int a)
            : value(a)
        {}

        int value{};
    };

    using TestRegistry = utils::registry::GlobalRegistry<Test, int>;

    auto error1 = TestRegistry::init({
        {0, Test(0)},
        {1, Test(1)}
    });
    REQUIRE(!error1);
    CHECK(TestRegistry::size() == 2);

    auto error2 = TestRegistry::init({
        {2, Test(2)}
    });
    CHECK(error2 == utils::registry::Error::eAlreadyInitialized);
    CHECK(TestRegistry::size() == 2);

    TestRegistry::clear();
}

TEST_CASE("6. append() returns error and still inserts non-duplicates", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(int a)
            : value(a)
        {}

        int value{};
    };

    using TestRegistry = utils::registry::GlobalRegistry<Test, int>;

    auto initError = TestRegistry::init({
        {0, Test(0)},
        {1, Test(1)}
    });
    REQUIRE(!initError);

    auto appendError = TestRegistry::append({
        {1, Test(99)},
        {2, Test(2) }
    });
    CHECK(appendError == utils::registry::Error::eDuplicateId);
    CHECK(appendError.message() == "duplicate id");

    REQUIRE(TestRegistry::size() == 3);
    CHECK(TestRegistry::get(1)->value == 1);
    CHECK(TestRegistry::get(2)->value == 2);

    TestRegistry::clear();
}

TEST_CASE("7. ids() returns all registered ids in sorted order", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(int a)
            : value(a)
        {}

        int value{};
    };

    using TestRegistry = utils::registry::GlobalRegistry<Test>;

    auto error = TestRegistry::init({
        {"banana", Test(2)},
        {"apple",  Test(1)},
        {"cherry", Test(3)}
    });
    REQUIRE(!error);

    auto registeredKeys = TestRegistry::ids();
    REQUIRE(registeredKeys.size() == 3);
    CHECK(registeredKeys[0] == "apple");
    CHECK(registeredKeys[1] == "banana");
    CHECK(registeredKeys[2] == "cherry");

    TestRegistry::clear();
    CHECK(TestRegistry::ids().empty());
}

TEST_CASE("8. get() supports heterogeneous lookup with string_view", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(int a)
            : value(a)
        {}

        int value{};
    };

    using TestRegistry = utils::registry::GlobalRegistry<Test>;

    auto error = TestRegistry::init({
        {"hello", Test(42)}
    });
    REQUIRE(!error);

    std::string_view key = "hello";
    auto instance = TestRegistry::get(key);
    REQUIRE(instance != nullptr);
    CHECK(instance->value == 42);

    std::string_view missing = "world";
    CHECK(TestRegistry::get(missing) == nullptr);

    TestRegistry::clear();
}

TEST_CASE("9. Concurrent get() calls are thread-safe", "[unit][GlobalRegistry]")
{
    struct Test {
        explicit Test(int a)
            : value(a)
        {}

        int value{};
    };

    using TestRegistry = utils::registry::GlobalRegistry<Test, int>;

    auto error = TestRegistry::init({
        {0, Test(0)},
        {1, Test(1)},
        {2, Test(2)}
    });
    REQUIRE(!error);

    constexpr int cThreadCount = 8;
    constexpr int cIterations = 1000;
    std::vector<bool> threadResults(cThreadCount, true);
    std::vector<std::thread> threads;
    threads.reserve(cThreadCount);

    for (int t = 0; t < cThreadCount; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < cIterations; ++i) {
                for (int id = 0; id < 3; ++id) {
                    auto instance = TestRegistry::get(id);
                    if (!instance || instance->value != id) {
                        threadResults[t] = false;
                        return;
                    }
                }
            }
        });
    }

    for (auto& thread : threads)
        thread.join();

    for (int t = 0; t < cThreadCount; ++t)
        CHECK(threadResults[t]);

    TestRegistry::clear();
}

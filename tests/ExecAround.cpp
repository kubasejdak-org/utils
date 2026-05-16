/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2021 Kuba Sejdak (kuba.sejdak@gmail.com)
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

#include <utils/functional/ExecAround.hpp>

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <utility>

struct TestType {
    TestType() = default;

    TestType(const TestType& /*other*/)
        : copyConstructed(true)
    {}

    TestType(TestType&& /*other*/) noexcept
        : moveConstructed(true)
    {}

    ~TestType() = default;

    TestType& operator=(const TestType& other)
    {
        if (this != &other)
            copyAssigned = true;

        return *this;
    }

    TestType& operator=(TestType&& other) noexcept
    {
        if (this != &other)
            moveAssigned = true;

        return *this;
    }

    bool copyConstructed{};
    bool moveConstructed{};
    bool copyAssigned{};
    bool moveAssigned{};
    int i{};
    double d{};

    void func1() { ++i; }

    void func2() { ++d; }
};

TEST_CASE("1. Simple wrapper around custom type", "[unit][ExecAround]")
{
    bool preCall{};
    bool postCall{};
    auto preAction = [&] { preCall = true; };
    auto postAction = [&] { postCall = true; };

    utils::functional::ExecAround<TestType> wrapper(TestType{}, preAction, postAction);
    CHECK(wrapper->i == 0);
    CHECK(wrapper->d == 0.0);

    wrapper->func1();
    CHECK(wrapper->i == 1);
    CHECK(wrapper->d == 0.0);
    CHECK(preCall);
    CHECK(postCall);

    preCall = false;
    postCall = false;

    wrapper->func2();
    CHECK(wrapper->i == 1);
    CHECK(wrapper->d == 1.0);
    CHECK(preCall);
    CHECK(postCall);
}

TEST_CASE("2. Moving ExecAround around", "[unit][ExecAround]")
{
    bool preCall{};
    bool postCall{};
    auto preAction = [&] { preCall = true; };
    auto postAction = [&] { postCall = true; };

    utils::functional::ExecAround<TestType> wrapper(TestType{}, preAction, postAction);

    SECTION("2.1. Copy construction")
    {
        auto copiedWrapper = wrapper;
        CHECK(copiedWrapper->copyConstructed);

        copiedWrapper->func1();
        CHECK(copiedWrapper->i == 1);
    }

    SECTION("2.2. Move construction")
    {
        auto movedWrapper(std::move(wrapper));
        CHECK(movedWrapper->moveConstructed);

        movedWrapper->func1();
        CHECK(movedWrapper->i == 1);
    }

    SECTION("2.3. Copy assignment")
    {
        utils::functional::ExecAround<TestType> copyAssignedWrapper;
        copyAssignedWrapper = wrapper; // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
        CHECK(copyAssignedWrapper->copyAssigned);

        copyAssignedWrapper->func1();
        CHECK(copyAssignedWrapper->i == 1);
    }

    SECTION("2.4. Move assignment")
    {
        utils::functional::ExecAround<TestType> moveAssignedWrapper;
        moveAssignedWrapper = std::move(wrapper);
        CHECK(moveAssignedWrapper->moveAssigned);

        moveAssignedWrapper->func1();
        CHECK(moveAssignedWrapper->i == 1);
    }

    CHECK(wrapper->i == 0); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
    CHECK(preCall);
    CHECK(postCall);
}

TEST_CASE("3. Passing wrapped object in different ways", "[unit][ExecAround]")
{
    bool preCall{};
    bool postCall{};
    auto preAction = [&] { preCall = true; };
    auto postAction = [&] { postCall = true; };

    SECTION("3.1. Copy from object")
    {
        TestType test;
        utils::functional::ExecAround<TestType> wrapper(test, preAction, postAction);
        CHECK(wrapper->copyConstructed);

        wrapper->func1();
        CHECK(wrapper->i == 1);
        CHECK(test.i == 0);
    }

    SECTION("3.2. Copy from temporary")
    {
        utils::functional::ExecAround<TestType> wrapper(TestType{}, preAction, postAction);
        CHECK(wrapper->moveConstructed);

        wrapper->func1();
    }

    SECTION("3.3. Move")
    {
        TestType test;
        utils::functional::ExecAround<TestType> wrapper(std::move(test), preAction, postAction);
        CHECK(wrapper->moveConstructed);

        wrapper->func1();
        CHECK(wrapper->i == 1);
        CHECK(test.i == 0); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
    }

    SECTION("3.4. Reference wrapper")
    {
        using TestTypeRef = std::reference_wrapper<TestType>;

        TestType test;
        utils::functional::ExecAround<TestTypeRef> wrapper(std::ref(test), preAction, postAction);
        CHECK(!wrapper->copyConstructed);
        CHECK(!wrapper->moveConstructed);

        wrapper->func1();
        CHECK(wrapper->i == 1);
        CHECK(test.i == 1);
    }

    SECTION("3.5. Pointer")
    {
        auto test = std::make_unique<TestType>();
        utils::functional::ExecAround<TestType*> wrapper(test.get(), preAction, postAction);
        CHECK(!wrapper->copyConstructed);
        CHECK(!wrapper->moveConstructed);

        wrapper->func1();
        CHECK(wrapper->i == 1);
        CHECK(test->i == 1);
    }

    SECTION("3.6. Shared pointer")
    {
        auto test = std::make_shared<TestType>();
        utils::functional::ExecAround<std::shared_ptr<TestType>> wrapper(test, preAction, postAction);
        CHECK(!wrapper->copyConstructed);
        CHECK(!wrapper->moveConstructed);

        wrapper->func1();
        CHECK(wrapper->i == 1);
        CHECK(test->i == 1);
    }

    SECTION("3.7. Reference to shared pointer")
    {
        using TestTypeSharedPtrRef = std::reference_wrapper<std::shared_ptr<TestType>>;

        auto test = std::make_shared<TestType>();
        utils::functional::ExecAround<TestTypeSharedPtrRef> wrapper(std::ref(test), preAction, postAction);
        CHECK(!wrapper->copyConstructed);
        CHECK(!wrapper->moveConstructed);

        wrapper->func1();
        CHECK(wrapper->i == 1);
        CHECK(test->i == 1);
    }

    CHECK(preCall);
    CHECK(postCall);
}

TEST_CASE("4. ExecAround with empty actions does not crash", "[unit][ExecAround]")
{
    utils::functional::ExecAround<TestType> wrapper(TestType{}, nullptr, nullptr);
    wrapper->func1();
    CHECK(wrapper->i == 1);
}

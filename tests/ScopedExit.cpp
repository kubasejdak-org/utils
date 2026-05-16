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

#include <utils/functional/ScopedExit.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("1. Single ScopedExit call", "[unit][ScopedExit]")
{
    bool called{};

    SECTION("1.1. Explicit object creation")
    {
        {
            utils::functional::ScopedExit onExit([&] { called = true; });
            CHECK(!called);
        }
    }

    SECTION("1.2. Object creation with helper macro")
    {
        {
            ON_EXIT([&] { called = true; });
            CHECK(!called);
        }
    }

    CHECK(called);
}

TEST_CASE("2. Multiple ScopedExit calls", "[unit][ScopedExit]")
{
    bool called1{};
    bool called2{};
    bool called3{};
    bool called4{};

    SECTION("2.1. Explicit object creation")
    {
        {
            utils::functional::ScopedExit onExit1([&] { called1 = true; });
            utils::functional::ScopedExit onExit2([&] { called2 = true; });
            utils::functional::ScopedExit onExit3([&] { called3 = true; });
            utils::functional::ScopedExit onExit4([&] { called4 = true; });
            CHECK(!called1);
            CHECK(!called2);
            CHECK(!called3);
            CHECK(!called4);
        }
    }

    SECTION("2.2. Object creation with helper macro")
    {
        {
            ON_EXIT([&] { called1 = true; });
            ON_EXIT([&] { called2 = true; });
            ON_EXIT([&] { called3 = true; });
            ON_EXIT([&] { called4 = true; });
            CHECK(!called1);
            CHECK(!called2);
            CHECK(!called3);
            CHECK(!called4);
        }
    }

    CHECK(called1);
    CHECK(called2);
    CHECK(called3);
    CHECK(called4);
}

TEST_CASE("3. More complex logic of ScopedExit calls", "[unit][ScopedExit]")
{
    bool called1{};
    bool called2{};
    bool called3{};
    bool called4{};

    {
        ON_EXIT([&] { called1 = true; });
        CHECK(!called1);
        CHECK(!called2);
        CHECK(!called3);
        CHECK(!called4);

        {
            ON_EXIT([&] { called2 = true; });
            CHECK(!called1);
            CHECK(!called2);
            CHECK(!called3);
            CHECK(!called4);

            {
                ON_EXIT([&] { called3 = true; });
                CHECK(!called1);
                CHECK(!called2);
                CHECK(!called3);
                CHECK(!called4);
            }

            CHECK(!called1);
            CHECK(!called2);
            CHECK(called3);
            CHECK(!called4);

            {
                ON_EXIT([&] { called4 = true; });
                CHECK(!called1);
                CHECK(!called2);
                CHECK(called3);
                CHECK(!called4);
            }

            CHECK(!called1);
            CHECK(!called2);
            CHECK(called3);
            CHECK(called4);
        }

        CHECK(!called1);
        CHECK(called2);
        CHECK(called3);
        CHECK(called4);
    }

    CHECK(called1);
    CHECK(called2);
    CHECK(called3);
    CHECK(called4);
}

TEST_CASE("4. ScopedExit with empty callback does not crash", "[unit][ScopedExit]")
{
    utils::functional::ScopedExit onExit(nullptr);
}

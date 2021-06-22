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

#include <utils/functional/ScopedExit.hpp>

#include <catch2/catch.hpp>

TEST_CASE("1. Single ScopedExit call", "[unit][ScopedExit]")
{
    bool called{};

    SECTION("1.1. Explicit object creation")
    {
        {
            utils::functional::ScopedExit onExit([&] { called = true; });
            REQUIRE(!called);
        }
    }

    SECTION("1.2. Object creation with helper macro")
    {
        {
            ON_EXIT([&] { called = true; });
            REQUIRE(!called);
        }
    }

    REQUIRE(called);
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
            REQUIRE(!called1);
            REQUIRE(!called2);
            REQUIRE(!called3);
            REQUIRE(!called4);
        }
    }

    SECTION("2.2. Object creation with helper macro")
    {
        {
            ON_EXIT([&] { called1 = true; });
            ON_EXIT([&] { called2 = true; });
            ON_EXIT([&] { called3 = true; });
            ON_EXIT([&] { called4 = true; });
            REQUIRE(!called1);
            REQUIRE(!called2);
            REQUIRE(!called3);
            REQUIRE(!called4);
        }
    }

    REQUIRE(called1);
    REQUIRE(called2);
    REQUIRE(called3);
    REQUIRE(called4);
}

TEST_CASE("3. More complex logic of ScopedExit calls", "[unit][ScopedExit]")
{
    bool called1{};
    bool called2{};
    bool called3{};
    bool called4{};

    {
        ON_EXIT([&] { called1 = true; });
        REQUIRE(!called1);
        REQUIRE(!called2);
        REQUIRE(!called3);
        REQUIRE(!called4);

        {
            ON_EXIT([&] { called2 = true; });
            REQUIRE(!called1);
            REQUIRE(!called2);
            REQUIRE(!called3);
            REQUIRE(!called4);

            {
                ON_EXIT([&] { called3 = true; });
                REQUIRE(!called1);
                REQUIRE(!called2);
                REQUIRE(!called3);
                REQUIRE(!called4);
            }

            REQUIRE(!called1);
            REQUIRE(!called2);
            REQUIRE(called3);
            REQUIRE(!called4);

            {
                ON_EXIT([&] { called4 = true; });
                REQUIRE(!called1);
                REQUIRE(!called2);
                REQUIRE(called3);
                REQUIRE(!called4);
            }

            REQUIRE(!called1);
            REQUIRE(!called2);
            REQUIRE(called3);
            REQUIRE(called4);
        }

        REQUIRE(!called1);
        REQUIRE(called2);
        REQUIRE(called3);
        REQUIRE(called4);
    }

    REQUIRE(called1);
    REQUIRE(called2);
    REQUIRE(called3);
    REQUIRE(called4);
}

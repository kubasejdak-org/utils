/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2021, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <utils/property.hpp>

#include <catch2/catch.hpp>

#include <string>
#include <type_traits>

using namespace std::string_view_literals;

ADD_PROPERTY_TYPE(KeyA, Type1);
ADD_PROPERTY_TYPE_2(KeyA, KeyB, Type2);
ADD_PROPERTY_TYPE_3(KeyA, KeyB, KeyC, Type2);

TEST_CASE("Type properties depending on multiple keys", "[unit][property]")
{
    REQUIRE(std::is_same_v<utils::PropertyType<KeyA>, Type1>);
    REQUIRE(std::is_same_v<utils::PropertyType<KeyA, KeyB>, Type2>);
    REQUIRE(std::is_same_v<utils::PropertyType<KeyA, KeyB, KeyC>, Type2>);
}

ADD_PROPERTY(KeyD, "ValueA"sv);
ADD_PROPERTY_2(KeyD, KeyE, "ValueB"sv);
ADD_PROPERTY_3(KeyD, KeyE, KeyF, "ValueC"sv);

TEST_CASE("Value properties depending on multiple keys", "[unit][property]")
{
    REQUIRE(utils::cPropertyValue<KeyD> == "ValueA"sv);
    REQUIRE(utils::cPropertyValue<KeyD, KeyE> == "ValueB"sv);
    REQUIRE(utils::cPropertyValue<KeyD, KeyE, KeyF> == "ValueC"sv);
}

ADD_PROPERTY_TYPE(BoardType, RaspberryPi);
ADD_PROPERTY(BoardName, "RaspberryPi");

ADD_PROPERTY_2(RaspberryPi, SpiA, "spi0"sv);
ADD_PROPERTY_2(RaspberryPi, SpiB, "spi1"sv);
ADD_PROPERTY_2(Cmpc30, SpiA, "spi2"sv);
ADD_PROPERTY_2(Cmpc30, SpiB, "spi3"sv);

TEST_CASE("Properties used in real use case with boards ans SPI configuration", "[unit][property]")
{
    using Board = utils::PropertyType<BoardType>;

    REQUIRE(utils::cPropertyValue<BoardName> == "RaspberryPi"sv);
    REQUIRE(utils::cPropertyValue<Board, SpiA> == "spi0"sv);
    REQUIRE(utils::cPropertyValue<Board, SpiB> == "spi1"sv);
}

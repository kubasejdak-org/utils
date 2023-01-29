/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2023, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <utils/types/property.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <type_traits>

ADD_PROPERTY_TYPE(KeyA, Type1);
ADD_PROPERTY_TYPE_2(KeyA, KeyB, Type2);
ADD_PROPERTY_TYPE_3(KeyA, KeyB, KeyC, Type2);

TEST_CASE("1. Type properties depending on multiple keys", "[unit][property]")
{
    REQUIRE(std::is_same_v<utils::types::PropertyType<KeyA>, Type1>);
    REQUIRE(std::is_same_v<utils::types::PropertyType<KeyA, KeyB>, Type2>);
    REQUIRE(std::is_same_v<utils::types::PropertyType<KeyA, KeyB, KeyC>, Type2>);
}

ADD_PROPERTY(KeyD, "ValueA");
ADD_PROPERTY_2(KeyD, KeyE, "ValueB");
ADD_PROPERTY_3(KeyD, KeyE, KeyF, "ValueC");

TEST_CASE("2. Value properties depending on multiple keys", "[unit][property]")
{
    REQUIRE_THAT(utils::types::cPropertyValue<KeyD>, Catch::Matchers::Equals("ValueA"));
    REQUIRE_THAT((utils::types::cPropertyValue<KeyD, KeyE>), Catch::Matchers::Equals("ValueB"));
    REQUIRE_THAT((utils::types::cPropertyValue<KeyD, KeyE, KeyF>), Catch::Matchers::Equals("ValueC"));
}

ADD_PROPERTY_TYPE(BoardType, RaspberryPi);
ADD_PROPERTY(BoardName, "RaspberryPi");

ADD_PROPERTY_2(RaspberryPi, SpiA, "spi0");
ADD_PROPERTY_2(RaspberryPi, SpiB, "spi1");
ADD_PROPERTY_2(Cmpc30, SpiA, "spi2");
ADD_PROPERTY_2(Cmpc30, SpiB, "spi3");

TEST_CASE("3. Properties used in real use case with boards ans SPI configuration", "[unit][property]")
{
    using Board = utils::types::PropertyType<BoardType>;

    REQUIRE_THAT(utils::types::cPropertyValue<BoardName>, Catch::Matchers::Equals("RaspberryPi"));
    REQUIRE_THAT((utils::types::cPropertyValue<Board, SpiA>), Catch::Matchers::Equals("spi0"));
    REQUIRE_THAT((utils::types::cPropertyValue<Board, SpiB>), Catch::Matchers::Equals("spi1"));
}

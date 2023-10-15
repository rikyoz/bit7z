// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include <internal/util.hpp>

using bit7z::check_overflow;
constexpr auto kMaxValue = ( std::numeric_limits< int64_t >::max )();
constexpr auto kMinValue = ( std::numeric_limits< int64_t >::min )();

TEST_CASE( "util: Calling check_overflow on a non-overflowing offset", "[util][check_overflow]" ) { //-V2008
    REQUIRE_FALSE( check_overflow( kMaxValue, 0 ) );
    REQUIRE_FALSE( check_overflow( kMaxValue, -1 ) );
    REQUIRE_FALSE( check_overflow( kMaxValue, -42 ) );
    REQUIRE_FALSE( check_overflow( kMaxValue, kMinValue ) );

    REQUIRE_FALSE( check_overflow( 42, 42 ) );
    REQUIRE_FALSE( check_overflow( 42, 1 ) );
    REQUIRE_FALSE( check_overflow( 42, 0 ) );
    REQUIRE_FALSE( check_overflow( 42, -1 ) );
    REQUIRE_FALSE( check_overflow( 42, -42 ) );
    REQUIRE_FALSE( check_overflow( 42, kMinValue ) );

    REQUIRE_FALSE( check_overflow( 1, 42 ) );
    REQUIRE_FALSE( check_overflow( 1, 1 ) );
    REQUIRE_FALSE( check_overflow( 1, 0 ) );
    REQUIRE_FALSE( check_overflow( 1, -1 ) );
    REQUIRE_FALSE( check_overflow( 1, -42 ) );
    REQUIRE_FALSE( check_overflow( 1, kMinValue ) );

    REQUIRE_FALSE( check_overflow( 0, kMaxValue ) );
    REQUIRE_FALSE( check_overflow( 0, 42 ) );
    REQUIRE_FALSE( check_overflow( 0, 1 ) );
    REQUIRE_FALSE( check_overflow( 0, 0 ) );
    REQUIRE_FALSE( check_overflow( 0, -1 ) );
    REQUIRE_FALSE( check_overflow( 0, -42 ) );
    REQUIRE_FALSE( check_overflow( 0, kMinValue ) );

    REQUIRE_FALSE( check_overflow( -1, kMaxValue ) );
    REQUIRE_FALSE( check_overflow( -1, 42 ) );
    REQUIRE_FALSE( check_overflow( -1, 1 ) );
    REQUIRE_FALSE( check_overflow( -1, 0 ) );
    REQUIRE_FALSE( check_overflow( -1, -1 ) );
    REQUIRE_FALSE( check_overflow( -1, -42 ) );

    REQUIRE_FALSE( check_overflow( -42, kMaxValue ) );
    REQUIRE_FALSE( check_overflow( -42, 42 ) );
    REQUIRE_FALSE( check_overflow( -42, 1 ) );
    REQUIRE_FALSE( check_overflow( -42, 0 ) );
    REQUIRE_FALSE( check_overflow( -42, -1 ) );
    REQUIRE_FALSE( check_overflow( -42, -42 ) );

    REQUIRE_FALSE( check_overflow( kMinValue, 0 ) );
    REQUIRE_FALSE( check_overflow( kMinValue, 1 ) );
    REQUIRE_FALSE( check_overflow( kMinValue, 42 ) );
    REQUIRE_FALSE( check_overflow( kMinValue, kMaxValue ) );
}

TEST_CASE( "util: Calling check_overflow on an overflowing offset", "[util][check_overflow]" ) {
    REQUIRE( check_overflow( kMaxValue, kMaxValue ) );
    REQUIRE( check_overflow( kMaxValue, 42 ) );
    REQUIRE( check_overflow( kMaxValue, 1 ) );
    REQUIRE( check_overflow( 42, kMaxValue ) );
    REQUIRE( check_overflow( 1, kMaxValue ) );
    REQUIRE( check_overflow( -1, kMinValue ) );
    REQUIRE( check_overflow( -42, kMinValue ) );
    REQUIRE( check_overflow( kMinValue, -1 ) );
    REQUIRE( check_overflow( kMinValue, -42 ) );
    REQUIRE( check_overflow( kMinValue, kMinValue ) );
}
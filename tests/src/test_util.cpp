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

#define NARROWING_TEST_STR( str ) std::make_tuple( L##str, (str) )

TEST_CASE( "util: Narrowing wide string to std::string", "[util][narrow]" ) {
    using bit7z::narrow;

    REQUIRE( narrow( nullptr, 0 ).empty() );
    REQUIRE( narrow( nullptr, 42 ).empty() );
    REQUIRE( narrow( L"", 0 ).empty() );

    const wchar_t* test_input = nullptr;
    const char* test_output = nullptr;
    std::tie( test_input, test_output ) = GENERATE( table< const wchar_t*, const char* >(
        {
            NARROWING_TEST_STR( "" ),
            NARROWING_TEST_STR( "h" ),
            NARROWING_TEST_STR( "hello world!" ),
            NARROWING_TEST_STR( "supercalifragilistichespiralidoso" ),
            NARROWING_TEST_STR( "perché" ),
            NARROWING_TEST_STR( "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" ) // メタルガルルモン
        }
    ) );

    REQUIRE( narrow( test_input, wcsnlen( test_input, 128 ) ) == test_output );
}

#define WIDENING_TEST_STR( str ) std::make_tuple( (str), L##str )

TEST_CASE( "util: Widening narrow string to std::wstring", "[util][widen]" ) {
    using bit7z::widen;
    using std::make_tuple;

    const char* test_input = nullptr;
    const wchar_t* test_output = nullptr;
    std::tie( test_input, test_output ) = GENERATE( table< const char*, const wchar_t* >(
        {
            WIDENING_TEST_STR( "" ),
            WIDENING_TEST_STR( "h" ),
            WIDENING_TEST_STR( "hello world!" ),
            WIDENING_TEST_STR( "supercalifragilistichespiralidoso" ),
            WIDENING_TEST_STR( "perché" ),
            WIDENING_TEST_STR( "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" ) // メタルガルルモン
        }
    ) );

    REQUIRE( widen( test_input ) == test_output );
}

using bit7z::check_overflow;
constexpr auto max_value = ( std::numeric_limits< int64_t >::max )();
constexpr auto min_value = ( std::numeric_limits< int64_t >::min )();

TEST_CASE( "util: Calling check_overflow on a non-overflowing offset", "[util][check_overflow]" ) {
    REQUIRE_FALSE( check_overflow( max_value, 0 ) );
    REQUIRE_FALSE( check_overflow( max_value, -1 ) );
    REQUIRE_FALSE( check_overflow( max_value, -42 ) );
    REQUIRE_FALSE( check_overflow( max_value, min_value ) );

    REQUIRE_FALSE( check_overflow( 42, 42 ) );
    REQUIRE_FALSE( check_overflow( 42, 1 ) );
    REQUIRE_FALSE( check_overflow( 42, 0 ) );
    REQUIRE_FALSE( check_overflow( 42, -1 ) );
    REQUIRE_FALSE( check_overflow( 42, -42 ) );
    REQUIRE_FALSE( check_overflow( 42, min_value ) );

    REQUIRE_FALSE( check_overflow( 1, 42 ) );
    REQUIRE_FALSE( check_overflow( 1, 1 ) );
    REQUIRE_FALSE( check_overflow( 1, 0 ) );
    REQUIRE_FALSE( check_overflow( 1, -1 ) );
    REQUIRE_FALSE( check_overflow( 1, -42 ) );
    REQUIRE_FALSE( check_overflow( 1, min_value ) );

    REQUIRE_FALSE( check_overflow( 0, max_value ) );
    REQUIRE_FALSE( check_overflow( 0, 42 ) );
    REQUIRE_FALSE( check_overflow( 0, 1 ) );
    REQUIRE_FALSE( check_overflow( 0, 0 ) );
    REQUIRE_FALSE( check_overflow( 0, -1 ) );
    REQUIRE_FALSE( check_overflow( 0, -42 ) );
    REQUIRE_FALSE( check_overflow( 0, min_value ) );

    REQUIRE_FALSE( check_overflow( -1, max_value ) );
    REQUIRE_FALSE( check_overflow( -1, 42 ) );
    REQUIRE_FALSE( check_overflow( -1, 1 ) );
    REQUIRE_FALSE( check_overflow( -1, 0 ) );
    REQUIRE_FALSE( check_overflow( -1, -1 ) );
    REQUIRE_FALSE( check_overflow( -1, -42 ) );

    REQUIRE_FALSE( check_overflow( -42, max_value ) );
    REQUIRE_FALSE( check_overflow( -42, 42 ) );
    REQUIRE_FALSE( check_overflow( -42, 1 ) );
    REQUIRE_FALSE( check_overflow( -42, 0 ) );
    REQUIRE_FALSE( check_overflow( -42, -1 ) );
    REQUIRE_FALSE( check_overflow( -42, -42 ) );

    REQUIRE_FALSE( check_overflow( min_value, 0 ) );
    REQUIRE_FALSE( check_overflow( min_value, 1 ) );
    REQUIRE_FALSE( check_overflow( min_value, 42 ) );
    REQUIRE_FALSE( check_overflow( min_value, max_value ) );
}

TEST_CASE( "util: Calling check_overflow on an overflowing offset", "[util][check_overflow]" ) {
    REQUIRE( check_overflow( max_value, max_value ) );
    REQUIRE( check_overflow( max_value, 42 ) );
    REQUIRE( check_overflow( max_value, 1 ) );
    REQUIRE( check_overflow( 42, max_value ) );
    REQUIRE( check_overflow( 1, max_value ) );
    REQUIRE( check_overflow( -1, min_value ) );
    REQUIRE( check_overflow( -42, min_value ) );
    REQUIRE( check_overflow( min_value, -1 ) );
    REQUIRE( check_overflow( min_value, -42 ) );
    REQUIRE( check_overflow( min_value, min_value ) );
}
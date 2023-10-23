// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <catch2/catch.hpp>

#include <internal/stringutil.hpp>

#if !defined( _WIN32 ) || ( !defined( BIT7Z_USE_NATIVE_STRING ) && !defined( BIT7Z_USE_SYSTEM_CODEPAGE ) )

#define NARROWING_TEST_STR( str ) std::make_tuple( L##str, (str) )

TEST_CASE( "util: Narrowing wide string to std::string", "[stringutil][narrow]" ) {
    using bit7z::narrow;

    SECTION( "Converting from nullptr C wide string" ) {
        REQUIRE( narrow( nullptr, 0 ).empty() );
        REQUIRE( narrow( nullptr, 42 ).empty() );
    }

#if !defined( _LIBCPP_VERSION )
    SECTION( "Converting wide strings with unencodable UTF-8 chars" ) {
        std::wstring testInput = L"\xDC80";
        std::string testOutput = narrow( testInput.c_str(), testInput.size() );
#if defined( _MSC_VER ) || !defined( BIT7Z_USE_STANDARD_FILESYSTEM )
        REQUIRE( testOutput == "\uFFFD" );
#else
        REQUIRE( testOutput == "\xED\xB2\x80" );
#endif

        testInput = L"\xD843";
        testOutput = narrow( testInput.c_str(), testInput.size() );
#if defined( _MSC_VER ) || !defined( BIT7Z_USE_STANDARD_FILESYSTEM )
        REQUIRE( testOutput == "\uFFFD" );
#else
        REQUIRE( testOutput == "\xED\xA1\x83" );
#endif
    }
#endif

    SECTION( "Converting wide strings without unencodable UTF-8 characters" ) {
        std::wstring testInput;
        std::string testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const wchar_t*, const char* >(
            {
                NARROWING_TEST_STR( "" ),
                NARROWING_TEST_STR( "h" ),
                NARROWING_TEST_STR( "hello world!" ),
                NARROWING_TEST_STR( "supercalifragilistichespiralidoso" ),
                NARROWING_TEST_STR( "perché" ),
                NARROWING_TEST_STR( "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" ) // メタルガルルモン
            }
        ) );

        DYNAMIC_SECTION( "Converting L\"" << testOutput << "\" to narrow string" ) {
            REQUIRE( narrow( testInput.c_str(), testInput.size() ) == testOutput );
        }
    }
}

#define WIDENING_TEST_STR( str ) std::make_tuple( (str), L##str )

TEST_CASE( "util: Widening narrow string to std::wstring", "[stringutil][widen]" ) {
    using bit7z::widen;
    using std::make_tuple;

    std::string testInput;
    std::wstring testOutput;
    std::tie( testInput, testOutput ) = GENERATE( table< const char*, const wchar_t* >(
        {
            WIDENING_TEST_STR( "" ),
            WIDENING_TEST_STR( "h" ),
            WIDENING_TEST_STR( "hello world!" ),
            WIDENING_TEST_STR( "supercalifragilistichespiralidoso" ),
            WIDENING_TEST_STR( "perché" ),
            WIDENING_TEST_STR( "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" ) // メタルガルルモン
        }
    ) );

    DYNAMIC_SECTION( "Converting \"" << testInput << "\" to wide string" ) {
        REQUIRE( widen( testInput ) == testOutput );
    }
}

#endif
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

#include <string>
#include <sstream>

#if !defined( _WIN32 ) || ( !defined( BIT7Z_USE_NATIVE_STRING ) && !defined( BIT7Z_USE_SYSTEM_CODEPAGE ) )
#include <tuple>

#define NARROWING_TEST_STR( str ) std::make_tuple( L##str, (str) )

namespace {
auto toHexString( const std::wstring& str ) -> std::string {
    std::ostringstream oss;
    oss << std::hex;
    for ( const wchar_t character : str ) {
        oss << "\\x" << std::uppercase << static_cast< std::uint32_t >( character ) << std::nouppercase;
    }
    return oss.str();
}
}

TEST_CASE( "util: Narrowing wide string to std::string", "[stringutil][narrow]" ) {
    using bit7z::narrow;

    SECTION( "Converting from nullptr C wide string" ) {
        REQUIRE( narrow( nullptr, 0 ).empty() );
        REQUIRE( narrow( nullptr, 42 ).empty() );
    }

    SECTION( "Converting wide strings with only one unencodable UTF-8" ) {
        const std::wstring testInput = GENERATE(
            L"\xD800", // Lone high surrogates.
            L"\xD801",
            L"\xD83D",
            L"\xDA80",
            L"\xDBFE",
            L"\xDBFF",
            L"\xDC00", // Lone low surrogates.
            L"\xDC01",
            L"\xDC80",
            L"\xDE02",
            L"\xDFFE",
            L"\xDFFF"
        );

        DYNAMIC_SECTION( "Converting L\"" << toHexString( testInput ) << "\" to narrow string" ) {
            const std::string testOutput = narrow( testInput.c_str(), testInput.size() );
            REQUIRE( testOutput == "\uFFFD" );
        }
    }

#ifndef _WIN32
    SECTION( "Converting wide strings with multiple unencodable UTF-8" ) {
        const std::wstring testInput = GENERATE(
            L"\xD83D\xD83D", // Two high surrogates.
            L"\xDE02\xDE02", // Two low surrogates.
            L"\xD800\xD83D\xDA80\xDBFF", // Only high surrogates.
            L"\xDC00\xDC80\xDE02\xDFFF", // Only low surrogates.
            L"\x110000", // Out-of-range Unicode characters.
            L"\x200000",
            L"\x200000\x300000",
            L"\x200000\x300000\x400000",
            L"\xD83D\x200000", // Mixed lone surrogate and out-of-range Unicode character.
            L"\uFFFD", // Replacement character.
            L"\uFFFD\uFFFD\uFFFD"
        );

        DYNAMIC_SECTION( "Converting L\"" << toHexString( testInput ) << "\" to narrow string" ) {
            const std::string testOutput = narrow( testInput.c_str(), testInput.size() );
            REQUIRE( testOutput == "\uFFFD" );
        }
    }
#endif

    SECTION( "Converting wide strings with at most two-bytes UTF-16 characters" ) {
        std::wstring testInput;
        std::string testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const wchar_t*, const char* >( {
            NARROWING_TEST_STR( "" ),
            NARROWING_TEST_STR( "h" ),
            NARROWING_TEST_STR( "Hello, World!" ),
            NARROWING_TEST_STR( "supercalifragilistichespiralidoso" ),
            NARROWING_TEST_STR( "ABC" ),
            NARROWING_TEST_STR( "perché" ),
            NARROWING_TEST_STR( "\u4E2D" ), // 中
            NARROWING_TEST_STR( "\u4E16\u754C" ), // 世界
            NARROWING_TEST_STR( "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" ), // メタルガルルモン
            NARROWING_TEST_STR( "English, \u65E5\u672C\u8A9E, \uD55C\uAD6D\uC5B4, "
                                "\u0627\u0644\u0639\u0631\u0628\u064A\u0629" ) // Long string with mixed characters.
        } ) );

        DYNAMIC_SECTION( "Converting L\"" << toHexString( testInput ) << "\" to narrow string" ) {
            REQUIRE( narrow( testInput.c_str(), testInput.size() ) == testOutput );
        }
    }

    SECTION( "Converting wide strings containing UTF-16 surrogates" ) {
        std::wstring testInput;
        std::string testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const wchar_t*, const char* >( {
            // U+10042 = 𐁂
            std::make_tuple( L"\xD800\xDC42", "\xF0\x90\x81\x82" ),
            // U+1F602 = 😂
            std::make_tuple( L"\xD83D\xDE02", "\xF0\x9F\x98\x82" ),
            // U+24B62 = 𤭢
            std::make_tuple( L"\xD852\xDF62", "\xF0\xA4\xAD\xA2" ),
            // Mixed characters: Aé中😂
            std::make_tuple( L"A\x00E9\x4E2D\xD83D\xDE02", "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x82" ),
            // Mixed characters: Hello 世界😊!
            std::make_tuple( L"Hello \x4E16\x754C \xD83D\xDE0A!", "Hello \xE4\xB8\x96\xE7\x95\x8C \xF0\x9F\x98\x8A!" ),
            // Mixed special characters: ¡§©®™𝄞€£¥»¿
            std::make_tuple( L"\xA1\xA7\xA9\xAE\x2122\xD834\xDD1E\x20AC\xA3\xA5\xBB\xBF",
                             "\xC2\xA1\xC2\xA7\xC2\xA9\xC2\xAE\xE2\x84\xA2\xF0\x9D\x84"
                             "\x9E\xE2\x82\xAC\xC2\xA3\xC2\xA5\xC2\xBB\xC2\xBF" ),
            // Invalid surrogate at the start of the string.
            std::make_tuple( L"\xD83D" L"A\xE9", "\xEF\xBF\xBD""A\xC3\xA9" ),
            // Invalid surrogate at the end of the string.
            std::make_tuple( L"\xE9" L"A\xD83D", "\xC3\xA9""A\xEF\xBF\xBD" )
        } ) );

        DYNAMIC_SECTION( "Converting L\"" << toHexString( testInput ) << "\" to narrow string" ) {
            REQUIRE( narrow( testInput.c_str(), testInput.size() ) == testOutput );
        }
    }

    SECTION( "Boundary value analysis" ) {
        std::wstring testInput;
        std::string testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const wchar_t*, const char* >( {
            // U+0000 ... U+007F
            std::make_tuple( L"\x0000", "\x00" ),
            std::make_tuple( L"\x0001", "\x01" ),
            std::make_tuple( L"\x007E", "\x7E" ),
            std::make_tuple( L"\x007F", "\x7F" ),
            // U+0080 ... U+07FF
            std::make_tuple( L"\x0080", "\xC2\x80" ),
            std::make_tuple( L"\x0081", "\xC2\x81" ),
            std::make_tuple( L"\x07FE", "\xDF\xBE" ),
            std::make_tuple( L"\x07FF", "\xDF\xBF" ),
            // U+0800 ... U+FFFF
            std::make_tuple( L"\x0800", "\xE0\xA0\x80" ),
            std::make_tuple( L"\x0801", "\xE0\xA0\x81" ),
            std::make_tuple( L"\xD7FF", "\xED\x9F\xBF" ), // Before surrogate range [0xD800, 0xDFFF].
            // BVA checks within the surrogate range is performed in the tests for the surrogate characters.
            std::make_tuple( L"\xE000", "\xEE\x80\x80" ), // After surrogate range [0xD800, 0xDFFF].
            std::make_tuple( L"\xFFFE", "\xEF\xBF\xBE" ),
            std::make_tuple( L"\xFFFF", "\xEF\xBF\xBF" ),
            // U+10000 ... U+10FFFF
            std::make_tuple( L"\U00010000", "\xF0\x90\x80\x80" ),
            std::make_tuple( L"\U00010001", "\xF0\x90\x80\x81" ),
            std::make_tuple( L"\U0010FFFE", "\xF4\x8F\xBF\xBE" ),
            std::make_tuple( L"\U0010FFFF", "\xF4\x8F\xBF\xBF" )
        } ) );

        DYNAMIC_SECTION( "Converting L\"" << toHexString( testInput ) << "\" to narrow string" ) {
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
            WIDENING_TEST_STR( "Hello, World!" ),
            WIDENING_TEST_STR( "supercalifragilistichespiralidoso" ),
            WIDENING_TEST_STR( "ABC" ),
            WIDENING_TEST_STR( "perché" ),
            WIDENING_TEST_STR( "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" ) // メタルガルルモン
        }
    ) );

    DYNAMIC_SECTION( "Converting \"" << testInput << "\" to wide string" ) {
        REQUIRE( widen( testInput ) == testOutput );
    }
}

#endif

TEST_CASE( "util: ends_with", "[stringutil][ends_with]" ) {
    using bit7z::ends_with;
    using bit7z::tstring;

    const tstring emptyTestString;
    REQUIRE( ends_with( emptyTestString, BIT7Z_STRING( "" ) ) );
    REQUIRE_FALSE( ends_with( emptyTestString, BIT7Z_STRING( " " ) ) );
    REQUIRE_FALSE( ends_with( emptyTestString, BIT7Z_STRING( "h" ) ) );
    REQUIRE_FALSE( ends_with( emptyTestString, BIT7Z_STRING( "hello world!" ) ) );
    REQUIRE( ends_with( emptyTestString, tstring{ BIT7Z_STRING( "" ) } ) );
    REQUIRE_FALSE( ends_with( emptyTestString, tstring{ BIT7Z_STRING( " " ) } ) );
    REQUIRE_FALSE( ends_with( emptyTestString, tstring{ BIT7Z_STRING( "h" ) } ) );
    REQUIRE_FALSE( ends_with( emptyTestString, tstring{ BIT7Z_STRING( "hello world!" ) } ) );

    const tstring oneCharTestString{ BIT7Z_STRING( "a" ) };
    REQUIRE( ends_with( oneCharTestString, BIT7Z_STRING( "" ) ) );
    REQUIRE( ends_with( oneCharTestString, BIT7Z_STRING( "a" ) ) );
    REQUIRE_FALSE( ends_with( oneCharTestString, BIT7Z_STRING( "h" ) ) );
    REQUIRE_FALSE( ends_with( oneCharTestString, BIT7Z_STRING( "hello world!" ) ) );
    REQUIRE( ends_with( oneCharTestString, tstring{ BIT7Z_STRING( "" ) } ) );
    REQUIRE( ends_with( oneCharTestString, tstring{ BIT7Z_STRING( "a" ) } ) );
    REQUIRE_FALSE( ends_with( oneCharTestString, tstring{ BIT7Z_STRING( "h" ) } ) );
    REQUIRE_FALSE( ends_with( oneCharTestString, tstring{ BIT7Z_STRING( "hello world!" ) } ) );

    const tstring testString{ BIT7Z_STRING( "hello world!" ) };
    REQUIRE_FALSE( ends_with( testString, BIT7Z_STRING( "a" ) ) );
    REQUIRE_FALSE( ends_with( testString, BIT7Z_STRING( "h" ) ) );
    REQUIRE_FALSE( ends_with( testString, BIT7Z_STRING( "b!" ) ) );
    REQUIRE_FALSE( ends_with( testString, BIT7Z_STRING( "hello" ) ) );
    REQUIRE_FALSE( ends_with( testString, BIT7Z_STRING( "oworld!" ) ) );
    REQUIRE_FALSE( ends_with( testString, BIT7Z_STRING( "hello world" ) ) );
    REQUIRE_FALSE( ends_with( testString, BIT7Z_STRING( "Hello World!" ) ) );
    REQUIRE_FALSE( ends_with( testString, BIT7Z_STRING( "hello world, hello world!" ) ) );
    REQUIRE( ends_with( testString, BIT7Z_STRING( "" ) ) );
    REQUIRE( ends_with( testString, BIT7Z_STRING( "!" ) ) );
    REQUIRE( ends_with( testString, BIT7Z_STRING( " world!" ) ) );
    REQUIRE( ends_with( testString, BIT7Z_STRING( "hello world!" ) ) );
    REQUIRE_FALSE( ends_with( testString, tstring{ BIT7Z_STRING( "a" ) } ) );
    REQUIRE_FALSE( ends_with( testString, tstring{ BIT7Z_STRING( "h" ) } ) );
    REQUIRE_FALSE( ends_with( testString, tstring{ BIT7Z_STRING( "b!" ) } ) );
    REQUIRE_FALSE( ends_with( testString, tstring{ BIT7Z_STRING( "hello" ) } ) );
    REQUIRE_FALSE( ends_with( testString, tstring{ BIT7Z_STRING( "oworld!" ) } ) );
    REQUIRE_FALSE( ends_with( testString, tstring{ BIT7Z_STRING( "hello world" ) } ) );
    REQUIRE_FALSE( ends_with( testString, tstring{ BIT7Z_STRING( "Hello World!" ) } ) );
    REQUIRE_FALSE( ends_with( testString, tstring{ BIT7Z_STRING( "hello world, hello world!" ) } ) );
    REQUIRE( ends_with( testString, tstring{ BIT7Z_STRING( "" ) } ) );
    REQUIRE( ends_with( testString, tstring{ BIT7Z_STRING( "!" ) } ) );
    REQUIRE( ends_with( testString, tstring{ BIT7Z_STRING( " world!" ) } ) );
    REQUIRE( ends_with( testString, tstring{ BIT7Z_STRING( "hello world!" ) } ) );
}
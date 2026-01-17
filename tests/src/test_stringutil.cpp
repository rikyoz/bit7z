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

#include <iomanip>
#include <string>
#include <sstream>

#if !defined( _WIN32 ) || ( !defined( BIT7Z_USE_NATIVE_STRING ) && !defined( BIT7Z_USE_SYSTEM_CODEPAGE ) )
#include <tuple>

#define NARROWING_TEST_STR( str ) std::make_tuple( L##str, (str) )

namespace {

template< typename CharT >
using is_char = std::integral_constant< bool,
                                        std::is_same< CharT, char >::value ||
                                        std::is_same< CharT, char16_t >::value ||
                                        std::is_same< CharT, char32_t >::value ||
                                        std::is_same< CharT, wchar_t >::value >;

template< typename CharT, typename = typename std::enable_if< is_char< CharT >::value >::type >
struct AsciiTrait {
    static constexpr auto kMinPrintableAscii = static_cast< CharT >( 0x20 );
    static constexpr auto kMaxPrintableAscii = static_cast< CharT >( 0x7E );
    static constexpr int kMaxAscii = 0x7F;

    static constexpr auto isPrintableAscii( CharT character ) -> bool {
        return kMinPrintableAscii <= character && character <= kMaxPrintableAscii;
    }
};

BIT7Z_ALWAYS_INLINE
constexpr auto toCodeunit( char character ) noexcept -> std::uint8_t {
    return static_cast< std::uint8_t >( character );
}

BIT7Z_ALWAYS_INLINE
constexpr auto toCodeunit( wchar_t character ) noexcept -> std::uint32_t {
    return static_cast< std::uint32_t >( character );
}

template< typename CharT >
auto toHex( CharT character, bool keepPrintableAscii = true ) -> std::string {
    std::ostringstream oss;
    if ( keepPrintableAscii && AsciiTrait< CharT >::isPrintableAscii( character ) ) {
        oss << static_cast< char >( character );
    } else {
        oss << "\\x" << std::hex << std::setw( 2 * sizeof( CharT ) ) << std::setfill( '0' ) << std::uppercase
            << +toCodeunit( character );
    }
    return oss.str();
}

template<typename CharT>
auto toHexString( CharT character, bool keepPrintableAscii = true ) -> std::string {
    return toHex( character, keepPrintableAscii );
}

template<typename CharT>
auto toHexString( const std::basic_string< CharT >& str, bool keepPrintableAscii = true ) -> std::string {
    std::string result;
    result.reserve( str.size() );
    for ( const CharT character : str ) {
        result += toHex( character, keepPrintableAscii );
    }
    return result;
}
}

TEST_CASE( "util: Narrowing wide string to std::string", "[stringutil][narrow]" ) {
    using bit7z::narrow;

    SECTION( "Converting from nullptr C wide string" ) {
        REQUIRE( narrow( nullptr, 0 ).empty() );
        REQUIRE( narrow( nullptr, 42 ).empty() );
    }

    SECTION( "Converting wide strings with a lone surrogate (invalid in UTF-8)" ) {
        const auto testInput = GENERATE( as< std::wstring >(),
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
    SECTION( "Converting wide strings with multiple UTF-16 codeunits invalid in UTF-8" ) {
        const std::wstring testInput = GENERATE(
            L"\xD83D\xD83D", // Two high surrogates.
            L"\xDE02\xDE02", // Two low surrogates.
            L"\xD800\xD83D\xDA80\xDBFF", // Only high surrogates.
            L"\xDC00\xDC80\xDE02\xDFFF", // Only low surrogates.
            L"\xDE02\xD83D", // Low surrogate before a high surrogate.
            L"\xDFFF\xD800",
            L"\x110000\x200000", // Multiple out-of-range Unicode characters.
            L"\x200000\x300000\x400000",
            L"\x300000\x400000\x500000\x600000",
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

    SECTION( "Converting wide strings contain a single ASCII character" ) {
        for ( int ascii = 0; ascii <= AsciiTrait< wchar_t >::kMaxAscii; ++ascii ) {
            const auto character = static_cast< wchar_t >( ascii );
            DYNAMIC_SECTION( "Converting L\"" << toHexString( character ) << "\" to narrow string" ) {
                const auto output = narrow( &character, 1 );
                REQUIRE( output.size() == 1 );
                REQUIRE( output.front() == static_cast< char >( ascii ) );
            }
        }
    }

    SECTION( "Converting wide strings to UTF-8 (no surrogate pairs)" ) {
        std::wstring testInput;
        std::string testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const wchar_t*, const char* >( {
            NARROWING_TEST_STR( "" ),
            NARROWING_TEST_STR( "Hello, World!" ),
            NARROWING_TEST_STR( "supercalifragilistichespiralidoso" ),
            NARROWING_TEST_STR( "ABC" ),
            NARROWING_TEST_STR( "perché" ),
            NARROWING_TEST_STR( "κόσμε" ),
            NARROWING_TEST_STR( "\u2010" ), // Hyphen ‐
            NARROWING_TEST_STR( "\u4E08" ), // 丈
            NARROWING_TEST_STR( "\u4E16\u754C" ), // 世界
            NARROWING_TEST_STR( "\uE000" ),
            NARROWING_TEST_STR( "\uFFFD" ), // Replacement character U+FFFD
            NARROWING_TEST_STR( "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" ), // メタルガルルモン
            // Long string with mixed characters (no UTF-16 surrogates in input string).
            NARROWING_TEST_STR(
                "English, \u65E5\u672C\u8A9E, \uD55C\uAD6D\uC5B4, \u0627\u0644\u0639\u0631\u0628\u064A\u0629"
            )
        } ) );

        DYNAMIC_SECTION( "Converting L\"" << toHexString( testInput ) << "\" to narrow string" ) {
            REQUIRE( narrow( testInput.c_str(), testInput.size() ) == testOutput );
        }
    }

    SECTION( "Converting wide strings containing UTF-16 surrogates to UTF-8" ) {
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
            // Mixed characters: Hello 世界 😊!
            std::make_tuple( L"Hello \x4E16\x754C \xD83D\xDE0A!", "Hello \xE4\xB8\x96\xE7\x95\x8C \xF0\x9F\x98\x8A!" ),
            // Mixed special characters: ¡§©®™𝄞€£¥»¿
            std::make_tuple( L"\xA1\xA7\xA9\xAE\x2122\xD834\xDD1E\x20AC\xA3\xA5\xBB\xBF",
                             "\xC2\xA1\xC2\xA7\xC2\xA9\xC2\xAE\xE2\x84\xA2\xF0\x9D\x84"
                             "\x9E\xE2\x82\xAC\xC2\xA3\xC2\xA5\xC2\xBB\xC2\xBF" ),
            // Invalid surrogate at the start of the string.
            std::make_tuple( L"\xD800" L"A\xE9", "\xEF\xBF\xBD""A\xC3\xA9" ),
            std::make_tuple( L"\xDC42" L"A\xE9", "\xEF\xBF\xBD""A\xC3\xA9" ),
            // Invalid surrogate in the middle of the string.
            std::make_tuple( L"ABC\xD83D" L"DEF", "ABC\xEF\xBF\xBD""DEF" ),
            std::make_tuple( L"ABC\xDE02" L"DEF", "ABC\xEF\xBF\xBD""DEF" ),
            // Invalid surrogate at the end of the string.
            std::make_tuple( L"\xE9" L"A\xD852", "\xC3\xA9""A\xEF\xBF\xBD" ),
            std::make_tuple( L"\xE9" L"A\xDF62", "\xC3\xA9""A\xEF\xBF\xBD" )
        } ) );

        DYNAMIC_SECTION( "Converting L\"" << toHexString( testInput ) << "\" to narrow string" ) {
            REQUIRE( narrow( testInput.c_str(), testInput.size() ) == testOutput );
        }
    }

#ifndef _WIN32
    SECTION( "Converting UTF-32 wide strings to UTF-8" ) {
        // On Linux and macOS, we support both UTF-32/UTF-16 codeunits in wide strings.
        std::wstring testInput;
        std::string testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const wchar_t*, const char* >( {
            NARROWING_TEST_STR( "\U00010000" ),
            NARROWING_TEST_STR( "\U00010001" ),
            NARROWING_TEST_STR( "\U00010042" ), // U+10042 = 𐁂
            NARROWING_TEST_STR( "\U0001F602" ), // U+1F602 = 😂
            NARROWING_TEST_STR( "\U00024B62" ), // U+24B62 = 𤭢
            NARROWING_TEST_STR( "\U0010FFFE" ),
            NARROWING_TEST_STR( "\U0010FFFF" )
        } ) );

        DYNAMIC_SECTION( "Converting L\"" << toHexString( testInput ) << "\" to narrow string" ) {
            REQUIRE( narrow( testInput.c_str(), testInput.size() ) == testOutput );
        }
    }
#endif

    SECTION( "Boundary value analysis" ) {
        std::wstring testInput;
        std::string testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const wchar_t*, const char* >( {
            // U+0000 ... U+007F
            NARROWING_TEST_STR( "\u0000" ),
            NARROWING_TEST_STR( "\u0001" ),
            NARROWING_TEST_STR( "\u007E" ),
            NARROWING_TEST_STR( "\u007F" ),
            // U+0080 ... U+07FF
            NARROWING_TEST_STR( "\u0080" ),
            NARROWING_TEST_STR( "\u0081" ),
            NARROWING_TEST_STR( "\u07FE" ),
            NARROWING_TEST_STR( "\u07FF" ),
            // U+0800 ... U+FFFF
            NARROWING_TEST_STR( "\u0800" ),
            NARROWING_TEST_STR( "\u0801" ),
            NARROWING_TEST_STR( "\uD7FE" ),
            NARROWING_TEST_STR( "\uD7FF" ), // Before surrogate range [0xD800, 0xDFFF].
            // BVA checks within the surrogate range is performed in the tests for the surrogate characters.
            NARROWING_TEST_STR( "\uE000" ), // After surrogate range [0xD800, 0xDFFF].
            NARROWING_TEST_STR( "\uE001" ),
            NARROWING_TEST_STR( "\uFFFE" ),
            NARROWING_TEST_STR( "\uFFFF" ),
            // U+10000 ... U+10FFFF
            std::make_tuple( L"\xD800\xDC00", "\xF0\x90\x80\x80" ),
            std::make_tuple( L"\xD800\xDC01", "\xF0\x90\x80\x81" ),
            std::make_tuple( L"\xDBFF\xDFFE", "\xF4\x8F\xBF\xBE" ),
            std::make_tuple( L"\xDBFF\xDFFF", "\xF4\x8F\xBF\xBF" ),
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

#ifndef _WIN32
    SECTION( "Converting UTF-8 strings containing byte sequences encoding UTF-16 surrogates (invalid in UTF-8)" ) {
        const std::string testInput = GENERATE(
            "\xED\xA0\x80", // Lone high surrogates (invalid both in UTF-8 and UTF-16).
            "\xED\xA0\x81",
            "\xED\xA0\xBD",
            "\xED\xAA\x80",
            "\xED\xAF\xBE",
            "\xED\xAF\xBF",
            "\xED\xB0\x80", // Lone low surrogates (invalid both in UTF-8 and UTF-16).
            "\xED\xB0\x81",
            "\xED\xB2\x80",
            "\xED\xB8\x82",
            "\xED\xBF\xBE",
            "\xED\xBF\xBF"
        );

        DYNAMIC_SECTION( "Converting \"" << toHexString( testInput ) << "\" to wide string" ) {
            const std::wstring testOutput = widen( testInput );
            REQUIRE( testOutput == L"\uFFFD" );
            REQUIRE( widen( "prefix" + testInput ) == L"prefix" + testOutput );
            REQUIRE( widen( testInput + "suffix" ) == testOutput + L"suffix" );
            REQUIRE( widen( "prefix" + testInput + "suffix" ) == L"prefix" + testOutput + L"suffix" );
        }
    }

    SECTION( "Converting UTF-8 strings containing invalid byte sequences" ) {
        std::string testInput;
        std::wstring testOutput;

        std::tie( testInput, testOutput ) = GENERATE( table< const char*, const wchar_t* >( {
            // Invalid UTF-8 bytes.
            std::make_tuple( "\xC0", L"\uFFFD" ),
            std::make_tuple( "\xC1", L"\uFFFD" ),
            std::make_tuple( "\xF5", L"\uFFFD" ), // Bytes form 0xF5 to 0xFF would encode 4-bytes sequences
            std::make_tuple( "\xF6", L"\uFFFD" ), // for codepoints outside the Unicode range, hence they are invalid.
            std::make_tuple( "\xF7", L"\uFFFD" ),
            std::make_tuple( "\xF8", L"\uFFFD" ),
            std::make_tuple( "\xF9", L"\uFFFD" ),
            std::make_tuple( "\xFA", L"\uFFFD" ),
            std::make_tuple( "\xFB", L"\uFFFD" ),
            std::make_tuple( "\xFC", L"\uFFFD" ),
            std::make_tuple( "\xFD", L"\uFFFD" ),
            std::make_tuple( "\xFE", L"\uFFFD" ),
            std::make_tuple( "\xFF", L"\uFFFD" ),
            // Surrogate pairs (invalid in UTF-8, but not in UTF-16).
            std::make_tuple( "\xED\xA0\x80\xED\xB0\x80", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xA0\x80\xED\xB1\x82", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xA0\x80\xED\xBF\xBF", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xA0\xBD\xED\xB8\x82", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xA1\x92\xED\xBD\xA2", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xAD\xBF\xED\xB0\x80", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xAD\xBF\xED\xBF\xBF", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xAE\x80\xED\xB0\x80", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xAE\x80\xED\xBF\xBF", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xAF\xBF\xED\xB0\x80", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xED\xAF\xBF\xED\xBF\xBF", L"\uFFFD\uFFFD" ),
            // Invalid continuation bytes.
            std::make_tuple( "\xC3\x28", L"\uFFFD\x28" ),
            std::make_tuple( "\xC3\xE8", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xC3\xFF", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\x28", L"\uFFFD\x28" ),
            std::make_tuple( "\xE2\x28\xA1", L"\uFFFD\x28\uFFFD" ),
            std::make_tuple( "\xE2\x82\x28", L"\uFFFD\x28" ),
            std::make_tuple( "\xE2\xC3\xA1", L"\uFFFD\xE1" ),
            std::make_tuple( "\xE2\x82\xC3", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\xE8\xA1", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\x82\xE8", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\xFF\xA1", L"\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\x82\xFF", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\x28\xE2\x28", L"\uFFFD\x28\uFFFD\x28" ),
            std::make_tuple( "\xF0\x28\x8C\xBC", L"\uFFFD\x28\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x80\x28\xBC", L"\uFFFD\x28\uFFFD" ),
            std::make_tuple( "\xF0\x80\x8C\x28", L"\uFFFD\x28" ),
            std::make_tuple( "\xF0\xC3\x8C\xBC", L"\uFFFD\xCC\uFFFD" ),
            std::make_tuple( "\xF0\x80\xC3\xBC", L"\uFFFD\xFC" ),
            std::make_tuple( "\xF0\x80\x8C\xC3", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\xE8\x8C\xBC", L"\uFFFD\u833C" ),
            std::make_tuple( "\xF0\x80\xE8\xBC", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x80\x8C\xE8", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\xFF\x8C\xBC", L"\uFFFD\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x80\xFF\xBC", L"\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x80\x8C\xFF", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x28\x8C\xBC\xFF", L"\uFFFD\x28\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\xC3\xE2\xF0", L"\uFFFD\uFFFD\uFFFD\uFFFD" ),
            // Invalid leading byte (continuation byte without leading byte).
            std::make_tuple( "\x80", L"\uFFFD" ),
            std::make_tuple( "\x81", L"\uFFFD" ),
            std::make_tuple( "\x9A", L"\uFFFD" ),
            std::make_tuple( "\xA0", L"\uFFFD" ),
            std::make_tuple( "\xA1", L"\uFFFD" ),
            std::make_tuple( "\xAF", L"\uFFFD" ),
            std::make_tuple( "\xB9", L"\uFFFD" ),
            std::make_tuple( "\xBE", L"\uFFFD" ),
            std::make_tuple( "\xBF", L"\uFFFD" ),
            std::make_tuple( "\x80\x81", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\x80\x81\x9A\xA0\xA1\xAF\xB9\xBE\xBF", // Many continuation bytes without a leading byte.
                             L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            // Invalid leading byte.
            std::make_tuple( "\xF5\x41", L"\uFFFD\x41" ),
            // Invalid surrogate at the start of the string.
            std::make_tuple( "\xED\xAA\x80""A\xC3\xA9", L"\uFFFD" L"A\xE9" ),
            std::make_tuple( "\xEF\xBF\xBD""A\xC3\xA9", L"\uFFFD" L"A\xE9" ),
            // Invalid surrogate at the end of the string.
            std::make_tuple( "\xC3\xA9""A\xED\xAA\x80", L"\xE9" L"A\uFFFD" ),
            std::make_tuple( "\xC3\xA9""A\xEF\xBF\xBD", L"\xE9" L"A\uFFFD" ),
            // Invalid surrogate in the middle of the string.
            std::make_tuple( "ABC\xEF\xBF\xBD""DEF", L"ABC\uFFFD" L"DEF" ),
            std::make_tuple( "ABC\xEF\xBF\xBD""DEF", L"ABC\uFFFD" L"DEF" ),
        } ) );

        DYNAMIC_SECTION( "Converting \"" << toHexString( testInput, false ) << "\" to wide string" ) {
            REQUIRE( widen( testInput ) == testOutput );
            REQUIRE( widen( "prefix" + testInput ) == L"prefix" + testOutput );
            REQUIRE( widen( testInput + "suffix" ) == testOutput + L"suffix" );
            REQUIRE( widen( "prefix" + testInput + "suffix" ) == L"prefix" + testOutput + L"suffix" );
        }
    }

    SECTION( "Converting UTF-8 strings containing truncated sequences" ) {
        std::string testInput;
        std::wstring testOutput;

        std::tie( testInput, testOutput ) = GENERATE( table< const char*, const wchar_t* >( {
            // Truncated 2-bytes sequences.
            std::make_tuple( "\xC3", L"\uFFFD" ),
            std::make_tuple( "\xC3\xC0", L"\uFFFD\uFFFD" ), // + invalid UTF-8 byte.
            std::make_tuple( "\xC3\xC1", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xC3\xF5", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xC3\xC2", L"\uFFFD\uFFFD" ), // + another truncated 2-bytes sequence.
            std::make_tuple( "\xC3\xC2\xA0", L"\uFFFD\xA0" ), // + a valid 2-bytes sequence.
            std::make_tuple( "\xC3\xE2", L"\uFFFD\uFFFD" ), // + a truncated 3-bytes sequence.
            std::make_tuple( "\xC3\xE2\x81", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xC3\xE2\x81\x82", L"\uFFFD\u2042" ), // + a valid 3-bytes sequence.
            std::make_tuple( "\xC3\xF0", L"\uFFFD\uFFFD" ), // + a truncated 4-bytes sequence.
            std::make_tuple( "\xC3\xF0\x90", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xC3\xF0\x90\x8D", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xC3\xF0\x90\x8D\x88", L"\uFFFD\xD800\xDF48" ), // + a valid 4-bytes sequence.
            // Truncated 3-bytes sequences.
            std::make_tuple( "\xE2", L"\uFFFD" ),
            std::make_tuple( "\xE2\xC0", L"\uFFFD\uFFFD" ), // + invalid UTF-8 byte.
            std::make_tuple( "\xE2\xC1", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\xF5", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\xC3", L"\uFFFD\uFFFD" ), // + a truncated 2-bytes sequence.
            std::make_tuple( "\xE2\xC3\xA9", L"\uFFFD\xE9" ), // + a valid 2-bytes sequence.
            std::make_tuple( "\xE2\xE4", L"\uFFFD\uFFFD" ), // + a truncated 3-bytes sequence.
            std::make_tuple( "\xE2\xE4\x82", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\xE4\x82\x96", L"\uFFFD\u4096" ), // + a valid 3-bytes sequence.
            std::make_tuple( "\xE2\xF0", L"\uFFFD\uFFFD" ), // + a truncated 4-bytes sequence.
            std::make_tuple( "\xE2\xF0\x90", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\xF0\x90\x8D", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\xF0\x90\x8D\x88", L"\uFFFD\xD800\xDF48" ), // + a valid 4-bytes sequence (𐍈, U+10348).
            std::make_tuple( "\xE2\x81", L"\uFFFD" ),
            std::make_tuple( "\xE2\x81\xC0", L"\uFFFD\uFFFD" ), // + invalid UTF-8 byte.
            std::make_tuple( "\xE2\x81\xC1", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\x81\xF5", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\x81\xC3", L"\uFFFD\uFFFD" ), // + a truncated 2-bytes sequence.
            std::make_tuple( "\xE2\x81\xC3\xA9", L"\uFFFD\xE9" ), // + a valid 2-bytes sequence.
            std::make_tuple( "\xE2\x81\xE4", L"\uFFFD\uFFFD" ), // + a truncated 3-bytes sequence.
            std::make_tuple( "\xE2\x81\xE4\x82", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\x81\xE4\x82\x96", L"\uFFFD\u4096" ), // + a valid 3-bytes sequence.
            std::make_tuple( "\xE2\x81\xF0", L"\uFFFD\uFFFD" ), // + a truncated 4-bytes sequence.
            std::make_tuple( "\xE2\x81\xF0\x90", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\x81\xF0\x90\x8D", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE2\x81\xF0\x90\x8D\x88", L"\uFFFD\xD800\xDF48" ), // + a valid 4-bytes sequence (𐍈, U+10348).
            // Truncated 4-bytes sequences.
            std::make_tuple( "\xF0", L"\uFFFD" ),
            std::make_tuple( "\xF0\xC0", L"\uFFFD\uFFFD" ), // + invalid UTF-8 byte.
            std::make_tuple( "\xF0\xC1", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\xF5", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\xD0", L"\uFFFD\uFFFD" ), // + a truncated 2-bytes sequence.
            std::make_tuple( "\xF0\xD0\x80", L"\uFFFD\u0400" ), // + a valid 2-bytes sequence (U+0400).
            std::make_tuple( "\xF0\xE0", L"\uFFFD\uFFFD" ), // + a truncated 3-bytes sequence.
            std::make_tuple( "\xF0\xE0\xB8", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\xE0\xB8\xA1", L"\uFFFD\u0E21" ), // + a valid 3-bytes sequence.
            std::make_tuple( "\xF0\xF3", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\xF3\xA0", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\xF3\xA0\x80", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\xF3\xA0\x80\xA2", L"\uFFFD\xDB40\xDC22" ), // + a valid 4-bytes sequence (U+E0022).
            std::make_tuple( "\xF0\x90", L"\uFFFD" ),
            std::make_tuple( "\xF0\x90\xC0", L"\uFFFD\uFFFD" ), // + invalid UTF-8 byte.
            std::make_tuple( "\xF0\x90\xC1", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\xF5", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\xC4", L"\uFFFD\uFFFD" ), // + a truncated 2-bytes sequence.
            std::make_tuple( "\xF0\x90\xC4\xA7", L"\uFFFD\u0127" ), // + a valid 2-bytes sequence (U+0127).
            std::make_tuple( "\xF0\x90\xE1", L"\uFFFD\uFFFD" ), // + a truncated 3-bytes sequence.
            std::make_tuple( "\xF0\x90\xE1\x82", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\xE1\x82\xB4", L"\uFFFD\u10B4" ), // + a valid 3-bytes sequence.
            std::make_tuple( "\xF0\x90\xF0", L"\uFFFD\uFFFD" ), // + a truncated 4-bytes sequence.
            std::make_tuple( "\xF0\x90\xF0\x90", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\xF0\x90\x8D", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\xF0\x90\x8D\x88", L"\uFFFD\xD800\xDF48" ), // + a valid 4-bytes sequence (𐍈, U+10348).
            std::make_tuple( "\xF0\x90\x8D", L"\uFFFD" ),
            std::make_tuple( "\xF0\x90\x8D\xC0", L"\uFFFD\uFFFD" ), // + invalid UTF-8 byte.
            std::make_tuple( "\xF0\x90\x8D\xC1", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\x8D\xF5", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\x8D\xC6", L"\uFFFD\uFFFD" ), // + a truncated 2-bytes sequence.
            std::make_tuple( "\xF0\x90\x8D\xC6\x90", L"\uFFFD\u0190" ), // + a valid 2-bytes sequence (U+0400).
            std::make_tuple( "\xF0\x90\x8D\xEA", L"\uFFFD\uFFFD" ), // + a truncated 3-bytes sequence.
            std::make_tuple( "\xF0\x90\x8D\xEA\xA7", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\x8D\xEA\xA7\x9E", L"\uFFFD\uA9DE" ), // + a valid 3-bytes sequence.
            std::make_tuple( "\xF0\x90\x8D\xF0", L"\uFFFD\uFFFD" ), // + a truncated 4-bytes sequence.
            std::make_tuple( "\xF0\x90\x8D\xF0\x90", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\x8D\xF0\x90\x8D", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x90\x8D\xF0\x90\x8D\x88", L"\uFFFD\xD800\xDF48" ), // + a valid 4-bytes sequence (𐍈, U+10348).
            // Sequence of 10 truncated sequences.
            std::make_tuple( "\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD"
                             "\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD",
                             L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD")
        } ) );

        DYNAMIC_SECTION( "Converting \"" << toHexString( testInput, false ) << "\" to wide string" ) {
            REQUIRE( widen( testInput ) == testOutput );
            REQUIRE( widen( "prefix" + testInput ) == L"prefix" + testOutput );
            REQUIRE( widen( testInput + "suffix" ) == testOutput + L"suffix" );
            REQUIRE( widen( "prefix" + testInput + "suffix" ) == L"prefix" + testOutput + L"suffix" );
        }
    }

    SECTION( "Converting UTF-8 strings containing overlong sequences" ) {
        std::string testInput;
        std::wstring testOutput;

        std::tie( testInput, testOutput ) = GENERATE( table< const char*, const wchar_t* >( {
            // Overlong 2-Byte sequences.
            std::make_tuple( "\xC0\x80", L"\uFFFD" ),
            std::make_tuple( "\xC0\xA0", L"\uFFFD" ),
            std::make_tuple( "\xC0\xAF", L"\uFFFD" ),
            std::make_tuple( "\xC0\xB9", L"\uFFFD" ),
            std::make_tuple( "\xC0\xBF", L"\uFFFD" ),
            std::make_tuple( "\xC1\x81", L"\uFFFD" ),
            std::make_tuple( "\xC1\x9A", L"\uFFFD" ),
            std::make_tuple( "\xC1\xA1", L"\uFFFD" ),
            std::make_tuple( "\xC1\xBE", L"\uFFFD" ),
            std::make_tuple( "\xC1\xBF", L"\uFFFD" ),
            // Overlong 3-Byte sequences.
            std::make_tuple( "\xE0\x80\x80", L"\uFFFD" ),
            std::make_tuple( "\xE0\x80\xA0", L"\uFFFD" ),
            std::make_tuple( "\xE0\x80\xAF", L"\uFFFD" ),
            std::make_tuple( "\xE0\x80\xB9", L"\uFFFD" ),
            std::make_tuple( "\xE0\x80\xBF", L"\uFFFD" ),
            std::make_tuple( "\xE0\x81\x81", L"\uFFFD" ),
            std::make_tuple( "\xE0\x81\x9A", L"\uFFFD" ),
            std::make_tuple( "\xE0\x81\xA1", L"\uFFFD" ),
            std::make_tuple( "\xE0\x81\xBE", L"\uFFFD" ),
            std::make_tuple( "\xE0\x82\xA2", L"\uFFFD" ), // Overlong sequence for non-ASCII character U+00A2.
            std::make_tuple( "\xE0\x9F\xBF", L"\uFFFD" ),
            // Overlong 4-Byte sequences.
            std::make_tuple( "\xF0\x80\x80\x80", L"\uFFFD" ),
            std::make_tuple( "\xF0\x80\x80\xA0", L"\uFFFD" ),
            std::make_tuple( "\xF0\x80\x80\xAF", L"\uFFFD" ),
            std::make_tuple( "\xF0\x80\x80\xB9", L"\uFFFD" ),
            std::make_tuple( "\xF0\x80\x80\xBF", L"\uFFFD" ),
            std::make_tuple( "\xF0\x80\x81\x81", L"\uFFFD" ),
            std::make_tuple( "\xF0\x80\x81\x9A", L"\uFFFD" ),
            std::make_tuple( "\xF0\x80\x81\xA1", L"\uFFFD" ),
            std::make_tuple( "\xF0\x80\x81\xBE", L"\uFFFD" ),
            std::make_tuple( "\xF0\x80\x82\xA2", L"\uFFFD" ), // Overlong sequence for non-ASCII character U+00A2.
            std::make_tuple( "\xF0\x82\x82\xAC", L"\uFFFD" ), // Overlong sequence for non-ASCII character U+20AC (€).
            std::make_tuple( "\xF0\x8F\xBF\xBF", L"\uFFFD" ),
            // Overlong 5-Byte sequences.
            std::make_tuple( "\xF8\x80\x80\x80\x80", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xF8\x80\x80\x80\xA0", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xF8\x80\x80\x80\xAF", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xF8\x87\xBF\xBF\xB9", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xF8\x87\xBF\xBF\xBF", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            // Overlong 6-Byte sequences.
            std::make_tuple( "\xFC\x80\x80\x80\x80\x80", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xFC\x80\x80\x80\x80\xAF", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xFC\x83\xBF\xBF\xBF\xBF", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            // Multiple overlong sequences.
            std::make_tuple( "\xC0\x80\xC0\x80", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xE0\x80\x80\xE0\x80\x80", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x80\x80\x80\xF0\x80\x80\x80", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF8\x80\x80\x80\x80\xF8\x80\x80\x80\x80",
                             L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            std::make_tuple( "\xFC\x80\x80\x80\x80\x80\xFC\x80\x80\x80\x80\x80",
                             L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ),
            // Multiple mixed overlong sequences.
            std::make_tuple( "\xC0\x80\xE0\x80\x80", L"\uFFFD\uFFFD" ),
            std::make_tuple( "\xF0\x80\x80\x80\xF8\x80\x80\x80\x80\xF8\x80\x80\x80\x80",
                             L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" )
        } ) );

        DYNAMIC_SECTION( "Converting \"" << toHexString( testInput, false ) << "\" to wide string" ) {
            REQUIRE( widen( testInput ) == testOutput );
            REQUIRE( widen( "prefix" + testInput ) == L"prefix" + testOutput );
            REQUIRE( widen( testInput + "suffix" ) == testOutput + L"suffix" );
            REQUIRE( widen( "prefix" + testInput + "suffix" ) == L"prefix" + testOutput + L"suffix" );
        }
    }
#endif

    SECTION( "Converting UTF-8 strings contain a single ASCII character" ) {
        for ( int ascii = 0; ascii <= AsciiTrait< char >::kMaxAscii; ++ascii ) {
            const auto character = static_cast< char >( ascii );
            DYNAMIC_SECTION( "Converting \"" << toHexString( character ) << "\" to wide string" ) {
                const std::string input( 1u, character );
                const auto output = widen( input );
                REQUIRE( output.size() == 1 );
                REQUIRE( output.front() == static_cast< wchar_t >( ascii ) );
            }
        }
    }

    SECTION( "Converting UTF-8 strings to wide strings" ) {
        std::string testInput;
        std::wstring testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const char*, const wchar_t* >( {
            WIDENING_TEST_STR( "" ),
            WIDENING_TEST_STR( "Hello, World!" ),
            WIDENING_TEST_STR( "supercalifragilistichespiralidoso" ),
            WIDENING_TEST_STR( "ABC" ),
            WIDENING_TEST_STR( "perché" ),
            WIDENING_TEST_STR( "κόσμε" ),
            WIDENING_TEST_STR( "\u2010" ), // Hyphen ‐
            WIDENING_TEST_STR( "\u4E08" ), // 丈
            WIDENING_TEST_STR( "\u4E16\u754C" ), // 世界
            WIDENING_TEST_STR( "\uE000" ),
            WIDENING_TEST_STR( "\uFFFD" ), // Replacement character U+FFFD
            WIDENING_TEST_STR( "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" ), // メタルガルルモン
            // Long string with mixed characters (no UTF-16 surrogates in output string).
            WIDENING_TEST_STR(
                "English, \u65E5\u672C\u8A9E, \uD55C\uAD6D\uC5B4, \u0627\u0644\u0639\u0631\u0628\u064A\u0629"
            )
        } ) );

        DYNAMIC_SECTION( "Converting \"" << toHexString( testInput ) << "\" to wide string" ) {
            REQUIRE( widen( testInput ) == testOutput );
        }
    }

    SECTION( "Converting UTF-8 strings to wide strings containing UTF-16 surrogates" ) {
        std::string testInput;
        std::wstring testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const char*, const wchar_t* >( {
            // U+10042 = 𐁂
            std::make_tuple( "\xF0\x90\x81\x82", L"\xD800\xDC42" ),
            // U+1F602 = 😂
            std::make_tuple( "\xF0\x9F\x98\x82", L"\xD83D\xDE02" ),
            // U+24B62 = 𤭢
            std::make_tuple( "\xF0\xA4\xAD\xA2", L"\xD852\xDF62" ),
            // Mixed characters: Aé中😂
            std::make_tuple( "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x82", L"A\x00E9\x4E2D\xD83D\xDE02" ),
            // Mixed characters: Hello 世界 😊!
            std::make_tuple( "Hello \xE4\xB8\x96\xE7\x95\x8C \xF0\x9F\x98\x8A!", L"Hello \x4E16\x754C \xD83D\xDE0A!" ),
            // Mixed special characters: ¡§©®™𝄞€£¥»¿
            std::make_tuple( "\xC2\xA1\xC2\xA7\xC2\xA9\xC2\xAE\xE2\x84\xA2\xF0\x9D\x84"
                             "\x9E\xE2\x82\xAC\xC2\xA3\xC2\xA5\xC2\xBB\xC2\xBF",
                             L"\xA1\xA7\xA9\xAE\x2122\xD834\xDD1E\x20AC\xA3\xA5\xBB\xBF" )
        } ) );

        DYNAMIC_SECTION( "Converting L\"" << toHexString( testInput ) << "\" to narrow string" ) {
            REQUIRE( widen( testInput ) == testOutput );
        }
    }

    SECTION( "Boundary value analysis" ) {
        std::string testInput;
        std::wstring testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const char*, const wchar_t* >(
            {
                // ASCII range U+0000 ... U+007F
                WIDENING_TEST_STR( "\u0000" ), // NUL
                WIDENING_TEST_STR( "\u0001" ), // SOH
                WIDENING_TEST_STR( "\u007E" ), // Tilde ~
                WIDENING_TEST_STR( "\u007F" ), // DEL
                // 2-bytes UTF-8 range U+0080 ... U+07FF
                WIDENING_TEST_STR( "\u0080" ), // PAD
                WIDENING_TEST_STR( "\u0081" ), // HOP
                WIDENING_TEST_STR( "\u07FE" ),
                WIDENING_TEST_STR( "\u07FF" ),
                // 3-bytes UTF-8 range U+0800 ... U+FFFF
                WIDENING_TEST_STR( "\u0800" ),
                WIDENING_TEST_STR( "\u0801" ),
                WIDENING_TEST_STR( "\uD7FE" ),
                WIDENING_TEST_STR( "\uD7FF" ),
                // The surrogate range [U+D800, U+DFFF] is tested in a separate section.
                WIDENING_TEST_STR( "\uF000" ),
                WIDENING_TEST_STR( "\uF001" ),
                WIDENING_TEST_STR( "\uFFFE" ),
                WIDENING_TEST_STR( "\uFFFF" ),
                // 4-bytes UTF-8 range U+10000 ... U+10FFFF
                std::make_tuple( "\xF0\x90\x80\x80", L"\xD800\xDC00" ),
                std::make_tuple( "\xF0\x90\x80\x81", L"\xD800\xDC01" ),
                std::make_tuple( "\xF4\x8F\xBF\xBE", L"\xDBFF\xDFFE" ),
                std::make_tuple( "\xF4\x8F\xBF\xBF", L"\xDBFF\xDFFF" )
            }
        ) );

        DYNAMIC_SECTION( "Converting \"" << toHexString( testInput ) << "\" to wide string" ) {
            REQUIRE( widen( testInput ) == testOutput );
        }
    }

#ifndef _WIN32
    SECTION( "Codepoints outside Unicode range specification" ) {
        std::string testInput;
        std::wstring testOutput;
        std::tie( testInput, testOutput ) = GENERATE( table< const char*, const wchar_t* >(
            {
                std::make_tuple( "\xF4\x90\x80\x80", L"\uFFFD" ), // U+110000
                std::make_tuple( "\xF4\x90\x80\x81", L"\uFFFD" ), // U+110001
                std::make_tuple( "\xF7\xBF\xBF\xBF", L"\uFFFD" ), // U+001FFFFF
                std::make_tuple( "\xF8\x88\x80\x80\x80", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ), // U+00200000
                std::make_tuple( "\xFB\xBF\xBF\xBF\xBF", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ), // U+03FFFFFF
                std::make_tuple( "\xFC\x84\x80\x80\x80\x80", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ), // U+04000000
                std::make_tuple( "\xFD\xBF\xBF\xBF\xBF\xBF", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" ), // U+7FFFFFFF
                std::make_tuple( "\xFE\xBF\xBF\xBF\xBF\xBF\xBF", L"\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD\uFFFD" )
            }
        ) );

        DYNAMIC_SECTION( "Converting \"" << toHexString( testInput ) << "\" to wide string" ) {
            REQUIRE( widen( testInput ) == testOutput );
        }
    }
#endif
}

#endif

TEST_CASE( "util: starts_with", "[stringutil][starts_with]" ) {
    using bit7z::starts_with;
    using bit7z::tstring;

    const tstring emptyTestString;
    REQUIRE( starts_with( emptyTestString, BIT7Z_STRING( "" ) ) );
    REQUIRE_FALSE( starts_with( emptyTestString, BIT7Z_STRING( " " ) ) );
    REQUIRE_FALSE( starts_with( emptyTestString, BIT7Z_STRING( "h" ) ) );
    REQUIRE_FALSE( starts_with( emptyTestString, BIT7Z_STRING( "hello world!" ) ) );
    REQUIRE( starts_with( emptyTestString, tstring{ BIT7Z_STRING( "" ) } ) );
    REQUIRE_FALSE( starts_with( emptyTestString, tstring{ BIT7Z_STRING( " " ) } ) );
    REQUIRE_FALSE( starts_with( emptyTestString, tstring{ BIT7Z_STRING( "h" ) } ) );
    REQUIRE_FALSE( starts_with( emptyTestString, tstring{ BIT7Z_STRING( "hello world!" ) } ) );

    const tstring oneCharTestString{ BIT7Z_STRING( "a" ) };
    REQUIRE( starts_with( oneCharTestString, BIT7Z_STRING( "" ) ) );
    REQUIRE( starts_with( oneCharTestString, BIT7Z_STRING( "a" ) ) );
    REQUIRE_FALSE( starts_with( oneCharTestString, BIT7Z_STRING( "h" ) ) );
    REQUIRE_FALSE( starts_with( oneCharTestString, BIT7Z_STRING( "hello world!" ) ) );
    REQUIRE( starts_with( oneCharTestString, tstring{ BIT7Z_STRING( "" ) } ) );
    REQUIRE( starts_with( oneCharTestString, tstring{ BIT7Z_STRING( "a" ) } ) );
    REQUIRE_FALSE( starts_with( oneCharTestString, tstring{ BIT7Z_STRING( "h" ) } ) );
    REQUIRE_FALSE( starts_with( oneCharTestString, tstring{ BIT7Z_STRING( "hello world!" ) } ) );

    const tstring testString{ BIT7Z_STRING( "hello world!" ) };
    REQUIRE_FALSE( starts_with( testString, BIT7Z_STRING( "a" ) ) );
    REQUIRE( starts_with( testString, BIT7Z_STRING( "h" ) ) );
    REQUIRE_FALSE( starts_with( testString, BIT7Z_STRING( "b!" ) ) );
    REQUIRE( starts_with( testString, BIT7Z_STRING( "hello" ) ) );
    REQUIRE( starts_with( testString, BIT7Z_STRING( "hello " ) ) );
    REQUIRE_FALSE( starts_with( testString, BIT7Z_STRING( "oworld!" ) ) );
    REQUIRE( starts_with( testString, BIT7Z_STRING( "hello world" ) ) );
    REQUIRE_FALSE( starts_with( testString, BIT7Z_STRING( "Hello World!" ) ) );
    REQUIRE_FALSE( starts_with( testString, BIT7Z_STRING( "hello world, hello world!" ) ) );
    REQUIRE( starts_with( testString, BIT7Z_STRING( "" ) ) );
    REQUIRE_FALSE( starts_with( testString, BIT7Z_STRING( "!" ) ) );
    REQUIRE_FALSE( starts_with( testString, BIT7Z_STRING( " world!" ) ) );
    REQUIRE( starts_with( testString, BIT7Z_STRING( "hello world!" ) ) );
    REQUIRE_FALSE( starts_with( testString, tstring{ BIT7Z_STRING( "a" ) } ) );
    REQUIRE( starts_with( testString, tstring{ BIT7Z_STRING( "h" ) } ) );
    REQUIRE_FALSE( starts_with( testString, tstring{ BIT7Z_STRING( "b!" ) } ) );
    REQUIRE( starts_with( testString, tstring{ BIT7Z_STRING( "hello" ) } ) );
    REQUIRE( starts_with( testString, tstring{ BIT7Z_STRING( "hello " ) } ) );
    REQUIRE_FALSE( starts_with( testString, tstring{ BIT7Z_STRING( "oworld!" ) } ) );
    REQUIRE( starts_with( testString, tstring{ BIT7Z_STRING( "hello world" ) } ) );
    REQUIRE_FALSE( starts_with( testString, tstring{ BIT7Z_STRING( "Hello World!" ) } ) );
    REQUIRE_FALSE( starts_with( testString, tstring{ BIT7Z_STRING( "hello world, hello world!" ) } ) );
    REQUIRE( starts_with( testString, tstring{ BIT7Z_STRING( "" ) } ) );
    REQUIRE_FALSE( starts_with( testString, tstring{ BIT7Z_STRING( "!" ) } ) );
    REQUIRE_FALSE( starts_with( testString, tstring{ BIT7Z_STRING( " world!" ) } ) );
    REQUIRE( starts_with( testString, tstring{ BIT7Z_STRING( "hello world!" ) } ) );
}

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
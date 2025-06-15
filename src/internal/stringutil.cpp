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

#include "internal/stringutil.hpp"

#ifdef _WIN32
#include <windows.h>
#ifdef BIT7Z_USE_SYSTEM_CODEPAGE
#define CODEPAGE CP_ACP
#define CODEPAGE_WC_FLAGS WC_NO_BEST_FIT_CHARS
#else
#define CODEPAGE CP_UTF8
#define CODEPAGE_WC_FLAGS 0
#endif
#else
#ifndef BIT7Z_USE_STANDARD_FILESYSTEM
// GCC 4.9 doesn't have the <codecvt> header; as a workaround,
// we use GHC filesystem's utility functions for string conversions.
#include "internal/fs.hpp"
#else
// The <codecvt> header has been deprecated in C++17; however, there's no real replacement
// (excluding third-party libraries); hence, for now we just disable the deprecation warnings (only here).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <codecvt>
using convert_type = std::codecvt_utf8< wchar_t >;
#endif
#endif

#include <cstddef>
#include <cstdint>
#include <locale>
#include <string>

namespace bit7z {

#if !defined( _WIN32 )
namespace {
constexpr char32_t kReplacementChar = 0xFFFDu;
constexpr char32_t kHighStart = 0xD800u;
constexpr char32_t kHighEnd = 0xDBFFu;
constexpr char32_t kLowStart  = 0xDC00u;
constexpr char32_t lowEnd = 0xDFFFu;
constexpr char32_t utf16SurrogateShift = 10u;
constexpr char32_t utf16SurrogateOffset = 0x35FDC00u;

BIT7Z_ALWAYS_INLINE
constexpr auto isHighSurrogate( char32_t codepoint ) noexcept -> bool {
    return kHighStart <= codepoint && codepoint <= kHighEnd;
}

BIT7Z_ALWAYS_INLINE
constexpr auto isLowSurrogate( char32_t codepoint ) noexcept -> bool {
    return kLowStart <= codepoint && codepoint <= lowEnd;
}

BIT7Z_ALWAYS_INLINE
constexpr auto isSurrogate( char32_t codepoint ) noexcept -> bool {
    return kHighStart <= codepoint && codepoint <= lowEnd;
}

// NOLINTBEGIN(*-magic-numbers)
BIT7Z_ALWAYS_INLINE
void toUtf8( char32_t codepoint, std::string& result ) {
    // Maximum codepoint expressible in N UTF-8 bytes.
    constexpr char32_t kMaxOneByteUtf8 = 0x007Fu;      // U+0000 ... U+007F
    constexpr char32_t kMaxTwoBytesUtf8 = 0x07FFu;     // U+0080 ... U+07FF
    constexpr char32_t kMaxThreeBytesUtf8 = 0xFFFFu;   // U+0800 ... U+FFFF
    constexpr char32_t kMaxFourBytesUtf8 = 0x10FFFFu;  // Maximum valid Unicode code-point

    if ( codepoint <= kMaxOneByteUtf8 ) {
        // 1-byte UTF-8: [U+0000, U+007F]
        result.push_back( static_cast< char >( codepoint ) ); // 0xxxxxxx
    } else if ( codepoint <= kMaxTwoBytesUtf8 ) {
        // 2-bytes UTF-8: [U+0080, U+07FF]
        result.push_back( static_cast< char >( 0xC0u | ( ( codepoint >> 6 ) & 0x1Fu ) ) );  // 110xxxxx
        result.push_back( static_cast< char >( 0x80u | ( codepoint & 0x3Fu ) ) );           // 10xxxxxx
    } else if ( codepoint <= kMaxThreeBytesUtf8 ) {
        // 3-bytes UTF-8: [U+0800, U+FFFF]
        // Note: UTF-16 surrogates are in this range, but we don't need to check for them
        // as this function gets passed a valid UTF-32 codepoint.
        if ( codepoint == kReplacementChar && ends_with( result, "\xEF\xBF\xBD" ) ) {
            // Avoiding sequences of multiple replacement characters.
            return;
        }
        result.push_back( static_cast< char >( 0xE0u | ( ( codepoint >> 12 ) & 0x0Fu ) ) ); // 1110xxxx
        result.push_back( static_cast< char >( 0x80u | ( ( codepoint >> 6 ) & 0x3Fu ) ) );  // 10xxxxxx
        result.push_back( static_cast< char >( 0x80u | ( codepoint & 0x3Fu ) ) );           // 10xxxxxx
    } else if ( codepoint <= kMaxFourBytesUtf8 ) {
        // 4-bytes UTF-8: [U+10000, U+10FFFF]
        result.push_back( static_cast< char >( 0xF0u | ( ( codepoint >> 18 ) & 0x07u ) ) ); // 11110xxx
        result.push_back( static_cast< char >( 0x80u | ( ( codepoint >> 12 ) & 0x3Fu ) ) ); // 10xxxxxx
        result.push_back( static_cast< char >( 0x80u | ( ( codepoint >> 6 ) & 0x3Fu ) ) );  // 10xxxxxx
        result.push_back( static_cast< char >( 0x80u | ( codepoint & 0x3Fu ) ) );           // 10xxxxxx
    } else if ( !ends_with( result, "\xEF\xBF\xBD" ) ) {
        // TODO: Add option to throw an exception on invalid UTF sequences
        // Invalid code point, use replacement character U+FFFD
        result += "\xEF\xBF\xBD";
    }
}
// NOLINTEND(*-magic-numbers)

BIT7Z_ALWAYS_INLINE
auto decodeCodepoint( const wchar_t* wideString, std::size_t size, std::size_t& index ) -> char32_t {
    // NOLINTNEXTLINE(*-pro-bounds-pointer-arithmetic)
    const auto currentChar = static_cast< char32_t >( wideString[ index ] );
    if ( !isSurrogate( currentChar ) ) {
        return currentChar;
    }

    if ( isHighSurrogate( currentChar ) && index + 1 < size ) {
        // High surrogate, must be followed by a low surrogate
        // NOLINTNEXTLINE(*-pro-bounds-pointer-arithmetic)
        const auto nextChar = static_cast< char32_t >( wideString[ index + 1 ] );
        if ( isLowSurrogate( nextChar ) ) {
            ++index;
            // The RFC 2781 standard formula for calculating the codepoint is the following:
            // codepoint = ((high - 0xD800) << 10u) + (low - 0xDC00) + 0x10000;
            // which can be simplified as follows:
            // codepoint = (high << 10u) - (0xD800 << 10u) + (low - 0xDC00) + 0x10000;
            // codepoint = (high << 10u) + low + (-(0xD800 << 10u) - 0xDC00 + 0x10000);
            // codepoint = (high << 10u) + low - (0x3600000 + 0xDC00 - 0x10000);
            return ( currentChar << utf16SurrogateShift ) + nextChar - utf16SurrogateOffset;
        }
    }

    // TODO: Add option to throw an exception on invalid UTF sequences.
    // Invalid code point, use replacement character U+FFFD.
    return kReplacementChar;
}
} // namespace
#endif

#if !defined( _WIN32 ) || !defined( BIT7Z_USE_NATIVE_STRING )
auto narrow( const wchar_t* wideString, std::size_t size ) -> std::string {
    if ( wideString == nullptr || size == 0 ) {
        return {};
    }
#ifdef _WIN32
    const int narrowStringSize = WideCharToMultiByte( CODEPAGE,
                                                      CODEPAGE_WC_FLAGS,
                                                      wideString,
                                                      static_cast< int >( size ),
                                                      nullptr,
                                                      0,
                                                      nullptr,
                                                      nullptr );
    if ( narrowStringSize == 0 ) {
        return "";
    }

    std::string result( static_cast< std::string::size_type >( narrowStringSize ), 0 );
    WideCharToMultiByte( CODEPAGE,
                         CODEPAGE_WC_FLAGS,
                         wideString,
                         -1,
                         &result[ 0 ],  // NOLINT(readability-container-data-pointer)
                         static_cast< int >( narrowStringSize ),
                         nullptr,
                         nullptr );
    return result;
#else
    // Note: this function supports wide strings containing a mix of UTF-16 and UTF-32 code units.
    std::string result;
    result.reserve( size * 3 );
    for ( std::size_t index = 0; index < size; ++index ) {
        const char32_t utf32char = decodeCodepoint( wideString, size, index );
        toUtf8( utf32char, result );
    }
    return result;
#endif
}

auto widen( const std::string& narrowString ) -> std::wstring {
#ifdef _WIN32
    const int narrowStringSize = static_cast< int >( narrowString.size() );
    const int wideStringSize = MultiByteToWideChar( CODEPAGE,
                                                    0,
                                                    narrowString.c_str(),
                                                    narrowStringSize,
                                                    nullptr,
                                                    0 );
    if ( wideStringSize == 0 ) {
        return {}; // Note: using L"" breaks release builds with MinGW when precompiled headers are used.
    }

    std::wstring result( static_cast< std::wstring::size_type >( wideStringSize ), 0 );
    MultiByteToWideChar( CODEPAGE,
                         0,
                         narrowString.c_str(),
                         narrowStringSize,
                         &result[ 0 ], // NOLINT(readability-container-data-pointer)
                         wideStringSize );
    return result;
#elif !defined( BIT7Z_USE_STANDARD_FILESYSTEM )
    return fs::detail::fromUtf8< std::wstring >( narrowString );
#else
    std::wstring_convert< convert_type, wchar_t > converter;
    return converter.from_bytes( narrowString );
#endif
}
#endif

#if !defined( _WIN32 ) && defined( BIT7Z_USE_STANDARD_FILESYSTEM )
#pragma GCC diagnostic pop
#endif

} // namespace bit7z
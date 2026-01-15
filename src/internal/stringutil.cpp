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

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace bit7z {

#if !defined( _WIN32 )
namespace {

// UTF-16 Constants
constexpr char32_t kHighStart = 0xD800u;
constexpr char32_t kHighEnd = 0xDBFFu;
constexpr char32_t kLowStart  = 0xDC00u;
constexpr char32_t kLowEnd = 0xDFFFu;
constexpr char32_t kSurrogateShift = 10u;
constexpr char32_t kSurrogateOffset = 0x35FDC00u;

// Unicode constants
constexpr char32_t kMaxUnicodeCodepoint = 0x10FFFFu;  // Maximum valid Unicode code-point
constexpr char32_t kReplacementChar = 0xFFFDu;

BIT7Z_ALWAYS_INLINE
constexpr auto isHighSurrogate( char32_t codepoint ) noexcept -> bool {
    return kHighStart <= codepoint && codepoint <= kHighEnd;
}

BIT7Z_ALWAYS_INLINE
constexpr auto isLowSurrogate( char32_t codepoint ) noexcept -> bool {
    return kLowStart <= codepoint && codepoint <= kLowEnd;
}

BIT7Z_ALWAYS_INLINE
constexpr auto isSurrogate( char32_t codepoint ) noexcept -> bool {
    return kHighStart <= codepoint && codepoint <= kLowEnd;
}

// NOLINTBEGIN(*-magic-numbers)
BIT7Z_ALWAYS_INLINE
void toUtf8( char32_t codepoint, std::string& result ) {
    // Maximum codepoint expressible in N UTF-8 bytes.
    constexpr char32_t kMaxOneByteUtf8 = 0x007Fu;      // U+0000 ... U+007F
    constexpr char32_t kMaxTwoBytesUtf8 = 0x07FFu;     // U+0080 ... U+07FF
    constexpr char32_t kMaxThreeBytesUtf8 = 0xFFFFu;   // U+0800 ... U+FFFF

    if ( codepoint <= kMaxOneByteUtf8 ) {
        // 1-byte UTF-8: [U+0000, U+007F]
        result.push_back( static_cast< char >( codepoint ) ); // 0xxxxxxx
    } else if ( codepoint <= kMaxTwoBytesUtf8 ) {
        // 2-bytes UTF-8: [U+0080, U+07FF]
        result.push_back( static_cast< char >( 0xC0u | ( ( codepoint >> 6u ) & 0x1Fu ) ) );  // 110xxxxx
        result.push_back( static_cast< char >( 0x80u | ( codepoint & 0x3Fu ) ) );           // 10xxxxxx
    } else if ( codepoint <= kMaxThreeBytesUtf8 ) {
        // 3-bytes UTF-8: [U+0800, U+FFFF]
        // Note: UTF-16 surrogates are in this range, but we don't need to check for them
        // as this function gets passed a valid UTF-32 codepoint.
        if ( codepoint == kReplacementChar && ends_with( result, "\xEF\xBF\xBD" ) ) {
            // Avoiding sequences of multiple replacement characters.
            return;
        }
        result.push_back( static_cast< char >( 0xE0u | ( ( codepoint >> 12u ) & 0x0Fu ) ) ); // 1110xxxx
        result.push_back( static_cast< char >( 0x80u | ( ( codepoint >> 6u ) & 0x3Fu ) ) );  // 10xxxxxx
        result.push_back( static_cast< char >( 0x80u | ( codepoint & 0x3Fu ) ) );           // 10xxxxxx
    } else if ( codepoint <= kMaxUnicodeCodepoint ) {
        // 4-bytes UTF-8: [U+10000, U+10FFFF]
        result.push_back( static_cast< char >( 0xF0u | ( ( codepoint >> 18u ) & 0x07u ) ) ); // 11110xxx
        result.push_back( static_cast< char >( 0x80u | ( ( codepoint >> 12u ) & 0x3Fu ) ) ); // 10xxxxxx
        result.push_back( static_cast< char >( 0x80u | ( ( codepoint >> 6u ) & 0x3Fu ) ) );  // 10xxxxxx
        result.push_back( static_cast< char >( 0x80u | ( codepoint & 0x3Fu ) ) );           // 10xxxxxx
    } else if ( !ends_with( result, "\xEF\xBF\xBD" ) ) {
        // TODO: Add option to throw an exception on invalid UTF sequences
        // Invalid code point, use replacement character U+FFFD
        result += "\xEF\xBF\xBD";
    }
}

BIT7Z_ALWAYS_INLINE
auto decodeCodepoint( const wchar_t* wideString, std::size_t size, std::size_t& index ) noexcept -> char32_t {
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
            return ( currentChar << kSurrogateShift ) + nextChar - kSurrogateOffset;
        }
    }

    // TODO: Add option to throw an exception on invalid UTF sequences.
    // Invalid code point, use replacement character U+FFFD.
    return kReplacementChar;
}

BIT7Z_ALWAYS_INLINE
void toUtf16( char32_t codepoint, std::wstring& result ) {
    if ( codepoint <= 0xFFFFu ) {
        result.push_back( static_cast< wchar_t >( codepoint ) );
    } else {
        codepoint -= 0x10000u;
        result.push_back( static_cast< wchar_t >( ( codepoint >> kSurrogateShift ) + kHighStart ) );
        result.push_back( static_cast< wchar_t >( ( codepoint & 0x3FFu ) + kLowStart ) );
    }
}

BIT7Z_ALWAYS_INLINE
constexpr auto isInvalidContinuationByte( std::uint8_t byte ) noexcept -> bool {
    return ( byte & 0xC0 ) != 0x80;
}

constexpr std::array<std::uint8_t, 32> utfSizes = {
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 2u, 2u, 2u, 2u, 3u, 3u, 4u, 0u
};

BIT7Z_ALWAYS_INLINE
constexpr auto byteSequenceSize( std::uint8_t leadingByte ) noexcept -> std::uint8_t {
    return utfSizes[ static_cast< std::uint8_t >( leadingByte >> 3u ) ]; // NOLINT(*-pro-bounds-constant-array-index)
}

constexpr std::array<uint32_t, 5> utfMinValues = {
    0, 0, 128, 2048, 65536
};

BIT7Z_ALWAYS_INLINE
constexpr auto isOutOfRangeCodepoint( char32_t codepoint, std::uint8_t sequenceSize ) noexcept -> bool {
    // Checking if the codepoint represents a non-canonical encoding
    // (i.e., overlong encoding), or it is outside of Unicode specification.
    // NOLINTNEXTLINE(*-pro-bounds-constant-array-index)
    return codepoint < utfMinValues[ sequenceSize ] || codepoint > kMaxUnicodeCodepoint;
}

constexpr std::array<std::uint8_t, 5> utfLeadMasks = {
    0, 0, 0x1Fu, 0x0Fu, 0x07u
};

BIT7Z_ALWAYS_INLINE
auto decodeCodepoint( std::string::const_iterator& it, const std::string::const_iterator& end ) noexcept -> char32_t {
    const auto leadingByte = static_cast< std::uint8_t >( *it++ );

    if ( leadingByte <= 0x7Fu ) { // ASCII codepoint.
        return static_cast< char32_t >( leadingByte );
    }

    // Here the UTF-8 byte sequence should have at least two bytes.

    if ( it == end ) { // Truncated UTF-8 sequence after the leading byte.
        return kReplacementChar;
    }

    const auto sequenceSize = byteSequenceSize( leadingByte );
    const auto leadMask = utfLeadMasks[ sequenceSize ]; // NOLINT(*-pro-bounds-constant-array-index)

    // Recreating the codepoint from the UTF-8 byte sequence.
    std::uint32_t codepoint = ( leadingByte & leadMask );
    std::uint8_t index = 1;
    for (; it != end && index < sequenceSize; ++index ) {
        const auto continuationByte = static_cast< std::uint8_t >( *it++ );
        if ( isInvalidContinuationByte( continuationByte ) ) {
            --it;
            return kReplacementChar;
        }
        codepoint = ( codepoint << 6u ) + ( continuationByte & 0x3Fu );
    }

    if ( index != sequenceSize ) { // Truncated sequence.
        return kReplacementChar;
    }
    if ( isOutOfRangeCodepoint( codepoint, sequenceSize ) ) {
        return kReplacementChar;
    }
    if ( isSurrogate( codepoint ) ) { // Lone surrogate.
        return kReplacementChar;
    }
    return static_cast< char32_t >( codepoint );
}
// NOLINTEND(*-magic-numbers)

} // namespace
#endif

#ifdef _WIN32
#ifdef BIT7Z_USE_SYSTEM_CODEPAGE
constexpr auto kCodePageWcFlags = WC_NO_BEST_FIT_CHARS;
#else
constexpr auto kCodePageWcFlags = 0;
#endif

auto narrow( const wchar_t* wideString, std::size_t size, unsigned codePage ) -> std::string {
#else
auto narrow( const wchar_t* wideString, std::size_t size ) -> std::string {
#endif
    if ( wideString == nullptr || size == 0 ) {
        return {};
    }
#ifdef _WIN32
    const int narrowStringSize = WideCharToMultiByte( codePage,
                                                      kCodePageWcFlags,
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
    WideCharToMultiByte( codePage,
                         kCodePageWcFlags,
                         wideString,
                         -1,
                         &result[ 0 ], // NOLINT(readability-container-data-pointer, *-avoid-unchecked-container-access)
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

#if !defined( _WIN32 ) || !defined( BIT7Z_USE_NATIVE_STRING )
auto widen( const std::string& narrowString ) -> std::wstring {
#ifdef _WIN32
    const int narrowStringSize = static_cast< int >( narrowString.size() );
    const int wideStringSize = MultiByteToWideChar( kDefaultCodePage,
                                                    0,
                                                    narrowString.c_str(),
                                                    narrowStringSize,
                                                    nullptr,
                                                    0 );
    if ( wideStringSize == 0 ) {
        return {}; // Note: using L"" breaks release builds with MinGW when precompiled headers are used.
    }

    std::wstring result( static_cast< std::wstring::size_type >( wideStringSize ), 0 );
    MultiByteToWideChar( kDefaultCodePage,
                         0,
                         narrowString.c_str(),
                         narrowStringSize,
                         &result[ 0 ], // NOLINT(readability-container-data-pointer, *-avoid-unchecked-container-access)
                         wideStringSize );
    return result;
#else
    std::wstring result;
    result.reserve( narrowString.size() );
    for ( auto it = narrowString.cbegin(); it != narrowString.cend(); ) {
        const char32_t utf32char = decodeCodepoint( it, narrowString.cend() );
        toUtf16( utf32char, result );
    }
    return result;
#endif
}
#endif

} // namespace bit7z
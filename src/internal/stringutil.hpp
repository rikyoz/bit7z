/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef STRINGUTIL_HPP
#define STRINGUTIL_HPP

#include "bittypes.hpp"

#include "bitwindows.hpp"

#include <algorithm>
#include <cctype> // For std::toupper
#include <cstddef>
#include <string>

#ifdef _WIN32
#include <cwctype> // For std::towupper
#endif

namespace bit7z {

#ifdef BIT7Z_USE_SYSTEM_CODEPAGE
constexpr auto kDefaultCodePage = CP_ACP;
#else
constexpr auto kDefaultCodePage = CP_UTF8;
#endif

#ifdef _WIN32
auto narrow( const wchar_t* wideString, std::size_t size, unsigned codePage = kDefaultCodePage ) -> std::string;
#else
auto narrow( const wchar_t* wideString, std::size_t size ) -> std::string;
#endif

#if defined( BIT7Z_USE_NATIVE_STRING ) && defined( _WIN32 )
// On Windows, with native strings enabled, strings are already wide!
#   define WIDEN( tstr ) tstr
#   define NATIVE( tstr ) tstr
#else
#   define WIDEN( tstr ) bit7z::widen(tstr)
#   ifdef _WIN32
#       define NATIVE( tstr ) bit7z::widen(tstr)
#   else
#       define NATIVE( tstr ) tstr
#   endif
auto widen( const std::string& narrowString ) -> std::wstring;
#endif

template< typename CharT >
auto starts_with( const std::basic_string< CharT >& str, const std::basic_string< CharT >& prefix ) noexcept -> bool {
    return str.rfind( prefix, 0 ) == 0;
}

template< typename CharT >
auto starts_with( const std::basic_string< CharT >& str, const CharT* prefix ) noexcept -> bool {
    return str.rfind( prefix, 0 ) == 0;
}

// Note: the implementation using std::equal seems to be faster than the alternatives in most cases;
// see https://quick-bench.com/q/G9D6M1h11PrwwmqcS7taJoAIdZU for a comparison.
template< typename CharT >
auto ends_with( const std::basic_string< CharT >& str, const std::basic_string< CharT >& suffix ) noexcept -> bool {
    return str.size() >= suffix.size() &&
           std::equal( suffix.crbegin(), suffix.crend(), str.crbegin(), str.crbegin() + suffix.size() );
}

template< typename CharT, std::size_t N >
// NOLINTNEXTLINE(*-avoid-c-arrays)
auto ends_with( const std::basic_string< CharT >& str, const CharT (&suffix)[N] ) noexcept -> bool {
    // Note: the suffix C array has a null termination character.
    constexpr auto suffixSize = N - 1;
    return str.size() >= suffixSize &&
           std::equal( std::crbegin( suffix ) + 1, std::crend( suffix ), str.crbegin(), str.crbegin() + suffixSize );
}

template< typename CharT >
struct to_upper;

template<>
struct to_upper< char > {
    auto operator()( const unsigned char input ) const noexcept -> char {
        return static_cast< char >( std::toupper( input ) );
    }
};

#ifdef _WIN32
template<>
struct to_upper< wchar_t > {
    auto operator()( const wchar_t input ) const noexcept -> wchar_t {
        return std::towupper( input );
    }
};
#endif

// https://quick-bench.com/q/fJw2JI2ft7uWKvJ15xmsWIH13Zg
template< typename CharT >
auto to_uppercase( const std::basic_string< CharT >& str, std::size_t offset = 0 ) -> std::basic_string< CharT > {
    std::basic_string< CharT > result{ str, offset };
    std::transform( result.begin(), result.end(), result.begin(), to_upper< CharT >() );
    return result;
}

} // namespace bit7z

#endif //STRINGUTIL_HPP

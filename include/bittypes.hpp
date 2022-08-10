/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITTYPES_HPP
#define BITTYPES_HPP

#include <string>
#include <vector>

#ifdef BIT7Z_REGEX_MATCHING
#include <regex>
#endif

namespace bit7z {
/**
 * @brief A type representing a byte (equivalent to an unsigned char).
 */
#ifdef BIT7Z_USE_STD_BYTE
#if __cpp_lib_byte
using byte_t = std::byte;
#else
enum class byte_t : unsigned char {}; //same as std::byte_t
#endif
#else
using byte_t = unsigned char;
#endif

using buffer_t = std::vector< byte_t >;
using index_t = std::ptrdiff_t; //like gsl::index (https://github.com/microsoft/GSL)

namespace { //private
template< class Char >
struct string_traits;

template<>
struct string_traits< char > {
    template< class T >
    static inline std::string convert_to_string( T&& t ) {
        return std::to_string( std::forward< T >( t ) );
    }
};

template<>
struct string_traits< wchar_t > {
    template< class T >
    static inline std::wstring convert_to_string( T&& t ) {
        return std::to_wstring( std::forward< T >( t ) );
    }
};
}  // namespace

#if defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32) // Windows
using tchar = wchar_t;
#define BIT7Z_STRING( str ) L##str
#else // Unix
using tchar = char;
#define BIT7Z_STRING( str ) str
#endif

using tstring = std::basic_string< tchar >;
#ifdef BIT7Z_REGEX_MATCHING
using tregex = std::basic_regex< tchar >;
#endif

template< typename T >
inline std::basic_string< tchar > to_tstring( T&& Arg ) {
    return string_traits< tchar >::convert_to_string( std::forward< T >( Arg ) );
}
}  // namespace bit7z
#endif // BITTYPES_HPP

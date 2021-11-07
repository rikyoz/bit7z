/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
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
                return std::to_wstring( std::forward< T >( t )) ;
            }
        };
    }

#ifdef _WIN32 // Windows
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
}
#endif // BITTYPES_HPP

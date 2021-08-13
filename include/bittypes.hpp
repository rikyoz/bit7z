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

#if defined( __cpp_lib_filesystem )
#   define USE_STANDARD_FILESYSTEM
#elif defined( __cplusplus ) && __cplusplus >= 201703L && defined( __has_include )
#   if __has_include( <filesystem> )
#       define USE_STANDARD_FILESYSTEM
#   endif
#endif

#if defined( __has_cpp_attribute )
#   if __has_cpp_attribute(nodiscard)
#       define BIT7Z_NODISCARD [[nodiscard]]
#   endif
#   if __has_cpp_attribute(maybe_unused)
#       define BIT7Z_MAYBE_UNUSED [[maybe_unused]]
#   endif
#elif defined( __cplusplus ) && __cplusplus >= 201703L
#   define BIT7Z_NODISCARD [[nodiscard]]
#   define BIT7Z_MAYBE_UNUSED [[maybe_unused]]
#endif

#ifndef BIT7Z_NODISCARD
#define BIT7Z_NODISCARD
#endif
#ifndef BIT7Z_MAYBE_UNUSED
#define BIT7Z_MAYBE_UNUSED
#endif

namespace bit7z {
    /**
     * @brief A type representing a byte (equivalent to an unsigned char).
     */
#if __cpp_lib_byte
    using byte_t = std::byte;
#else
    enum class byte_t : unsigned char {}; //same as std::byte_t
#endif
    using buffer_t = std::vector< byte_t >;
    using index_t = std::ptrdiff_t; //like gsl::index (https://github.com/microsoft/GSL)

#ifdef _WIN32 // Windows
    using tchar = wchar_t;
    using tstring = std::wstring;
#ifdef BIT7Z_REGEX_MATCHING
    using tregex = std::wregex;
#endif
#define TSTRING( str ) L##str
#define to_tstring std::to_wstring
#else // Unix
    using tchar = char;
    using tstring = std::string;
#ifdef BIT7Z_REGEX_MATCHING
    using tregex = std::regex;
#endif
#define TSTRING( str ) str
#define to_tstring std::to_string

    constexpr auto ERROR_OPEN_FAILED = EIO;
    constexpr auto ERROR_SEEK = EIO;
    constexpr auto ERROR_READ_FAULT = EIO;
    constexpr auto ERROR_WRITE_FAULT = EIO;

//Macros not defined by p7zip
#define HRESULT_FACILITY(hr)  (((hr) >> 16) & 0x1fff)
#define HRESULT_CODE(hr)    ((hr) & 0xFFFF)
#define COM_DECLSPEC_NOTHROW
#endif
}
#endif // BITTYPES_HPP

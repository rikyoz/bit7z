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

#ifndef BITDEFINES_HPP
#define BITDEFINES_HPP

#if defined( __cplusplus ) && __cplusplus >= 201703
#define BIT7Z_CPP17
#endif

#if defined( __cpp_lib_filesystem )
#   define BIT7Z_USE_STANDARD_FILESYSTEM
#elif defined( BIT7Z_CPP17 ) && defined( __has_include )
#   if __has_include( <filesystem> )
#       define BIT7Z_USE_STANDARD_FILESYSTEM
#   endif
#endif

/* Macro defines for [[nodiscard]] and [[maybe_unused]] attributes. */
#if defined( __has_cpp_attribute )
#   if __has_cpp_attribute( nodiscard )
#       define BIT7Z_NODISCARD [[nodiscard]]
#   endif
#   if __has_cpp_attribute( maybe_unused )
#       define BIT7Z_MAYBE_UNUSED [[maybe_unused]]
#   endif
#endif

/* The compiler doesn't support __has_cpp_attribute, but it's using C++17 standard. */
#if !defined( BIT7Z_NODISCARD ) && defined( BIT7Z_CPP17 )
#   define BIT7Z_NODISCARD [[nodiscard]]
#endif

#if !defined( BIT7Z_MAYBE_UNUSED ) && defined( BIT7Z_CPP17 )
#   define BIT7Z_MAYBE_UNUSED [[maybe_unused]]
#endif

/* Compiler is using C++14 standard, define empty macros. */
#ifndef BIT7Z_NODISCARD
#define BIT7Z_NODISCARD
#endif
#ifndef BIT7Z_MAYBE_UNUSED
#define BIT7Z_MAYBE_UNUSED
#endif

#endif //BITDEFINES_HPP

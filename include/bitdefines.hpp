/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITDEFINES_HPP
#define BITDEFINES_HPP

/* Uncomment the following macros if you don't want to define them yourself in your project files */
//#define BIT7Z_AUTO_FORMAT
//#define BIT7Z_REGEX_MATCHING

#if ( defined( _MSVC_LANG ) && _MSVC_LANG >= 201703L ) || ( defined( __cplusplus ) && __cplusplus >= 201703 )
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

/* The compiler doesn't support __has_cpp_attribute, but it is using C++17 standard. */
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

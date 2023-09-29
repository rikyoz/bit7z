/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMPILER_HPP
#define COMPILER_HPP

#if defined( _LIBCPP_VERSION ) || defined( _MSC_VER )
#include <ostream>
#endif

#define STRING2( x ) #x
#define STRING( x ) STRING2(x)
#define VER_STRING( major, minor, patch ) STRING(major) "." STRING(minor) "." STRING(patch)

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {
namespace compiler {

// Compiler name and version
#if defined( __clang__ )
constexpr auto name = "clang++";
constexpr auto version = VER_STRING(__clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined( __GNUC__ )
#   if !( defined( __MINGW32__ ) || defined( __MINGW64__ ) )
constexpr auto name = "g++";
#   elif defined( __MINGW64__ )
constexpr auto name = "MinGW-w64";
#   else
constexpr auto name = "MinGW";
#   endif
constexpr auto version = VER_STRING( __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__ );
#elif defined( _MSC_VER )
constexpr auto name = "MSVC";
constexpr auto version = STRING( _MSC_VER );
#else
constexpr auto name = "Unknown";
constexpr auto version = "N/A";
#endif

#ifdef _LIBCPP_VERSION
constexpr auto standard_library = "libc++";
const auto standard_library_version = [](std::ostream& out) -> std::ostream& {
    constexpr auto libcpp_major = _LIBCPP_VERSION / 1000;
    constexpr auto libcpp_minor = ( _LIBCPP_VERSION % 1000 ) / 100;
    return out << libcpp_major << "." << libcpp_minor;
};
#elif defined( _GLIBCXX_RELEASE ) || defined( __GLIBCXX__ ) || defined( __GLIBCPP__ )
constexpr auto standard_library = "libstdc++";
#   if defined( _GLIBCXX_RELEASE )
constexpr auto standard_library_version = _GLIBCXX_RELEASE;
#   elif defined( __GLIBCXX__ )
constexpr auto standard_library_version = __GLIBCXX__;
#   elif defined( __GLIBCPP__ )
constexpr auto standard_library_version = __GLIBCPP__;
#   else
constexpr auto standard_library_version = __GNUC__;
#   endif
#elif defined( _MSC_VER )
constexpr auto standard_library = "Microsoft STL";
static const auto standard_library_version = [](std::ostream& out) -> std::ostream& {
#   if _MSC_VER >= 1900
    constexpr auto msvc_major = ( _MSC_VER / 100 ) - 5;
#   else
    constexpr auto msvc_major = ( _MSC_VER / 100 ) - 6;
#   endif
    constexpr auto msvc_minor = _MSC_VER % 100;
    return out << msvc_major << "." << msvc_minor;
};
#else
constexpr auto standard_library = "Unknown";
constexpr auto standard_library_version = "N/A";
#endif

#ifdef __GLIBC__
constexpr auto c_runtime = "glibc";
#elif defined( _UCRT )
constexpr auto c_runtime = "ucrt + vcruntime";
#elif defined( __MSVCRT_VERSION__ )
constexpr auto c_runtime = "msvcrt";
#else
constexpr auto c_runtime = "Unknown";
#endif

// Compiler target architecture
#if defined( _WIN64 ) || defined( __x86_64__ )
constexpr auto target_arch = "x64";
#elif defined( _WIN32 ) || defined( __i386__ )
constexpr auto target_arch = "x86";
#else
constexpr auto target_arch = "Unsupported Architecture";
#endif

} // namespace compiler
} // namespace test
} // namespace bit7z

#endif //COMPILER_HPP

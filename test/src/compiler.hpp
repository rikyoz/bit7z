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


#define STRING2( x ) #x
#define STRING( x ) STRING2(x)
#define VER_STRING( major, minor, patch ) STRING(major) "." STRING(minor) "." STRING(patch)

namespace bit7z {
namespace test {
namespace compiler {

// Compiler name and version
#if defined(__clang__)
constexpr auto name = "clang++";
constexpr auto version = VER_STRING(__clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
#   if !(defined(__MINGW32__) || defined(__MINGW64__))
constexpr auto name = "g++";
#   elif defined(__MINGW64__)
constexpr auto name = "MinGW-w64";
#   else
constexpr auto name = "MinGW";
#   endif
constexpr auto version = VER_STRING(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
constexpr auto name = "MSVC";
constexpr auto version = STRING( _MSC_VER );
#else
constexpr auto name = "Unknown";
constexpr auto name = "N/A";
#endif

// Compiler target architecture
#if defined(_WIN64) || defined(__x86_64__)
constexpr auto target_arch = "x64";
#elif defined(_WIN32) || defined(__i386__)
constexpr auto target_arch = "x86";
#else
constexpr auto target_arch = "Unsupported Architecture";
#endif

}
}
}

#endif //COMPILER_HPP

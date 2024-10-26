/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FLAGS_HPP
#define FLAGS_HPP

namespace bit7z {
namespace test {
namespace flags {

#ifdef NDEBUG
constexpr auto debug_or_release = "RELEASE BUILD";
#else
constexpr auto debug_or_release = "DEBUG BUILD";
#endif

#ifdef BIT7Z_AUTO_FORMAT
constexpr auto auto_format = "ON";
#else
constexpr auto auto_format = "OFF";
#endif

#ifdef BIT7Z_REGEX_MATCHING
constexpr auto regex_matching = "ON";
#else
constexpr auto regex_matching = "OFF";
#endif

#ifdef BIT7Z_USE_NATIVE_STRING
constexpr auto native_string = "ON";
#else
constexpr auto native_string = "OFF";
#endif

#ifdef BIT7Z_USE_STANDARD_FILESYSTEM
constexpr auto standard_filesystem = "ON";
#else
constexpr auto standard_filesystem = "OFF";
#endif

#ifdef BIT7Z_USE_SYSTEM_CODEPAGE
constexpr auto use_system_codepage = "ON";
#else
constexpr auto use_system_codepage = "OFF";
#endif

#ifdef BIT7Z_AUTO_PREFIX_LONG_PATHS
constexpr auto auto_prefix_long_paths = "ON";
#else
constexpr auto auto_prefix_long_paths = "OFF";
#endif

#ifdef BIT7Z_PATH_SANITIZATION
constexpr auto path_sanitization = "ON";
#else
constexpr auto path_sanitization = "OFF";
#endif

#ifndef _WIN32
#ifdef BIT7Z_USE_LEGACY_IUNKNOWN
constexpr auto use_legacy_iunknown = "ON";
#else
constexpr auto use_legacy_iunknown = "OFF";
#endif
#endif

} // namespace flags
} // namespace test
} // namespace bit7z

#endif //FLAGS_HPP

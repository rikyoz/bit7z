/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SHARED_LIB_HPP
#define SHARED_LIB_HPP

#include <bit7z/bittypes.hpp>

#ifndef BIT7Z_USE_SYSTEM_7ZIP
#include "filesystem.hpp"
#endif

namespace bit7z {
namespace test {

inline auto sevenzip_lib_path() -> tstring {
#ifdef BIT7Z_TESTS_USE_SYSTEM_7ZIP
#ifdef _WIN64
    static const tstring lib_path = BIT7Z_STRING( "C:\\Program Files\\7-Zip\\7z.dll" );
#elif defined( _WIN32 )
    static const tstring lib_path = BIT7Z_STRING( "C:\\Program Files (x86)\\7-Zip\\7z.dll" );
#elif defined( __linux__ )
    static const tstring lib_path = "/usr/lib/p7zip/7z.so"; //default installation path of p7zip's shared library
#else
    static const tstring lib_path = "./7z.so";
#endif
#elif defined(_WIN32)
    static const auto lib_path = ( filesystem::exe_path().parent_path() / "7z.dll" ).string< tchar >();
#else
    static const auto lib_path = ( filesystem::exe_path().parent_path() / "7z.so" ).string();
#endif
    return lib_path;
}

} // namespace test
} // namespace bit7z

#endif //SHARED_LIB_HPP

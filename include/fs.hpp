/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2020  Riccardo Ostani - All Rights Reserved.
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

#ifndef FS_HPP
#define FS_HPP

#if defined(__cpp_lib_filesystem) || \
    ( defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>) )

#include <fstream>
#include <filesystem>

namespace fs {
    using namespace std::filesystem;
    using ifstream = std::ifstream;
    using ofstream = std::ofstream;
    using fstream = std::fstream;
}
#else
#define GHC_WIN_WSTRING_STRING_TYPE
#include "ghc/filesystem.hpp"

namespace fs {
    using namespace ghc::filesystem;
    using ifstream = ghc::filesystem::ifstream;
    using ofstream = ghc::filesystem::ofstream;
    using fstream = ghc::filesystem::fstream;
}
#endif

#endif // FS_HPP

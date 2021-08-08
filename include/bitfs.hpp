// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#ifndef BITFS_HPP
#define BITFS_HPP

/* Header for forward declaring fs namespace */

#include "bittypes.hpp" /* it defines USE_STANDARD_FILESYSTEM */

#ifdef USE_STANDARD_FILESYSTEM
#include <filesystem>
#else
/* Notes: we use this forward declaration to avoid including private headers (e.g. fs.hpp).
 *        Since some public API headers include bitgenericitem.hpp (e.g. "bitoutputarchive.hpp"),
 *        including private headers here would result in the "leaking" out of these latter in the public API.*/
namespace ghc {
    namespace filesystem {
        class path;
    }
}
#endif

namespace fs {
#ifdef USE_STANDARD_FILESYSTEM
    using namespace std::filesystem;
#else
    using namespace ghc::filesystem;
#endif
}

#endif //BITFS_HPP

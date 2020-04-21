/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#ifndef FSUTIL_HPP
#define FSUTIL_HPP

#include <string>

#include "../include/bittypes.hpp"

#include <windows.h>

#include "fs.hpp"

namespace bit7z {
    namespace filesystem {
        namespace fsutil {
            tstring filename( const tstring& path, bool ext = false );

            tstring extension( const tstring& path );

            bool setFileModifiedTime( const fs::path& filePath, const FILETIME& ft_modified );

            bool wildcardMatch( const tstring& pattern, const tstring& str );

            uint32_t getFileAttributes( const fs::path& filePath );

            bool setFileAttributes( const fs::path& filePath, uint32_t attributes );

            bool getFileTimes( const fs::path& filePath,
                               FILETIME& creationTime,
                               FILETIME& accessTime,
                               FILETIME& writeTime );
        }
    }
}
#endif // FSUTIL_HPP

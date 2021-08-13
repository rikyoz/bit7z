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

#ifndef FSUTIL_HPP
#define FSUTIL_HPP

#include <string>

#include <windows.h>

#include "bittypes.hpp"
#include "internal/fs.hpp"

#ifndef _WIN32
typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    // not needed fields
    // DWORD    nFileSizeHigh;
    // DWORD    nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA;
#endif

namespace bit7z {
    namespace filesystem {
        namespace fsutil {
            BIT7Z_NODISCARD tstring filename( const tstring& path, bool ext = false );

            BIT7Z_NODISCARD tstring extension( const tstring& path );

            BIT7Z_NODISCARD bool wildcardMatch( const tstring& pattern, const tstring& str );

            BIT7Z_NODISCARD bool getFileAttributesEx( const fs::path& filePath, WIN32_FILE_ATTRIBUTE_DATA& fileInfo ) noexcept;

            bool setFileModifiedTime( const fs::path& filePath, const FILETIME& ftModified ) noexcept;

            bool setFileAttributes( const fs::path& filePath, DWORD attributes ) noexcept;

            BIT7Z_NODISCARD fs::path inArchivePath( const fs::path& file_path, const fs::path& search_path = fs::path() );
        }
    }
}
#endif // FSUTIL_HPP

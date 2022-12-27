/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FSUTIL_HPP
#define FSUTIL_HPP

#include <string>

#include "bitdefines.hpp"
#include "bittypes.hpp"
#include "internal/fs.hpp"
#include "internal/windows.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {
namespace fsutil {

BIT7Z_NODISCARD tstring basename( const tstring& path );

BIT7Z_NODISCARD tstring extension( const fs::path& path );

BIT7Z_NODISCARD bool wildcardMatch( const tstring& pattern, const tstring& str );

BIT7Z_NODISCARD bool getFileAttributesEx( const fs::path& filePath,
                                          WIN32_FILE_ATTRIBUTE_DATA& fileMetadata ) noexcept;

bool setFileModifiedTime( const fs::path& filePath, const FILETIME& ftModified ) noexcept;

bool setFileAttributes( const fs::path& filePath, DWORD attributes ) noexcept;

BIT7Z_NODISCARD fs::path inArchivePath( const fs::path& file_path,
                                        const fs::path& search_path = fs::path() );

}  // namespace fsutil
}  // namespace filesystem
}  // namespace bit7z

#endif // FSUTIL_HPP

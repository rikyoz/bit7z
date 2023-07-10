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

BIT7Z_NODISCARD auto basename( const tstring& path ) -> tstring;

BIT7Z_NODISCARD auto extension( const fs::path& path ) -> tstring;

BIT7Z_NODISCARD auto wildcardMatch( const tstring& pattern, const tstring& str ) -> bool;

BIT7Z_NODISCARD auto getFileAttributesEx( const fs::path& filePath,
                                          WIN32_FILE_ATTRIBUTE_DATA& fileMetadata ) noexcept -> bool;

auto setFileModifiedTime( const fs::path& filePath, const FILETIME& ftModified ) noexcept -> bool;

auto setFileAttributes( const fs::path& filePath, DWORD attributes ) noexcept -> bool;

BIT7Z_NODISCARD auto inArchivePath( const fs::path& file_path,
                                    const fs::path& search_path = fs::path() ) -> fs::path;

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )

BIT7Z_NODISCARD auto should_format_long_path( const fs::path& path ) -> bool;

BIT7Z_NODISCARD auto format_long_path( const fs::path& path ) -> fs::path;

#   define FORMAT_LONG_PATH( path ) \
        filesystem::fsutil::should_format_long_path( path ) ? filesystem::fsutil::format_long_path( path ) : path

#else
#   define FORMAT_LONG_PATH( path ) path
#endif

/**
 * @brief When writing multi-volume archives, we keep all the volume streams open until we finished.
 * This is less than ideal, and there's a limit in the number of open file descriptors/handles.
 * This function is a temporary workaround, where we increase such a limit to the maximum value allowed by the OS.
 */
void increase_opened_files_limit();

}  // namespace fsutil
}  // namespace filesystem
}  // namespace bit7z

#endif // FSUTIL_HPP

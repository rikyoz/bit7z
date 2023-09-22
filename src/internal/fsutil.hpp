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

#if defined( _WIN32 ) && defined( __GLIBCXX__ ) && defined( _WIO_DEFINED )
#include <ext/stdio_filebuf.h>
#include <fcntl.h>
#include <share.h>
#endif

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

enum struct SymlinkPolicy {
    Follow,
    DoNotFollow
};

namespace fsutil {

BIT7Z_NODISCARD auto basename( const tstring& path ) -> tstring;

BIT7Z_NODISCARD auto extension( const fs::path& path ) -> tstring;

BIT7Z_NODISCARD auto wildcard_match( const tstring& pattern, const tstring& str ) -> bool;

BIT7Z_NODISCARD auto get_file_attributes_ex( const fs::path& filePath,
                                             SymlinkPolicy symlinkPolicy,
                                             WIN32_FILE_ATTRIBUTE_DATA& fileMetadata ) noexcept -> bool;

auto set_file_modified_time( const fs::path& filePath, FILETIME ftModified ) noexcept -> bool;

auto set_file_attributes( const fs::path& filePath, DWORD attributes ) noexcept -> bool;

BIT7Z_NODISCARD auto in_archive_path( const fs::path& file_path,
                                      const fs::path& search_path = fs::path{} ) -> fs::path;

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

#if defined( _WIN32 ) && defined( __GLIBCXX__ ) && defined( _WIO_DEFINED )
inline auto open_flags(std::ios::openmode mode) -> int {
    const bool out = (mode & std::ios::out) != 0;
    const bool in = (mode & std::ios::in) != 0;
    int flags = 0;
    if (in && out) {
        flags |= _O_RDWR | _O_CREAT;
    } else if (out) {
        flags |= _O_WRONLY | _O_CREAT;
    } else {
        flags |= _O_RDONLY;
    }
    if ((mode & std::ios::app) != 0) {
        flags |= _O_APPEND;
    }
    if ((mode & std::ios::trunc) != 0) {
        flags |= _O_TRUNC;
    }
    flags |= (mode & std::ios::binary) != 0 ? _O_BINARY : _O_TEXT;
    return flags;
}

inline auto permission_flags(std::ios::openmode mode) -> int {
    int flags = 0;
    if ((mode & std::ios::in) != 0) {
        flags |= _S_IREAD;
    }
    if ((mode & std::ios::out) != 0) {
        flags |= _S_IWRITE;
    }
    return flags;
}

/**
 * On old MinGW versions, ghc::filesystem's streams cannot open files with Unicode characters in the path.
 * This is just a workaround for opening a file using the native UTF-16 path string.
 *
 * @tparam charT    The character type used by the filebuf.
 * @tparam traits   The traits of the character type used by the filebuf.
 *
 * @param path  The path of the file to be opened.
 * @param mode  The mode to use when opening the file.
 *
 * @return a basic_filebuf objected created by opening the file at the given path.
 */
template< class charT, class traits = std::char_traits< charT>>
auto open_filebuf(const fs::path& path, std::ios_base::openmode mode) -> std::basic_filebuf< charT, traits > {
    int fd = 0;
    _wsopen_s(&fd, GHC_NATIVEWP( path ), open_flags( mode ), _SH_DENYNO, permission_flags( mode ));
    return __gnu_cxx::stdio_filebuf< charT, traits >( fd, mode );
}
#endif

#if defined( _WIN32 ) && defined( BIT7Z_PATH_SANITIZATION )
/**
 * Sanitizes the given file path, removing any eventual Windows illegal character
 * (https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file)
 *
 * @param path The path to be sanitized.
 *
 * @return the sanitized path, where illegal characters are replaced with the '_' character.
 */
auto sanitize_path( const fs::path& path ) -> fs::path;
#endif

}  // namespace fsutil
}  // namespace filesystem
}  // namespace bit7z

#endif // FSUTIL_HPP

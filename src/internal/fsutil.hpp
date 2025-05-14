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

#include "bitdefines.hpp"
#include "bititemsvector.hpp" // For SymlinkPolicy
#include "bittypes.hpp"
#include "internal/fs.hpp"
#include "internal/stringutil.hpp"
#include "internal/windows.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)

/**
 * Checks if the given character is a valid path separator on the target platform.
 *
 * @note On Windows, both / and \ are considered as path separators.
 *       On other platforms, only / is considered a path separator.
 *
 * @param character the character value to be checked.
 *
 * @return true if the character is a valid path separator on the target platform, false otherwise.
 */
constexpr auto isPathSeparator( char character ) -> bool {
#ifdef _WIN32
    return character == '/' || character == '\\';
#else
    return character == '/';
#endif
}

#ifdef _WIN32
constexpr auto isPathSeparator( wchar_t character ) -> bool {
    return character == L'/' || character == L'\\';
}
#endif

namespace filesystem {

namespace fsutil {

BIT7Z_NODISCARD auto stem( const tstring& path ) -> tstring;

BIT7Z_NODISCARD auto extension( const fs::path& path ) -> tstring;

// Note: wildcard_match is "semi-public", so we cannot pass the path as fs::path.
BIT7Z_NODISCARD auto wildcard_match( const tstring& pattern, const tstring& path ) -> bool;

BIT7Z_NODISCARD auto get_file_attributes_ex( const fs::path& filePath,
                                             SymlinkPolicy symlinkPolicy,
                                             WIN32_FILE_ATTRIBUTE_DATA& fileMetadata ) noexcept -> bool;

#ifdef _WIN32
// TODO: In future, use std::optional instead of empty FILETIME objects.
auto set_file_time( const fs::path& filePath, FILETIME creation, FILETIME access, FILETIME modified ) noexcept -> bool;
#else
auto set_file_modified_time( const fs::path& filePath, FILETIME ftModified ) noexcept -> bool;
#endif

auto set_file_attributes( const fs::path& filePath, DWORD attributes ) noexcept -> bool;

BIT7Z_NODISCARD auto in_archive_path( const fs::path& filePath,
                                      const fs::path& searchPath = fs::path{} ) -> fs::path;

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

auto sanitized_extraction_path( const fs::path& outDir, const fs::path& itemPath ) -> fs::path;
#endif

}  // namespace fsutil
}  // namespace filesystem

inline auto path_to_tstring( const fs::path& path ) -> tstring {
    /* In an ideal world, we should only use fs::path's string< tchar >() function for converting a path to a tstring.
     * However, MSVC converts paths to std::string using the system codepage instead of UTF-8,
     * which is the default encoding of bit7z. */
#if !defined( _WIN32 ) || defined( BIT7Z_USE_NATIVE_STRING )
    return path.native();
#elif !defined( BIT7Z_USE_SYSTEM_CODEPAGE ) && !defined( BIT7Z_CPP20_U8STRING )
    return path.u8string();
#else // On Windows, if we are using either the system codepage or building bit7z using the C++20 standard.
    /* If we encounter a path with Unicode characters, MSVC will throw an exception
     * while converting from a fs::path to std::string if any character is invalid in the system codepage.
     * Hence, here we use bit7z's own string conversion function, which substitutes invalid Unicode characters
     * with '?' characters. */
    const auto& native_path = path.native();
    return narrow( native_path.c_str(), native_path.size() );
#endif
}

#if defined( _MSC_VER ) && !defined( BIT7Z_USE_NATIVE_STRING ) && !defined( BIT7Z_USE_SYSTEM_CODEPAGE )
#define PATH_FROM_TSTRING( str ) fs::u8path( str )
#else
#define PATH_FROM_TSTRING( str ) fs::path{ str }
#endif

inline auto tstring_to_path( const tstring& str ) -> fs::path {
#ifdef _MSC_VER
    // fs::u8path() has been deprecated in C++20, so we disable the warning to keep using it.
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
    auto result = PATH_FROM_TSTRING( str );
    if ( filesystem::fsutil::should_format_long_path( result ) ) {
        result = filesystem::fsutil::format_long_path( result );
    }
    return result;
#else
    // By default, MSVC treats strings as encoded using the system codepage, but bit7z uses UTF-8.
    return PATH_FROM_TSTRING( str );
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

inline auto path_to_wide_string( const fs::path& path ) -> std::wstring {
#if defined( _MSC_VER ) || !defined( BIT7Z_USE_STANDARD_FILESYSTEM )
    return path.wstring();
#else
    /* On some compilers and platforms (e.g., GCC before v12.3),
     * the direct conversion of the fs::path to wstring might throw an exception due to unicode characters.
     * So we simply convert to tstring, and then widen it if necessary. */
    return WIDEN( path.string< tchar >() );
#endif
}

}  // namespace bit7z

#endif // FSUTIL_HPP

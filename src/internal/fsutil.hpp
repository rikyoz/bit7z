/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FSUTIL_HPP
#define FSUTIL_HPP

#include "bitdefines.hpp"
#include "bitexception.hpp"
#include "bititemsvector.hpp" // For SymlinkPolicy
#include "bittypes.hpp"
#include "internal/fs.hpp"
#include "internal/stringutil.hpp"
#include "internal/windows.hpp"

#include <type_traits>

#ifndef _WIN32
#include <sys/stat.h>

#   if defined( __APPLE__ ) || defined( BSD ) || \
        defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __DragonFly__ )
using FileMetadata = struct stat;
const auto os_lstat = &lstat;
const auto os_stat = &stat;
#   else
using FileMetadata = struct stat64;
const auto os_lstat = &lstat64;
const auto os_stat = &stat64;
#   endif
#else
using FileMetadata = WIN32_FILE_ATTRIBUTE_DATA;
#endif

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
constexpr auto isPathSeparator( const char character ) noexcept -> bool {
#ifdef _WIN32
    return character == '/' || character == '\\';
#else
    return character == '/';
#endif
}

#ifdef _WIN32
constexpr auto isPathSeparator( const wchar_t character ) noexcept -> bool {
    return character == L'/' || character == L'\\';
}
#endif

inline auto pathToTstring( const fs::path& path ) -> tstring {
    /* In an ideal world, we should only use fs::path's string< tchar >() function for converting a path to a tstring.
     * However, MSVC converts paths to std::string using the system codepage instead of UTF-8,
     * which is the default encoding of bit7z. */
#if !defined( _WIN32 ) || defined( BIT7Z_USE_NATIVE_STRING )
    return path.native();
#else // Windows, without BIT7Z_USE_NATIVE_STRING.
    /* If we encounter a path with Unicode characters, MSVC will throw an exception
     * while converting from a fs::path to std::string if any character is invalid in the system codepage.
     * Hence, here we use bit7z's own string conversion function, which substitutes invalid Unicode characters
     * with '?' characters. */
    const auto& native_path = path.native();
    return narrow( native_path.c_str(), native_path.size() );
#endif
}

inline auto pathToSevenzipString( const fs::path& path ) -> std::wstring {
#ifdef _WIN32
    return path.native();
#else
    /* On some compilers and platforms (e.g., GCC before v12.3),
     * the direct conversion of the fs::path to wstring might throw an exception due to Unicode characters.
     * So we simply convert to string, and then widen it if necessary. */
    return widen( path.native() );
#endif
}

class SafeOutPathBuilder final {
    fs::path mBasePath;

    public:
        explicit SafeOutPathBuilder( const tstring& basePath );

        BIT7Z_NODISCARD
        auto buildPath( const fs::path& path ) const -> fs::path;

#ifndef _WIN32
        BIT7Z_NODISCARD
        auto restoreSymlink( const fs::path& symlinkFilePath ) const -> bool;
#endif

        BIT7Z_NODISCARD
        auto basePath() const -> const fs::path&;
};

namespace filesystem {
namespace fsutil {

BIT7Z_NODISCARD auto stem( const tstring& path ) -> tstring;

BIT7Z_NODISCARD auto extension( const fs::path& path ) -> tstring;

template< typename T, typename Enable = void >
struct dot_constant;

template< typename CharT >
struct dot_constant< CharT, typename std::enable_if< std::is_same< CharT, char >::value >::type > {
    static constexpr char value = '.';
};

template< typename CharT >
struct dot_constant< CharT, typename std::enable_if< std::is_same< CharT, wchar_t >::value >::type > {
    static constexpr wchar_t value = L'.';
};

template< typename CharT >
auto containsDotReferences( const std::basic_string< CharT >& path ) -> bool {
    const std::size_t length = path.length();
    std::size_t pos = 0;

    // Search for the first occurrence of '.'
    do {
        pos = path.find( dot_constant< CharT >::value, pos );
        // Exit the loop if no more dots are found.
        if ( pos == std::basic_string< CharT >::npos ) {
            return false;
        }

        // Check if we found a single "." or double dots ".." by looking at surrounding characters.

        // Case 1: Single dot "."
        if ( ( pos == 0 || isPathSeparator( path[ pos - 1 ] ) ) && // Start of string, or preceding char is a separator.
             ( pos + 1 == length || isPathSeparator( path[ pos + 1 ] ) ) ) {
            // End of string, or following char is a separator.
            return true;
        }

        // Case 2: Double dots ".."
        if ( ( pos + 1 < length && path[ pos + 1 ] == dot_constant< CharT >::value && // Two consecutive dots.
               ( pos == 0 || isPathSeparator( path[ pos - 1 ] ) ) ) &&
             // Start of string, or preceding char is a separator.
             ( pos + 2 == length || isPathSeparator( path[ pos + 2 ] ) ) ) {
            // End of string, or following char is a separator.
            return true;
        }

        ++pos;
    } while ( pos != std::basic_string< CharT >::npos );

    return false;
}

// Note: wildcard_match is "semi-public", so we cannot pass the path as fs::path.
BIT7Z_NODISCARD auto wildcardMatch( const tstring& pattern, const tstring& path ) -> bool;

#ifndef _WIN32
auto setFileModifiedTime( const fs::path& filePath, FILETIME ftModified ) noexcept -> bool;
#endif

BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto getFileMetadata( const fs::path& filePath, SymlinkPolicy policy ) -> FileMetadata {
    FileMetadata fileMetadata;
#ifdef _WIN32
    ( void )policy;
    const auto result = ::GetFileAttributesExW( filePath.c_str(), GetFileExInfoStandard, &fileMetadata );
    if ( result == FALSE ) {
#else
    const auto statRes = policy == SymlinkPolicy::Follow
                             ? os_stat( filePath.c_str(), &fileMetadata )
                             : os_lstat( filePath.c_str(), &fileMetadata );
    if ( statRes != 0 ) {
#endif
        const auto error = lastErrorCode();
        throw BitException( "Could not read filesystem item properties", error, pathToTstring( filePath ) );
    }
    return fileMetadata;
}

auto setFileAttributes(
    const SafeOutPathBuilder& pathBuilder,
    const fs::path& filePath,
    DWORD attributes
) noexcept -> bool;

BIT7Z_NODISCARD auto inArchivePath(
    const fs::path& filePath,
    const fs::path& searchPath = fs::path{}
) -> fs::path;

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )

BIT7Z_NODISCARD auto shouldFormatLongPath( const fs::path& path ) -> bool;

BIT7Z_NODISCARD auto formatLongPath( const fs::path& path ) -> fs::path;

#   define FORMAT_LONG_PATH( path ) \
        filesystem::fsutil::shouldFormatLongPath( path ) ? filesystem::fsutil::formatLongPath( path ) : path

#else
#   define FORMAT_LONG_PATH( path ) path
#endif

#ifdef BIT7Z_PATH_SANITIZATION
/**
 * Sanitizes the given file path, removing any eventual illegal path character.
 * For Windows illegal path characters: https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file.
 * For Unix illegal path characters: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_170
 *
 * @note On Unix, we only sanitize null byte characters \0, as path separators are automatically handled by fs::path.
 *
 * @param path The path to be sanitized.
 *
 * @return the sanitized path, where illegal characters are replaced with the '_' character.
 */
BIT7Z_NODISCARD
auto sanitizePath( const fs::path& path ) -> fs::path;
#endif

} // namespace fsutil
} // namespace filesystem

#if defined( _MSC_VER ) && !defined( BIT7Z_USE_NATIVE_STRING ) && !defined( BIT7Z_USE_SYSTEM_CODEPAGE )
#define PATH_FROM_TSTRING( str ) fs::u8path( str )
#else
#define PATH_FROM_TSTRING( str ) fs::path{ str }
#endif

inline auto tstringToPath( const tstring& str ) -> fs::path {
#ifdef _MSC_VER
    // fs::u8path() has been deprecated in C++20, so we disable the warning to keep using it.
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
    auto result = PATH_FROM_TSTRING( str );
    if ( filesystem::fsutil::shouldFormatLongPath( result ) ) {
        result = filesystem::fsutil::formatLongPath( result );
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

inline auto sevenzipStringToPath( const sevenzip_string& str ) -> fs::path {
#if BIT7Z_AUTO_PREFIX_LONG_PATHS
    fs::path result{ str };
    if ( filesystem::fsutil::shouldFormatLongPath( result ) ) {
        result = filesystem::fsutil::formatLongPath( result );
    }
    return result;
#elif defined( _WIN32 )
    return str;
#else
    return narrow( str.data(), str.size() );
#endif
}

} // namespace bit7z

#endif // FSUTIL_HPP

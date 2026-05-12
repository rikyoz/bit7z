// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <algorithm> //for std::adjacent_find

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bittypes.hpp"
#include "bitwindows.hpp"

#ifndef _WIN32
#include <sys/resource.h> // for rlimit, getrlimit, and setrlimit
#include <sys/stat.h>
#include <unistd.h>

#include "internal/dateutil.hpp"

// For some reason, GCC on macOS requires including <climits> for defining OPEN_MAX.
#if defined( __APPLE__ ) && !defined( __clang__ )
#include <climits>
#endif
#elif defined( BIT7Z_PATH_SANITIZATION )
#include <cwctype> // for iswdigit
#endif

#include "internal/fsutil.hpp"
#include "internal/stringutil.hpp"

using namespace std;

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

auto fsutil::stem( const tstring& path ) -> tstring {
    return path_to_tstring( tstring_to_path( path ).stem() );
}

auto fsutil::extension( const fs::path& path ) -> tstring {
    const fs::path ext = path.extension();
    if ( ext.empty() ) {
        return {};
    }

    // We don't want the leading dot of the extension!
    const tstring result = path_to_tstring( ext );
    return result.substr( 1 );
}

inline auto contains_dot_references( const fs::path& path ) -> bool {
    return std::find_if( path.begin(), path.end(), [] ( const fs::path& component ) -> bool {
        const auto& nativeComponent = component.native();
        return nativeComponent == BIT7Z_NATIVE_STRING( "." ) || nativeComponent == BIT7Z_NATIVE_STRING( ".." );
    }) != path.end();
}

auto fsutil::in_archive_path( const fs::path& filePath, const fs::path& searchPath ) -> fs::path {
    /* Note: the following algorithm tries to emulate the behavior of 7-zip when dealing with
             paths of items in archives. */

    const auto& normalPath = filePath.lexically_normal();

    auto filename = normalPath.filename();
    if ( filename == BIT7Z_NATIVE_STRING( "." ) || filename == BIT7Z_NATIVE_STRING( ".." ) ) {
        return {};
    }
    if ( filename.empty() ) {
        filename = normalPath.parent_path().filename();
    }

    if ( filePath.is_absolute() || contains_dot_references( filePath ) ) {
        // Note: in this case, if the file was found while indexing a directory passed by the user, we need to retain
        // the internal structure of that folder (mSearchPath), otherwise we use only the file name.
        if ( searchPath.empty() ) {
            return filename;
        }
        return searchPath / filename;
    }

    // Here, the path is relative and without ./ or ../ => e.g. foo/bar/test.txt

    if ( !searchPath.empty() ) {
        // The item was found while indexing a directory
        return searchPath / filename;
    }
    return filePath;
}

// A modified version of the code found here: https://stackoverflow.com/a/3300547
auto w_match( tstring::const_iterator patternIt, // NOLINT(misc-no-recursion)
              const tstring::const_iterator& patternEnd,
              tstring::const_iterator strIt,
              const tstring::const_iterator& strEnd ) -> bool {
    for ( ; patternIt != patternEnd; ++patternIt ) {
        switch ( *patternIt ) {
            case BIT7Z_STRING( '?' ):
                if ( strIt == strEnd ) {
                    return false;
                }
                ++strIt;
                break;
            case BIT7Z_STRING( '*' ): {
                while ( patternIt + 1 != patternEnd && *( patternIt + 1 ) == '*' ) {
                    ++patternIt;
                }
                if ( patternIt + 1 == patternEnd ) {
                    return true;
                }
                for ( auto i = strIt; i != strEnd; ++i ) {
                    if ( w_match( patternIt + 1, patternEnd, i, strEnd ) ) {
                        return true;
                    }
                }
                return false;
            }
            default:
                if ( strIt == strEnd || *strIt != *patternIt ) {
                    return false;
                }
                ++strIt;
        }
    }
    return strIt == strEnd;
}

auto fsutil::wildcard_match( const tstring& pattern, const tstring& str ) -> bool { // NOLINT(misc-no-recursion)
    if ( pattern.empty() ) {
        return wildcard_match( BIT7Z_STRING( "*" ), str );
    }
    return w_match( pattern.cbegin(), pattern.cend(), str.begin(), str.end() );
}

#ifndef _WIN32
static const mode_t global_umask = []() noexcept -> mode_t {
    // Getting and setting the current umask.
    // Note: flawfinder warns about umask with the mask set to 0;
    // however, we use it only to read the current umask,
    // then we restore the old value, hence we can ignore the warning.
    const mode_t currentUmask{ umask( 0 ) }; // flawfinder: ignore

    // Restoring the umask.
    umask( currentUmask ); // flawfinder: ignore

    return static_cast< mode_t >( static_cast< int >( fs::perms::all ) & ( ~currentUmask ) );
}();

#endif

#ifndef _WIN32
#if defined( __APPLE__ ) || defined( BSD ) || \
    defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __DragonFly__ )
using stat_t = struct stat;
const auto os_lstat = &lstat;
const auto os_stat = &stat;
#else
using stat_t = struct stat64;
const auto os_lstat = &lstat64;
const auto os_stat = &stat64;
#endif
#endif

auto fsutil::set_file_attributes(
    const SafeOutPathBuilder& pathBuilder,
    const fs::path& filePath,
    DWORD attributes
) noexcept -> bool {
    if ( filePath.empty() ) {
        return false;
    }

#ifdef _WIN32
    (void)pathBuilder; // Unused on Windows.
    if ( ( attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) == FILE_ATTRIBUTE_UNIX_EXTENSION ) {
        constexpr auto kUnixWritePermissionsMask = 0222u;
        // Most likely, this is a Tar archive, which doesn't store Windows attributes, but only Unix permissions.
        // 7-Zip returns them as (unixPermissions << 16) | FILE_ATTRIBUTE_UNIX_EXTENSION
        const auto unixPermissions = static_cast< std::uint32_t >( attributes >> 16u );
        if ( ( unixPermissions & kUnixWritePermissionsMask ) == 0u ) { // No write permissions
            attributes = FILE_ATTRIBUTE_READONLY;
        }
    }
    return ::SetFileAttributesW( filePath.c_str(), attributes ) != FALSE;
#else
    stat_t fileStat{};
    if ( os_lstat( filePath.c_str(), &fileStat ) != 0 ) {
        return false;
    }

    if ( ( attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) != 0 ) {
        fileStat.st_mode = static_cast< mode_t >( attributes >> 16U );
        if ( S_ISLNK( fileStat.st_mode ) ) {
            return pathBuilder.restoreSymlink( filePath );
        }

        if ( S_ISDIR( fileStat.st_mode ) ) {
            fileStat.st_mode |= ( S_IRUSR | S_IWUSR | S_IXUSR );
        } else if ( !S_ISREG( fileStat.st_mode ) ) {
            return true;
        }
    } else if ( S_ISLNK( fileStat.st_mode ) ) {
        return true;
    } else if ( !S_ISDIR( fileStat.st_mode ) && ( attributes & FILE_ATTRIBUTE_READONLY ) != 0 ) {
        fileStat.st_mode &= static_cast< mode_t >( ~( S_IWUSR | S_IWGRP | S_IWOTH ) );
    }

    const fs::perms filePermissions = static_cast< fs::perms >( fileStat.st_mode & global_umask ) & fs::perms::mask;
    std::error_code error;
    fs::permissions( filePath, filePermissions, error );
    return !error;
#endif
}

#ifdef _WIN32
auto fsutil::set_file_time( const fs::path& filePath,
                            FILETIME creation,
                            FILETIME access,
                            FILETIME modified ) noexcept -> bool {
    if ( filePath.empty() ) {
        return false;
    }

    bool res = false;
    HANDLE hFile = ::CreateFileW( filePath.c_str(),
                                  GENERIC_READ | FILE_WRITE_ATTRIBUTES,
                                  FILE_SHARE_READ,
                                  nullptr,
                                  OPEN_EXISTING,
                                  0,
                                  nullptr );
    if ( hFile != INVALID_HANDLE_VALUE ) { // NOLINT(cppcoreguidelines-pro-type-cstyle-cast,performance-no-int-to-ptr)
        res = ::SetFileTime( hFile, &creation, &access, &modified ) != FALSE;
        CloseHandle( hFile );
    }
    return res;
}
#else
auto fsutil::set_file_modified_time( const fs::path& filePath, FILETIME ftModified ) noexcept -> bool {
    if ( filePath.empty() ) {
        return false;
    }

    std::error_code error;
    auto fileTime = FILETIME_to_file_time_type( ftModified );
    fs::last_write_time( filePath, fileTime, error );
    return !error;
}
#endif

auto fsutil::get_file_attributes_ex( const fs::path& filePath,
                                     SymlinkPolicy symlinkPolicy,
                                     WIN32_FILE_ATTRIBUTE_DATA& fileMetadata ) noexcept -> bool {
    if ( filePath.empty() ) {
        return false;
    }

#ifdef _WIN32
    (void)symlinkPolicy;
    return ::GetFileAttributesExW( filePath.c_str(), GetFileExInfoStandard, &fileMetadata ) != FALSE;
#else
    stat_t statInfo{};
    const auto statRes = symlinkPolicy == SymlinkPolicy::Follow ?
                         os_stat( filePath.c_str(), &statInfo ) :
                         os_lstat( filePath.c_str(), &statInfo );
    if ( statRes != 0 ) {
        return false;
    }

    // File attributes
    fileMetadata.dwFileAttributes = S_ISDIR( statInfo.st_mode ) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_ARCHIVE;
    if ( ( statInfo.st_mode & S_IWUSR ) == 0 ) {
        fileMetadata.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    }
    constexpr auto kMask = 0xFFFFu;
    std::uint32_t unixAttributes = ( ( statInfo.st_mode & kMask ) << 16u );
    fileMetadata.dwFileAttributes |= FILE_ATTRIBUTE_UNIX_EXTENSION + unixAttributes;

    // File times
    fileMetadata.ftCreationTime = time_to_FILETIME( statInfo.st_ctime );
    fileMetadata.ftLastAccessTime = time_to_FILETIME( statInfo.st_atime );
    fileMetadata.ftLastWriteTime = time_to_FILETIME( statInfo.st_mtime );
    return true;
#endif
}

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )

namespace {
constexpr auto kLongPathPrefix = BIT7Z_NATIVE_STRING( R"(\\?\)" );
} // namespace

auto fsutil::should_format_long_path( const fs::path& path ) -> bool {
    constexpr auto kMaxDosFilenameSize = 12;

    if ( !path.is_absolute() ) {
        return false;
    }
    const auto& pathStr = path.native();
    if ( pathStr.size() < static_cast<std::size_t>( MAX_PATH - kMaxDosFilenameSize ) ) {
        return false;
    }
    return !starts_with( pathStr, kLongPathPrefix );
}

auto fsutil::format_long_path( const fs::path& path ) -> fs::path {
    fs::path longPath = kLongPathPrefix;
    // Note: we call this function after checking if we should format the given path as a long path.
    // This means that if the path starts with the \\ prefix,
    // it is a UNC path (e.g., \\server\share) and not a long path prefixed with \\?\.
    if ( starts_with( path.native(), BIT7Z_NATIVE_STRING( R"(\\)" ) ) ) {
        longPath += L"UNC";
        longPath /= &path.native()[2]; // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
    } else {
        longPath += path;
    }
    return longPath;
}

#endif

void fsutil::increase_opened_files_limit() {
#if defined( _MSC_VER )
    // http://msdn.microsoft.com/en-us/library/6e3b887c.aspx
    _setmaxstdio( 8192 );
#elif defined( __MINGW32__ )
    // MinGW uses an older max value for this function
    _setmaxstdio( 2048 );
#else
    rlimit limits{ 0, 0 };
    if ( getrlimit( RLIMIT_NOFILE, &limits ) == 0 ) {
#ifdef __APPLE__
        limits.rlim_cur = std::min( static_cast< rlim_t >( OPEN_MAX ), limits.rlim_max );
#else
        limits.rlim_cur = limits.rlim_max;
#endif
        setrlimit( RLIMIT_NOFILE, &limits );
    }
#endif
}

#if defined( _WIN32 ) && defined( BIT7Z_PATH_SANITIZATION )
namespace {
BIT7Z_NODISCARD
auto is_windows_reserved_name( const std::wstring& component, std::size_t offset ) -> bool {
    const auto upperComponent = to_uppercase( component, offset );

    // Reserved file names that can't be used on Windows: CON, PRN, AUX, NUL, CONIN$, CONOUT$.
    if ( upperComponent == L"CON" || upperComponent == L"PRN" || upperComponent == L"AUX" || upperComponent == L"NUL" ||
         upperComponent == L"CONIN$" || upperComponent == L"CONOUT$" ) {
        return true;
    }
    // Reserved file names that can't be used on Windows:
    // COM0, COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9,
    // LPT0, LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, and LPT9.
    constexpr auto reserved_component_size = 4;
    return upperComponent.size() == reserved_component_size &&
           ( starts_with( upperComponent, L"COM" ) || starts_with( upperComponent, L"LPT" ) ) &&
           std::iswdigit( upperComponent.back() ) != 0;
}

BIT7Z_NODISCARD
auto sanitize_path_component( std::wstring component ) -> std::wstring {
    if ( component.empty() ) {
        return {}; // Note: using L"" breaks release builds with MinGW when precompiled headers are used.
    }

    const auto firstNonSlash = component.find_first_not_of( L"/\\" );
    if ( firstNonSlash == std::wstring::npos ) {
        return {};
    }
    if ( firstNonSlash != 0 ) {
        component.erase( 0, firstNonSlash );
    }

    const auto firstNonSpace = component.find_first_not_of( L' ' );
    if ( firstNonSpace == std::wstring::npos ) {
        return L"_";
    }

    // If the component is a reserved name on Windows, we prepend it with a '_' character.
    if ( is_windows_reserved_name( component, firstNonSpace ) ) {
        component.insert( firstNonSpace, 1, L'_' );
        return component;
    }

    // Replacing all reserved characters in the component with the '_' character
    // (https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file).
    std::replace_if( component.begin(), component.end(), []( wchar_t chr ) -> bool {
        constexpr auto kLastNonPrintableAscii = 31;
        return chr <= kLastNonPrintableAscii || chr == L'<' || chr == L'>' || chr == L':' ||
               chr == L'"' || chr == L'/' || chr == L'\\' || chr == L'|' || chr == L'?' || chr == L'*';
    }, L'_' );
    return component;
}

BIT7Z_NODISCARD
auto sanitize_path_components( const fs::path& path ) -> fs::path {
    fs::path sanitizedPath;
    for( const auto& pathComponent : path ) {
        // cppcheck-suppress useStlAlgorithm
        sanitizedPath /= sanitize_path_component( pathComponent.wstring() );
    }
    return sanitizedPath;
}

BIT7Z_NODISCARD
auto is_drive_relative( const fs::path& path ) -> bool {
    // NOLINTBEGIN(*-pro-bounds-avoid-unchecked-container-access)
    const auto& nativePath = path.native();
    return nativePath.size() > 2 &&
           std::iswalpha( nativePath[0] ) != 0 &&
           nativePath[1] == L':' &&
           !isPathSeparator( nativePath[2] );
    // NOLINTEND(*-pro-bounds-avoid-unchecked-container-access)
}
} // namespace
#endif

#ifdef BIT7Z_PATH_SANITIZATION
auto fsutil::sanitize_path( const fs::path& path ) -> fs::path {
#ifdef _WIN32
    if ( path == L"/" ) {
        return L"_";
    }

    if ( is_drive_relative( path ) ) {
        // Drive-relative paths are sanitized as C_<rest of the path> (e.g., C:file.txt -> C_file.txt).
        const auto sanitizedRoot = sanitize_path_component( path.root_path().native() );
        return sanitizedRoot + sanitize_path_components( path.relative_path() ).native();
    }

    // Absolute paths: C:/abc/COM0/?invalid:filename.txt -> C_/abc/_COM0/_invalid_filename.txt
    // Relative paths:   /abc/COM0/?invalid:filename.txt -> abc/_COM0/_invalid_filename.txt
    //                    abc/COM0/?invalid:filename.txt -> abc/_COM0/_invalid_filename.txt
    return sanitize_path_components( path );
#else
    auto result = path.native();
    std::replace( result.begin(), result.end(), '\0', '_' );
    return fs::path{ std::move( result ) };
#endif
}
#endif

namespace {
#if !defined( _WIN32 ) || !defined( BIT7Z_PATH_SANITIZATION )
BIT7Z_NODISCARD
auto is_absolute( const fs::path& path ) -> bool {
#ifdef GHC_FILESYSTEM_VERSION
#   ifdef _WIN32
    // On Windows, ghc::filesystem considers \\server as not absolute (bug?), but we want a consistent behavior.
    return path.is_absolute() || ( starts_with( path.native(), L"\\\\" ) && !path.has_root_directory() );
#   else
    // ghc::filesystem considers //foo as not absolute (not a bug), but we want a consistent behavior.
    const auto& nativePath = path.native();
    return !nativePath.empty() && isPathSeparator( nativePath.front() );
#   endif
#else
    return path.is_absolute();
#endif
}
#endif

#if defined( BIT7Z_PATH_SANITIZATION ) && !defined( _WIN32 )
// On POSIX, sanitize_path only replaces NUL bytes; if there are none,
// skip the intermediate fs::path allocation and join the input directly.
BIT7Z_NODISCARD
auto join_with_sanitization( const fs::path& base, const fs::path& path ) -> fs::path {
    if ( path.native().find( fs::path::value_type{} ) == fs::path::string_type::npos ) {
        return base / path;
    }
    return base / fsutil::sanitize_path( path );
}
#endif

BIT7Z_NODISCARD
auto sanitize_path_join( const fs::path& base, const fs::path& path ) -> fs::path {
#if defined( _WIN32 ) && defined( BIT7Z_PATH_SANITIZATION )
    return base / fsutil::sanitize_path( path );
#else
#ifndef BIT7Z_PATH_SANITIZATION
    // Null bytes would cause a mismatch between the validated path and the OS-interpreted path,
    // which truncates at the first null on both POSIX (char*) and Win32 (LPWSTR) APIs.
    if ( path.native().find( fs::path::value_type{} ) != fs::path::string_type::npos ) {
        throw BitException(
            "Invalid item path",
            make_error_code( BitError::InvalidItemPath ),
            path_to_tstring( path )
        );
    }
#endif

    if ( is_absolute( path ) ) {
#ifdef BIT7Z_PATH_SANITIZATION
        // POSIX-only: strip leading slashes from the native string directly; relative_path()
        // is unreliable here because ghc::filesystem returns empty for //foo (consumed as
        // root_name) and returns //foo for ///foo (another root_name).
        const auto& native = path.native();
        const auto firstNonSlash = native.find_first_not_of( '/' );
        const auto strippedPath = firstNonSlash != fs::path::string_type::npos
            ? fs::path{ native.substr( firstNonSlash ) }
            : fs::path{};
        return join_with_sanitization( base, strippedPath );
#else
        throw BitException(
            "Invalid item path",
            make_error_code( BitError::ItemHasAbsolutePath ),
            path_to_tstring( path )
        );
#endif
    }

#ifdef _WIN32
    if ( path.has_root_name() && path.root_name() != base.root_name() ) {
        // "C:<path>" (but base path has different drive letter).
        throw BitException{
            "Cannot extract the item '" + path.string() + "'",
            BitError::ItemPathOutsideOutputDirectory
        };
    }

    if ( path.has_root_path() ) {
        // The path either has a root name (e.g., drive letter) or a root directory
        // (not both, since it's a relative path).
        // E.g., "C:<path>" or "/<path>" (which is drive-relative on Windows).
        return base / path.relative_path();
    }
#endif

#ifdef BIT7Z_PATH_SANITIZATION
    // POSIX-only
    return join_with_sanitization( base, path );
#else
    return base / path;
#endif
#endif
}

BIT7Z_NODISCARD
auto path_is_outside_base( const fs::path& path, const fs::path& basePath ) noexcept -> bool {
    const auto& nativePath = path.native();
    const auto& nativeBase = basePath.native();

#ifdef _WIN32
    /* Partially quoting Douglas Adams: “In the beginning, a case-insensitive filesystem was created.
     * This has made a lot of people very angry and been widely regarded as a bad move”.
     *
     * On Windows, paths are usually case-insensitive, i.e. out/dir/ is equivalent to OUT/DIR/.
     * Since Windows 10 version 1803 (build 17134) and later, users can set the content of a specific folder
     * to be case-sensitive, as on Unix systems.
     * However, we do not currently take this edge case into account,
     * as it is not easy to handle programmatically and is not worth the hassle.
     * In the worst case, we reject extracting a file that could be extracted (not a security issue). */
    const auto result = std::mismatch(
        nativeBase.cbegin(),
        nativeBase.cend(),
        nativePath.cbegin(),
        nativePath.cend(),
        []( wchar_t first, wchar_t second ) noexcept -> bool {
            /* Here we use user32.lib's CharUpperBuffW because it provides case conversions regardless of locale.
             *
             * According to some old MSDN blog posts, using CharUpperBuffW (or CharUpperW) is
             * the correct way to compare paths on Windows, i.e. the closest to how the OS behaves.
             * References: https://archives.miloush.net/michkap/archive/2005/10/17/481600.html
             *             https://archives.miloush.net/michkap/archive/2005/10/18/482088.html
             *
             * Note: CharUpperW would work as well, but it requires some reinterpret_casts to work properly.
             *       CompareStringOrdinal also would work, but it has slightly more overhead for single characters.
             *
             * Note 2: The C++ standard alternative, std::towupper, is locale-dependent,
             *         and often doesn't correctly handle non-ASCII. */
            CharUpperBuffW( &first, 1 );
            CharUpperBuffW( &second, 1 );
            return first == second;
        }
    );
#else
    const auto result = std::mismatch( nativeBase.begin(), nativeBase.end(), nativePath.begin(), nativePath.end() );
#endif

    if ( result.first != nativeBase.cend() ) { // Base path didn't fully match.
        /* Here the path is inside the base only if it fully matched,
         * the mismatch in the base path is on its last character,
         * and this latter is a path separator (e.g., base:/home/user/, path:/home/user).*/
        return result.second != nativePath.cend() || // The path didn't fully match -> outside.
               std::next( result.first ) != nativeBase.cend() || // Mismatch not on last base char -> outside.
               !isPathSeparator( nativeBase.back() ); // Last base char (mismatch) is not a path separator -> outside.
    }

    if ( result.second == nativePath.cend() ) {
        return false; // Both base and path matched fully, thus they're the same path (inside).
    }

    if ( isPathSeparator( nativeBase.back() ) ) {
        return false; // Base path fully matched, prefix match is sufficient (inside).
    }

    /* Here, base path fully matched, and it doesn't end with a trailing path separator.
     * If the first mismatched character on the path is a path separator, the path is inside,
     * e.g., base = /home/john and path = /home/john/hello.txt (in general path = <base>/<...>).
     * Otherwise, the path is outside (e.g., base = /home/john and path = /home/johnny).*/
    return !isPathSeparator( *result.second );
}

BIT7Z_NODISCARD
auto sanitize_base_path( const tstring& basePath ) -> fs::path {
    if ( basePath.empty() || basePath == BIT7Z_STRING( "." ) ) {
        return fs::current_path();
    }
    auto result = tstring_to_path( basePath );
    if ( !result.is_absolute() ) {
        result = fs::absolute( result );
    }
    return result.lexically_normal();
}
} // namespace
} // namespace filesystem

SafeOutPathBuilder::SafeOutPathBuilder( const tstring& basePath )
    : mBasePath{ filesystem::sanitize_base_path( basePath ) } {
#ifdef _WIN32
    if ( mBasePath == BIT7Z_NATIVE_STRING( "//" ) || mBasePath == BIT7Z_NATIVE_STRING( "\\\\" ) ) {
        throw BitException{ "Invalid output base path", BitError::InvalidDirectoryPath };
    }
#endif
}

auto SafeOutPathBuilder::buildPath( const fs::path& path ) const -> fs::path {
    if ( path.empty() ) {
        return mBasePath;
    }

    // TODO: Avoid normalization if not needed.
    auto builtPath = filesystem::sanitize_path_join( mBasePath, path ).lexically_normal();
    if ( filesystem::path_is_outside_base( builtPath, mBasePath ) ) {
        throw BitException{ "Cannot extract the item '" + path.string() + "'",
                    BitError::ItemPathOutsideOutputDirectory };
    }

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
    if ( filesystem::fsutil::should_format_long_path( builtPath ) ) {
        return filesystem::fsutil::format_long_path( builtPath );
    }
#endif

    return builtPath;
}

#ifndef _WIN32
// TODO: Add support for symbolic links on Windows (reparse points).
// TODO: Add support for stopping the extraction process if the symbolic link cannot be restored.
auto SafeOutPathBuilder::restoreSymlink( const fs::path& symlinkFilePath ) const -> bool {
    fs::ifstream ifs( symlinkFilePath, std::ios::in | std::ios::binary );
    if ( !ifs.is_open() ) {
        return false;
    }

    // Reading the path stored in the link file.
    std::string targetPath;
    targetPath.resize( MAX_PATH_SIZE );
    // NOLINTNEXTLINE(readability-container-data-pointer, *-pro-bounds-avoid-unchecked-container-access)
    ifs.read( &targetPath[ 0 ], MAX_PATH_SIZE );

    if ( ifs.bad() ) { // Error while reading the path, exiting.
        return false;
    }

    // Shrinking the path string to its actual size.
    targetPath.resize( static_cast< std::size_t >( ifs.gcount() ) );

    // No need to keep the file open.
    ifs.close();

    // Removing the link file.
    std::error_code error;
    fs::remove( symlinkFilePath, error );

    if ( error ) {
        return false;
    }

    const auto symlinkParent = symlinkFilePath.parent_path();

    // TODO: Avoid normalization if not needed.
    const auto resolvedTargetPath = filesystem::sanitize_path_join( symlinkParent, targetPath ).lexically_normal();
    if ( filesystem::path_is_outside_base( resolvedTargetPath, mBasePath ) ) {
        return false;
    }

    // We determined that the resolved symlink target path is inside the base path.
    // So it is safe to create the symbolic link using the target path stored in the archive.

    // sanitize_path_join may have rewritten targetPath; derive target from validated result.
    const auto symlinkTarget = resolvedTargetPath.lexically_relative( symlinkParent );
    if ( symlinkTarget.empty() ) {
        return false; // Defensive, should not happen.
    }

    // Restoring the symbolic link to the target file.
    // Note: when the OS will resolve this symbolic link target, it will resolve it against the symlink's parent path.
    fs::create_symlink( symlinkTarget, symlinkFilePath, error );
    return !error;
}
#endif

auto SafeOutPathBuilder::basePath() const -> const fs::path& {
    return mBasePath;
}
} // namespace bit7z
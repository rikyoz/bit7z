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

#include "internal/fsutil.hpp"

#include <algorithm> //for std::adjacent_find

#ifndef _WIN32
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>

#include "internal/dateutil.hpp"
#endif

using namespace std;
using namespace bit7z;
using namespace bit7z::filesystem;

tstring fsutil::filename( const tstring& path, bool ext ) {
    const size_t start = path.find_last_of( BIT7Z_STRING( "/\\" ) ) + 1;
    const size_t end = ext ? path.size() : path.find_last_of( BIT7Z_STRING( '.' ) );
    return path.substr( start, end - start ); //RVO :)
}

tstring fsutil::extension( const tstring& path ) {
    tstring name = filename( path, true );
    const size_t last_dot = name.find_last_of( BIT7Z_STRING( '.' ) );
    return last_dot != tstring::npos ? name.substr( last_dot + 1 ) : tstring{};
}

bool contains_dot_references( const fs::path& path ) {
    /* Note: here we suppose that path does not contain file names with a final dot (e.g., "foo.").
             This must be true on Windows, but not on Unix systems! */
    const auto& native_path = path.native();
    return std::adjacent_find( native_path.begin(), native_path.end(), []( tchar a, tchar b ) {
        return a == BIT7Z_STRING( '.' ) && ( b == BIT7Z_STRING( '/' ) || b == BIT7Z_STRING( '\\') );
    } ) != native_path.end();
}

fs::path fsutil::inArchivePath( const fs::path& file_path, const fs::path& search_path ) {
    /* Note: the following algorithm tries to emulate the behavior of 7-zip when dealing with
             paths of items in archives. */

    const auto& normal_path = file_path.lexically_normal();

    auto filename = normal_path.filename();
    if ( filename == "." || filename == ".." ) {
        return {};
    }
    if ( filename.empty() ) {
        filename = normal_path.parent_path().filename();
    }

    if ( file_path.is_absolute() || contains_dot_references( file_path ) ) {
        // Note: in this case if the file was found while indexing a directory passed by the user, we need to retain
        // the internal structure of that folder (mSearchPath), otherwise we use only the file name.
        if ( search_path.empty() ) {
            return filename;
        }
        return search_path / filename;
    }

    // Here, path is relative and without ./ or ../ => e.g. foo/bar/test.txt

    if ( !search_path.empty() ) {
        // The item was found while indexing a directory
        return search_path / filename;
    }
    return file_path;
}

// A modified version of the code found here: https://stackoverflow.com/a/3300547
bool w_match( tstring::const_iterator pattern_it, const tstring::const_iterator& pattern_end,
              tstring::const_iterator str_it, const tstring::const_iterator& str_end ) {
    for ( ; pattern_it != pattern_end; ++pattern_it ) {
        switch ( *pattern_it ) {
            case BIT7Z_STRING( '?' ):
                if ( str_it == str_end ) {
                    return false;
                }
                ++str_it;
                break;
            case BIT7Z_STRING( '*' ): {
                if ( pattern_it + 1 == pattern_end ) {
                    return true;
                }
                for ( auto i = str_it; i != str_end; ++i ) {
                    if ( w_match( pattern_it + 1, pattern_end, i, str_end ) ) {
                        return true;
                    }
                }
                return false;
            }
            default:
                if ( str_it == str_end || *str_it != *pattern_it ) {
                    return false;
                }
                ++str_it;
        }
    }
    return str_it == str_end;
}

bool fsutil::wildcardMatch( const tstring& pattern, const tstring& str ) {
    if ( pattern.empty() ) {
        return wildcardMatch( BIT7Z_STRING( "*" ), str );
    }
    return w_match( pattern.cbegin(), pattern.cend(), str.begin(), str.end() );
}

#ifndef _WIN32
bool restore_symlink( const std::string& name ) {
    std::ifstream ifs( name, std::ios::in | std::ios::binary );
    if ( !ifs.is_open() ) {
        return false;
    }

    // Reading the path stored in the link file.
    std::string link_path;
    link_path.resize( MAX_PATHNAME_LEN );
    ifs.getline( &link_path[ 0 ], MAX_PATHNAME_LEN ); // NOLINT(readability-container-data-pointer)

    if ( !ifs ) { // Error while reading the path, exiting.
        return false;
    }

    // Shrinking the path string to its actual size.
    link_path.resize( ifs.gcount() );

    // No need to keep the file open.
    ifs.close();

    // Removing the link file.
    std::error_code ec;
    fs::remove( name, ec );

    // Restoring the symbolic link to the target file.
    return !ec && symlink( link_path.c_str(), name.c_str() ) == 0;
}

static const mode_t global_umask = []() noexcept {
    // Getting and setting the current umask.
    mode_t current_umask{ umask( 0 ) };

    // Restoring the umask.
    umask( current_umask );

    return 0777 & ( ~current_umask );
}();
#endif

bool fsutil::setFileAttributes( const fs::path& filePath, DWORD attributes ) noexcept {
    if ( filePath.empty() ) {
        return false;
    }

#ifdef _WIN32
    return ::SetFileAttributes( filePath.c_str(), attributes ) != FALSE;
#else
    struct stat file_stat{};
    if ( lstat( filePath.c_str(), &file_stat ) != 0 ) {
        return false;
    }

    if ( attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) {
        file_stat.st_mode = attributes >> 16u;
        if ( S_ISLNK( file_stat.st_mode ) ) {
            return restore_symlink( filePath );
        } else if ( S_ISDIR( file_stat.st_mode ) ) {
            file_stat.st_mode |= ( S_IRUSR | S_IWUSR | S_IXUSR );
        } else if ( !S_ISREG( file_stat.st_mode ) ) {
            return true;
        }
    } else if ( S_ISLNK( file_stat.st_mode ) ) {
        return true;
    } else if ( !S_ISDIR( file_stat.st_mode ) && ( attributes & FILE_ATTRIBUTE_READONLY ) != 0 ) {
        file_stat.st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
    }
    chmod( filePath.c_str(), file_stat.st_mode & global_umask );
    return true;
#endif
}

bool fsutil::setFileModifiedTime( const fs::path& filePath, const FILETIME& ftModified ) noexcept {
    if ( filePath.empty() ) {
        return false;
    }

#ifdef _WIN32
    bool res = false;
    HANDLE hFile = ::CreateFile( filePath.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, nullptr,
                                 OPEN_EXISTING, 0, nullptr );
    if ( hFile != INVALID_HANDLE_VALUE ) { // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        res = ::SetFileTime( hFile, nullptr, nullptr, &ftModified ) != FALSE;
        CloseHandle( hFile );
    }
    return res;
#else
    std::error_code ec;
    auto ft = FILETIME_to_file_time_type( ftModified );
    fs::last_write_time( filePath, ft, ec );
    return !ec;
#endif
}

bool fsutil::getFileAttributesEx( const fs::path& filePath, WIN32_FILE_ATTRIBUTE_DATA& fileMetadata ) noexcept {
    if ( filePath.empty() ) {
        return false;
    }

#ifdef _WIN32
    return ::GetFileAttributesEx( filePath.c_str(), GetFileExInfoStandard, &fileMetadata ) != FALSE;
#else
    struct stat stat_info{};
    if ( lstat( filePath.c_str(), &stat_info ) != 0 ) {
        return false;
    }

    // File attributes
    fileMetadata.dwFileAttributes = S_ISDIR( stat_info.st_mode ) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_ARCHIVE;
    if ( ( stat_info.st_mode & S_IWUSR ) == 0 ) {
        fileMetadata.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    }
    fileMetadata.dwFileAttributes |= FILE_ATTRIBUTE_UNIX_EXTENSION + ( ( stat_info.st_mode & 0xFFFF ) << 16 );

    // File times
    fileMetadata.ftCreationTime = time_to_FILETIME( stat_info.st_ctime );
    fileMetadata.ftLastAccessTime = time_to_FILETIME( stat_info.st_atime );
    fileMetadata.ftLastWriteTime = time_to_FILETIME( stat_info.st_mtime );
    return true;
#endif
}

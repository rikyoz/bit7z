// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "../include/fsutil.hpp"

#ifndef _WIN32
#include <chrono>
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>
#include <myWindows/StdAfx.h>
#endif

using namespace std;
using namespace bit7z;
using namespace bit7z::filesystem;


tstring fsutil::filename( const tstring& path, bool ext ) {
    size_t start = path.find_last_of( TSTRING( "/\\" ) ) + 1;
    size_t end = ext ? path.size() : path.find_last_of( L'.' );
    return path.substr( start, end - start ); //RVO :)
}

tstring fsutil::extension( const tstring& path ) {
    tstring name = filename( path, true );
    size_t last_dot = name.find_last_of( L'.' );
    return last_dot != tstring::npos ? name.substr( last_dot + 1 ) : TSTRING( "" );
}

bool contains_dot_references( const fs::path& path ) {
    /* Note: here we are supposing that path does not contain file names with a final dot (e.g., "foo.").
             This must be true on Windows, but not on Unix systems! */
    const auto& native_path = path.native();
    return std::adjacent_find( native_path.begin(), native_path.end(), []( tchar a, tchar b ) {
        return a == TSTRING('.') && ( b == TSTRING('/') || b == TSTRING('\\') );
    } ) != native_path.end();
}

fs::path fsutil::inArchivePath( const fs::path& file_path, const fs::path& search_path ) {
    /* Note: the following algorithm tries to emulate the behavior of 7-zip when dealing with
             paths of items in archives. */

    const auto& normal_path = file_path.lexically_normal();

    auto filename = normal_path.filename();
    if ( filename == "." || filename == ".." ) {
        return fs::path();
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

// Modified version of code found here: https://stackoverflow.com/a/3300547
bool w_match( const tchar* needle, const tchar* haystack, size_t max ) {
    for ( ; *needle != TSTRING( '\0' ); ++needle ) {
        switch ( *needle ) {
            case TSTRING( '?' ):
                if ( *haystack == TSTRING( '\0' ) ) {
                    return false;
                }
                ++haystack;
                break;
            case TSTRING( '*' ): {
                if ( needle[ 1 ] == TSTRING( '\0' ) ) {
                    return true;
                }
                for ( size_t i = 0; i < max; i++ ) {
                    if ( w_match( needle + 1, haystack + i, max - i ) ) {
                        return true;
                    }
                }
                return false;
            }
            default:
                if ( *haystack != *needle ) {
                    return false;
                }
                ++haystack;
        }
    }
    return *haystack == TSTRING( '\0' );
}

bool fsutil::wildcardMatch( const tstring& pattern, const tstring& str ) {
    return w_match( pattern.empty() ? TSTRING( "*" ) : pattern.c_str(), str.c_str(), str.size() );
}

#ifndef _WIN32 //code from p7zip
static int convert_to_symlink( const tstring& name ) {
    FILE* file = fopen( name.c_str(), "rb" );
    if ( file ) {
        char buf[MAX_PATHNAME_LEN + 1];
        char* ret = fgets( buf, sizeof( buf ) - 1, file );
        fclose( file );
        if ( ret ) {
            int ir = unlink( name.c_str() );
            if ( ir == 0 ) {
                ir = symlink( buf, name.c_str() );
            }
            return ir;
        }
    }
    return -1;
}

class Umask {
    public:
        mode_t current_umask;
        mode_t mask;

        Umask() {
            current_umask = umask( 0 );  /* get and set the umask */
            umask( current_umask );   /* restore the umask */
            mask = 0777 & ( ~current_umask );
        }
};

static Umask gbl_umask;
#endif

bool fsutil::setFileAttributes( const fs::path& filePath, DWORD attributes ) {
#ifdef _WIN32
    return ::SetFileAttributes( filePath.c_str(), attributes ) != FALSE;
#else
    struct stat stat_info{};
    if ( lstat( filePath.c_str(), &stat_info ) != 0 ) {
        return false;
    }

    if ( attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) {
        stat_info.st_mode = attributes >> 16u;
        if ( S_ISLNK( stat_info.st_mode ) ) {
            if ( convert_to_symlink( filePath ) != 0 ) {
                return false;
            }
        } else if ( S_ISDIR( stat_info.st_mode ) ) {
            stat_info.st_mode |= ( S_IRUSR | S_IWUSR | S_IXUSR );
        }
        chmod( filePath.c_str(), stat_info.st_mode & gbl_umask.mask );
    } else if ( !S_ISLNK( stat_info.st_mode ) ) {
        if ( !S_ISDIR( stat_info.st_mode ) && attributes & FILE_ATTRIBUTE_READONLY ) {
            stat_info.st_mode &= ~0222;
        }
        chmod( filePath.c_str(), stat_info.st_mode & gbl_umask.mask );
    }

    return true;
#endif
}

#ifndef _WIN32
// 100ns intervals
using FileTimeTickRate = std::ratio<1, 10'000'000>;
// FileTimeDuration has the same layout as FILETIME;
using FileTimeDuration = chrono::duration< int64_t, FileTimeTickRate >;
// January 1, 1601 (NT epoch) - January 1, 1970 (Unix epoch):
constexpr chrono::seconds nt_to_unix_epoch{ -11644473600 };

fs::file_time_type FILETIME_to_file_time_type( const FILETIME& fileTime ) {
    const FileTimeDuration asDuration{
        static_cast< int64_t >( ( static_cast< uint64_t >( fileTime.dwHighDateTime ) << 32u ) | fileTime.dwLowDateTime )
    };
    const auto withUnixEpoch = asDuration + nt_to_unix_epoch;
    return fs::file_time_type{ chrono::duration_cast< chrono::system_clock::duration >( withUnixEpoch ) };
}
#endif

bool fsutil::setFileModifiedTime( const fs::path& filePath, const FILETIME& ftModified ) {
    if ( filePath.empty() ) {
        return false;
    }

#ifdef _WIN32
    bool res = false;
    HANDLE hFile = ::CreateFile( filePath.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, nullptr,
                                 OPEN_EXISTING, 0, nullptr );
    if ( hFile != INVALID_HANDLE_VALUE ) {
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

#ifndef _WIN32
FILETIME time_to_FILETIME( const std::time_t& time ) {
    uint64_t secs = ( time * 10000000ull ) + 116444736000000000;
    FILETIME fileTime;
    fileTime.dwLowDateTime = static_cast< DWORD >( secs );
    fileTime.dwHighDateTime = static_cast< DWORD >( secs >> 32 );
    return fileTime;
}
#endif

bool fsutil::getFileAttributesEx( const fs::path& filePath, WIN32_FILE_ATTRIBUTE_DATA& fileInfo ) {
#ifdef _WIN32
    return ::GetFileAttributesEx( filePath.c_str(), GetFileExInfoStandard, &fileInfo ) != FALSE;
#else
    struct stat stat_info{};
    if ( lstat( filePath.c_str(), &stat_info ) != 0 ) {
        return false;
    }
    fileInfo.dwFileAttributes = S_ISDIR( stat_info.st_mode ) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_ARCHIVE;
    if ( !( stat_info.st_mode & S_IWUSR ) ) {
        fileInfo.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    }
    fileInfo.dwFileAttributes |= FILE_ATTRIBUTE_UNIX_EXTENSION + ( ( stat_info.st_mode & 0xFFFF ) << 16 );

    fileInfo.ftCreationTime = time_to_FILETIME( stat_info.st_ctime );
    fileInfo.ftLastAccessTime = time_to_FILETIME( stat_info.st_atime );
    fileInfo.ftLastWriteTime = time_to_FILETIME( stat_info.st_mtime );
    return true;
#endif
}

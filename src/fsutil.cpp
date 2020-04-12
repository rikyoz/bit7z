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

using namespace std;
using namespace bit7z;
using namespace bit7z::filesystem;


wstring fsutil::filename( const wstring& path, bool ext ) {
    size_t start = path.find_last_of( L"/\\" ) + 1;
    size_t end = ext ? path.size() : path.find_last_of( L'.' );
    return path.substr( start, end - start ); //RVO :)
}

wstring fsutil::extension( const wstring& path ) {
    wstring name = filename( path, true );
    size_t last_dot = name.find_last_of( L'.' );
    return last_dot != wstring::npos ? name.substr( last_dot + 1 ) : L"";
}

bool fsutil::setFileModifiedTime( const fs::path& filePath, const FILETIME& ft_modified ) {
    if ( filePath.empty() ) {
        return false;
    }

    bool res = false;
    HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, 0, nullptr );
    if ( hFile != INVALID_HANDLE_VALUE ) {
        res = SetFileTime( hFile, nullptr, nullptr, &ft_modified ) != FALSE;
        CloseHandle( hFile );
    }
    return res;
}

// Modified version of code found here: https://stackoverflow.com/a/3300547
bool w_match( const wchar_t* needle, const wchar_t* haystack, size_t max ) {
    for ( ; *needle != L'\0'; ++needle ) {
        switch ( *needle ) {
            case L'?':
                if ( *haystack == L'\0' ) {
                    return false;
                }
                ++haystack;
                break;
            case L'*': {
                if ( needle[ 1 ] == L'\0' ) {
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
    return *haystack == L'\0';
}

bool fsutil::wildcardMatch( const wstring& pattern, const wstring& str ) {
    return w_match( pattern.empty() ? L"*" : pattern.c_str(), str.c_str(), str.size() );
}

uint32_t fsutil::getFileAttributes( const wstring& name ) {
#ifdef _WIN32
    return ::GetFileAttributes( name.c_str() );
#else

#endif
}

bool fsutil::setFileAttributes( const fs::path& filePath, uint32_t attributes ) {
#ifdef _WIN32
    return ::SetFileAttributes( filePath.c_str(), attributes ) != FALSE;
#else
    struct stat stat_info {};
    if ( lstat( name, &stat_info ) != 0 ) {
        return false;
    }

    if ( fileAttributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) {
        stat_info.st_mode = fileAttributes >> 16;
        if ( S_ISLNK( stat_info.st_mode ) ) {
            if ( convert_to_symlink( name ) != 0 ) {
                return false;
            }
        } else if ( S_ISDIR( stat_info.st_mode ) ) {
            stat_info.st_mode |= ( S_IRUSR | S_IWUSR | S_IXUSR );
        }
        chmod( name, stat_info.st_mode & gbl_umask.mask );
    } else if ( !S_ISLNK( stat_info.st_mode ) ) {
        if ( !S_ISDIR( stat_info.st_mode ) && fileAttributes & FILE_ATTRIBUTE_READONLY ) {
            stat_info.st_mode &= ~0222;
        }
        chmod( name, stat_info.st_mode & gbl_umask.mask );
    }

    return true;
#endif
}

bool fsutil::getFileTimes( const fs::path& filePath, FILETIME& creationTime, FILETIME& accessTime, FILETIME& writeTime ) {
#ifdef _WIN32
    bool res = false;
    HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, nullptr,
                              OPEN_EXISTING, 0, nullptr );
    if ( hFile != INVALID_HANDLE_VALUE ) {
        res = ::GetFileTime( hFile, &creationTime, &accessTime, &writeTime ) != FALSE;
        CloseHandle( hFile );
    }
    return res;
#else
#endif
}

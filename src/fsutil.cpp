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

#include <Windows.h>

using namespace std;
using namespace bit7z;
using namespace bit7z::filesystem;

bool fsutil::isDirectory( const wstring& path ) {
    return 0 != ( GetFileAttributes( path.c_str() ) & FILE_ATTRIBUTE_DIRECTORY );
}

bool fsutil::pathExists( const wstring& path ) {
    return GetFileAttributes( path.c_str() ) != INVALID_FILE_ATTRIBUTES;
}

bool fsutil::renameFile( const wstring& old_name, const wstring& new_name ) {
    //NOTE: It overwrites the destination file!
    return MoveFileEx( old_name.c_str(), new_name.c_str(), MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING ) !=
           FALSE; //WinAPI BOOL
}

void fsutil::normalizePath( wstring& path ) { //this assumes that the passed path is not a file path!
    if ( !path.empty() && path.back() != L'\\' && path.back() != L'/' ) {
        path.append( L"\\" );
    }
}

wstring fsutil::dirname( const wstring& path ) {
    //the directory containing the path (hence, up directory if the path is a folder)
    size_t pos = path.find_last_of( L"\\/" );
    return ( pos != wstring::npos ) ? path.substr( 0, pos ) : L"";
}

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

// TODO: check if find_first_of is necessary or use front()
bool fsutil::isRelativePath( const wstring& path ) {
    //return PathIsRelativeW( path.c_str() ); //WinAPI version (requires Shlwapi lib!)
    return path.empty() || ( path.find_first_of( L"/\\" ) != 0 && !( path.length() >= 2 && path[ 1 ] == L':' ) );
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

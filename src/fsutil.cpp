// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/fsutil.hpp"

#include "../include/bitexception.hpp"

#include <Windows.h>

using namespace std;
using namespace bit7z::filesystem;

bool fsutil::is_directory( const wstring& path ) {
    return 0 != ( GetFileAttributes( path.c_str() ) & FILE_ATTRIBUTE_DIRECTORY );
}

bool fsutil::path_exists( const wstring& path ) {
    return GetFileAttributes( path.c_str() ) != INVALID_FILE_ATTRIBUTES;
}

bool fsutil::has_ending( wstring const& str, const wstring& ending ) {
    return ( str.length() >= ending.length() ) &&
           ( 0 == str.compare( str.length() - ending.length(), ending.length(), ending ) );
}

void fsutil::normalize_path( wstring& path ) { //this assumes that the passed path is not a file path!
    if ( path.empty() ) {
        return;
    }
    if ( path.find_last_of( L"\\/" ) != path.length() - 1 ) {
        path.append( L"\\" );
    }
}

wstring fsutil::filename( const wstring& path, bool ext ) {
    size_t start = path.find_last_of( L"/\\" ) + 1;
    size_t end   = ext ? path.size() : path.find_last_of( L'.' );
    return path.substr( start, end - start ); //RVO :)
}

wstring fsutil::extension( const wstring& path ) {
    /*size_t last_dot = path.find_last_of( L'.' ) + 1;
    size_t last_sep = path.find_last_of( L"/\\") + 1;
    return last_dot != wstring::npos && last_sep < last_dot ? path.substr( last_dot ) : L""; */
    wstring name = filename( path, true );
    size_t last_dot = name.find_last_of( L'.' );
    return last_dot != wstring::npos ? name.substr( last_dot + 1 ) : L"";
}

// TODO: check if find_first_of is necessary or use front()
bool fsutil::is_relative_path( const wstring& path ) {
    //return PathIsRelativeW( path.c_str() ); //WinAPI version (requires Shlwapi lib!)
    return path.empty() || ( path.find_first_of(L"/\\") != 0 && !( path.length() >= 2 && path[1] == L':' ) );
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
                if ( needle[1] == L'\0' ) {
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

bool fsutil::wildcard_match( const wstring& pattern, const wstring& str ) {
    return w_match( pattern.empty() ? L"*" : pattern.c_str(), str.c_str(), str.size() );
}

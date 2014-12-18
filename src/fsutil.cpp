#include "../include/fsutil.hpp"

#include "../include/bitexception.hpp"

#include <Windows.h>

using namespace bit7z::filesystem;

bool FSUtil::is_directory( const wstring& path ) {
    return ( GetFileAttributes( path.c_str() ) & FILE_ATTRIBUTE_DIRECTORY ) != 0;
}

bool FSUtil::path_exists( const wstring& path ) {
    return ( GetFileAttributes( path.c_str() ) != INVALID_FILE_ATTRIBUTES );
}

bool FSUtil::has_ending( wstring const& str, const wstring& ending ) {
    return ( str.length() >= ending.length() )
           && ( 0 == str.compare( str.length() - ending.length(), ending.length(),
                                  ending ) );
}

void FSUtil::normalize_path( wstring& path ) {
    if ( path.empty() ) return;
    if ( path.find_last_of( L"\\/" ) != path.length() - 1 )
        path.append( L"\\" );
}

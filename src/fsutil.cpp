#include "../include/fsutil.hpp"

#include "../include/bitexception.hpp"

#include <Windows.h>

using namespace bit7z::filesystem;

bool fsutil::is_directory( const wstring& path ) {
    return 0 != ( GetFileAttributes( path.c_str() ) & FILE_ATTRIBUTE_DIRECTORY );
}

bool fsutil::path_exists( const wstring& path ) {
    return ( GetFileAttributes( path.c_str() ) != INVALID_FILE_ATTRIBUTES );
}

bool fsutil::has_ending( wstring const& str, const wstring& ending ) {
    return ( str.length() >= ending.length() ) &&
           ( 0 == str.compare( str.length() - ending.length(), ending.length(), ending ) );
}

void fsutil::normalize_path( wstring& path ) {
    if ( path.empty() ) return;
    if ( path.find_last_of( L"\\/" ) != path.length() - 1 )
        path.append( L"\\" );
}

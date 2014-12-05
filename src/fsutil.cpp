#include "../include/fsutil.hpp"

#include "../include/bitexception.hpp"

using namespace Bit7z::FileSystem;

bool Util::is_directory( const wstring& path ) {
    return ( GetFileAttributes( path.c_str() ) & FILE_ATTRIBUTE_DIRECTORY ) != 0;
}

bool Util::path_exists( const wstring& path ) {
    return ( GetFileAttributes( path.c_str() ) != INVALID_FILE_ATTRIBUTES );
}

bool Util::has_ending(wstring const& str, const wstring& ending ) {
    return ( str.length() >= ending.length() )
           && ( 0 == str.compare( str.length() - ending.length(), ending.length(),
                                  ending ) );
}

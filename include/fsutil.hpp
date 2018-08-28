#ifndef BITFILESYSTEM_HPP
#define BITFILESYSTEM_HPP

#include <iostream>

namespace bit7z {
    namespace filesystem {
        namespace fsutil {
            using std::wstring;

            bool is_relative_path( const wstring& path );

            bool is_directory( const wstring& path );
            bool path_exists( const wstring& path );
            bool has_ending( const wstring& str, const wstring& ending );

            void normalize_path( wstring& path );
            wstring dirname( const wstring& path );
            wstring filename( const wstring& path, bool ext = false );
            wstring extension( const wstring& path );
            bool wildcard_match( const wstring& pattern, const wstring& str );
        }
    }
}
#endif // BITFILESYSTEM_HPP

#ifndef BITFILESYSTEM_HPP
#define BITFILESYSTEM_HPP

#include <iostream>

namespace bit7z {
    namespace filesystem {
        namespace fsutil {

            using std::wstring;

            bool is_directory( const wstring& path );
            bool path_exists( const wstring& path );
            bool has_ending( const wstring& str, const wstring& ending );

            void normalize_path( wstring& path );
        }
    }
}

#endif // BITFILESYSTEM_HPP

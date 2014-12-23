#ifndef BITFILESYSTEM_HPP
#define BITFILESYSTEM_HPP

#include <iostream>

using namespace std;

namespace bit7z {
    namespace filesystem {
        namespace fsutil {
            bool is_directory( const wstring& path );
            bool path_exists( const wstring& path );
            bool has_ending( const wstring& str, const wstring& ending );

            void normalize_path( wstring& path );
        }
    }
}

#endif // BITFILESYSTEM_HPP

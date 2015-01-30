#ifndef BITFILESYSTEM_HPP
#define BITFILESYSTEM_HPP

#include <iostream>

namespace bit7z {
    namespace filesystem {
        namespace fsutil {
            bool is_directory( const std::wstring& path );
            bool path_exists( const std::wstring& path );
            bool has_ending( const std::wstring& str, const std::wstring& ending );

            void normalize_path( std::wstring& path );
        }
    }
}

#endif // BITFILESYSTEM_HPP

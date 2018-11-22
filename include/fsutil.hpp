/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
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

#ifndef FSUTIL_HPP
#define FSUTIL_HPP

#include <iostream>

namespace bit7z {
    namespace filesystem {
        namespace fsutil {
            using std::wstring;

            bool is_relative_path( const wstring& path );

            bool is_directory( const wstring& path );
            bool path_exists( const wstring& path );
            bool rename_file( const wstring& old_name, const wstring& new_name );
            //bool has_ending( const wstring& str, const wstring& ending );

            void normalize_path( wstring& path );
            wstring dirname( const wstring& path );
            wstring filename( const wstring& path, bool ext = false );
            wstring extension( const wstring& path );
            bool wildcard_match( const wstring& pattern, const wstring& str );
        }
    }
}
#endif // FSUTIL_HPP

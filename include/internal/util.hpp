/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <limits>

#ifndef _WIN32
#include <string>
#endif

namespace bit7z {
#ifdef _WIN32
#define WIDEN( tstr ) tstr
#else
    using std::string;
    using std::wstring;

    string narrow( const wchar_t* wideString, size_t size );
    wstring widen( const string& narrowString );

#define WIDEN( tstr ) bit7z::widen(tstr)
#endif

    constexpr bool check_overflow( int64_t position, int64_t offset ) noexcept {
        return ( offset > 0 && position > (std::numeric_limits< int64_t >::max)() - offset ) ||
               ( offset < 0 && position < (std::numeric_limits< int64_t >::min)() - offset );
    }

}

#endif //UTIL_HPP

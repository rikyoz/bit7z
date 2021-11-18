// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#ifndef _WIN32
#include "internal/util.hpp"
#include <codecvt>
#include <sstream>

using std::wostringstream;
using std::ctype;
using std::use_facet;
using convert_type = std::codecvt_utf8<wchar_t>;

using namespace bit7z;

string bit7z::narrow( const wchar_t* wideString ) {
    if ( wideString == nullptr ) {
        return "";
    }
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes( wideString );
}

wstring bit7z::widen( const string& narrowString ) {
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.from_bytes( narrowString );
}
#endif
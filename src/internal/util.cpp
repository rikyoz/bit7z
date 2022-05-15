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

#include "internal/util.hpp"
#include <locale>

#ifndef WIN32
#include <codecvt>
using convert_type = std::codecvt_utf8<wchar_t>;
#endif

using namespace bit7z;

std::string bit7z::narrow( const wchar_t* wideString, size_t size ) {
    if ( wideString == nullptr ) {
        return "";
    }
#ifdef WIN32
    int narrowStringSize = WideCharToMultiByte( CP_UTF8,
                                                0,
                                                wideString,
                                                ( size != 0U ? static_cast<int>( size ) : -1 ),
                                                nullptr,
                                                0,
                                                nullptr,
                                                nullptr );
    if ( narrowStringSize == 0 ) {
        return "";
    }

    std::string result( narrowStringSize, 0 );
    WideCharToMultiByte( CP_UTF8,
                         0,
                         wideString,
                         -1,
                         &result[0],  // NOLINT(readability-container-data-pointer)
                         static_cast<int>( narrowStringSize ),
                         nullptr,
                         nullptr );
    if ( size == 0U ) {
        result.resize( narrowStringSize - 1 );
    } //output is null-terminated
    return result;
#else
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes( wideString, wideString + size );
#endif
}

std::wstring bit7z::widen( const std::string& narrowString ) {
#ifdef WIN32
    int wideStringSize = MultiByteToWideChar( CP_UTF8,
                                              0,
                                              narrowString.c_str(),
                                              static_cast<int>( narrowString.size() ),
                                              nullptr,
                                              0 );
    if ( wideStringSize == 0 ) {
        return L"";
    }

    std::wstring result( wideStringSize, 0 );
    MultiByteToWideChar( CP_UTF8,
                         0,
                         narrowString.c_str(),
                         static_cast<int>( narrowString.size() ),
                         &result[0], // NOLINT(readability-container-data-pointer)
                         wideStringSize );
    return result;
#else
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.from_bytes( narrowString );
#endif
}
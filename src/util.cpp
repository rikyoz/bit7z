// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2020  Riccardo Ostani - All Rights Reserved.
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
#include "../include/util.hpp"
#include <sstream>

using std::ostringstream;
using std::wostringstream;
using std::ctype;
using std::use_facet;

using namespace bit7z;

string bit7z::narrow( const wchar_t* wideString, size_t size ) {
    ostringstream stream;
    stream.imbue( std::locale( "C" ) );
    const auto& ctfacet = use_facet< ctype< char > >( stream.getloc() ) ;
    for ( size_t i = 0 ; i < size; ++i ) {
        stream << ctfacet.narrow( wideString[ i ], 0 );
    }
    return stream.str();
}

wstring bit7z::widen( const string& narrowString ) {
    wostringstream stream;
    stream.imbue( std::locale( "" ) );
    const auto& ctfacet = use_facet< ctype< wchar_t > >( stream.getloc() ) ;
    for ( char c : narrowString ) {
        stream << ctfacet.widen( c ) ;
    }
    return stream.str() ;
}
#endif
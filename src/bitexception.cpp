// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#include "../include/bitexception.hpp"

using std::string;
using namespace bit7z;

std::string ws2s( const std::wstring& wstr ) {
    int num_chars = WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), static_cast< int >( wstr.length() ), nullptr, 0, nullptr, nullptr );
    std::string result;
    if ( num_chars > 0 ) {
        result.resize( static_cast< size_t >( num_chars ) );
        WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), static_cast< int >( wstr.length() ), &result[0], num_chars, nullptr, nullptr );
    }
    return result;
}

BitException::BitException( const char* const message, HRESULT code ) : runtime_error( message ), mErrorCode( code ) {}

BitException::BitException( const char* const message, DWORD code )
    : runtime_error( message ), mErrorCode( HRESULT_FROM_WIN32( code ) ) {}

BitException::BitException( const wstring& message, HRESULT code )
    : runtime_error( ws2s( message ) ), mErrorCode( code ) {}

BitException::BitException( const wstring& message, DWORD code )
    : runtime_error( ws2s( message ) ), mErrorCode( HRESULT_FROM_WIN32( code ) ) {}

HRESULT BitException::getErrorCode() const {
    return mErrorCode;
}

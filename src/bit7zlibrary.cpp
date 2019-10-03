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

#include "../include/bit7zlibrary.hpp"

#include "../include/bitexception.hpp"

using namespace bit7z;

Bit7zLibrary::Bit7zLibrary( const std::wstring& dll_path ) : mLibrary( LoadLibrary( dll_path.c_str() ) ) {
    if ( !mLibrary ) {
        throw BitException( L"Cannot load 7-zip library (error " + std::to_wstring( GetLastError() ) + L")", GetLastError() );
    }

    mCreateObjectFunc = reinterpret_cast< CreateObjectFunc >( GetProcAddress( mLibrary, "CreateObject" ) );

    if ( !mCreateObjectFunc ) {
        FreeLibrary( mLibrary );
        throw BitException( L"Cannot get CreateObject (error " + std::to_wstring( GetLastError() ) + L")", GetLastError() );
    }
}

Bit7zLibrary::~Bit7zLibrary() {
    FreeLibrary( mLibrary );
}

void Bit7zLibrary::createArchiveObject( const GUID* format_ID, const GUID* interface_ID, void** out_object ) const {
    HRESULT res = mCreateObjectFunc( format_ID, interface_ID, out_object );
    if ( res != S_OK ) {
        throw BitException( "Cannot get class object", res );
    }
}

void Bit7zLibrary::setLargePageMode() {
    auto pSetLargePageMode = reinterpret_cast< SetLargePageMode >( GetProcAddress( mLibrary, "SetLargePageMode") );
    if ( !pSetLargePageMode ) {
        throw BitException( "Cannot set large page mode", GetLastError() );
    }
    pSetLargePageMode();
}

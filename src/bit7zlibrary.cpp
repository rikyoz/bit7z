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

#include "bit7zlibrary.hpp"

#include "bitexception.hpp"
#include "internal/windows.hpp"
#include "internal/util.hpp"

#ifdef _WIN32
#   define Bit7zLoadLibrary(lib_name) LoadLibraryW( WIDEN( (library_path) ).c_str() )
#   define ERROR_CODE( errc ) bit7z::last_error_code()
#else
#   include <dlfcn.h>

#   define Bit7zLoadLibrary( lib_name ) dlopen( (lib_name).c_str(), RTLD_LAZY )
#   define GetProcAddress dlsym
#   define FreeLibrary dlclose
#   define ERROR_CODE( errc ) std::make_error_code( errc )  //same behavior as boost::shared_library
#endif

using namespace bit7z;

Bit7zLibrary::Bit7zLibrary( const tstring& library_path ) : mLibrary( Bit7zLoadLibrary( library_path ) ) {
    if ( mLibrary == nullptr ) {
        throw BitException( "Failed to load 7-zip library", ERROR_CODE( std::errc::bad_file_descriptor ) );
    }

    mCreateObjectFunc = reinterpret_cast< CreateObjectFunc >( GetProcAddress( mLibrary, "CreateObject" ) );

    if ( mCreateObjectFunc == nullptr ) {
        FreeLibrary( mLibrary );
        throw BitException( "Failed to get CreateObject function", ERROR_CODE( std::errc::invalid_seek ) );
    }
}

Bit7zLibrary::~Bit7zLibrary() {
    FreeLibrary( mLibrary );
}

void Bit7zLibrary::createArchiveObject( const GUID* format_ID, const GUID* interface_ID, void** out_object ) const {
    const HRESULT res = mCreateObjectFunc( format_ID, interface_ID, out_object );
    if ( res != S_OK ) {
        throw BitException( "Failed to get class object", make_hresult_code( res ) );
    }
}

void Bit7zLibrary::setLargePageMode() {
    using SetLargePageMode = HRESULT ( WINAPI* )();

    auto pSetLargePageMode = reinterpret_cast< SetLargePageMode >( GetProcAddress( mLibrary, "SetLargePageMode" ) );
    if ( pSetLargePageMode == nullptr ) {
        throw BitException( "Failed to get SetLargePageMode function", ERROR_CODE( std::errc::invalid_seek ) );
    }
    const HRESULT res = pSetLargePageMode();
    if ( res != S_OK ) {
        throw BitException( "Failed to set large page mode", make_hresult_code( res ) );
    }
}

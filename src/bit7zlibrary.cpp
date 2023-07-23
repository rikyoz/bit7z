// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bit7zlibrary.hpp"
#include "bitexception.hpp"
#include "internal/guids.hpp"
#include "internal/util.hpp"
#include "internal/windows.hpp"

#ifdef _WIN32
#   define Bit7zLoadLibrary( lib_name ) LoadLibraryW( WIDEN( (lib_name) ).c_str() )
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
        throw BitException( "Failed to load the 7-zip library", ERROR_CODE( std::errc::bad_file_descriptor ) );
    }

    mCreateObjectFunc = GetProcAddress( mLibrary, "CreateObject" );

    if ( mCreateObjectFunc == nullptr ) {
        FreeLibrary( mLibrary );
        throw BitException( "Failed to get CreateObject function", ERROR_CODE( std::errc::invalid_seek ) );
    }
}

Bit7zLibrary::~Bit7zLibrary() {
    FreeLibrary( mLibrary );
}

void Bit7zLibrary::setLargePageMode() {
    using SetLargePageMode = HRESULT ( WINAPI* )();

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto pSetLargePageMode = reinterpret_cast< SetLargePageMode >( GetProcAddress( mLibrary, "SetLargePageMode" ) );
    if ( pSetLargePageMode == nullptr ) {
        throw BitException( "Failed to get SetLargePageMode function", ERROR_CODE( std::errc::invalid_seek ) );
    }
    const HRESULT res = pSetLargePageMode();
    if ( res != S_OK ) {
        throw BitException( "Failed to set the large page mode", make_hresult_code( res ) );
    }
}

using CreateObjectFunc = HRESULT ( WINAPI* )( const GUID* clsID, const GUID* interfaceID, void** out );

auto Bit7zLibrary::initInArchive( const BitInFormat& format ) const -> IInArchive* {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto createObject = reinterpret_cast< CreateObjectFunc >( mCreateObjectFunc );
    const auto format_ID = formatGUID( format );
    IInArchive* in_archive = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const HRESULT res = createObject( &format_ID, &::IID_IInArchive, reinterpret_cast< void** >( &in_archive ) );
    if ( res != S_OK || in_archive == nullptr ) {
        throw BitException( "Failed to initialize the input archive object", make_hresult_code( res ) );
    }
    return in_archive;
}

auto Bit7zLibrary::initOutArchive( const BitInFormat& format ) const -> IOutArchive* {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto createObject = reinterpret_cast< CreateObjectFunc >( mCreateObjectFunc );
    const auto format_ID = formatGUID( format );
    IOutArchive* out_archive = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const HRESULT res = createObject( &format_ID, &::IID_IOutArchive, reinterpret_cast< void** >( &out_archive ) );
    if ( res != S_OK || out_archive == nullptr ) {
        throw BitException( "Failed to initialize the output archive object", make_hresult_code( res ) );
    }
    return out_archive;
}

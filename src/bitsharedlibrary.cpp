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

#include "bitsharedlibrary.hpp"

#include "bitexception.hpp"
#include "bittypes.hpp"

#ifdef _WIN32
#   include "internal/stringutil.hpp"
#   define ERROR_CODE( errc ) bit7z::last_error_code()
#else
#   include <dlfcn.h>
#   define ERROR_CODE( errc ) std::make_error_code( errc )  //same behavior as boost::shared_library
#endif

namespace bit7z {

inline auto load_library( const tstring& libraryPath ) -> LibraryHandle {
#ifdef _WIN32
    LibraryHandle handle = LoadLibraryW( WIDEN( libraryPath ).c_str() );
#else
    LibraryHandle handle = dlopen( libraryPath.c_str(), RTLD_LAZY );
#endif
    if ( handle == nullptr ) {
        // Note: MSVC 2015 doesn't correctly get the last error
        // when inlining the error variable in the BitException constructor call,
        // so we need to store the error in a separate variable. ¯\_(ツ)_/¯
        const auto error = ERROR_CODE( std::errc::bad_file_descriptor );
        throw BitException( "Failed to load the library", error );
    }
    return handle;
}

BitSharedLibrary::BitSharedLibrary( const tstring& libraryPath ) : mLibrary{ load_library( libraryPath ) } {}

BitSharedLibrary::~BitSharedLibrary() {
#ifdef _WIN32
    FreeLibrary( mLibrary );
#else
    dlclose( mLibrary );
#endif
}

auto BitSharedLibrary::getSymbol( const char* symbolName ) -> LibrarySymbol {
#ifdef _WIN32
    LibrarySymbol symbol = GetProcAddress( mLibrary, symbolName );
#else
    LibrarySymbol symbol = dlsym( mLibrary, symbolName );
#endif
    if ( symbol  == nullptr ) {
        throw BitException( "Failed to get the function symbol", ERROR_CODE( std::errc::invalid_seek ) );
    }
    return symbol;
}

} // namespace bit7z
// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2026 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bit7zlibraryloader.hpp"

#include "biterror.hpp"
#include "bitexception.hpp"

namespace bit7z {

Bit7zLibraryLoader::Bit7zLibraryLoader() noexcept : mEmpty{}, mLoaded{ false } {}

Bit7zLibraryLoader::Bit7zLibraryLoader( const tstring& libraryPath ) : Bit7zLibraryLoader{} {
    load( libraryPath );
}

Bit7zLibraryLoader::~Bit7zLibraryLoader() {
    unload();
}

void Bit7zLibraryLoader::load( const tstring& libraryPath ) {
    unload();
    new ( &mLibrary ) Bit7zLibrary{ libraryPath }; // NOLINT(*-pro-type-union-access)
    mLoaded = true;
}

void Bit7zLibraryLoader::load( const tstring& libraryPath, std::error_code& ec ) noexcept try {
    load( libraryPath );
    ec.clear();
} catch ( const std::system_error& ex ) {
    ec = ex.code();
} catch (...) {
    ec = make_error_code( BitError::Fail );
}

auto Bit7zLibraryLoader::library() const -> const Bit7zLibrary& {
    if ( !mLoaded ) {
        throw BitException{ "Library not loaded", make_error_code( BitError::UnsupportedOperation ) };
    }
    return mLibrary; // NOLINT(*-pro-type-union-access)
}

void Bit7zLibraryLoader::unload() noexcept {
    if ( mLoaded ) {
        mLibrary.~Bit7zLibrary(); // NOLINT(*-pro-type-union-access)
        mLoaded = false;
    }
}

auto Bit7zLibraryLoader::isLoaded() const noexcept -> bool {
    return mLoaded;
}

auto Bit7zLibraryLoader::operator->() const -> const Bit7zLibrary* {
    return &library();
}

Bit7zLibraryLoader::operator const Bit7zLibrary&() const {
    return library();
}

} // namespace bit7z

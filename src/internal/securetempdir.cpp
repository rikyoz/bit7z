// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _WIN32

#include "internal/securetempdir.hpp"

#include "bitexception.hpp"
#include "internal/stringutil.hpp"

// For mkdtemp
#include <unistd.h> // IWYU pragma: keep
#include <cstdlib>

#include <string>
#include <system_error>
#include <utility>

namespace bit7z {

SecureTempDir::SecureTempDir( const fs::path& nearPath ) {
    std::string filenameTemplate = nearPath.native() + ".tmp.XXXXXX";
    if ( mkdtemp( &filenameTemplate[ 0 ] ) == nullptr ) {
        const auto error = last_error_code();
        throw BitException( "Failed to create temporary directory",
                            error,
                            error == std::errc::no_such_file_or_directory
                                ? path_to_tstring( nearPath.parent_path() )
                                : path_to_tstring( nearPath ) );

    }
    mDirectory = std::move( filenameTemplate );
}

SecureTempDir::~SecureTempDir() {
    if ( !mDirectory.empty() ) {
        std::error_code ignored;
        fs::remove_all( mDirectory, ignored );
    }
}

SecureTempDir::SecureTempDir( SecureTempDir&& other ) noexcept
    : mDirectory{ std::move( other.mDirectory ) } {
    other.mDirectory.clear();
}

SecureTempDir& SecureTempDir::operator=( SecureTempDir&& other ) noexcept {
    if ( this != &other ) {
        if ( !mDirectory.empty() ) {
            std::error_code ignored;
            fs::remove_all( mDirectory, ignored );
        }
        mDirectory = std::move( other.mDirectory );
        other.mDirectory.clear();
    }
    return *this;
}

} // namespace bit7z

#endif // !_WIN32
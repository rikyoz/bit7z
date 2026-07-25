// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/callback.hpp"

#include <exception>

namespace bit7z {

void Callback::rethrowStoredException() const {
    if ( mErrorException ) {
        std::rethrow_exception( mErrorException );
    }
}

Callback::Callback( const BitAbstractArchiveHandler& handler ) : mHandler( handler ) {}

void Callback::setErrorException( const char* message, std::error_code code ) noexcept {
    mErrorException = std::make_exception_ptr( BitException( message, code ) );
}

void Callback::setErrorException( const BitException& exception ) noexcept {
    mErrorException = std::make_exception_ptr( exception );
}

void Callback::setErrorException( const std::exception_ptr& exception ) noexcept {
    mErrorException = exception;
}

} // namespace bit7z

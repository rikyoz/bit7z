/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef EXCEPTIONUTIL_HPP
#define EXCEPTIONUTIL_HPP

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bitwindows.hpp"

#include <exception>
#include <string>

namespace bit7z {

// Wraps a caught std::exception into a BitException, prefixing its message with context and
// falling back to a generic error code (a plain std::exception carries none).
// Don't use this for a std::system_error: construct BitException( context, error.code() )
// directly instead, to preserve its code and avoid duplicating its already-suffixed what().
inline auto toBitException( const std::string& context, const std::exception& error ) -> BitException {
    return BitException( context + ": " + error.what(), make_hresult_code( E_ABORT ) );
}

// Throws BitError::NullCallback if callback is empty. Callback is a template rather than a fixed
// type since callers pass different std::function specializations (ItemBufferCallback,
// BufferCallback, RawDataCallback, ...); all that's required is a bool-testable callable.
template< typename Callback >
void requireCallbackForExtraction( const Callback& callback ) {
    if ( !callback ) {
        throw BitException(
            "Cannot extract the archive using an empty callback",
            make_error_code( BitError::NullCallback )
        );
    }
}

} // namespace bit7z

#endif // EXCEPTIONUTIL_HPP
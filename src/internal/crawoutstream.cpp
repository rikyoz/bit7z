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

#include <utility>

#include "internal/crawoutstream.hpp"

namespace bit7z {

CRawOutStream::CRawOutStream( RawDataCallback callback ) : mOutputCallback{ std::move( callback ) } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CRawOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 ) {
        return S_OK;
    }

    bool callbackResult = mOutputCallback( static_cast< const byte_t* >( data ), size );

    if ( processedSize != nullptr ) {
        *processedSize = size;
    }

    return callbackResult ? S_OK : E_ABORT;
}

} // namespace bit7z
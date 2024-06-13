// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/csynchronizedoutstream.hpp"
#include "internal/extractcallback.hpp"
#include "util.hpp"

namespace bit7z {

CSynchronizedOutStream::CSynchronizedOutStream( BufferQueue& queue ) : mBufferQueue{ queue } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CSynchronizedOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( data == nullptr || size == 0 ) {
        return E_FAIL;
    }

    // size is uin32_t, but std::next expects a signed integer of at least 32 bits (on x64, 64 bits),
    // so if size is greater than the std::numeric_limits<std::ptrdiff_t>::max(), it is invalid.
    if ( cmp_greater( size, std::numeric_limits< buffer_t::difference_type >::max() ) ) {
        return E_FAIL;
    }

    const auto data_size = clamp_cast< buffer_t::difference_type >( size );
    const auto* data_start = static_cast< const byte_t* >( data ); //-V2571
    const auto* data_end = std::next( data_start, data_size );

    try {
        buffer_t extractedData( data_start, data_end );
        mBufferQueue.push( std::move( extractedData ) );
    } catch (...) {
        return E_OUTOFMEMORY;
    }

    if ( processedSize != nullptr ) {
        *processedSize = size;
    }
    return S_OK;
}

} // namespace bit7z
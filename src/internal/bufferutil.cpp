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

#include "internal/bufferutil.hpp"
#include "internal/windows.hpp"

auto bit7z::seek( const buffer_t& buffer,
                  const buffer_t::const_iterator& currentPosition,
                  int64_t offset,
                  uint32_t seekOrigin,
                  int64_t& newPosition ) -> HRESULT {
    int64_t currentIndex; // NOLINT(cppcoreguidelines-init-variables)
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET: {
            currentIndex = 0;
            break;
        }
        case STREAM_SEEK_CUR: {
            currentIndex = ( currentPosition - buffer.cbegin() );
            break;
        }
        case STREAM_SEEK_END: {
            currentIndex = ( buffer.cend() - buffer.cbegin() );
            break;
        }
        default:
            return STG_E_INVALIDFUNCTION;
    }

    // Checking if the sum between currentIndex and offset would result in an integer overflow or underflow.
    if ( check_overflow( currentIndex, offset ) ) {
        return E_INVALIDARG;
    }

    const int64_t newIndex = currentIndex + offset;

    // Making sure the newIndex value is between 0 and mBuffer.size()
    if ( newIndex < 0 ) {
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }

    /* Note: a std::vector's max size can be at most std::numeric_limits< std::ptrdiff_t >::max()
     *       (see https://en.cppreference.com/w/cpp/container/vector/max_size).
     *       Since index_t is just an alias for std::ptrdiff_t, the following cast is safe. */
    if ( newIndex > static_cast< index_t >( buffer.size() ) ) {
        return E_INVALIDARG;
    }

    newPosition = newIndex;
    return S_OK;
}
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

#include "internal/bufferutil.hpp"
#include "internal/windows.hpp"

HRESULT bit7z::seek( const buffer_t& buffer,
                     const buffer_t::const_iterator& current_position,
                     int64_t offset,
                     uint32_t seek_origin,
                     int64_t& new_position ) {
    int64_t current_index; // NOLINT(cppcoreguidelines-init-variables)
    switch ( seek_origin ) {
        case STREAM_SEEK_SET: {
            current_index = 0;
            break;
        }
        case STREAM_SEEK_CUR: {
            current_index = ( current_position - buffer.cbegin() );
            break;
        }
        case STREAM_SEEK_END: {
            current_index = ( buffer.cend() - buffer.cbegin() );
            break;
        }
        default:
            return STG_E_INVALIDFUNCTION;
    }

    // Checking if the sum between current_index and offset would result in an integer overflow or underflow.
    if ( check_overflow( current_index, offset ) ) {
        return E_INVALIDARG;
    }

    int64_t new_index = current_index + offset;

    // Making sure the new_index value is between 0 and mBuffer.size()
    if ( new_index < 0 ) {
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }

    /* Note: a std::vector's max size can be at most std::numeric_limits< std::ptrdiff_t >::max()
     *       (see https://en.cppreference.com/w/cpp/container/vector/max_size).
     *       Since index_t is just an alias for std::ptrdiff_t, the following cast is safe. */
    if ( new_index > static_cast< index_t >( buffer.size() ) ) {
        return E_INVALIDARG;
    }

    new_position = new_index;
    return S_OK;
}
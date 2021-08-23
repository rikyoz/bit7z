// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */
#ifdef _MSC_VER
// Disable warning
//    C4996: '...': Function call with parameters that may be unsafe
// This is due to the call to std::copy_n with a raw buffer pointer as destination.
#pragma warning(disable:4996)
#endif

#include "internal/cbufferinstream.hpp"

#include <algorithm> //for std::copy_n
#include <cstdint>

#ifndef _WIN32
#include <myWindows/StdAfx.h>
#endif

using namespace bit7z;

CBufferInStream::CBufferInStream( const vector< byte_t >& in_buffer )
    : mBuffer( in_buffer ), mCurrentPosition{ mBuffer.begin() } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferInStream::Read( void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 || mCurrentPosition == mBuffer.cend() ) {
        return S_OK;
    }

    /* Note: thanks to CBufferInStream::Seek, we can safely assume mCurrentPosition to always be a valid iterator;
     * so "remaining" will always be > 0 (and casts to unsigned types are safe) */
    size_t remaining = mBuffer.cend() - mCurrentPosition;
    if ( remaining > static_cast< size_t >( size ) ) {
        /* Remaining buffer still to read is bigger than the buffer size requested by the user,
         * so we need to read just "size" number of bytes */
        remaining = static_cast< size_t >( size );
    }
    /* else, the user requested to read a number of bytes greater than or equal to the number
     * of remaining bytes to be read from the buffer.
     * So we just read all the remaining bytes, not more or less. */

    /* Note: here remaining is > 0 */
    std::copy_n( mCurrentPosition, remaining, static_cast< byte_t* >( data ) );
    std::advance( mCurrentPosition, remaining );

    if ( processedSize != nullptr ) {
        /* Note: even though on 64-bit systems "remaining" will be a 64-bit unsigned integer (size_t),
         * its value cannot be greater than "size", which is a 32-bit unsigned int. Hence, this cast is safe! */
        *processedSize = static_cast< UInt32 >( remaining );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferInStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    int64_t current_index;
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET: {
            current_index = 0;
            break;
        }
        case STREAM_SEEK_CUR: {
            current_index = ( mCurrentPosition - mBuffer.cbegin() );
            break;
        }
        case STREAM_SEEK_END: {
            current_index = ( mBuffer.cend() - mBuffer.cbegin() );
            break;
        }
        default:
            return STG_E_INVALIDFUNCTION;
    }

    // Checking if the sum between current_index and offset would result in an integer overflow or underflow
    if ( check_overflow( current_index, offset ) ) {
        return E_INVALIDARG;
    }

    int64_t new_index = current_index + offset;

    // Making sure that the new_index value is between 0 and mBuffer.size()
    if ( new_index < 0 ) {
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }

    /* Note: a std::vector's max size can be at most std::numeric_limits< std::ptrdiff_t >::max()
     *       (see https://en.cppreference.com/w/cpp/container/vector/max_size).
     *       Since index_t is just an alias for std::ptrdiff_t, the following cast is safe. */
    if ( new_index > static_cast< index_t >( mBuffer.size() ) ) {
        return E_INVALIDARG;
    }

    // Note: new_index can be equal to mBuffer.size(); in this case, mCurrentPosition == mBuffer.cend()
    mCurrentPosition = mBuffer.cbegin() + static_cast< index_t >( new_index );

    if ( newPosition != nullptr ) {
        // Safe cast, since new_index > 0
        *newPosition = static_cast< UInt64 >( new_index );
    }

    return S_OK;
}

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#include "../include/cbufferoutstream.hpp"

#include <cstdint>

using namespace bit7z;

CBufferOutStream::CBufferOutStream( vector< byte_t >& out_buffer ) : mBuffer( out_buffer ), mCurrentPosition( 0 ) {}

STDMETHODIMP CBufferOutStream::SetSize( UInt64 newSize ) {
    mBuffer.resize( static_cast< vector< byte_t >::size_type >( newSize ) );
    return S_OK;
}

STDMETHODIMP CBufferOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) {
    int64_t new_pos;

    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            new_pos = offset;
            break;
        case STREAM_SEEK_CUR: {
            new_pos = static_cast< int64_t >( mCurrentPosition ) + offset;
            break;
        }
        case STREAM_SEEK_END: {
            new_pos = static_cast< int64_t >( mBuffer.size() ) + offset;
            break;
        }
        default:
            return STG_E_INVALIDFUNCTION;
    }

    if ( new_pos < 0 ) {
        return STG_E_INVALIDFUNCTION;
    }

    mCurrentPosition = static_cast< size_t >( new_pos );

    if ( newPosition != nullptr ) {
        *newPosition = mCurrentPosition;
    }

    return S_OK;
}

STDMETHODIMP CBufferOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( data == nullptr || size == 0 ) {
        return E_FAIL;
    }

    size_t new_pos = mCurrentPosition + static_cast< size_t >( size );
    if ( new_pos > mBuffer.size() ) {
        mBuffer.resize( new_pos );
    }

    const auto* byte_data = static_cast< const byte_t* >( data );
    std::copy_n( byte_data, size, mBuffer.begin() + mCurrentPosition );
    mCurrentPosition = new_pos;

    if ( processedSize != nullptr ) {
        *processedSize = size;
    }

    return S_OK;
}

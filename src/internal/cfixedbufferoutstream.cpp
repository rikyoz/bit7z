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

#include "internal/cfixedbufferoutstream.hpp"

#include <cstdint>

#ifndef _WIN32
#include <myWindows/StdAfx.h>
#endif

using namespace bit7z;

CFixedBufferOutStream::CFixedBufferOutStream( byte_t* buffer, std::size_t size )
: mBuffer( buffer ), mBufferSize(size ), mCurrentPosition( 0 ) {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::SetSize( UInt64 newSize ) {
    return newSize != mBufferSize ? E_INVALIDARG : S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    index_t current_index;
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET: {
            current_index = 0;
            break;
        }
        case STREAM_SEEK_CUR: {
            current_index = mCurrentPosition;
            break;
        }
        case STREAM_SEEK_END: {
            current_index = static_cast< index_t >( mBufferSize - 1 );
            break;
        }
        default:
            return STG_E_INVALIDFUNCTION;
    }

    // Checking if the sum between current_index and offset would result in an integer overflow or underflow
    if ( check_overflow( current_index, offset ) ) {
        return E_INVALIDARG;
    }

    index_t new_index = current_index + offset;

    // Making sure that the new_index value is between 0 and mBufferSize - 1
    if ( new_index < 0 ) {
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }

    if ( new_index >= static_cast< index_t >( mBufferSize ) ) {
        return E_INVALIDARG;
    }

    mCurrentPosition = static_cast< index_t >( new_index );

    if ( newPosition != nullptr ) {
        *newPosition = static_cast< UInt64 >( new_index );
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( data == nullptr || size == 0 ) {
        return E_FAIL;
    }

    size_t write_size = size;
    const size_t new_pos = mCurrentPosition + size;
    if ( new_pos >= mBufferSize ) {
        write_size = mBufferSize - mCurrentPosition;
    }

    const auto* byte_data = static_cast< const byte_t* >( data );
    try {
        std::copy_n( byte_data, write_size, &mBuffer[mCurrentPosition] ); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }

    mCurrentPosition += static_cast< index_t >( write_size );

    if ( processedSize != nullptr ) {
        *processedSize = static_cast< UInt32 >( write_size );
    }

    return S_OK;
}

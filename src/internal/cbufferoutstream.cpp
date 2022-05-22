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

#include "internal/cbufferoutstream.hpp"
#include "internal/bufferutil.hpp"

#include <cstdint>
#include <algorithm> //for std::copy_n

using namespace bit7z;

CBufferOutStream::CBufferOutStream( vector< byte_t >& out_buffer )
: mBuffer( out_buffer ), mCurrentPosition{ mBuffer.begin() } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferOutStream::SetSize( UInt64 newSize ) {
    try {
        mBuffer.resize( static_cast< vector< byte_t >::size_type >( newSize ) );
        return S_OK;
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    int64_t new_index{};
    HRESULT res = seek( mBuffer, mCurrentPosition, offset, seekOrigin, new_index );

    if ( res != S_OK ) {
        // new_index is not in the range [0, mBuffer.size]
        return res;
    }

    // Note: new_index can be equal to mBuffer.size(); in this case, mCurrentPosition == mBuffer.cend()
    mCurrentPosition = mBuffer.begin() + static_cast< index_t >( new_index );

    if ( newPosition != nullptr ) {
        // Safe cast, since new_index >=0 0
        *newPosition = static_cast< UInt64 >( new_index );
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( data == nullptr || size == 0 ) {
        return E_FAIL;
    }

    auto old_pos = ( mCurrentPosition - mBuffer.begin() );
    const size_t new_pos = old_pos + size;
    if ( new_pos > mBuffer.size() ) {
        try {
            mBuffer.resize( new_pos );
        } catch ( ... ) {
            return E_OUTOFMEMORY;
        }
        mCurrentPosition = mBuffer.begin() + old_pos; //resize invalidated the old mCurrentPosition iterator
    }

    const auto* byte_data = static_cast< const byte_t* >( data );
    try {
        std::copy_n( byte_data, size, mCurrentPosition );
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }

    std::advance( mCurrentPosition, size );

    if ( processedSize != nullptr ) {
        *processedSize = size;
    }

    return S_OK;
}

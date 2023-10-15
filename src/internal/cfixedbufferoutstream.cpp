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

#include <algorithm> //for std::copy_n

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/cfixedbufferoutstream.hpp"
#include "internal/util.hpp"

namespace bit7z {

CFixedBufferOutStream::CFixedBufferOutStream( byte_t* buffer, std::size_t size )
    : mBuffer( buffer ), mBufferSize( size ), mCurrentPosition( 0 ) {
    if ( size == 0 || cmp_greater( size, ( std::numeric_limits< int64_t >::max )() ) ) {
        throw BitException( "Could not initialize output buffer stream",
                            make_error_code( BitError::InvalidOutputBufferSize ) );
    }
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::SetSize( UInt64 newSize ) noexcept {
    return newSize != mBufferSize ? E_INVALIDARG : S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    int64_t currentIndex{};
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET: {
            break;
        }
        case STREAM_SEEK_CUR: {
            currentIndex = mCurrentPosition;
            break;
        }
        case STREAM_SEEK_END: {
            currentIndex = static_cast< int64_t >( mBufferSize );
            break;
        }
        default:
            return STG_E_INVALIDFUNCTION;
    }

    // Checking if the sum between the currentIndex and offset would result in an integer overflow or underflow
    if ( check_overflow( currentIndex, offset ) ) {
        return E_INVALIDARG;
    }

    const int64_t newIndex = currentIndex + offset;

    // Making sure the newIndex value is between 0 and mBufferSize - 1
    if ( newIndex < 0 ) {
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }

    if ( cmp_greater_equal( newIndex, mBufferSize ) ) {
        return E_INVALIDARG;
    }

    mCurrentPosition = newIndex;

    if ( newPosition != nullptr ) {
        *newPosition = newIndex;
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( data == nullptr || size == 0 ) {
        return E_FAIL;
    }

    uint32_t writeSize = size;
    if ( cmp_greater_equal( size, mBufferSize - mCurrentPosition ) ) {
        /* Writing only to the remaining part of the output buffer!
         * Note: since size is an uint32_t, and size >= mBufferSize - mCurrentPosition, the cast is safe! */
        writeSize = static_cast< uint32_t >( mBufferSize - mCurrentPosition );
    }

    const auto* byteData = static_cast< const byte_t* >( data ); //-V2571
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::copy_n( byteData, writeSize, &mBuffer[ mCurrentPosition ] ); //-V2563
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }

    mCurrentPosition += writeSize;

    if ( processedSize != nullptr ) {
        *processedSize = writeSize;
    }

    return S_OK;
}

} // namespace bit7z
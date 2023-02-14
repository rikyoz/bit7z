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

#include "internal/cfixedbufferoutstream.hpp"

#include <algorithm> //for std::copy_n

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/util.hpp"

using namespace bit7z;

/* Safe integer comparison like in C++20 */
#ifdef __cpp_if_constexpr

template< class T, class U >
constexpr auto cmp_less( T t, U u ) noexcept -> bool {
    using UT = std::make_unsigned_t< T >;
    using UU = std::make_unsigned_t< U >;
    if constexpr ( std::is_signed< T >::value == std::is_signed< U >::value ) {
        return t < u;
    } else if constexpr ( std::is_signed< T >::value ) {
        return t < 0 || UT( t ) < u;
    } else {
        return u >= 0 && t < UU( u );
    }
}

#else // SFINAE implementation for C++14

template< class T, class U >
constexpr std::enable_if_t< std::is_signed< T >::value == std::is_signed< U >::value, bool >
cmp_less( T t, U u ) noexcept {
    return t < u;
}

template< class T, class U >
constexpr std::enable_if_t< std::is_signed< T >::value && !std::is_signed< U >::value, bool >
cmp_less( T t, U u ) noexcept {
    using UT = std::make_unsigned_t< T >;
    return t < 0 || UT( t ) < u;
}

template< class T, class U >
constexpr std::enable_if_t< !std::is_signed< T >::value && std::is_signed< U >::value, bool >
cmp_less( T t, U u ) noexcept {
    using UU = std::make_unsigned_t< U >;
    return u >= 0 && t < UU( u );
}

#endif

template< class T, class U >
constexpr auto cmp_greater( T t, U u ) noexcept -> bool {
    return cmp_less( u, t );
}

template< class T, class U >
constexpr auto cmp_greater_equal( T t, U u ) noexcept -> bool {
    return !cmp_less( t, u );
}

CFixedBufferOutStream::CFixedBufferOutStream( byte_t* buffer, std::size_t size )
    : mBuffer( buffer ), mBufferSize( size ), mCurrentPosition( 0 ) {
    if ( size == 0 || cmp_greater( size, ( std::numeric_limits< int64_t >::max )() ) ) {
        throw BitException( "Could not initialize output buffer stream",
                            make_error_code( BitError::InvalidOutputBufferSize ) );
    }
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::SetSize( UInt64 newSize ) {
    return newSize != mBufferSize ? E_INVALIDARG : S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    int64_t current_index{};
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET: {
            break;
        }
        case STREAM_SEEK_CUR: {
            current_index = mCurrentPosition;
            break;
        }
        case STREAM_SEEK_END: {
            current_index = static_cast< int64_t >( mBufferSize );
            break;
        }
        default:
            return STG_E_INVALIDFUNCTION;
    }

    // Checking if the sum between the current_index and offset would result in an integer overflow or underflow
    if ( check_overflow( current_index, offset ) ) {
        return E_INVALIDARG;
    }

    const int64_t new_index = current_index + offset;

    // Making sure the new_index value is between 0 and mBufferSize - 1
    if ( new_index < 0 ) {
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }

    if ( cmp_greater_equal( new_index, mBufferSize ) ) {
        return E_INVALIDARG;
    }

    mCurrentPosition = new_index;

    if ( newPosition != nullptr ) {
        *newPosition = new_index;
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

    uint32_t write_size = size;
    if ( cmp_greater_equal( size, mBufferSize - mCurrentPosition ) ) {
        /* Writing only to the remaining part of the output buffer!
         * Note: since size is an uint32_t, and size >= mBufferSize - mCurrentPosition, the cast is safe! */
        write_size = static_cast< uint32_t >( mBufferSize - mCurrentPosition );
    }

    const auto* byte_data = static_cast< const byte_t* >( data );
    try {
        std::copy_n( byte_data, write_size,
                     &mBuffer[ mCurrentPosition ] ); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }

    mCurrentPosition += write_size;

    if ( processedSize != nullptr ) {
        *processedSize = write_size;
    }

    return S_OK;
}

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2020  Riccardo Ostani - All Rights Reserved.
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

#include "../include/cbufferinstream.hpp"

#include <algorithm>
#include <cstdint>

#ifndef _WIN32
#include <myWindows/StdAfx.h>
#endif

using namespace bit7z;

CBufferInStream::CBufferInStream( const vector< byte_t >& in_buffer ) : mBuffer( in_buffer ), mCurrentPosition( 0 ) {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferInStream::Read( void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 || mCurrentPosition >= mBuffer.size() ) {
        return S_OK;
    }

    size_t remaining = mBuffer.size() - mCurrentPosition;
    if ( remaining > size ) { //-V104
        remaining = static_cast< size_t >( size );
    }

    std::copy_n( mBuffer.begin() + mCurrentPosition, remaining, static_cast< byte_t* >( data ) );

    mCurrentPosition += remaining;

    if ( processedSize != nullptr ) {
        *processedSize = static_cast< UInt32 >( remaining );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferInStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) {
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
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }

    mCurrentPosition = static_cast< size_t >( new_pos );

    if ( newPosition != nullptr ) {
        *newPosition = mCurrentPosition;
    }

    return S_OK;
}

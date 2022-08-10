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

#include "internal/cstdinstream.hpp"

#include "internal/streamutil.hpp"

using namespace bit7z;

CStdInStream::CStdInStream( istream& inputStream ) : mInputStream( inputStream ) {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CStdInStream::Read( void* data, UInt32 size, UInt32* processedSize ) {
    mInputStream.clear();

    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 ) {
        return S_OK;
    }

    mInputStream.read( static_cast< char* >( data ), size );

    if ( processedSize != nullptr ) {
        *processedSize = static_cast< uint32_t >( mInputStream.gcount() );
    }

    return mInputStream.bad() ? HRESULT_FROM_WIN32( ERROR_READ_FAULT ) : S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CStdInStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) {
    mInputStream.clear();

    std::ios_base::seekdir way; // NOLINT(cppcoreguidelines-init-variables)
    RINOK( to_seekdir( seekOrigin, way ) );

    /*if ( offset < 0 ) { // GZip uses negative offsets!
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }*/

    mInputStream.seekg( static_cast< std::istream::off_type >( offset ), way );

    if ( mInputStream.bad() ) {
        return HRESULT_FROM_WIN32( ERROR_SEEK );
    }

    if ( newPosition != nullptr ) {
        *newPosition = static_cast< uint64_t >( mInputStream.tellg() );
    }

    return S_OK;
}

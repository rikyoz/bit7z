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

#include "../include/cstdinstream.hpp"

using namespace bit7z;

CStdInStream::CStdInStream( istream& inputStream ) : mInputStream( inputStream ) {}

CStdInStream::~CStdInStream() {}

STDMETHODIMP CStdInStream::Read( void* data, uint32_t size, uint32_t* processedSize ) {
    mInputStream.clear();

    if ( processedSize ) {
        *processedSize = 0;
    }

    if ( size == 0 ) {
        return S_OK;
    }

    mInputStream.read( static_cast< char* >( data ), size );

    if ( processedSize ) {
        *processedSize = static_cast< uint32_t >( mInputStream.gcount() );
    }

    return mInputStream.bad() ? HRESULT_FROM_WIN32( ERROR_READ_FAULT ) : S_OK;
}

STDMETHODIMP CStdInStream::Seek( int64_t offset, uint32_t seekOrigin, uint64_t* newPosition ) {
    mInputStream.clear();

    std::ios_base::seekdir way;
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            way = std::ios_base::beg;
            break;
        case STREAM_SEEK_CUR:
            way = std::ios_base::cur;
            break;
        case STREAM_SEEK_END:
            way = std::ios_base::end;
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }

    /*if ( offset < 0 ) { // GZip uses negative offsets!
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }*/

    mInputStream.seekg( static_cast< std::istream::off_type >( offset ), way );

    if ( mInputStream.bad() ) {
        return HRESULT_FROM_WIN32( ERROR_SEEK );
    }

    if ( newPosition ) {
        *newPosition = mInputStream.tellg();
    }

    return S_OK;
}

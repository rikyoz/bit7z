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

#include "internal/fixedbufferextractcallback.hpp"

#include "internal/cfixedbufferoutstream.hpp"
#include "internal/fs.hpp"

using bit7z::BitInputArchive;
using bit7z::FixedBufferExtractCallback;
using bit7z::byte_t;

FixedBufferExtractCallback::FixedBufferExtractCallback( const BitInputArchive &inputArchive,
                                                        byte_t *buffer,
                                                        size_t size )
        : ExtractCallback( inputArchive ), mBuffer( buffer ), mSize( size ) {}

void FixedBufferExtractCallback::releaseStream() {
    mOutMemStream.Release();
}

HRESULT FixedBufferExtractCallback::getOutStream( uint32_t index,
                                                  ISequentialOutStream **outStream,
                                                  int32_t askExtractMode ) {
    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract || mInputArchive.isItemFolder( index ) ) {
        return S_OK;
    }

    // Get Name
    BitPropVariant prop = mInputArchive.itemProperty( index, BitProperty::Path );
    tstring fullPath;

    if ( prop.isEmpty() ) {
        fullPath = kEmptyFileAlias;
    } else if ( prop.isString() ) {
        fullPath = prop.getString();
    } else {
        return E_FAIL;
    }

    if ( mHandler.fileCallback() ) {
        mHandler.fileCallback()( fullPath );
    }

    auto outStreamLoc = bit7z::make_com< CFixedBufferOutStream, ISequentialOutStream >( mBuffer, mSize );
    mOutMemStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    return S_OK;
}

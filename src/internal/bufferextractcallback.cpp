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

#include "internal/bufferextractcallback.hpp"

#include "internal/cbufferoutstream.hpp"
#include "internal/fsutil.hpp"
#include "internal/util.hpp"

using namespace std;
using namespace NWindows;
using namespace bit7z;

BufferExtractCallback::BufferExtractCallback( const BitInputArchive& inputArchive,
                                              map< tstring, vector< byte_t > >& buffersMap )
    : ExtractCallback( inputArchive ),
      mBuffersMap( buffersMap ) {}

void BufferExtractCallback::releaseStream() {
    mOutMemStream.Release();
}

HRESULT BufferExtractCallback::getOutStream( uint32_t index,
                                             ISequentialOutStream** outStream,
                                             int32_t askExtractMode ) {
    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract || mInputArchive.isItemFolder( index ) ) {
        return S_OK;
    }

    // Get Name
    BitPropVariant prop = mInputArchive.getItemProperty( index, BitProperty::Path );
    tstring fullPath;

    if ( prop.isEmpty() ) {
        fullPath = fs::path( kEmptyFileAlias ).native();
    } else if ( prop.isString() ) {
        fullPath = prop.getString();
    } else {
        return E_FAIL;
    }

    if ( mHandler.fileCallback() ) {
        mHandler.fileCallback()( fullPath );
    }

    //Note: using [] operator it creates the buffer if it does not exist already!
    auto outStreamLoc = bit7z::make_com< CBufferOutStream, ISequentialOutStream >( mBuffersMap[ fullPath ] );
    mOutMemStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    return S_OK;
}


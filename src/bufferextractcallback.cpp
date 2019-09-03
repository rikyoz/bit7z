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

#include "../include/bufferextractcallback.hpp"

#include "7zip/Common/StreamObjects.h"

#include "../include/cbufoutstream.hpp"
#include "../include/bitpropvariant.hpp"
#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

using namespace std;
using namespace NWindows;
using namespace bit7z;

BufferExtractCallback::BufferExtractCallback( const BitArchiveHandler& handler,
                                              const BitInputArchive& inputArchive,
                                              map< wstring, vector< byte_t > >& buffersMap )
    : ExtractCallback ( handler, inputArchive ),
      mBuffersMap( buffersMap ) {}

BufferExtractCallback::~BufferExtractCallback() {}

STDMETHODIMP BufferExtractCallback::GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode ) try {
    *outStream = nullptr;
    mOutMemStream.Release();

    // Get Name
    BitPropVariant prop = mInputArchive.getItemProperty( index, BitProperty::Path );
    wstring fullPath;

    if ( prop.isEmpty() ) {
        fullPath = kEmptyFileAlias;
    } else if ( prop.isString() ) {
        fullPath = prop.getString();
    } else {
        return E_FAIL;
    }

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract ) {
        return S_OK;
    }

    if ( !mInputArchive.isItemFolder( index ) ) {
        //Note: using [] operator it creates the buffer if it does not exists already!
        auto* out_mem_stream_spec = new CBufOutStream( mBuffersMap[ fullPath ] );
        CMyComPtr< ISequentialOutStream > outStreamLoc( out_mem_stream_spec );
        mOutMemStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }

    return S_OK;
} catch ( const BitException& ) {
    return E_OUTOFMEMORY;
}

STDMETHODIMP BufferExtractCallback::SetOperationResult( Int32 operationResult ) {
    switch ( operationResult ) {
        case NArchive::NExtract::NOperationResult::kOK:
            break;

        default: {
            mNumErrors++;

            switch ( operationResult ) {
                case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
                    mErrorMessage = kUnsupportedMethod;
                    break;

                case NArchive::NExtract::NOperationResult::kCRCError:
                    mErrorMessage = kCRCFailed;
                    break;

                case NArchive::NExtract::NOperationResult::kDataError:
                    mErrorMessage = kDataError;
                    break;

                default:
                    mErrorMessage = kUnknownError;
            }
        }
    }

    mOutMemStream.Release();

    return mNumErrors > 0 ? E_FAIL : S_OK;
}


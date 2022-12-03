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

#include "internal/streamextractcallback.hpp"

#include "bitexception.hpp"
#include "internal/cstdoutstream.hpp"

using namespace std;
using namespace NWindows;
using namespace bit7z;

StreamExtractCallback::StreamExtractCallback( const BitInputArchive& inputArchive, ostream& outputStream )
    : ExtractCallback( inputArchive ),
      mOutputStream( outputStream ) {}

void StreamExtractCallback::throwException( HRESULT error ) {
    if ( !mOutputStream ) {
        throw BitException( "Stream error", std::error_code( errno, std::generic_category() ) );
    }
    Callback::throwException( error );
}

void StreamExtractCallback::releaseStream() {
    mStdOutStream.Release();
}

HRESULT StreamExtractCallback::getOutStream( uint32_t index, ISequentialOutStream** outStream ) {
    if ( mInputArchive.isItemFolder( index ) ) {
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

    auto outStreamLoc = bit7z::make_com< CStdOutStream, IOutStream >( mOutputStream );
    mStdOutStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    return S_OK;
}
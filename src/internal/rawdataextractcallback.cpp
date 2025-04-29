// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/rawdataextractcallback.hpp"

#include "bitinputarchive.hpp"
#include "bitpropvariant.hpp"
#include "internal/crawoutstream.hpp"
#include "internal/extractcallback.hpp"
#include "internal/util.hpp"

#include <cstdint>
#include <ostream>
#include <utility>

using namespace NWindows;

namespace bit7z {

RawDataExtractCallback::RawDataExtractCallback( const BitInputArchive& inputArchive, RawDataCallback callback )
    : ExtractCallback( inputArchive ),
      mCallback( std::move( callback ) ) {}

void RawDataExtractCallback::releaseStream() {
    mCallbackStream.Release();
}

auto RawDataExtractCallback::getOutStream( uint32_t index, ISequentialOutStream** outStream ) -> HRESULT {
    if ( isItemFolder( index ) ) {
        return S_OK;
    }

    if ( mHandler.fileCallback() ) {
        // Get Name
        const BitPropVariant prop = itemProperty( index, BitProperty::Path );
        tstring fullPath;

        if ( prop.isEmpty() ) {
            fullPath = kEmptyFileAlias;
        } else if ( prop.isString() ) {
            fullPath = prop.getString();
        } else {
            return E_FAIL;
        }

        mHandler.fileCallback()( fullPath );
    }

    auto outStreamLoc = bit7z::make_com< CRawOutStream, ISequentialOutStream >( mCallback );
    mCallbackStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    return S_OK;
}

} // namespace bit7z
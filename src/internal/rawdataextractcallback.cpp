// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/rawdataextractcallback.hpp"

#include "bitinputarchive.hpp"
#include "internal/crawoutstream.hpp"
#include "internal/extractcallback.hpp"
#include "internal/util.hpp"

#include <cstdint>
#include <utility>

using namespace NWindows;

namespace bit7z {

RawDataExtractCallback::RawDataExtractCallback( const BitInputArchive& inputArchive, RawDataCallback callback )
    : ExtractCallback( inputArchive ),
      mCallback( std::move( callback ) ) {}

void RawDataExtractCallback::releaseStream() {
    mCallbackStream.Release();
}

auto RawDataExtractCallback::getOutStream( const BitArchiveItem& item, ISequentialOutStream** outStream ) -> HRESULT {
    if ( item.isDir() ) {
        return S_OK;
    }

    if ( mHandler.fileCallback() ) {
        const auto fullPath = itemExtractionPath( item );
        if ( !fullPath ) {
            return E_FAIL;
        }
        mHandler.fileCallback()( *fullPath );
    }

    auto outStreamLoc = bit7z::make_com< CRawOutStream, ISequentialOutStream >( mCallback );
    mCallbackStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    return S_OK;
}

} // namespace bit7z

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

#include "rawdataextractcallbackwid.hpp"

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

RawDataExtractCallbackWid::RawDataExtractCallbackWid( const BitInputArchive& inputArchive, CallbackType& callback )
    : ExtractCallback( inputArchive ),
      mCallback( callback ) {}

void RawDataExtractCallbackWid::releaseStream() {
    mCallbackStream.Release();
}

auto RawDataExtractCallbackWid::getOutStream( uint32_t index, ISequentialOutStream** outStream ) -> HRESULT {
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

    RawDataCallback callback = [this](const byte_t* data_start, std::size_t data_size) -> bool {
        return mCallback.write(data_start, data_size);
    };

    mCallback.onNewFile(index);
    auto outStreamLoc = bit7z::make_com< CRawOutStream, ISequentialOutStream >( std::move(callback) );
    mCallbackStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    return S_OK;
}

} // namespace bit7z
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

#include "internal/streamextractcallback.hpp"

#include "bitinputarchive.hpp"
#include "internal/cstdoutstream.hpp"
#include "internal/extractcallback.hpp"
#include "internal/util.hpp"

#include <ostream>

using namespace NWindows;

namespace bit7z {

StreamExtractCallback::StreamExtractCallback( const BitInputArchive& inputArchive, std::ostream& outputStream )
    : ExtractCallback( inputArchive ),
      mOutputStream( outputStream ) {}

void StreamExtractCallback::releaseStream() {
    mStdOutStream.Release();
}

auto StreamExtractCallback::getOutStream( const BitArchiveItem& item, ISequentialOutStream** outStream ) -> HRESULT {
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

    auto outStreamLoc = bit7z::make_com< CStdOutStream, IOutStream >( mOutputStream );
    mStdOutStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    return S_OK;
}

} // namespace bit7z

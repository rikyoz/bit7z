// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/sequentialextractcallback.hpp"

#include "bitinputarchive.hpp"
#include "internal/csynchronizedoutstream.hpp"
#include "internal/extractcallback.hpp"
#include "internal/util.hpp"

#include <cstdint>

using namespace NWindows;

namespace bit7z {

SequentialExtractCallback::SequentialExtractCallback( const BitInputArchive& inputArchive, BufferQueue& queue )
    : ExtractCallback( inputArchive ),
      mBufferQueue{ queue } {}

void SequentialExtractCallback::releaseStream() {
    mSeqOutStream.Release();
}

auto SequentialExtractCallback::getOutStream( uint32_t index, ISequentialOutStream** outStream ) -> HRESULT {
    if ( isItemFolder( index ) ) {
        return S_OK;
    }

    auto outStreamLoc = bit7z::make_com< CSynchronizedOutStream, ISequentialOutStream >( mBufferQueue );
    mSeqOutStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    return S_OK;
}

auto SequentialExtractCallback::finishOperation( OperationResult operationResult ) -> HRESULT {
    mSeqOutStream.Release();
    return ExtractCallback::finishOperation( operationResult );
}

} // namespace bit7z
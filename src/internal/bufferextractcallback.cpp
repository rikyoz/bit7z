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
#include "internal/bufferextractcallback.hpp"

#include "bitabstractarchivehandler.hpp"
#include "bitexception.hpp"
#include "bitinputarchive.hpp"
#include "bitpropvariant.hpp"
#include "bittypes.hpp"
#include "internal/cbufferoutstream.hpp"
#include "internal/extractcallback.hpp"
#include "internal/fs.hpp"
#include "internal/fsutil.hpp"
#include "internal/util.hpp"

#include <cstdint>

namespace bit7z {

BufferExtractCallback::BufferExtractCallback( const BitInputArchive& inputArchive,
                                              BufferCallback callback,
                                              FilterCallback filterCallback )
    : ExtractCallback( inputArchive, std::move( filterCallback ) ),
      mBufferCallback{ std::move( callback ) },
      mExtractionAttempted{ false } {}

auto BufferExtractCallback::extractionAttempted() const -> bool {
    return mExtractionAttempted;
}

void BufferExtractCallback::releaseStream() {
    mOutMemStream.Release();
}

auto BufferExtractCallback::getOutStream( std::uint32_t index, ISequentialOutStream** outStream ) -> HRESULT try {
    if ( isItemFolder( index ) ) {
        return S_OK;
    }

    // Get Name
    const BitPropVariant prop = itemProperty( index, BitProperty::Path );
    tstring fullPath;

    if ( prop.isEmpty() ) {
        fullPath = kEmptyFileAlias;
    } else if ( prop.isString() ) {
        if ( !mHandler.retainDirectories() ) {
            fullPath = path_to_tstring( fs::path{ prop.getNativeString() }.filename() );
        } else {
            fullPath = prop.getString();
        }
    } else {
        return E_FAIL;
    }

    if ( mHandler.fileCallback() ) {
        mHandler.fileCallback()( fullPath );
    }

    auto& outBuffer = mBufferCallback( index, fullPath );
    if ( !outBuffer.empty() ) {
        switch ( mHandler.overwriteMode() ) {
            case OverwriteMode::None: {
                throw BitException( "Cannot erase output buffer", make_hresult_code( E_ABORT ) );
            }
            case OverwriteMode::Skip: {
                return S_OK;
            }
            case OverwriteMode::Overwrite:
            default: {
                outBuffer.clear();
                break;
            }
        }
    }

    auto outStreamLoc = bit7z::make_com< CBufferOutStream, ISequentialOutStream >( outBuffer );
    mOutMemStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    mExtractionAttempted = true;
    return S_OK;
} catch( const BitException& ex ) {
    return ex.hresultCode();
}

} // namespace bit7z
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

#include "../include/streamextractcallback.hpp"

#include "../include/cstdoutstream.hpp"
#include "../include/bitexception.hpp"

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

HRESULT StreamExtractCallback::getOutStream( uint32_t index,
                                             ISequentialOutStream** outStream,
                                             int32_t askExtractMode ) {
    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract ) {
        return S_OK;
    }

    if ( !mInputArchive.isItemFolder( index ) ) {
        auto* out_std_stream_spec = new CStdOutStream( mOutputStream );
        CMyComPtr< IOutStream > outStreamLoc( out_std_stream_spec );
        mStdOutStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }
    return S_OK;
}

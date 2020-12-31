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

#include "../include/streamupdatecallback.hpp"

#include "../include/cstdinstream.hpp"

using namespace std;
using namespace bit7z;

/* Most of this code is taken from the CUpdateCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of std::vector instead of CRecordVector, CObjectVector and UStringVector
 *  + Use of tstring instead of UString (see Callback base interface)
 *  + Error messages are not showed (see comments in ExtractCallback)
 *  + The work performed originally by the Init method is now performed by the class constructor
 *  + FSItem class is used instead of CDirItem struct */

StreamUpdateCallback::StreamUpdateCallback( const BitArchiveCreator& creator,
                                            istream& in_stream,
                                            const tstring& in_stream_name )
    : UpdateCallback( creator ),
      mStream( in_stream ),
      mStreamName( in_stream_name.empty() ? kEmptyFileAlias : in_stream_name ) {}

uint32_t StreamUpdateCallback::itemsCount() const {
    return mOldArcItemsCount + 1;
}

BitPropVariant StreamUpdateCallback::getNewItemProperty( uint32_t /*index*/, PROPID propID ) {
    BitPropVariant prop;
    switch ( propID ) {
        case kpidPath:
            prop = mStreamName.wstring();
            break;
        case kpidIsDir:
            prop = false;
            break;
        case kpidSize: {
            auto original_pos = mStream.tellg();
            mStream.seekg( 0, std::ios::end ); // seeking to the end of the stream
            prop = static_cast< uint64_t >( mStream.tellg() - original_pos ); // size of the stream
            mStream.seekg( original_pos ); // seeking back to the original position in the stream
            break;
        }
        case kpidAttrib:
            prop = static_cast< uint32_t >( FILE_ATTRIBUTE_NORMAL );
            break;
        case kpidCTime:
        case kpidATime:
        case kpidMTime: {
            FILETIME ft;
            SYSTEMTIME st;

            GetSystemTime( &st ); // gets current time
            SystemTimeToFileTime( &st, &ft ); // converts to file time format
            prop = ft;
            break;
        }
        default: //empty prop
            break;
    }
    return prop;
}

HRESULT StreamUpdateCallback::getNewItemStream( uint32_t /*index*/, ISequentialInStream** inStream ) {
    //Note: CMyComPtr is needed for correctly counting references inside CBufferInStream
    CMyComPtr< ISequentialInStream > inStreamLoc = new CStdInStream( mStream );
    *inStream = inStreamLoc.Detach(); //Note: 7-zip will take care of freeing the memory!
    return S_OK;
}
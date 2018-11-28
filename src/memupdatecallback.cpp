// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
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

#include "../include/memupdatecallback.hpp"

#include <iostream>
#include <string>

#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamObjects.h"
#include "Common/IntToString.h"

#include "../include/bitpropvariant.hpp"

using namespace std;
using namespace bit7z;

/* Most of this code is taken from the CUpdateCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of std::vector instead of CRecordVector, CObjectVector and UStringVector
 *  + Use of std::wstring instead of UString (see Callback base interface)
 *  + Error messages are not showed (see comments in ExtractCallback)
 *  + The work performed originally by the Init method is now performed by the class constructor
 *  + FSItem class is used instead of CDirItem struct */

const std::wstring kEmptyFileAlias = L"[Content]";

MemUpdateCallback::MemUpdateCallback( const BitArchiveCreator& creator, const vector< byte_t >& out_buffer, const wstring& buffer_name ) :
    mCreator( creator ),
    mAskPassword( false ),
    mNeedBeClosed( false ),
    mBuffer( out_buffer ),
    mBufferName( buffer_name ) {}

MemUpdateCallback::~MemUpdateCallback() {
    Finilize();
}

HRESULT MemUpdateCallback::SetTotal( UInt64 size ) {
    if ( mCreator.totalCallback() ) {
        mCreator.totalCallback()( size );
    }
    return S_OK;
}

HRESULT MemUpdateCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mCreator.progressCallback() ) {
        mCreator.progressCallback()( *completeValue );
    }
    return S_OK;
}

HRESULT MemUpdateCallback::EnumProperties( IEnumSTATPROPSTG** /* enumerator */ ) {
    return E_NOTIMPL;
}

HRESULT MemUpdateCallback::GetUpdateItemInfo( UInt32 /* index */, Int32* newData,
                                              Int32* newProperties, UInt32* indexInArchive ) {
    if ( newData != nullptr ) {
        *newData = 1; //= true;
    }
    if ( newProperties != nullptr ) {
        *newProperties = 1; //= true;
    }
    if ( indexInArchive != nullptr ) {
        *indexInArchive = static_cast< uint32_t >( -1 );
    }

    return S_OK;
}

HRESULT MemUpdateCallback::GetProperty( UInt32 /*index*/, PROPID propID, PROPVARIANT* value ) {
    BitPropVariant prop;

    if ( propID == kpidIsAnti ) {
        prop = false;
        *value = prop;
        return S_OK;
    }

    FILETIME ft;
    SYSTEMTIME st;

    GetSystemTime( &st ); // gets current time
    SystemTimeToFileTime( &st, &ft ); // converts to file time format

    switch ( propID ) {
        case kpidPath:
            prop = ( mBufferName.empty() ) ? kEmptyFileAlias : mBufferName;
            break;
        case kpidIsDir:
            prop = false;
            break;
        case kpidSize:
            prop = static_cast< uint64_t >( sizeof( byte_t ) * mBuffer.size() );
            break;
        case kpidAttrib:
            prop = static_cast< uint32_t >( FILE_ATTRIBUTE_NORMAL );
            break;
        case kpidCTime:
            prop = ft;
            break;
        case kpidATime:
            prop = ft;
            break;
        case kpidMTime:
            prop = ft;
            break;
    }

    *value = prop;
    return S_OK;
}

HRESULT MemUpdateCallback::Finilize() {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

HRESULT MemUpdateCallback::GetStream( UInt32 /*index*/, ISequentialInStream** inStream ) {
    RINOK( Finilize() );

    auto* inStreamSpec = new CBufInStream;
    CMyComPtr< ISequentialInStream > inStreamLoc( inStreamSpec );
    inStreamSpec->Init( &mBuffer[0], mBuffer.size() );

    *inStream = inStreamLoc.Detach();
    return S_OK;
}

HRESULT MemUpdateCallback::SetOperationResult( Int32 /* operationResult */ ) {
    mNeedBeClosed = true;
    return S_OK;
}

HRESULT MemUpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
    if ( !mCreator.isPasswordDefined() ) {
        if ( mAskPassword ) {
            // You can ask real password here from user
            // Password = GetPassword(OutStream);
            // PasswordIsDefined = true;
            mErrorMessage = L"Password is not defined";
            return E_ABORT;
        }
    }

    *passwordIsDefined = ( mCreator.isPasswordDefined() ? 1 : 0 );
    return StringToBstr( mCreator.password().c_str(), password );
}

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#include "internal/updatecallback.hpp"

#include "internal/cfileoutstream.hpp"
#include "internal/genericstreamitem.hpp"
#include "internal/util.hpp"

using namespace bit7z;

UpdateCallback::UpdateCallback( const BitOutputArchive& output )
    : Callback{ output.getHandler() },
      mOutputArchive{ output },
      mNeedBeClosed{ false } {}

UpdateCallback::~UpdateCallback() {
    Finalize();
}

HRESULT UpdateCallback::Finalize() noexcept {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetTotal( UInt64 size ) {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetCompleted( const UInt64* completeValue ) {
    if ( completeValue != nullptr && mHandler.progressCallback() ) {
        return mHandler.progressCallback()( *completeValue ) ? S_OK : E_ABORT;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) {
    if ( inSize != nullptr && outSize != nullptr && mHandler.ratioCallback() ) {
        mHandler.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value ) {
    BitPropVariant prop;
    if ( propID == kpidIsAnti ) {
        prop = false;
    } else {
        prop = mOutputArchive.getOutputItemProperty( index, static_cast< BitProperty >( propID ) );
    }
    *value = prop;
    prop.bstrVal = nullptr;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetStream( UInt32 index, ISequentialInStream** inStream ) {
    RINOK( Finalize() )

    if ( mHandler.fileCallback() ) {
        BitPropVariant filePath = mOutputArchive.getOutputItemProperty( index, BitProperty::Path );
        if ( filePath.isString() ) {
            mHandler.fileCallback()( filePath.getString() );
        }
    }

    return mOutputArchive.getOutputItemStream( index, inStream );
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetVolumeSize( UInt32 /*index*/, UInt64* /*size*/ ) noexcept {
    return S_FALSE;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetVolumeStream( UInt32 index, ISequentialOutStream** volumeStream ) {
    tstring res = to_tstring( index + 1 );
    if ( res.length() < 3 ) {
        //adding leading zeros for a total res length of 3 (e.g. volume 42 will have extension .042)
        res.insert( res.begin(), 3 - res.length(), TSTRING( '0' ) );
    }

    tstring fileName = TSTRING( '.' ) + res;// + mVolExt;
    auto stream = bit7z::make_com< CFileOutStream >( fileName );

    if ( stream->fail() ) {
        return HRESULT_FROM_WIN32( ERROR_OPEN_FAILED );
    }

    *volumeStream = stream.Detach();
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetUpdateItemInfo( UInt32 index,
                                                Int32* newData,
                                                Int32* newProperties,
                                                UInt32* indexInArchive ) noexcept {
    if ( newData != nullptr ) {
        *newData = static_cast< Int32 >( mOutputArchive.hasNewData( index ) ); //1 = true, 0 = false;
    }
    if ( newProperties != nullptr ) {
        *newProperties = static_cast< Int32 >( mOutputArchive.hasNewProperties( index ) ); //1 = true, 0 = false;
    }
    if ( indexInArchive != nullptr ) {
        *indexInArchive = mOutputArchive.getIndexInArchive( index );
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetOperationResult( Int32 /* operationResult */ ) noexcept {
    mNeedBeClosed = true;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
    *passwordIsDefined = ( mHandler.isPasswordDefined() ? 1 : 0 );
    return StringToBstr( WIDEN( mHandler.password() ).c_str(), password );
}
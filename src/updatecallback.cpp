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

#include "../include/updatecallback.hpp"

using namespace bit7z;

UpdateCallback::UpdateCallback( const BitArchiveCreator& creator )
    : Callback( creator ),
      mOldArc( nullptr ),
      mOldArcItemsCount( 0 ),
      mAskPassword( false ),
      mNeedBeClosed( false ) {}

UpdateCallback::~UpdateCallback() {
    Finilize();
}

void UpdateCallback::setOldArc( const BitInputArchive* old_arc ) {
    if ( old_arc ) {
        mOldArc = old_arc;
        mOldArcItemsCount = old_arc->itemsCount();
    }
}

HRESULT UpdateCallback::Finilize() {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

STDMETHODIMP UpdateCallback::SetTotal( UInt64 size ) {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

STDMETHODIMP UpdateCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mHandler.progressCallback() && completeValue != nullptr ) {
        mHandler.progressCallback()( *completeValue );
    }
    return S_OK;
}

STDMETHODIMP UpdateCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) {
    if ( mHandler.ratioCallback() && inSize != nullptr && outSize != nullptr ) {
        mHandler.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

HRESULT UpdateCallback::EnumProperties( IEnumSTATPROPSTG** /* enumerator */ ) {
    return E_NOTIMPL;
}

HRESULT UpdateCallback::GetUpdateItemInfo( UInt32 index,
                                           Int32* newData,
                                           Int32* newProperties,
                                           UInt32* indexInArchive ) {

    bool isOldItem = index < mOldArcItemsCount;

    if ( newData != nullptr ) {
        *newData = isOldItem ? 0 : 1; //= true;
    }
    if ( newProperties != nullptr ) {
        *newProperties = isOldItem ? 0 : 1; //= true;
    }
    if ( indexInArchive != nullptr ) {
        *indexInArchive = isOldItem ? index : static_cast< uint32_t >( -1 );
    }

    return S_OK;
}

HRESULT UpdateCallback::SetOperationResult( Int32 /* operationResult */ ) {
    mNeedBeClosed = true;
    return S_OK;
}

HRESULT UpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
    if ( !mHandler.isPasswordDefined() ) {
        if ( mAskPassword ) {
            // You can ask real password here from user
            // Password = GetPassword(OutStream);
            // PasswordIsDefined = true;
            mErrorMessage = L"Password is not defined";
            return E_ABORT;
        }
    }

    *passwordIsDefined = ( mHandler.isPasswordDefined() ? 1 : 0 );
    return StringToBstr( mHandler.password().c_str(), password );
}

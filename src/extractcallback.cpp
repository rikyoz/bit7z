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

#include "../include/extractcallback.hpp"

using namespace bit7z;

ExtractCallback::ExtractCallback( const BitArchiveHandler& handler,
                                  const BitInputArchive& inputArchive )
    : Callback( handler ),
      mInputArchive( inputArchive ),
      mExtractMode( true ),
      mNumErrors( 0 ) {}

ExtractCallback::~ExtractCallback() {}

STDMETHODIMP ExtractCallback::SetTotal( UInt64 size ) {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

STDMETHODIMP ExtractCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mHandler.progressCallback() && completeValue != nullptr ) {
        mHandler.progressCallback()( *completeValue );
    }
    return S_OK;
}

STDMETHODIMP ExtractCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) {
    if ( mHandler.ratioCallback() && inSize != nullptr && outSize != nullptr ) {
        mHandler.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

STDMETHODIMP ExtractCallback::PrepareOperation( Int32 askExtractMode ) {
    mExtractMode = false;

    // in future we might use this switch to handle an event like onOperationStart(Operation o)
    // with enum Operation{Extract, Test, Skip}

    switch ( askExtractMode ) {
    case NArchive::NExtract::NAskMode::kExtract:
        mExtractMode = true;
        break;

    case NArchive::NExtract::NAskMode::kTest:
        mExtractMode = false;
        break;
    }

    return S_OK;
}

STDMETHODIMP ExtractCallback::CryptoGetTextPassword( BSTR* password ) {
    wstring pass;
    if ( !mHandler.isPasswordDefined() ) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        if ( mHandler.passwordCallback() ) {
            pass = mHandler.passwordCallback()();
        }

        if ( pass.empty() ) {
            mErrorMessage = L"Password is not defined";
            return E_FAIL;
        }
    } else {
        pass = mHandler.password();
    }

    return StringToBstr( pass.c_str(), password );
}

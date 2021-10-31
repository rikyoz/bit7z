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

#include "internal/extractcallback.hpp"

#include "bitexception.hpp"

using namespace bit7z;

ExtractCallback::ExtractCallback( const BitInputArchive& inputArchive )
    : Callback( inputArchive.handler() ),
      mInputArchive( inputArchive ),
      mExtractMode( true ),
      mNumErrors( 0 ) {}

void ExtractCallback::finishOperation() {
    releaseStream();
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetTotal( UInt64 size ) {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mHandler.progressCallback() && completeValue != nullptr ) {
        return mHandler.progressCallback()( *completeValue ) ? S_OK : E_ABORT;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) {
    if ( mHandler.ratioCallback() && inSize != nullptr && outSize != nullptr ) {
        mHandler.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::PrepareOperation( Int32 askExtractMode ) noexcept {
    // in the future, we might use a switch to handle an event like onOperationStart(Operation o)
    // with enum Operation{Extract, Test, Skip}
    mExtractMode = ( askExtractMode == NArchive::NExtract::NAskMode::kExtract );
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode ) try {
    *outStream = nullptr;
    releaseStream();

    return getOutStream( index, outStream, askExtractMode );
} catch ( const BitException& ) {
    return E_OUTOFMEMORY;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetOperationResult( Int32 operationResult ) {
    using namespace NArchive::NExtract;
    constexpr auto kUnsupportedMethod  = "Unsupported Method";
    constexpr auto kCRCFailed          = "CRC Failed";
    constexpr auto kDataError          = "Data Error";
    constexpr auto kUnknownError       = "Unknown Error";

    if ( operationResult != NOperationResult::kOK ) {
        mNumErrors++;

        switch ( operationResult ) {
            case NOperationResult::kUnsupportedMethod:
                mErrorMessage = kUnsupportedMethod;
                break;

            case NOperationResult::kCRCError:
                mErrorMessage = kCRCFailed;
                break;

            case NOperationResult::kDataError:
                mErrorMessage = kDataError;
                break;

            default:
                mErrorMessage = kUnknownError;
        }
    }

    finishOperation();

    return mNumErrors > 0 ? E_FAIL : S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::CryptoGetTextPassword( BSTR* password ) {
    std::wstring pass;
    if ( !mHandler.isPasswordDefined() ) {
        if ( mHandler.passwordCallback() ) {
            pass = WIDEN( mHandler.passwordCallback()() );
        }

        if ( pass.empty() ) {
            mErrorMessage = kPasswordNotDefined;
            return E_FAIL;
        }
    } else {
        pass = WIDEN( mHandler.password() );
    }

    return StringToBstr( pass.c_str(), password );
}

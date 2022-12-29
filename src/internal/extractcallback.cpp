// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/extractcallback.hpp"

#include "bitexception.hpp"
#include "internal/util.hpp"

using namespace bit7z;

ExtractCallback::ExtractCallback( const BitInputArchive& inputArchive )
    : Callback( inputArchive.handler() ),
      mInputArchive( inputArchive ),
      mExtractMode( ExtractMode::Extract ) {}

HRESULT ExtractCallback::finishOperation( OperationResult operation_result ) {
    releaseStream();
    return operation_result != OperationResult::Success ? E_FAIL : S_OK;
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
    // with enum ExtractMode.
    mExtractMode = static_cast< ExtractMode >( askExtractMode );
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode ) try {
    *outStream = nullptr;
    releaseStream();

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract ) {
        return S_OK;
    }

    return getOutStream( index, outStream );
} catch ( const BitException& ) {
    return E_OUTOFMEMORY;
} catch ( const std::runtime_error& ) {
    return E_ABORT;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetOperationResult( Int32 operationResult ) {
    using namespace NArchive::NExtract;
    constexpr auto kUnsupportedMethod = "Unsupported Method";
    constexpr auto kCRCFailed = "CRC Failed";
    constexpr auto kDataError = "Data Error";
    constexpr auto kUnknownError = "Unknown Error";

    auto result = static_cast< OperationResult >( operationResult );
    if ( result != OperationResult::Success ) {
        switch ( result ) {
            case OperationResult::UnsupportedMethod:
                mErrorMessage = kUnsupportedMethod;
                break;

            case OperationResult::CRCError:
                mErrorMessage = kCRCFailed;
                break;

            case OperationResult::DataError:
                mErrorMessage = kDataError;
                break;

            default:
                mErrorMessage = kUnknownError;
        }
    }

    return finishOperation( result );
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

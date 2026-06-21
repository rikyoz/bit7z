// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/extractcallback.hpp"

#include "bitabstractarchivehandler.hpp"
#include "bitexception.hpp"
#include "bitinputarchive.hpp"
#include "bitpropvariant.hpp"
#include "internal/callback.hpp"
#include "internal/operationresult.hpp"
#include "internal/stringutil.hpp"

#include <7zip/Archive/IArchive.h>

#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>

namespace bit7z {

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetTotal( UInt64 size ) noexcept {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetCompleted( const UInt64* completeValue ) noexcept {
    if ( mHandler.progressCallback() && completeValue != nullptr ) {
        return mHandler.progressCallback()( *completeValue ) ? S_OK : E_ABORT;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) noexcept {
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
STDMETHODIMP ExtractCallback::GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode ) noexcept
try {
    *outStream = nullptr;
    releaseStream();

    // The index comes from 7-Zip and is guaranteed valid, so we build the item once without
    // re-validating it, and reuse it for the encrypted check, the filter callback, and getOutStream.
    const auto item = mInputArchive.itemAtUnchecked( index );

    const auto isEncrypted = item.itemProperty( BitProperty::Encrypted );
    if ( isEncrypted.isBool() ) {
        mIsLastItemEncrypted = isEncrypted.getBool();
    }

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract ) {
        return S_OK;
    }

    if ( mFilterCallback ) {
        const auto filterResult = mFilterCallback( item );
        if ( filterResult == FilterResult::SkipItem ) {
            return S_OK;
        }
        if ( filterResult == FilterResult::AbortOperation ) {
            return E_ABORT;
        }
        // if filterResult == FilterResult::ProcessItem, continue.
    }

    return getOutStream( item, outStream );
} catch ( const BitException& exception ) {
    mErrorException = std::make_exception_ptr( exception );
    return exception.hresultCode();
} catch ( const std::runtime_error& ) {
    mErrorException = std::make_exception_ptr(
        BitException( "Failed to get the stream", make_hresult_code( E_ABORT ) )
    );
    return E_ABORT;
}

ExtractCallback::ExtractCallback( const BitInputArchive& inputArchive, FilterCallback filterCallback )
    : Callback( inputArchive.handler() ),
      mInputArchive( inputArchive ),
      mExtractMode( ExtractMode::Extract ),
      mIsLastItemEncrypted{ false },
      mFilterCallback{ std::move( filterCallback ) } {}

auto ExtractCallback::finishOperation( OperationResult operationResult ) -> HRESULT {
    releaseStream();
    return operationResult != OperationResult::Success ? E_FAIL : S_OK;
}

namespace {
auto mapOperationResult( Int32 operationResult, bool isLastItemEncrypted ) -> OperationResult {
    using namespace NArchive::NExtract;

    if ( isLastItemEncrypted ) {
        if ( operationResult == NOperationResult::kCRCError ) {
            return OperationResult::CRCErrorEncrypted;
        }

        if ( operationResult == NOperationResult::kDataError ) {
            return OperationResult::DataErrorEncrypted;
        }
    }

    return static_cast< OperationResult >( operationResult );
}
} // namespace

constexpr auto kTestFailed = "Failed to test the archive";
constexpr auto kExtractFailed = "Failed to extract the archive";

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetOperationResult( Int32 operationResult ) noexcept {
    using namespace NArchive::NExtract;

    const auto result = mapOperationResult( operationResult, mIsLastItemEncrypted );
    if ( result != OperationResult::Success ) {
        const auto* msg = mExtractMode == ExtractMode::Test ? kTestFailed : kExtractFailed;
        const auto error = make_error_code( result );
        mErrorException = std::make_exception_ptr( BitException( msg, error ) );
    }

    return finishOperation( result );
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::CryptoGetTextPassword( BSTR* password ) noexcept {
    std::wstring pass;
    if ( !mHandler.isPasswordDefined() ) {
        if ( mHandler.passwordCallback() ) {
            pass = WIDEN( mHandler.passwordCallback()() );
        }

        if ( pass.empty() ) {
            const auto* msg = mExtractMode == ExtractMode::Test ? kTestFailed : kExtractFailed;
            const auto error = make_error_code( OperationResult::EmptyPassword );
            mErrorException = std::make_exception_ptr( BitException( msg, error ) );
            return E_FAIL;
        }
    } else {
        pass = WIDEN( mHandler.password() );
    }

    return StringToBstr( pass.c_str(), password );
}

auto ExtractCallback::inputArchive() const -> const BitInputArchive& {
    return mInputArchive;
}

auto ExtractCallback::errorException() const -> const std::exception_ptr& {
    return mErrorException;
}

auto ExtractCallback::extractionAttempted() const -> bool {
    return true;
}

auto ExtractCallback::extractMode() const noexcept -> ExtractMode {
    return mExtractMode;
}

} // namespace bit7z

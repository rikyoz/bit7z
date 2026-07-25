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

#include "internal/updatecallback.hpp"

#include "bitexception.hpp"
#include "bitoutputarchive.hpp"
#include "bitpropvariant.hpp"
#include "bittypes.hpp"
#include "bitwindows.hpp"
#include "internal/callback.hpp"
#include "internal/cfileoutstream.hpp"
#include "internal/exceptionutil.hpp"
#include "internal/stringutil.hpp"
#include "internal/util.hpp"

#include <new>
#include <string>
#include <system_error>

namespace bit7z {

UpdateCallback::UpdateCallback( const BitOutputArchive& output )
    : Callback{ output.handler() },
      mOutputArchive{ output },
      mNeedBeClosed{ false } {}

UpdateCallback::~UpdateCallback() {
    finalize();
}

auto UpdateCallback::finalize() noexcept -> HRESULT {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetTotal( UInt64 size ) noexcept {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetCompleted( const UInt64* completeValue ) noexcept {
    if ( completeValue != nullptr && mHandler.progressCallback() ) {
        return mHandler.progressCallback()( *completeValue ) ? S_OK : E_ABORT;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) noexcept {
    if ( inSize != nullptr && outSize != nullptr && mHandler.ratioCallback() ) {
        mHandler.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetProperty( UInt32 index, PROPID propId, PROPVARIANT* value ) noexcept try {
    BitPropVariant prop;
    if ( propId == kpidIsAnti ) {
        prop = false;
    } else {
        const auto property = static_cast< BitProperty >( propId );
        if ( mOutputArchive.creator().storeSymbolicLinks() || property != BitProperty::SymLink ) {
            prop = mOutputArchive.outputItemProperty( index, property );
        }
    }
    *value = prop;
    prop.bstrVal = nullptr;
    return S_OK;
} catch ( const BitException& exception ) {
    return exception.hresultCode();
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetStream( UInt32 index, ISequentialInStream** inStream ) noexcept
try {
    RINOK( finalize() ) //-V3504

    if ( mHandler.fileCallback() ) {
        const BitPropVariant filePath = mOutputArchive.outputItemProperty( index, BitProperty::Path );
        if ( filePath.isString() ) {
            mHandler.fileCallback()( filePath.getString() );
        }
    }

    return mOutputArchive.outputItemStream( index, inStream );
} catch ( const BitException& exception ) {
    setErrorException( exception );
    return exception.hresultCode();
} catch ( const std::system_error& exception ) {
    // exception.what() already embeds its category's message, and BitException's own
    // std::system_error base would append that same message again if reused here, so only the
    // error code is preserved, not the original message.
    setErrorException( "Failed to get the stream", exception.code() );
    return E_ABORT;
} catch ( const std::bad_alloc& ) {
    // Avoid allocating while already handling an out-of-memory condition: unlike toBitException()
    // (which needs a new string + BitException), current_exception() only bumps a refcount on the
    // exception object the runtime already allocated when it was thrown.
    setErrorException( std::current_exception() );
    return E_OUTOFMEMORY;
} catch ( const std::exception& exception ) {
    setErrorException( toBitException( "Failed to get the stream", exception ) );
    return E_ABORT;
} catch ( ... ) {
    // E.g., a user-provided FileCallback threw an exception not derived from std::exception;
    // the exception must not escape this noexcept COM method, so we store it for compressOut() to rethrow to the user.
    setErrorException( std::current_exception() );
    return E_ABORT;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetVolumeSize( UInt32 /*index*/, UInt64* /*size*/ ) noexcept {
    return S_FALSE;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetVolumeStream( UInt32 index, ISequentialOutStream** volumeStream ) noexcept {
    tstring res = to_tstring( index + 1 );
    if ( res.length() < 3 ) {
        // Adding leading zeros for a total res length of 3 (e.g., volume 42 will have the extension .042)
        res.insert( res.begin(), 3 - res.length(), BIT7Z_STRING( '0' ) );
    }

    const tstring fileName = BIT7Z_STRING( '.' ) + res; // + mVolExt;

    try {
        auto stream = bit7z::make_com< CFileOutStream >( fileName );
        *volumeStream = stream.Detach();
    } catch ( const BitException& exception ) {
        return exception.nativeCode();
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetUpdateItemInfo(
    UInt32 index,
    Int32* newData,
    Int32* newProperties,
    UInt32* indexInArchive
) noexcept {
    if ( newData != nullptr ) {
        *newData = static_cast< Int32 >( mOutputArchive.hasNewData( index ) ); //1 = true, 0 = false;
    }
    if ( newProperties != nullptr ) {
        *newProperties = static_cast< Int32 >( mOutputArchive.hasNewProperties( index ) ); //1 = true, 0 = false;
    }
    if ( indexInArchive != nullptr ) {
        *indexInArchive = mOutputArchive.indexInArchive( index );
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetOperationResult( Int32 /* operationResult */ ) noexcept {
    mNeedBeClosed = true;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) noexcept {
    *passwordIsDefined = ( mHandler.isPasswordDefined() ? 1 : 0 );
    return StringToBstr( WIDEN( mHandler.password() ).c_str(), password );
}

} // namespace bit7z

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

#include "internal/opencallback.hpp"

#include "bitexception.hpp"
#include "internal/cfileinstream.hpp"
#include "internal/util.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

OpenCallback::OpenCallback( const BitAbstractArchiveHandler& handler, const fs::path& filename )
    : Callback( handler ), mSubArchiveMode( false ), mFileItem( filename ) {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::SetTotal( const UInt64* /* files */, const UInt64* /* bytes */ ) noexcept {
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::SetCompleted( const UInt64* /* files */, const UInt64* /* bytes */ ) noexcept {
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::GetProperty( PROPID propID, PROPVARIANT* value ) {
    BitPropVariant prop;
    if ( mSubArchiveMode ) {
        if ( propID == kpidName ) {
            prop = mSubArchiveName;
            // case kpidSize: prop = _subArchiveSize; break; // we don't use it for now.
        }
    } else {
        switch ( propID ) {
            case kpidName:
                prop = fs::path( mFileItem.name() ).wstring();
                break;
            case kpidIsDir:
                prop = mFileItem.isDir();
                break;
            case kpidSize:
                prop = mFileItem.size();
                break;
            case kpidAttrib:
                prop = mFileItem.attributes();
                break;
            case kpidCTime:
                prop = mFileItem.creationTime();
                break;
            case kpidATime:
                prop = mFileItem.lastAccessTime();
                break;
            case kpidMTime:
                prop = mFileItem.lastWriteTime();
                break;
            default: //prop is empty
                break;
        }
    }
    *value = prop;
    prop.bstrVal = nullptr;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::GetStream( const wchar_t* name, IInStream** inStream ) {
    try {
        *inStream = nullptr;
        if ( mSubArchiveMode ) {
            return S_FALSE;
        }
        if ( mFileItem.isDir() ) {
            return S_FALSE;
        }
        auto stream_path = fs::path{ mFileItem.path() };
        if ( name != nullptr ) {
            stream_path = stream_path.parent_path();
            stream_path.append( name );
            const auto stream_status = fs::status( stream_path );
            if ( !fs::exists( stream_status ) || fs::is_directory( stream_status ) ) {  // avoid exceptions using status
                return S_FALSE;
            }
        }

        try {
            auto inStreamTemp = bit7z::make_com< CFileInStream >( stream_path );
            *inStream = inStreamTemp.Detach();
        } catch ( const BitException& ex ) {
            return ex.nativeCode();
        }
        return S_OK;
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::SetSubArchiveName( const wchar_t* name ) {
    mSubArchiveMode = true;
    try {
        mSubArchiveName = name;
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::CryptoGetTextPassword( BSTR* password ) {
    std::wstring pass;
    if ( !mHandler.isPasswordDefined() ) {
        if ( mHandler.passwordCallback() ) {
            pass = WIDEN( mHandler.passwordCallback()() );
        }

        if ( pass.empty() ) {
            return E_ABORT;
        }
    } else {
        pass = WIDEN( mHandler.password() );
    }

    return StringToBstr( pass.c_str(), password );
}

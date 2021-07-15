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

#include "../include/opencallback.hpp"

#include "7zip/Common/FileStreams.h"

#include "../include/bitpropvariant.hpp"
#include "../include/fsutil.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

/* Most of this code is taken from the COpenCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of wstring instead of UString (see Callback base interface)
 *  + Error messages are not showed (see comments in ExtractCallback) */

OpenCallback::OpenCallback( const BitArchiveHandler& handler, const wstring& filename )
    : Callback( handler ), mSubArchiveMode( false ), mSubArchiveName( L"" ), mFileItem( filename ) {}

OpenCallback::~OpenCallback() {}

STDMETHODIMP OpenCallback::SetTotal( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP OpenCallback::SetCompleted( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP OpenCallback::GetProperty( PROPID propID, PROPVARIANT* value ) {
    BitPropVariant prop;
    if ( mSubArchiveMode ) {
        switch ( propID ) {
            case kpidName:
                prop = mSubArchiveName;
                break;
                // case kpidSize:  prop = _subArchiveSize; break; // we don't use it now
        }
    } else {
        switch ( propID ) {
            case kpidName:
                prop = mFileItem.name();
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
        }
    }
    *value = prop;
    prop.bstrVal = nullptr;
    return S_OK;
}

STDMETHODIMP OpenCallback::GetStream( const wchar_t* name, IInStream** inStream ) {
    try {
        *inStream = nullptr;
        if ( mSubArchiveMode ) {
            return S_FALSE;
        }
        if ( mFileItem.isDir() ) {
            return S_FALSE;
        }
        wstring stream_path = mFileItem.path();
        if ( name != nullptr ) {
            stream_path = fsutil::dirname( stream_path ) + WCHAR_PATH_SEPARATOR + name;
            if ( !fsutil::pathExists( stream_path ) || fsutil::isDirectory( stream_path ) ) {
                return S_FALSE;
            }
        }
        auto* inFile = new CInFileStream;
        CMyComPtr< IInStream > inStreamTemp = inFile;
        if ( !inFile->Open( stream_path.c_str() ) ) {
            DWORD last_error = ::GetLastError();
            return ( last_error == 0 ) ? E_FAIL : HRESULT_FROM_WIN32( last_error );
        }
        *inStream = inStreamTemp.Detach();
        return S_OK;
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }
}

STDMETHODIMP OpenCallback::SetSubArchiveName( const wchar_t* name ) {
    mSubArchiveMode = true;
    mSubArchiveName = name;
    return S_OK;
}

STDMETHODIMP OpenCallback::CryptoGetTextPassword( BSTR* password ) {
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
            return E_ABORT;
        }
    } else {
        pass = mHandler.password();
    }

    return StringToBstr( pass.c_str(), password );
}

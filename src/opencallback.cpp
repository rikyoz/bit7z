#include "../include/opencallback.hpp"

#include "../include/fsutil.hpp"

#include <iostream>

#include "7zip/Common/FileStreams.h"
#include "Windows/PropVariant.h"

using namespace std;
using namespace bit7z;
using namespace bit7z::filesystem;

/* Most of this code is taken from the COpenCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of wstring instead of UString (see Callback base interface)
 *  + Error messages are not showed (see comments in ExtractCallback) */

OpenCallback::OpenCallback( const BitArchiveOpener& opener, const wstring &filename )
    : mOpener( opener ), mSubArchiveMode( false ), mSubArchiveName( L"" ), mFileItem( filename ) {}

OpenCallback::~OpenCallback() {}

STDMETHODIMP OpenCallback::SetTotal( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP OpenCallback::SetCompleted( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP OpenCallback::GetProperty( PROPID propID, PROPVARIANT* value ) {
    NWindows::NCOM::CPropVariant prop;
    if ( mSubArchiveMode ) {
        switch ( propID ) {
            case kpidName:
                prop = mSubArchiveName.c_str();
                break;
                // case kpidSize:  prop = _subArchiveSize; break; // we don't use it now
        }
    } else {
        switch ( propID ) {
            case kpidName:
                prop = mFileItem.name().c_str();
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
    prop.Detach( value );
    return S_OK;
}

STDMETHODIMP OpenCallback::GetStream( const wchar_t* name, IInStream** inStream ) {
    try {
        *inStream = NULL;
        if ( mSubArchiveMode ) {
            return S_FALSE;
        }
        wstring fullPath = mFileItem.upDirectory();
        if ( !fullPath.empty() ) {
            fullPath += L"\\";
        }
        fullPath += name;
        if ( !fsutil::path_exists( fullPath ) || fsutil::is_directory( fullPath ) ) {
            return S_FALSE;
        }
        CInFileStream* inFile = new CInFileStream;
        CMyComPtr< IInStream > inStreamTemp = inFile;
        if ( !inFile->Open( fullPath.c_str() ) ) {
            return ::GetLastError();
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
    if ( !mOpener.isPasswordDefined() ) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        if ( mOpener.passwordCallback() ) {
            pass = mOpener.passwordCallback()();
        }

        if ( pass.empty() ){
            mErrorMessage = L"Password is not defined";
            return E_ABORT;
        }
    } else {
        pass = mOpener.password();
    }

    return StringToBstr( pass.c_str(), password );
}

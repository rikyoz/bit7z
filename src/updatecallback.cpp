#include "../include/updatecallback.hpp"

#include <iostream>
#include <string>

#include "7zip/Common/FileStreams.h"
#include "Common/IntToString.h"
#include "Windows/PropVariant.h"

using namespace std;
using namespace bit7z;

/* Most of this code is taken from the CUpdateCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of std::vector instead of CRecordVector, CObjectVector and UStringVector
 *  + Use of std::wstring instead of UString (see Callback base interface)
 *  + Error messages are not showed (see comments in ExtractCallback)
 *  + The work performed originally by the Init method is now performed by the class constructor
 *  + FSItem class is used instead of CDirItem struct */

const std::wstring kEmptyFileAlias = L"[Content]";

UpdateCallback::UpdateCallback( const BitArchiveCreator& creator, const vector< FSItem >& dirItems ) :
    mVolSize( 0 ),
    mDirItems( dirItems ),
    mCreator( creator ),
    mAskPassword( false ) {
    mNeedBeClosed = false;
    mFailedFiles.clear();
    mFailedCodes.clear();
}

UpdateCallback::~UpdateCallback() {
    Finilize();
}

HRESULT UpdateCallback::SetTotal( UInt64 size ) {
    if ( mCreator.totalCallback() ) {
        mCreator.totalCallback()( size );
    }
    return S_OK;
}

HRESULT UpdateCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mCreator.progressCallback() && completeValue != nullptr ) {
        mCreator.progressCallback()( *completeValue );
    }
    return S_OK;
}

STDMETHODIMP UpdateCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) {
    if ( mCreator.ratioCallback() && inSize != nullptr && outSize != nullptr ) {
        mCreator.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

HRESULT UpdateCallback::EnumProperties( IEnumSTATPROPSTG** /* enumerator */ ) {
    return E_NOTIMPL;
}

HRESULT UpdateCallback::GetUpdateItemInfo( UInt32 /* index */, Int32* newData,
                                           Int32* newProperties, UInt32* indexInArchive ) {
    if ( newData != NULL ) {
        *newData = 1; //= true;
    }
    if ( newProperties != NULL ) {
        *newProperties = 1; //= true;
    }
    if ( indexInArchive != NULL ) {
        *indexInArchive = static_cast< UInt32 >( -1 );
    }

    return S_OK;
}

HRESULT UpdateCallback::GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value ) {
    NWindows::NCOM::CPropVariant prop;

    if ( propID == kpidIsAnti ) {
        prop = false;
        prop.Detach( value );
        return S_OK;
    }

    const FSItem dirItem = mDirItems[index];

    switch ( propID ) {
        case kpidPath: prop = dirItem.relativePath().c_str();
            break;
        case kpidIsDir: prop = dirItem.isDir();
            break;
        case kpidSize: prop = dirItem.size();
            break;
        case kpidAttrib: prop = dirItem.attributes();
            break;
        case kpidCTime: prop = dirItem.creationTime();
            break;
        case kpidATime: prop = dirItem.lastAccessTime();
            break;
        case kpidMTime: prop = dirItem.lastWriteTime();
            break;
    }

    prop.Detach( value );
    return S_OK;
}

HRESULT UpdateCallback::Finilize() {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

HRESULT UpdateCallback::GetStream( UInt32 index, ISequentialInStream** inStream ) {
    RINOK( Finilize() );
    const FSItem dirItem = mDirItems[index];

    if ( mCreator.fileCallback() ) {
        mCreator.fileCallback()( dirItem.name() );
    }

    if ( dirItem.isDir() ) {
        return S_OK;
    }

    CInFileStream* inStreamSpec = new CInFileStream;
    CMyComPtr< ISequentialInStream > inStreamLoc( inStreamSpec );
    wstring path = dirItem.fullPath();

    if ( !inStreamSpec->Open( path.c_str() ) ) {
        DWORD sysError = ::GetLastError();
        mFailedCodes.push_back( sysError );
        mFailedFiles.push_back( path );
        // if (systemError == ERROR_SHARING_VIOLATION)
        {
            mErrorMessage = L"WARNING: Can't open file";
            // PrintString(NError::MyFormatMessageW(systemError));
            return S_FALSE;
        }
        // return sysError;
    }

    *inStream = inStreamLoc.Detach();
    return S_OK;
}

HRESULT UpdateCallback::SetOperationResult( Int32 /* operationResult */ ) {
    mNeedBeClosed = true;
    return S_OK;
}

HRESULT UpdateCallback::GetVolumeSize( UInt32 /*index*/, UInt64* size ) {
    if ( mVolSize == 0 ) return S_FALSE;

    *size = mVolSize;
    return S_OK;
}

HRESULT UpdateCallback::GetVolumeStream( UInt32 index, ISequentialOutStream** volumeStream ) {
    wchar_t temp[16];
    ConvertUInt32ToString( index + 1, temp );
    wstring res = temp;

    while ( res.length() < 2 ) {
        res = L'0' + res;
    }

    wstring fileName = mVolName + L'.' + res + mVolExt;
    COutFileStream* streamSpec = new COutFileStream;
    CMyComPtr< ISequentialOutStream > streamLoc( streamSpec );

    if ( !streamSpec->Create( fileName.c_str(), false ) ) {
        return ::GetLastError();
    }

    *volumeStream = streamLoc.Detach();
    return S_OK;
}

HRESULT UpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
    if ( !mCreator.isPasswordDefined() ) {
        if ( mAskPassword ) {
            // You can ask real password here from user
            // Password = GetPassword(OutStream);
            // PasswordIsDefined = true;
            mErrorMessage = L"Password is not defined";
            return E_ABORT;
        }
    }

    *passwordIsDefined = ( mCreator.isPasswordDefined() ? 1 : 0 );
    return StringToBstr( mCreator.password().c_str(), password );
}

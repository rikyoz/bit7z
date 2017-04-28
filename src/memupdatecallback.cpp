#include "../include/memupdatecallback.hpp"

#include <iostream>
#include <string>

#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamObjects.h"
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

MemUpdateCallback::MemUpdateCallback( const BitArchiveCreator& creator, const vector< byte_t >& out_buffer, const wstring& buffer_name ) :
    mCreator( creator ),
    mAskPassword( false ),
    mNeedBeClosed( false ),
    mBuffer( out_buffer ),
    mBufferName( buffer_name ) {
    mFailedFiles.clear();
    mFailedCodes.clear();
}

MemUpdateCallback::~MemUpdateCallback() {
    Finilize();
}

HRESULT MemUpdateCallback::SetTotal( UInt64 size ) {
    if ( mCreator.totalCallback() ) {
        mCreator.totalCallback()( size );
    }
    return S_OK;
}

HRESULT MemUpdateCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mCreator.progressCallback() ) {
        mCreator.progressCallback()( *completeValue );
    }
    return S_OK;
}

HRESULT MemUpdateCallback::EnumProperties( IEnumSTATPROPSTG** /* enumerator */ ) {
    return E_NOTIMPL;
}

HRESULT MemUpdateCallback::GetUpdateItemInfo( UInt32 /* index */, Int32* newData,
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

HRESULT MemUpdateCallback::GetProperty( UInt32 /*index*/, PROPID propID, PROPVARIANT* value ) {
    NWindows::NCOM::CPropVariant prop;

    if ( propID == kpidIsAnti ) {
        prop = false;
        prop.Detach( value );
        return S_OK;
    }

    FILETIME ft;
    SYSTEMTIME st;

    GetSystemTime( &st ); // gets current time
    SystemTimeToFileTime( &st, &ft ); // converts to file time format

    switch ( propID ) {
        case kpidPath: prop = ( mBufferName.empty() ) ? kEmptyFileAlias.c_str() : mBufferName.c_str();
            break;
        case kpidIsDir: prop = false;
            break;
        case kpidSize: {
            prop.vt = VT_UI8;
            prop.uhVal.QuadPart = ( sizeof( byte_t ) * mBuffer.size() );
            break;
        }
        case kpidAttrib: {
            prop.vt = VT_UI4;
            prop.ulVal = FILE_ATTRIBUTE_NORMAL;
            break;
        }
        case kpidCTime: prop = ft;
            break;
        case kpidATime: prop = ft;
            break;
        case kpidMTime: prop = ft;
            break;
    }

    prop.Detach( value );
    /*NWindows::NCOM::CPropVariant prop;

       if ( propID == kpidIsAnti ) {
        prop = false;
        prop.Detach( value );
        return S_OK;
       }

       const FSItem dirItem = mDirItems[index];

       switch ( propID ) {
        case kpidPath  : prop = dirItem.relativePath().c_str(); break;
        case kpidIsDir : prop = dirItem.isDir(); break;
        case kpidSize  : prop = dirItem.size(); break;
        case kpidAttrib: prop = dirItem.attributes(); break;
        case kpidCTime : prop = dirItem.creationTime(); break;
        case kpidATime : prop = dirItem.lastAccessTime(); break;
        case kpidMTime : prop = dirItem.lastWriteTime(); break;
       }

       prop.Detach( value );*/
    return S_OK;
}

HRESULT MemUpdateCallback::Finilize() {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

HRESULT MemUpdateCallback::GetStream( UInt32 /*index*/, ISequentialInStream** inStream ) {
    RINOK( Finilize() );
    /*const FSItem dirItem = mDirItems[index];

       if ( dirItem.isDir() )
        return S_OK;*/

    CBufInStream* inStreamSpec = new CBufInStream;
    CMyComPtr< ISequentialInStream > inStreamLoc( inStreamSpec );
    inStreamSpec->Init( &mBuffer[0], mBuffer.size() );


//    wstring path = dirItem.fullPath();

//    if ( !inStreamSpec->Open( path.c_str() ) ) {
//        DWORD sysError = ::GetLastError();
//        mFailedCodes.push_back( sysError );
//        mFailedFiles.push_back( path );
//        // if (systemError == ERROR_SHARING_VIOLATION)
//        mErrorMessage = L"WARNING: Can't open file";
//        // PrintString(NError::MyFormatMessageW(systemError));
//        return S_FALSE;
//        // return sysError;
//    }

    *inStream = inStreamLoc.Detach();
    return S_OK;
}

HRESULT MemUpdateCallback::SetOperationResult( Int32 /* operationResult */ ) {
    mNeedBeClosed = true;
    return S_OK;
}

HRESULT MemUpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
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

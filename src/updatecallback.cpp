#include "../include/updatecallback.hpp"

#include <iostream>

#include "Common/IntToString.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"

using namespace std;
using namespace Bit7z;

const std::wstring kEmptyFileAlias = L"[Content]";

HRESULT UpdateCallback::SetTotal( UInt64 /* size */ ) {
    return S_OK;
}

HRESULT UpdateCallback::SetCompleted( const UInt64* /* completeValue */ ) {
    return S_OK;
}

HRESULT UpdateCallback::EnumProperties( IEnumSTATPROPSTG** /* enumerator */ ) {
    return E_NOTIMPL;
}

HRESULT UpdateCallback::GetUpdateItemInfo( UInt32 /* index */, Int32* newData,
                                                Int32* newProperties, UInt32* indexInArchive ) {
    if ( newData != NULL )
        *newData = BoolToInt( true );

    if ( newProperties != NULL )
        *newProperties = BoolToInt( true );

    if ( indexInArchive != NULL )
        *indexInArchive = ( UInt32 ) - 1;

    return S_OK;
}

HRESULT UpdateCallback::GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value ) {
    NWindows::NCOM::CPropVariant prop;

    if ( propID == kpidIsAnti ) {
        prop = false;
        prop.Detach( value );
        return S_OK;
    }

    {
        const CDirItem& dirItem = ( *mDirItems )[index];

        switch ( propID ) {
            case kpidPath:
                prop = dirItem.Name;
                break;

            case kpidIsDir:
                prop = dirItem.isDir();
                break;

            case kpidSize:
                prop = dirItem.Size;
                break;

            case kpidAttrib:
                prop = dirItem.Attrib;
                break;

            case kpidCTime:
                prop = dirItem.CTime;
                break;

            case kpidATime:
                prop = dirItem.ATime;
                break;

            case kpidMTime:
                prop = dirItem.MTime;
                break;
        }
    }

    prop.Detach( value );
    return S_OK;
}

HRESULT UpdateCallback::Finilize() {
    if ( mNeedBeClosed ) {
        cout << endl;
        mNeedBeClosed = false;
    }

    return S_OK;
}

static void GetStream2( const wchar_t* name ) {
    cout << "Compressing  ";

    if ( name[0] == 0 )
        name = kEmptyFileAlias.c_str();

    wcout << name;
}

HRESULT UpdateCallback::GetStream( UInt32 index, ISequentialInStream** inStream ) {
    RINOK( Finilize() );
    const CDirItem& dirItem = ( *mDirItems )[index];
    GetStream2( dirItem.Name );

    if ( dirItem.isDir() )
        return S_OK;

    {
        CInFileStream* inStreamSpec = new CInFileStream;
        CMyComPtr<ISequentialInStream> inStreamLoc( inStreamSpec );
        UString path = mDirPrefix + dirItem.FullPath;

        if ( !inStreamSpec->Open( path ) ) {
            DWORD sysError = ::GetLastError();
            mFailedCodes.Add( sysError );
            mFailedFiles.Add( path );
            // if (systemError == ERROR_SHARING_VIOLATION)
            {
                cerr << endl << "WARNING: can't open file";
                // PrintString(NError::MyFormatMessageW(systemError));
                return S_FALSE;
            }
            // return sysError;
        }

        *inStream = inStreamLoc.Detach();
    }
    return S_OK;
}

HRESULT UpdateCallback::SetOperationResult( Int32 /* operationResult */ ) {
    mNeedBeClosed = true;
    return S_OK;
}

HRESULT UpdateCallback::GetVolumeSize( UInt32 index, UInt64* size ) {
    if ( mVolumesSizes.Size() == 0 )
        return S_FALSE;

    if ( index >= ( UInt32 )mVolumesSizes.Size() )
        index = mVolumesSizes.Size() - 1;

    *size = mVolumesSizes[index];
    return S_OK;
}

HRESULT UpdateCallback::GetVolumeStream( UInt32 index, ISequentialOutStream** volumeStream ) {
    wchar_t temp[16];
    ConvertUInt32ToString( index + 1, temp );
    UString res = temp;

    while ( res.Length() < 2 )
        res = UString( L'0' ) + res;

    UString fileName = mVolName;
    fileName += L'.';
    fileName += res;
    fileName += mVolExt;
    COutFileStream* streamSpec = new COutFileStream;
    CMyComPtr<ISequentialOutStream> streamLoc( streamSpec );

    if ( !streamSpec->Create( fileName, false ) )
        return ::GetLastError();

    *volumeStream = streamLoc.Detach();
    return S_OK;
}

HRESULT UpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
    if ( !mIsPasswordDefined ) {
        if ( mAskPassword ) {
            // You can ask real password here from user
            // Password = GetPassword(OutStream);
            // PasswordIsDefined = true;
            cerr << "Password is not defined";
            return E_ABORT;
        }
    }

    *passwordIsDefined = BoolToInt( mIsPasswordDefined );
    return StringToBstr( mPassword, password );
}

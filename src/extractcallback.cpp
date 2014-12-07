#include "../include/extractcallback.hpp"

//#include "Common/StringConvert.h"
#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
#include "Windows/PropVariant.h"
//#include "Windows/PropVariantConversions.h"

#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

using namespace std;
using namespace NWindows;
using namespace Bit7z;

static const wstring kCantDeleteOutputFile = L"Cannot delete output file ";

static const wstring kTestingString    =  L"Testing     ";
static const wstring kExtractingString =  L"Extracting  ";
static const wstring kSkippingString   =  L"Skipping    ";

static const wstring kUnsupportedMethod = L"Unsupported Method";
static const wstring kCRCFailed         = L"CRC Failed";
static const wstring kDataError         = L"Data Error";
static const wstring kUnknownError      = L"Unknown Error";
static const wstring kEmptyFileAlias    = L"[Content]";

static HRESULT IsArchiveItemProp( IInArchive* archive, UInt32 index, PROPID propID, bool& result ) {
    NCOM::CPropVariant prop;
    RINOK( archive->GetProperty( index, propID, &prop ) );

    if ( prop.vt == VT_BOOL )
        result = VARIANT_BOOLToBool( prop.boolVal );
    else if ( prop.vt == VT_EMPTY )
        result = false;
    else
        return E_FAIL;

    return S_OK;
}

static HRESULT IsArchiveItemFolder( IInArchive* archive, UInt32 index, bool& result ) {
    return IsArchiveItemProp( archive, index, kpidIsDir, result );
}

ExtractCallback::ExtractCallback( IInArchive* archiveHandler, const wstring& directoryPath )
    : mPassword( L"" ), mNumErrors( 0 ), mArchiveHandler( archiveHandler ),
      mDirectoryPath( directoryPath ) {
    //NFile::NName::NormalizeDirPathPrefix( mDirectoryPath );
    FileSystem::FSUtil::normalize_path( mDirectoryPath );
}

void ExtractCallback::setPassword( const wstring& password ) {
    mPassword = password;
}

STDMETHODIMP ExtractCallback::SetTotal( UInt64 /* size */ ) {
    return S_OK;
}

STDMETHODIMP ExtractCallback::SetCompleted( const UInt64* /* completeValue */ ) {
    return S_OK;
}

STDMETHODIMP ExtractCallback::GetStream( UInt32 index,
                                         ISequentialOutStream** outStream,
                                         Int32 askExtractMode ) {
    *outStream = 0;
    mOutFileStream.Release();
    // Get Name
    NCOM::CPropVariant prop;
    RINOK( mArchiveHandler->GetProperty( index, kpidPath, &prop ) );
    wstring fullPath;

    if ( prop.vt == VT_EMPTY )
        fullPath = kEmptyFileAlias;
    else {
        if ( prop.vt != VT_BSTR )
            return E_FAIL;

        fullPath = prop.bstrVal;
    }

    mFilePath = fullPath;

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract )
        return S_OK;


    // Get Attrib
    NCOM::CPropVariant prop2;
    RINOK( mArchiveHandler->GetProperty( index, kpidAttrib, &prop2 ) );

    if ( prop2.vt == VT_EMPTY ) {
        _processedFileInfo.Attrib = 0;
        _processedFileInfo.AttribDefined = false;
    } else {
        if ( prop2.vt != VT_UI4 )
            return E_FAIL;

        _processedFileInfo.Attrib = prop2.ulVal;
        _processedFileInfo.AttribDefined = true;
    }

    RINOK( IsArchiveItemFolder( mArchiveHandler, index, _processedFileInfo.isDir ) );
    // Get Modified Time
    NCOM::CPropVariant prop3;
    RINOK( mArchiveHandler->GetProperty( index, kpidMTime, &prop3 ) );
    _processedFileInfo.MTimeDefined = false;

    switch ( prop3.vt ) {
        case VT_EMPTY:
            // _processedFileInfo.MTime = _utcMTimeDefault;
            break;

        case VT_FILETIME:
            _processedFileInfo.MTime = prop3.filetime;
            _processedFileInfo.MTimeDefined = true;
            break;

        default:
            return E_FAIL;
    }

    // Get Size
    NCOM::CPropVariant prop4;
    RINOK( mArchiveHandler->GetProperty( index, kpidSize, &prop4 ) );
    bool newFileSizeDefined = ( prop4.vt != VT_EMPTY );
    UInt64 newFileSize;

    if ( newFileSizeDefined ) {
        //taken from ConvertPropVariantToUInt64
        switch ( prop4.vt ) {
            case VT_UI1: newFileSize = prop4.bVal; break;
            case VT_UI2: newFileSize = prop4.uiVal; break;
            case VT_UI4: newFileSize = prop4.ulVal; break;
            case VT_UI8: newFileSize = ( UInt64 )prop4.uhVal.QuadPart; break;
            default:
                mErrorMessage = L"151199";
                return E_FAIL;
        }

        //newFileSize = ConvertPropVariantToUInt64( prop4 );
    }


// Create folders for file
    size_t slashPos = mFilePath.rfind( WSTRING_PATH_SEPARATOR );

    if ( slashPos >= 0 && slashPos != wstring::npos )
        NFile::NDirectory::CreateComplexDirectory( ( mDirectoryPath + mFilePath.substr( 0,
                                                     slashPos ) ).c_str() );
    wstring fullProcessedPath = mDirectoryPath + mFilePath;
    mDiskFilePath = fullProcessedPath;

    if ( _processedFileInfo.isDir )
        NFile::NDirectory::CreateComplexDirectory( fullProcessedPath.c_str() );
    else {
        NFile::NFind::CFileInfoW fi;

        if ( fi.Find( fullProcessedPath.c_str() ) ) {
            if ( !NFile::NDirectory::DeleteFileAlways( fullProcessedPath.c_str() ) ) {
                //cerr << UString( kCantDeleteOutputFile ) << fullProcessedPath << endl;
                //throw BitException( kCantDeleteOutputFile + fullProcessedPath );
                mErrorMessage = kCantDeleteOutputFile + fullProcessedPath;
                return E_ABORT;
            }
        }

        mOutFileStreamSpec = new COutFileStream;
        CMyComPtr<ISequentialOutStream> outStreamLoc( mOutFileStreamSpec );

        if ( !mOutFileStreamSpec->Open( fullProcessedPath.c_str(), CREATE_ALWAYS ) ) {
            //cerr <<  ( UString )L"cannot open output file " + fullProcessedPath << endl;
            //throw BitException( L"cannot open output file " + fullProcessedPath );
            mErrorMessage = L"Cannot open output file " + fullProcessedPath;
            return E_ABORT;
        }

        mOutFileStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }

    return S_OK;
}

STDMETHODIMP ExtractCallback::PrepareOperation( Int32 askExtractMode ) {
    mExtractMode = false;

    // in future we might use this switch to handle an event like onOperationStart(Operation o)
    // with enum Operation{Extract, Test, Skip}

    switch ( askExtractMode ) {
        case NArchive::NExtract::NAskMode::kExtract:
            mExtractMode = true;
            //cout <<  kExtractingString;
            break;

            /*case NArchive::NExtract::NAskMode::kTest:
                cout <<  kTestingString;
                break;

            case NArchive::NExtract::NAskMode::kSkip:
                cout <<  kSkippingString;
                break;*/
    };

    //wcout << mFilePath << endl;;
    return S_OK;
}

STDMETHODIMP ExtractCallback::SetOperationResult( Int32 operationResult ) {
    switch ( operationResult ) {
        case NArchive::NExtract::NOperationResult::kOK:
            break;

        default: {
            mNumErrors++;

            //cout <<  "     ";

            switch ( operationResult ) {
                case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
                    mErrorMessage = kUnsupportedMethod;
                    break;

                case NArchive::NExtract::NOperationResult::kCRCError:
                    mErrorMessage = kCRCFailed;
                    break;

                case NArchive::NExtract::NOperationResult::kDataError:
                    mErrorMessage = kDataError;
                    break;

                default:
                    mErrorMessage = kUnknownError;
            }
        }
    }

    if ( mOutFileStream != NULL ) {
        if ( _processedFileInfo.MTimeDefined )
            mOutFileStreamSpec->SetMTime( &_processedFileInfo.MTime );

        RINOK( mOutFileStreamSpec->Close() );
    }

    mOutFileStream.Release();

    if ( mExtractMode && _processedFileInfo.AttribDefined )
        NFile::NDirectory::MySetFileAttributes( mDiskFilePath.c_str(), _processedFileInfo.Attrib );

    if ( mNumErrors > 0 ) return E_FAIL;
        //throw BitException( mErrorString );

    //cout << endl;
    return S_OK;
}


STDMETHODIMP ExtractCallback::CryptoGetTextPassword( BSTR* password ) {
    if ( mPassword.length() == 0 ) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        //in future, no exception but an event (i.e. onPasswordRequest) call
        mErrorMessage = L"Password is not defined";
        return E_FAIL;
    }

    return StringToBstr( mPassword.c_str(), password );
}

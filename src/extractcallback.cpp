#include "../include/extractcallback.hpp"

#include "Common/StringConvert.h"
#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"

#include "../include/bitexception.hpp"

using namespace std;
using namespace NWindows;
using namespace Bit7z;

static const UString kCantDeleteOutputFile = L"ERROR: Cannot delete output file ";

static const char* kTestingString    =  "Testing     ";
static const char* kExtractingString =  "Extracting  ";
static const char* kSkippingString   =  "Skipping    ";

static const char* kUnsupportedMethod = "Unsupported Method";
static const char* kCRCFailed = "CRC Failed";
static const char* kDataError = "Data Error";
static const char* kUnknownError = "Unknown Error";
const wstring kEmptyFileAlias = L"[Content]";

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

ExtractCallback::ExtractCallback( IInArchive* archiveHandler, const UString& directoryPath )
    : hasPassword( false ), numErrors( 0 ), _archiveHandler( archiveHandler ),
      _directoryPath( directoryPath ) {
    NFile::NName::NormalizeDirPathPrefix( _directoryPath );
}

void ExtractCallback::setPassword(const UString &password) {
    this->password = password;
    this->hasPassword = password.Length() > 0;
}

HRESULT ExtractCallback::SetTotal( UInt64 /* size */ ) {
    return S_OK;
}

HRESULT ExtractCallback::SetCompleted( const UInt64* /* completeValue */ ) {
    return S_OK;
}

HRESULT ExtractCallback::GetStream( UInt32 index,
                                    ISequentialOutStream** outStream,
                                    Int32 askExtractMode ) {
    *outStream = 0;
    _outFileStream.Release();
    {
        // Get Name
        NCOM::CPropVariant prop;
        RINOK( _archiveHandler->GetProperty( index, kpidPath, &prop ) );
        UString fullPath;

        if ( prop.vt == VT_EMPTY )
            fullPath = kEmptyFileAlias.c_str();
        else {
            if ( prop.vt != VT_BSTR )
                return E_FAIL;

            fullPath = prop.bstrVal;
        }

        _filePath = fullPath;
    }

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract )
        return S_OK;

    {
        // Get Attrib
        NCOM::CPropVariant prop;
        RINOK( _archiveHandler->GetProperty( index, kpidAttrib, &prop ) );

        if ( prop.vt == VT_EMPTY ) {
            _processedFileInfo.Attrib = 0;
            _processedFileInfo.AttribDefined = false;
        } else {
            if ( prop.vt != VT_UI4 )
                return E_FAIL;

            _processedFileInfo.Attrib = prop.ulVal;
            _processedFileInfo.AttribDefined = true;
        }
    }
    RINOK( IsArchiveItemFolder( _archiveHandler, index, _processedFileInfo.isDir ) );
    {
        // Get Modified Time
        NCOM::CPropVariant prop;
        RINOK( _archiveHandler->GetProperty( index, kpidMTime, &prop ) );
        _processedFileInfo.MTimeDefined = false;

        switch ( prop.vt ) {
            case VT_EMPTY:
                // _processedFileInfo.MTime = _utcMTimeDefault;
                break;

            case VT_FILETIME:
                _processedFileInfo.MTime = prop.filetime;
                _processedFileInfo.MTimeDefined = true;
                break;

            default:
                return E_FAIL;
        }
    }
    {
        // Get Size
        NCOM::CPropVariant prop;
        RINOK( _archiveHandler->GetProperty( index, kpidSize, &prop ) );
        bool newFileSizeDefined = ( prop.vt != VT_EMPTY );
        UInt64 newFileSize;

        if ( newFileSizeDefined )
            newFileSize = ConvertPropVariantToUInt64( prop );
    }
    {
        // Create folders for file
        int slashPos = _filePath.ReverseFind( WCHAR_PATH_SEPARATOR );

        if ( slashPos >= 0 )
            NFile::NDirectory::CreateComplexDirectory( _directoryPath + _filePath.Left( slashPos ) );
    }
    UString fullProcessedPath = _directoryPath + _filePath;
    _diskFilePath = fullProcessedPath;

    if ( _processedFileInfo.isDir )
        NFile::NDirectory::CreateComplexDirectory( fullProcessedPath );
    else {
        NFile::NFind::CFileInfoW fi;

        if ( fi.Find( fullProcessedPath ) ) {
            if ( !NFile::NDirectory::DeleteFileAlways( fullProcessedPath ) ) {
                //cerr << UString( kCantDeleteOutputFile ) << fullProcessedPath << endl;
                throw BitException( kCantDeleteOutputFile + fullProcessedPath );
                return E_ABORT;
            }
        }

        _outFileStreamSpec = new COutFileStream;
        CMyComPtr<ISequentialOutStream> outStreamLoc( _outFileStreamSpec );

        if ( !_outFileStreamSpec->Open( fullProcessedPath, CREATE_ALWAYS ) ) {
            //cerr <<  ( UString )L"cannot open output file " + fullProcessedPath << endl;
            throw BitException( ( UString )L"cannot open output file " + fullProcessedPath );
            return E_ABORT;
        }

        _outFileStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }

    return S_OK;
}

HRESULT ExtractCallback::PrepareOperation( Int32 askExtractMode ) {
    _extractMode = false;

    // in future we might use this switch to handle an event like onOperationStart(Operation o)
    // with enum Operation{Extract, Test, Skip}

    switch ( askExtractMode ) {
        case NArchive::NExtract::NAskMode::kExtract:
            _extractMode = true;
            //cout <<  kExtractingString;
            break;

        /*case NArchive::NExtract::NAskMode::kTest:
            cout <<  kTestingString;
            break;

        case NArchive::NExtract::NAskMode::kSkip:
            cout <<  kSkippingString;
            break;*/
    };

    //wcout << GetOemString( _filePath ) << endl;;
    return S_OK;
}

HRESULT ExtractCallback::SetOperationResult( Int32 operationResult ) {
    string errorMessage;

    switch ( operationResult ) {
        case NArchive::NExtract::NOperationResult::kOK:
            break;

        default: {
            numErrors++;

            //cout <<  "     ";

            switch ( operationResult ) {
                case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
                    errorMessage = kUnsupportedMethod;
                    break;

                case NArchive::NExtract::NOperationResult::kCRCError:
                    errorMessage = kCRCFailed;
                    break;

                case NArchive::NExtract::NOperationResult::kDataError:
                    errorMessage = kDataError;
                    break;

                default:
                    errorMessage = kUnknownError;
            }
        }
    }

    if ( _outFileStream != NULL ) {
        if ( _processedFileInfo.MTimeDefined )
            _outFileStreamSpec->SetMTime( &_processedFileInfo.MTime );

        RINOK( _outFileStreamSpec->Close() );
    }

    _outFileStream.Release();

    if ( _extractMode && _processedFileInfo.AttribDefined )
        NFile::NDirectory::MySetFileAttributes( _diskFilePath, _processedFileInfo.Attrib );

    if ( numErrors > 0 )
        throw BitException( errorMessage );

    //cout << endl;
    return S_OK;
}


HRESULT ExtractCallback::CryptoGetTextPassword( BSTR* password ) {
    if ( !hasPassword ) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        //in future, no exception but an event (i.e. onPasswordRequest) call
        throw BitException( "Password is not defined" );
    }

    return StringToBstr( this->password, password );
}

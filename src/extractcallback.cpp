#include "../include/extractcallback.hpp"

#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
#include "Windows/PropVariant.h"

#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

using namespace std;
using namespace NWindows;
using namespace bit7z;

/* Most of this code, though heavily modified, is taken from the CExtractCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of wstring instead of UString
 *  + Error messages are not showed. Instead, they are memorized into a wstring and used by BitExtractor to throw
 *    exceptions (see also Callback interface). Note that this class doesn't throw exceptions, as other classes in bit7,
 *    because it must implement interfaces with nothrow methods.
 *  + The work performed originally by the Init method is now performed by the class constructor */

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

    if ( prop.vt == VT_BOOL ) {
        result = VARIANT_BOOLToBool( prop.boolVal );
    } else if ( prop.vt == VT_EMPTY ) {
        result = false;
    } else {
        return E_FAIL;
    }

    return S_OK;
}

static HRESULT IsArchiveItemFolder( IInArchive* archive, UInt32 index, bool& result ) {
    return IsArchiveItemProp( archive, index, kpidIsDir, result );
}

ExtractCallback::ExtractCallback( const BitArchiveOpener& opener, IInArchive* archiveHandler, const wstring& directoryPath ) :
    mOpener( opener ),
    mArchiveHandler( archiveHandler ),
    mDirectoryPath( directoryPath ),
    mExtractMode( true ),
    mProcessedFileInfo(),
    mOutFileStreamSpec( NULL ),
    mNumErrors( 0 ) {
    //NFile::NName::NormalizeDirPathPrefix( mDirectoryPath );
    filesystem::fsutil::normalize_path( mDirectoryPath );
}

ExtractCallback::~ExtractCallback() {}

STDMETHODIMP ExtractCallback::SetTotal( UInt64 size ) {
    if ( mOpener.totalCallback() ) {
        mOpener.totalCallback()( size );
    }
    return S_OK;
}

STDMETHODIMP ExtractCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mOpener.progressCallback() && completeValue != nullptr ) {
        mOpener.progressCallback()( *completeValue );
    }
    return S_OK;
}

STDMETHODIMP ExtractCallback::SetRatioInfo(const UInt64* inSize, const UInt64* outSize) {
    if ( mOpener.ratioCallback() && inSize != nullptr && outSize != nullptr ) {
        mOpener.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

//TODO: clean and optimize!
STDMETHODIMP ExtractCallback::GetStream( UInt32                 index,
                                         ISequentialOutStream** outStream,
                                         Int32                  askExtractMode ) {
    *outStream = 0;
    mOutFileStream.Release();
    // Get Name
    NCOM::CPropVariant prop;
    RINOK( mArchiveHandler->GetProperty( index, kpidPath, &prop ) );
    wstring fullPath;

    if ( prop.vt == VT_EMPTY ) {
        fullPath = kEmptyFileAlias;
    } else {
        if ( prop.vt != VT_BSTR ) {
            return E_FAIL;
        }

        fullPath = prop.bstrVal;
    }

    mFilePath = fullPath;

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract ) {
        return S_OK;
    }


    // Get Attrib
    NCOM::CPropVariant prop2;
    RINOK( mArchiveHandler->GetProperty( index, kpidAttrib, &prop2 ) );

    if ( prop2.vt == VT_EMPTY ) {
        mProcessedFileInfo.Attrib = 0;
        mProcessedFileInfo.AttribDefined = false;
    } else {
        if ( prop2.vt != VT_UI4 ) {
            return E_FAIL;
        }

        mProcessedFileInfo.Attrib = prop2.ulVal;
        mProcessedFileInfo.AttribDefined = true;
    }

    RINOK( IsArchiveItemFolder( mArchiveHandler, index, mProcessedFileInfo.isDir ) );
    // Get Modified Time
    NCOM::CPropVariant prop3;
    RINOK( mArchiveHandler->GetProperty( index, kpidMTime, &prop3 ) );
    mProcessedFileInfo.MTimeDefined = false;

    switch ( prop3.vt ) {
        case VT_EMPTY:
            // mProcessedFileInfo.MTime = _utcMTimeDefault;
            break;

        case VT_FILETIME:
            mProcessedFileInfo.MTime = prop3.filetime;
            mProcessedFileInfo.MTimeDefined = true;
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
            case VT_UI1: newFileSize = prop4.bVal;
                break;
            case VT_UI2: newFileSize = prop4.uiVal;
                break;
            case VT_UI4: newFileSize = prop4.ulVal;
                break;
            case VT_UI8: newFileSize = ( UInt64 )prop4.uhVal.QuadPart;
                break;
            default:
                mErrorMessage = L"151199";
                return E_FAIL;
        }

        //newFileSize = ConvertPropVariantToUInt64( prop4 );
    }


    // Create folders for file
    size_t slashPos = mFilePath.rfind( WSTRING_PATH_SEPARATOR );

    if ( slashPos != wstring::npos ) {
        NFile::NDir::CreateComplexDir( ( mDirectoryPath + mFilePath.substr( 0,
                                                                                        slashPos ) ).c_str() );
    }
    wstring fullProcessedPath = mDirectoryPath + mFilePath;
    mDiskFilePath = fullProcessedPath;

    if ( mProcessedFileInfo.isDir ) {
        NFile::NDir::CreateComplexDir( fullProcessedPath.c_str() );
    } else {
        NFile::NFind::CFileInfo fi;

        if ( mOpener.fileCallback() ) {
            wstring filename;
            filesystem::fsutil::filename( fullProcessedPath, filename, true );
            mOpener.fileCallback()( filename );
        }

        if ( fi.Find( fullProcessedPath.c_str() ) ) {
            if ( !NFile::NDir::DeleteFileAlways( fullProcessedPath.c_str() ) ) {
                //cerr << UString( kCantDeleteOutputFile ) << fullProcessedPath << endl;
                //throw BitException( kCantDeleteOutputFile + fullProcessedPath );
                mErrorMessage = kCantDeleteOutputFile + fullProcessedPath;
                return E_ABORT;
            }
        }

        mOutFileStreamSpec = new COutFileStream;
        CMyComPtr< ISequentialOutStream > outStreamLoc( mOutFileStreamSpec );

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
            //wcout <<  kExtractingString;
            break;

            /*case NArchive::NExtract::NAskMode::kTest:
                cout <<  kTestingString;
                break;

               case NArchive::NExtract::NAskMode::kSkip:
                cout <<  kSkippingString;
                break;*/
    }

    //wcout << mFilePath << endl;;
    return S_OK;
}

STDMETHODIMP ExtractCallback::SetOperationResult( Int32 operationResult ) {
    switch ( operationResult ) {
        case NArchive::NExtract::NOperationResult::kOK:
            break;

        default: {
            mNumErrors++;

            switch ( operationResult ) {
                case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
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
        if ( mProcessedFileInfo.MTimeDefined ) {
            mOutFileStreamSpec->SetMTime( &mProcessedFileInfo.MTime );
        }

        RINOK( mOutFileStreamSpec->Close() );
    }

    mOutFileStream.Release();

    if ( mExtractMode && mProcessedFileInfo.AttribDefined ) {
        NFile::NDir::SetFileAttrib( mDiskFilePath.c_str(), mProcessedFileInfo.Attrib );
    }

    if ( mNumErrors > 0 ) {
        return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP ExtractCallback::CryptoGetTextPassword( BSTR* password ) {
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
            return E_FAIL;
        }
    } else {
        pass = mOpener.password();
    }

    return StringToBstr( pass.c_str(), password );
}

#include "../include/memextractcallback.hpp"

#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
#include "Windows/PropVariant.h"
#include "7zip/Common/StreamObjects.h"

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

//static const wstring kCantDeleteOutputFile = L"Cannot delete output file ";

//static const wstring kTestingString    =  L"Testing     ";
//static const wstring kExtractingString =  L"Extracting  ";
//static const wstring kSkippingString   =  L"Skipping    ";

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

MemExtractCallback::MemExtractCallback( const BitArchiveOpener& opener, IInArchive* archiveHandler, vector< byte_t >& buffer ) :
    mOpener( opener ),
    mArchiveHandler( archiveHandler ),
    mBuffer( buffer ),
    mExtractMode( true ),
    mProcessedFileInfo(),
    mOutMemStreamSpec( NULL ),
    mNumErrors( 0 ) {}

MemExtractCallback::~MemExtractCallback() {}

STDMETHODIMP MemExtractCallback::SetTotal( UInt64 size ) {
    if ( mOpener.totalCallback() ) {
        mOpener.totalCallback()( size );
    }
    return S_OK;
}

STDMETHODIMP MemExtractCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mOpener.progressCallback() ) {
        mOpener.progressCallback()( *completeValue );
    }
    return S_OK;
}

STDMETHODIMP MemExtractCallback::GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode ) {
    *outStream = 0;
    mOutMemStream.Release();
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
            case VT_UI8: newFileSize = static_cast< UInt64 >( prop4.uhVal.QuadPart );
                break;
            default:
                mErrorMessage = L"151199";
                return E_FAIL;
        }

        //newFileSize = ConvertPropVariantToUInt64( prop4 );
    }

    if ( !mProcessedFileInfo.isDir ) {
        mOutMemStreamSpec = new COutMemStream( mBuffer );
        CMyComPtr< ISequentialOutStream > outStreamLoc( mOutMemStreamSpec );
        mOutMemStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }

    return S_OK;
}

STDMETHODIMP MemExtractCallback::PrepareOperation( Int32 askExtractMode ) {
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

STDMETHODIMP MemExtractCallback::SetOperationResult( Int32 operationResult ) {
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

//    if ( mOutBuffStream != NULL ) {
//        RINOK( mOutBuffStreamSpec->Close() );
//    }
    mOutMemStream.Release();

    if ( mNumErrors > 0 ) {
        return E_FAIL;
    }

    return S_OK;
}


STDMETHODIMP MemExtractCallback::CryptoGetTextPassword( BSTR* password ) {
    if ( !mOpener.isPasswordDefined() ) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        //in future, no exception but an event (i.e. onPasswordRequest) call
        mErrorMessage = L"Password is not defined";
        return E_FAIL;
    }

    return StringToBstr( mOpener.password().c_str(), password );
}

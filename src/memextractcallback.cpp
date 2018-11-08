// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
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

#include "../include/memextractcallback.hpp"

#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "7zip/Common/StreamObjects.h"

#include "../include/bitpropvariant.hpp"
#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"
#include "../include/util.hpp"


using namespace std;
using namespace NWindows;
using namespace bit7z;
using namespace bit7z::util;

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

MemExtractCallback::MemExtractCallback( const BitArchiveOpener& opener, IInArchive* archiveHandler, vector< byte_t >& buffer ) :
    mOpener( opener ),
    mArchiveHandler( archiveHandler ),
    mBuffer( buffer ),
    mExtractMode( true ),
    mProcessedFileInfo(),
    mOutMemStreamSpec( nullptr ),
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
    *outStream = nullptr;
    mOutMemStream.Release();
    // Get Name
    BitPropVariant prop;
    RINOK( mArchiveHandler->GetProperty( index, kpidPath, &prop ) );
    wstring fullPath;

    if ( prop.isEmpty() ) {
        fullPath = kEmptyFileAlias;
    } else {
        if ( prop.type() != BitPropVariantType::String ) {
            return E_FAIL;
        }

        fullPath = prop.getString();
    }

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract ) {
        return S_OK;
    }

    // Get Attrib
    BitPropVariant prop2;
    RINOK( mArchiveHandler->GetProperty( index, kpidAttrib, &prop2 ) );

    if ( prop2.isEmpty() ) {
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
    BitPropVariant prop3;
    RINOK( mArchiveHandler->GetProperty( index, kpidMTime, &prop3 ) );
    mProcessedFileInfo.MTimeDefined = false;

    switch ( prop3.type() ) {
        case BitPropVariantType::Empty:
            // mProcessedFileInfo.MTime = _utcMTimeDefault;
            break;

        case BitPropVariantType::Filetime:
            mProcessedFileInfo.MTime = prop3.filetime;
            mProcessedFileInfo.MTimeDefined = true;
            break;

        default:
            return E_FAIL;
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

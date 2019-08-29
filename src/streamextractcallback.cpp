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

#include "../include/streamextractcallback.hpp"

#include "../include/cstdoutstream.hpp"
#include "../include/bitpropvariant.hpp"
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

StreamExtractCallback::StreamExtractCallback( const BitArchiveHandler& handler,
                                              const BitInputArchive& inputArchive,
                                              ostream& outputStream )
    : mHandler( handler ),
      mInputArchive( inputArchive ),
      mOutputStream( outputStream ),
      mExtractMode( true ),
      mProcessedFileInfo(),
      mNumErrors( 0 ) {}

StreamExtractCallback::~StreamExtractCallback() {}

wstring StreamExtractCallback::getErrorMessage() const {
    if ( !mOutputStream ) {
        return L"Stream error (errno: " + std::to_wstring( errno ) + L")";
    }
    return Callback::getErrorMessage();
}

STDMETHODIMP StreamExtractCallback::SetTotal( UInt64 size ) {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

STDMETHODIMP StreamExtractCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mHandler.progressCallback() && completeValue != nullptr ) {
        mHandler.progressCallback()( *completeValue );
    }
    return S_OK;
}

STDMETHODIMP StreamExtractCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) {
    if ( mHandler.ratioCallback() && inSize != nullptr && outSize != nullptr ) {
        mHandler.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

STDMETHODIMP StreamExtractCallback::GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode ) try {
    *outStream = nullptr;
    mStdOutStream.Release();
    // Get Name
    BitPropVariant prop = mInputArchive.getItemProperty( index, BitProperty::Path );
    wstring fullPath;

    if ( prop.isEmpty() ) {
        fullPath = kEmptyFileAlias;
    } else if ( prop.isString() ) {
        fullPath = prop.getString();
    } else {
        return E_FAIL;
    }

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract ) {
        return S_OK;
    }

    // Get Attrib
    BitPropVariant prop2 = mInputArchive.getItemProperty( index, BitProperty::Attrib );

    if ( prop2.isEmpty() ) {
        mProcessedFileInfo.Attrib = 0;
        mProcessedFileInfo.AttribDefined = false;
    } else {
        if ( !prop2.isUInt32() ) {
            return E_FAIL;
        }

        mProcessedFileInfo.Attrib = prop2.getUInt32();
        mProcessedFileInfo.AttribDefined = true;
    }

    //RINOK( IsArchiveItemFolder( mInputArchive, index, mProcessedFileInfo.isDir ) );
    mProcessedFileInfo.isDir = mInputArchive.isItemFolder( index );

    // Get Modified Time
    BitPropVariant prop3 = mInputArchive.getItemProperty( index, BitProperty::MTime );
    mProcessedFileInfo.MTimeDefined = false;

    switch ( prop3.type() ) {
        case BitPropVariantType::Empty:
            // mProcessedFileInfo.MTime = _utcMTimeDefault;
            break;

        case BitPropVariantType::Filetime:
            mProcessedFileInfo.MTime = prop3.getFiletime();
            mProcessedFileInfo.MTimeDefined = true;
            break;

        default:
            return E_FAIL;
    }

    if ( !mProcessedFileInfo.isDir ) {
        auto* out_std_stream_spec = new CStdOutStream( mOutputStream );
        CMyComPtr< IOutStream > outStreamLoc( out_std_stream_spec );
        mStdOutStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }

    return S_OK;
} catch ( const BitException& ) {
    return E_OUTOFMEMORY;
}

STDMETHODIMP StreamExtractCallback::PrepareOperation( Int32 askExtractMode ) {
    mExtractMode = false;

    // in future we might use this switch to handle an event like onOperationStart(Operation o)
    // with enum Operation{Extract, Test, Skip}

    switch ( askExtractMode ) {
        case NArchive::NExtract::NAskMode::kExtract:
            mExtractMode = true;
            //wcout <<  kExtractingString;
            break;

        case NArchive::NExtract::NAskMode::kTest:
            mExtractMode = false;
            //wcout <<  kTestingString;
            break;

            /* case NArchive::NExtract::NAskMode::kSkip:
                cout <<  kSkippingString;
                break;*/
    }

    //wcout << mFilePath << endl;;
    return S_OK;
}

STDMETHODIMP StreamExtractCallback::SetOperationResult( Int32 operationResult ) {
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
    mStdOutStream.Release();

    if ( mNumErrors > 0 ) {
        return E_FAIL;
    }

    return S_OK;
}


STDMETHODIMP StreamExtractCallback::CryptoGetTextPassword( BSTR* password ) {
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
            return E_FAIL;
        }
    } else {
        pass = mHandler.password();
    }

    return StringToBstr( pass.c_str(), password );
}

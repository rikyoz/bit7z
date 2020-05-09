// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#include <utility>

#include "../include/fileextractcallback.hpp"
#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

#include <iostream>

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

FileExtractCallback::FileExtractCallback( const BitArchiveHandler& handler,
                                          const BitInputArchive& inputArchive,
                                          fs::path inFilePath,
                                          fs::path directoryPath,
                                          bool retainDirectories )
    : ExtractCallback( handler, inputArchive ),
      mInFilePath( std::move( inFilePath ) ),
      mDirectoryPath( std::move( directoryPath ) ),
      mRetainDirectories( retainDirectories ),
      mProcessedFileInfo() {}

//TODO: clean and optimize!
STDMETHODIMP FileExtractCallback::GetStream( UInt32 index,
                                             ISequentialOutStream** outStream,
                                             Int32 askExtractMode ) try {
    *outStream = nullptr;
    mFileOutStream.Release();
    // Get Name
    BitPropVariant prop = mInputArchive.getItemProperty( index, BitProperty::Path );

    fs::path filePath;
    if ( prop.isEmpty() ) {
        filePath = !mInFilePath.empty() ? mInFilePath.stem() : fs::path( kEmptyFileAlias );
    } else if ( prop.isString() ) {
        filePath = fs::path( prop.getString() );
        if ( !mRetainDirectories ) {
            filePath = filePath.filename();
        }
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

    mDiskFilePath = mDirectoryPath / filePath;

    if ( !mProcessedFileInfo.isDir ) { // File
        if ( mHandler.fileCallback() ) {
            mHandler.fileCallback()( mDiskFilePath.filename() );
        }

        std::error_code ec;
        fs::create_directories( mDiskFilePath.parent_path(), ec );

        if ( fs::exists( mDiskFilePath, ec ) && !fs::remove( mDiskFilePath, ec ) ) {
            mErrorMessage = kCannotDeleteOutput;
            return E_ABORT;
        }

        CMyComPtr< CFileOutStream > outStreamLoc = new CFileOutStream( mDiskFilePath, true );
        if ( outStreamLoc->fail() ) {
            mErrorMessage = kCannotOpenOutput;
            return E_ABORT;
        }

        mFileOutStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    } else if ( mRetainDirectories ) { // Directory, and we must retain it
        error_code ec;
        fs::create_directories( mDiskFilePath, ec );
    }

        return S_OK;
} catch ( const BitException& ) {
    return E_OUTOFMEMORY;
}

STDMETHODIMP FileExtractCallback::SetOperationResult( Int32 operationResult ) {
    if ( operationResult != NArchive::NExtract::NOperationResult::kOK ) {
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

    if ( mFileOutStream != nullptr ) {
        if ( mFileOutStream->fail() ) {
            return E_FAIL;
        }
        mFileOutStream.Release(); // We need to release the file to change its modified time!

        if ( mProcessedFileInfo.MTimeDefined ) {
            filesystem::fsutil::setFileModifiedTime( mDiskFilePath, mProcessedFileInfo.MTime );
        }
    }

    if ( mExtractMode && mProcessedFileInfo.AttribDefined ) {
        filesystem::fsutil::setFileAttributes( mDiskFilePath, mProcessedFileInfo.Attrib );
    }

    if ( mNumErrors > 0 ) {
        return E_FAIL;
    }

    return S_OK;
}

void FileExtractCallback::throwException( HRESULT error ) {
    if ( mErrorMessage != nullptr ) {
        throw BitException( mErrorMessage, mDiskFilePath.native(), error );
    }
    Callback::throwException( error );
}

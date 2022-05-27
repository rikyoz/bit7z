// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#include "internal/fileextractcallback.hpp"

#include "bitexception.hpp"
#include "internal/fsutil.hpp"

using namespace std;
using namespace NWindows;
using namespace bit7z;

constexpr auto kCannotOpenOutput = "Cannot open output file";
constexpr auto kCannotDeleteOutput = "Cannot delete output file";

FileExtractCallback::FileExtractCallback( const BitInputArchive& inputArchive,
                                          const tstring& directoryPath )
    : ExtractCallback( inputArchive ),
      mInFilePath( inputArchive.archivePath() ),
      mDirectoryPath( directoryPath ),
      mRetainDirectories( inputArchive.handler().retainDirectories() ) {}

void FileExtractCallback::releaseStream() {
    mFileOutStream.Release(); // We need to release the file to change its modified time!
}

void FileExtractCallback::finishOperation() {
    if ( mFileOutStream != nullptr ) {
        if ( mFileOutStream->fail() ) {
            ++mNumErrors; // SetOperationResult, which called releaseStream(), will return E_FAIL since mNumErrors > 0.
            return;
        }
        mFileOutStream.Release(); // We need to release the file to change its modified time!

        if ( mCurrentItem.isModifiedTimeDefined() ) {
            filesystem::fsutil::setFileModifiedTime( mFilePathOnDisk, mCurrentItem.modifiedTime() );
        }
    }

    if ( mExtractMode && mCurrentItem.areAttributesDefined() ) {
        filesystem::fsutil::setFileAttributes( mFilePathOnDisk, mCurrentItem.attributes() );
    }
}

void FileExtractCallback::throwException( HRESULT error ) {
    if ( mErrorMessage != nullptr ) {
        throw BitException( mErrorMessage, make_hresult_code( error ), mFilePathOnDisk.native() );
    }
    Callback::throwException( error );
}

HRESULT FileExtractCallback::getOutStream( uint32_t index, ISequentialOutStream** outStream ) {
    try {
        mCurrentItem.loadItemInfo( mInputArchive, index );
    } catch ( const BitException& ex ) {
        mErrorMessage = ex.what();
        return E_FAIL;
    }

    fs::path filePath = mCurrentItem.path();
    if ( filePath.empty() ) {
        filePath = !mInFilePath.empty() ? mInFilePath.stem() : fs::path( kEmptyFileAlias );
    } else if ( !mRetainDirectories ) {
        filePath = filePath.filename();
    } else {
        // No action needed
    }
    mFilePathOnDisk = mDirectoryPath / filePath;

    if ( !mInputArchive.isItemFolder( index ) ) { // File
        if ( mHandler.fileCallback() ) {
            mHandler.fileCallback()( filePath.string< tchar >() );
        }

        std::error_code ec;
        fs::create_directories( mFilePathOnDisk.parent_path(), ec );

        if ( fs::exists( mFilePathOnDisk, ec ) && !fs::remove( mFilePathOnDisk, ec ) ) {
            mErrorMessage = kCannotDeleteOutput;
            return E_ABORT;
        }

        try {
            auto outStreamLoc = bit7z::make_com< CFileOutStream >( mFilePathOnDisk, true );
            mFileOutStream = outStreamLoc;
            *outStream = outStreamLoc.Detach();
        } catch ( const BitException& ) {
            mErrorMessage = kCannotOpenOutput;
            return E_ABORT;
        }
    } else if ( mRetainDirectories ) { // Directory, and we must retain it
        std::error_code ec;
        fs::create_directories( mFilePathOnDisk, ec );
    } else {
        // No action needed
    }
    return S_OK;
}

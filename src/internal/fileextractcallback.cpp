// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/fileextractcallback.hpp"

#include "bitabstractarchivehandler.hpp"
#include "bitexception.hpp"
#include "bitinputarchive.hpp"
#include "bittypes.hpp"
#include "extractcallback.hpp"
#include "internal/fsutil.hpp"
#include "internal/stringutil.hpp"
#include "internal/util.hpp"
#include "operationresult.hpp"

#include <cstdint>
#include <system_error>

using namespace NWindows;

namespace bit7z {

FileExtractCallback::FileExtractCallback( const BitInputArchive& inputArchive,
                                          const tstring& directoryPath,
                                          RenameCallback callback )
    : ExtractCallback( inputArchive ),
      mInFilePath( tstring_to_path( inputArchive.archivePath() ) ),
      mDirectoryPath( tstring_to_path( directoryPath ) ),
      mRetainDirectories( inputArchive.handler().retainDirectories() ),
      mRenameCallback{ std::move( callback ) } {}

void FileExtractCallback::releaseStream() {
    mFileOutStream.Release(); // We need to release the file to change its modified time.
}

auto FileExtractCallback::finishOperation( OperationResult operationResult ) -> HRESULT {
    const HRESULT result = operationResult != OperationResult::Success ? E_FAIL : S_OK;
    if ( mFileOutStream == nullptr ) {
        return result;
    }

    mFileOutStream.Release(); // We need to release the file to change its modified time.

    if ( extractMode() != ExtractMode::Extract ) { // No need to set attributes or modified time of the file.
        return result;
    }

#ifdef _WIN32
    const auto creationTime = mCurrentItem.hasCreationTime() ? mCurrentItem.creationTime() : FILETIME{};
    const auto accessTime = mCurrentItem.hasAccessTime() ? mCurrentItem.accessTime() : FILETIME{};
    const auto modifiedTime = mCurrentItem.hasModifiedTime() ? mCurrentItem.modifiedTime() : FILETIME{};
    filesystem::fsutil::set_file_time( mFilePathOnDisk, creationTime, accessTime, modifiedTime );
#else
    if ( mCurrentItem.hasModifiedTime() ) {
        filesystem::fsutil::set_file_modified_time( mFilePathOnDisk, mCurrentItem.modifiedTime() );
    }
#endif

    if ( mCurrentItem.areAttributesDefined() ) {
        filesystem::fsutil::set_file_attributes( mFilePathOnDisk, mCurrentItem.attributes() );
    }
    return result;
}

auto FileExtractCallback::getCurrentItemPath() const -> fs::path {
    fs::path filePath = mCurrentItem.path();
    if ( filePath.empty() ) {
        filePath = !mInFilePath.empty() ? mInFilePath.stem() : fs::path{ kEmptyFileAlias };
    } else if ( !mRetainDirectories ) {
        filePath = filePath.filename();
    } else {
        // No action needed
    }
    return filePath;
}

constexpr auto kCannotDeleteOutput = "Cannot delete output file";

auto FileExtractCallback::getOutStream( uint32_t index, ISequentialOutStream** outStream ) -> HRESULT {
    mCurrentItem.loadItemInfo( inputArchive(), index );

    auto filePath = getCurrentItemPath();

    if ( mRenameCallback ) {
#if !defined( _WIN32 ) || defined( BIT7Z_USE_NATIVE_STRING )
        // Here we don't use the path_to_tstring function to avoid allocating a new string object.
        const auto& filePathString = filePath.native();
#else
        const auto filePathString = path_to_tstring( filePath );
#endif
        filePath = tstring_to_path( mRenameCallback( index, filePathString ) );
    }

    if ( filePath.empty() ) {
        return S_OK;
    }
#if defined( _WIN32 ) && defined( BIT7Z_PATH_SANITIZATION )
    mFilePathOnDisk = mDirectoryPath / filesystem::fsutil::sanitize_path( filePath );
#else
    mFilePathOnDisk = mDirectoryPath / filePath;
#endif

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
    if ( filesystem::fsutil::should_format_long_path( mFilePathOnDisk ) ) {
        mFilePathOnDisk = filesystem::fsutil::format_long_path( mFilePathOnDisk );
    }
#endif

    if ( !isItemFolder( index ) ) { // File
        if ( mHandler.fileCallback() ) {
#if !defined( _WIN32 ) || defined( BIT7Z_USE_NATIVE_STRING )
            // Here we don't use the path_to_tstring function to avoid allocating a new string object.
            const auto& filePathString = filePath.native();
#else
            const auto filePathString = path_to_tstring( filePath );
#endif
            mHandler.fileCallback()( filePathString );
        }

        std::error_code error;

        if ( fs::exists( mFilePathOnDisk, error ) ) {
            const OverwriteMode overwriteMode = mHandler.overwriteMode();

            switch ( overwriteMode ) {
                case OverwriteMode::None: {
                    throw BitException( kCannotDeleteOutput,
                                        make_hresult_code( E_ABORT ),
                                        path_to_tstring( mFilePathOnDisk ) );
                }
                case OverwriteMode::Skip: {
                    return S_OK;
                }
                case OverwriteMode::Overwrite:
                default: {
                    if ( !fs::remove( mFilePathOnDisk, error ) ) {
                        throw BitException( kCannotDeleteOutput, error, path_to_tstring( mFilePathOnDisk ) );
                    }
                    break;
                }
            }
        } else {
            const auto parentPath = mFilePathOnDisk.parent_path();
            if ( !fs::exists( parentPath, error ) ) {
                fs::create_directories( parentPath, error );
            }
            // TODO: Handle errors
        }

        auto outStreamLoc = bit7z::make_com< CFileOutStream >( mFilePathOnDisk, true );
        mFileOutStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    } else if ( mRetainDirectories ) { // Directory, and we must retain it
        std::error_code error;
        if ( !fs::exists( mFilePathOnDisk, error ) ) {
            fs::create_directories( mFilePathOnDisk, error );
        }
        // TODO: Handle errors
    } else {
        // No action needed
    }
    return S_OK;
}

} // namespace bit7z
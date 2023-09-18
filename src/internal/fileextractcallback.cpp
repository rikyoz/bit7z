// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/fileextractcallback.hpp"

#include "bitexception.hpp"
#include "internal/fsutil.hpp"
#include "internal/util.hpp"

using namespace std;
using namespace NWindows;
using namespace bit7z;
using namespace bit7z::filesystem;

constexpr auto kCannotDeleteOutput = "Cannot delete output file";

FileExtractCallback::FileExtractCallback( const BitInputArchive& inputArchive, const tstring& directoryPath )
    : ExtractCallback( inputArchive ),
      mInFilePath( inputArchive.archivePath() ),
      mDirectoryPath( directoryPath ),
      mRetainDirectories( inputArchive.handler().retainDirectories() ) {}

void FileExtractCallback::releaseStream() {
    mFileOutStream.Release(); // We need to release the file to change its modified time!
}

HRESULT FileExtractCallback::finishOperation( OperationResult operation_result ) {
    const HRESULT result = operation_result != OperationResult::Success ? E_FAIL : S_OK;
    if ( mFileOutStream == nullptr ) {
        return result;
    }

    if ( mFileOutStream->fail() ) {
        return E_FAIL;
    }

    mFileOutStream.Release(); // We need to release the file to change its modified time!

    if ( extractMode() != ExtractMode::Extract ) { // No need to set attributes or modified time of the file.
        return result;
    }

    if ( mCurrentItem.isModifiedTimeDefined() ) {
        filesystem::fsutil::setFileModifiedTime( mFilePathOnDisk, mCurrentItem.modifiedTime() );
    }

    if ( mCurrentItem.areAttributesDefined() ) {
        filesystem::fsutil::setFileAttributes( mFilePathOnDisk, mCurrentItem.attributes() );
    }
    return result;
}

fs::path FileExtractCallback::getCurrentItemPath() const {
    fs::path filePath = mCurrentItem.path();
    if ( filePath.empty() ) {
        filePath = !mInFilePath.empty() ? mInFilePath.stem() : fs::path( kEmptyFileAlias );
    } else if ( !mRetainDirectories ) {
        filePath = filePath.filename();
    } else {
        // No action needed
    }
    return filePath;
}

#ifdef _WIN32
/**
 * Sanitizes the given file path, removing any eventual Windows illegal character
 * (https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file)
 *
 * @param path The path to be sanitized.
 *
 * @return the sanitized path, where illegal characters are replaced with the '_' character.
 */
auto sanitize_path( const fs::path& path ) -> fs::path {
    auto path_str = path.wstring();
    std::replace_if( path_str.begin(), path_str.end(), []( wchar_t chr ) {
        constexpr auto last_non_printable_ascii = 31;
        return chr <= last_non_printable_ascii || chr == L'[' || chr == L'<' || chr == L'>' || chr == L':' ||
               chr == L'"' || chr == L'/' || chr == L'|' || chr == L'?' || chr == L'*' || chr == L']';
    }, L'_' );
    return fs::path{ path_str };
}
#endif

HRESULT FileExtractCallback::getOutStream( uint32_t index, ISequentialOutStream** outStream ) {
    mCurrentItem.loadItemInfo( inputArchive(), index );

    auto filePath = getCurrentItemPath();
#ifdef _WIN32
    filePath = sanitize_path( filePath );
#endif
    mFilePathOnDisk = mDirectoryPath / filePath;

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
    if ( fsutil::should_format_long_path( mFilePathOnDisk ) ) {
        mFilePathOnDisk = fsutil::format_long_path( mFilePathOnDisk );
    }
#endif

    if ( !isItemFolder( index ) ) { // File
        if ( mHandler.fileCallback() ) {
            mHandler.fileCallback()( filePath.string< tchar >() );
        }

        std::error_code error;
        fs::create_directories( mFilePathOnDisk.parent_path(), error );

        if ( fs::exists( mFilePathOnDisk, error ) ) {
            const OverwriteMode overwrite_mode = mHandler.overwriteMode();

            switch ( overwrite_mode ) {
                case OverwriteMode::None: {
                    throw BitException( kCannotDeleteOutput,
                                        make_hresult_code( E_ABORT ),
                                        mFilePathOnDisk.string< tchar >() );
                }
                case OverwriteMode::Skip: {
                    return S_OK;
                }
                case OverwriteMode::Overwrite:
                default: {
                    if ( !fs::remove( mFilePathOnDisk, error ) ) {
                        throw BitException( kCannotDeleteOutput,
                                            make_hresult_code( E_ABORT ),
                                            mFilePathOnDisk.string< tchar >() );
                    }
                    break;
                }
            }
        }

        auto outStreamLoc = bit7z::make_com< CFileOutStream >( mFilePathOnDisk, true );
        mFileOutStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    } else if ( mRetainDirectories ) { // Directory, and we must retain it
        std::error_code error;
        fs::create_directories( mFilePathOnDisk, error );
    } else {
        // No action needed
    }
    return S_OK;
}

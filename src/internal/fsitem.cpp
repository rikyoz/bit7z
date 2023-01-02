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

#include "internal/fsitem.hpp"

#include <system_error>

#include "bitexception.hpp"
#include "internal/cfileinstream.hpp"
#include "internal/fsutil.hpp"
#include "internal/util.hpp"

using bit7z::tstring;
using bit7z::filesystem::FSItem;

/* NOTES:
 * 1) mPath contains the path to the file, including the filename. It can be relative or absolute, according to what
 *    the user passes as path parameter in the constructor. If it is a directory, it doesn't contain a trailing / or \
 *    character, to use the method FindFirstFile without problems (as requested by that WinAPI function).
 * 2) mSearchPath contains the search path in which the item was found (e.g., if FSIndexer is searching items in
 *    "foo/bar/", each FSItem created for the elements it found will have mSearchPath == "foo/bar").
 *    As in mPath, mSearchPath does not contain trailing / or \! *
 * 3) mInArchivePath is the path of the item in the archive. If not already given (i.e., the user doesn't want to custom
 *    the path of the file in the archive), the path in the archive is calculated form mPath and mSearchPath
 *    (see inArchivePath() method). */

FSItem::FSItem( const fs::path& itemPath, fs::path inArchivePath )
    : mFileAttributeData(),
      mInArchivePath( !inArchivePath.empty() ? std::move( inArchivePath ) : fsutil::inArchivePath( itemPath ) ) {
    std::error_code error;
    mFileEntry.assign( itemPath, error );
    if ( error ) {
        throw BitException( "Cannot read file entry", error, itemPath.string< tchar >() );
    }
    if ( !mFileEntry.exists( error ) ) { // NOLINT
        if ( !error ) { // call to "exists(error)" succeeded
            error = std::make_error_code( std::errc::no_such_file_or_directory );
        }
        throw BitException( "Invalid path", error, itemPath.string< tchar >() );
    }
    initAttributes( itemPath );
}

FSItem::FSItem( fs::directory_entry entry, const fs::path& searchPath )
    : mFileEntry( std::move( entry ) ),
      mFileAttributeData(),
      mInArchivePath( fsutil::inArchivePath( mFileEntry.path(), searchPath ) ) {
    initAttributes( mFileEntry.path() );
}

void FSItem::initAttributes( const fs::path& itemPath ) {
    if ( !fsutil::getFileAttributesEx( itemPath.c_str(), mFileAttributeData ) ) {
        //should not happen, but anyway...
        throw BitException( "Could not retrieve file attributes", last_error_code(), itemPath.string< tchar >() );
    }
}

bool FSItem::isDots() const {
    const auto filename = mFileEntry.path().filename();
    return ( filename == "." || filename == ".." );
}

bool FSItem::isDir() const noexcept {
    std::error_code error;
    const bool res = mFileEntry.is_directory( error );
    return !error && res;
}

uint64_t FSItem::size() const noexcept {
    std::error_code error;
    const auto res = mFileEntry.file_size( error );
    return !error ? res : 0;
}

FILETIME FSItem::creationTime() const noexcept {
    return mFileAttributeData.ftCreationTime;
}

FILETIME FSItem::lastAccessTime() const noexcept {
    return mFileAttributeData.ftLastAccessTime;
}

FILETIME FSItem::lastWriteTime() const noexcept {
    return mFileAttributeData.ftLastWriteTime;
}

tstring FSItem::name() const {
    BIT7Z_MAYBE_UNUSED std::error_code error;
    return fs::canonical( mFileEntry.path(), error ).filename().string< tchar >();
}

tstring FSItem::path() const {
    return mFileEntry.path().string< tchar >();
}

/* NOTE:
 * inArchivePath() returns the path that should be used inside the archive when compressing the item, i.e., the path
 * relative to the 'root' of the archive.
 * This is needed to behave like 7-zip and retaining the directory structure when creating new archives.
 *
 * In particular, 7-zip behaves differently according to the kind of paths that are passed to it:
 * + absolute paths (e.g. "C:\foo\bar\test.txt"):
 *   the file is compressed without any directory structure (e.g. "test.txt"), unless it was inside a directory passed
 *   by the user and scanned by FSIndexer: in this case only the directory structure is retained.
 *
 * + relative paths containing current dir or outside references (e.g. containing a "./" or "../" substring,
 *   like in "../foo/bar/test.txt"):
 *   same as absolute paths (e.g. "test.txt").
 *
 * + relative paths (e.g. "foo/bar/test.txt"):
 *   the file is compressed retaining the directory structure (e.g. "foo/bar/test.txt" in both example cases).
 *
 * If the mInArchivePath is already given (i.e., the user wants a custom mapping of files), this one is returned.*/
fs::path FSItem::inArchivePath() const {
    return mInArchivePath;
}

uint32_t FSItem::attributes() const noexcept {
    return mFileAttributeData.dwFileAttributes;
}

HRESULT FSItem::getStream( ISequentialInStream** inStream ) const {
    if ( isDir() ) {
        return S_OK;
    }

    try {
        auto inStreamLoc = bit7z::make_com< CFileInStream >( path() );
        *inStream = inStreamLoc.Detach();
    } catch ( const BitException& ex ) {
        return ex.nativeCode();
    }
    return S_OK;
}

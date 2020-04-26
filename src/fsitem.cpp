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
#include <system_error>

#include "../include/fsitem.hpp"

#include "../include/bitexception.hpp"

using namespace bit7z::filesystem;

/* NOTES:
 * 1) mPath contains the path to the file, including the filename. It can be relative or absolute, according to what
 *    the user passes as path parameter in the constructor. If it is a directory, it doesn't contain a trailing / or \
 *    character, in order to use the method FindFirstFile without problems (as requested by that WinAPI function).
 * 2) mSearchPath contains the search path in which the item was found (e.g. if FSIndexer is searching items in
 *    "foo/bar/", each FSItem created for the elements it found will have mSearchPath == "foo/bar").
 *    As in mPath, mSearchPath does not contain trailing / or \! *
 * 3) mInArchivePath is the path of the item in the archive. If not already given (i.e. the user doesn't want to custom
 *    the path of the file in the archive), the path in the archive is calculated form mPath and mSearchPath
 *    (see inArchivePath() method). */

FSItem::FSItem( const fs::path& itemPath, fs::path inArchivePath )
    : mFileAttributeData(), mSearchPath(), mInArchivePath( std::move( inArchivePath ) ) {
    std::error_code ec;
    mFileEntry.assign( itemPath, ec );
    if ( !mFileEntry.exists() ) { // NOLINT
        //TODO: use error_code instead of WinAPI error codes or HRESULT
        throw BitException( "Invalid path", itemPath.native(), HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) );
    }
    initAttributes( itemPath );
}

FSItem::FSItem( fs::directory_entry entry, fs::path searchPath )
    : mFileEntry( std::move( entry ) ), mFileAttributeData(), mSearchPath( std::move( searchPath ) ) {
    initAttributes( mFileEntry.path() );
}

void FSItem::initAttributes( const fs::path& itemPath ) {
    if ( !fsutil::getFileAttributesEx( itemPath.c_str(), mFileAttributeData ) ) {
        //should not happen, but anyway...
        throw BitException( "Could not retrieve file attributes", itemPath.native(), HRESULT_FROM_WIN32( GetLastError() ) );
    }
}

bool FSItem::isDots() const {
    const auto filename = mFileEntry.path().filename();
    return ( filename == "." || filename == ".." );
}

bool FSItem::isDir() const {
    std::error_code ec;
    bool res = mFileEntry.is_directory( ec );
    return !ec ? res : false;
}

uint64_t FSItem::size() const {
    std::error_code ec;
    auto res = mFileEntry.file_size( ec );
    return !ec ? res : 0;
}

FILETIME FSItem::creationTime() const {
    return mFileAttributeData.ftCreationTime;
}

FILETIME FSItem::lastAccessTime() const {
    return mFileAttributeData.ftLastAccessTime;
}

FILETIME FSItem::lastWriteTime() const {
    return mFileAttributeData.ftLastWriteTime;
}

bit7z::tstring FSItem::name() const {
    [[maybe_unused]] std::error_code ec;
    return canonical( mFileEntry.path(), ec ).filename();
}

fs::path FSItem::path() const {
    return mFileEntry.path();
}

/* NOTE:
 * inArchivePath() returns the path that should be used inside the archive when compressing the item, i.e. the path
 * relative to the 'root' of the archive.
 * This is needed in order to behave like 7-zip and retaining the directory structure when creating new archives.
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
 * If the mInArchivePath is already given (i.e. the user wants a custom mapping of files), this one is returned.*/
fs::path FSItem::inArchivePath() const {
    using namespace fsutil;

    if ( !mInArchivePath.empty() ) {
        return mInArchivePath;
    }

    const auto& path = mFileEntry.path();
    const auto& filename = path.lexically_normal().filename();
    if ( filename == "." || filename == ".." ) {
        return TSTRING( "" );
    }

    const auto& native_path = path.native();
    if ( path.is_absolute() ||
         native_path.find( TSTRING( "./" ) ) != bit7z::tstring::npos ||
         native_path.find( TSTRING( ".\\" ) ) != bit7z::tstring::npos ) {
        // Note: in this case if the file was found while searching in a directory passed by the user, we need to retain
        // the internal structure of that folder (mSearchPath), otherwise we use only the file name.
        if ( mSearchPath.empty() ) {
            return name();
        }
        return mSearchPath / name();
    }

    //path is relative and without ./ or ../ => e.g. foo/bar/test.txt
    return path;
}

uint32_t FSItem::attributes() const {
    return mFileAttributeData.dwFileAttributes;
}

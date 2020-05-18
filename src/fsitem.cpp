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

#include "../include/fsitem.hpp"

#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

#include <algorithm>

using namespace bit7z::filesystem;

/* NOTES:
 * 1) mPath contains the path to the file, including the filename. It can be relative or absolute, according to what
 *    the user passes as path parameter in the constructor. If it is a directory, it doesn't contain a trailing / or \
 *    character, in order to use the method FindFirstFile without problems (as requested by that winapi function).
 * 2) mSearchPath contains the search path in which the item was found (e.g. if FSIndexer is searching items in
 *    "foo/bar/", each FSItem created for the elements it found will have mSearchPath == "foo/bar").
 *    As in mPath, mSearchPath does not contain trailing / or \! *
 * 3) mInArchivePath is the path of the item in the archive. If not already given (i.e. the user doesn't want to custom
 *    the path of the file in the archive), the path in the archive is calculated form mPath and mSearchPath
 *    (see inArchivePath() method). */

FSItem::FSItem( const wstring& path, const wstring& inArchivePath )
    : mPath( path ), mFileData(), mSearchPath( L"" ), mInArchivePath( inArchivePath ) {
    bool is_dir = fsutil::isDirectory( mPath );
    if ( is_dir && !mPath.empty() ) {
        // The FSItem is a directory!
        // If the path ends with a / or a \, it's removed, since FindFirstFile doesn't want it!
        if ( mPath.back() == L'/' || mPath.back() == L'\\' ) {
            mPath.pop_back();
        }
    }
    HANDLE find_handle = FindFirstFile( mPath.c_str(), &mFileData );
    if ( find_handle == INVALID_HANDLE_VALUE ) {
        throw BitException( L"Invalid path '" + mPath + L"'!", GetLastError() );
    }
    FindClose( find_handle );
}

FSItem::FSItem( const wstring& dir, FSItemInfo data, const wstring& search_path )
    : mPath( dir ), mFileData( data ), mSearchPath( search_path ) {
    /* Now mPath is the path without the filename, since dir is the path containing the file 'data'!
     * So we must add the filename! */
    if ( mPath.back() == L'/' || mPath.back() == L'\\' ) {
        mPath += name();
    } else {
        mPath += L"\\" + name();
    }
}

bool FSItem::isDots() const {
    return ( name() == L"." || name() == L".." );
}

bool FSItem::isDir() const {
    return ( mFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
}

uint64_t FSItem::size() const {
    ULARGE_INTEGER size;
    size.LowPart = mFileData.nFileSizeLow;
    size.HighPart = mFileData.nFileSizeHigh;
    return size.QuadPart;
}

FILETIME FSItem::creationTime() const {
    return mFileData.ftCreationTime;
}

FILETIME FSItem::lastAccessTime() const {
    return mFileData.ftLastAccessTime;
}

FILETIME FSItem::lastWriteTime() const {
    return mFileData.ftLastWriteTime;
}

wstring FSItem::name() const {
    return static_cast< const wchar_t* >( mFileData.cFileName );
}

wstring FSItem::path() const {
    return mPath;
}

bool contains_dot_references( const wstring& path ) {
    /* Note: here we are supposing that path does not contain file names with a final dot (e.g., "foo.").
             This must be true on Windows, but not on Unix systems! */
    return std::adjacent_find( path.begin(), path.end(), []( wchar_t a, wchar_t b ) {
               return a == L'.' && ( b == L'/' || b == L'\\' );
           } ) != path.end();
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
wstring FSItem::inArchivePath() const {
    using namespace fsutil;

    if ( !mInArchivePath.empty() ) {
        return mInArchivePath;
    }

    if ( !isRelativePath( mPath ) || contains_dot_references( mPath ) ) {
        // Note: in this case if the file was found while searching in a directory passed by the user, we need to retain
        // the interal structure of that folder (mSearchPath), otherwise we use only the file name.
        return mSearchPath.empty() ? name() : mSearchPath + L"\\" + name();
    }

    if ( mPath == L"." || mPath == L".." ) {
        return L"";
    }

    //path is relative and without ./ or ../ => e.g. foo/bar/test.txt

    if ( !mSearchPath.empty() ) {
        // The item was found while indexing a directory
        return mSearchPath + L"\\" + name();
    }
    return mPath;
}

uint32_t FSItem::attributes() const {
    return mFileData.dwFileAttributes;
}

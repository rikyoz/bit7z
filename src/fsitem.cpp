// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/fsitem.hpp"

#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

#include <string>

using namespace std;
using namespace bit7z::filesystem;

/* NOTES:
 * 1) mPath contains the path to the file, including the filename. It can be relative or absolute, according to what
 *    the user passes as path parameter in the constructor. If it is a directory, it doesn't contain a trailing / or \
 *    character, in order to use the method FindFirstFile without problems (as requested by that winapi function).
 * 2) mSearchDirname contains the search path in which the item was found (e.g. if FSIndexer is searching items in
 *    "foo/bar/", each FSItem created for the found elements will have mSearchDirname == "foo/bar").
 *    As in mPath, mSearchDirname does not contain trailing / or \! */

FSItem::FSItem( const wstring& path, const wstring& inArchivePath )
    : mPath( path ), mFileData(), mSearchPath( L"" ), mInArchivePath( inArchivePath ) {
    bool is_dir = fsutil::is_directory( mPath );
    if ( is_dir && !mPath.empty() ) {
        // The FSItem is a directory!
        // If the path ends with a / or a \, it's removed, since FindFirstFile doesn't want it!
        if ( mPath.back() == L'/' || mPath.back() == L'\\' ) {
            mPath.pop_back();
        }
    }
    HANDLE find_handle = FindFirstFile( mPath.c_str(), &mFileData );
    if ( find_handle == INVALID_HANDLE_VALUE ) {
        throw BitException( L"Invalid path '" + mPath + L"'!" );
    }
    FindClose( find_handle );
}

FSItem::FSItem( const wstring& dir, FSItemInfo data, const wstring& search_path ) :
    mPath( dir ), mFileData( data ), mSearchPath( search_path ) {
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
    return mFileData.cFileName;
}

wstring FSItem::path() const {
    return mPath;
}

/* NOTE:
 * inArchivePath() returns the path that should be used when compressing the item in an archive, that is the path
 * relative to the 'root' of the archive.
 * This is needed in order to behave like 7-zip and retaining the directory structure when creating new archives.
 * In particular, 7-zip behaves differently according to the kind of paths that are passed to it:
 * + absolute paths (e.g. "C:\foo\bar\test.txt"):
 *   the file is compressed without any directory structure (e.g. "test.txt"), unless it was inside a directory passed
 *   by the user and scanned by FSIndexer: in this case only the directory structure is retained.
 *
 * + relative paths containing current dir or outside references (e.g. containing a "./" or "../" substring,
 *   like in "../foo/bar/test.txt"): same as absolute paths (e.g. "test.txt").
 *
 * + relative paths (e.g. "foo/bar/test.txt"):
 *   the file is compressed retaining the directory structure (e.g. "foo/bar/test.txt" in both example cases). */
wstring FSItem::inArchivePath() const {
    using namespace fsutil;

    if ( !mInArchivePath.empty() ) {
        return mInArchivePath;
    }

    if ( !is_relative_path( mPath ) ||
            mPath.find( L"./" ) != wstring::npos || mPath.find( L".\\" ) != wstring::npos ) {
        // Note: in this case if the file was found while searching in a directory passed by the user, we need to retain
        // the interal structure of that folder (mSearchDirname), otherwise we use only the file name.
        return mSearchPath.empty() ? name() : mSearchPath + L"\\" + name();
    }

    if ( mPath == L"." || mPath == L".." ) {
        return L"";
    }

    //path is relative and without ./ or ../ => e.g. foo/bar/test.txt
    return mPath;
}

uint32_t FSItem::attributes() const {
    return mFileData.dwFileAttributes;
}

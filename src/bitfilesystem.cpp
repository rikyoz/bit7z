#include "../include/bitfilesystem.hpp"

#include "../include/bitexception.hpp"

#include <string>

using namespace Bit7z::FileSystem;

FSItem::FSItem( const wstring& path, const wstring& prefix ) : mPrefix( prefix ) {
    FindFirstFile( path.c_str(), &mFileData );
    if ( isDir() ) mDirectory = path;
}

FSItem::FSItem( const wstring& directory, const wstring& prefix,
                FSItemInfo data ) : mDirectory( directory ), mPrefix( prefix ), mFileData( data ) {}

bool FSItem::isDir() {
    return ( mFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
}

UInt64 FSItem::size() {
    ULARGE_INTEGER size;
    size.LowPart = mFileData.nFileSizeLow;
    size.HighPart = mFileData.nFileSizeHigh;
    return size.QuadPart;
}

FILETIME FSItem::creationTime() {
    return mFileData.ftCreationTime;
}

FILETIME FSItem::lastAccessTime() {
    return mFileData.ftLastAccessTime;
}

FILETIME FSItem::lastWriteTime() {
    return mFileData.ftLastWriteTime;
}

wstring FSItem::name() {
    return mFileData.cFileName;
}

wstring FSItem::relativePath() {
    if ( mPrefix.empty() )
        return mFileData.cFileName;
    return mPrefix + L"\\" + mFileData.cFileName;
}

wstring FSItem::fullPath() {
    return mDirectory + mFileData.cFileName;
}

UInt32 FSItem::attributes() {
    return mFileData.dwFileAttributes;
}

bool path_exists( const wstring& path ) {
    return ( GetFileAttributes( path.c_str() ) != INVALID_FILE_ATTRIBUTES );
}

bool hasEnding ( wstring const& fullString, wstring const& ending ) {
    if ( fullString.length() >= ending.length() )
        return ( 0 == fullString.compare ( fullString.length() - ending.length(), ending.length(),
                                           ending ) );
    else
        return false;
}

void FSIndexer::listFilesInDirectory( vector<FSItem>& result, const wstring& directory, const wstring& filter, bool recursive ) {
    wstring clean_dir = directory;
    if ( hasEnding( directory, L"\\" ) || hasEnding( directory, L"/" ) )
        clean_dir.pop_back();
    FSItem dirItem( clean_dir );
    FSIndexer::listFilesInDirectory( result, directory, filter, recursive, dirItem.name() );
}

void FSIndexer::listFilesInDirectory( vector<FSItem>& result, const wstring& directory, const wstring& filter, bool recursive,
                                      const wstring& prefix ) {
    wstring filtered_path = directory;
    if ( hasEnding( directory, L"\\" ) || hasEnding( directory, L"/" ) )
        filtered_path += filter;
    else
        filtered_path += L"\\" + filter;
    FSItemInfo data;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    //DWORD dwError=0;
    hFind = FindFirstFile( filtered_path.c_str(), &data );

    if ( INVALID_HANDLE_VALUE == hFind )
        throw BitException( UString( filtered_path.c_str() ) + L" is an invalid path!" );

    // List all the files in the directory with some info about them.
    do {
        FSItem currentItem = FSItem( directory, prefix, data );
        if ( currentItem.isDir() ) {
            if ( recursive && !hasEnding( currentItem.relativePath(), L"." ) && !hasEnding( currentItem.relativePath(), L"..") ) {
                wstring pfix = ( prefix.empty() ? currentItem.name() : prefix + L"\\" + currentItem.name() );
                listFilesInDirectory( result, directory + L"\\" + currentItem.name(), filter, true, pfix );
            }
        } else {
            result.push_back( currentItem );
        }
    } while ( FindNextFile( hFind, &data ) != 0 );

    //dwError = GetLastError();
    //if ( dwError != ERROR_NO_MORE_FILES )
    //error!!

    FindClose( hFind );
}



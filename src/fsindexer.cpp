#include "../include/fsindexer.hpp"

#include "../include/fsutil.hpp"
#include "../include/bitexception.hpp"

using std::wstring;
using namespace bit7z::filesystem;

FSIndexer::FSIndexer( const wstring& directory, const wstring& filter ) : mDirectory( directory ), mFilter( filter ) {
    const size_t lastSlashIndex = mDirectory.find_last_of( L"\\/" );
    if ( lastSlashIndex == mDirectory.length() - 1 ) {
        mDirectory.pop_back();
    }
    FSItem dirItem( mDirectory );
    if ( !dirItem.exists() ) {
        // It seems that msvc2010 doesn't support the concatenation operator+ for wstrings
        std::wstring msg = L"'";
        msg += dirItem.name();
        msg += L"' does not exist!";
        throw BitException( msg );
    }
    if ( !dirItem.isDir() ) {
        std::wstring msg = L"'";
        msg += dirItem.name();
        msg += L"' is not a directory!";
        throw BitException( msg );
    }
    mDirName = dirItem.name();
}

void FSIndexer::listFilesInDirectory( vector< FSItem >& result, bool recursive ) {
    FSIndexer::listFilesInDirectory( result, recursive, L"" );
}

void FSIndexer::listFiles( const vector< wstring >& in_paths, vector< FSItem >& out_files ) {
    for ( auto itr = in_paths.cbegin(); itr != in_paths.cend(); ++itr ) {
        const std::wstring & filePath = *itr;
        FSItem item( filePath );
        if ( !item.exists() ) {
            std::wstring msg = L"Item '";
            msg += item.name();
            msg += L"' does not exist!";
            throw BitException( msg );
        }
        if ( item.isDir() ) {
            FSIndexer indexer( filePath );
            indexer.listFilesInDirectory( out_files );
        } else{
            out_files.push_back( item );
        }
    }
}

void FSIndexer::removeListedDirectories( const vector< wstring >& in_paths, vector< FSItem >& out_files ) {
    for ( auto itr = in_paths.cbegin(); itr != in_paths.cend(); ++itr ) {
        const std::wstring & filePath = *itr;
        FSItem item( filePath );
        if ( item.exists() && !item.isDir() ) {
            out_files.push_back( item );
        }
    }
}

void FSIndexer::listFilesInDirectory( vector< FSItem >& result, bool recursive, const wstring& prefix ) {
    wstring filtered_path = mDirectory;
    filtered_path += L"\\";

    if ( !prefix.empty() ) {
        filtered_path += prefix;
        filtered_path += L"\\";
    }
    filtered_path += mFilter;
    FSItemInfo data;
    HANDLE hFind = FindFirstFile( filtered_path.c_str(), &data );

    if ( INVALID_HANDLE_VALUE == hFind ) {
        std::wstring msg = L"Invalid path '";
        msg += filtered_path;
        msg += L"'";
        throw BitException( msg );
    }

    do {
        //NOTE: This should definitely be optimized!!
        wstring ndir;
        if ( prefix.empty() ) {
            ndir = mDirectory;
        } else if ( prefix[0] == '\\' || prefix[0] == '/' ) {
            ndir = mDirectory;
            ndir += prefix;
        } else {
            ndir = mDirectory;
            ndir += L"\\";
            ndir += prefix;
        }

        wstring dirName = mDirName;
        dirName += prefix;

        FSItem currentItem = FSItem( ndir, dirName, data );
        if ( currentItem.isDir() ) {
            if ( recursive && currentItem.name().compare(L".") != 0 && currentItem.name().compare(L"..") != 0 ) {
                //wstring nprefix = ( prefix.empty() ) ? currentItem.name() : prefix + L"\\" + currentItem.name();
                wstring listDir = prefix;
                listDir += L"\\";
                listDir += currentItem.name();
                listFilesInDirectory( result, true, listDir );
            }
        } else {
            result.push_back( currentItem );
        }
    } while ( FindNextFile( hFind, &data ) != 0 );

    FindClose( hFind );
}

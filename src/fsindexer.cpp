#include "../include/fsindexer.hpp"

#include <string>

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
        throw BitException( L"'" + dirItem.name() + L"' does not exist!" );
    }
    if ( !dirItem.isDir() ) {
        throw BitException( L"'" + dirItem.name() + L"' is not a directory!" );
    }
    mDirName = dirItem.name();
}

vector<FSItem> FSIndexer::listFilesInDirectory( bool recursive ) {
    vector<FSItem> result;
    FSIndexer::listFilesInDirectory( result, recursive, L"" );
    return result;
}

vector< FSItem > FSIndexer::listFiles( const vector< wstring >& in_paths ) {
    vector< FSItem > out_files;
    for ( auto itr = in_paths.cbegin(); itr != in_paths.cend(); ++itr ) {
        FSItem item( *itr );
        if ( !item.exists() ) {
            throw BitException( L"Item '" + item.name() + L"' does not exist!" );
        }
        if ( item.isDir() ) {
            FSIndexer indexer( *itr );
            indexer.listFilesInDirectory( out_files, true, L"" );
        } else{
            out_files.push_back( item );
        }
    }
    return out_files;
}

vector< FSItem > FSIndexer::removeListedDirectories( const vector< wstring >& in_paths ) {
    vector< FSItem > out_files;
    for ( auto itr = in_paths.cbegin(); itr != in_paths.cend(); ++itr ) {
        FSItem item( *itr );
        if ( item.exists() && !item.isDir() ) {
            out_files.push_back( item );
        }
    }
    return out_files;
}

void FSIndexer::listFilesInDirectory( vector< FSItem >& result, bool recursive, const wstring& prefix ) {
    wstring filtered_path = mDirectory + L"\\";
    if ( !prefix.empty() ) {
        filtered_path += prefix + L"\\";
    }
    filtered_path += mFilter;
    FSItemInfo data;
    HANDLE hFind = FindFirstFile( filtered_path.c_str(), &data );

    if ( INVALID_HANDLE_VALUE == hFind ) {
        throw BitException( L"Invalid path '" + filtered_path + L"'" );
    }

    do {
        //NOTE: This should definitely be optimized!!
        wstring ndir;
        if ( prefix.empty() ) {
            ndir = mDirectory;
        } else if ( prefix[0] == '\\' || prefix[0] == '/' ) {
            ndir = mDirectory + prefix;
        } else {
            ndir = mDirectory + L"\\" + prefix;
        }

        wstring dirName = mDirName + prefix;

        FSItem currentItem = FSItem( ndir, dirName, data );
        if ( currentItem.isDir() ) {
            if ( recursive && currentItem.name() != L"." && currentItem.name() != L".." ) {
                //wstring nprefix = ( prefix.empty() ) ? currentItem.name() : prefix + L"\\" + currentItem.name();
                wstring listDir = prefix + L"\\" + currentItem.name();
                listFilesInDirectory( result, true, listDir );
            }
        } else {
            result.push_back( currentItem );
        }
    } while ( FindNextFile( hFind, &data ) != 0 );

    FindClose( hFind );
}

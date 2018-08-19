// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/fsindexer.hpp"

#include <string>

#include "../include/fsutil.hpp"
#include "../include/bitexception.hpp"

using std::wstring;
using namespace bit7z::filesystem;

FSIndexer::FSIndexer( const wstring& directory, const wstring& filter ) : mDirItem( directory ), mFilter( filter ) {
    if ( !mDirItem.isDir() ) {
        throw BitException( L"'" + mDirItem.name() + L"' is not a directory!" );
    }
}

// NOTE: It indexes all the items whose metadata are needed in the archive to be created!
void FSIndexer::listDirectoryItems( vector< FSItem >& result, bool recursive, const wstring& prefix ) {
    wstring filtered_path = mDirItem.path();
    if ( !prefix.empty() ) {
        filtered_path += L"\\" + prefix;
    }
    // Listing all files! The filter is applied separately, so we can recurse and match files also in sub directories!
    filtered_path += L"\\*";
    FSItemInfo data;
    HANDLE hFind = FindFirstFile( filtered_path.c_str(), &data );

    if ( INVALID_HANDLE_VALUE == hFind ) {
        throw BitException( L"Invalid path '" + filtered_path + L"'" );
    }

    do {
        wstring ndir = mDirItem.path();
        wstring search_path = !mFilter.empty() ? L"" : mDirItem.inArchivePath();
        if ( !prefix.empty() ) {
            ndir += L"\\" + prefix;
            search_path += search_path.empty() ? prefix : L"\\" + prefix;
        }

        FSItem current_item = FSItem( ndir, data, search_path );
        if ( current_item.isDots() ) {
            continue;
        }

        bool item_matches = fsutil::wildcard_match( mFilter, current_item.name() );
        if ( item_matches ) {
            result.push_back( current_item );
        }

        if ( current_item.isDir() && ( recursive || item_matches ) ) { //currentItem is a directory and we must index it recursively!
            wstring next_dir = prefix.empty() ? current_item.name() : prefix + L"\\" + current_item.name();
            listDirectoryItems( result, true, next_dir );
        }
    } while ( FindNextFile( hFind, &data ) != 0 );

    FindClose( hFind );
}


vector< FSItem > FSIndexer::indexDirectory( const wstring& in_dir, const wstring& filter, bool recursive ) {
    vector< FSItem > result;
    FSItem dir_item( in_dir );
    if ( filter.empty() && !dir_item.inArchivePath().empty() ) {
        result.push_back( dir_item );
    }
    FSIndexer indexer( in_dir, filter );
    indexer.listDirectoryItems( result, recursive );
    return result;
}

vector< FSItem > FSIndexer::indexPaths( const vector< wstring >& in_paths, bool ignore_dirs ) {
    vector< FSItem > out_files;
    for ( auto itr = in_paths.cbegin(); itr != in_paths.cend(); ++itr ) {
        FSItem item( *itr );
        if ( !item.isDir() ) {
            out_files.push_back( item );
        } else if ( !ignore_dirs ) { //item is a directory
            if ( !item.inArchivePath().empty() ) {
                out_files.push_back( item );
            }
            FSIndexer indexer( item.path() );
            indexer.listDirectoryItems( out_files, true );
        }
    }
    return out_files;
}

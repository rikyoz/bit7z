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

#include "../include/fsindexer.hpp"

#include "../include/fsutil.hpp"
#include "../include/bitexception.hpp"

using namespace bit7z::filesystem;

FSIndexer::FSIndexer( const FSItem& directory, const wstring& filter ) : mDirItem( directory ), mFilter( filter ) {
    if ( !mDirItem.isDir() ) {
        throw BitException( L"'" + mDirItem.name() + L"' is not a directory!", ERROR_DIRECTORY );
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
        throw BitException( L"Invalid path '" + filtered_path + L"'", GetLastError() );
    }

    bool include_root_path = mFilter.empty() ||
                             fsutil::dirname( mDirItem.path() ).empty() ||
                             fsutil::filename( mDirItem.inArchivePath() ) != mDirItem.name();

    do {
        wstring ndir = mDirItem.path();
        wstring search_path = include_root_path ? mDirItem.inArchivePath() : L"";
        if ( !prefix.empty() ) {
            ndir += L"\\" + prefix;
            search_path += search_path.empty() ? prefix : L"\\" + prefix;
        }

        FSItem current_item = FSItem( ndir, data, search_path );
        if ( current_item.isDots() ) {
            continue;
        }

        bool item_matches = fsutil::wildcardMatch( mFilter, current_item.name() );
        if ( item_matches ) {
            result.push_back( current_item );
        }

        if ( current_item.isDir() && ( recursive || item_matches ) ) {
            //currentItem is a directory and we must list it only if:
            // > indexing is done recursively
            // > indexing is not recursive but the directory name matched the filter
            wstring next_dir = prefix.empty() ? current_item.name() : prefix + L"\\" + current_item.name();
            listDirectoryItems( result, true, next_dir );
        }
    } while ( FindNextFile( hFind, &data ) != 0 );

    FindClose( hFind );
}

void FSIndexer::indexItem( const FSItem& item, bool ignore_dirs, vector< FSItem >& result ) {
    if ( !item.isDir() ) {
        result.push_back( item );
    } else if ( !ignore_dirs ) { //item is a directory
        if ( !item.inArchivePath().empty() ) {
            result.push_back( item );
        }
        FSIndexer indexer( item );
        indexer.listDirectoryItems( result, true );
    }
}

vector< FSItem > FSIndexer::indexDirectory( const wstring& in_dir, const wstring& filter, bool recursive ) {
    vector< FSItem > result;
    FSItem dir_item( in_dir );
    if ( filter.empty() && !dir_item.inArchivePath().empty() ) {
        result.push_back( dir_item );
    }
    FSIndexer indexer( dir_item, filter );
    indexer.listDirectoryItems( result, recursive );
    return result;
}

vector< FSItem > FSIndexer::indexPaths( const vector< wstring >& in_paths, bool ignore_dirs ) {
    vector< FSItem > out_files;
    for ( const auto& file_path : in_paths ) {
        FSItem item( file_path );
        indexItem( item, ignore_dirs, out_files );
    }
    return out_files;
}

vector< FSItem > FSIndexer::indexPathsMap( const map< wstring, wstring >& in_paths, bool ignore_dirs ) {
    vector< FSItem > out_files;
    for ( const auto& file_pair : in_paths ) {
        FSItem item( file_pair.first, file_pair.second );
        indexItem( item, ignore_dirs, out_files );
    }
    return out_files;
}

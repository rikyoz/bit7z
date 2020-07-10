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

#include "../include/fsindexer.hpp"
#include "../include/bitexception.hpp"

using bit7z::tstring;
using namespace bit7z::filesystem;

FSIndexer::FSIndexer( const FSItem& directory, tstring filter )
    : mDirItem( directory ), mFilter( std::move( filter ) ) {
    if ( !mDirItem.isDir() ) {
        throw BitException( "Invalid path", std::make_error_code( std::errc::not_a_directory ), mDirItem.name() );
    }
}

// NOTE: It indexes all the items whose metadata are needed in the archive to be created!
void FSIndexer::listDirectoryItems( vector< FSItem >& result, bool recursive, const fs::path& prefix ) {
    auto path = mDirItem.path();
    if ( !prefix.empty() ) {
        path = path / prefix;
    }
    bool include_root_path = mFilter.empty() ||
                             mDirItem.path().parent_path().empty() ||
                             mDirItem.inArchivePath().filename() != mDirItem.name();
    std::error_code ec;
    for ( auto& current_entry : fs::directory_iterator( path, ec ) ) {
        auto search_path = include_root_path ? mDirItem.inArchivePath() : fs::path();
        if ( !prefix.empty() ) {
            search_path = search_path.empty() ? prefix : search_path / prefix;
        }

        FSItem current_item{ current_entry, search_path };
        bool item_matches = fsutil::wildcardMatch( mFilter, current_item.name() );
        if ( item_matches ) {
            result.push_back( current_item );
        }

        if ( current_item.isDir() && ( recursive || item_matches ) ) {
            //currentItem is a directory and we must list it only if:
            // > indexing is done recursively
            // > indexing is not recursive but the directory name matched the filter
            fs::path next_dir = prefix.empty() ? fs::path( current_item.name() ) : prefix / current_item.name();
            listDirectoryItems( result, true, next_dir );
        }
    }
}

void FSIndexer::indexItem( const FSItem& item, bool ignore_dirs, vector< FSItem >& result ) {
    if ( !item.isDir() ) {
        result.push_back( item );
    } else if ( !ignore_dirs ) { //item is a directory
        if ( !item.inArchivePath().empty() ) {
            result.push_back( item );
        }
        FSIndexer indexer{ item };
        indexer.listDirectoryItems( result, true );
    }
}

vector< FSItem > FSIndexer::indexDirectory( const fs::path& in_dir, const tstring& filter, bool recursive ) {
    vector< FSItem > result;
    FSItem dir_item{ in_dir };
    if ( filter.empty() && !dir_item.inArchivePath().empty() ) {
        result.push_back( dir_item );
    }
    FSIndexer indexer{ dir_item, filter };
    indexer.listDirectoryItems( result, recursive );
    return result;
}

vector< FSItem > FSIndexer::indexPaths( const vector< tstring >& in_paths, bool ignore_dirs ) {
    vector< FSItem > out_files;
    for ( const auto& file_path : in_paths ) {
        FSItem item{ file_path };
        indexItem( item, ignore_dirs, out_files );
    }
    return out_files;
}

vector< FSItem > FSIndexer::indexPathsMap( const map< tstring, tstring >& in_paths, bool ignore_dirs ) {
    vector< FSItem > out_files;
    for ( const auto& file_pair : in_paths ) {
        FSItem item{ fs::path( file_pair.first ), fs::path( file_pair.second ) };
        indexItem( item, ignore_dirs, out_files );
    }
    return out_files;
}

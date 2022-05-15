// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#include "internal/fsindexer.hpp"

#include "bitexception.hpp"
#include "internal/fsutil.hpp"

using bit7z::GenericInputItem;
using bit7z::tstring;
using namespace bit7z::filesystem;

FSIndexer::FSIndexer( FSItem directory, tstring filter)
    : mDirItem( std::move( directory ) ), mFilter( std::move( filter ) ) {
    if ( !mDirItem.isDir() ) {
        throw BitException( "Invalid path", std::make_error_code( std::errc::not_a_directory ), mDirItem.name() );
    }
}

// NOTE: It indexes all the items whose metadata are needed in the archive to be created!
void FSIndexer::listDirectoryItems( vector< unique_ptr< GenericInputItem > >& result,
                                    bool recursive,
                                    const fs::path& prefix ) {
    fs::path path = mDirItem.path();
    if ( !prefix.empty() ) {
        path = path / prefix;
    }
    bool include_root_path = mFilter.empty() ||
                             fs::path{ mDirItem.path() }.parent_path().empty() ||
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
            result.emplace_back( std::make_unique< FSItem >( current_item ) );
        }

        if ( current_item.isDir() && ( recursive || item_matches ) ) {
            //currentItem is a directory, and we must list it only if:
            // > indexing is done recursively
            // > indexing is not recursive, but the directory name matched the filter.
            fs::path next_dir = prefix.empty() ? fs::path( current_item.name() ) : prefix / current_item.name();
            listDirectoryItems( result, true, next_dir );
        }
    }
}
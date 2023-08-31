// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bitexception.hpp"
#include "internal/fsindexer.hpp"
#include "internal/fsutil.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

FSIndexer::FSIndexer( FSItem directory, tstring filter, FilterPolicy policy, bool only_files )
    : mDirItem{ std::move( directory ) }, mFilter{ std::move( filter ) }, mPolicy{ policy }, mOnlyFiles{ only_files } {
    if ( !mDirItem.isDir() ) {
        throw BitException( "Invalid path", std::make_error_code( std::errc::not_a_directory ), mDirItem.name() );
    }
}

// NOTE: It indexes all the items whose metadata are needed in the archive to be created!
// NOLINTNEXTLINE(misc-no-recursion)
void FSIndexer::listDirectoryItems( vector< unique_ptr< GenericInputItem > >& result,
                                    bool recursive,
                                    const fs::path& prefix ) {
    fs::path path = mDirItem.filesystemPath();
    if ( !prefix.empty() ) {
        path = path / prefix;
    }
    const bool include_root_path = mFilter.empty() ||
                                   mDirItem.filesystemPath().parent_path().empty() ||
                                   mDirItem.inArchivePath().filename() != mDirItem.filesystemName();
    const bool should_include_matched_items = mPolicy == FilterPolicy::Include;
    std::error_code error;
    for ( const auto& current_entry : fs::directory_iterator( path, error ) ) {
        auto search_path = include_root_path ? mDirItem.inArchivePath() : fs::path();
        if ( !prefix.empty() ) {
            search_path = search_path.empty() ? prefix : search_path / prefix;
        }

        const FSItem current_item{ current_entry, search_path };
        /* An item matches if:
         *  - Its name matches the wildcard pattern, and
         *  - Either is a file, or we are interested also to include folders in the index.
         *
         * Note: The boolean expression uses short-circuiting to optimize the evaluation. */
        const bool item_matches = ( !mOnlyFiles || !current_item.isDir() ) &&
                                  fsutil::wildcardMatch( mFilter, current_item.name() );
        if ( item_matches == should_include_matched_items ) {
            result.emplace_back( std::make_unique< FSItem >( current_item ) );
        }

        if ( current_item.isDir() && ( recursive || ( item_matches == should_include_matched_items ) ) ) {
            //currentItem is a directory, and we must list it only if:
            // > indexing is done recursively
            // > indexing is not recursive, but the directory name matched the filter.
            const fs::path next_dir = prefix.empty() ?
                                      current_item.filesystemName() :  prefix / current_item.filesystemName();
            listDirectoryItems( result, true, next_dir );
        }
    }
}

} // namespace filesystem
} // namespace bit7z
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

FilesystemIndexer::FilesystemIndexer( FilesystemItem directory,
                                      tstring filter,
                                      FilterPolicy policy,
                                      SymlinkPolicy symlinkPolicy,
                                      bool onlyFiles )
    : mDirItem{ std::move( directory ) },
      mFilter{ std::move( filter ) },
      mPolicy{ policy },
      mSymlinkPolicy{ symlinkPolicy },
      mOnlyFiles{ onlyFiles } {
    if ( !mDirItem.isDir() ) {
        throw BitException( "Invalid path", std::make_error_code( std::errc::not_a_directory ), mDirItem.name() );
    }
}

// NOTE: It indexes all the items whose metadata are needed in the archive to be created!
// NOLINTNEXTLINE(misc-no-recursion)
void FilesystemIndexer::listDirectoryItems( vector< unique_ptr< GenericInputItem > >& result,
                                            bool recursive,
                                            const fs::path& prefix ) {
    fs::path path = mDirItem.filesystemPath();
    if ( !prefix.empty() ) {
        path = path / prefix;
    }
    const bool includeRootPath = mFilter.empty() ||
                                 !mDirItem.filesystemPath().has_parent_path() ||
                                 mDirItem.inArchivePath().filename() != mDirItem.filesystemName();
    const bool shouldIncludeMatchedItems = mPolicy == FilterPolicy::Include;
    std::error_code error;
    for ( const auto& currentEntry : fs::directory_iterator( path, error ) ) {
        auto searchPath = includeRootPath ? mDirItem.inArchivePath() : fs::path{};
        if ( !prefix.empty() ) {
            searchPath = searchPath.empty() ? prefix : searchPath / prefix;
        }

        const FilesystemItem currentItem{ currentEntry, searchPath, mSymlinkPolicy };
        /* An item matches if:
         *  - Its name matches the wildcard pattern, and
         *  - Either is a file, or we are interested also to include folders in the index.
         *
         * Note: The boolean expression uses short-circuiting to optimize the evaluation. */
        const bool itemMatches = ( !mOnlyFiles || !currentItem.isDir() ) &&
                                 fsutil::wildcard_match( mFilter, currentItem.name() );
        if ( itemMatches == shouldIncludeMatchedItems ) {
            result.emplace_back( std::make_unique< FilesystemItem >( currentItem ) );
        }

        if ( currentItem.isDir() && ( recursive || ( itemMatches == shouldIncludeMatchedItems ) ) ) {
            //currentItem is a directory, and we must list it only if:
            // > indexing is done recursively
            // > indexing is not recursive, but the directory name matched the filter.
            const fs::path nextDir = prefix.empty() ?
                                     currentItem.filesystemName() : prefix / currentItem.filesystemName();
            listDirectoryItems( result, true, nextDir );
        }
    }
}

} // namespace filesystem
} // namespace bit7z
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

#include "internal/fsindexer.hpp"

#include "bitabstractarchivehandler.hpp"
#include "bittypes.hpp"
#include "internal/fsitem.hpp"
#include "internal/fsutil.hpp"
#include "internal/stringutil.hpp"

#include <memory>
#include <vector>
#include <system_error>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

namespace {
auto countItemsInPath( const fs::path& path ) -> std::size_t {
    std::error_code error;
    const auto begin = fs::recursive_directory_iterator{ path, fs::directory_options::skip_permission_denied, error };
    return error ? 0 : static_cast< std::size_t >( std::distance( begin, fs::recursive_directory_iterator{} ) );
}
} // namespace

void listDirectoryItems( const fs::path& basePath,
                         const fs::path& inArchivePath,
                         const tstring& filter,
                         IndexingOptions options,
                         BitItemsVector& result ) {
    std::error_code error;
    const bool includeRootPath = filter.empty() ||
                                 !basePath.has_parent_path() ||
                                 inArchivePath.filename() != fs::canonical( basePath, error ).filename();
    const bool shouldIncludeMatchedItems = options.filterPolicy == FilterPolicy::Include;

    result.reserve( result.size() + countItemsInPath( basePath ) );
    for ( auto iterator = fs::recursive_directory_iterator{ basePath, fs::directory_options::skip_permission_denied, error };
          iterator != fs::recursive_directory_iterator{};
          ++iterator ) {
        const auto& currentEntry = *iterator;
        const auto& itemPath = currentEntry.path();

        const auto itemIsDir = currentEntry.is_directory( error );
        const auto itemName = path_to_tstring( itemPath.filename() );

        /* An item matches if:
         *  - Its name matches the wildcard pattern, and
         *  - Either is a file, or we are interested also to include folders in the index.
         *
         * Note: The boolean expression uses short-circuiting to optimize the evaluation. */
        const bool itemMatches = ( !options.onlyFiles || !itemIsDir ) && fsutil::wildcard_match( filter, itemName );
        if ( itemMatches == shouldIncludeMatchedItems ) {
            const auto prefix = fs::relative( itemPath, basePath, error ).remove_filename();
            const auto searchPath = includeRootPath ? inArchivePath / prefix : prefix;
            result.emplace_back( std::make_unique< FilesystemItem >( searchPath, currentEntry, options.symlinkPolicy ) );
        }

        /* We don't need to recurse inside the current item if:
         *  - it is not a directory; or
         *  - we are not indexing recursively, and the directory's name doesn't match the wildcard filter. */
        if ( !itemIsDir || ( !options.recursive && ( itemMatches != shouldIncludeMatchedItems ) ) ) {
            iterator.disable_recursion_pending();
        }
    }
}

} // namespace filesystem
} // namespace bit7z
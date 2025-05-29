/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FSINDEXER_HPP
#define FSINDEXER_HPP

#include "bititemsvector.hpp"
#include "bittypes.hpp"
#include "internal/fsutil.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

void listDirectoryItems( const fs::path& basePath,
                         const fs::path& inArchivePathStr,
                         const tstring& filter,
                         IndexingOptions options,
                         BitItemsVector& result );

BIT7Z_ALWAYS_INLINE
void listDirectoryItems( const fs::path& basePath,
                         const sevenzip_string& inArchivePathStr,
                         const tstring& filter,
                         IndexingOptions options,
                         BitItemsVector& result ) {
    const fs::path inArchivePath = sevenzip_string_to_path( inArchivePathStr );
    listDirectoryItems( basePath, inArchivePath, filter, options, result );
}

}  // namespace filesystem
}  // namespace bit7z

#endif // FSINDEXER_HPP

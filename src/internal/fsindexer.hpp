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
#include "internal/fsitem.hpp"

#include <vector>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

void listDirectoryItems( const FilesystemItem& directory,
                         const tstring& filter,
                         IndexingOptions options,
                         BitItemsVector& result );

}  // namespace filesystem
}  // namespace bit7z

#endif // FSINDEXER_HPP

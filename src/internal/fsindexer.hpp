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

#include <string>
#include <vector>
#include <map>

#include "bitabstractarchivehandler.hpp"
#include "internal/fsitem.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

using std::vector;
using std::unique_ptr;

class FSIndexer final {
    public:
        explicit FSIndexer( FSItem directory,
                            tstring filter = {},
                            FilterPolicy policy = FilterPolicy::Include,
                            bool only_files = false );

        void listDirectoryItems( vector< unique_ptr< GenericInputItem > >& result,
                                 bool recursive,
                                 const fs::path& prefix = fs::path() );

    private:
        FSItem mDirItem;
        tstring mFilter;
        FilterPolicy mPolicy;
        bool mOnlyFiles;
};

}  // namespace filesystem
}  // namespace bit7z

#endif // FSINDEXER_HPP

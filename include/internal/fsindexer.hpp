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

#ifndef FSINDEXER_HPP
#define FSINDEXER_HPP

#include <string>
#include <vector>
#include <map>

#include "internal/fsitem.hpp"

namespace bit7z {
    namespace filesystem {
        using std::vector;
        using std::map;
        using std::unique_ptr;

        class FSIndexer {
            public:
                explicit FSIndexer( FSItem directory, tstring filter = TSTRING( "" ) );

                void listDirectoryItems( vector< unique_ptr< GenericInputItem > >& result,
                                         bool recursive,
                                         const fs::path& prefix = fs::path() );

            private:
                FSItem mDirItem;
                tstring mFilter;
        };
    }
}
#endif // FSINDEXER_HPP

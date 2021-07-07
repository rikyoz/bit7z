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

#ifndef ITEMSINDEX_HPP
#define ITEMSINDEX_HPP

#include <vector>
#include <map>
#include <memory>

#include "../include/bittypes.hpp"

/* Notes: we use these forward declarations to avoid including private headers (e.g. fs.hpp).
 *        Since some public API headers include itemsindex.hpp (e.g. "bitoutputarchive.hpp"),
 *        including private headers here would result in the "leaking" out of these latter in the public API.*/
#ifdef USE_STANDARD_FILESYSTEM
#include <filesystem>
#else
namespace ghc {
    namespace filesystem {
        class path;
    }
}
#endif

namespace fs {
#ifdef USE_STANDARD_FILESYSTEM
    using namespace std::filesystem;
#else
    using namespace ghc::filesystem;
#endif
}

namespace bit7z {
    using std::vector;
    using std::map;
    using std::unique_ptr;

    namespace filesystem {
        class FSItem;
    }

    using filesystem::FSItem;

    struct GenericItem;

    class ItemsIndex {
        public:
            void indexDirectory( const fs::path& in_dir,
                                 const tstring& filter = TSTRING( "" ),
                                 bool recursive = true );

            void indexPaths( const vector< tstring >& in_paths, bool ignore_dirs = false );

            void indexPathsMap( const map< tstring, tstring >& in_paths, bool ignore_dirs = false );

            void indexFile( const tstring& in_file );

            void indexBuffer( const vector <byte_t>& in_buffer, const tstring& name );

            void indexStream( std::istream& in_stream, const tstring& name );

            size_t size() const;

            const GenericItem& operator[](size_t index) const;

            virtual ~ItemsIndex();

        private:
            vector< unique_ptr< GenericItem > > mItems;

            void indexItem( const FSItem& item, bool ignore_dirs );
    };
}


#endif //ITEMSINDEX_HPP

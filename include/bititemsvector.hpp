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

#ifndef BITITEMSVECTOR_HPP
#define BITITEMSVECTOR_HPP

#include <vector>
#include <map>
#include <memory>

#include "bitfs.hpp"
#include "bittypes.hpp"

namespace bit7z {
    using std::vector;
    using std::map;
    using std::unique_ptr;

    namespace filesystem {
        class FSItem;
    }

    using filesystem::FSItem;

    struct GenericStreamItem;
    using GenericStreamItemPtr = std::unique_ptr< GenericStreamItem >;
    using GenericStreamItemVector = std::vector< GenericStreamItemPtr >;

    class BitItemsVector {
        public:
            using value_type = GenericStreamItemPtr;

            void indexDirectory( const fs::path& in_dir,
                                 const tstring& filter = TSTRING( "" ),
                                 bool recursive = true );

            void indexPaths( const vector< tstring >& in_paths, bool ignore_dirs = false );

            void indexPathsMap( const map< tstring, tstring >& in_paths, bool ignore_dirs = false );

            void indexFile( const tstring& in_file, const tstring& name = TSTRING( "" ) );

            void indexBuffer( const vector< byte_t >& in_buffer, const tstring& name );

            void indexStream( std::istream& in_stream, const tstring& name );

            size_t size() const;

            const GenericStreamItem& operator[]( size_t index) const;

            GenericStreamItemVector::const_iterator begin() const noexcept;

            GenericStreamItemVector::const_iterator end() const noexcept;

            GenericStreamItemVector::const_iterator cbegin() const noexcept;

            GenericStreamItemVector::const_iterator cend() const noexcept;

            virtual ~BitItemsVector();

        private:
            GenericStreamItemVector mItems;

            void indexItem( const FSItem& item, bool ignore_dirs );
    };
}


#endif //BITITEMSVECTOR_HPP

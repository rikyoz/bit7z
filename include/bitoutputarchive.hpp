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

#ifndef BITOUTPUTARCHIVE_HPP
#define BITOUTPUTARCHIVE_HPP

#include "../include/bitarchivecreator.hpp"

#include "../include/itemsindex.hpp"

#include <istream>

namespace bit7z {
    using std::istream;

    class BitOutputArchive {
        public:
            explicit BitOutputArchive( const BitArchiveCreator& creator, tstring in_file = TSTRING( "" ) );

            BitOutputArchive( const BitArchiveCreator& creator, const vector< byte_t >& in_buffer );

            BitOutputArchive( const BitArchiveCreator& creator, std::istream& in_stream );

            void addItems( const vector< tstring >& in_paths );

            void addItems( const map< tstring, tstring >& in_paths );

            void addFile( const tstring& in_file, const tstring& name = TSTRING( "" ) );

            void addFile( const vector< byte_t >& in_buffer, const tstring& name );

            void addFile( istream& in_stream, const tstring& name );

            void addFiles( const vector< tstring >& in_files );

            void addFiles( const tstring& in_dir, bool recursive = true, const tstring& filter = TSTRING( "*.*" ) );

            void addDirectory( const tstring& in_dir );

            void compressTo( const tstring& out_file ) const;

            void compressTo( std::vector< byte_t >& out_buffer ) const;

            void compressTo( ostream& out_stream ) const;

            virtual ~BitOutputArchive() = default;

        protected:
            unique_ptr< BitInputArchive > mInputArchive;
            ItemsIndex mNewItemsIndex;

            virtual CMyComPtr< UpdateCallback > initUpdateCallback() const;

        private:
            const BitArchiveCreator& mArchiveCreator;

            CMyComPtr< IOutArchive > initOutArchive() const;

            CMyComPtr< IOutStream > initOutFileStream( const tstring& out_archive, bool updating_archive ) const;

            void compressOut( IOutArchive* out_arc,
                              IOutStream* out_stream,
                              UpdateCallback* update_callback ) const;

            void compressToFile( const tstring& out_file, UpdateCallback* update_callback ) const;

            void setArchiveProperties( IOutArchive* out_archive ) const;
    };
}

#endif //BITOUTPUTARCHIVE_HPP

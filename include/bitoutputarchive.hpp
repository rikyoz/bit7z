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

#include "bitarchivecreator.hpp"

#include "bitpropvariant.hpp"
#include "bitexception.hpp"
#include "itemsvector.hpp"

#include <istream>
#include <set>

struct ISequentialInStream;

namespace bit7z {
    using std::istream;

    using DeletedItems = std::set< uint32_t >;

    /* General note: I tried my best to explain how indices work here, but it's a bit complex. */

    /* We introduce a strong index type to differentiate between indices in the output
     * archive (uint32_t, as used by the UpdateCallback) and the corresponding indexes
     * in the input archive (input_index). In this way, we avoid implicit conversions
     * between the two kinds of indices.
     *
     * UpdateCallback uses indices in the range [0, BitOutputArchive::itemsCount())
     *
     * Now, if the user doesn't delete any item in the input archive, itemsCount()
     * is just equal to <n. of items in the input archive> + <n. of newly added items>.
     * In this case, an input_index value is just equal to the index used by UpdateCallback.
     *
     * On the contrary, if the user wants to delete an item in the input archive, the value
     * of a input_index may differ from the corresponding UpdateCallback's index.
     *
     * Note: given an input_index i:
     *         if i <  mInputArchiveItemsCount, the item is old (old item in the input archive);
     *         if i >= mInputArchiveItemsCount, the item is new (added by the user); */
    enum class input_index : uint32_t {};

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

            void compressTo( const tstring& out_file );

            void compressTo( std::vector< byte_t >& out_buffer );

            void compressTo( ostream& out_stream );

            BitPropVariant getOutputItemProperty( uint32_t index, BitProperty propID ) const;

            HRESULT getOutputItemStream( uint32_t index, ISequentialInStream** inStream ) const;

            uint32_t itemsCount() const;

            virtual bool hasNewData( uint32_t index ) const;

            virtual bool hasNewProperties( uint32_t index ) const;

            uint32_t getIndexInArchive( uint32_t index ) const;

            const BitArchiveHandler& getHandler() const;

            virtual ~BitOutputArchive() = default;

        protected:
            unique_ptr< BitInputArchive > mInputArchive;
            uint32_t mInputArchiveItemsCount;

            ItemsVector mNewItemsVector;
            DeletedItems mDeletedItems;

            mutable FailedFiles mFailedFiles;

            /* mInputIndices:
             *   Position i = index in range [0, itemsCount()) used by UpdateCallback.
             *   Value at pos. i = corresponding index in the input archive (type input_index).
             *
             * if there are some deleted items, then i != mInputIndices[i]
             * (at least for values of i greater than the index of the first deleted item).
             *
             * if there are no deleted items, mInputIndices is empty, and getItemInputIndex(i)
             * will return input_index with value i.
             *
             * This vector is either empty or it has size equal to itemsCount() (thanks to updateInputIndices()). */
            std::vector< input_index > mInputIndices;

            input_index getItemInputIndex( uint32_t new_index ) const;

            virtual BitPropVariant getItemProperty( input_index index, BitProperty prop ) const;

            virtual HRESULT getItemStream( input_index index, ISequentialInStream** inStream ) const;

        private:
            const BitArchiveCreator& mArchiveCreator;

            CMyComPtr< IOutArchive > initOutArchive() const;

            CMyComPtr< IOutStream > initOutFileStream( const tstring& out_archive, bool updating_archive ) const;

            void compressToFile( const tstring& out_file, UpdateCallback* update_callback );

            void compressOut( IOutArchive* out_arc,
                              IOutStream* out_stream,
                              UpdateCallback* update_callback );

            void setArchiveProperties( IOutArchive* out_archive ) const;

            void updateInputIndices();
    };
}

#endif //BITOUTPUTARCHIVE_HPP

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2020  Riccardo Ostani - All Rights Reserved.
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

#ifndef BITINPUTARCHIVE_H
#define BITINPUTARCHIVE_H

#include "../include/bitarchivehandler.hpp"
#include "../include/bitarchiveiteminfo.hpp"
#include "../include/bitarchiveitemoffset.hpp"
#include "../include/bitformat.hpp"
#include "../include/bitpropvariant.hpp"
#include "../include/bittypes.hpp"

#include <vector>
#include <string>
#include <cstdint>

struct IInStream;
struct IInArchive;
struct IOutArchive;
struct IArchiveExtractCallback;

namespace bit7z {
    using std::vector;

    class ExtractCallback;

    class BitInputArchive {
        public:
            BitInputArchive( const BitArchiveHandler& handler, tstring in_file );

            BitInputArchive( const BitArchiveHandler& handler, const vector< byte_t >& in_buffer );

            BitInputArchive( const BitArchiveHandler& handler, std::istream& in_stream );

            virtual ~BitInputArchive();

#ifdef BIT7Z_AUTO_FORMAT
            /**
             * @return the detected format of the file.
             */
            const BitInFormat& detectedFormat() const;
#endif

            /**
             * @brief Gets the specified archive property.
             *
             * @param property  the property to be retrieved.
             *
             * @return the current value of the archive property or an empty BitPropVariant if no value is specified.
             */
            BitPropVariant getArchiveProperty( BitProperty property ) const;

            /**
             * @brief Gets the specified property of an item in the archive.
             *
             * @param index     the index (in the archive) of the item.
             * @param property  the property to be retrieved.
             *
             * @return the current value of the item property or an empty BitPropVariant if the item has no value for
             * the property.
             */
            BitPropVariant getItemProperty( uint32_t index, BitProperty property ) const;

            /**
             * @return the number of items contained in the archive.
             */
            uint32_t itemsCount() const;

            /**
             * @param index the index of an item in the archive.
             *
             * @return true if and only if the item at index is a folder.
             */
            bool isItemFolder( uint32_t index ) const;

            /**
             * @param index the index of an item in the archive.
             *
             * @return true if and only if the item at index is encrypted.
             */
            bool isItemEncrypted( uint32_t index ) const;

            const tstring& getArchivePath() const;

        protected:
            IInArchive* openArchiveStream( const BitArchiveHandler& handler,
                                           const tstring& name,
                                           IInStream* in_stream );

            HRESULT initUpdatableArchive( IOutArchive** newArc ) const;

            void extract( const vector< uint32_t >& indices, ExtractCallback* extract_callback ) const;

            void test( ExtractCallback* extract_callback ) const;

            HRESULT close() const;

            friend class BitArchiveOpener;

            friend class BitArchiveCreator;

        private:
            IInArchive* mInArchive;
#ifdef BIT7Z_AUTO_FORMAT
            const BitInFormat* mDetectedFormat;
#endif
            const tstring mArchivePath;

        public:
            class const_iterator {
                public:
                    // iterator traits
                    using iterator_category = std::input_iterator_tag;
                    using value_type = BitArchiveItemOffset;
                    using reference = const BitArchiveItemOffset&;
                    using pointer = const BitArchiveItemOffset*;
                    using difference_type = uint32_t; //so that count_if returns a uint32_t

                    const_iterator& operator++();

                    const_iterator operator++( int );

                    bool operator==( const const_iterator& other ) const;

                    bool operator!=( const const_iterator& other ) const;

                    reference operator*();

                    pointer operator->();

                private:
                    BitArchiveItemOffset mItemOffset;

                    const_iterator( uint32_t item_index, const BitInputArchive& item_archive );

                    friend class BitInputArchive;
            };

            const_iterator begin() const noexcept;

            const_iterator end() const noexcept;

            const_iterator cbegin() const noexcept;

            const_iterator cend() const noexcept;
    };
}

#endif //BITINPUTARCHIVE_H

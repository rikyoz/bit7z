/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#ifndef BITARCHIVEITEM_HPP
#define BITARCHIVEITEM_HPP

#include "../include/bitpropvariant.hpp"

#include "../include/bittypes.hpp"

namespace bit7z {

    class BitArchiveItem {
        public:
            /**
             * @brief BitArchiveItem destructor.
             */
            virtual ~BitArchiveItem() = default;

            /**
             * @return the index of the item in the archive.
             */
            uint32_t index() const;

            /**
             * @return true if and only if the item is a directory (i.e. it has the property BitProperty::IsDir).
             */
            bool isDir() const;

            /**
             * @return the name of the item, if available or inferable from the path, or an empty string otherwise.
             */
            tstring name() const;

            /**
             * @return the extension of the item, if available or inferable from the name, or an empty string otherwise
             * (e.g. when the item is a folder).
             */
            tstring extension() const;

            /**
             * @return the path of the item in the archive, if available or inferable from the name, or an empty string
             * otherwise.
             */
            tstring path() const;

            /**
             * @return the uncompressed size of the item.
             */
            uint64_t size() const;

            /**
             * @return the compressed size of the item.
             */
            uint64_t packSize() const;

            /**
             * @return true if and only if the item is encrypted.
             */
            bool isEncrypted() const;

            /**
             * @brief Gets the specified item property.
             *
             * @param property  the property to be retrieved.
             *
             * @return the value of the item property, if available, or an empty BitPropVariant.
             */
            virtual BitPropVariant getProperty( BitProperty property ) const = 0;

        protected:
            uint32_t mItemIndex; //Note: it is not const since sub-class BitArchiveItemOffset can increment it!

            explicit BitArchiveItem( uint32_t item_index );
    };

}

#endif // BITARCHIVEITEM_HPP

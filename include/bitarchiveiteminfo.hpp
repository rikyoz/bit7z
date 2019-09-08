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

#ifndef BITARCHIVEITEMINFO_HPP
#define BITARCHIVEITEMINFO_HPP

#include <map>

#include "../include/bitarchiveitem.hpp"

namespace bit7z {
    using std::wstring;
    using std::map;

    /**
     * @brief The BitArchiveItem class represents an item contained in an archive and contains all its properties.
     */
    class BitArchiveItemInfo : public BitArchiveItem {
        public:
            /**
             * @brief Gets the specified item property.
             *
             * @param property  the property to be retrieved.
             *
             * @return the value of the item property, if available, or an empty BitPropVariant.
             */
            BitPropVariant getProperty( BitProperty property ) const override;

            /**
             * @return a map of all the available (i.e. non empty) item properties and their respective values.
             */
            map< BitProperty, BitPropVariant > itemProperties() const;

        private:
            map< BitProperty, BitPropVariant > mItemProperties;

            /* BitArchiveItem objects can be created and updated only by BitArchiveReader */
            explicit BitArchiveItemInfo( uint32_t item_index );

            void setProperty( BitProperty property, const BitPropVariant& value );

            friend class BitArchiveInfo;
    };
}

#endif // BITARCHIVEITEMINFO_HPP

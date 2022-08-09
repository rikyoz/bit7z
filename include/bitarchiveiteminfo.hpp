/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITARCHIVEITEMINFO_HPP
#define BITARCHIVEITEMINFO_HPP

#include <map>

#include "bitarchiveitem.hpp"

namespace bit7z {
using std::wstring;
using std::map;

/**
 * @brief The BitArchiveItemInfo class represents an archived item and that stores all its properties for later use.
 */
class BitArchiveItemInfo final : public BitArchiveItem {
    public:
        /**
         * @brief Gets the specified item property.
         *
         * @param property  the property to be retrieved.
         *
         * @return the value of the item property, if available, or an empty BitPropVariant.
         */
        BIT7Z_NODISCARD BitPropVariant itemProperty( BitProperty property ) const override;

        /**
         * @return a map of all the available (i.e., non-empty) item properties and their respective values.
         */
        BIT7Z_NODISCARD map< BitProperty, BitPropVariant > itemProperties() const;

    private:
        map< BitProperty, BitPropVariant > mItemProperties;

        /* BitArchiveItem objects can be created and updated only by BitArchiveReader */
        explicit BitArchiveItemInfo( uint32_t item_index );

        void setProperty( BitProperty property, const BitPropVariant& value );

        friend class BitArchiveReader;
};
}  // namespace bit7z

#endif // BITARCHIVEITEMINFO_HPP

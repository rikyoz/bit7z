/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITGENERICITEM_HPP
#define BITGENERICITEM_HPP

#include "bitpropvariant.hpp"

namespace bit7z {
/**
 * @bfrief The BitGenericItem interface class represents a generic item (either inside or outside an archive).
 */
class BitGenericItem {
    public:
        /**
         * @return true if and only if the item is a directory (i.e., it has the property BitProperty::IsDir).
         */
        BIT7Z_NODISCARD virtual bool isDir() const = 0;

        /**
         * @return the uncompressed size of the item.
         */
        BIT7Z_NODISCARD virtual uint64_t size() const = 0;

        /**
         * @return the name of the item, if available or inferable from the path, or an empty string otherwise.
         */
        BIT7Z_NODISCARD virtual tstring name() const = 0;

        /**
         * @return the path of the item.
         */
        BIT7Z_NODISCARD virtual tstring path() const = 0;

        /**
         * @return the item attributes.
         */
        BIT7Z_NODISCARD virtual uint32_t attributes() const = 0;

        /**
         * @brief Gets the specified item property.
         *
         * @param property  the property to be retrieved.
         *
         * @return the value of the item property, if available, or an empty BitPropVariant.
         */
        BIT7Z_NODISCARD virtual BitPropVariant itemProperty( BitProperty property ) const = 0;

        virtual ~BitGenericItem() = default;
};
}  // namespace bit7z

#endif //BITGENERICITEM_HPP

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITARCHIVEITEMOFFSET_HPP
#define BITARCHIVEITEMOFFSET_HPP

#include "bitarchiveitem.hpp"

namespace bit7z {
class BitInputArchive;

/**
 * @brief The BitArchiveItemOffset class represents an archived item but doesn't store its properties.
 */
class BitArchiveItemOffset final : public BitArchiveItem {
    public:
        BitArchiveItemOffset& operator++() noexcept;

        BitArchiveItemOffset operator++( int ) noexcept;

        bool operator==( const BitArchiveItemOffset& other ) const noexcept;

        bool operator!=( const BitArchiveItemOffset& other ) const noexcept;

        /**
         * @brief Gets the specified item property.
         *
         * @param property  the property to be retrieved.
         *
         * @return the value of the item property, if available, or an empty BitPropVariant.
         */
        BIT7Z_NODISCARD BitPropVariant itemProperty( BitProperty property ) const override;

    private:
        /* Note: a pointer, instead of a reference, allows this class, and hence BitInputArchive::const_iterator,
         * to be CopyConstructible so that stl algorithms can be used with const_iterator! */
        const BitInputArchive* mArc;

        BitArchiveItemOffset( uint32_t item_index, const BitInputArchive& item_arc ) noexcept;

        friend class BitInputArchive;
};
}  // namespace bit7z

#endif // BITARCHIVEITEMOFFSET_HPP

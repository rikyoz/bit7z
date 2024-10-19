/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITARCHIVEITEMOFFSET_HPP
#define BITARCHIVEITEMOFFSET_HPP

#include "bitarchiveitem.hpp"
#include "bitdefines.hpp"
#include "bitpropvariant.hpp"

#include <cstdint>
#include <functional>

namespace bit7z {

class BitInputArchive;

/**
 * @brief The BitArchiveItemOffset class represents an archived item but doesn't store its properties.
 */
class BitArchiveItemOffset final : public BitArchiveItem {
    public:
        auto operator++() noexcept -> BitArchiveItemOffset&;

        auto operator++( int ) noexcept -> BitArchiveItemOffset; // NOLINT(cert-dcl21-cpp)

        auto operator==( const BitArchiveItemOffset& other ) const noexcept -> bool;

        auto operator!=( const BitArchiveItemOffset& other ) const noexcept -> bool;

        /**
         * @brief Gets the specified item property.
         *
         * @param property  the property to be retrieved.
         *
         * @return the value of the item property, if available, or an empty BitPropVariant.
         */
        BIT7Z_NODISCARD auto itemProperty( BitProperty property ) const -> BitPropVariant override;

        /**
         * @brief Checks whether the item has the specified property or not.
         *
         * @param property the property to be checked.
         *
         * @return true if the item has the property, false otherwise.
         */
        BIT7Z_NODISCARD auto hasProperty( BitProperty property ) const -> bool;

    private:
        /* Note: we use a std::reference_wrapper to make this class, and hence BitInputArchive::ConstIterator,
         * to be CopyConstructible so that stl algorithms can be used with ConstIterator. */
        std::reference_wrapper< const BitInputArchive > mArc;

        BitArchiveItemOffset( const BitInputArchive& inputArchive, std::uint32_t itemIndex ) noexcept;

        friend class BitInputArchive;
};

}  // namespace bit7z

#endif // BITARCHIVEITEMOFFSET_HPP

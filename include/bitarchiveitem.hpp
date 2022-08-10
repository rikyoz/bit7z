/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITARCHIVEITEM_HPP
#define BITARCHIVEITEM_HPP

#include "bitgenericitem.hpp"

namespace bit7z {

/**
 * The BitArchiveItem class represents a generic item inside an archive.
 */
class BitArchiveItem : public BitGenericItem {
    public:
        /**
         * @brief BitArchiveItem destructor.
         */
        ~BitArchiveItem() override = default;

        /**
         * @return the index of the item in the archive.
         */
        BIT7Z_NODISCARD uint32_t index() const noexcept;

        /**
         * @return true if and only if the item is a directory (i.e., it has the property BitProperty::IsDir).
         */
        BIT7Z_NODISCARD bool isDir() const override;

        /**
         * @return the item's name; if not available, it tries to get it from the element's path or,
         *         if not possible, it returns an empty string.
         */
        BIT7Z_NODISCARD tstring name() const override;

        /**
         * @return the extension of the item, if available or if it can be inferred from the name;
         *         otherwise it returns an empty string (e.g., when the item is a folder).
         */
        BIT7Z_NODISCARD tstring extension() const;

        /**
         * @return the path of the item in the archive, if available or inferable from the name, or an empty string
         *         otherwise.
         */
        BIT7Z_NODISCARD tstring path() const override;

        /**
         * @return the uncompressed size of the item.
         */
        BIT7Z_NODISCARD uint64_t size() const override;

        /**
         * @return the item creation time.
         */
        BIT7Z_NODISCARD time_type creationTime() const;

        /**
         * @return the item last access time.
         */
        BIT7Z_NODISCARD time_type lastAccessTime() const;

        /**
         * @return the item last write time.
         */
        BIT7Z_NODISCARD time_type lastWriteTime() const;

        /**
         * @return the item attributes.
         */
        BIT7Z_NODISCARD uint32_t attributes() const override;

        /**
         * @return the compressed size of the item.
         */
        BIT7Z_NODISCARD uint64_t packSize() const;

        /**
         * @return true if and only if the item is encrypted.
         */
        BIT7Z_NODISCARD bool isEncrypted() const;

    protected:
        uint32_t mItemIndex; //Note: it is not const since the subclass BitArchiveItemOffset can increment it!

        explicit BitArchiveItem( uint32_t item_index ) noexcept;
};

}  // namespace bit7z

#endif // BITARCHIVEITEM_HPP

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITNESTEDARCHIVEREADER_HPP
#define BITNESTEDARCHIVEREADER_HPP

#include "bitabstractarchiveopener.hpp"
#include "bitarchiveiteminfo.hpp"
#include "bitinputarchive.hpp"

namespace bit7z {

class BitNestedArchiveReader final : public BitAbstractArchiveOpener {
        BitInputArchive mArchive;
        const BitInputArchive& mParentArchive;
        std::uint32_t mIndexInParent;
        std::uint64_t mMaxMemoryUsage;

        mutable std::uint32_t mCachedItemsCount;
        mutable std::uint32_t mLastReadItem; // TODO: Use std::optional< std::uint32_t > once we move to C++17
        mutable std::size_t mOpenCount;

        void openSequentially() const;

        BIT7Z_NODISCARD
        auto needReopen( std::uint32_t index = 0 ) const -> bool;

        BIT7Z_NODISCARD
        auto calculateItemsCount() const -> std::uint32_t;

    public:
        BitNestedArchiveReader( const Bit7zLibrary& lib,
                                BitInputArchive& inArchive,
                                const BitInFormat& format,
                                const tstring& password = {},
                                std::uint32_t index = 0 );

        BIT7Z_NODISCARD
        auto maxMemoryUsage() const noexcept -> std::uint64_t;

        /**
         * @return the detected format of the file.
         */
        BIT7Z_NODISCARD
        auto detectedFormat() const noexcept -> const BitInFormat&;

        /**
         * @brief Gets the specified archive property.
         *
         * @param property  the property to be retrieved.
         *
         * @return the current value of the archive property or an empty BitPropVariant if no value is specified.
         */
        BIT7Z_NODISCARD
        auto archiveProperty( BitProperty property ) const -> BitPropVariant;

        /**
         * @brief Gets the specified property of an item in the archive.
         *
         * @param index     the index (in the archive) of the item.
         * @param property  the property to be retrieved.
         *
         * @return the current value of the item property or an empty BitPropVariant if the item has no value for
         * the property.
         */
        BIT7Z_NODISCARD
        auto itemProperty( std::uint32_t index, BitProperty property ) const -> BitPropVariant;

        /**
         * @return the number of items contained in the archive.
         */
        BIT7Z_NODISCARD
        auto itemsCount() const -> std::uint32_t;

        /**
         * @return a vector of all the archive items as BitArchiveItem objects.
         */
        BIT7Z_NODISCARD
        auto items() const -> std::vector< BitArchiveItemInfo >;

        /**
         * @brief Extracts the archive to the chosen directory.
         *
         * @param outDir   the output directory where the extracted files will be put.
         */
        void extractTo( const tstring& outDir ) const;

        /**
         * @brief Extracts the content of the archive to a map of memory buffers, where the keys are the paths
         * of the files (inside the archive), and the values are their decompressed contents.
         *
         * @param outMap   the output map.
         */
        void extractTo( std::map< tstring, buffer_t >& outMap ) const;

        /**
         * @brief Tests the archive without extracting its content.
         *
         * @throws BitException if the archive is not valid.
         */
        void test() const;

        BIT7Z_NODISCARD
        auto openCount() const -> std::size_t;

        void setMaxMemoryUsage( std::uint64_t value ) noexcept;
};

} // namespace bit7z

#endif //BITNESTEDARCHIVEREADER_HPP

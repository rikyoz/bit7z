/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
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

/**
 * @brief The BitNestedArchiveReader class allows reading and extracting nested archives
 *        (e.g., the tarball inside a .tar.gz archive).
 *
 * @note Unlike BitInputArchive, this class intentionally does not expose index-based
 * extractTo/test overloads. The nested archive is opened through a forward-only sequential
 * stream, and at least one supported nested format (TAR) only discovers its items lazily,
 * interleaved with extraction; passing it an explicit index list reads past the end of its
 * (still empty) item table. Only extract-everything and FilterCallback/RenameCallback-based
 * extraction are safe under this opening mode, so those are the only overloads offered here.
 *
 * @note This class does not support reentrant calls on the same instance: calling any of its
 * operations that may need to access the underlying sequential stream (including indirectly,
 * e.g., from a FilterCallback/RenameCallback/ItemBufferCallback, or from a FileCallback/
 * ProgressCallback registered on this object) while another such operation on the same instance
 * is already in progress throws a BitException, instead of silently reopening or reading from the
 * stream while it is still in use further up the stack. The only exception is a call that can be
 * answered entirely from a result already cached by a prior successful call (e.g., itemsCount()
 * once it has been computed once): since that touches no shared state, it is always safe to call,
 * including reentrantly.
 */
class BitNestedArchiveReader final : public BitAbstractArchiveOpener {
    public:
        /**
         * @brief Constructs a BitNestedArchiveReader object.
         *
         * @note The constructor doesn't open the archive, it will be opened only when needed.
         *
         * @param lib           the 7z library used.
         * @param parentArchive the parent archive containing the nested archive.
         * @param index         the index of the nested archive within the parent archive.
         * @param format        the format of the nested archive.
         * @param password      (optional) the password needed for opening the nested archive.
         *
         * @throws BitException if the format is BitFormat::Auto
         *                      (automatic format detection of nested archives is not supported).
         */
        BitNestedArchiveReader(
            const Bit7zLibrary& lib,
            const BitInputArchive& parentArchive,
            std::uint32_t index,
            const BitInFormat& format,
            const tstring& password = {}
        );

        /**
         * @brief Constructs a BitNestedArchiveReader object of the first item in the parent archive.
         *
         * @note The constructor doesn't open the archive, it will be opened only when needed.
         *
         * @param lib           the 7z library used.
         * @param parentArchive the parent archive containing the nested archive.
         * @param format        the format of the nested archive.
         * @param password      (optional) the password needed for opening the nested archive.
         *
         * @throws BitException if the format is BitFormat::Auto
         *                      (automatic format detection of nested archives is not supported).
         */
        BitNestedArchiveReader(
            const Bit7zLibrary& lib,
            const BitInputArchive& parentArchive,
            const BitInFormat& format,
            const tstring& password = {}
        );

        /**
         * @return the max memory usage limit applied while extracting the parent archive.
         */
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
         *
         * @throws BitException if index equals std::numeric_limits<std::uint32_t>::max(), a value
         *                      reserved to mean "no item" and that no real item can ever have.
         */
        BIT7Z_NODISCARD
        auto itemProperty( std::uint32_t index, BitProperty property ) const -> BitPropVariant;

        /**
         * @return the number of items contained in the archive.
         *
         * @note Once successfully computed, the result is cached: further calls return it
         * immediately without touching the underlying stream, so, unlike this class's other
         * operations, they remain safe to call even while another operation on this instance
         * is already in progress (see the class-level @note above).
         *
         * @throws BitException if the number of items could not be determined.
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
         * @brief Extracts to the output directory all the items that satisfy the given filtering criteria.
         *
         * @param outDir            the output directory where extracted files will be put.
         * @param filterCallback    the filtering callback that specifies whether to extract an item or not.
         */
        void extractTo( const tstring& outDir, FilterCallback filterCallback ) const;

        /**
         * @brief Extracts the archive to the chosen directory,
         * specifying the names of the extracted items via a RenameCallback.
         *
         * @note The callback receives the archive item being extracted and must return the path
         * that the extracted item must have on the filesystem.
         * If the path of the item must not change, simply return the item's path in the callback.
         * If the item must not be extracted, return an empty string in the callback.
         *
         * @param outDir            the output directory where the extracted files will be put.
         * @param renameCallback    the callback that returns the names for the extracted files.
         */
        void extractTo( const tstring& outDir, RenameCallback renameCallback ) const;

        /**
         * @brief Extracts the content of the archive to a map of memory buffers, where the keys are the paths
         * of the files (inside the archive), and the values are their decompressed contents.
         *
         * @param outMap   the output map.
         */
        void extractTo( std::map< tstring, buffer_t >& outMap ) const;

        /**
         * @brief Extracts to the buffers provided by the given ItemBufferCallback
         *        all the items that satisfy the given filtering criteria.
         *
         * @param callback          the function providing the buffers.
         * @param filterCallback    the filtering callback that specifies whether to extract an item or not.
         */
        void extractTo( ItemBufferCallback callback, FilterCallback filterCallback ) const;

        /**
         * @brief Tests the archive without extracting its content.
         *
         * @throws BitException if the archive is not valid.
         */
        void test() const;

        /**
         * @return the number of times the parent archive was extracted, and the nested archive was opened.
         */
        BIT7Z_NODISCARD
        auto openCount() const -> std::size_t;

        /**
         * @brief Sets the max memory usage limit to be used while extracting the parent archive.
         *
         * @param value the max memory limit to be used (in bytes).
         */
        void setMaxMemoryUsage( std::uint64_t value ) noexcept;

    private:
        BitInputArchive mNestedArchive;
        const BitInputArchive& mParentArchive;
        std::uint32_t mIndexInParent;
        std::uint64_t mMaxMemoryUsage;

        // max() means "not cached yet": 0 is a legitimate item count (an empty nested archive).
        // We can't use our internal Optional because these are value variables in the public API.
        mutable std::uint32_t mCachedItemsCount; // TODO: Use std::optional< std::uint32_t > once we move to C++17.
        mutable std::uint32_t mLastReadItem; // TODO: Use std::optional< std::uint32_t > once we move to C++17
        mutable std::size_t mOpenCount;
        mutable bool mOperationInProgress; // Reentrancy guard: see the class-level @note above.

        void openSequentially() const;

        void reopenIfNeeded( std::uint32_t index = 0 ) const;

        // Guards operation, reopening the nested archive first if needed, and returns its result
        // (if any). Used by itemProperty() and the extractTo overloads/test(), which share this
        // exact "guard, reopen, delegate" shape.
        template< typename Operation >
        auto withReentrancyGuard( Operation&& operation, std::uint32_t index = 0 ) const -> decltype( operation() );

        BIT7Z_NODISCARD
        auto needReopen( std::uint32_t index = 0 ) const noexcept -> bool;

        BIT7Z_NODISCARD
        auto calculateItemsCount() const -> std::uint32_t;
};

} // namespace bit7z

#endif //BITNESTEDARCHIVEREADER_HPP

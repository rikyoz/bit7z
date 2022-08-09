/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef BITINPUTARCHIVE_HPP
#define BITINPUTARCHIVE_HPP

#include <array>
#include <map>

#include "bitabstractarchivehandler.hpp"
#include "bitarchiveitemoffset.hpp"
#include "bitformat.hpp"

struct IInStream;
struct IInArchive;
struct IOutArchive;

namespace bit7z {
using std::vector;

/**
 * @brief The BitInputArchive class, given a handler object, allows reading/extracting the content of archives.
 */
class BitInputArchive {
    public:
        /**
         * @brief Constructs a BitInputArchive object, opening the input file archive.
         *
         * @param handler the reference to the BitAbstractArchiveHandler object containing all the settings to
         *                be used for reading the input archive
         * @param in_file the path to the input archive file
         */
        BitInputArchive( const BitAbstractArchiveHandler& handler, tstring in_file );

        /**
         * @brief Constructs a BitInputArchive object, opening the archive given in the input buffer.
         *
         * @param handler   the reference to the BitAbstractArchiveHandler object containing all the settings to
         *                  be used for reading the input archive
         * @param in_buffer the buffer containing the input archive
         */
        BitInputArchive( const BitAbstractArchiveHandler& handler, const vector< byte_t >& in_buffer );

        /**
         * @brief Constructs a BitInputArchive object, opening the archive by reading the given input stream.
         *
         * @param handler   the reference to the BitAbstractArchiveHandler object containing all the settings to
         *                  be used for reading the input archive
         * @param in_stream the standard input stream of the input archive
         */
        BitInputArchive( const BitAbstractArchiveHandler& handler, std::istream& in_stream );

        BitInputArchive( const BitInputArchive& ) = delete;

        BitInputArchive( BitInputArchive&& ) = delete;

        BitInputArchive& operator=( const BitInputArchive& ) = delete;

        BitInputArchive& operator=( BitInputArchive&& ) = delete;

        virtual ~BitInputArchive();

        /**
         * @return the detected format of the file.
         */
        BIT7Z_NODISCARD const BitInFormat& detectedFormat() const noexcept;

        /**
         * @brief Gets the specified archive property.
         *
         * @param property  the property to be retrieved.
         *
         * @return the current value of the archive property or an empty BitPropVariant if no value is specified.
         */
        BIT7Z_NODISCARD BitPropVariant archiveProperty( BitProperty property ) const;

        /**
         * @brief Gets the specified property of an item in the archive.
         *
         * @param index     the index (in the archive) of the item.
         * @param property  the property to be retrieved.
         *
         * @return the current value of the item property or an empty BitPropVariant if the item has no value for
         * the property.
         */
        BIT7Z_NODISCARD BitPropVariant itemProperty( uint32_t index, BitProperty property ) const;

        /**
         * @return the number of items contained in the archive.
         */
        BIT7Z_NODISCARD uint32_t itemsCount() const;

        /**
         * @param index the index of an item in the archive.
         *
         * @return true if and only if the item at the given index is a folder.
         */
        BIT7Z_NODISCARD bool isItemFolder( uint32_t index ) const;

        /**
         * @param index the index of an item in the archive.
         *
         * @return true if and only if the item at the given index is encrypted.
         */
        BIT7Z_NODISCARD bool isItemEncrypted( uint32_t index ) const;

        /**
         * @return the path to the archive (the empty string for buffer/stream archives).
         */
        BIT7Z_NODISCARD const tstring& archivePath() const noexcept;

        /**
         * @return the BitAbstractArchiveHandler object containing the settings for reading the archive.
         */
        BIT7Z_NODISCARD const BitAbstractArchiveHandler& handler() const noexcept;

        /**
         * @brief Extracts the specified items to the chosen directory.
         *
         * @param out_dir   the output directory where the extracted files will be put.
         * @param indices   the array of indices of the files in the archive that must be extracted.
         */
        void extract( const tstring& out_dir, const vector< uint32_t >& indices = {} ) const;

        /**
         * @brief Extracts a file to the output buffer.
         *
         * @param out_buffer   the output buffer where the content of the archive will be put.
         * @param index        the index of the file to be extracted.
         */
        void extract( vector< byte_t >& out_buffer, uint32_t index = 0 ) const;

        /**
         * @brief Extracts a file to the pre-allocated output buffer.
         *
         * @tparam N     the size of the output buffer (it must be equal to the unpacked size
         *               of the item to be extracted).
         * @param buffer the pre-allocated output buffer.
         * @param index  the index of the file to be extracted.
         */
        template< std::size_t N >
        void extract( std::array< byte_t, N >& buffer, uint32_t index = 0 ) const {
            extract( buffer.data(), buffer.size(), index );
        }

        /**
         * @brief Extracts a file to the pre-allocated output buffer.
         *
         * @tparam N     the size of the output buffer (it must be equal to the unpacked size
         *               of the item to be extracted).
         * @param buffer the pre-allocated output buffer.
         * @param index  the index of the file to be extracted.
         */
        template< std::size_t N >
        void extract( byte_t (& buffer)[N], uint32_t index = 0 ) const { // NOLINT(modernize-avoid-c-arrays)
            extract( buffer, N, index );
        }

        /**
         * @brief Extracts a file to the pre-allocated output buffer.
         *
         * @param buffer the pre-allocated output buffer.
         * @param size   the size of the output buffer (it must be equal to the unpacked size
         *               of the item to be extracted).
         * @param index  the index of the file to be extracted.
         */
        void extract( byte_t* buffer, std::size_t size, uint32_t index = 0 ) const;

        /**
         * @brief Extracts a file to the output stream.
         *
         * @param out_stream   the (binary) stream where the content of the archive will be put.
         * @param index        the index of the file to be extracted.
         */
        void extract( std::ostream& out_stream, uint32_t index = 0 ) const;

        /**
         * @brief Extracts the content of the archive to a map of memory buffers, where the keys are the paths
         * of the files (inside the archive), and the values are their decompressed contents.
         *
         * @param out_map   the output map.
         */
        void extract( std::map< tstring, vector< byte_t > >& out_map ) const;

        /**
         * @brief Tests the archive without extracting its content.
         *
         * If the archive is not valid, a BitException is thrown!
         */
        void test() const;

    protected:
        IInArchive* openArchiveStream( const tstring& name, IInStream* in_stream );

        HRESULT initUpdatableArchive( IOutArchive** newArc ) const;

        BIT7Z_NODISCARD HRESULT close() const noexcept;

        friend class BitAbstractArchiveOpener;

        friend class BitAbstractArchiveCreator;

        friend class BitOutputArchive;

    private:
        IInArchive* mInArchive;
        const BitInFormat* mDetectedFormat;
        const BitAbstractArchiveHandler& mArchiveHandler;
        const tstring mArchivePath;

    public:
        class const_iterator {
            public:
                // iterator traits
                using iterator_category = std::input_iterator_tag;
                using value_type = BitArchiveItemOffset;
                using reference = const BitArchiveItemOffset&;
                using pointer = const BitArchiveItemOffset*;
                using difference_type = uint32_t; //so that count_if returns an uint32_t

                const_iterator& operator++() noexcept;

                const_iterator operator++( int ) noexcept;

                bool operator==( const const_iterator& other ) const noexcept;

                bool operator!=( const const_iterator& other ) const noexcept;

                reference operator*() noexcept;

                pointer operator->() noexcept;

            private:
                BitArchiveItemOffset mItemOffset;

                const_iterator( uint32_t item_index, const BitInputArchive& item_archive ) noexcept;

                friend class BitInputArchive;
        };

        /**
         * @return an iterator to the first element of the archive. If the archive is empty,
         *         the returned iterator will be equal to end().
         */
        BIT7Z_NODISCARD const_iterator begin() const noexcept;

        /**
         * @return an iterator to the element following the last element of the archive.
         *         This element acts as a placeholder; attempting to access it results in undefined behavior.
         */
        BIT7Z_NODISCARD const_iterator end() const noexcept;

        /**
         * @return an iterator to the first element of the archive. If the archive is empty,
         *         the returned iterator will be equal to end().
         */
        BIT7Z_NODISCARD const_iterator cbegin() const noexcept;

        /**
         * @return an iterator to the element following the last element of the archive.
         *         This element acts as a placeholder; attempting to access it results in undefined behavior.
         */
        BIT7Z_NODISCARD const_iterator cend() const noexcept;

        /**
         * @brief Find an item in the archive that has the given path.
         *
         * @param path the path to be searched in the archive.
         *
         * @return an iterator to the item with the given path, or an iterator equal to end() if no item is found.
         */
        BIT7Z_NODISCARD const_iterator find( const tstring& path ) const noexcept;

        /**
         * @brief Find if there is an item in the archive that has the given path.
         *
         * @param path the path to be searched in the archive.
         *
         * @return true if and only if an item with the given path exists in the archive.
         */
        BIT7Z_NODISCARD bool contains( const tstring& path ) const noexcept;
};
}  // namespace bit7z

#endif //BITINPUTARCHIVE_HPP

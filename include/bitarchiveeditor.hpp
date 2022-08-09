/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITARCHIVEEDITOR_HPP
#define BITARCHIVEEDITOR_HPP

#include <unordered_map>

#include "bitarchivewriter.hpp"

namespace bit7z {
using std::vector;

using EditedItems = std::unordered_map< uint32_t, BitItemsVector::value_type >;

/**
 * @brief The BitArchiveEditor class allows to create new file archives or update old ones.
 *        Update operations supported are the addition of new items,
 *        as well as renaming/updating/deleting old items;
 *
 * @note  Changes are applied to the archive only after calling the applyChanges() method.
 */
class BitArchiveEditor final : public BitArchiveWriter {
    public:
        /**
         * @brief Constructs a BitArchiveEditor object, reading the given archive file path.
         *
         * @param lib      the 7z library to use.
         * @param in_file  the path to an input archive file.
         * @param format   the input/output archive format.
         * @param password (optional) the password needed to read the input archive.
         */
        BitArchiveEditor( const Bit7zLibrary& lib,
                          const tstring& in_file,
                          const BitInOutFormat& format,
                          const tstring& password = {} );

        BitArchiveEditor( const BitArchiveEditor& ) = delete;

        BitArchiveEditor( BitArchiveEditor&& ) = delete;

        BitArchiveEditor& operator=( const BitArchiveEditor& ) = delete;

        BitArchiveEditor& operator=( BitArchiveEditor&& ) = delete;

        ~BitArchiveEditor() override;

        /**
         * @brief Sets how the editor performs the update of the items in the archive.
         *
         * @note BitArchiveEditor doesn't support UpdateMode::None.
         *
         * @param mode the desired update mode (either UpdateMode::Append or UpdateMode::Overwrite).
         */
        void setUpdateMode( UpdateMode mode ) override;

        /**
         * @brief Requests to change the path of the item at the specified index with the given one.
         *
         * @param index    the index of the item to be renamed.
         * @param new_path the new path (in the archive) desired for the item.
         */
        void renameItem( uint32_t index, const tstring& new_path );

        /**
         * @brief Requests to change the path of the item from old_path to new_path.
         *
         * @param old_path the old path (in the archive) of the item to be renamed.
         * @param new_path the new path (in the archive) desired for the item.
         */
        void renameItem( const tstring& old_path, const tstring& new_path );

        /**
         * @brief Requests to update the content of the item at the specified index
         *        with the data from the given file.
         *
         * @param index    the index of the item to be updated.
         * @param in_file  the path to the file containing the new data for the item.
         */
        void updateItem( uint32_t index, const tstring& in_file );

        /**
         * @brief Requests to update the content of the item at the specified index
         *        with the data from the given buffer.
         *
         * @param index     the index of the item to be updated.
         * @param in_buffer the buffer containing the new data for the item.
         */
        void updateItem( uint32_t index, const vector< byte_t >& in_buffer );

        /**
         * @brief Requests to update the content of the item at the specified index
         *        with the data from the given stream.
         *
         * @param index     the index of the item to be updated.
         * @param in_stream the stream of new data for the item.
         */
        void updateItem( uint32_t index, istream& in_stream );

        /**
         * @brief Requests to update the content of the item at the specified path
         *        with the data from the given file.
         *
         * @param item_path the path (in the archive) of the item to be updated.
         * @param in_file   the path to the file containing the new data for the item.
         */
        void updateItem( const tstring& item_path, const tstring& in_file );

        /**
         * @brief Requests to update the content of the item at the specified path
         *        with the data from the given buffer.
         *
         * @param item_path the path (in the archive) of the item to be updated.
         * @param in_buffer the buffer containing the new data for the item.
         */
        void updateItem( const tstring& item_path, const vector< byte_t >& in_buffer );

        /**
         * @brief Requests to update the content of the item at the specified path
         *        with the data from the given stream.
         *
         * @param item_path the path (in the archive) of the item to be updated.
         * @param in_stream the stream of new data for the item.
         */
        void updateItem( const tstring& item_path, istream& in_stream );

        /**
         * @brief Marks the item at the given index as deleted.
         *
         * @param index the index of the item to be deleted.
         */
        void deleteItem( uint32_t index );

        /**
         * @brief Marks the item at the given path (in the archive) as deleted.
         *
         * @param item_path the path (in the archive) of the item to be deleted.
         */
        void deleteItem( const tstring& item_path );

        /**
         * @brief Applies the requested changes (i.e., rename/update/delete operations) to the input archive.
         */
        void applyChanges();

    private:
        EditedItems mEditedItems;

        uint32_t findItem( const tstring& item_path );

        void checkIndex( uint32_t index );

        BitPropVariant itemProperty( input_index index, BitProperty propID ) const override;

        HRESULT itemStream( input_index index, ISequentialInStream** inStream ) const override;

        bool hasNewData( uint32_t index ) const noexcept override;

        bool hasNewProperties( uint32_t index ) const noexcept override;
};
}  // namespace bit7z

#endif //BITARCHIVEEDITOR_HPP

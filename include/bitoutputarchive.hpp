/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITOUTPUTARCHIVE_HPP
#define BITOUTPUTARCHIVE_HPP

#include <istream>
#include <set>

#include "bitabstractarchivecreator.hpp"
#include "bititemsvector.hpp"
#include "bitexception.hpp" //for FailedFiles
#include "bitpropvariant.hpp"

struct ISequentialInStream;

namespace bit7z {
using std::istream;

using DeletedItems = std::set< uint32_t >;

/* General note: I tried my best to explain how indices work here, but it is a bit complex. */

/* We introduce a strong index type to differentiate between indices in the output
 * archive (uint32_t, as used by the UpdateCallback), and the corresponding indexes
 * in the input archive (input_index). In this way, we avoid implicit conversions
 * between the two kinds of indices.
 *
 * UpdateCallback uses indices in the range [0, BitOutputArchive::itemsCount() - 1]
 *
 * Now, if the user doesn't delete any item in the input archive, itemsCount()
 * is just equal to <n. of items in the input archive> + <n. of newly added items>.
 * In this case, an input_index value is just equal to the index used by UpdateCallback.
 *
 * On the contrary, if the user wants to delete an item in the input archive, the value
 * of an input_index may differ from the corresponding UpdateCallback's index.
 *
 * Note: given an input_index i:
 *         if i < mInputArchiveItemsCount, the item is old (old item in the input archive);
 *         if i >= mInputArchiveItemsCount, the item is new (added by the user); */
enum class input_index : std::uint32_t {};

class UpdateCallback;

/**
 * @brief The BitOutputArchive class, given a creator object, allows creating new archives.
 */
class BitOutputArchive {
    public:
        /**
         * @brief Constructs a BitOutputArchive object, opening an (optional) input file archive.
         *
         * If a non-empty input file path is passed, the corresponding archive will be opened and
         * used as a base for the creation of the new archive. Otherwise, the class will behave
         * as if it is creating a completely new archive.
         *
         * @param creator the reference to the BitAbstractArchiveCreator object containing all the settings to
         *                be used for creating the new archive and reading the (optional) input archive.
         * @param in_file (optional) the path to an input archive file.
         */
        explicit BitOutputArchive( const BitAbstractArchiveCreator& creator, tstring in_file = {} );


        /**
         * @brief Constructs a BitOutputArchive object, opening an input file archive from the given buffer.
         *
         * If a non-empty input buffer is passed, the archive file it contains will be opened and
         * used as a base for the creation of the new archive. Otherwise, the class will behave
         * as if it is creating a completely new archive.
         *
         * @param creator   the reference to the BitAbstractArchiveCreator object containing all the settings to
         *                  be used for creating the new archive and reading the (optional) input archive.
         * @param in_buffer the buffer containing an input archive file.
         */
        BitOutputArchive( const BitAbstractArchiveCreator& creator, const vector< byte_t >& in_buffer );


        /**
         * @brief Constructs a BitOutputArchive object, reading an input file archive from the given std::istream.
         *
         * @param creator   the reference to the BitAbstractArchiveCreator object containing all the settings to
         *                  be used for creating the new archive and reading the (optional) input archive.
         * @param in_stream the standard input stream of the input archive file.
         */
        BitOutputArchive( const BitAbstractArchiveCreator& creator, std::istream& in_stream );

        BitOutputArchive( const BitOutputArchive& ) = delete;

        BitOutputArchive( BitOutputArchive&& ) = delete;

        BitOutputArchive& operator=( const BitOutputArchive& ) = delete;

        BitOutputArchive& operator=( BitOutputArchive&& ) = delete;

        /**
         * @brief Adds all the items that can be found by indexing the given vector of filesystem paths.
         *
         * @param in_paths the vector of filesystem paths.
         */
        void addItems( const vector< tstring >& in_paths );

        /**
         * @brief Adds all the items that can be found by indexing the keys of the given map of filesystem paths;
         *       the corresponding mapped values are the user-defined paths wanted inside the output archive.
         *
         * @param in_paths map of filesystem paths with the corresponding user-defined path desired inside the
         *                 output archive.
         */
        void addItems( const map< tstring, tstring >& in_paths );

        /**
         * @brief Adds the given file path, with an optional user-defined path to be used in the output archive.
         *
         * @note If a directory path is given, a BitException is thrown.
         *
         * @param in_file the path to the filesystem file to be added to the output archive.
         * @param name    (optional) user-defined path to be used inside the output archive.
         */
        void addFile( const tstring& in_file, const tstring& name = {} );

        /**
         * @brief Adds the given buffer file, using the given name as path when compressed in the output archive.
         *
         * @param in_buffer the buffer containing the file to be added to the output archive.
         * @param name      user-defined path to be used inside the output archive.
         */
        void addFile( const vector< byte_t >& in_buffer, const tstring& name );

        /**
         * @brief Adds the given standard input stream, using the given name as path when compressed
         *        in the output archive.
         *
         * @param in_stream
         * @param name
         */
        void addFile( istream& in_stream, const tstring& name );

        /**
         * @brief Adds all the files in the given vector of filesystem paths.
         *
         * @note Paths to directories are ignored.
         *
         * @param in_files the vector of paths to files.
         */
        void addFiles( const vector< tstring >& in_files );

        /**
         * @brief Adds all the files inside the given directory path that match the given wildcard filter.
         *
         * @param in_dir    the directory where to search for files to be added to the output archive.
         * @param filter    (optional) the wildcard filter to be used for searching the files.
         * @param recursive (optional) recursively search the files in the given directory
         *                  and all of its subdirectories.
         */
        void addFiles( const tstring& in_dir,
                       const tstring& filter = BIT7Z_STRING( "*.*" ),
                       bool recursive = true );

        /**
         * @brief Adds all the items inside the given directory path.
         *
         * @param in_dir the directory where to search for items to be added to the output archive.
         */
        void addDirectory( const tstring& in_dir );

        /**
         * @brief Compresses all the items added to this object to the specified archive file path.
         *
         * @note If this object was created by passing an input archive file path, and this latter is the same as
         * the out_file path parameter, the file will be updated.
         *
         * @param out_file the output archive file path.
         */
        void compressTo( const tstring& out_file );

        /**
         * @brief Compresses all the items added to this object to the specified buffer.
         *
         * @param out_buffer the output buffer.
         */
        void compressTo( std::vector< byte_t >& out_buffer );

        /**
         * @brief Compresses all the items added to this object to the specified buffer.
         *
         * @param out_stream the output standard stream.
         */
        void compressTo( ostream& out_stream );

        /**
         * @return the total number of item added to the output archive object.
         */
        uint32_t itemsCount() const;

        /**
         * @return a constant reference to the BitAbstractArchiveHandler object containing the
         *         settings for writing the output archive.
         */
        const BitAbstractArchiveHandler& handler() const noexcept;

        virtual ~BitOutputArchive() = default;

    protected:
        unique_ptr< BitInputArchive > mInputArchive;
        uint32_t mInputArchiveItemsCount;

        BitItemsVector mNewItemsVector;
        DeletedItems mDeletedItems;

        mutable FailedFiles mFailedFiles;

        /* mInputIndices:
         *   Position i = index in range [0, itemsCount() - 1] used by UpdateCallback.
         *   Value at pos. i = corresponding index in the input archive (type input_index).
         *
         * If there are some deleted items, then i != mInputIndices[i]
         * (at least for values of i greater than the index of the first deleted item).
         *
         * Otherwise, if there are no deleted items, mInputIndices is empty, and itemInputIndex(i)
         * will return input_index with value i.
         *
         * This vector is either empty, or it has size equal to itemsCount() (thanks to updateInputIndices()). */
        std::vector< input_index > mInputIndices;

        virtual BitPropVariant itemProperty( input_index index, BitProperty prop ) const;

        virtual HRESULT itemStream( input_index index, ISequentialInStream** inStream ) const;

        virtual bool hasNewData( uint32_t index ) const noexcept;

        virtual bool hasNewProperties( uint32_t index ) const noexcept;

        input_index itemInputIndex( uint32_t new_index ) const noexcept;

        BitPropVariant outputItemProperty( uint32_t index, BitProperty propID ) const;

        HRESULT outputItemStream( uint32_t index, ISequentialInStream** inStream ) const;

        uint32_t indexInArchive( uint32_t index ) const noexcept;

        friend class UpdateCallback;

    private:
        const BitAbstractArchiveCreator& mArchiveCreator;

        CMyComPtr< IOutArchive > initOutArchive() const;

        CMyComPtr< IOutStream > initOutFileStream( const tstring& out_archive, bool updating_archive ) const;

        void compressToFile( const tstring& out_file, UpdateCallback* update_callback );

        void compressOut( IOutArchive* out_arc,
                          IOutStream* out_stream,
                          UpdateCallback* update_callback );

        void setArchiveProperties( IOutArchive* out_archive ) const;

        void updateInputIndices();
};
}  // namespace bit7z

#endif //BITOUTPUTARCHIVE_HPP

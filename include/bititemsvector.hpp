/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITITEMSVECTOR_HPP
#define BITITEMSVECTOR_HPP

#include <map>
#include <memory>

#include "bitfs.hpp"
#include "bittypes.hpp"

namespace bit7z {
using std::vector;
using std::map;
using std::unique_ptr;

namespace filesystem {
class FSItem;
}

using filesystem::FSItem;

struct GenericInputItem;
using GenericInputItemPtr = std::unique_ptr< GenericInputItem >;
using GenericInputItemVector = std::vector< GenericInputItemPtr >;

/**
 * @brief The BitItemsVector class represents a vector of generic input items, i.e., items that can come
 * from the filesystem, from memory buffers, or from standard streams.
 */
class BitItemsVector final {
    public:
        using value_type = GenericInputItemPtr;

        /**
         * @brief Indexes the given directory, adding to the vector all the files that match the wildcard filter.
         *
         * @param in_dir    the directory to be indexed.
         * @param filter    (optional) the wildcard filter to be used for indexing;
         *                  empty string means "index all files".
         * @param recursive (optional) recursively index the given directory and all of its subdirectories.
         */
        void indexDirectory( const fs::path& in_dir,
                             const tstring& filter = {},
                             bool recursive = true );

        /**
         * @brief Indexes the given vector of filesystem paths, adding to the item vector all the files.
         *
         * @param in_paths    the vector of filesystem paths.
         * @param ignore_dirs if false, any directory path in the vector is also recursively indexed, and
         *                    the found files are added to the vector; otherwise, directory paths are ignored.
         */
        void indexPaths( const vector< tstring >& in_paths, bool ignore_dirs = false );

        /**
         * @brief Indexes the given map of filesystem paths, adding to the vector all the files.
         *
         * @note Map keys represent the filesystem paths to be indexed; the corresponding mapped values are
         * the user-defined (possibly different) paths wanted inside archives.
         *
         * @param in_paths    map of filesystem paths with the corresponding user-defined path desired inside the
         *                    output archive.
         * @param ignore_dirs if false, any directory path in the vector is also recursively indexed, and
         *                    the found files are added to the vector; otherwise, directory paths are ignored.
         */
        void indexPathsMap( const map< tstring, tstring >& in_paths, bool ignore_dirs = false );

        /**
         * @brief Indexes the given file path, with an optional user-defined path to be used in output archives.
         *
         * @note If a directory path is given, a BitException is thrown.
         *
         * @param in_file the path to the filesystem file to be indexed in the vector.
         * @param name    (optional) user-defined path to be used inside archives.
         */
        void indexFile( const tstring& in_file, const tstring& name = {} );

        /**
         * @brief Indexes the given buffer, using the given name as path when compressed in archives.
         *
         * @param in_buffer the buffer containing the file to be indexed in the vector.
         * @param name      user-defined path to be used inside archives.
         */
        void indexBuffer( const vector< byte_t >& in_buffer, const tstring& name );

        /**
         * @brief Indexes the given standard input stream, using the given name as path when compressed in archives.
         *
         * @param in_stream the standard input stream of the file to be indexed in the vector.
         * @param name      user-defined path to be used inside archives.
         */
        void indexStream( std::istream& in_stream, const tstring& name );

        /**
         * @return the size of the items vector.
         */
        BIT7Z_NODISCARD size_t size() const;

        /**
         * @param index the index of the desired item in the vector.
         * @return a constant reference to the GenericInputItem at the given index.
         */
        const GenericInputItem& operator[]( GenericInputItemVector::size_type index ) const;

        /**
         * @return an iterator to the first element of the vector. If the vector is empty,
         *         the returned iterator will be equal to end().
         */
        BIT7Z_NODISCARD GenericInputItemVector::const_iterator begin() const noexcept;

        /**
         * @return an iterator to the element following the last element of the vector.
         *         This element acts as a placeholder; attempting to access it results in undefined behavior.
         */
        BIT7Z_NODISCARD GenericInputItemVector::const_iterator end() const noexcept;

        /**
         * @return an iterator to the first element of the vector. If the vector is empty,
         *         the returned iterator will be equal to end().
         */
        BIT7Z_NODISCARD GenericInputItemVector::const_iterator cbegin() const noexcept;

        /**
         * @return an iterator to the element following the last element of the vector.
         *         This element acts as a placeholder; attempting to access it results in undefined behavior.
         */
        BIT7Z_NODISCARD GenericInputItemVector::const_iterator cend() const noexcept;

        ~BitItemsVector();

    private:
        GenericInputItemVector mItems;

        void indexItem( const FSItem& item, bool ignore_dirs );
};
}  // namespace bit7z


#endif //BITITEMSVECTOR_HPP

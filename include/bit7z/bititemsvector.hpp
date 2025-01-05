/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITITEMSVECTOR_HPP
#define BITITEMSVECTOR_HPP

#include "bitabstractarchivehandler.hpp"
#include "bitfs.hpp"
#include "bittypes.hpp"

#include <cstddef>
#include <istream>
#include <map>
#include <memory>
#include <vector>

namespace bit7z {

struct GenericInputItem;
using GenericInputItemPtr = std::unique_ptr< GenericInputItem >;

/** @cond **/
struct IndexingOptions {
    bool recursive = true;
    bool retainFolderStructure = false;
    bool onlyFiles = false;
    bool followSymlinks = true;
};
/** @endcond **/

using BitItemsVector = std::vector< GenericInputItemPtr >;

/**
 * @brief Indexes the given directory, adding to the vector all the files that match the wildcard filter.
 *
 * @param inDir     the directory to be indexed.
 * @param filter    (optional) the wildcard filter to be used for indexing;
 *                  empty string means "index all files".
 * @param policy    (optional) the filtering policy to be applied to the matched items.
 * @param options   (optional) the settings to be used while indexing the given directory
 *                  and all of its subdirectories.
 */
void indexDirectory( BitItemsVector& outVector,
                     const fs::path& inDir,
                     const tstring& filter = {},
                     FilterPolicy policy = FilterPolicy::Include,
                     IndexingOptions options = {} );
/**
 * @brief Indexes the given vector of filesystem paths, adding to the item vector all the files.
 *
 * @param inPaths   the vector of filesystem paths.
 * @param options   (optional) the settings to be used while indexing the given directory
 *                  and all of its subdirectories.
 */
void indexPaths( BitItemsVector& outVector, const std::vector< tstring >& inPaths, IndexingOptions options = {} );

/**
 * @brief Indexes the given map of filesystem paths, adding to the vector all the files.
 *
 * @note Map keys represent the filesystem paths to be indexed; the corresponding mapped values are
 * the user-defined (possibly different) paths wanted inside archives.
 *
 * @param inPaths   map of filesystem paths with the corresponding user-defined path desired inside the
 *                  output archive.
 * @param options   (optional) the settings to be used while indexing the given directory
 *                  and all of its subdirectories.
 */
void indexPathsMap( BitItemsVector& outVector,
                    const std::map< tstring, tstring >& inPaths,
                    IndexingOptions options = {} );

/**
 * @brief Indexes the given file path, with an optional user-defined path to be used in output archives.
 *
 * @note If a directory path is given, a BitException is thrown.
 *
 * @param inFile          the path to the filesystem file to be indexed in the vector.
 * @param name            (optional) user-defined path to be used inside archives.
 * @param followSymlinks  (optional) whether to follow symbolic links or not.
 */
void indexFile( BitItemsVector& outVector,
                const tstring& inFile,
                const tstring& name = {},
                bool followSymlinks = true );

/**
 * @brief Indexes the given buffer, using the given name as a path when compressed in archives.
 *
 * @param inBuffer  the buffer containing the file to be indexed in the vector.
 * @param name      user-defined path to be used inside archives.
 */
void indexBuffer( BitItemsVector& outVector, const buffer_t& inBuffer, const tstring& name );

/**
 * @brief Indexes the given standard input stream, using the given name as a path when compressed in archives.
 *
 * @param inStream  the standard input stream of the file to be indexed in the vector.
 * @param name      user-defined path to be used inside archives.
 */
void indexStream( BitItemsVector& outVector, std::istream& inStream, const tstring& name );

}  // namespace bit7z

#endif //BITITEMSVECTOR_HPP

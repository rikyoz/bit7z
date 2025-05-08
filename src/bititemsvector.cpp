// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bititemsvector.hpp"

#include "bitexception.hpp"
#include "bittypes.hpp"
#include "internal/bufferitem.hpp"
#include "internal/fsindexer.hpp"
#include "internal/fsitem.hpp"
#include "internal/fsutil.hpp"
#include "internal/stdinputitem.hpp"
#include "internal/stringutil.hpp"

#include <cstddef>
#include <istream>
#include <map>
#include <memory>
#include <system_error>
#include <vector>

namespace bit7z {

using filesystem::FilesystemItem;

void indexDirectory( BitItemsVector& outVector,
                     const fs::path& inDir,
                     const tstring& filter,
                     IndexingOptions options ) {
    // Note: if inDir is an invalid path, FilesystemItem constructor throws a BitException.
    const FilesystemItem dirItem{ inDir, options.retainFolderStructure ? inDir : fs::path{}, options.symlinkPolicy };
    if ( filter.empty() && !dirItem.inArchivePath().empty() ) {
        outVector.emplace_back( std::make_unique< FilesystemItem >( dirItem ) );
    }
    filesystem::listDirectoryItems( inDir, dirItem.inArchivePath(), filter, options, outVector );
}

namespace {
void indexItem( BitItemsVector& outVector, const FilesystemItem& item, IndexingOptions options ) {
    if ( !item.isDir() ) {
        outVector.emplace_back( std::make_unique< FilesystemItem >( item ) );
    } else if ( options.recursive ) { // The item is a directory
        if ( !item.inArchivePath().empty() ) {
            outVector.emplace_back( std::make_unique< FilesystemItem >( item ) );
        }
        filesystem::listDirectoryItems( item.filesystemPath(), item.inArchivePath(), {}, options, outVector );
    } else {
        // No action needed
    }
}
} // namespace

void indexPaths( BitItemsVector& outVector, const std::vector< tstring >& inPaths, IndexingOptions options ) {
    for ( const auto& inputPath : inPaths ) {
        const auto filePath = tstring_to_path( inputPath );
        const FilesystemItem item{ filePath,
                                   options.retainFolderStructure ? filePath : fs::path{},
                                   options.symlinkPolicy };
        indexItem( outVector, item, options );
    }
}

void indexPathsMap( BitItemsVector& outVector, const std::map< tstring, tstring >& inPaths, IndexingOptions options ) {
    for ( const auto& filePair : inPaths ) {
        const FilesystemItem item{ tstring_to_path( filePair.first ),
                                   tstring_to_path( filePair.second ),
                                   options.symlinkPolicy };
        indexItem( outVector, item, options );
    }
}

void indexFile( BitItemsVector& outVector, const tstring& inFile, const tstring& name, SymlinkPolicy symlinkPolicy ) {
    const fs::path filePath = tstring_to_path( inFile );
    if ( fs::is_directory( filePath ) ) {
        throw BitException( "Input path points to a directory, not a file",
                            std::make_error_code( std::errc::invalid_argument ), inFile );
    }
    outVector.emplace_back( std::make_unique< FilesystemItem >( filePath, tstring_to_path( name ), symlinkPolicy ) );
}

void indexBuffer( BitItemsVector& outVector, const buffer_t& inBuffer, const tstring& name ) {
    outVector.emplace_back( std::make_unique< BufferItem >( inBuffer, tstring_to_path( name ) ) );
}

void indexStream( BitItemsVector& outVector, std::istream& inStream, const tstring& name ) {
    outVector.emplace_back( std::make_unique< StdInputItem >( inStream, tstring_to_path( name ) ) );
}

} // namespace bit7z
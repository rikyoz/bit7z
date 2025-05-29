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

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bittypes.hpp"
#include "internal/fsindexer.hpp"
#include "internal/fsutil.hpp"

#include <map>
#include <system_error>
#include <vector>

namespace bit7z {

void indexDirectory( BitItemsVector& outVector,
                     const tstring& inDir,
                     const tstring& filter,
                     IndexingOptions options ) {
    const auto inDirPath = tstring_to_path( inDir );
    // Note: if inDir is an invalid path, FilesystemItem constructor throws a BitException.
    const BitInputItem item{ inDirPath, options.retainFolderStructure ? inDirPath : fs::path{}, options.symlinkPolicy };
    if ( filter.empty() && !item.inArchivePath().empty() ) {
        outVector.emplace_back( item );
    }
    filesystem::listDirectoryItems( inDirPath, item.inArchivePath(), filter, options, outVector );
}

void indexDirectoryContent( BitItemsVector& outVector,
                            const tstring& inDir,
                            const tstring& filter,
                            IndexingOptions options ) {
    if ( inDir.empty() ) {
        throw BitException( "Could not index directory", make_error_code( BitError::InvalidDirectoryPath ), inDir );
    }
    std::error_code error;
    const auto inDirPath = fs::absolute( tstring_to_path( inDir ), error );
    if ( error ) {
        throw BitException( "Could not index directory", error, inDir );
    }
    if ( !fs::exists( inDirPath, error ) ) {
        throw BitException( "Could not index directory",
                            make_error_code( std::errc::no_such_file_or_directory ),
                            inDir );
    }
    filesystem::listDirectoryItems( inDirPath, fs::path{}, filter, options, outVector );
}

namespace {
void indexItem( BitItemsVector& outVector,
                const fs::path& filePath,
                const fs::path& inArchivePath,
                IndexingOptions options ) {
    BitInputItem item{ filePath, inArchivePath, options.symlinkPolicy };
    if ( !item.isDir() ) {
        outVector.emplace_back( std::move( item ) );
    } else if ( options.recursive ) { // The item is a directory
        if ( !item.inArchivePath().empty() ) {
            outVector.emplace_back( item );
        }
        // Note: we are using item.inArchivePath() rather than the parameter as the first is calculated more accurately
        // by the BitInputItem class constructor.
        filesystem::listDirectoryItems( filePath, item.inArchivePath(), {}, options, outVector );
    } else {
        // No action needed
    }
}
} // namespace

void indexPaths( BitItemsVector& outVector, const std::vector< tstring >& inPaths, IndexingOptions options ) {
    for ( const auto& inputPath : inPaths ) {
        const auto filePath = tstring_to_path( inputPath );
        indexItem( outVector, filePath, options.retainFolderStructure ? filePath : fs::path{}, options );
    }
}

void indexPathsMap( BitItemsVector& outVector, const std::map< tstring, tstring >& inPaths, IndexingOptions options ) {
    for ( const auto& filePair : inPaths ) {
        indexItem( outVector, tstring_to_path( filePair.first ), tstring_to_path( filePair.second ), options );
    }
}

void indexFile( BitItemsVector& outVector, const tstring& inFile, const tstring& name, SymlinkPolicy symlinkPolicy ) {
    const fs::path filePath = tstring_to_path( inFile );
    if ( fs::is_directory( filePath ) ) {
        throw BitException( "Input path points to a directory, not a file",
                            std::make_error_code( std::errc::invalid_argument ), inFile );
    }
    outVector.emplace_back( filePath, tstring_to_path( name ), symlinkPolicy );
}

void indexBuffer( BitItemsVector& outVector, const buffer_t& inBuffer, const tstring& name ) {
    outVector.emplace_back( inBuffer, name );
}

void indexStream( BitItemsVector& outVector, std::istream& inStream, const tstring& name ) {
    outVector.emplace_back( inStream, name );
}

} // namespace bit7z
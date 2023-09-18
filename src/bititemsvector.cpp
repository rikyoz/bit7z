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

#include "bitexception.hpp"
#include "bititemsvector.hpp"
#include "internal/bufferitem.hpp"
#include "internal/fsindexer.hpp"
#include "internal/stdinputitem.hpp"

using namespace bit7z;
using filesystem::FSItem;
using filesystem::FSIndexer;
using filesystem::SymlinkPolicy;

void BitItemsVector::indexDirectory( const fs::path& in_dir,
                                     const tstring& filter,
                                     FilterPolicy policy,
                                     IndexingOptions options ) {
    const auto symlinkPolicy = options.follow_symlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    // Note: if in_dir is an invalid path, FSItem constructor throws a BitException!
    const FSItem dir_item{ in_dir,options.retain_folder_structure ? in_dir : fs::path{}, symlinkPolicy };
    if ( filter.empty() && !dir_item.inArchivePath().empty() ) {
        mItems.emplace_back( std::make_unique< FSItem >( dir_item ) );
    }
    FSIndexer indexer{ dir_item, filter, policy, symlinkPolicy, options.only_files };
    indexer.listDirectoryItems( mItems, options.recursive );
}

void BitItemsVector::indexPaths( const std::vector< tstring >& in_paths, IndexingOptions options ) {
    const auto symlinkPolicy = options.follow_symlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    for ( const auto& file_path : in_paths ) {
        const FSItem item{ file_path, options.retain_folder_structure ? file_path : BIT7Z_STRING( "" ), symlinkPolicy };
        indexItem( item, options );
    }
}

void BitItemsVector::indexPathsMap( const std::map< tstring, tstring >& in_paths, IndexingOptions options ) {
    const auto symlinkPolicy = options.follow_symlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    for ( const auto& file_pair : in_paths ) {
        const FSItem item{ fs::path( file_pair.first ), fs::path( file_pair.second ), symlinkPolicy };
        indexItem( item, options );
    }
}

void BitItemsVector::indexItem( const FSItem& item, IndexingOptions options ) {
    if ( !item.isDir() ) {
        mItems.emplace_back( std::make_unique< FSItem >( item ) );
    } else if ( options.recursive ) { // The item is a directory
        if ( !item.inArchivePath().empty() ) {
            mItems.emplace_back( std::make_unique< FSItem >( item ) );
        }
        const auto symlink_policy = options.follow_symlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
        FSIndexer indexer{ item, {}, FilterPolicy::Include, symlink_policy, options.only_files };
        indexer.listDirectoryItems( mItems, true );
    } else {
        // No action needed
    }
}

void BitItemsVector::indexFile( const tstring& in_file, const tstring& name, bool follow_symlinks ) {
    if ( fs::is_directory( in_file ) ) {
        throw BitException( "Input path points to a directory, not a file",
                            std::make_error_code( std::errc::invalid_argument ), in_file );
    }
    const auto symlinkPolicy = follow_symlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    mItems.emplace_back( std::make_unique< FSItem >( in_file, name, symlinkPolicy ) );
}

void BitItemsVector::indexBuffer( const vector< byte_t >& in_buffer, const tstring& name ) {
    mItems.emplace_back( std::make_unique< BufferItem >( in_buffer, name ) );
}

void BitItemsVector::indexStream( std::istream& in_stream, const tstring& name ) {
    mItems.emplace_back( std::make_unique< StdInputItem >( in_stream, name ) );
}

auto BitItemsVector::size() const -> size_t {
    return mItems.size();
}

auto BitItemsVector::operator[]( GenericInputItemVector::size_type index ) const -> const GenericInputItem& {
    // Note: here index is expected to be correct!
    return *mItems[ index ];
}

auto BitItemsVector::begin() const noexcept -> GenericInputItemVector::const_iterator {
    return mItems.cbegin();
}

auto BitItemsVector::end() const noexcept -> GenericInputItemVector::const_iterator {
    return mItems.cend();
}

auto BitItemsVector::cbegin() const noexcept -> GenericInputItemVector::const_iterator {
    return mItems.cbegin();
}

auto BitItemsVector::cend() const noexcept -> GenericInputItemVector::const_iterator {
    return mItems.cend();
}

/* Note: separate declaration/definition of the default destructor is needed to use an incomplete type
 *       for the unique_ptr objects stored in the vector. */
BitItemsVector::~BitItemsVector() = default;
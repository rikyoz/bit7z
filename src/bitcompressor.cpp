// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/bitcompressor.hpp"

#include "../include/bitexception.hpp"
#include "../include/fsindexer.hpp"
#include "../include/fileupdatecallback.hpp"

using namespace std;
using namespace bit7z;

BitCompressor::BitCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

/* from filesystem to filesystem */

void BitCompressor::compress( const vector< tstring >& in_paths, const tstring& out_file ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, std::make_error_code( std::errc::function_not_supported ) );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_paths );
    compressOut( fs_items, out_file );
}

void BitCompressor::compress( const map< tstring, tstring >& in_paths, const tstring& out_file ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, std::make_error_code( std::errc::function_not_supported ) );
    }
    vector< FSItem > fs_items = FSIndexer::indexPathsMap( in_paths );
    compressOut( fs_items, out_file );
}

void BitCompressor::compressFile( const tstring& in_file, const tstring& out_file ) const {
    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Input path points to a directory, not a file",
                            std::make_error_code( std::errc::invalid_argument ) );
    }
    vector< FSItem > fs_items;
    fs_items.push_back( item );
    compressOut( fs_items, out_file );
}

void BitCompressor::compressFiles( const vector< tstring >& in_files, const tstring& out_file ) const {
    if ( in_files.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, std::make_error_code( std::errc::function_not_supported ) );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_files, true );
    compressOut( fs_items, out_file );
}

void BitCompressor::compressFiles( const tstring& in_dir, const tstring& out_file,
                                   bool recursive, const tstring& filter ) const {
    if ( !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, std::make_error_code( std::errc::function_not_supported ) );
    }
    vector< FSItem > fs_items = FSIndexer::indexDirectory( in_dir, filter, recursive );
    compressOut( fs_items, out_file );
}

void BitCompressor::compressDirectory( const tstring& in_dir, const tstring& out_file ) const {
    compressFiles( in_dir, out_file, true, TSTRING( "" ) );
}

/* from filesystem to memory buffer */

void BitCompressor::compressFile( const tstring& in_file, vector< byte_t >& out_buffer ) const {
    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Cannot compress a directory into a memory buffer",
                            std::make_error_code( std::errc::invalid_argument ) );
    }

    vector< FSItem > fs_items;
    fs_items.push_back( item );

    CMyComPtr< UpdateCallback > update_callback = new FileUpdateCallback( *this, fs_items );
    BitArchiveCreator::compressToBuffer( out_buffer, update_callback );
}

/* from filesystem to stream */

void BitCompressor::compress( const vector< tstring >& in_paths, ostream& out_stream ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, std::make_error_code( std::errc::function_not_supported ) );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_paths );
    compressOut( fs_items, out_stream );
}

void BitCompressor::compress( const map< tstring, tstring >& in_paths, ostream& out_stream ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, std::make_error_code( std::errc::function_not_supported ) );
    }
    vector< FSItem > fs_items = FSIndexer::indexPathsMap( in_paths );
    compressOut( fs_items, out_stream );
}

void BitCompressor::compressOut( const vector< FSItem >& in_items, const tstring& out_file ) const {
    CMyComPtr< UpdateCallback > update_callback = new FileUpdateCallback( *this, in_items );
    BitArchiveCreator::compressToFile( out_file, update_callback );
}

void BitCompressor::compressOut( const vector< FSItem >& in_items, ostream& out_stream ) const {
    CMyComPtr< UpdateCallback > update_callback = new FileUpdateCallback( *this, in_items );
    BitArchiveCreator::compressToStream( out_stream, update_callback );
}


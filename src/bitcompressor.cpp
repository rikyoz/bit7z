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

void BitCompressor::compress( const vector< wstring >& in_paths, const wstring& out_file ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, ERROR_NOT_SUPPORTED );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_paths );
    compressOut( fs_items, out_file );
}

void BitCompressor::compress( const map< wstring, wstring >& in_paths, const wstring& out_file ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, ERROR_NOT_SUPPORTED );
    }
    vector< FSItem > fs_items = FSIndexer::indexPathsMap( in_paths );
    compressOut( fs_items, out_file );
}

void BitCompressor::compressFile( const wstring& in_file, const wstring& out_file ) const {
    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Wrong argument: input path points to a directory, not a file!", E_INVALIDARG );
    }
    vector< FSItem > fs_items;
    fs_items.push_back( item );
    compressOut( fs_items, out_file );
}

void BitCompressor::compressFiles( const vector< wstring >& in_files, const wstring& out_file ) const {
    if ( in_files.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, ERROR_NOT_SUPPORTED );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_files, true );
    compressOut( fs_items, out_file );
}

void BitCompressor::compressFiles( const wstring& in_dir, const wstring& out_file,
                                   bool recursive, const wstring& filter ) const {
    if ( !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, ERROR_NOT_SUPPORTED );
    }
    vector< FSItem > fs_items = FSIndexer::indexDirectory( in_dir, filter, recursive );
    compressOut( fs_items, out_file );
}

void BitCompressor::compressDirectory( const wstring& in_dir, const wstring& out_file ) const {
    compressFiles( in_dir, out_file, true, L"" );
}

/* from filesystem to memory buffer */

void BitCompressor::compressFile( const wstring& in_file, vector< byte_t >& out_buffer ) const {
    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Cannot compress a directory into a memory buffer!", E_INVALIDARG );
    }

    vector< FSItem > fs_items;
    fs_items.push_back( item );

    CMyComPtr< UpdateCallback > update_callback = new FileUpdateCallback( *this, fs_items );
    BitArchiveCreator::compressToBuffer( out_buffer, update_callback );
}

/* from filesystem to stream */

void BitCompressor::compress( const vector< wstring >& in_paths, ostream& out_stream ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, ERROR_NOT_SUPPORTED );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_paths );
    compressOut( fs_items, out_stream );
}

void BitCompressor::compress( const map< wstring, wstring >& in_paths, ostream& out_stream ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( kUnsupportedOperation, ERROR_NOT_SUPPORTED );
    }
    vector< FSItem > fs_items = FSIndexer::indexPathsMap( in_paths );
    compressOut( fs_items, out_stream );
}

void BitCompressor::compressOut( const vector< FSItem >& in_items, const wstring& out_file ) const {
    CMyComPtr< UpdateCallback > update_callback = new FileUpdateCallback( *this, in_items );
    BitArchiveCreator::compressToFile( out_file, update_callback );
}

void BitCompressor::compressOut( const vector< FSItem >& in_items, ostream& out_stream ) const {
    CMyComPtr< UpdateCallback > update_callback = new FileUpdateCallback( *this, in_items );
    BitArchiveCreator::compressToStream( out_stream, update_callback );
}


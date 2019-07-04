// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
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
#include "../include/updatecallback.hpp"

using namespace std;
using namespace bit7z;

BitCompressor::BitCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

/* from filesystem to filesystem */

void BitCompressor::compress( const vector< wstring >& in_paths, const wstring& out_archive ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_paths );
    compressOut( fs_items, out_archive );
}

void BitCompressor::compress( const map< wstring, wstring >& in_paths, const wstring& out_archive ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > fs_items = FSIndexer::indexPathsMap( in_paths );
    compressOut( fs_items, out_archive );
}

void BitCompressor::compressFile( const wstring& in_file, const wstring& out_archive ) const {
    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Wrong argument: input path points to a directory, not a file!" );
    }
    vector< FSItem > fs_items;
    fs_items.push_back( item );
    compressOut( fs_items, out_archive );
}

void BitCompressor::compressFiles( const vector< wstring >& in_files, const wstring& out_archive ) const {
    if ( in_files.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_files, true );
    compressOut( fs_items, out_archive );
}

void BitCompressor::compressFiles( const wstring& in_dir, const wstring& out_archive,
                                   bool recursive, const wstring& filter ) const {
    if ( !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > fs_items = FSIndexer::indexDirectory( in_dir, filter, recursive );
    compressOut( fs_items, out_archive );
}

void BitCompressor::compressDirectory( const wstring& in_dir, const wstring& out_archive ) const {
    compressFiles( in_dir, out_archive, true, L"" );
}

/* from filesystem to memory buffer */

void BitCompressor::compressFile( const wstring& in_file, vector< byte_t >& out_buffer ) const {
    if ( !mFormat.hasFeature( INMEM_COMPRESSION ) ) {
        throw BitException( "Unsupported format for in-memory compression!" );
    }

    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Cannot compress a directory into a memory buffer!" );
    }

    vector< FSItem > fs_items;
    fs_items.push_back( item );

    compressOut( fs_items, out_buffer );
}

/* from filesystem to stream */

void BitCompressor::compressFile( const wstring& in_file, ostream& out_stream ) const {
    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Cannot compress a directory into a stream!" );
    }

    vector< FSItem > fs_items;
    fs_items.push_back( item );

    compressOut( fs_items, out_stream );
}

void BitCompressor::compressOut( const vector< FSItem >& in_items, const wstring& out_archive ) const {
    unique_ptr< BitInputArchive > old_arc = nullptr;
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_file_stream = initOutFileStream( out_archive, new_arc, old_arc );
    CMyComPtr< CompressCallback > update_callback = new UpdateCallback( *this, in_items, old_arc.get() );
    BitArchiveCreator::compressOut( new_arc, out_file_stream, update_callback );
    cleanupOldArc( old_arc.get(), out_file_stream, out_archive );
}

void BitCompressor::compressOut( const vector< FSItem >& in_items, vector< byte_t >& out_buffer ) const {
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< ISequentialOutStream > out_mem_stream = initOutMemStream( out_buffer );
    CMyComPtr< CompressCallback > update_callback = new UpdateCallback( *this, in_items );
    BitArchiveCreator::compressOut( new_arc, out_mem_stream, update_callback );
}

void BitCompressor::compressOut( const vector< FSItem >& in_items, ostream& out_stream ) const {
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_std_stream = initOutStdStream( out_stream );
    CMyComPtr< CompressCallback > update_callback = new UpdateCallback( *this, in_items );
    BitArchiveCreator::compressOut( new_arc, out_std_stream, update_callback );
}


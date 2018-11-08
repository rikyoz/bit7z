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

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"

#include "../include/fsitem.hpp"
#include "../include/util.hpp"
#include "../include/bitexception.hpp"
#include "../include/coutmemstream.hpp"
#include "../include/coutmultivolstream.hpp"
#include "../include/memupdatecallback.hpp"
#include "../include/updatecallback.hpp"

#include <sstream>

using namespace std;
using namespace bit7z;
using namespace bit7z::util;
using namespace NWindows;

template< class T >
void compressOut( const CMyComPtr< IOutArchive >& out_arc, CMyComPtr< T > out_stream,
                  const vector< FSItem >& in_items, const BitArchiveCreator& creator ) {
    auto* update_callback_spec = new UpdateCallback( creator, in_items );

    CMyComPtr< IArchiveUpdateCallback2 > update_callback( update_callback_spec );
    HRESULT result = out_arc->UpdateItems( out_stream, static_cast< uint32_t >( in_items.size() ), update_callback );
    update_callback_spec->Finilize();

    if ( result == E_NOTIMPL ) {
        throw BitException( "Unsupported operation!" );
    }

    if ( result == E_FAIL && update_callback_spec->getErrorMessage().empty() ) {
        throw BitException( "Failed operation (unkwown error)!" );
    }

    if ( result != S_OK ) {
        throw BitException( update_callback_spec->getErrorMessage() );
    }

    if ( !update_callback_spec->mFailedFiles.empty() ) {
        wstringstream wsstream;
        wsstream << L"Error for files: " << endl;
        for ( const auto& failed_file : update_callback_spec->mFailedFiles ) {
            wsstream << failed_file.first << L" (error code: " << failed_file.second << L")" << endl;
        }
        throw BitException( wsstream.str() );
    }
}

BitCompressor::BitCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

/* from filesystem to filesystem */

void BitCompressor::compress( const vector< wstring >& in_paths, const wstring& out_archive ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_paths );
    compressToFileSystem( fs_items, out_archive );
}

void BitCompressor::compress( const map<wstring, wstring>& in_paths, const wstring& out_archive ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > fs_items = FSIndexer::indexPathsMap( in_paths );
    compressToFileSystem( fs_items, out_archive );
}

void BitCompressor::compressFile( const wstring& in_file, const wstring& out_archive ) const {
    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Wrong argument: input path points to a directory, not a file!" );
    }
    vector< FSItem > fs_items;
    fs_items.push_back( item );
    compressToFileSystem( fs_items, out_archive );
}

void BitCompressor::compressFiles( const vector< wstring >& in_files, const wstring& out_archive ) const {
    if ( in_files.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > fs_items = FSIndexer::indexPaths( in_files, true );
    compressToFileSystem( fs_items, out_archive );
}

void BitCompressor::compressFiles( const wstring& in_dir, const wstring& out_archive,
                                   bool recursive, const wstring& filter ) const {
    if ( !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > fs_items = FSIndexer::indexDirectory( in_dir, filter, recursive );
    compressToFileSystem( fs_items, out_archive );
}

void BitCompressor::compressDirectory( const wstring& in_dir, const wstring& out_archive ) const {
    compressFiles( in_dir, out_archive, true, L"" );
}

/* from filesystem to memory buffer */

void BitCompressor::compressFile( const wstring& in_file, vector< byte_t >& out_buffer ) const {
    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Cannot compress a directory into a memory buffer!" );
    }
    vector< FSItem > fs_items;
    fs_items.push_back( item );
    compressToMemory( fs_items, out_buffer );
}

/* Most of this code, though heavily modified, is taken from the main() of Client7z.cpp in the 7z SDK
 * Main changes made:
 *  + Generalized the code to work with any type of format (original works only with 7z format)
 *  + Use of exceptions instead of error codes */
void BitCompressor::compressToFileSystem( const vector< FSItem >& in_items, const wstring& out_archive ) const {
    CMyComPtr< IOutArchive > out_arc = initOutArchive( mLibrary, mFormat, mCompressionLevel, mCryptHeaders, mSolidMode );

    CMyComPtr< IOutStream > out_file_stream;
    if ( mVolumeSize > 0 ) {
        auto* out_multivol_stream_spec = new COutMultiVolStream( mVolumeSize, out_archive );
        out_file_stream = out_multivol_stream_spec;
    } else {
        auto* out_file_stream_spec = new COutFileStream();
        /* note: if you remove the following line (and you pass the outFileStreamSpec to UpdateItems method), you will not
         * have any problem... until you try to compress files with GZip format! In that case your program will crash!! */
        out_file_stream = out_file_stream_spec;
        if ( !out_file_stream_spec->Create( out_archive.c_str(), false ) ) {
            throw BitException( L"Can't create archive file '" + out_archive + L"'" );
        }
    }

    compressOut( out_arc, out_file_stream, in_items, *this );
}

// FS -> Memory
void BitCompressor::compressToMemory( const vector< FSItem >& in_items, vector< byte_t >& out_buffer ) const {
    if ( in_items.empty() ) {
        throw BitException( "The list of files/directories cannot be empty!" );
    }
    if ( !mFormat.hasFeature( INMEM_COMPRESSION ) ) {
        throw BitException( "Unsupported format for in-memory compression!" );
    }

    CMyComPtr< IOutArchive > out_arc = initOutArchive( mLibrary, mFormat, mCompressionLevel, mCryptHeaders, mSolidMode );

    auto* out_mem_stream_spec = new COutMemStream( out_buffer );
    CMyComPtr< ISequentialOutStream > out_mem_stream( out_mem_stream_spec );

    compressOut( out_arc, out_mem_stream, in_items, *this );
}

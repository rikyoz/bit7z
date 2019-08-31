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

#include "../include/bitmemextractor.hpp"

#include "7zip/Archive/IArchive.h"

#include "../include/bitexception.hpp"
#include "../include/fileextractcallback.hpp"
#include "../include/memextractcallback.hpp"
#include "../include/streamextractcallback.hpp"

using namespace bit7z;

BitMemExtractor::BitMemExtractor( const Bit7zLibrary& lib, const BitInFormat& format )
    : BitArchiveOpener( lib, format ) {
#ifdef BIT7Z_AUTOFORMAT
    if ( format == BitFormat::Auto ) {
        throw BitException( "Automatic format detection not supported for in-memory archives", E_INVALIDARG );
    }
#endif
}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, const wstring& out_dir ) const {
    BitInputArchive in_archive( *this, in_buffer );

    auto* extract_callback_spec = new FileExtractCallback( *this, in_archive, L"", out_dir );
    in_archive.extract( vector< uint32_t >(), extract_callback_spec );
}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer,
                               vector< byte_t >& out_buffer,
                               unsigned int index ) const {
    BitInputArchive in_archive( *this, in_buffer );

    uint32_t number_items = in_archive.itemsCount();
    if ( index >= number_items ) {
        throw BitException( L"Index " + std::to_wstring( index ) + L" is out of range", E_INVALIDARG );
    }

    if ( in_archive.isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( kCannotExtractFolderToBuffer, E_INVALIDARG );
    }

    const vector< uint32_t > indices( 1, index );
    map< wstring, vector< byte_t > > buffers_map;
    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, buffers_map );
    in_archive.extract( indices, extract_callback_spec );
    out_buffer = std::move( buffers_map.begin()->second );
}

void BitMemExtractor::extract( const vector<byte_t>& in_buffer, std::ostream& out_stream, unsigned int index ) const {
    BitInputArchive in_archive( *this, in_buffer );

    uint32_t number_items = in_archive.itemsCount();
    if ( index >= number_items ) {
        throw BitException( L"Index " + std::to_wstring( index ) + L" is out of range", E_INVALIDARG );
    }

    if ( in_archive.isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( kCannotExtractFolderToBuffer, E_INVALIDARG );
    }

    const vector< uint32_t > indices( 1, index );
    auto* extract_callback_spec = new StreamExtractCallback( *this, in_archive, out_stream );
    in_archive.extract( indices, extract_callback_spec );
}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, map< wstring, vector< byte_t > >& out_map ) const {
    BitInputArchive in_archive( *this, in_buffer );

    uint32_t number_items = in_archive.itemsCount();
    vector< uint32_t > files_indices;
    for ( uint32_t i = 0; i < number_items; ++i ) {
        if ( !in_archive.isItemFolder( i ) ) { //Consider only files, not folders
            files_indices.push_back( i );
        }
    }

    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, out_map );
    in_archive.extract( files_indices, extract_callback_spec );
}

void BitMemExtractor::test( const vector<byte_t>& in_buffer ) const {
    BitInputArchive in_archive( *this, in_buffer );

    map< wstring, vector< byte_t > > dummy_map; //output map (not used since we are testing!)
    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, dummy_map );
    in_archive.test( extract_callback_spec );
}

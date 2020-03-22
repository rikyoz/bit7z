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

#include "../include/bitarchiveopener.hpp"

#include "../include/bitexception.hpp"
#include "../include/bitinputarchive.hpp"
#include "../include/fileextractcallback.hpp"
#include "../include/bufferextractcallback.hpp"
#include "../include/streamextractcallback.hpp"

#include <map>

using std::map;
using namespace bit7z;

CONSTEXPR auto kCannotExtractFolderToBuffer = "Cannot extract a folder to a buffer";

BitArchiveOpener::BitArchiveOpener( const Bit7zLibrary& lib, const BitInFormat& format, const wstring& password )
    : BitArchiveHandler( lib, password ), mFormat( format ) {}

BitArchiveOpener::~BitArchiveOpener() {}

const BitInFormat& BitArchiveOpener::format() const {
    return mFormat;
}

const BitInFormat& BitArchiveOpener::extractionFormat() const {
    return mFormat;
}

void BitArchiveOpener::extractToFileSystem( const BitInputArchive& in_archive,
                                            const wstring& in_file,
                                            const wstring& out_dir,
                                            const vector< uint32_t >& indices ) const {
    CMyComPtr< ExtractCallback > extract_callback = new FileExtractCallback( *this, in_archive, in_file, out_dir );
    in_archive.extract( indices, extract_callback );
}

void BitArchiveOpener::extractToStream( const BitInputArchive& in_archive,
                                        std::ostream& out_stream,
                                        unsigned int index ) const {
    uint32_t number_items = in_archive.itemsCount();
    if ( index >= number_items ) {
        throw BitException( L"Index " + std::to_wstring( index ) + L" is out of range", E_INVALIDARG );
    }

    if ( in_archive.isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( kCannotExtractFolderToBuffer, E_INVALIDARG );
    }

    const vector< uint32_t > indices( 1, index );
    CMyComPtr< ExtractCallback > extract_callback = new StreamExtractCallback( *this, in_archive, out_stream );
    in_archive.extract( indices, extract_callback );
}

void BitArchiveOpener::extractToBuffer( const BitInputArchive& in_archive,
                                        vector< byte_t >& out_buffer,
                                        unsigned int index ) const {
    uint32_t number_items = in_archive.itemsCount();
    if ( index >= number_items ) {
        throw BitException( L"Index " + std::to_wstring( index ) + L" is out of range", E_INVALIDARG );
    }

    if ( in_archive.isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( kCannotExtractFolderToBuffer, E_INVALIDARG );
    }

    const vector< uint32_t > indices( 1, index );
    map< wstring, vector< byte_t > > buffers_map;
    CMyComPtr< ExtractCallback > extract_callback = new BufferExtractCallback( *this, in_archive, buffers_map );
    in_archive.extract( indices, extract_callback );
    out_buffer = std::move( buffers_map.begin()->second );
}

void BitArchiveOpener::extractToBufferMap( const BitInputArchive& in_archive,
                                           map< wstring, vector< byte_t > >& out_map ) const {
    uint32_t number_items = in_archive.itemsCount();
    vector< uint32_t > files_indices;
    for ( uint32_t i = 0; i < number_items; ++i ) {
        if ( !in_archive.isItemFolder( i ) ) { //Consider only files, not folders
            files_indices.push_back( i );
        }
    }

    CMyComPtr< ExtractCallback > extract_callback = new BufferExtractCallback( *this, in_archive, out_map );
    in_archive.extract( files_indices, extract_callback );

}

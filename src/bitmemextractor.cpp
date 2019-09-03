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

#include "../include/bitmemextractor.hpp"

#include "../include/bitinputarchive.hpp"
#include "../include/bitexception.hpp"
#include "../include/bufferextractcallback.hpp"

using namespace bit7z;

BitMemExtractor::BitMemExtractor( const Bit7zLibrary& lib, const BitInFormat& format )
    : BitArchiveOpener( lib, format ) {}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, const wstring& out_dir ) const {
    BitInputArchive in_archive( *this, in_buffer );
    extractToFileSystem( in_archive, L"", out_dir, vector< uint32_t >() );
}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer,
                               vector< byte_t >& out_buffer,
                               unsigned int index ) const {
    BitInputArchive in_archive( *this, in_buffer );
    extractToBuffer( in_archive, out_buffer, index );
}

void BitMemExtractor::extract( const vector<byte_t>& in_buffer, std::ostream& out_stream, unsigned int index ) const {
    BitInputArchive in_archive( *this, in_buffer );
    extractToStream( in_archive, out_stream, index );
}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, map< wstring, vector< byte_t > >& out_map ) const {
    BitInputArchive in_archive( *this, in_buffer );
    extractToBufferMap( in_archive, out_map );
}

void BitMemExtractor::test( const vector< byte_t >& in_buffer ) const {
    BitInputArchive in_archive( *this, in_buffer );

    map< wstring, vector< byte_t > > dummy_map; //output map (not used since we are testing!)
    CMyComPtr< ExtractCallback > extract_callback = new BufferExtractCallback( *this, in_archive, dummy_map );
    in_archive.test( extract_callback );
}

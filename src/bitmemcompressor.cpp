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

#include "../include/bitmemcompressor.hpp"

#include "../include/bitexception.hpp"
#include "../include/bufferupdatecallback.hpp"
#include "../include/fsutil.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;
using std::wstring;
using std::vector;

BitMemCompressor::BitMemCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer,
                                 const wstring& out_file,
                                 const wstring& in_buffer_name ) const {
    const wstring& name = in_buffer_name.empty() ? fsutil::filename( out_file ) : in_buffer_name;

    CMyComPtr< UpdateCallback > update_callback = new BufferUpdateCallback( *this, in_buffer, name );
    BitArchiveCreator::compressToFile( out_file, update_callback );
}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer,
                                 vector< byte_t >& out_buffer,
                                 const wstring& in_buffer_name ) const {
    CMyComPtr< UpdateCallback > update_callback = new BufferUpdateCallback( *this, in_buffer, in_buffer_name );
    BitArchiveCreator::compressToBuffer( out_buffer, update_callback );
}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer,
                                 std::ostream& out_stream,
                                 const std::wstring& in_buffer_name ) const {
    CMyComPtr< UpdateCallback > update_callback = new BufferUpdateCallback( *this, in_buffer, in_buffer_name );
    BitArchiveCreator::compressToStream( out_stream, update_callback );
}

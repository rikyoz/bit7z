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

#include "../include/bitstreamcompressor.hpp"

#include "../include/streamupdatecallback.hpp"
#include "../include/fsutil.hpp"

#include "../include/bitexception.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

BitStreamCompressor::BitStreamCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

void BitStreamCompressor::compress( istream& in_stream, ostream& out_stream, const wstring& in_stream_name ) const {
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_std_stream = initOutStdStream( out_stream );
    CMyComPtr< CompressCallback > update_callback = new StreamUpdateCallback( *this, in_stream, in_stream_name );
    BitArchiveCreator::compressOut( new_arc, out_std_stream, update_callback );
}

void BitStreamCompressor::compress( istream& in_stream, vector<byte_t>& out_buffer, const wstring& in_stream_name ) const {
    if ( !mFormat.hasFeature( INMEM_COMPRESSION ) ) {
        throw BitException( kUnsupportedInMemoryFormat, ERROR_NOT_SUPPORTED );
    }

    if ( !out_buffer.empty() ) {
        throw BitException( kCannotOverwriteBuffer, E_INVALIDARG );
    }

    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< ISequentialOutStream > out_mem_stream = initOutMemStream( out_buffer );
    CMyComPtr< CompressCallback > update_callback = new StreamUpdateCallback( *this, in_stream, in_stream_name );
    BitArchiveCreator::compressOut( new_arc, out_mem_stream, update_callback );
}

void BitStreamCompressor::compress( istream& in_stream, const wstring& out_file, const wstring& in_stream_name ) const {
    const wstring& name = in_stream_name.empty() ? fsutil::filename( out_file ) : in_stream_name;

    unique_ptr< BitInputArchive > old_arc = nullptr;
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_file_stream = initOutFileStream( out_file, new_arc, old_arc );
    CMyComPtr< CompressCallback > update_callback = new StreamUpdateCallback( *this, in_stream, name, old_arc.get() );
    BitArchiveCreator::compressOut( new_arc, out_file_stream, update_callback );
    cleanupOldArc( old_arc.get(), out_file_stream, out_file );
}

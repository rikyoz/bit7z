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

#include "../include/bitmemcompressor.hpp"

#include "../include/bitexception.hpp"
#include "../include/memupdatecallback.hpp"
#include "../include/fsutil.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;
using std::wstring;
using std::vector;

BitMemCompressor::BitMemCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer,
                                 const wstring& out_archive,
                                 const wstring& in_buffer_name ) const {

    const wstring& name = in_buffer_name.empty() ? fsutil::filename( out_archive ) : in_buffer_name;

    unique_ptr< BitInputArchive > old_arc = nullptr;
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_file_stream = initOutFileStream( out_archive, new_arc, old_arc );
    CMyComPtr< CompressCallback > update_callback = new MemUpdateCallback( *this, in_buffer, name, old_arc.get() );
    compressOut( new_arc, out_file_stream, update_callback );
    cleanupOldArc( old_arc.get(), out_file_stream, out_archive );
}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer,
                                 vector< byte_t >& out_buffer,
                                 const wstring& in_buffer_name ) const {
    if ( !mFormat.hasFeature( INMEM_COMPRESSION ) ) {
        throw BitException( kUnsupportedInMemoryFormat, ERROR_NOT_SUPPORTED );
    }

    if ( !out_buffer.empty() ) {
        throw BitException( kCannotOverwriteBuffer, E_INVALIDARG );
    }

    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< ISequentialOutStream > out_mem_stream = initOutMemStream( out_buffer );
    CMyComPtr< CompressCallback > update_callback = new MemUpdateCallback( *this, in_buffer, in_buffer_name );
    compressOut( new_arc, out_mem_stream, update_callback );
}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer,
                                 std::ostream& out_stream,
                                 const std::wstring& in_buffer_name ) const {
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_std_stream = initOutStdStream( out_stream );
    CMyComPtr< CompressCallback > update_callback = new MemUpdateCallback( *this, in_buffer, in_buffer_name );
    compressOut( new_arc, out_std_stream, update_callback );
}

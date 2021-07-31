// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#include "bitstreamcompressor.hpp"

#include "bitoutputarchive.hpp"
#include "internal/fsutil.hpp"
#include "internal/genericstreamitem.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

BitStreamCompressor::BitStreamCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

void BitStreamCompressor::compress( istream& in_stream, ostream& out_stream, const tstring& in_stream_name ) const { //-V669
    BitOutputArchive output_archive{ *this };
    output_archive.addFile( in_stream, in_stream_name );
    output_archive.compressTo( out_stream );
}


void BitStreamCompressor::compress( istream& in_stream, //-V669
                                    vector< byte_t >& out_buffer,
                                    const tstring& in_stream_name ) const {
    BitOutputArchive output_archive{ *this, out_buffer };
    output_archive.addFile( in_stream, in_stream_name );
    output_archive.compressTo( out_buffer );
}

void BitStreamCompressor::compress( istream& in_stream, //-V669
                                    const tstring& out_file,
                                    const tstring& in_stream_name ) const {
    const tstring& name = in_stream_name.empty() ? fsutil::filename( out_file ) : in_stream_name;

    BitOutputArchive output_archive{ *this, out_file };
    output_archive.addFile( in_stream, name );
    output_archive.compressTo( out_file );
}

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

#include "bitarchivewriter.hpp"

using bit7z::BitArchiveWriter;

BitArchiveWriter::BitArchiveWriter( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitAbstractArchiveCreator( lib, format ), BitOutputArchive( *this, TSTRING( "") ) {}

BitArchiveWriter::BitArchiveWriter( const Bit7zLibrary& lib,
                                    const tstring& in_file,
                                    const BitInOutFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveCreator( lib, format, password, UpdateMode::Append ),
      BitOutputArchive( *this, in_file ) {}

BitArchiveWriter::BitArchiveWriter( const Bit7zLibrary& lib,
                                    const std::vector< byte_t >& in_buffer,
                                    const BitInOutFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveCreator( lib, format, password, UpdateMode::Append ),
      BitOutputArchive( *this, in_buffer ) {}

BitArchiveWriter::BitArchiveWriter( const Bit7zLibrary& lib,
                                    std::istream& in_stream,
                                    const BitInOutFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveCreator( lib, format, password, UpdateMode::Append ),
      BitOutputArchive( *this, in_stream ) {}
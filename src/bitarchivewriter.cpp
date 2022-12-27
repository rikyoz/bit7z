// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bitarchivewriter.hpp"

namespace bit7z {

BitArchiveWriter::BitArchiveWriter( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitAbstractArchiveCreator( lib, format ), BitOutputArchive( *this, tstring{} ) {}

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

} // namespace bit7z
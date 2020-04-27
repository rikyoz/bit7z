// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2020  Riccardo Ostani - All Rights Reserved.
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

#include "../include/cfileoutstream.hpp"

#include "../include/bitexception.hpp"

using namespace bit7z;

CFileOutStream::CFileOutStream( const fs::path& filePath, bool createAlways ) : CStdOutStream( mFileStream ) {
    open( filePath, createAlways );
}

void CFileOutStream::open( const fs::path& filePath, bool createAlways ) {
    std::error_code ec;
    if ( !createAlways && fs::exists( filePath, ec ) ) {
        throw BitException( "File already exists", filePath.native(), ERROR_FILE_EXISTS );
    }
    mFileStream.open( filePath, std::ios::binary | std::ios::trunc );
}

bool CFileOutStream::fail() {
    return mFileStream.fail();
}

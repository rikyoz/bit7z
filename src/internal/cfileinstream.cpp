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

#include "internal/cfileinstream.hpp"

#include "bitexception.hpp"

using namespace bit7z;

CFileInStream::CFileInStream( const fs::path& filePath ) : CStdInStream( mFileStream ), mBuffer{} {
    open( filePath );

    /* By default, file stream performance is relatively poor due to the default buffer size used
     * (e.g., GCC uses a small 1024 bytes buffer).
     * This is a known problem (see https://stackoverflow.com/questions/26095160/why-are-stdfstreams-so-slow).
     * We make the underlying file stream use a bigger buffer (1 MiB) for optimizing the reading of big files.  */
    mFileStream.rdbuf()->pubsetbuf( mBuffer.data(), buffer_size );
}

void CFileInStream::open( const fs::path& filePath ) {
    mFileStream.open( filePath, std::ios::in | std::ios::binary );
    if ( mFileStream.fail() ) {
        //Note: CFileInStream constructor does not directly throw exceptions since it is also used in nothrow functions.
        throw BitException( "Failed to open the archive file",
                            make_hresult_code( HRESULT_FROM_WIN32( ERROR_OPEN_FAILED ) ),
                            filePath.native() );
    }
}

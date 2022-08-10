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

#include "internal/cfileoutstream.hpp"

#include "bitexception.hpp"

using namespace bit7z;

CFileOutStream::CFileOutStream( const fs::path& filePath, bool createAlways ) : CStdOutStream( mFileStream ), mBuffer{} {
    std::error_code ec;
    if ( !createAlways && fs::exists( filePath, ec ) ) {
        if ( !ec ) { // the call to fs::exists succeeded, but filePath doesn't exist, and this is an error!
            ec = std::make_error_code( std::errc::file_exists );
        }
        throw BitException( "Failed to create the output file", ec, filePath.native() );
    }
    mFileStream.open( filePath, std::ios::binary | std::ios::trunc );
    if ( mFileStream.fail() ) {
        throw BitException( "Failed to create the output file",
                            make_hresult_code( HRESULT_FROM_WIN32( ERROR_OPEN_FAILED ) ),
                            filePath.native() );
    }

    mFileStream.rdbuf()->pubsetbuf( mBuffer.data(), buffer_size );
}

bool CFileOutStream::fail() {
    return mFileStream.fail();
}

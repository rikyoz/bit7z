// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/cfileoutstream.hpp"

#include "bitexception.hpp"
#include "internal/cstdoutstream.hpp"
#include "internal/stringutil.hpp"

#include <ios>
#include <system_error>
#include <utility>

namespace bit7z {

CFileOutStream::CFileOutStream( fs::path filePath, bool createAlways )
    : CStdOutStream( mFileStream ), mFilePath{ std::move( filePath ) } {
    std::error_code error;
    if ( !createAlways && fs::exists( mFilePath, error ) ) {
        if ( !error ) {
            // The call to fs::exists succeeded, but the filePath exists, and this is an error.
            error = std::make_error_code( std::errc::file_exists );
        }
        throw BitException( "Failed to create the output file", error, path_to_tstring( mFilePath ) );
    }

    /* Disabling std::ofstream's buffering, as unbuffered IO gives better performance
     * with the data block sizes written by 7-Zip.
     * Note: we need to do this before and after opening the file (https://stackoverflow.com/a/59161297/3497024). */
    mFileStream.rdbuf()->pubsetbuf( nullptr, 0 );
    mFileStream.open( mFilePath, std::ios::binary | std::ios::trunc ); // flawfinder: ignore
    if ( mFileStream.fail() ) {
#if defined( __MINGW32__ ) || defined( __MINGW64__ )
        error = std::error_code{ errno, std::generic_category() };
        throw BitException( "Failed to open the output file", error, path_to_tstring( mFilePath ) );
#else
        throw BitException( "Failed to open the output file", last_error_code(), path_to_tstring( mFilePath ) );
#endif
    }
    mFileStream.rdbuf()->pubsetbuf( nullptr, 0 );
}

auto CFileOutStream::fail() const -> bool {
    return mFileStream.fail();
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFileOutStream::SetSize( UInt64 newSize ) noexcept {
    std::error_code error;
    fs::resize_file( mFilePath, newSize, error );
    return error ? E_FAIL : S_OK;
}

auto CFileOutStream::path() const -> const fs::path& {
    return mFilePath;
}

} // namespace bit7z
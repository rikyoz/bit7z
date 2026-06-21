// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/cfileoutstream.hpp"

namespace bit7z {

CFileOutStream::CFileOutStream( const fs::path& filePath, FileFlag fileFlag )
    : mFile( filePath.native(), fileFlag ), mFilePath{ filePath } {}

#ifdef _WIN32
void CFileOutStream::setFileTime( FILETIME creation, FILETIME access, FILETIME modified ) const noexcept {
    ( void )mFile.setFileTime( creation, access, modified );
}
#endif

auto CFileOutStream::path() const & noexcept -> const fs::path& {
    return mFilePath;
}

auto CFileOutStream::path() && noexcept -> fs::path {
    return std::move( mFilePath );
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFileOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 ) {
        return S_OK;
    }

    std::uint32_t totalBytesWritten = 0;
    const auto result = mFile.write( data, size, totalBytesWritten );
    if ( processedSize != nullptr ) {
        *processedSize = totalBytesWritten;
    }
    return result;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFileOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    SeekOrigin origin; // NOLINT(cppcoreguidelines-init-variables)
    RINOK( toSeekOrigin( seekOrigin, origin ) ) //-V3504

    std::uint64_t finalPosition = 0;
    const auto result = mFile.seek( origin, offset, finalPosition );
    if ( newPosition != nullptr ) {
        *newPosition = finalPosition;
    }
    return result;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFileOutStream::SetSize( UInt64 newSize ) noexcept {
    return mFile.resize( static_cast< std::uint64_t >( newSize ) ) ? S_OK : E_FAIL;
}

} // namespace bit7z

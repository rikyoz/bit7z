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

#include "internal/cfileinstream.hpp"

namespace bit7z {

CFileInStream::CFileInStream( const native_string& filePath, bool storeOpenFiles )
    : mFile{ filePath, storeOpenFiles } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFileInStream::Read( void* data, UInt32 size, UInt32* processedSize ) noexcept { // NOSONAR
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 ) {
        return S_OK;
    }

    std::uint32_t totalBytesRead = 0;
    const auto result = mFile.read( data, size, totalBytesRead );
    if ( processedSize != nullptr ) {
        *processedSize = totalBytesRead;
    }
    return result;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFileInStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    SeekOrigin origin; // NOLINT(cppcoreguidelines-init-variables)
    RINOK( toSeekOrigin( seekOrigin, origin ) ) //-V3504

    std::uint64_t finalPosition = 0;
    const auto result = mFile.seek( origin, offset, finalPosition );
    if ( newPosition != nullptr ) {
        *newPosition = finalPosition;
    }
    return result;
}

} // namespace bit7z

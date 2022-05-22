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

#include "internal/cvolumeoutstream.hpp"

using bit7z::tstring;
using bit7z::CVolumeOutStream;

CVolumeOutStream::CVolumeOutStream( const tstring& volumeName )
    : CFileOutStream( volumeName ), mPath{ volumeName }, mCurrentOffset{ 0 }, mCurrentSize{ 0 } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CVolumeOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) {
    RINOK( CStdOutStream::Seek( offset, seekOrigin, newPosition ) )
    mCurrentOffset = offset;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CVolumeOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    UInt32 writtenSize{};
    RINOK( CStdOutStream::Write( data, size, &writtenSize ) )

    if ( writtenSize == 0 && size != 0 ) {
        return E_FAIL;
    }

    mCurrentOffset += writtenSize;
    if ( processedSize != nullptr ) {
        *processedSize += writtenSize;
    }
    return S_OK;
}

uint64_t CVolumeOutStream::currentOffset() const {
    return mCurrentOffset;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CVolumeOutStream::SetSize( UInt64 newSize ) {
    RINOK( CStdOutStream::SetSize( newSize ) )
    mCurrentSize = newSize;
    return S_OK;
}

uint64_t CVolumeOutStream::currentSize() const {
    return mCurrentSize;
}

void CVolumeOutStream::setCurrentSize( uint64_t currentSize ) {
    mCurrentSize = currentSize;
}

fs::path CVolumeOutStream::path() const {
    return mPath;
}

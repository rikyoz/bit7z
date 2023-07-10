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

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "internal/cmultivolumeinstream.hpp"
#include "internal/util.hpp"
#include "internal/fsutil.hpp"

using bit7z::CMultiVolumeInStream;
using bit7z::CVolumeInStream;

CMultiVolumeInStream::CMultiVolumeInStream( const fs::path& first_volume ) : mCurrentPosition{ 0 }, mTotalSize{ 0 } {
    constexpr size_t volume_digits = 3u;
    size_t volume_index = 1u;
    fs::path volume_path = first_volume;
    while ( fs::exists( volume_path ) ) {
        addVolume( volume_path );

        ++volume_index;
        tstring volume_ext = to_tstring( volume_index );
        if ( volume_ext.length() < volume_digits ) {
            volume_ext.insert( volume_ext.begin(), volume_digits - volume_ext.length(), BIT7Z_STRING( '0' ) );
        }
        volume_path.replace_extension( volume_ext );

        // TODO: Avoid keeping all the volumes streams open
        constexpr auto opened_files_threshold = 500;
        if ( volume_index == opened_files_threshold ) {
            // Note: we use == to avoid increasing the limit more than once;
            // the volume_index is always increasing, so it is not an issue here.
            filesystem::fsutil::increase_opened_files_limit();
        }
    }
}

auto CMultiVolumeInStream::currentVolume() -> const CMyComPtr< CVolumeInStream >& {
    size_t left = 0;
    size_t right = mVolumes.size();
    size_t midpoint = right / 2;
    while ( true ) {
        auto& volume = mVolumes[ midpoint ];
        if ( mCurrentPosition < volume->globalOffset() ) {
            right = midpoint;
        } else if ( mCurrentPosition >= volume->globalOffset() + volume->size() ) {
            left = midpoint + 1;
        } else {
            return volume;
        }
        midpoint = ( left + right ) / 2;
    }
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeInStream::Read( void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 || mCurrentPosition >= mTotalSize ) {
        return S_OK;
    }

    const auto& current_volume = currentVolume();
    UInt64 local_offset = mCurrentPosition - current_volume->globalOffset();
    HRESULT result = current_volume->Seek( static_cast< Int64 >( local_offset ), STREAM_SEEK_SET, &local_offset );
    if ( result != S_OK ) {
        return result;
    }

    const uint64_t remaining = current_volume->size() - local_offset;
    if ( size > remaining ) {
        size = static_cast< UInt32 >( remaining );
    }
    result = current_volume->Read( data, size, &size );
    mCurrentPosition += size;

    if ( processedSize != nullptr ) {
        *processedSize = size;
    }
    return result;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeInStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) {
    uint64_t origin_position; // NOLINT(cppcoreguidelines-init-variables)
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            origin_position = 0;
            break;
        case STREAM_SEEK_CUR:
            origin_position = mCurrentPosition;
            break;
        case STREAM_SEEK_END:
            origin_position = mTotalSize;
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }

    // Checking if adding the (negative) offset would result in the unsigned wrap around of the current position.
    if ( offset < 0 && origin_position < static_cast< uint64_t >( -offset ) ) {
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }

    // Checking if adding the (positive) offset would result in the unsigned wrap around of the current position.
    if ( offset > 0 ) {
        const auto positive_offset = static_cast< uint64_t >( offset );
        const uint64_t seek_position = origin_position + positive_offset;
        if ( seek_position < origin_position || seek_position < positive_offset ) {
            return E_INVALIDARG;
        }
    }
    mCurrentPosition = origin_position + offset;

    if ( newPosition != nullptr ) {
        *newPosition = mCurrentPosition;
    }
    return S_OK;
}

void CMultiVolumeInStream::addVolume( const fs::path& volume_path ) {
    uint64_t global_offset = 0;
    if ( !mVolumes.empty() ) {
        const auto& last_stream = mVolumes.back();
        global_offset = last_stream->globalOffset() + last_stream->size();
    }
    mVolumes.emplace_back( make_com< CVolumeInStream >( volume_path, global_offset ) );
    mTotalSize += mVolumes.back()->size();
}

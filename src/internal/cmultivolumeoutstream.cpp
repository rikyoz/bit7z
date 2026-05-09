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

#include "internal/cmultivolumeoutstream.hpp"

#include "bitexception.hpp"
#include "bittypes.hpp"
#include "internal/fsutil.hpp"
#include "internal/util.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <system_error>

namespace bit7z {

CMultiVolumeOutStream::CMultiVolumeOutStream( std::uint64_t volSize, fs::path archiveName )
    : mMaxVolumeSize( volSize ),
      mVolumePrefix( std::move( archiveName ) ),
      mAbsolutePosition( 0 ),
      mTotalSize( 0 ) {}

auto CMultiVolumeOutStream::currentVolume() -> CachedVolume<CFileOutStream>& {
    const auto volumeIndex = static_cast< std::size_t >( mAbsolutePosition / mMaxVolumeSize );

    for ( auto newVolumeIndex = mVolumes.size(); newVolumeIndex <= volumeIndex; ++newVolumeIndex ) {
        /* The current volume stream still doesn't exist, so we need to create it. */
        tstring name = to_tstring( static_cast< std::uint64_t >( newVolumeIndex ) + 1 );
        if ( name.length() < 3 ) {
            name.insert( 0, 3 - name.length(), BIT7Z_STRING( '0' ) );
        }

        fs::path volumePath = mVolumePrefix;
        volumePath += BIT7Z_STRING( "." ) + name;
        CachedVolume< CFileOutStream > volume{
            volumePath,
            0u,
            newVolumeIndex * mMaxVolumeSize,
            0u,
            {}
        };
        mVolumes.emplace_back( std::move( volume ) );
    }

    auto& cachedVolume = mVolumes[ volumeIndex ];

#ifndef _WIN32
    if ( volumeIndex == mVolumes.newest() ) {
        return cachedVolume; // Already the newest open volume, no need to ensure it is opened.
    }
#endif

    ensureVolumeOpen( cachedVolume, volumeIndex );
    return cachedVolume;
}

void CMultiVolumeOutStream::ensureVolumeOpen( CachedVolume< CFileOutStream >& cachedVolume, std::size_t volumeIndex ) {
#ifdef _WIN32
    (void)volumeIndex;
    if ( cachedVolume.stream == nullptr ) {
        cachedVolume.stream = make_com< CFileOutStream >( cachedVolume.volumePath.native() );
    }
#else
    if ( cachedVolume.stream == nullptr ) {
        cachedVolume.stream = make_com< CFileOutStream >( cachedVolume.volumePath.native(), FileFlag::OpenAlways );
        mVolumes.trackReopen( cachedVolume, volumeIndex );
    } else {
        mVolumes.promote( cachedVolume, volumeIndex );
    }
#endif
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) noexcept try {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    /* Getting the current volume stream. */
    auto& volume = currentVolume();

    const auto volumeOffset = mAbsolutePosition % mMaxVolumeSize;
    if ( volumeOffset != volume.seekPosition ) {
        /* The offset we must write to is different from the last offset we wrote to. */
        UInt64 newPosition{};
        RINOK( volume.stream->Seek( static_cast< Int64 >( volumeOffset ), STREAM_SEEK_SET, &newPosition ) ) //-V3504
        volume.seekPosition = newPosition;
    }

    /* Determining how much we can write to the volume stream */
    const auto writeSize = std::min( size, static_cast< UInt32 >( mMaxVolumeSize - volume.seekPosition ) );

    /* Writing to the volume stream */
    UInt32 bytesWritten{};
    RINOK( volume.stream->Write( data, writeSize, &bytesWritten ) ) //-V3504

    /* Updating the positions */
    volume.seekPosition += bytesWritten;
    mAbsolutePosition += bytesWritten;

    /* We might have written beyond the old known full size of the output archive, updating it. */
    mTotalSize = std::max( mAbsolutePosition, mTotalSize );
    volume.volumeSize = std::max( volume.seekPosition, volume.volumeSize );

    if ( processedSize != nullptr ) {
        *processedSize += bytesWritten;
    }
    return S_OK;
} catch ( const BitException& ex ) {
    return ex.hresultCode();
} catch ( ... ) {
    return E_FAIL;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    std::uint64_t seekPosition{};
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            break;
        case STREAM_SEEK_CUR:
            seekPosition = mAbsolutePosition;
            break;
        case STREAM_SEEK_END:
            seekPosition = mTotalSize;
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }

    RINOK( seek_to_offset( seekPosition, offset ) ) //-V3504
    mAbsolutePosition = seekPosition;
    if ( newPosition != nullptr ) {
        *newPosition = mAbsolutePosition;
    }
    return S_OK;
}

namespace {
// Utility function for computing the ceil division of two unsigned integers.
// Computes ceil( a / b ) for unsigned integers using ( ( a - 1 ) / b ) + 1 instead of the naive
// ( a + b - 1 ) / b to avoid overflow, with an explicit check for a == 0 to prevent underflow.
template< typename T, typename = typename std::enable_if< std::is_unsigned< T >::value >::type >
constexpr auto ceil_div( T dividend, T divisor ) -> T {
    return dividend == 0 ? 0 : ( ( dividend - 1 ) / divisor ) + 1;
}
} // namespace

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeOutStream::SetSize( UInt64 newSize ) noexcept {
    // Max number of volumes we can track/index in the vector of volumes.
    // Note: we exclude one for the kNoVolume index flag value.
    const auto maxVolumes = mVolumes.max_size() - 1;

    // The new size would produce a number of volumes above the maximum allowed.
    // Note: mMaxVolumeSize is non-zero by construction.
    if ( ceil_div( static_cast< std::uint64_t >( newSize ), mMaxVolumeSize ) > maxVolumes ) {
        return E_INVALIDARG;
    }

    while ( !mVolumes.empty() ) {
        auto& lastVolume = mVolumes.back();

        // This volume starts before newSize, so it may need truncation (handled below) but not deletion.
        if ( lastVolume.globalOffset < newSize ) {
            break;
        }

        // The volume starts at or beyond the new size, so it must be released and deleted.
        if ( lastVolume.stream != nullptr ) {
#ifndef _WIN32
            mVolumes.unlink( lastVolume );
            mVolumes.trackClosed();
#endif
            lastVolume.stream.Release();
        }

        std::error_code error;
        fs::remove( lastVolume.volumePath, error );
        if ( error ) {
            return E_FAIL;
        }
        mVolumes.pop_back();
    }

    if ( !mVolumes.empty() ) {
        auto& lastVolume = mVolumes.back();

        const auto newVolumeSize = newSize - lastVolume.globalOffset;
        if ( newVolumeSize < lastVolume.volumeSize ) {
            // Truncating the last volume as it extends beyond newSize.

            if ( lastVolume.stream == nullptr ) {
                try {
                    // Volume was evicted, we need to reopen it.
                    ensureVolumeOpen( lastVolume, mVolumes.size() - 1 );
                } catch ( const BitException& ex ) {
                    return ex.hresultCode();
                } catch ( ... ) {
                    return E_FAIL;
                }
            }

            // Truncating the volume size of the last volume.

            if ( lastVolume.stream != nullptr ) { // ensureVolumeOpen may fail to reopen the stream.
                RINOK( lastVolume.stream->SetSize( static_cast< UInt64 >( newVolumeSize ) ) );
                lastVolume.volumeSize = newVolumeSize;
            }
        }
    }

    mTotalSize = newSize;
    return S_OK;
}

} // namespace bit7z
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
      mCurrentVolumeIndex( 0 ),
      mCurrentVolumeOffset( 0 ),
      mAbsolutePosition( 0 ),
      mTotalSize( 0 ) {}

auto CMultiVolumeOutStream::currentVolume() -> CachedVolume<CFileOutStream>& {
    return mVolumes[ mCurrentVolumeIndex ];
}

void CMultiVolumeOutStream::ensureVolumeOpen( CachedVolume< CFileOutStream >& cachedVolume, std::size_t volumeIndex ) {
#ifdef _WIN32
    (void)volumeIndex;
    if ( cachedVolume.stream == nullptr ) {
        cachedVolume.stream = make_com< CFileOutStream >( cachedVolume.volumePath.native() );
    }
#else
    if ( cachedVolume.stream == nullptr ) {
        // The volume was evicted from the LRU list, so we need to reopen it.
        static const auto openedFilesThreshold = openHandlesThreshold();

        // Opening the volume before evicting the oldest one so that
        // we can handle an open failure without evicting the oldest one.
        cachedVolume.stream = make_com< CFileOutStream >( cachedVolume.volumePath.native(), FileFlag::OpenAlways );
        ++mOpenCount;

        if ( mOpenCount >= openedFilesThreshold ) {
            // Too many open volumes, evicting the newest one (i.e., the head of the open volumes list).
            // Many archive formats store their internal metadata at the beginning of the archive (7-Zip does the same).
            auto& newest = mVolumes[ mNewestVolume ];

            // The "new" newest volume is the volume before the "old" newest volume.
            mNewestVolume = newest.olderVolume;
            if ( mNewestVolume != kNoVolume ) {
                mVolumes[ mNewestVolume ].newerVolume = kNoVolume;
            } else {
                // The "old" newest volume didn't have an older volume, i.e., the list had only one volume.
                mOldestVolume = kNoVolume;
            }

            // Evicting the "old" newest volume.
            newest.newerVolume = kNoVolume;
            newest.olderVolume = kNoVolume;
            newest.stream.Release();
            --mOpenCount;
        }

        if ( cachedVolume.seekPosition != 0 ) {
            // Restoring the seek position so Write() can skip re-seeking in the common case.
            UInt64 newPosition{}; // UInt64 (not std::uint64_t) to match Seek's output parameter type on all platforms.
            const HRESULT seekResult = cachedVolume.stream->Seek(
                static_cast< Int64 >( cachedVolume.seekPosition ),
                STREAM_SEEK_SET,
                &newPosition
            );
            if ( seekResult != S_OK ) {
                cachedVolume.seekPosition = 0; // Failed to seek, stream is still at the beginning.
            } else {
                cachedVolume.seekPosition = newPosition;
            }
        }
    } else if ( volumeIndex == mNewestVolume ) {
        return; // Already the newest open volume, nothing to do.
    } else {
        // Before promoting this volume to the head, we unlink it from its current position.
        if ( cachedVolume.olderVolume != kNoVolume ) {
            // Before: newer <- cached <- older, after: newer <- older
            mVolumes[ cachedVolume.olderVolume ].newerVolume = cachedVolume.newerVolume;
        } else {
            // Before: newer <- cached (oldest), after: newer (oldest)
            mOldestVolume = cachedVolume.newerVolume;
        }
        if ( cachedVolume.newerVolume != kNoVolume ) {
            // Before: newer -> cached -> older, after: newer -> older
            mVolumes[ cachedVolume.newerVolume ].olderVolume = cachedVolume.olderVolume;
        }
    }

    if ( mNewestVolume != kNoVolume ) {
        // Before: newest, after: (newest) cached <- (old) newest
        mVolumes[ mNewestVolume ].newerVolume = volumeIndex;
    }
    if ( mOldestVolume == kNoVolume ) {
        mOldestVolume = volumeIndex;
    }

    // Promoting this volume to the head of the open volumes list.
    cachedVolume.olderVolume = mNewestVolume;
    cachedVolume.newerVolume = kNoVolume;
    mNewestVolume = volumeIndex;
#endif
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    mCurrentVolumeIndex += static_cast< std::size_t >( mCurrentVolumeOffset / mMaxVolumeSize );
    mCurrentVolumeOffset = mCurrentVolumeOffset % mMaxVolumeSize;

    for ( auto newVolumeIndex = mVolumes.size(); newVolumeIndex <= mCurrentVolumeIndex; ++newVolumeIndex ) {
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

    /* Getting the current volume stream. */
    auto& volume = currentVolume();

    try {
        ensureVolumeOpen( volume, mCurrentVolumeIndex );
    } catch ( const BitException& ex ) {
        return ex.hresultCode();
    } catch ( ... ) {
        return E_FAIL;
    }

    if ( mCurrentVolumeOffset != volume.seekPosition ) {
        /* The offset we must write to is different from the last offset we wrote to. */
        UInt64 newPosition{};
        RINOK( volume.stream->Seek( static_cast< Int64 >( mCurrentVolumeOffset ), STREAM_SEEK_SET, &newPosition ) ) //-V3504
        volume.seekPosition = newPosition;
    }

    /* Determining how much we can write to the volume stream */
    const auto writeSize = std::min( size, static_cast< UInt32 >( mMaxVolumeSize - volume.seekPosition ) );

    /* Writing to the volume stream */
    UInt32 writtenSize{};
    RINOK( volume.stream->Write( data, writeSize, &writtenSize ) ) //-V3504

    /* Updating the offsets */
    volume.seekPosition += writtenSize;
    mCurrentVolumeOffset += writtenSize;
    mAbsolutePosition += writtenSize;

    /* We might have written beyond the old known full size of the output archive, updating it. */
    mTotalSize = std::max(mAbsolutePosition, mTotalSize);
    volume.volumeSize = std::max(mCurrentVolumeOffset, volume.volumeSize);

    if ( processedSize != nullptr ) {
        *processedSize += writtenSize;
    }

    if ( volume.seekPosition == mMaxVolumeSize ) {
        /* We reached the max size for the current volume, so we need to continue on the next one. */
        ++mCurrentVolumeIndex;
        mCurrentVolumeOffset = 0;
    }
    return S_OK;
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
    mCurrentVolumeOffset = mAbsolutePosition;
    if ( newPosition != nullptr ) {
        *newPosition = mAbsolutePosition;
    }
    mCurrentVolumeIndex = 0;
    return S_OK;
}

namespace {
// Utility function for computing the ceil division of two unsigned integers.
// Computes ceil( a / b ) for unsigned integers using ( ( a - 1 ) / b ) + 1 instead of the naive
// ( a + b - 1 ) / b to avoid overflow, with an explicit check for a == 0 to prevent underflow.
template<typename T, typename = typename std::enable_if< std::is_unsigned< T >::value >::type >
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
            lastVolume.stream.Release();
#ifndef _WIN32
            // Unlink from the open-volumes list.
            if ( lastVolume.newerVolume != kNoVolume ) {
                mVolumes[ lastVolume.newerVolume ].olderVolume = kNoVolume;
            } else { // lastVolume is the newest volume
                mNewestVolume = lastVolume.olderVolume;
            }
            if ( lastVolume.olderVolume != kNoVolume ) {
                mVolumes[ lastVolume.olderVolume ].newerVolume = kNoVolume;
            } else { // lastVolume is the oldest volume
                mOldestVolume = lastVolume.newerVolume;
            }
            --mOpenCount;
#endif
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

    mCurrentVolumeOffset = mAbsolutePosition;
    mCurrentVolumeIndex = 0;
    mTotalSize = newSize;
    return S_OK;
}

} // namespace bit7z
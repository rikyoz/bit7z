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

#include "internal/cmultivolumeinstream.hpp"

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bittypes.hpp"
#include "internal/fs.hpp"
#include "internal/util.hpp"

#include <algorithm>
#include <cstdint>

#ifndef _WIN32
#include <climits> // For _POSIX_OPEN_MAX
#include <unistd.h> // For sysconf
#endif

namespace bit7z {

CMultiVolumeInStream::CMultiVolumeInStream( const fs::path& firstVolume ) : mAbsolutePosition{ 0 }, mTotalSize{ 0 } {
    constexpr std::size_t kVolumeDigits = 3u;
    std::size_t volumeIndex = 1u;
    fs::path volumePath = firstVolume;
    while ( fs::exists( volumePath ) ) {
        addVolume( volumePath );

        ++volumeIndex;
        tstring volumeExt = to_tstring( volumeIndex );
        if ( volumeExt.length() < kVolumeDigits ) {
            volumeExt.insert( volumeExt.begin(), kVolumeDigits - volumeExt.length(), BIT7Z_STRING( '0' ) );
        }
        volumePath.replace_extension( volumeExt );
    }
}

// NOLINTBEGIN(*-pro-bounds-avoid-unchecked-container-access)
auto CMultiVolumeInStream::currentVolume() -> CachedVolume& {
    std::size_t left = 0;
    std::size_t right = mVolumes.size();
    std::size_t midpoint = mLastOpenedVolume == kNoVolume ? right / 2 : mLastOpenedVolume;
    while ( true ) {
        auto& cachedVolume = mVolumes[ midpoint ];
        if ( mAbsolutePosition < cachedVolume.globalOffset ) {
            // We need to search in [left, midpoint).
            right = midpoint;
        } else if ( mAbsolutePosition >= cachedVolume.globalOffset + cachedVolume.volumeSize ) {
            // We need to search in [midpoint + 1, right).
            left = midpoint + 1;
        } else {
#ifdef _WIN32
            if ( midpoint == mLastOpenedVolume ) {
                return cachedVolume; // Already the last opened, nothing to do.
            }
#else
            if ( midpoint == mNewestVolume ) {
                return cachedVolume; // Already the newest, nothing to do.
            }
#endif

            // Volume found, not the currently newest, so we update it to be the newest.
            ensureVolumeOpen( cachedVolume, midpoint );
            return cachedVolume;
        }
        midpoint = ( left + right ) / 2;
    }
}

#ifndef _WIN32
namespace {
// Here we use the same constants and thresholds used by 7-Zip.

// 7-Zip takes into account this number of handles as reserved for other internal needs of the process.
constexpr std::size_t kHandlesReserve = 10;

// 7-Zip supposes that we have up to 3 file processing tasks within the process.
constexpr std::size_t kProcessingTasks = 3;

auto openHandlesThreshold() -> std::size_t {
    constexpr std::size_t kMinOpenHandles = 3;

    long systemMaxOpenHandles = sysconf( _SC_OPEN_MAX );
    if ( systemMaxOpenHandles < 1 ) {
#ifdef _POSIX_OPEN_MAX
        systemMaxOpenHandles = _POSIX_OPEN_MAX;
#else
        systemMaxOpenHandles = 30;
#endif
    }
    const auto availableOpenHandles = static_cast< std::size_t >( systemMaxOpenHandles );
    const auto result = availableOpenHandles > kHandlesReserve
        ? ( availableOpenHandles - kHandlesReserve ) / kProcessingTasks
        : 1;
    return std::max( result, kMinOpenHandles );
}
} // namespace
#endif

void CMultiVolumeInStream::ensureVolumeOpen( CachedVolume& cachedVolume, VolumeIndex midpoint ) {
#ifdef _WIN32
    if ( cachedVolume.stream == nullptr ) {
        cachedVolume.stream = make_com< CFileInStream >( cachedVolume.volumePath.native() );
    }
    mLastOpenedVolume = midpoint;
#else
    if ( cachedVolume.stream == nullptr ) {
        // The volume was evicted from the LRU list, so we need to reopen it.
        static const auto openedFilesThreshold = openHandlesThreshold();

        // Opening the volume before evicting the oldest one so that
        // we can handle an open failure without evicting the oldest one.
        cachedVolume.stream = make_com< CFileInStream >( cachedVolume.volumePath.native() );
        ++mOpenCount;

        if ( mOpenCount >= openedFilesThreshold ) {
            // Too many open volumes, evicting the oldest one (i.e., the tail of the open volumes list).
            auto& oldest = mVolumes[ mOldestVolume ];

            // The "new" oldest volume is the volume before the "old" oldest volume.
            mOldestVolume = oldest.newerVolume;
            if ( mOldestVolume != kNoVolume ) {
                mVolumes[ mOldestVolume ].olderVolume = kNoVolume;
            }

            // Evicting the "old" oldest volume.
            oldest.newerVolume = kNoVolume;
            oldest.olderVolume = kNoVolume;
            oldest.stream.Release();
            --mOpenCount;
        }

        if ( cachedVolume.seekPosition != 0 ) {
            // Restoring the seek position so Read() can skip re-seeking in the common case.
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
        mVolumes[ mNewestVolume ].newerVolume = midpoint;
    }
    if ( mOldestVolume == kNoVolume ) {
        mOldestVolume = midpoint;
    }

    // Promoting this volume to the head of the open volumes list.
    cachedVolume.olderVolume = mNewestVolume;
    cachedVolume.newerVolume = kNoVolume;
    mNewestVolume = midpoint;
    mLastOpenedVolume = midpoint;
#endif
}
// NOLINTEND(*-pro-bounds-avoid-unchecked-container-access)

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeInStream::Read( void* data, UInt32 size, UInt32* processedSize ) noexcept try {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 || mAbsolutePosition >= mTotalSize ) {
        return S_OK;
    }

    auto& cachedVolume = currentVolume();

    UInt64 localOffset = mAbsolutePosition - cachedVolume.globalOffset;
    HRESULT result = S_OK;
    if ( localOffset != cachedVolume.seekPosition ) {
        result = cachedVolume.stream->Seek( static_cast< Int64 >( localOffset ), STREAM_SEEK_SET, &localOffset );
        if ( result != S_OK ) {
            return result;
        }
        cachedVolume.seekPosition = localOffset;
    }

    const std::uint64_t remaining = cachedVolume.volumeSize - localOffset;
    if ( size > remaining ) {
        size = static_cast< UInt32 >( remaining );
    }
    result = cachedVolume.stream->Read( data, size, &size );
    // Note: size is the number of bytes successfully read by CFileInStream::Read,
    // so we don't need to check for result here, and we can update the positions unconditionally.
    mAbsolutePosition += size;
    cachedVolume.seekPosition += size;

    if ( processedSize != nullptr ) {
        *processedSize = size;
    }
    return result;
} catch ( const BitException& ex ) {
    return ex.hresultCode();
} catch ( ... ) {
    return E_FAIL;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeInStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
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

void CMultiVolumeInStream::addVolume( const fs::path& volumePath ) {
    const auto volumeSize = fs::file_size( volumePath );
    if ( volumeSize == 0 ) {
        throw BitException{ "Invalid volume archive", make_error_code( BitError::Fail ) };
    }
    mTotalSize += volumeSize;

    const std::uint64_t globalOffset = [&]() -> std::uint64_t {
        if ( mVolumes.empty() ) {
            return 0;
        }

        const auto& lastStream = mVolumes.back();
        return lastStream.globalOffset + lastStream.volumeSize;
    }();
    CachedVolume cachedVolume{ volumePath, volumeSize, globalOffset, 0u, {} };
    mVolumes.push_back( std::move( cachedVolume ) );
}

} // namespace bit7z

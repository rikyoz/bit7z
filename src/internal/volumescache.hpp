/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2026 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef VOLUMESCACHE_HPP
#define VOLUMESCACHE_HPP

#include "internal/com.hpp"
#include "internal/fs.hpp"

#include <limits>
#include <type_traits>

#ifndef _WIN32
#include <climits> // For _POSIX_OPEN_MAX
#include <unistd.h>
#endif

namespace bit7z {

constexpr auto kNoVolume = std::numeric_limits< std::size_t >::max();

enum class EvictionPolicy : std::uint8_t {
    Oldest,
    Newest
};

template< typename T >
struct CachedVolume final {
    fs::path volumePath;
    std::uint64_t volumeSize;
    std::uint64_t globalOffset;
    std::uint64_t seekPosition;
    CMyComPtr< T > stream;
#ifndef _WIN32
    std::size_t newerVolume = kNoVolume;
    std::size_t olderVolume = kNoVolume;
#endif
};

#ifdef _WIN32

// On Windows there is no open-handles limit to work around, so VolumesCache is just a plain
// std::vector and the EvictionPolicy parameter is unused; the default lets the template-argument
// list match the non-Windows definition so call sites can spell the type the same way on both platforms.
template< typename T, EvictionPolicy = EvictionPolicy::Oldest >
using VolumesCache = std::vector< CachedVolume< T > >;

#else // !_WIN32

BIT7Z_ALWAYS_INLINE
auto openHandlesThreshold() -> std::size_t {
    // Here we use the same constants and thresholds used by 7-Zip.

    // 7-Zip takes into account this number of handles as reserved for other internal needs of the process.
    constexpr std::size_t kHandlesReserve = 10;

    // 7-Zip supposes that we have up to 3 file processing tasks within the process.
    constexpr std::size_t kProcessingTasks = 3;

    // We want at least 3 open handles as a threshold.
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

/**
 * @brief Wraps a std::vector of CachedVolume with an LRU doubly-linked list of open volumes,
 * threaded through CachedVolume::newerVolume/olderVolume.
 * The "newest" end is the head (most recently accessed), the "oldest" end is the tail.
 */
template< typename T, EvictionPolicy Policy >
class VolumesCache {
        std::vector< CachedVolume< T > > mVolumes;
        std::size_t mOpenCount = 0;
        std::size_t mNewest = kNoVolume;
        std::size_t mOldest = kNoVolume;

        // Links a volume as the newest (head of the list).
        void linkAsNewest( CachedVolume< T >& volume, std::size_t index ) noexcept {
            if ( mNewest != kNoVolume ) {
                mVolumes[ mNewest ].newerVolume = index; // NOLINT(*-pro-bounds-*)
            }
            if ( mOldest == kNoVolume ) {
                mOldest = index;
            }
            volume.olderVolume = mNewest;
            volume.newerVolume = kNoVolume;
            mNewest = index;
        }

        // Evicts the oldest volume (tail of the list). Releases its stream.
        template< EvictionPolicy P = Policy >
        auto evict() noexcept -> typename std::enable_if< P == EvictionPolicy::Oldest >::type {
            auto& evicted = mVolumes[ mOldest ]; // NOLINT(*-pro-bounds-*)
            mOldest = evicted.newerVolume;
            if ( mOldest != kNoVolume ) {
                mVolumes[ mOldest ].olderVolume = kNoVolume; // NOLINT(*-pro-bounds-*)
            } else {
                mNewest = kNoVolume;
            }
            evicted.newerVolume = kNoVolume;
            evicted.olderVolume = kNoVolume;
            evicted.stream.Release();
            --mOpenCount;
        }

        // Evicts the newest volume (head of the list). Releases its stream.
        template< EvictionPolicy P = Policy >
        auto evict() noexcept -> typename std::enable_if< P == EvictionPolicy::Newest >::type {
            auto& evicted = mVolumes[ mNewest ]; // NOLINT(*-pro-bounds-*)
            mNewest = evicted.olderVolume;
            if ( mNewest != kNoVolume ) {
                mVolumes[ mNewest ].newerVolume = kNoVolume; // NOLINT(*-pro-bounds-*)
            } else {
                mOldest = kNoVolume;
            }
            evicted.newerVolume = kNoVolume;
            evicted.olderVolume = kNoVolume;
            evicted.stream.Release();
            --mOpenCount;
        }

        // Tracks a newly opened stream and evicts a volume if the open-handles threshold is reached.
        void trackOpened() noexcept {
            ++mOpenCount;
            static const auto threshold = openHandlesThreshold();
            if ( mOpenCount >= threshold ) {
                evict();
            }
        }

    public:
        /** @brief Returns the cached volume at the given index. */
        BIT7Z_NODISCARD
        auto operator[]( std::size_t index ) noexcept -> CachedVolume< T >& {
            return mVolumes[ index ]; // NOLINT(*-pro-bounds-*)
        }

        /** @copydoc operator[]() */
        BIT7Z_NODISCARD
        auto operator[]( std::size_t index ) const noexcept -> const CachedVolume< T >& {
            return mVolumes[ index ]; // NOLINT(*-pro-bounds-*)
        }

        /** @brief Returns the number of cached volumes. */
        BIT7Z_NODISCARD
        auto size() const noexcept -> std::size_t {
            return mVolumes.size();
        }

        /** @brief Returns whether the cache contains no volumes. */
        BIT7Z_NODISCARD
        auto empty() const noexcept -> bool {
            return mVolumes.empty();
        }

        /** @brief Returns the last cached volume. */
        BIT7Z_NODISCARD
        auto back() noexcept -> CachedVolume< T >& {
            return mVolumes.back();
        }

        /** @copydoc back() */
        BIT7Z_NODISCARD
        auto back() const noexcept -> const CachedVolume< T >& {
            return mVolumes.back();
        }

        /** @brief Returns the maximum number of volumes the cache can hold. */
        BIT7Z_NODISCARD
        auto max_size() const noexcept -> std::size_t {
            return mVolumes.max_size();
        }

        /** @brief Appends a cached volume by move. */
        void push_back( CachedVolume< T >&& volume ) {
            mVolumes.push_back( std::move( volume ) );
        }

        /** @brief Constructs a cached volume in-place at the end. */
        template< typename... Args >
        void emplace_back( Args&&... args ) {
            mVolumes.emplace_back( std::forward< Args >( args )... );
        }

        /** @brief Removes the last cached volume. */
        void pop_back() {
            mVolumes.pop_back();
        }

        /** @brief Returns the index of the most recently accessed open volume, or kNoVolume if none. */
        BIT7Z_NODISCARD
        auto newest() const noexcept -> std::size_t {
            return mNewest;
        }

        /**
         * @brief Tracks a reopened volume: evicts if needed, restores seek position, and links as newest.
         *
         * Call this after opening the stream on a volume that was previously evicted (stream was null).
         *
         * @param volume The volume whose stream was just reopened.
         * @param index  The index of the volume in the cache.
         */
        void trackReopen( CachedVolume< T >& volume, std::size_t index ) noexcept {
            trackOpened();
            if ( volume.seekPosition != 0 ) {
                UInt64 newPosition{}; // UInt64 (not std::uint64_t) to match Seek's output parameter type.
                const HRESULT seekResult = volume.stream->Seek(
                    static_cast< Int64 >( volume.seekPosition ),
                    STREAM_SEEK_SET,
                    &newPosition
                );
                if ( seekResult != S_OK ) {
                    volume.seekPosition = 0; // Failed to seek, stream is still at the beginning.
                } else {
                    volume.seekPosition = newPosition;
                }
            }
            linkAsNewest( volume, index );
        }

        /**
         * @brief Promotes an already-open volume to the newest position in the LRU list.
         *
         * @param volume The volume to promote.
         * @param index  The index of the volume in the cache.
         */
        void promote( CachedVolume< T >& volume, std::size_t index ) noexcept {
            unlink( volume );
            linkAsNewest( volume, index );
        }

        /**
         * @brief Unlinks a volume from its current position in the LRU list (does not release the stream).
         *
         * @param volume The volume to unlink.
         */
        void unlink( CachedVolume< T >& volume ) noexcept {
            if ( volume.olderVolume != kNoVolume ) {
                mVolumes[ volume.olderVolume ].newerVolume = volume.newerVolume; // NOLINT(*-pro-bounds-*)
            } else {
                mOldest = volume.newerVolume;
            }
            if ( volume.newerVolume != kNoVolume ) {
                mVolumes[ volume.newerVolume ].olderVolume = volume.olderVolume; // NOLINT(*-pro-bounds-*)
            } else {
                mNewest = volume.olderVolume;
            }
            volume.olderVolume = kNoVolume;
            volume.newerVolume = kNoVolume;
        }

        /**
         * @brief Decrements the open-volume count to account for a volume's stream being released externally.
         *
         * The ordering relative to the external Release() call is not significant, since this method
         * only adjusts the LRU bookkeeping counter.
         */
        void trackClosed() noexcept {
            --mOpenCount;
        }
};

#endif // !_WIN32

} // namespace bit7z

#endif //VOLUMESCACHE_HPP

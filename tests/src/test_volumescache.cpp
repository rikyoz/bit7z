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

#ifndef _WIN32

#include <catch2/catch.hpp>

#include <bitwindows.hpp>
#include <internal/guids.hpp>
#include <internal/macros.hpp>
#include <internal/util.hpp>
#include <internal/volumescache.hpp>

#include <7zip/IStream.h>

#include <vector>

namespace bit7z {
namespace {

// MockStream must live inside namespace bit7z so that MY_UNKNOWN_IMP1's unqualified reference
// to IID_IInStream finds bit7z::IID_IInStream before the homonymous symbol in 7-Zip's global
// namespace, avoiding an ambiguous lookup on platforms where IStream.h declares it at global scope.
// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
class MockStream final : public IInStream, public CMyUnknownImp {
    public:
        // NOLINTNEXTLINE(*-const-correctness, *-use-noexcept, *-use-trailing-return-type)
        MY_UNKNOWN_IMP1( IInStream ) //-V2507 //-V2511 //-V835 //-V3504

        BIT7Z_STDMETHOD( Read, void* /*data*/, UInt32 /*size*/, UInt32* processedSize ) {
            if ( processedSize != nullptr ) {
                *processedSize = 0;
            }
            return S_OK;
        }

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 /*seekOrigin*/, UInt64* newPosition ) {
            mSeekWasCalled = true;
            mLastSeekOffset = offset;
            if ( mNextSeekResult != S_OK ) {
                return mNextSeekResult;
            }
            if ( newPosition != nullptr ) {
                *newPosition = static_cast< UInt64 >( offset );
            }
            return S_OK;
        }

        BIT7Z_NODISCARD
        auto seekWasCalled() const -> bool {
            return mSeekWasCalled;
        }

        BIT7Z_NODISCARD
        auto lastSeekOffset() const -> Int64 {
            return mLastSeekOffset;
        }

        void setNextSeekResult( HRESULT value ) {
            mNextSeekResult = value;
        }

    private:
        bool mSeekWasCalled = false;
        Int64 mLastSeekOffset = 0;
        HRESULT mNextSeekResult = S_OK;
};

// NOLINTBEGIN(*-pro-bounds-avoid-unchecked-container-access)

template< EvictionPolicy Policy >
void addOpenVolume( VolumesCache< MockStream, Policy >& cache ) {
    const auto index = cache.size();
    cache.emplace_back();
    auto& volume = cache[ index ];
    volume.stream = make_com< MockStream >();
    cache.trackReopen( volume, index );
}

template< EvictionPolicy Policy >
auto lruOrder( VolumesCache< MockStream, Policy >& cache ) -> std::vector< std::size_t > {
    std::vector< std::size_t > result;
    auto index = cache.newest();
    while ( index != kNoVolume ) {
        result.push_back( index );
        index = cache[ index ].olderVolume;
    }
    return result;
}

} // namespace

// Container operations

TEST_CASE( "VolumesCache: Empty cache", "[volumescache]" ) {
    const VolumesCache< MockStream, EvictionPolicy::Oldest > cache;
    REQUIRE( cache.empty() );
    REQUIRE( cache.size() == 0 ); // NOLINT(*-container-size-empty)
    REQUIRE( cache.newest() == kNoVolume );
}

TEST_CASE( "VolumesCache: Container operations", "[volumescache]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;

    SECTION( "emplace_back increases size" ) {
        cache.emplace_back();
        REQUIRE( cache.size() == 1 );
        REQUIRE_FALSE( cache.empty() );

        cache.emplace_back();
        REQUIRE( cache.size() == 2 );
    }

    SECTION( "push_back increases size" ) {
        CachedVolume< MockStream > volume{};
        cache.push_back( std::move( volume ) );
        REQUIRE( cache.size() == 1 );
    }

    SECTION( "operator[] accesses the correct element" ) {
        cache.emplace_back();
        cache.emplace_back();
        cache[ 0 ].volumeSize = 100;
        cache[ 1 ].volumeSize = 200;
        REQUIRE( cache[ 0 ].volumeSize == 100 );
        REQUIRE( cache[ 1 ].volumeSize == 200 );
    }

    SECTION( "back returns the last element" ) {
        cache.emplace_back();
        cache[ 0 ].volumeSize = 42;
        REQUIRE( cache.back().volumeSize == 42 );

        cache.emplace_back();
        cache[ 1 ].volumeSize = 99;
        REQUIRE( cache.back().volumeSize == 99 );
    }

    SECTION( "pop_back removes the last element" ) {
        cache.emplace_back();
        cache.emplace_back();
        REQUIRE( cache.size() == 2 );
        cache.pop_back();
        REQUIRE( cache.size() == 1 );
    }

    SECTION( "const operator[] is available" ) {
        cache.emplace_back();
        cache[ 0 ].volumeSize = 42;
        const auto& constCache = cache;
        REQUIRE( constCache[ 0 ].volumeSize == 42 );
    }

    SECTION( "const back is available" ) {
        cache.emplace_back();
        cache[ 0 ].volumeSize = 42;
        const auto& constCache = cache;
        REQUIRE( constCache.back().volumeSize == 42 );
    }
}

// LRU list management

TEST_CASE( "VolumesCache: trackReopen builds the LRU list", "[volumescache][lru]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;

    SECTION( "Single volume becomes newest" ) {
        addOpenVolume( cache );
        REQUIRE( cache.newest() == 0 );
        REQUIRE( cache[ 0 ].newerVolume == kNoVolume );
        REQUIRE( cache[ 0 ].olderVolume == kNoVolume );
    }

    SECTION( "Two volumes: second becomes newest" ) {
        addOpenVolume( cache );
        addOpenVolume( cache );
        REQUIRE( cache.newest() == 1 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 1, 0 } );

        REQUIRE( cache[ 0 ].newerVolume == 1 );
        REQUIRE( cache[ 0 ].olderVolume == kNoVolume );
        REQUIRE( cache[ 1 ].newerVolume == kNoVolume );
        REQUIRE( cache[ 1 ].olderVolume == 0 );
    }

    SECTION( "Three volumes: last opened is newest, first is oldest" ) {
        addOpenVolume( cache );
        addOpenVolume( cache );
        addOpenVolume( cache );
        REQUIRE( cache.newest() == 2 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 2, 1, 0 } );
    }
}

TEST_CASE( "VolumesCache: promote reorders the LRU list", "[volumescache][lru]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;

    // Build list: [0 (oldest)] <- [1] <- [2 (newest)]
    addOpenVolume( cache );
    addOpenVolume( cache );
    addOpenVolume( cache );

    SECTION( "Promote the oldest to newest" ) {
        cache.promote( cache[ 0 ], 0 );
        REQUIRE( cache.newest() == 0 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 0, 2, 1 } );
    }

    SECTION( "Promote the middle element to newest" ) {
        cache.promote( cache[ 1 ], 1 );
        REQUIRE( cache.newest() == 1 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 1, 2, 0 } );
    }

    SECTION( "Promote the newest is a no-op reorder" ) {
        cache.promote( cache[ 2 ], 2 );
        REQUIRE( cache.newest() == 2 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 2, 1, 0 } );
    }
}

TEST_CASE( "VolumesCache: promote with two elements", "[volumescache][lru]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;

    addOpenVolume( cache );
    addOpenVolume( cache );
    // List: [0 (oldest)] <- [1 (newest)]

    SECTION( "Promote the oldest" ) {
        cache.promote( cache[ 0 ], 0 );
        REQUIRE( cache.newest() == 0 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 0, 1 } );
    }

    SECTION( "Promote the newest" ) {
        cache.promote( cache[ 1 ], 1 );
        REQUIRE( cache.newest() == 1 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 1, 0 } );
    }
}

TEST_CASE( "VolumesCache: promote the only element", "[volumescache][lru]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;
    addOpenVolume( cache );

    cache.promote( cache[ 0 ], 0 );
    REQUIRE( cache.newest() == 0 );
    REQUIRE( cache[ 0 ].newerVolume == kNoVolume );
    REQUIRE( cache[ 0 ].olderVolume == kNoVolume );
}

TEST_CASE( "VolumesCache: unlink removes a volume from the LRU list", "[volumescache][lru]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;

    addOpenVolume( cache );
    addOpenVolume( cache );
    addOpenVolume( cache );
    // List: [0 (oldest)] <- [1] <- [2 (newest)]

    SECTION( "Unlink the middle element" ) {
        cache.unlink( cache[ 1 ] );
        REQUIRE( cache.newest() == 2 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 2, 0 } );
        REQUIRE( cache[ 1 ].newerVolume == kNoVolume );
        REQUIRE( cache[ 1 ].olderVolume == kNoVolume );
    }

    SECTION( "Unlink the oldest" ) {
        cache.unlink( cache[ 0 ] );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 2, 1 } );
        REQUIRE( cache[ 0 ].newerVolume == kNoVolume );
        REQUIRE( cache[ 0 ].olderVolume == kNoVolume );
    }

    SECTION( "Unlink the newest" ) {
        cache.unlink( cache[ 2 ] );
        REQUIRE( cache.newest() == 1 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 1, 0 } );
        REQUIRE( cache[ 2 ].newerVolume == kNoVolume );
        REQUIRE( cache[ 2 ].olderVolume == kNoVolume );
    }

    SECTION( "Unlink all elements one by one" ) {
        cache.unlink( cache[ 1 ] );
        cache.unlink( cache[ 0 ] );
        REQUIRE( cache.newest() == 2 );
        REQUIRE( lruOrder( cache ) == std::vector< std::size_t >{ 2 } );

        cache.unlink( cache[ 2 ] );
        REQUIRE( cache.newest() == kNoVolume );
        REQUIRE( lruOrder( cache ).empty() );
    }
}

TEST_CASE( "VolumesCache: unlink the only element", "[volumescache][lru]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;
    addOpenVolume( cache );

    cache.unlink( cache[ 0 ] );
    REQUIRE( cache.newest() == kNoVolume );
    REQUIRE( lruOrder( cache ).empty() );
    REQUIRE( cache[ 0 ].newerVolume == kNoVolume );
    REQUIRE( cache[ 0 ].olderVolume == kNoVolume );
}

// Eviction

TEST_CASE( "VolumesCache: Eviction with Oldest policy", "[volumescache][eviction]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;
    const auto threshold = openHandlesThreshold();
    REQUIRE( threshold >= 3 );

    // Open (threshold - 1) volumes without triggering eviction.
    for ( std::size_t i = 0; i < threshold - 1; ++i ) {
        addOpenVolume( cache );
    }
    for ( std::size_t i = 0; i < threshold - 1; ++i ) {
        REQUIRE( cache[ i ].stream != nullptr );
    }

    // Opening the threshold-th volume triggers eviction of the oldest (index 0).
    addOpenVolume( cache );
    REQUIRE( cache[ 0 ].stream == nullptr );
    REQUIRE( cache[ 0 ].newerVolume == kNoVolume );
    REQUIRE( cache[ 0 ].olderVolume == kNoVolume );
    REQUIRE( cache[ 1 ].stream != nullptr );
    REQUIRE( cache[ threshold - 1 ].stream != nullptr );
    REQUIRE( cache.newest() == threshold - 1 );

    SECTION( "Opening another volume evicts the next oldest" ) {
        addOpenVolume( cache );
        REQUIRE( cache[ 1 ].stream == nullptr );
        REQUIRE( cache[ 1 ].newerVolume == kNoVolume );
        REQUIRE( cache[ 1 ].olderVolume == kNoVolume );
        REQUIRE( cache[ 2 ].stream != nullptr );
        REQUIRE( cache[ threshold ].stream != nullptr );
    }
}

TEST_CASE( "VolumesCache: Eviction with Newest policy", "[volumescache][eviction]" ) {
    VolumesCache< MockStream, EvictionPolicy::Newest > cache;
    const auto threshold = openHandlesThreshold();
    REQUIRE( threshold >= 3 );

    // Open (threshold - 1) volumes without triggering eviction.
    for ( std::size_t i = 0; i < threshold - 1; ++i ) {
        addOpenVolume( cache );
    }
    for ( std::size_t i = 0; i < threshold - 1; ++i ) {
        REQUIRE( cache[ i ].stream != nullptr );
    }

    // Opening the threshold-th volume triggers eviction of the newest before it (threshold - 2).
    addOpenVolume( cache );
    REQUIRE( cache[ threshold - 2 ].stream == nullptr );
    REQUIRE( cache[ threshold - 2 ].newerVolume == kNoVolume );
    REQUIRE( cache[ threshold - 2 ].olderVolume == kNoVolume );
    REQUIRE( cache[ 0 ].stream != nullptr );
    REQUIRE( cache[ threshold - 1 ].stream != nullptr );
    REQUIRE( cache.newest() == threshold - 1 );

    // Expected LRU order (newest to oldest): the just-added volume, then the untouched
    // volumes from threshold - 3 down to 0 (threshold - 2 was evicted).
    std::vector< std::size_t > expectedOrder{ threshold - 1 };
    for ( std::size_t i = threshold - 3; i != kNoVolume; --i ) {
        expectedOrder.push_back( i );
    }
    REQUIRE( lruOrder( cache ) == expectedOrder );

    SECTION( "Opening another volume evicts the previous newest" ) {
        // The current newest is (threshold - 1); opening one more should evict it.
        addOpenVolume( cache );
        REQUIRE( cache[ threshold - 1 ].stream == nullptr );
        REQUIRE( cache[ 0 ].stream != nullptr );
        REQUIRE( cache[ threshold ].stream != nullptr );
        REQUIRE( cache.newest() == threshold );

        // (threshold - 1) was evicted; the rest of the list from (threshold - 3) down to 0 is unchanged.
        std::vector< std::size_t > expectedOrderAfter{ threshold };
        for ( std::size_t i = threshold - 3; i != kNoVolume; --i ) {
            expectedOrderAfter.push_back( i );
        }
        REQUIRE( lruOrder( cache ) == expectedOrderAfter );
    }
}

TEST_CASE( "VolumesCache: trackClosed delays eviction", "[volumescache][eviction]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;
    const auto threshold = openHandlesThreshold();
    REQUIRE( threshold >= 3 );

    // Open (threshold - 1) volumes.
    for ( std::size_t i = 0; i < threshold - 1; ++i ) {
        addOpenVolume( cache );
    }

    // Manually close one volume (unlink + release + trackClosed).
    cache.unlink( cache[ 0 ] );
    cache[ 0 ].stream.Release();
    cache.trackClosed();

    // Now open count is (threshold - 2), so opening one more doesn't trigger eviction.
    addOpenVolume( cache );
    for ( std::size_t i = 1; i < threshold; ++i ) {
        REQUIRE( cache[ i ].stream != nullptr );
    }

    // Opening yet another volume reaches the threshold and triggers eviction of the oldest (index 1).
    addOpenVolume( cache );
    REQUIRE( cache[ 1 ].stream == nullptr );
    REQUIRE( cache[ 2 ].stream != nullptr );
    REQUIRE( cache[ threshold ].stream != nullptr );
}

// Seek restoration

TEST_CASE( "VolumesCache: trackReopen restores seek position", "[volumescache][seek]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;

    SECTION( "Seek is called when seekPosition is non-zero" ) {
        cache.emplace_back();
        auto& volume = cache[ 0 ];
        volume.seekPosition = 42;
        volume.stream = make_com< MockStream >();

        cache.trackReopen( volume, 0 );

        REQUIRE( volume.stream->seekWasCalled() );
        REQUIRE( volume.stream->lastSeekOffset() == 42 );
        REQUIRE( volume.seekPosition == 42 );
    }

    SECTION( "Seek is not called when seekPosition is zero" ) {
        cache.emplace_back();
        auto& volume = cache[ 0 ];
        volume.seekPosition = 0;
        volume.stream = make_com< MockStream >();

        cache.trackReopen( volume, 0 );

        REQUIRE_FALSE( volume.stream->seekWasCalled() );
    }

    SECTION( "Seek failure resets seekPosition to zero" ) {
        cache.emplace_back();
        auto& volume = cache[ 0 ];
        volume.seekPosition = 42;
        volume.stream = make_com< MockStream >();
        volume.stream->setNextSeekResult( E_FAIL );

        cache.trackReopen( volume, 0 );

        REQUIRE( volume.stream->seekWasCalled() );
        REQUIRE( volume.seekPosition == 0 );
    }
}

TEST_CASE( "VolumesCache: trackReopen restores seek after eviction", "[volumescache][eviction][seek]" ) {
    VolumesCache< MockStream, EvictionPolicy::Oldest > cache;
    const auto threshold = openHandlesThreshold();
    REQUIRE( threshold >= 3 );

    // Open (threshold - 1) volumes, each with a distinct non-zero seek position.
    for ( std::size_t i = 0; i < threshold - 1; ++i ) {
        const auto index = cache.size();
        cache.emplace_back();
        auto& volume = cache[ index ];
        volume.seekPosition = ( index + 1 ) * 100;
        volume.stream = make_com< MockStream >();
        cache.trackReopen( volume, index );
    }

    // Opening the threshold-th volume evicts the oldest (index 0); its seek position is preserved.
    addOpenVolume( cache );
    REQUIRE( cache[ 0 ].stream == nullptr );
    REQUIRE( cache[ 0 ].seekPosition == 100 );

    // Simulate the production reopen path: caller assigns a fresh stream, then calls trackReopen.
    cache[ 0 ].stream = make_com< MockStream >();
    cache.trackReopen( cache[ 0 ], 0 );

    REQUIRE( cache[ 0 ].stream->seekWasCalled() );
    REQUIRE( cache[ 0 ].stream->lastSeekOffset() == 100 );
    REQUIRE( cache[ 0 ].seekPosition == 100 );
    REQUIRE( cache.newest() == 0 );
}

} // namespace bit7z

// NOLINTEND(*-pro-bounds-avoid-unchecked-container-access)

#endif // !_WIN32

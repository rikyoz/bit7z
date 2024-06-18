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

#include "bitnestedarchivereader.hpp"

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/csynchronizedinstream.hpp"
#include <internal/util.hpp>

#include <7zip/Archive/IArchive.h>

#ifdef _WIN32
#include <Windows.h>
#elif defined( __APPLE__ )
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#if !defined( _SC_AVPHYS_PAGES ) || !defined( _SC_PAGE_SIZE )
# include <sys/sysinfo.h>
#endif
#endif

#include <utility>

namespace bit7z {

// Minimum value for the maximum memory usage allowed for the BufferQueue.
constexpr std::uint64_t kMinMaxMemoryUsage = 4ULL * 1024 * 1024; // 4 MiB

auto get_free_ram() -> std::uint64_t {
#if defined( _WIN64 )
    MEMORYSTATUSEX memStatus{};
    memStatus.dwLength = sizeof( memStatus );
    GlobalMemoryStatusEx( &memStatus );
    return memStatus.ullAvailPhys;
#elif defined( _WIN32 )
    MEMORYSTATUS memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUS);
    ::GlobalMemoryStatus(&memStatus);
    return memStatus.dwAvailPhys;
#elif defined( __APPLE__ )
    static int mib[] = { CTL_HW, HW_USERMEM };
    std::uint64_t value = 0;
    std::size_t length = sizeof( value );

    if ( sysctl( mib, 2, &value, &length, nullptr, 0 ) == 0 ) {
        return value;
    }
    return 0;
#elif defined( _SC_AVPHYS_PAGES ) && defined( _SC_PAGE_SIZE )
    long pages = sysconf( _SC_AVPHYS_PAGES );
    long page_size = sysconf( _SC_PAGE_SIZE );
    if ( pages < 0 || page_size < 0 ) {
        return 0;
    }
    return static_cast< std::uint64_t >( pages ) * static_cast< std::uint64_t >( page_size );
#else
    struct sysinfo info{};
    sysinfo( &info );
    return ( info.freeram + info.bufferram ) * info.mem_unit;
#endif
}

#ifdef BIT7Z_AUTO_FORMAT
inline auto validateFormat( const BitInFormat& format ) -> const BitInFormat& {
    if ( format == BitFormat::Auto ) {
        throw BitException{ "Automatic format detection not supported in nested archives",
                            BitError::UnsupportedOperation };
    }
    return format;
}
#define VALIDATE_FORMAT(x) validateFormat(x)
#else
#define VALIDATE_FORMAT(x) x
#endif

BitNestedArchiveReader::BitNestedArchiveReader( const Bit7zLibrary& lib,
                                                const BitInputArchive& parentArchive,
                                                const BitInFormat& format,
                                                const tstring& password )
    : BitNestedArchiveReader{ lib, parentArchive, 0, format, password } {}

BitNestedArchiveReader::BitNestedArchiveReader( const Bit7zLibrary& lib,
                                                const BitInputArchive& parentArchive,
                                                std::uint32_t index,
                                                const BitInFormat& format,
                                                const tstring& password )
    : BitAbstractArchiveOpener{ lib, VALIDATE_FORMAT( format ), password },
      mNestedArchive{ *this, parentArchive.itemAt( index ) },
      mParentArchive{ parentArchive },
      mIndexInParent{ index },
      mMaxMemoryUsage{ std::max( get_free_ram() / 4, kMinMaxMemoryUsage ) },
      mCachedItemsCount{ 0 },
      mLastReadItem{ std::numeric_limits< decltype( mLastReadItem ) >::max() },
      mOpenCount{ 0 } {}

auto BitNestedArchiveReader::maxMemoryUsage() const noexcept -> std::uint64_t {
    return mMaxMemoryUsage;
}

auto BitNestedArchiveReader::detectedFormat() const noexcept -> const BitInFormat& {
    return mNestedArchive.detectedFormat();
}

auto BitNestedArchiveReader::archiveProperty( BitProperty property ) const -> BitPropVariant {
    return mNestedArchive.archiveProperty( property );
}

auto BitNestedArchiveReader::itemProperty( std::uint32_t index, BitProperty property ) const -> BitPropVariant {
    if ( needReopen( index ) ) {
        openSequentially();
    }

    const auto result = mNestedArchive.itemProperty( index, property );
    mLastReadItem = index;
    return result;
}

auto BitNestedArchiveReader::calculateItemsCount() const -> std::uint32_t {
    if ( needReopen() ) {
        openSequentially();
    }

    for ( std::uint32_t index = 0; index < std::numeric_limits< std::uint32_t >::max(); ++index ) {
        /* All archive formats provide BitProperty::IsDir for _valid_ items,
         * so if the item at the index doesn't have this property,
         * it means the archive doesn't have an item at the given index. */
        if ( !mNestedArchive.itemHasProperty( index, BitProperty::IsDir ) ) {
            mLastReadItem = index;
            return index;
        }
    }
    return 0;
}

auto BitNestedArchiveReader::itemsCount() const -> std::uint32_t {
    if ( mCachedItemsCount > 0 ) {
        return mCachedItemsCount;
    }

    mCachedItemsCount = mNestedArchive.itemsCount();
    if ( mCachedItemsCount == std::numeric_limits< std::uint32_t >::max() ) {
        mCachedItemsCount = calculateItemsCount();
    }
    return mCachedItemsCount;
}

auto BitNestedArchiveReader::items() const -> std::vector< BitArchiveItemInfo > {
    if ( needReopen() ) {
        openSequentially();
    }

    std::vector< BitArchiveItemInfo > result;

    /* The TAR format always reports std::numeric_limits< std::uint32_t >::max()
     * as itemsCount() when the archive is opened sequentially.
     * Other formats that support sequential opening only support single file compression.
     * Therefore:
     * - For TAR archives, we don't know the actual number of items in the archive,
     *   so we can't reserve space in the vector (as we do in BitArchiveReader::items()),
     *   and we stop when we encounter the first item not reporting the BitProperty::IsDir property.
     * - For other archives, we stop when we reach itemsCount() (most likely one) items added to the vector. */
    const auto itemsCount = mNestedArchive.itemsCount();
    if ( itemsCount < std::numeric_limits< std::uint32_t >::max() ) {
        result.reserve( itemsCount );
    }

    for ( std::uint32_t index = 0; index < itemsCount; ++index ) {
        if ( !mNestedArchive.itemHasProperty( index, BitProperty::IsDir ) ) {
            mLastReadItem = index;
            return result;
        }

        BitArchiveItemInfo item( index );
        for ( std::uint32_t j = kpidNoProperty; j <= kpidCopyLink; ++j ) {
            // We cast property twice (here and in itemProperty), to make the code is easier to read.
            const auto property = static_cast< BitProperty >( j );
            const auto propertyValue = mNestedArchive.itemProperty( index, property );

            if ( !propertyValue.isEmpty() ) {
                item.setProperty( property, propertyValue );
            }
        }
        result.push_back( std::move( item ) );
    }
    mLastReadItem = std::numeric_limits< decltype( mLastReadItem ) >::max();
    return result;
}

void BitNestedArchiveReader::extractTo( const tstring& outDir ) const {
    if ( needReopen() ) {
        openSequentially();
    }
    mNestedArchive.extractTo( outDir );
    mLastReadItem = std::numeric_limits< decltype( mLastReadItem ) >::max();
}

void BitNestedArchiveReader::extractTo( std::map< tstring, buffer_t >& outMap ) const {
    if ( needReopen() ) {
        openSequentially();
    }
    mNestedArchive.extractTo( outMap );
    mLastReadItem = std::numeric_limits< decltype( mLastReadItem ) >::max();
}

void BitNestedArchiveReader::test() const {
    if ( needReopen() ) {
        openSequentially();
    }
    mNestedArchive.test();
    mLastReadItem = std::numeric_limits< decltype( mLastReadItem ) >::max();
}

void BitNestedArchiveReader::openSequentially() const {
    auto stream = bit7z::make_com< CSynchronizedInStream, ISequentialInStream >( mMaxMemoryUsage,
                                                                                 mParentArchive,
                                                                                 mIndexInParent );
    mNestedArchive.openArchiveSeqStream( stream );
    mLastReadItem = 0;
    ++mOpenCount;
}

auto BitNestedArchiveReader::needReopen( std::uint32_t index ) const -> bool {
    return index < mLastReadItem;
}

auto BitNestedArchiveReader::openCount() const -> std::size_t {
    return mOpenCount;
}

void BitNestedArchiveReader::setMaxMemoryUsage( std::uint64_t value ) noexcept {
    // TODO: Throw an exception if the value is below the minimum?
    mMaxMemoryUsage = std::max( value, kMinMaxMemoryUsage );
}

} // namespace bit7z
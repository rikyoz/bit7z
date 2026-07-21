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

#include "bitnestedarchivereader.hpp"

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/csynchronizedinstream.hpp"
#include <internal/util.hpp>

#ifdef _WIN32
#include <windows.h>
#elif defined( __APPLE__ ) || defined( BSD ) || \
      defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __DragonFly__ )
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
constexpr std::uint64_t kMinMaxMemoryUsage = 4ULL * 1024 * 1024; // 4 MiB //-V112

// Sentinels: IInArchive::GetNumberOfItems/GetProperty/Extract all index items via UInt32,
// so no archive can ever report more items than max() in the first place; safe to reserve as "unset".
constexpr std::uint32_t kItemsCountUnset = std::numeric_limits< std::uint32_t >::max();
constexpr std::uint32_t kNoItemRead = std::numeric_limits< std::uint32_t >::max();

namespace {
auto getFreeRam() -> std::uint64_t {
#if defined( _WIN64 ) || defined( _WIN32 )
    MEMORYSTATUSEX memStatus{};
    memStatus.dwLength = sizeof( memStatus );
    GlobalMemoryStatusEx( &memStatus );
    return memStatus.ullAvailPhys;
#elif defined( __APPLE__ ) || defined( BSD ) || \
      defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __DragonFly__ )
    static int mib[] = { CTL_HW, HW_USERMEM };
    std::uint64_t value = 0;
    std::size_t length = sizeof( value );

    if ( sysctl( mib, 2, &value, &length, nullptr, 0 ) == 0 ) {
        return value;
    }
    return 0;
#elif defined( _SC_AVPHYS_PAGES ) && defined( _SC_PAGE_SIZE )
    const long pages = sysconf( _SC_AVPHYS_PAGES );
    const long page_size = sysconf( _SC_PAGE_SIZE );
    if ( pages < 0 || page_size < 0 ) {
        return 0;
    }
    return static_cast< std::uint64_t >( pages ) * static_cast< std::uint64_t >( page_size );
#else
    struct sysinfo info{};
    sysinfo (&info);
    return ( info.freeram + info.bufferram ) * info.mem_unit;
#endif
}

#ifdef BIT7Z_AUTO_FORMAT
auto validateFormat( BitInFormat&& format ) -> const BitInFormat&  = delete;

auto validateFormat( const BitInFormat& format ) -> const BitInFormat& {
    if ( format == BitFormat::Auto ) {
        throw BitException{
            "Automatic format detection not supported in nested archives",
            BitError::UnsupportedOperation
        };
    }
    return format;
}

#define VALIDATE_FORMAT(x) validateFormat(x)
#else
#define VALIDATE_FORMAT(x) x
#endif

/* Detects reentrant calls into the same BitNestedArchiveReader instance (e.g., a user
 * FilterCallback/RenameCallback/ItemBufferCallback, or a FileCallback/ProgressCallback
 * registered on the object, calling back into it) while an operation is already in progress.
 * Reentering would make needReopen()/openSequentially() reinitialize the underlying sequential
 * stream while mNestedArchive's own Extract()/Open() call is still active further up the stack. */
class ReentrancyGuard {
    public:
        explicit ReentrancyGuard( bool& flag ) : mFlag{ flag } {
            if ( mFlag ) {
                throw BitException(
                    "BitNestedArchiveReader does not support reentrant calls on the same instance",
                    BitError::Fail
                );
            }
            mFlag = true;
        }

        ~ReentrancyGuard() {
            mFlag = false;
        }

        ReentrancyGuard( const ReentrancyGuard& ) = delete;
        auto operator=( const ReentrancyGuard& ) -> ReentrancyGuard& = delete;
        ReentrancyGuard( ReentrancyGuard&& ) = delete;
        auto operator=( ReentrancyGuard&& ) -> ReentrancyGuard& = delete;

    private:
        bool& mFlag;
};
} // namespace

BitNestedArchiveReader::BitNestedArchiveReader(
    const Bit7zLibrary& lib,
    const BitInputArchive& parentArchive,
    std::uint32_t index,
    const BitInFormat& format,
    const tstring& password
) : BitAbstractArchiveOpener{ lib, VALIDATE_FORMAT( format ), password },
    mNestedArchive{ *this, parentArchive.itemAt( index ) },
    mParentArchive{ parentArchive },
    mIndexInParent{ index },
    mMaxMemoryUsage{ std::max( getFreeRam() / 4, kMinMaxMemoryUsage ) },
    mCachedItemsCount{ kItemsCountUnset },
    mLastReadItem{ kNoItemRead },
    mOpenCount{ 0 },
    mOperationInProgress{ false } {}

BitNestedArchiveReader::BitNestedArchiveReader(
    const Bit7zLibrary& lib,
    const BitInputArchive& parentArchive,
    const BitInFormat& format,
    const tstring& password
) : BitNestedArchiveReader{ lib, parentArchive, 0, format, password } {}

auto BitNestedArchiveReader::maxMemoryUsage() const noexcept -> std::uint64_t {
    return mMaxMemoryUsage;
}

auto BitNestedArchiveReader::detectedFormat() const noexcept -> const BitInFormat& {
    return mNestedArchive.detectedFormat();
}

auto BitNestedArchiveReader::archiveProperty( BitProperty property ) const -> BitPropVariant {
    // Some archive-level properties (e.g., BitProperty::Characts for TAR) are populated
    // incrementally as items are parsed during extraction; reentering this call while another
    // operation is in progress could silently return an incomplete/stale value.
    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    return mNestedArchive.archiveProperty( property );
}

auto BitNestedArchiveReader::itemProperty( std::uint32_t index, BitProperty property ) const -> BitPropVariant {
    // kNoItemRead is reserved internally to mean "not positioned yet"; no real archive item can
    // ever legitimately sit at that index (see the Sentinels comment above), so reject it here
    // before it can be mistaken by needReopen() for "already past it, no reopen needed".
    if ( index == kNoItemRead ) {
        throw BitException(
            "Cannot retrieve the property of the item at the index " + std::to_string( index ),
            make_error_code( BitError::InvalidIndex )
        );
    }

    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    reopenIfNeeded( index );
    const auto result = mNestedArchive.itemProperty( index, property );
    mLastReadItem = index;
    return result;
}

auto BitNestedArchiveReader::itemsCount() const -> std::uint32_t {
    if ( mCachedItemsCount != kItemsCountUnset ) {
        return mCachedItemsCount;
    }

    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };

    // BitInputArchive::itemsCount() and calculateItemsCount() can both throw;
    // mCachedItemsCount must stay unset (max()) on failure so the next call retries
    // instead of caching a poisoned value.
    auto count = mNestedArchive.itemsCount();
    if ( count == std::numeric_limits< std::uint32_t >::max() ) {
        count = calculateItemsCount();
    }
    mCachedItemsCount = count;
    return mCachedItemsCount;
}

auto BitNestedArchiveReader::items() const -> std::vector< BitArchiveItemInfo > {
    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    reopenIfNeeded();

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
        result.reserve( static_cast< std::size_t >( itemsCount ) );
    }

    for ( const auto& item : mNestedArchive ) {
        if ( !item.hasProperty( BitProperty::IsDir ) ) {
            mLastReadItem = item.index();
            return result;
        }

        result.emplace_back( item );
    }
    return result;
}

void BitNestedArchiveReader::extractTo( const tstring& outDir ) const {
    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    reopenIfNeeded();
    mNestedArchive.extractTo( outDir );
}

void BitNestedArchiveReader::extractTo( const tstring& outDir, FilterCallback filterCallback ) const {
    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    reopenIfNeeded();
    mNestedArchive.extractTo( outDir, std::move( filterCallback ) );
}

void BitNestedArchiveReader::extractTo( const tstring& outDir, RenameCallback renameCallback ) const {
    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    reopenIfNeeded();
    mNestedArchive.extractTo( outDir, std::move( renameCallback ) );
}

void BitNestedArchiveReader::extractTo( std::map< tstring, buffer_t >& outMap ) const {
    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    reopenIfNeeded();
    mNestedArchive.extractTo( outMap );
}

void BitNestedArchiveReader::extractTo( ItemBufferCallback callback, FilterCallback filterCallback ) const {
    // Checked here, before touching the stream, so an empty callback fails fast instead of paying
    // for a reopen first. filterCallback isn't checked: unlike callback (the only source of output
    // buffers), an empty FilterCallback is a valid "extract every item, skip none" default.
    if ( !callback ) {
        throw BitException(
            "Cannot extract the archive using an empty callback",
            make_error_code( BitError::NullCallback )
        );
    }

    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    reopenIfNeeded();
    mNestedArchive.extractTo( std::move( callback ), std::move( filterCallback ) );
}

void BitNestedArchiveReader::test() const {
    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    reopenIfNeeded();
    mNestedArchive.test();
}

auto BitNestedArchiveReader::openCount() const -> std::size_t {
    return mOpenCount;
}

void BitNestedArchiveReader::setMaxMemoryUsage( std::uint64_t value ) noexcept {
    // TODO: Throw an exception if the value is below the minimum?
    mMaxMemoryUsage = std::max( value, kMinMaxMemoryUsage );
}

void BitNestedArchiveReader::openSequentially() const {
    const auto stream = bit7z::make_com< CSynchronizedInStream, ISequentialInStream >(
        mMaxMemoryUsage,
        mParentArchive,
        mIndexInParent
    );
    mNestedArchive.openArchiveSeqStream( stream );
    mLastReadItem = 0;
    ++mOpenCount;
}

void BitNestedArchiveReader::reopenIfNeeded( std::uint32_t index ) const {
    if ( needReopen( index ) ) {
        openSequentially();
    }

    // Poisoning mLastReadItem before anything after this function call can throw, so a failure forces
    // the next operation to reopen instead of reusing a possibly-advanced stream.
    mLastReadItem = kNoItemRead;
}

auto BitNestedArchiveReader::needReopen( std::uint32_t index ) const noexcept -> bool {
    return index < mLastReadItem;
}

auto BitNestedArchiveReader::calculateItemsCount() const -> std::uint32_t {
    reopenIfNeeded();

    for ( std::uint32_t index = 0; index < std::numeric_limits< std::uint32_t >::max(); ++index ) {
        /* All archive formats provide BitProperty::IsDir for _valid_ items,
         * so if the item at the index doesn't have this property,
         * it means the archive doesn't have an item at the given index. */
        if ( !mNestedArchive.itemHasProperty( index, BitProperty::IsDir ) ) {
            mLastReadItem = index;
            return index;
        }
    }
    throw BitException(
        "Could not determine the number of items in the nested archive",
        BitError::Fail
    );
}

} // namespace bit7z

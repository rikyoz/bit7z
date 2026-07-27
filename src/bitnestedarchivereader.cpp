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
#include "internal/exceptionutil.hpp"
#include <internal/util.hpp>

#ifdef _WIN32
#   include <windows.h>
#elif defined( __APPLE__ )
#   include <TargetConditionals.h>
#   if defined( TARGET_OS_OSX ) && TARGET_OS_OSX
#       include <mach/mach.h>
#       include <mach/mach_host.h>
#   else
#       include <os/proc.h>
#   endif
#elif defined( __FreeBSD__ )
#   include <sys/types.h>
#   include <sys/sysctl.h>
#elif defined( __OpenBSD__ )
#   include <sys/types.h>
#   include <sys/sysctl.h>
#   include <uvm/uvmexp.h>
#elif defined( __NetBSD__ )
#   include <sys/types.h>
#   include <sys/sysctl.h>
#   include <uvm/uvm_extern.h>
#   include <cstddef>
#elif defined( BSD ) ||\
        defined( __DragonFly__ )
#   include <sys/types.h>
#   include <sys/sysctl.h>
#else
#   include <unistd.h>
#   if !defined( _SC_AVPHYS_PAGES ) || !defined( _SC_PAGE_SIZE )
#       include <sys/sysinfo.h>
#   endif
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
#elif defined( __APPLE__ )
#   if defined( TARGET_OS_OSX ) && TARGET_OS_OSX
    mach_port_t host = mach_host_self();
    vm_size_t pageSize = 0;
    vm_statistics64_data_t vmStats{};
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

    auto res = host_page_size( host, &pageSize );
    if ( res != KERN_SUCCESS ) {
        return 0;
    }

    res = host_statistics64(
        host,
        HOST_VM_INFO64,
        reinterpret_cast< host_info64_t >( &vmStats ),
        &count
    );
    if ( res != KERN_SUCCESS ) {
        return 0;
    }

    // free_count: immediately usable pages; inactive_count: clean, cheaply reclaimable.
    return ( static_cast< std::uint64_t >( vmStats.free_count ) + vmStats.inactive_count ) * pageSize;
#   else
    // iOS/tvOS/watchOS have no system-wide "free RAM" concept exposed to apps; instead,
    // os_proc_available_memory() reports how much more *this process* can allocate before
    // hitting its jetsam memory limit, which is the right analog for this platform family.
    return os_proc_available_memory();
#   endif
#elif defined( __FreeBSD__ )
    unsigned int freePages = 0;
    std::size_t size = sizeof( freePages );
    if (
        sysctlbyname( "vm.stats.vm.v_free_count", &freePages, &size, nullptr, 0 ) != 0 ||
        size != sizeof( freePages )
    ) {
        return 0;
    }

    unsigned int inactivePages = 0;
    size = sizeof( inactivePages );
    if (
        sysctlbyname( "vm.stats.vm.v_inactive_count", &inactivePages, &size, nullptr, 0 ) != 0 ||
        size != sizeof( inactivePages )
    ) {
        return 0;
    }

    unsigned int pageSize = 0;
    size = sizeof( pageSize );
    if (
        sysctlbyname( "vm.stats.vm.v_page_size", &pageSize, &size, nullptr, 0 ) != 0 ||
        size != sizeof( pageSize )
    ) {
        return 0;
    }

    return ( static_cast< std::uint64_t >( freePages ) + inactivePages ) * pageSize;
#elif defined( __OpenBSD__ )
    static int mib[] = { CTL_VM, VM_UVMEXP };
    struct uvmexp usage{};
    std::size_t length = sizeof( usage );

    if (
        sysctl( mib, 2, &usage, &length, nullptr, 0 ) != 0 ||
        length != sizeof( usage ) ||
        usage.free < 0 ||
        usage.inactive < 0 ||
        usage.pagesize < 0
    ) {
        return 0;
    }
    return ( static_cast< std::uint64_t >( usage.free ) + static_cast< std::uint64_t >( usage.inactive ) ) *
            static_cast< std::uint64_t >( usage.pagesize );
#elif defined( __NetBSD__ )
    static int mib[] = { CTL_VM, VM_UVMEXP2 };
    struct uvmexp_sysctl usage{};
    std::size_t length = sizeof( usage );

    // Unlike the other BSD branches, VM_UVMEXP2 is deliberately kernel-version independent:
    // NetBSD's handler copies min(our buffer size, its own struct size), so a future NetBSD
    // release appending trailing fields must not fail this check. It's enough that the fields
    // we read below - pagesize, free, inactive, all within the struct's leading fields - were
    // actually filled, rather than requiring the two sizes to match exactly.
    const auto kRequiredLength = offsetof( struct uvmexp_sysctl, inactive ) + sizeof( usage.inactive );
    if (
        sysctl( mib, 2, &usage, &length, nullptr, 0 ) != 0 ||
        length < kRequiredLength ||
        usage.free < 0 ||
        usage.inactive < 0 ||
        usage.pagesize < 0
    ) {
        return 0;
    }
    return ( static_cast< std::uint64_t >( usage.free ) + static_cast< std::uint64_t >( usage.inactive ) ) *
            static_cast< std::uint64_t >( usage.pagesize );
#elif defined( BSD ) ||\
        defined( __DragonFly__ )
    static int mib[] = { CTL_HW, HW_USERMEM };
    std::uint64_t value = 0;
    std::size_t length = sizeof( value );

    // No portable "currently free" sysctl exists across these BSDs under CTL_HW;
    // HW_USERMEM approximates non-kernel RAM (close to total, not actual free memory).
    if ( sysctl( mib, 2, &value, &length, nullptr, 0 ) != 0 || length != sizeof( value ) ) {
        return 0;
    }
    return value;
#elif defined( _SC_AVPHYS_PAGES ) && defined( _SC_PAGE_SIZE )
    const long pages = sysconf( _SC_AVPHYS_PAGES );
    const long page_size = sysconf( _SC_PAGE_SIZE );
    if ( pages < 0 || page_size < 0 ) {
        return 0;
    }
    return static_cast< std::uint64_t >( pages ) * static_cast< std::uint64_t >( page_size );
#else
    struct sysinfo info{};
    if ( sysinfo( &info ) != 0 ) {
        return 0;
    }
    return ( static_cast< std::uint64_t >( info.freeram ) + info.bufferram ) * info.mem_unit;
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

    return withReentrancyGuard( [ this, index, property ]() -> BitPropVariant {
        const auto result = mNestedArchive.itemProperty( index, property );
        mLastReadItem = index;
        return result;
    }, index );
}

auto BitNestedArchiveReader::itemsCount() const -> std::uint32_t {
    if ( mCachedItemsCount != kItemsCountUnset ) {
        return mCachedItemsCount;
    }

    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };

    // Both calls below can throw; mCachedItemsCount must stay unset on failure so the next
    // call retries instead of caching a poisoned value.
    auto count = mNestedArchive.itemsCount();
    if ( count == 0 || count == kItemsCountUnset ) {
        // Ambiguous: some handlers (e.g. SWF) report 0 before Open() populates their real
        // state, indistinguishable from a genuinely empty archive. Formats that report a
        // reliable count unopened (e.g. single-stream formats, always 1) skip this entirely.
        reopenIfNeeded();
        if ( count == 0 ) {
            // Re-checking only helps formats like SWF, which report the real count once
            // opened; TAR stays kItemsCountUnset either way, so skip to the manual walk.
            count = mNestedArchive.itemsCount();
        }
        if ( count == kItemsCountUnset ) {
            count = calculateItemsCount();
        }
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
    if ( itemsCount < kItemsCountUnset ) {
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
    withReentrancyGuard( [ this, &outDir ]() -> void { mNestedArchive.extractTo( outDir ); } );
}

void BitNestedArchiveReader::extractTo( const tstring& outDir, FilterCallback filterCallback ) const {
    withReentrancyGuard( [ this, &outDir, &filterCallback ]() -> void {
        mNestedArchive.extractTo( outDir, std::move( filterCallback ) );
    } );
}

void BitNestedArchiveReader::extractTo( const tstring& outDir, RenameCallback renameCallback ) const {
    withReentrancyGuard( [ this, &outDir, &renameCallback ]() -> void {
        mNestedArchive.extractTo( outDir, std::move( renameCallback ) );
    } );
}

void BitNestedArchiveReader::extractTo( std::map< tstring, buffer_t >& outMap ) const {
    withReentrancyGuard( [ this, &outMap ]() -> void { mNestedArchive.extractTo( outMap ); } );
}

void BitNestedArchiveReader::extractTo( ItemBufferCallback callback, FilterCallback filterCallback ) const {
    // Checked here, before touching the stream, so an empty callback fails fast instead of paying
    // for a reopen first. filterCallback isn't checked: unlike callback (the only source of output
    // buffers), an empty FilterCallback is a valid "extract every item, skip none" default.
    requireCallbackForExtraction( callback );

    withReentrancyGuard( [ this, &callback, &filterCallback ]() -> void {
        mNestedArchive.extractTo( std::move( callback ), std::move( filterCallback ) );
    } );
}

void BitNestedArchiveReader::test() const {
    withReentrancyGuard( [ this ]() -> void { mNestedArchive.test(); } );
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

template< typename Operation >
auto BitNestedArchiveReader::withReentrancyGuard(
    Operation&& operation,
    std::uint32_t index
) const -> decltype( operation() ) {
    const ReentrancyGuard reentrancyGuard{ mOperationInProgress };
    reopenIfNeeded( index );
    return std::forward< Operation >( operation )();
}

auto BitNestedArchiveReader::needReopen( std::uint32_t index ) const noexcept -> bool {
    return index < mLastReadItem;
}

auto BitNestedArchiveReader::calculateItemsCount() const -> std::uint32_t {
    // No reopenIfNeeded() here: itemsCount() (this function's only caller) already reopened the
    // archive; doing it again would discard an unread stream and bump openCount() for nothing.

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

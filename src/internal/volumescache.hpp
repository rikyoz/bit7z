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

#ifndef _WIN32
#include <climits> // For _POSIX_OPEN_MAX
#include <unistd.h>
#endif

namespace bit7z {

constexpr auto kNoVolume = std::numeric_limits< std::size_t >::max();

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

template< typename T >
using VolumesCache = std::vector< CachedVolume< T > >;

#ifndef _WIN32
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
#endif

} // namespace bit7z

#endif //VOLUMESCACHE_HPP

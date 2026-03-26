/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CMULTIVOLUMEINSTREAM_HPP
#define CMULTIVOLUMEINSTREAM_HPP

#include "internal/com.hpp"
#include "internal/cfileinstream.hpp"
#include "internal/guiddef.hpp"
#include "internal/macros.hpp"

#include <7zip/IStream.h>

#include <cstdint>
#include <limits>
#include <vector>

namespace bit7z {

using VolumeIndex = std::size_t;

constexpr auto kNoVolume = std::numeric_limits< VolumeIndex >::max();

struct CachedVolume final {
    fs::path volumePath;
    std::uint64_t volumeSize;
    std::uint64_t globalOffset;
    std::uint64_t seekPosition;
    CMyComPtr< CFileInStream > stream;
#ifndef _WIN32
    VolumeIndex newerVolume = kNoVolume;
    VolumeIndex olderVolume = kNoVolume;
#endif
};

using VolumesCache = std::vector< CachedVolume >;

class CMultiVolumeInStream final : public IInStream, public CMyUnknownImp {
        std::uint64_t mCurrentPosition;
        std::uint64_t mTotalSize;
        VolumesCache mVolumesCache;
#ifndef _WIN32
        std::size_t mOpenCount = 0;
        VolumeIndex mNewestVolume = kNoVolume;
        VolumeIndex mOldestVolume = kNoVolume;
#endif
        VolumeIndex mLastOpenedVolume = kNoVolume;

        auto currentVolume() -> CachedVolume&;

        void ensureVolumeOpen( CachedVolume& cachedVolume, VolumeIndex midpoint );

        void addVolume( const fs::path& volumePath );

    public:
        explicit CMultiVolumeInStream( const fs::path& firstVolume );

        CMultiVolumeInStream( const CMultiVolumeInStream& ) = delete;

        CMultiVolumeInStream( CMultiVolumeInStream&& ) = delete;

        auto operator=( const CMultiVolumeInStream& ) -> CMultiVolumeInStream& = delete;

        auto operator=( CMultiVolumeInStream&& ) -> CMultiVolumeInStream& = delete;

        MY_UNKNOWN_DESTRUCTOR( ~CMultiVolumeInStream() ) = default;

        // IInStream
        BIT7Z_STDMETHOD( Read, void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        // NOLINTNEXTLINE(modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( IInStream ) //-V2507 //-V2511 //-V835 //-V3504
};

} // namespace bit7z

#endif //CMULTIVOLUMEINSTREAM_HPP

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CSYNCHRONIZEDINSTREAM_HPP
#define CSYNCHRONIZEDINSTREAM_HPP

#include "bitinputarchive.hpp"
#include "internal/com.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"
#include "internal/bufferqueue.hpp"

#include <7zip/IStream.h>

#include <mutex>
#include <queue>
#include <thread>

namespace bit7z {

class CSynchronizedInStream final : public ISequentialInStream, public CMyUnknownImp {
    public:
        explicit CSynchronizedInStream( std::uint64_t maxMemoryUsage,
                                        const BitInputArchive& parentArchive,
                                        std::uint32_t index );

        explicit CSynchronizedInStream( const CSynchronizedInStream& ) = delete;

        CSynchronizedInStream( CSynchronizedInStream&& ) = delete;

        auto operator=( const CSynchronizedInStream& ) -> CSynchronizedInStream& = delete;

        auto operator=( CSynchronizedInStream&& ) -> CSynchronizedInStream& = delete;

        MY_UNKNOWN_DESTRUCTOR( ~CSynchronizedInStream() );

        // ISequentialInStream
        BIT7Z_STDMETHOD( Read, void* data, UInt32 size, UInt32* processedSize );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( ISequentialInStream ); //-V2507 //-V2511 //-V835 //-V3504

    private:
        void extractParentArchive();

        BufferQueue mBufferQueue;
        const BitInputArchive& mParentArchive;
        uint32_t mIndex;

        std::thread mExtractorThread;
        std::atomic_bool mExtractionStarted;
        std::atomic_bool mExtractionFinished;

        buffer_t mReadBuffer;
        buffer_t::iterator mCurrentReadPosition;
};

} // namespace bit7z

#endif //CSYNCHRONIZEDINSTREAM_HPP

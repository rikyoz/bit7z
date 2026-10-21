/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BUFFERQUEUE_HPP
#define BUFFERQUEUE_HPP

#include "bittypes.hpp"

#include <atomic>
#include <cstddef>
#include <condition_variable>
#include <queue>
#include <mutex>

namespace bit7z {

class BufferQueue final {
    public:
        explicit BufferQueue( std::uint64_t maxMemoryUsage );

        BufferQueue( const BufferQueue& ) = delete;

        BufferQueue( BufferQueue&& other ) = delete;

        auto operator=( const BufferQueue& ) -> BufferQueue& = delete;

        auto operator=( BufferQueue&& ) -> BufferQueue& = delete;

        ~BufferQueue() = default;

        auto pop() -> buffer_t;

        void push( buffer_t&& item );

        void notifyFinished();

        void reset();

        auto empty() const -> bool;

    private:
        std::queue< buffer_t > mQueue;
        mutable std::mutex mMutex;
        std::condition_variable mEmptyCondition;
        std::condition_variable mFullCondition;
        std::atomic_bool mFinished{ false };
        // Size of the queued in-memory buffers, so it always fits in a size_t,
        // which keeps the atomic lock-free on 32-bit targets as well.
        std::atomic< std::size_t > mMemoryUsage;
        std::uint64_t mMaxMemoryUsage;
};

} // namespace bit7z

#endif //BUFFERQUEUE_HPP

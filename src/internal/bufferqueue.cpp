// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bufferqueue.hpp"

#include <algorithm>

namespace bit7z {

BufferQueue::BufferQueue( std::uint64_t maxMemoryUsage ) : mMemoryUsage{ 0 }, mMaxMemoryUsage{ maxMemoryUsage } {}

auto BufferQueue::pop() -> buffer_t {
    std::unique_lock< std::mutex > lock( mMutex );
    mEmptyCondition.wait( lock, [ this ] () -> bool { return !mQueue.empty() || mFinished; } );
    if ( mQueue.empty() && mFinished ) {
        return {};
    }
    buffer_t value{ std::move( mQueue.front() ) };
    mQueue.pop();
    mMemoryUsage -= value.size();
    mFullCondition.notify_one();
    return value;
}

void BufferQueue::push( buffer_t&& item ) {
    std::unique_lock< std::mutex > lock( mMutex );
    mFullCondition.wait( lock, [ this, &item ] () -> bool { return mMemoryUsage + item.size() < mMaxMemoryUsage; } );
    mMemoryUsage += item.size();
    mQueue.push( std::move( item ) );
    mEmptyCondition.notify_one();
}

void BufferQueue::notifyFinished() {
    std::lock_guard< std::mutex > lock( mMutex );
    mFinished = true;
    mEmptyCondition.notify_one();
}

void BufferQueue::reset() {
    mFinished = false;
}

auto BufferQueue::empty() const -> bool {
    std::lock_guard< std::mutex > lock( mMutex );
    return mQueue.empty();
}

} // namespace bit7z
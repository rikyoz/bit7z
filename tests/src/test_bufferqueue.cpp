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

#include <catch2/catch.hpp>

#include <bit7z/bittypes.hpp>
#include <internal/bufferqueue.hpp>

#include <chrono>
#include <future>
#include <memory>
#include <thread>

using bit7z::buffer_t;
using bit7z::byte_t;
using bit7z::BufferQueue;

namespace {

// Generous but bounded: long enough that a passing test never gets close to it,
// short enough that a deadlock regression fails the test instead of hanging CI.
constexpr auto kWaitTimeout = std::chrono::seconds( 5 );

} // namespace

TEST_CASE( "BufferQueue: a newly constructed queue is empty", "[bufferqueue]" ) {
    const BufferQueue queue{ 1024 };
    REQUIRE( queue.empty() );
}

TEST_CASE( "BufferQueue: push and pop preserve the pushed data and FIFO order", "[bufferqueue]" ) {
    BufferQueue queue{ 1024 };

    const buffer_t first{ static_cast< byte_t >( 1 ), static_cast< byte_t >( 2 ), static_cast< byte_t >( 3 ) };
    const buffer_t second{ static_cast< byte_t >( 4 ), static_cast< byte_t >( 5 ) };

    queue.push( buffer_t{ first } );
    REQUIRE_FALSE( queue.empty() );

    queue.push( buffer_t{ second } );

    REQUIRE( queue.pop() == first );
    REQUIRE_FALSE( queue.empty() );

    REQUIRE( queue.pop() == second );
    REQUIRE( queue.empty() );
}

TEST_CASE( "BufferQueue: pop returns an empty buffer once finished with nothing queued", "[bufferqueue]" ) {
    BufferQueue queue{ 1024 };
    queue.notifyFinished();

    const auto popped = queue.pop();
    REQUIRE( popped.empty() );
}

TEST_CASE( "BufferQueue: pop returns an empty buffer once finished after all items are drained", "[bufferqueue]" ) {
    BufferQueue queue{ 1024 };

    const buffer_t data{ static_cast< byte_t >( 1 ), static_cast< byte_t >( 2 ), static_cast< byte_t >( 3 ) };
    queue.push( buffer_t{ data } );
    REQUIRE( queue.pop() == data );

    queue.notifyFinished();

    const auto popped = queue.pop();
    REQUIRE( popped.empty() );
}

TEST_CASE( "BufferQueue: notifyFinished wakes a pop that is already waiting", "[bufferqueue][threading]" ) {
    // Heap-allocated, not a stack variable: if the wake-up regresses, the block below leaks
    // queue instead of destroying it while the detached thread may still be using it.
    auto queue = std::make_unique< BufferQueue >( 1024 ); // NOLINT(*-magic-numbers)

    auto poppedPromise = std::make_shared< std::promise< buffer_t > >();
    auto poppedFuture = poppedPromise->get_future();

    std::thread producer( [ rawQueue = queue.get(), poppedPromise ]() -> void { // NOSONAR
        poppedPromise->set_value( rawQueue->pop() );
    } );
    // A still-joinable std::thread calls std::terminate() on destruction, so if pop() deadlocks
    // we must detach rather than join; completion is instead observed through poppedFuture.
    // poppedPromise is heap-allocated and kept alive by the thread's own shared_ptr copy, so a
    // REQUIRE failure below that unwinds this stack frame early can't leave the thread writing
    // into an already-destroyed promise.
    producer.detach(); // NOSONAR

    // Give the pop() call a chance to actually start waiting before we signal it.
    std::this_thread::sleep_for( std::chrono::milliseconds{ 100 } ); // NOLINT(*-magic-numbers)

    queue->notifyFinished();

    const auto status = poppedFuture.wait_for( kWaitTimeout );
    if ( status != std::future_status::ready ) {
        // The wake-up regressed: the detached producer above may still be blocked inside
        // pop(), touching *queue through the raw pointer it captured. Releasing instead of
        // letting the unique_ptr destroy queue deliberately leaks it rather than risking the
        // undefined behavior of destroying an object a running thread still references.
        queue.release(); // NOLINT(*-unused-return-value)
    }
    REQUIRE( status == std::future_status::ready );
    REQUIRE( poppedFuture.get().empty() ); // NOLINT(*-cplusplus.Move)
}

TEST_CASE(
    "BufferQueue: push waits while the queue already holds data and is over capacity",
    "[bufferqueue][threading]"
) {
    // See the rationale in the "notifyFinished wakes a pop" test above for why this is
    // heap-allocated rather than a stack variable.
    auto queue = std::make_unique< BufferQueue >( 10 ); // NOLINT(*-magic-numbers)
    // 9 of 10 bytes used; queue is non-empty.
    queue->push( buffer_t( 9, static_cast< byte_t >( 0 ) ) ); // NOLINT(*-magic-numbers)

    auto pushedPromise = std::make_shared< std::promise< void > >();
    const auto pushedFuture = pushedPromise->get_future();

    std::thread producer( [ rawQueue = queue.get(), pushedPromise ]() -> void { // NOSONAR
        // Would bring usage to 18 > 10: must wait.
        rawQueue->push( buffer_t( 9, static_cast< byte_t >( 0 ) ) ); // NOLINT(*-magic-numbers)
        pushedPromise->set_value();
    } );
    // See the rationale in the "notifyFinished wakes a pop" test above: detaching avoids
    // std::terminate() on a still-joinable thread if push() deadlocks, and pushedPromise is
    // heap-allocated and kept alive by the thread's own shared_ptr copy, so it safely outlives
    // this stack frame even if a REQUIRE below throws and unwinds early.
    producer.detach(); // NOSONAR

    // Not enough room, and the queue isn't empty: the second push must still be blocked.
    REQUIRE( pushedFuture.wait_for( std::chrono::milliseconds( 300 ) ) == std::future_status::timeout );

    queue->pop(); // Frees the 9 bytes, unblocking the pending push.

    const auto status = pushedFuture.wait_for( kWaitTimeout );
    if ( status != std::future_status::ready ) {
        // The wake-up regressed: the detached producer above may still be blocked inside
        // push(), touching *queue through the raw pointer it captured. Releasing instead of
        // letting the unique_ptr destroy queue deliberately leaks it rather than risking the
        // undefined behavior of destroying an object a running thread still references.
        queue.release(); // NOLINT(*-unused-return-value)
    }
    REQUIRE( status == std::future_status::ready );
    REQUIRE_FALSE( queue->empty() );
}

// Regression test for the deadlock previously possible in BufferQueue::push(): if a single pushed
// item is, by itself, as large as (or larger than) mMaxMemoryUsage, the old wait condition
// `mMemoryUsage + item.size() < mMaxMemoryUsage` could never become true on an empty queue, since
// there is nothing left for pop() to remove to free up space. push() must let such an oversized
// item through when the queue is empty, instead of waiting forever.
TEST_CASE(
    "BufferQueue: pushing a single oversized item on an empty queue does not deadlock",
    "[bufferqueue][regression][threading]"
) {
    // See the rationale in the "notifyFinished wakes a pop" test above for why this is
    // heap-allocated rather than a stack variable.
    auto queue = std::make_unique< BufferQueue >( 16 );
    // Far larger than the 16-byte capacity.
    buffer_t oversizedItem( 100, static_cast< byte_t >( 0 ) ); // NOLINT(*-magic-numbers)

    auto pushCompleted = std::make_shared< std::promise< void > >();
    const auto pushCompletedFuture = pushCompleted->get_future();

    std::thread producer(
        [ rawQueue = queue.get(), item = std::move( oversizedItem ), pushCompleted ]() mutable -> void {
            rawQueue->push( std::move( item ) );
            pushCompleted->set_value();
        }
    );
    // Giving producer a wider scope instead of detaching would not help: if push() deadlocks,
    // it still can't be joined, and a still-joinable std::thread calls std::terminate() on
    // destruction, turning a clean, isolated test failure into a hard process abort. Completion
    // is already observable through pushCompleted/pushCompletedFuture, not the thread handle.
    // pushCompleted is heap-allocated and kept alive by the thread's own shared_ptr copy, so a
    // REQUIRE failure below that unwinds this stack frame early can't leave the thread writing
    // into an already-destroyed promise.
    producer.detach(); // NOSONAR

    const auto status = pushCompletedFuture.wait_for( kWaitTimeout );
    if ( status != std::future_status::ready ) {
        // The deadlock regressed: the detached producer above may still be blocked inside
        // push(), touching *queue through the raw pointer it captured. Releasing instead of
        // letting the unique_ptr destroy queue deliberately leaks it rather than risking the
        // undefined behavior of destroying an object a running thread still references.
        queue.release(); // NOLINT(*-unused-return-value)
    }
    REQUIRE( status == std::future_status::ready );

    const auto popped = queue->pop();
    REQUIRE( popped.size() == 100 );
    REQUIRE( queue->empty() );
}
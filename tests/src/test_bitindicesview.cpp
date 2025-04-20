// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include <bit7z/bitinputarchive.hpp>

#include <algorithm>
#include <numeric>

using bit7z::BitIndicesView;

TEST_CASE( "Creating an empty view", "[bitindicesview]" ) {
    constexpr BitIndicesView view{};

    REQUIRE( view.data() == nullptr );
    REQUIRE( view.size() == 0 ); // NOLINT(*-container-size-empty)
    REQUIRE( view.empty() );

    REQUIRE( view.begin() == view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() >= view.end() );
    REQUIRE( view.cbegin() == view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.begin() != view.end() );
    REQUIRE_FALSE( view.begin() < view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() != view.cend() );
    REQUIRE_FALSE( view.cbegin() < view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );

    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + 0 ) == view.end() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == 0 );
    REQUIRE( ( view.cend() - view.cbegin() ) == 0 );
}

TEMPLATE_TEST_CASE( "Creating a view of a single integer", "[bitindicesview]", const std::uint32_t, std::uint32_t ) {
    TestType index = GENERATE(0, 1, 42, 1024);
    CAPTURE( index );

    // Note: Using () instead of {} avoids calling the initializer_list constructor,
    // which would cause lifetime issues (the view would store a dangling pointer
    // to the initializer_list's temporary array, which is destroyed after the constructor exits).
    const BitIndicesView view( index );

    REQUIRE( view.data() == &index );
    REQUIRE( view.size() == 1 );
    REQUIRE_FALSE( view.empty() );

    REQUIRE_FALSE( view.begin() == view.end() );
    REQUIRE_FALSE( view.begin() >= view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() == view.cend() );
    REQUIRE_FALSE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );
    REQUIRE( view.begin() != view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() < view.end() );
    REQUIRE( view.cbegin() != view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() < view.cend() );

    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + 1 ) == view.end() );
    REQUIRE( ( view.cbegin() + 1 ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == 1 );
    REQUIRE( ( view.cend() - view.cbegin() ) == 1 );

    REQUIRE( *view.data() == index );
    REQUIRE( *view.begin() == index );
    REQUIRE( view.begin()[0] == index );
}

TEST_CASE( "Creating a view of an empty std::vector", "[bitindicesview]" ) {
    const std::vector< std::uint32_t > testIndices{};
    const BitIndicesView view{ testIndices };

    REQUIRE( view.data() == nullptr );
    REQUIRE( view.size() == 0 ); // NOLINT(*-container-size-empty)
    REQUIRE( view.empty() );

    REQUIRE( view.begin() == view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() >= view.end() );
    REQUIRE( view.cbegin() == view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.begin() != view.end() );
    REQUIRE_FALSE( view.begin() < view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() != view.cend() );
    REQUIRE_FALSE( view.cbegin() < view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );

    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + 0 ) == view.end() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == 0 );
    REQUIRE( ( view.cend() - view.cbegin() ) == 0 );
}

TEST_CASE( "Creating a view of a single-element std::vector", "[bitindicesview]" ) {
    const std::vector< std::uint32_t > testIndices = { 42 };
    const BitIndicesView view{ testIndices };

    REQUIRE_FALSE( view.data() == nullptr );
    REQUIRE( view.data() == testIndices.data() );
    REQUIRE( view.size() == 1 );
    REQUIRE_FALSE( view.empty() );

    REQUIRE_FALSE( view.begin() == view.end() );
    REQUIRE_FALSE( view.begin() >= view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() == view.cend() );
    REQUIRE_FALSE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );
    REQUIRE( view.begin() != view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() < view.end() );
    REQUIRE( view.cbegin() != view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() < view.cend() );

    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + 1 ) == view.end() );
    REQUIRE( ( view.cbegin() + 1 ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == 1 );
    REQUIRE( ( view.cend() - view.cbegin() ) == 1 );

    REQUIRE( *view.data() == testIndices[0] );
    REQUIRE( *view.begin() == testIndices[0] );
    REQUIRE( view.begin()[0] == testIndices[0] );
}

TEST_CASE( "Creating a view of a non-empty std::vector", "[bitindicesview]" ) {
    using namespace Catch::Generators;

    std::vector< std::uint32_t > testIndices( 16 );
    std::iota( testIndices.begin(), testIndices.end(), 0u );
    std::shuffle( testIndices.begin(), testIndices.end(), std::mt19937{ std::random_device{}() } );
    CAPTURE( testIndices );

    const BitIndicesView view{ testIndices };

    REQUIRE_FALSE( view.data() == nullptr );
    REQUIRE( view.data() == testIndices.data() );
    REQUIRE( view.size() == testIndices.size() );
    REQUIRE_FALSE( view.empty() );

    REQUIRE_FALSE( view.begin() == view.end() );
    REQUIRE_FALSE( view.begin() >= view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() == view.cend() );
    REQUIRE_FALSE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );
    REQUIRE( view.begin() != view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() < view.end() );
    REQUIRE( view.cbegin() != view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() < view.cend() );

    const auto signedSize = static_cast< std::ptrdiff_t >( testIndices.size() );
    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + signedSize ) == view.end() );
    REQUIRE( ( view.cbegin() + signedSize ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == signedSize );
    REQUIRE( ( view.cend() - view.cbegin() ) == signedSize );

    REQUIRE( std::equal( view.cbegin(), view.cend(), testIndices.cbegin() ) );
}

TEMPLATE_TEST_CASE( "Creating a view of an empty std::array", "[bitindicesview]", const std::uint32_t, std::uint32_t ) {
    constexpr std::array< TestType, 0 > array{};
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const BitIndicesView view{ array };

    REQUIRE( view.data() == nullptr );
    REQUIRE( view.size() == 0 ); // NOLINT(*-container-size-empty)
    REQUIRE( view.empty() );

    REQUIRE( view.begin() == view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() >= view.end() );
    REQUIRE( view.cbegin() == view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.begin() != view.end() );
    REQUIRE_FALSE( view.begin() < view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() != view.cend() );
    REQUIRE_FALSE( view.cbegin() < view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );

    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + 0 ) == view.end() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == 0 );
    REQUIRE( ( view.cend() - view.cbegin() ) == 0 );
}

TEMPLATE_TEST_CASE( "Creating a view of a non-empty std::array", "[bitindicesview]", const std::uint32_t, std::uint32_t ) {
    constexpr std::array< TestType, 16 > testIndices = {1, 3, 5, 7, 9, 11, 13, 15, 17};
    const BitIndicesView view{ testIndices };

    REQUIRE_FALSE( view.data() == nullptr );
    REQUIRE( view.data() == testIndices.data() );
    REQUIRE( view.size() == testIndices.size() );
    REQUIRE_FALSE( view.empty() );

    REQUIRE_FALSE( view.begin() == view.end() );
    REQUIRE_FALSE( view.begin() >= view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() == view.cend() );
    REQUIRE_FALSE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );
    REQUIRE( view.begin() != view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() < view.end() );
    REQUIRE( view.cbegin() != view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() < view.cend() );

    constexpr auto signedSize = static_cast< std::ptrdiff_t >( testIndices.size() );
    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + signedSize ) == view.end() );
    REQUIRE( ( view.cbegin() + signedSize ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == signedSize );
    REQUIRE( ( view.cend() - view.cbegin() ) == signedSize );

    REQUIRE( std::equal( view.cbegin(), view.cend(), testIndices.cbegin() ) );
}

TEMPLATE_TEST_CASE( "Creating a view of a C array", "[bitindicesview]", const std::uint32_t, std::uint32_t ) {
    constexpr std::size_t arraySize = 16u;
    TestType testIndices[arraySize] = {2, 4, 6, 8, 10, 12, 14, 16}; // NOLINT(*-avoid-c-arrays)
    const BitIndicesView view{ testIndices };

    REQUIRE_FALSE( view.data() == nullptr );
    REQUIRE( view.data() == testIndices ); // NOLINT(*-pro-bounds-array-to-pointer-decay, *-no-array-decay)
    REQUIRE( view.size() == arraySize );
    REQUIRE_FALSE( view.empty() );

    REQUIRE_FALSE( view.begin() == view.end() );
    REQUIRE_FALSE( view.begin() >= view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() == view.cend() );
    REQUIRE_FALSE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );
    REQUIRE( view.begin() != view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() < view.end() );
    REQUIRE( view.cbegin() != view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() < view.cend() );

    constexpr auto signedSize = static_cast< std::ptrdiff_t >( arraySize );
    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + signedSize ) == view.end() );
    REQUIRE( ( view.cbegin() + signedSize ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == signedSize );
    REQUIRE( ( view.cend() - view.cbegin() ) == signedSize );

    REQUIRE( std::equal( view.cbegin(), view.cend(), std::begin( testIndices ) ) );
}

TEST_CASE( "Creating a view of an empty initializer_list", "[bitindicesview]" ) {
    const std::initializer_list< std::uint32_t > testIndices{};
    const BitIndicesView view{ testIndices };

    REQUIRE( view.data() == nullptr );
    REQUIRE( view.size() == 0 ); // NOLINT(*-container-size-empty)
    REQUIRE( view.empty() );

    REQUIRE( view.begin() == view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() >= view.end() );
    REQUIRE( view.cbegin() == view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.begin() != view.end() );
    REQUIRE_FALSE( view.begin() < view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() != view.cend() );
    REQUIRE_FALSE( view.cbegin() < view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );

    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + 0 ) == view.end() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == 0 );
    REQUIRE( ( view.cend() - view.cbegin() ) == 0 );
}

TEST_CASE( "Creating a view of a non-empty initializer_list", "[bitindicesview]" ) {
    const std::initializer_list< std::uint32_t > testIndices = {0, 1, 1, 2, 3, 5, 8, 13, 21};

    const BitIndicesView view{ testIndices };

    REQUIRE_FALSE( view.data() == nullptr );
    REQUIRE( view.data() == testIndices.begin() );
    REQUIRE( view.size() == testIndices.size() );
    REQUIRE_FALSE( view.empty() );

    REQUIRE_FALSE( view.begin() == view.end() );
    REQUIRE_FALSE( view.begin() >= view.end() );
    REQUIRE_FALSE( view.begin() > view.end() );
    REQUIRE_FALSE( view.cbegin() == view.cend() );
    REQUIRE_FALSE( view.cbegin() >= view.cend() );
    REQUIRE_FALSE( view.cbegin() > view.cend() );
    REQUIRE( view.begin() != view.end() );
    REQUIRE( view.begin() <= view.end() );
    REQUIRE( view.begin() < view.end() );
    REQUIRE( view.cbegin() != view.cend() );
    REQUIRE( view.cbegin() <= view.cend() );
    REQUIRE( view.cbegin() < view.cend() );

    const auto signedSize = static_cast< std::ptrdiff_t >( testIndices.size() );
    REQUIRE( ( view.begin() + 0 ) == view.begin() );
    REQUIRE( ( view.cbegin() + 0 ) == view.cbegin() );
    REQUIRE( ( view.begin() + signedSize ) == view.end() );
    REQUIRE( ( view.cbegin() + signedSize ) == view.cend() );
    REQUIRE( ( view.end() - view.begin() ) == signedSize );
    REQUIRE( ( view.cend() - view.cbegin() ) == signedSize );

    REQUIRE( std::equal( view.cbegin(), view.cend(), testIndices.begin() ) );
}

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

#include <bitwindows.hpp>
#include <internal/util.hpp>

#include <cstdint>
#include <limits>

using bit7z::checkOverflow;
using bit7z::seekToOffset;

TEST_CASE( "util: Calling checkOverflow on a non-overflowing offset", "[util][checkOverflow]" ) { //-V2008
    constexpr auto kMaxValue = ( std::numeric_limits< std::int64_t >::max )();
    constexpr auto kMinValue = ( std::numeric_limits< std::int64_t >::min )();

    REQUIRE_FALSE( checkOverflow( kMaxValue, 0 ) );
    REQUIRE_FALSE( checkOverflow( kMaxValue, -1 ) );
    REQUIRE_FALSE( checkOverflow( kMaxValue, -42 ) );
    REQUIRE_FALSE( checkOverflow( kMaxValue, kMinValue ) );

    REQUIRE_FALSE( checkOverflow( 42, 42 ) );
    REQUIRE_FALSE( checkOverflow( 42, 1 ) );
    REQUIRE_FALSE( checkOverflow( 42, 0 ) );
    REQUIRE_FALSE( checkOverflow( 42, -1 ) );
    REQUIRE_FALSE( checkOverflow( 42, -42 ) );
    REQUIRE_FALSE( checkOverflow( 42, kMinValue ) );

    REQUIRE_FALSE( checkOverflow( 1, 42 ) );
    REQUIRE_FALSE( checkOverflow( 1, 1 ) );
    REQUIRE_FALSE( checkOverflow( 1, 0 ) );
    REQUIRE_FALSE( checkOverflow( 1, -1 ) );
    REQUIRE_FALSE( checkOverflow( 1, -42 ) );
    REQUIRE_FALSE( checkOverflow( 1, kMinValue ) );

    REQUIRE_FALSE( checkOverflow( 0, kMaxValue ) );
    REQUIRE_FALSE( checkOverflow( 0, 42 ) );
    REQUIRE_FALSE( checkOverflow( 0, 1 ) );
    REQUIRE_FALSE( checkOverflow( 0, 0 ) );
    REQUIRE_FALSE( checkOverflow( 0, -1 ) );
    REQUIRE_FALSE( checkOverflow( 0, -42 ) );
    REQUIRE_FALSE( checkOverflow( 0, kMinValue ) );

    REQUIRE_FALSE( checkOverflow( -1, kMaxValue ) );
    REQUIRE_FALSE( checkOverflow( -1, 42 ) );
    REQUIRE_FALSE( checkOverflow( -1, 1 ) );
    REQUIRE_FALSE( checkOverflow( -1, 0 ) );
    REQUIRE_FALSE( checkOverflow( -1, -1 ) );
    REQUIRE_FALSE( checkOverflow( -1, -42 ) );

    REQUIRE_FALSE( checkOverflow( -42, kMaxValue ) );
    REQUIRE_FALSE( checkOverflow( -42, 42 ) );
    REQUIRE_FALSE( checkOverflow( -42, 1 ) );
    REQUIRE_FALSE( checkOverflow( -42, 0 ) );
    REQUIRE_FALSE( checkOverflow( -42, -1 ) );
    REQUIRE_FALSE( checkOverflow( -42, -42 ) );

    REQUIRE_FALSE( checkOverflow( kMinValue, 0 ) );
    REQUIRE_FALSE( checkOverflow( kMinValue, 1 ) );
    REQUIRE_FALSE( checkOverflow( kMinValue, 42 ) );
    REQUIRE_FALSE( checkOverflow( kMinValue, kMaxValue ) );
}

TEST_CASE( "util: Calling checkOverflow on an overflowing offset", "[util][checkOverflow]" ) {
    constexpr auto kMaxValue = ( std::numeric_limits< std::int64_t >::max )();
    constexpr auto kMinValue = ( std::numeric_limits< std::int64_t >::min )();

    REQUIRE( checkOverflow( kMaxValue, kMaxValue ) );
    REQUIRE( checkOverflow( kMaxValue, 42 ) );
    REQUIRE( checkOverflow( kMaxValue, 1 ) );
    REQUIRE( checkOverflow( 42, kMaxValue ) );
    REQUIRE( checkOverflow( 1, kMaxValue ) );
    REQUIRE( checkOverflow( -1, kMinValue ) );
    REQUIRE( checkOverflow( -42, kMinValue ) );
    REQUIRE( checkOverflow( kMinValue, -1 ) );
    REQUIRE( checkOverflow( kMinValue, -42 ) );
    REQUIRE( checkOverflow( kMinValue, kMinValue ) );
}


TEST_CASE( "util: Calculate the absolute position from the given position, and the offset", "[util][seekToOffset]" ) {
    std::uint64_t position = 0;
    REQUIRE( seekToOffset( position, 0 ) == S_OK );
    REQUIRE( position == 0 );

    REQUIRE( seekToOffset( position, 1 ) == S_OK );
    REQUIRE( position == 1 );

    REQUIRE( seekToOffset( position, -1 ) == S_OK );
    REQUIRE( position == 0 );

    REQUIRE( seekToOffset( position, -1 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seekToOffset( position, 42 ) == S_OK );
    REQUIRE( position == 42 );

    REQUIRE( seekToOffset( position, -42 ) == S_OK );
    REQUIRE( position == 0 );

    REQUIRE( seekToOffset( position, -42 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::min() ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::min() + 1 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::max() ) == S_OK );
    REQUIRE( position == static_cast< std::uint64_t >( std::numeric_limits< std::int64_t >::max() ) );

    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::min() + 1 ) == S_OK );
    REQUIRE( position == 0 );

    position = 1;
    REQUIRE( seekToOffset( position, -42 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 1 );

    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::min() ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 1 );

    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::max() ) == S_OK );
    REQUIRE( position == ( 1u + static_cast< std::uint64_t >( std::numeric_limits< std::int64_t >::max() ) ) );

    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::min() ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == ( 1u + static_cast< std::uint64_t >( std::numeric_limits< std::int64_t >::max() ) ) );

    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::min() + 1 ) == S_OK );
    REQUIRE( position == 1 );

    position = 64;
    REQUIRE( seekToOffset( position, -1 ) == S_OK );
    REQUIRE( position == 63 );

    position = 64;
    REQUIRE( seekToOffset( position, 0 ) == S_OK );
    REQUIRE( position == 64 );

    REQUIRE( seekToOffset( position, 1 ) == S_OK );
    REQUIRE( position == 65 );

    position = std::numeric_limits< std::uint64_t >::max() - 1u;
    REQUIRE( seekToOffset( position, 1 ) == S_OK );
    REQUIRE( position == std::numeric_limits< std::uint64_t >::max() );

    REQUIRE( seekToOffset( position, 1 ) ==  E_INVALIDARG );
    REQUIRE( position == std::numeric_limits< std::uint64_t >::max() );

    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::min() + 1 ) == S_OK );
    REQUIRE( position == ( std::numeric_limits< std::uint64_t >::max() - static_cast< std::uint64_t >( std::numeric_limits< std::int64_t >::max() ) ) );

    position = std::numeric_limits< std::uint64_t >::max();
    REQUIRE( seekToOffset( position, std::numeric_limits< std::int64_t >::max() ) ==  E_INVALIDARG );
    REQUIRE( position == std::numeric_limits< std::uint64_t >::max() );
}

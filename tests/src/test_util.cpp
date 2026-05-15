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

using bit7z::check_overflow;
using bit7z::clamp_cast;
using bit7z::seek_to_offset;

TEST_CASE( "util: Calling check_overflow on a non-overflowing offset", "[util][check_overflow]" ) { //-V2008
    constexpr auto kMaxValue = ( std::numeric_limits< std::int64_t >::max )();
    constexpr auto kMinValue = ( std::numeric_limits< std::int64_t >::min )();

    REQUIRE_FALSE( check_overflow( kMaxValue, 0 ) );
    REQUIRE_FALSE( check_overflow( kMaxValue, -1 ) );
    REQUIRE_FALSE( check_overflow( kMaxValue, -42 ) );
    REQUIRE_FALSE( check_overflow( kMaxValue, kMinValue ) );

    REQUIRE_FALSE( check_overflow( 42, 42 ) );
    REQUIRE_FALSE( check_overflow( 42, 1 ) );
    REQUIRE_FALSE( check_overflow( 42, 0 ) );
    REQUIRE_FALSE( check_overflow( 42, -1 ) );
    REQUIRE_FALSE( check_overflow( 42, -42 ) );
    REQUIRE_FALSE( check_overflow( 42, kMinValue ) );

    REQUIRE_FALSE( check_overflow( 1, 42 ) );
    REQUIRE_FALSE( check_overflow( 1, 1 ) );
    REQUIRE_FALSE( check_overflow( 1, 0 ) );
    REQUIRE_FALSE( check_overflow( 1, -1 ) );
    REQUIRE_FALSE( check_overflow( 1, -42 ) );
    REQUIRE_FALSE( check_overflow( 1, kMinValue ) );

    REQUIRE_FALSE( check_overflow( 0, kMaxValue ) );
    REQUIRE_FALSE( check_overflow( 0, 42 ) );
    REQUIRE_FALSE( check_overflow( 0, 1 ) );
    REQUIRE_FALSE( check_overflow( 0, 0 ) );
    REQUIRE_FALSE( check_overflow( 0, -1 ) );
    REQUIRE_FALSE( check_overflow( 0, -42 ) );
    REQUIRE_FALSE( check_overflow( 0, kMinValue ) );

    REQUIRE_FALSE( check_overflow( -1, kMaxValue ) );
    REQUIRE_FALSE( check_overflow( -1, 42 ) );
    REQUIRE_FALSE( check_overflow( -1, 1 ) );
    REQUIRE_FALSE( check_overflow( -1, 0 ) );
    REQUIRE_FALSE( check_overflow( -1, -1 ) );
    REQUIRE_FALSE( check_overflow( -1, -42 ) );

    REQUIRE_FALSE( check_overflow( -42, kMaxValue ) );
    REQUIRE_FALSE( check_overflow( -42, 42 ) );
    REQUIRE_FALSE( check_overflow( -42, 1 ) );
    REQUIRE_FALSE( check_overflow( -42, 0 ) );
    REQUIRE_FALSE( check_overflow( -42, -1 ) );
    REQUIRE_FALSE( check_overflow( -42, -42 ) );

    REQUIRE_FALSE( check_overflow( kMinValue, 0 ) );
    REQUIRE_FALSE( check_overflow( kMinValue, 1 ) );
    REQUIRE_FALSE( check_overflow( kMinValue, 42 ) );
    REQUIRE_FALSE( check_overflow( kMinValue, kMaxValue ) );
}

TEST_CASE( "util: Calling check_overflow on an overflowing offset", "[util][check_overflow]" ) {
    constexpr auto kMaxValue = ( std::numeric_limits< std::int64_t >::max )();
    constexpr auto kMinValue = ( std::numeric_limits< std::int64_t >::min )();

    REQUIRE( check_overflow( kMaxValue, kMaxValue ) );
    REQUIRE( check_overflow( kMaxValue, 42 ) );
    REQUIRE( check_overflow( kMaxValue, 1 ) );
    REQUIRE( check_overflow( 42, kMaxValue ) );
    REQUIRE( check_overflow( 1, kMaxValue ) );
    REQUIRE( check_overflow( -1, kMinValue ) );
    REQUIRE( check_overflow( -42, kMinValue ) );
    REQUIRE( check_overflow( kMinValue, -1 ) );
    REQUIRE( check_overflow( kMinValue, -42 ) );
    REQUIRE( check_overflow( kMinValue, kMinValue ) );
}


TEST_CASE( "util: Calculate the absolute position from the given position, and the offset", "[util][seek_to_offset]" ) {
    std::uint64_t position = 0;
    REQUIRE( seek_to_offset( position, 0 ) == S_OK );
    REQUIRE( position == 0 );

    REQUIRE( seek_to_offset( position, 1 ) == S_OK );
    REQUIRE( position == 1 );

    REQUIRE( seek_to_offset( position, -1 ) == S_OK );
    REQUIRE( position == 0 );

    REQUIRE( seek_to_offset( position, -1 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seek_to_offset( position, 42 ) == S_OK );
    REQUIRE( position == 42 );

    REQUIRE( seek_to_offset( position, -42 ) == S_OK );
    REQUIRE( position == 0 );

    REQUIRE( seek_to_offset( position, -42 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::min() ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::min() + 1 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::max() ) == S_OK );
    REQUIRE( position == static_cast< std::uint64_t >( std::numeric_limits< std::int64_t >::max() ) );

    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::min() + 1 ) == S_OK );
    REQUIRE( position == 0 );

    position = 1;
    REQUIRE( seek_to_offset( position, -42 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 1 );

    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::min() ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 1 );

    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::max() ) == S_OK );
    REQUIRE( position == ( 1u + static_cast< std::uint64_t >( std::numeric_limits< std::int64_t >::max() ) ) );

    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::min() ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == ( 1u + static_cast< std::uint64_t >( std::numeric_limits< std::int64_t >::max() ) ) );

    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::min() + 1 ) == S_OK );
    REQUIRE( position == 1 );

    position = 64;
    REQUIRE( seek_to_offset( position, -1 ) == S_OK );
    REQUIRE( position == 63 );

    position = 64;
    REQUIRE( seek_to_offset( position, 0 ) == S_OK );
    REQUIRE( position == 64 );

    REQUIRE( seek_to_offset( position, 1 ) == S_OK );
    REQUIRE( position == 65 );

    position = std::numeric_limits< std::uint64_t >::max() - 1u;
    REQUIRE( seek_to_offset( position, 1 ) == S_OK );
    REQUIRE( position == std::numeric_limits< std::uint64_t >::max() );

    REQUIRE( seek_to_offset( position, 1 ) ==  E_INVALIDARG );
    REQUIRE( position == std::numeric_limits< std::uint64_t >::max() );

    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::min() + 1 ) == S_OK );
    REQUIRE( position == ( std::numeric_limits< std::uint64_t >::max() - static_cast< std::uint64_t >( std::numeric_limits< std::int64_t >::max() ) ) );

    position = std::numeric_limits< std::uint64_t >::max();
    REQUIRE( seek_to_offset( position, std::numeric_limits< std::int64_t >::max() ) ==  E_INVALIDARG );
    REQUIRE( position == std::numeric_limits< std::uint64_t >::max() );
}
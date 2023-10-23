// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include <internal/util.hpp>

using bit7z::check_overflow;
using bit7z::clamp_cast;
using bit7z::cmp_greater_equal;
using bit7z::seek_to_offset;

TEST_CASE( "util: Calling check_overflow on a non-overflowing offset", "[util][check_overflow]" ) { //-V2008
    constexpr auto kMaxValue = ( std::numeric_limits< int64_t >::max )();
    constexpr auto kMinValue = ( std::numeric_limits< int64_t >::min )();

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
    constexpr auto kMaxValue = ( std::numeric_limits< int64_t >::max )();
    constexpr auto kMinValue = ( std::numeric_limits< int64_t >::min )();

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
    uint64_t position = 0;
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

    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::min() ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::min() + 1 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 0 );

    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::max() ) == S_OK );
    REQUIRE( position == static_cast< std::uint64_t >( std::numeric_limits< int64_t >::max() ) );

    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::min() + 1 ) == S_OK );
    REQUIRE( position == 0 );

    position = 1;
    REQUIRE( seek_to_offset( position, -42 ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 1 );

    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::min() ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == 1 );

    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::max() ) == S_OK );
    REQUIRE( position == ( 1u + static_cast< std::uint64_t >( std::numeric_limits< int64_t >::max() ) ) );

    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::min() ) == HRESULT_WIN32_ERROR_NEGATIVE_SEEK );
    REQUIRE( position == ( 1u + static_cast< std::uint64_t >( std::numeric_limits< int64_t >::max() ) ) );

    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::min() + 1 ) == S_OK );
    REQUIRE( position == 1 );

    position = 64;
    REQUIRE( seek_to_offset( position, -1 ) == S_OK );
    REQUIRE( position == 63 );

    position = 64;
    REQUIRE( seek_to_offset( position, 0 ) == S_OK );
    REQUIRE( position == 64 );

    REQUIRE( seek_to_offset( position, 1 ) == S_OK );
    REQUIRE( position == 65 );

    position = std::numeric_limits< uint64_t >::max() - 1u;
    REQUIRE( seek_to_offset( position, 1 ) == S_OK );
    REQUIRE( position == std::numeric_limits< uint64_t >::max() );

    REQUIRE( seek_to_offset( position, 1 ) ==  E_INVALIDARG );
    REQUIRE( position == std::numeric_limits< uint64_t >::max() );

    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::min() + 1 ) == S_OK );
    REQUIRE( position == ( std::numeric_limits< uint64_t >::max() - static_cast< uint64_t >( std::numeric_limits< int64_t >::max() ) ) );

    position = std::numeric_limits< uint64_t >::max();
    REQUIRE( seek_to_offset( position, std::numeric_limits< int64_t >::max() ) ==  E_INVALIDARG );
    REQUIRE( position == std::numeric_limits< uint64_t >::max() );
}

/* unsigned -> unsigned */
TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::uint8_t", "[util][clamp_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( clamp_cast< std::uint8_t >( TestType{ 0u } ) == std::uint8_t{ 0u } );
    REQUIRE( clamp_cast< std::uint8_t >( TestType{ 1u } ) == std::uint8_t{ 1u } );
    REQUIRE( clamp_cast< std::uint8_t >( TestType{ 42u } ) == std::uint8_t{ 42u } );

    if ( sizeof( TestType ) > sizeof( std::uint8_t ) ) {
        TestType maxValue8bit{ std::numeric_limits< std::uint8_t >::max() };
        REQUIRE( clamp_cast< std::uint8_t >( maxValue8bit ) == std::numeric_limits< std::uint8_t >::max() );
    }

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( clamp_cast< std::uint8_t >( maxValue ) == std::numeric_limits< std::uint8_t >::max() );
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::uint16_t", "[util][clamp_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( clamp_cast< std::uint16_t >( TestType{ 0u } ) == std::uint16_t{ 0u } );
    REQUIRE( clamp_cast< std::uint16_t >( TestType{ 1u } ) == std::uint16_t{ 1u } );
    REQUIRE( clamp_cast< std::uint16_t >( TestType{ 42u } ) == std::uint16_t{ 42u } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::uint16_t maxDestinationValue{ std::numeric_limits< std::uint16_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( clamp_cast< std::uint16_t >( maxValue ) == maxDestinationValue );
        REQUIRE( clamp_cast< std::uint16_t >( static_cast< TestType >( maxDestinationValue ) ) == maxDestinationValue );
    } else { // widening
        REQUIRE( clamp_cast< std::uint16_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::uint32_t", "[util][clamp_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( clamp_cast< std::uint32_t >( TestType{ 0u } ) == std::uint32_t{ 0u } );
    REQUIRE( clamp_cast< std::uint32_t >( TestType{ 1u } ) == std::uint32_t{ 1u } );
    REQUIRE( clamp_cast< std::uint32_t >( TestType{ 42u } ) == std::uint32_t{ 42u } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::uint32_t maxDestinationValue{ std::numeric_limits< std::uint32_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( clamp_cast< std::uint32_t >( maxValue ) == maxDestinationValue );
        REQUIRE( clamp_cast< std::uint32_t >( static_cast< TestType >( maxDestinationValue ) ) == maxDestinationValue );
    } else { // widening
        REQUIRE( clamp_cast< std::uint32_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from std::uint64_t to std::uint64_t", "[util][clamp_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( clamp_cast< std::uint64_t >( std::uint64_t{ 0u } ) == std::uint64_t{ 0u } );
    REQUIRE( clamp_cast< std::uint64_t >( std::uint64_t{ 1u } ) == std::uint64_t{ 1u } );
    REQUIRE( clamp_cast< std::uint64_t >( std::uint64_t{ 42u } ) == std::uint64_t{ 42u } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( clamp_cast< std::uint64_t >( maxValue ) == maxValue );
}

/* signed -> unsigned */
TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::uint8_t", "[util][clamp_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( clamp_cast< std::uint8_t >( TestType{ 0 } ) == std::uint8_t{ 0u } );
    REQUIRE( clamp_cast< std::uint8_t >( TestType{ 1 } ) == std::uint8_t{ 1u } );
    REQUIRE( clamp_cast< std::uint8_t >( TestType{ 42 } ) == std::uint8_t{ 42u } );

    REQUIRE( clamp_cast< std::uint8_t >( TestType{ -1 } ) == std::uint8_t{ 0u } );
    REQUIRE( clamp_cast< std::uint8_t >( TestType{ -42 } ) == std::uint8_t{ 0u } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( clamp_cast< std::uint8_t >( minValue ) == 0u );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::uint8_t maxDestinationValue{ std::numeric_limits< std::uint8_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( clamp_cast< std::uint8_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( clamp_cast< std::uint8_t >( maxValue ) == static_cast< std::uint8_t >( maxValue ) );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::uint16_t", "[util][clamp_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( clamp_cast< std::uint16_t >( TestType{ 0 } ) == std::uint16_t{ 0u } );
    REQUIRE( clamp_cast< std::uint16_t >( TestType{ 1 } ) == std::uint16_t{ 1u } );
    REQUIRE( clamp_cast< std::uint16_t >( TestType{ 42 } ) == std::uint16_t{ 42u } );

    REQUIRE( clamp_cast< std::uint16_t >( TestType{ -1 } ) == std::uint16_t{ 0u } );
    REQUIRE( clamp_cast< std::uint16_t >( TestType{ -42 } ) == std::uint16_t{ 0u } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( clamp_cast< std::uint16_t >( minValue ) == 0u );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::uint16_t maxDestinationValue{ std::numeric_limits< std::uint16_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( clamp_cast< std::uint16_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( clamp_cast< std::uint16_t >( maxValue ) == static_cast< std::uint16_t >( maxValue ) );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::uint32_t", "[util][clamp_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( clamp_cast< std::uint32_t >( TestType{ 0 } ) == std::uint32_t{ 0u } );
    REQUIRE( clamp_cast< std::uint32_t >( TestType{ 1 } ) == std::uint32_t{ 1u } );
    REQUIRE( clamp_cast< std::uint32_t >( TestType{ 42 } ) == std::uint32_t{ 42u } );

    REQUIRE( clamp_cast< std::uint32_t >( TestType{ -1 } ) == std::uint32_t{ 0u } );
    REQUIRE( clamp_cast< std::uint32_t >( TestType{ -42 } ) == std::uint32_t{ 0u } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( clamp_cast< std::uint32_t >( minValue ) == 0u );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::uint32_t maxDestinationValue{ std::numeric_limits< std::uint32_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( clamp_cast< std::uint32_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( clamp_cast< std::uint32_t >( maxValue ) == static_cast< std::uint32_t >( maxValue ) );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::uint64_t", "[util][clamp_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( clamp_cast< std::uint64_t >( TestType{ 0 } ) == std::uint64_t{ 0u } );
    REQUIRE( clamp_cast< std::uint64_t >( TestType{ 1 } ) == std::uint64_t{ 1u } );
    REQUIRE( clamp_cast< std::uint64_t >( TestType{ 42 } ) == std::uint64_t{ 42u } );

    REQUIRE( clamp_cast< std::uint64_t >( TestType{ -1 } ) == std::uint64_t{ 0u } );
    REQUIRE( clamp_cast< std::uint64_t >( TestType{ -42 } ) == std::uint64_t{ 0u } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( clamp_cast< std::uint64_t >( minValue ) == 0u );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( clamp_cast< std::uint64_t >( maxValue ) == static_cast< std::uint64_t >( maxValue ) );
}

/* unsigned -> signed */
TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::int8_t", "[util][clamp_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( clamp_cast< std::int8_t >( TestType{ 0u } ) == std::int8_t{ 0 } );
    REQUIRE( clamp_cast< std::int8_t >( TestType{ 1u } ) == std::int8_t{ 1 } );
    REQUIRE( clamp_cast< std::int8_t >( TestType{ 42u } ) == std::int8_t{ 42 } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( clamp_cast< std::int8_t >( maxValue ) == std::numeric_limits< std::int8_t >::max() );
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::int16_t", "[util][clamp_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( clamp_cast< std::int16_t >( TestType{ 0u } ) == std::int16_t{ 0 } );
    REQUIRE( clamp_cast< std::int16_t >( TestType{ 1u } ) == std::int16_t{ 1 } );
    REQUIRE( clamp_cast< std::int16_t >( TestType{ 42u } ) == std::int16_t{ 42 } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::int16_t maxDestinationValue{ std::numeric_limits< std::int16_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( clamp_cast< std::int16_t >( maxValue ) == maxDestinationValue );
    } else { // widening
        REQUIRE( clamp_cast< std::int16_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::int32_t", "[util][clamp_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( clamp_cast< std::int32_t >( TestType{ 0u } ) == std::int32_t{ 0 } );
    REQUIRE( clamp_cast< std::int32_t >( TestType{ 1u } ) == std::int32_t{ 1 } );
    REQUIRE( clamp_cast< std::int32_t >( TestType{ 42u } ) == std::int32_t{ 42 } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::int32_t maxDestinationValue{ std::numeric_limits< std::int32_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( clamp_cast< std::int32_t >( maxValue ) == maxDestinationValue );
    } else { // widening
        REQUIRE( clamp_cast< std::int32_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::int64_t", "[util][clamp_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( clamp_cast< std::int64_t >( TestType{ 0u } ) == std::int64_t{ 0 } );
    REQUIRE( clamp_cast< std::int64_t >( TestType{ 1u } ) == std::int64_t{ 1 } );
    REQUIRE( clamp_cast< std::int64_t >( TestType{ 42u } ) == std::int64_t{ 42 } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::int64_t maxDestinationValue{ std::numeric_limits< std::int64_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( clamp_cast< std::int64_t >( maxValue ) == maxDestinationValue );
    } else { // widening
        REQUIRE( clamp_cast< std::int64_t >( maxValue ) == maxValue );
    }
}

/* signed -> signed */
TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::int8_t", "[util][clamp_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( clamp_cast< std::int8_t >( TestType{ 0 } ) == std::int8_t{ 0 } );
    REQUIRE( clamp_cast< std::int8_t >( TestType{ 1 } ) == std::int8_t{ 1 } );
    REQUIRE( clamp_cast< std::int8_t >( TestType{ 42 } ) == std::int8_t{ 42 } );

    REQUIRE( clamp_cast< std::int8_t >( TestType{ -1 } ) == std::int8_t{ -1 } );
    REQUIRE( clamp_cast< std::int8_t >( TestType{ -42 } ) == std::int8_t{ -42 } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( clamp_cast< std::int8_t >( minValue ) == std::numeric_limits< std::int8_t >::min() );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( clamp_cast< std::int8_t >( maxValue ) == std::numeric_limits< std::int8_t >::max() );
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::int16_t", "[util][clamp_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( clamp_cast< std::int16_t >( TestType{ 0 } ) == std::int16_t{ 0 } );
    REQUIRE( clamp_cast< std::int16_t >( TestType{ 1 } ) == std::int16_t{ 1 } );
    REQUIRE( clamp_cast< std::int16_t >( TestType{ 42 } ) == std::int16_t{ 42 } );

    REQUIRE( clamp_cast< std::int16_t >( TestType{ -1 } ) == std::int16_t{ -1 } );
    REQUIRE( clamp_cast< std::int16_t >( TestType{ -42 } ) == std::int16_t{ -42 } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    std::int16_t minDestinationValue{ std::numeric_limits< std::int16_t >::min() };
    if ( minValue <= minDestinationValue ) {
        REQUIRE( clamp_cast< std::int16_t >( minValue ) == minDestinationValue );
    } else {
        REQUIRE( clamp_cast< std::int16_t >( minValue ) == minValue );
    }

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::int16_t maxDestinationValue{ std::numeric_limits< std::int16_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( clamp_cast< std::int16_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( clamp_cast< std::int16_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::int32_t", "[util][clamp_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( clamp_cast< std::int32_t >( TestType{ 0 } ) == std::int32_t{ 0 } );
    REQUIRE( clamp_cast< std::int32_t >( TestType{ 1 } ) == std::int32_t{ 1 } );
    REQUIRE( clamp_cast< std::int32_t >( TestType{ 42 } ) == std::int32_t{ 42 } );

    REQUIRE( clamp_cast< std::int32_t >( TestType{ -1 } ) == std::int32_t{ -1 } );
    REQUIRE( clamp_cast< std::int32_t >( TestType{ -42 } ) == std::int32_t{ -42 } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    std::int32_t minDestinationValue{ std::numeric_limits< std::int32_t >::min() };
    if ( minValue <= minDestinationValue ) {
        REQUIRE( clamp_cast< std::int32_t >( minValue ) == minDestinationValue );
    } else {
        REQUIRE( clamp_cast< std::int32_t >( minValue ) == minValue );
    }

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::int32_t maxDestinationValue{ std::numeric_limits< std::int32_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( clamp_cast< std::int32_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( clamp_cast< std::int32_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from std::int64_t to std::int64_t", "[util][clamp_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( clamp_cast< std::int64_t >( TestType{ 0 } ) == std::int64_t{ 0 } );
    REQUIRE( clamp_cast< std::int64_t >( TestType{ 1 } ) == std::int64_t{ 1 } );
    REQUIRE( clamp_cast< std::int64_t >( TestType{ 42 } ) == std::int64_t{ 42 } );

    REQUIRE( clamp_cast< std::int64_t >( TestType{ -1 } ) == std::int64_t{ -1 } );
    REQUIRE( clamp_cast< std::int64_t >( TestType{ -42 } ) == std::int64_t{ -42 } );

    TestType minValue{ std::numeric_limits< TestType >::max() };
    std::int64_t minDestinationValue{ std::numeric_limits< std::int64_t >::min() };
    if ( minValue <= minDestinationValue ) {
        REQUIRE( clamp_cast< std::int64_t >( minValue ) == minDestinationValue );
    } else {
        REQUIRE( clamp_cast< std::int64_t >( minValue ) == minValue );
    }

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    std::int64_t maxDestinationValue{ std::numeric_limits< std::int64_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( clamp_cast< std::int64_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( clamp_cast< std::int64_t >( maxValue ) == maxValue );
    }
}
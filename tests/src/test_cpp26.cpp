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

#include <internal/cpp20.hpp>
#include <internal/cpp26.hpp>

using bit7z::cpp20::cmp_greater;
using bit7z::cpp20::cmp_greater_equal;
using bit7z::cpp26::saturating_cast;

/* unsigned -> unsigned */
TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::uint8_t", "[util][saturating_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( saturating_cast< std::uint8_t >( TestType{ 0u } ) == std::uint8_t{ 0u } );
    REQUIRE( saturating_cast< std::uint8_t >( TestType{ 1u } ) == std::uint8_t{ 1u } );
    REQUIRE( saturating_cast< std::uint8_t >( TestType{ 42u } ) == std::uint8_t{ 42u } );

    if ( sizeof( TestType ) > sizeof( std::uint8_t ) ) {
        TestType maxValue8bit{ std::numeric_limits< std::uint8_t >::max() };
        REQUIRE( saturating_cast< std::uint8_t >( maxValue8bit ) == std::numeric_limits< std::uint8_t >::max() );
    }

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( saturating_cast< std::uint8_t >( maxValue ) == std::numeric_limits< std::uint8_t >::max() );
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::uint16_t", "[util][saturating_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( saturating_cast< std::uint16_t >( TestType{ 0u } ) == std::uint16_t{ 0u } );
    REQUIRE( saturating_cast< std::uint16_t >( TestType{ 1u } ) == std::uint16_t{ 1u } );
    REQUIRE( saturating_cast< std::uint16_t >( TestType{ 42u } ) == std::uint16_t{ 42u } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::uint16_t maxDestinationValue{ std::numeric_limits< std::uint16_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( saturating_cast< std::uint16_t >( maxValue ) == maxDestinationValue );
        REQUIRE( saturating_cast< std::uint16_t >( static_cast< TestType >( maxDestinationValue ) ) == maxDestinationValue );
    } else { // widening
        REQUIRE( saturating_cast< std::uint16_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::uint32_t", "[util][saturating_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( saturating_cast< std::uint32_t >( TestType{ 0u } ) == std::uint32_t{ 0u } );
    REQUIRE( saturating_cast< std::uint32_t >( TestType{ 1u } ) == std::uint32_t{ 1u } );
    REQUIRE( saturating_cast< std::uint32_t >( TestType{ 42u } ) == std::uint32_t{ 42u } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::uint32_t maxDestinationValue{ std::numeric_limits< std::uint32_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( saturating_cast< std::uint32_t >( maxValue ) == maxDestinationValue );
        REQUIRE( saturating_cast< std::uint32_t >( static_cast< TestType >( maxDestinationValue ) ) == maxDestinationValue );
    } else { // widening
        REQUIRE( saturating_cast< std::uint32_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from std::uint64_t to std::uint64_t", "[util][saturating_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( saturating_cast< std::uint64_t >( std::uint64_t{ 0u } ) == std::uint64_t{ 0u } );
    REQUIRE( saturating_cast< std::uint64_t >( std::uint64_t{ 1u } ) == std::uint64_t{ 1u } );
    REQUIRE( saturating_cast< std::uint64_t >( std::uint64_t{ 42u } ) == std::uint64_t{ 42u } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( saturating_cast< std::uint64_t >( maxValue ) == maxValue );
}

/* signed -> unsigned */
TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::uint8_t", "[util][saturating_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( saturating_cast< std::uint8_t >( TestType{ 0 } ) == std::uint8_t{ 0u } );
    REQUIRE( saturating_cast< std::uint8_t >( TestType{ 1 } ) == std::uint8_t{ 1u } );
    REQUIRE( saturating_cast< std::uint8_t >( TestType{ 42 } ) == std::uint8_t{ 42u } );

    REQUIRE( saturating_cast< std::uint8_t >( TestType{ -1 } ) == std::uint8_t{ 0u } );
    REQUIRE( saturating_cast< std::uint8_t >( TestType{ -42 } ) == std::uint8_t{ 0u } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( saturating_cast< std::uint8_t >( minValue ) == 0u );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::uint8_t maxDestinationValue{ std::numeric_limits< std::uint8_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( saturating_cast< std::uint8_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( saturating_cast< std::uint8_t >( maxValue ) == static_cast< std::uint8_t >( maxValue ) );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::uint16_t", "[util][saturating_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( saturating_cast< std::uint16_t >( TestType{ 0 } ) == std::uint16_t{ 0u } );
    REQUIRE( saturating_cast< std::uint16_t >( TestType{ 1 } ) == std::uint16_t{ 1u } );
    REQUIRE( saturating_cast< std::uint16_t >( TestType{ 42 } ) == std::uint16_t{ 42u } );

    REQUIRE( saturating_cast< std::uint16_t >( TestType{ -1 } ) == std::uint16_t{ 0u } );
    REQUIRE( saturating_cast< std::uint16_t >( TestType{ -42 } ) == std::uint16_t{ 0u } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( saturating_cast< std::uint16_t >( minValue ) == 0u );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::uint16_t maxDestinationValue{ std::numeric_limits< std::uint16_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( saturating_cast< std::uint16_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( saturating_cast< std::uint16_t >( maxValue ) == static_cast< std::uint16_t >( maxValue ) );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::uint32_t", "[util][saturating_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( saturating_cast< std::uint32_t >( TestType{ 0 } ) == std::uint32_t{ 0u } );
    REQUIRE( saturating_cast< std::uint32_t >( TestType{ 1 } ) == std::uint32_t{ 1u } );
    REQUIRE( saturating_cast< std::uint32_t >( TestType{ 42 } ) == std::uint32_t{ 42u } );

    REQUIRE( saturating_cast< std::uint32_t >( TestType{ -1 } ) == std::uint32_t{ 0u } );
    REQUIRE( saturating_cast< std::uint32_t >( TestType{ -42 } ) == std::uint32_t{ 0u } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( saturating_cast< std::uint32_t >( minValue ) == 0u );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::uint32_t maxDestinationValue{ std::numeric_limits< std::uint32_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( saturating_cast< std::uint32_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( saturating_cast< std::uint32_t >( maxValue ) == static_cast< std::uint32_t >( maxValue ) );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::uint64_t", "[util][saturating_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( saturating_cast< std::uint64_t >( TestType{ 0 } ) == std::uint64_t{ 0u } );
    REQUIRE( saturating_cast< std::uint64_t >( TestType{ 1 } ) == std::uint64_t{ 1u } );
    REQUIRE( saturating_cast< std::uint64_t >( TestType{ 42 } ) == std::uint64_t{ 42u } );

    REQUIRE( saturating_cast< std::uint64_t >( TestType{ -1 } ) == std::uint64_t{ 0u } );
    REQUIRE( saturating_cast< std::uint64_t >( TestType{ -42 } ) == std::uint64_t{ 0u } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( saturating_cast< std::uint64_t >( minValue ) == 0u );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( saturating_cast< std::uint64_t >( maxValue ) == static_cast< std::uint64_t >( maxValue ) );
}

/* unsigned -> signed */
TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::int8_t", "[util][saturating_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( saturating_cast< std::int8_t >( TestType{ 0u } ) == std::int8_t{ 0 } );
    REQUIRE( saturating_cast< std::int8_t >( TestType{ 1u } ) == std::int8_t{ 1 } );
    REQUIRE( saturating_cast< std::int8_t >( TestType{ 42u } ) == std::int8_t{ 42 } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( saturating_cast< std::int8_t >( maxValue ) == std::numeric_limits< std::int8_t >::max() );
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::int16_t", "[util][saturating_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( saturating_cast< std::int16_t >( TestType{ 0u } ) == std::int16_t{ 0 } );
    REQUIRE( saturating_cast< std::int16_t >( TestType{ 1u } ) == std::int16_t{ 1 } );
    REQUIRE( saturating_cast< std::int16_t >( TestType{ 42u } ) == std::int16_t{ 42 } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::int16_t maxDestinationValue{ std::numeric_limits< std::int16_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( saturating_cast< std::int16_t >( maxValue ) == maxDestinationValue );
    } else { // widening
        REQUIRE( saturating_cast< std::int16_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::int32_t", "[util][saturating_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( saturating_cast< std::int32_t >( TestType{ 0u } ) == std::int32_t{ 0 } );
    REQUIRE( saturating_cast< std::int32_t >( TestType{ 1u } ) == std::int32_t{ 1 } );
    REQUIRE( saturating_cast< std::int32_t >( TestType{ 42u } ) == std::int32_t{ 42 } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::int32_t maxDestinationValue{ std::numeric_limits< std::int32_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( saturating_cast< std::int32_t >( maxValue ) == maxDestinationValue );
    } else { // widening
        REQUIRE( saturating_cast< std::int32_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any unsigned type to std::int64_t", "[util][saturating_cast]",
                    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t ) {
    REQUIRE( saturating_cast< std::int64_t >( TestType{ 0u } ) == std::int64_t{ 0 } );
    REQUIRE( saturating_cast< std::int64_t >( TestType{ 1u } ) == std::int64_t{ 1 } );
    REQUIRE( saturating_cast< std::int64_t >( TestType{ 42u } ) == std::int64_t{ 42 } );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::int64_t maxDestinationValue{ std::numeric_limits< std::int64_t >::max() };
    if ( cmp_greater_equal( maxValue, maxDestinationValue ) ) {
        REQUIRE( saturating_cast< std::int64_t >( maxValue ) == maxDestinationValue );
    } else { // widening
        REQUIRE( saturating_cast< std::int64_t >( maxValue ) == maxValue );
    }
}

/* signed -> signed */
TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::int8_t", "[util][saturating_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( saturating_cast< std::int8_t >( TestType{ 0 } ) == std::int8_t{ 0 } );
    REQUIRE( saturating_cast< std::int8_t >( TestType{ 1 } ) == std::int8_t{ 1 } );
    REQUIRE( saturating_cast< std::int8_t >( TestType{ 42 } ) == std::int8_t{ 42 } );

    REQUIRE( saturating_cast< std::int8_t >( TestType{ -1 } ) == std::int8_t{ -1 } );
    REQUIRE( saturating_cast< std::int8_t >( TestType{ -42 } ) == std::int8_t{ -42 } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    REQUIRE( saturating_cast< std::int8_t >( minValue ) == std::numeric_limits< std::int8_t >::min() );

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    REQUIRE( saturating_cast< std::int8_t >( maxValue ) == std::numeric_limits< std::int8_t >::max() );
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::int16_t", "[util][saturating_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( saturating_cast< std::int16_t >( TestType{ 0 } ) == std::int16_t{ 0 } );
    REQUIRE( saturating_cast< std::int16_t >( TestType{ 1 } ) == std::int16_t{ 1 } );
    REQUIRE( saturating_cast< std::int16_t >( TestType{ 42 } ) == std::int16_t{ 42 } );

    REQUIRE( saturating_cast< std::int16_t >( TestType{ -1 } ) == std::int16_t{ -1 } );
    REQUIRE( saturating_cast< std::int16_t >( TestType{ -42 } ) == std::int16_t{ -42 } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    constexpr std::int16_t minDestinationValue{ std::numeric_limits< std::int16_t >::min() };
    if ( minValue <= minDestinationValue ) {
        REQUIRE( saturating_cast< std::int16_t >( minValue ) == minDestinationValue );
    } else {
        REQUIRE( saturating_cast< std::int16_t >( minValue ) == minValue );
    }

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::int16_t maxDestinationValue{ std::numeric_limits< std::int16_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( saturating_cast< std::int16_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( saturating_cast< std::int16_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from any signed type to std::int32_t", "[util][saturating_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( saturating_cast< std::int32_t >( TestType{ 0 } ) == std::int32_t{ 0 } );
    REQUIRE( saturating_cast< std::int32_t >( TestType{ 1 } ) == std::int32_t{ 1 } );
    REQUIRE( saturating_cast< std::int32_t >( TestType{ 42 } ) == std::int32_t{ 42 } );

    REQUIRE( saturating_cast< std::int32_t >( TestType{ -1 } ) == std::int32_t{ -1 } );
    REQUIRE( saturating_cast< std::int32_t >( TestType{ -42 } ) == std::int32_t{ -42 } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    constexpr std::int32_t minDestinationValue{ std::numeric_limits< std::int32_t >::min() };
    if ( minValue <= minDestinationValue ) {
        REQUIRE( saturating_cast< std::int32_t >( minValue ) == minDestinationValue );
    } else {
        REQUIRE( saturating_cast< std::int32_t >( minValue ) == minValue );
    }

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::int32_t maxDestinationValue{ std::numeric_limits< std::int32_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( saturating_cast< std::int32_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( saturating_cast< std::int32_t >( maxValue ) == maxValue );
    }
}

TEMPLATE_TEST_CASE( "util: Clamp cast from std::int64_t to std::int64_t", "[util][saturating_cast]",
                    std::int8_t, std::int16_t, std::int32_t, std::int64_t ) {
    REQUIRE( saturating_cast< std::int64_t >( TestType{ 0 } ) == std::int64_t{ 0 } );
    REQUIRE( saturating_cast< std::int64_t >( TestType{ 1 } ) == std::int64_t{ 1 } );
    REQUIRE( saturating_cast< std::int64_t >( TestType{ 42 } ) == std::int64_t{ 42 } );

    REQUIRE( saturating_cast< std::int64_t >( TestType{ -1 } ) == std::int64_t{ -1 } );
    REQUIRE( saturating_cast< std::int64_t >( TestType{ -42 } ) == std::int64_t{ -42 } );

    TestType minValue{ std::numeric_limits< TestType >::min() };
    constexpr std::int64_t minDestinationValue{ std::numeric_limits< std::int64_t >::min() };
    if ( minValue <= minDestinationValue ) {
        REQUIRE( saturating_cast< std::int64_t >( minValue ) == minDestinationValue );
    } else {
        REQUIRE( saturating_cast< std::int64_t >( minValue ) == minValue );
    }

    TestType maxValue{ std::numeric_limits< TestType >::max() };
    constexpr std::int64_t maxDestinationValue{ std::numeric_limits< std::int64_t >::max() };
    if ( maxValue >= maxDestinationValue ) {
        REQUIRE( saturating_cast< std::int64_t >( maxValue ) == maxDestinationValue );
    } else {
        REQUIRE( saturating_cast< std::int64_t >( maxValue ) == maxValue );
    }
}
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

#if !defined(__GNUC__) || __GNUC__ >= 5
#include <catch2/catch.hpp>

#include <internal/dateutil.hpp>

using namespace bit7z;

/* Note: std::time_t is usually a UNIX timestamp, so we are using only dates after the UNIX epoch datetime.
 * We do not expect the conversion function to check whether the input is correct,
 * so we don't perform tests with wrong inputs. */

#ifndef _WIN32

TEST_CASE( "fsutil: Date conversion from std::time_t to FILETIME", "[fsutil][date functions]" ) {
    auto test_date = GENERATE( table< const char*, std::time_t, FILETIME >(
        {
            { "21 December 2012, 12:00", 1356091200, { 3017121792, 30269298 } },
            { "1 January 1970, 00:00",   0,          { 3577643008, 27111902 } }
        }
    ) );

    DYNAMIC_SECTION( "Date: " << std::get< 0 >( test_date ) ) {
        const std::time_t input = std::get< 1 >( test_date );
        const FILETIME expected_output = std::get< 2 >( test_date );

        auto output = time_to_FILETIME( input );
        REQUIRE( output.dwHighDateTime == expected_output.dwHighDateTime );
        REQUIRE( output.dwLowDateTime == expected_output.dwLowDateTime );
    }
}

#endif

TEST_CASE( "fsutil: Date conversion from FILETIME to time types", "[fsutil][date functions]" ) {
    using namespace std::chrono;
    using std::chrono::seconds;

    auto test_date = GENERATE( table< const char*, FILETIME, std::time_t >(
        {
            { "21 December 2012, 12:00", { 3017121792, 30269298 }, 1356091200 },
            { "1 January 1970, 00:00",   { 3577643008, 27111902 }, 0 }
        }
    ) );

    DYNAMIC_SECTION( "Date: " << std::get< 0 >( test_date ) ) {
        const FILETIME input = std::get< 1 >( test_date );
        const std::time_t expected_output = std::get< 2 >( test_date );

        std::time_t output{};

#ifndef _WIN32
        SECTION( "FILETIME to std::filesystem::file_time_type" ) {
            auto result = FILETIME_to_file_time_type( input );
            output = std::time_t{ duration_cast< seconds >( result.time_since_epoch() ).count() };
        }
#endif

        SECTION( "FILETIME to bit7z::time_type" ) {
            auto result = FILETIME_to_time_type( input );
            output = bit7z::time_type::clock::to_time_t( result );
        }

        REQUIRE( output == expected_output );
    }
}
#endif
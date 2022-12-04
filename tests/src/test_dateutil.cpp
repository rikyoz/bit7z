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

#include <internal/dateutil.hpp>

using namespace bit7z;

/* Note: std::time_t is usually a UNIX timestamp, so we are using only dates after the UNIX epoch datetime.
 * We do not expect the conversion function to check whether the input is correct,
 * so we don't perform tests with wrong inputs. */

#ifndef _WIN32

TEST_CASE( "fsutil: Date conversion from std::time_t to FILETIME", "[fsutil][date functions]" ) {
    auto test_date = GENERATE( table< const char*, std::time_t, FILETIME >(
        {
            { "21 December 2012, 12:00", 1356091200, { .dwLowDateTime = 3017121792, .dwHighDateTime = 30269298 } },
            { "1 January 1970, 00:00",   0,          { .dwLowDateTime = 3577643008, .dwHighDateTime = 27111902 } }
        }
    ) );

    SECTION( std::get< 0 >( test_date ) ) { // Date string as section title
        std::time_t std_time = std::get< 1 >( test_date );
        FILETIME file_time = std::get< 2 >( test_date );

        auto result = time_to_FILETIME( std_time );

        REQUIRE( result.dwHighDateTime == file_time.dwHighDateTime );
        REQUIRE( result.dwLowDateTime == file_time.dwLowDateTime );
    }
}

#endif

TEST_CASE( "fsutil: Date conversion from FILETIME to time types", "[fsutil][date functions]" ) {
    using namespace std::chrono;
    using std::chrono::seconds;

    auto test_date = GENERATE( table< const char*, FILETIME, std::time_t >(
        {
            { "21 December 2012, 12:00", { .dwLowDateTime = 3017121792, .dwHighDateTime = 30269298 }, 1356091200 },
            { "1 January 1970, 00:00", { .dwLowDateTime = 3577643008, .dwHighDateTime = 27111902 }, 0 }
        }
    ) );

    SECTION( std::get< 0 >( test_date ) ) { // Date string as section title
        FILETIME file_time = std::get< 1 >( test_date );
        std::time_t std_time = std::get< 2 >( test_date );

#ifndef _WIN32
        SECTION( "FILETIME to std::filesystem::file_time_type" ) {
            auto result = FILETIME_to_file_time_type( file_time );

            auto time_result = std::time_t{ duration_cast< seconds >( result.time_since_epoch() ).count() };
            REQUIRE( std_time == time_result );
        }
#endif

        SECTION( "FILETIME to bit7z::time_type" ) {
            auto result = FILETIME_to_time_type( file_time );

            auto time_result = bit7z::time_type::clock::to_time_t( result );
            REQUIRE( std_time == time_result );
        }
    }
}
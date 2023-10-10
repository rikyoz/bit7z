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

struct DateConversionTest {
    const char* name;
    std::time_t dateTime;
    FILETIME fileTime;
};

TEST_CASE( "fsutil: Date conversions", "[fsutil][date functions]" ) {
    using namespace std::chrono;
    using std::chrono::seconds;

    auto testDate = GENERATE( as< DateConversionTest >(),
                              DateConversionTest{ "21 December 2012, 12:00", 1356091200, { 3017121792, 30269298 } },
                              DateConversionTest{ "1 January 1970, 00:00",   0,          { 3577643008, 27111902 } } );

    DYNAMIC_SECTION( "Date: " << testDate.name ) {

#ifndef _WIN32
        SECTION( "From std::time_t to FILETIME" ) {
            auto output = time_to_FILETIME( testDate.dateTime );
            REQUIRE( output.dwHighDateTime == testDate.fileTime.dwHighDateTime );
            REQUIRE( output.dwLowDateTime == testDate.fileTime.dwLowDateTime );
        }
#endif

        SECTION( "From FILETIME...") {
            std::time_t output{};
#ifndef _WIN32
            SECTION( "...to std::filesystem::file_time_type" ) {
                auto result = FILETIME_to_file_time_type( testDate.fileTime );
                output = std::time_t{ duration_cast< seconds >( result.time_since_epoch() ).count() };
            }
#endif

            SECTION( "...to bit7z::time_type" ) {
                auto result = FILETIME_to_time_type( testDate.fileTime );
                output = bit7z::time_type::clock::to_time_t( result );
            }

            REQUIRE( output == testDate.dateTime );
        }
    }
}
#endif
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

#ifndef _WIN32
#include "utils/archive.hpp"
#include "utils/datetime.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/bitarchivereader.hpp>
#endif

#include <internal/dateutil.hpp>

#ifndef _WIN32
#include <chrono>
#endif
#include <ctime> // For std::time_t (on MSVC 2015).

using namespace bit7z;

#ifndef _WIN32
using namespace bit7z::test;
using namespace bit7z::test::filesystem;
#endif

/* Note: std::time_t is usually a UNIX timestamp, so we are using only dates after the UNIX epoch datetime.
 * We do not expect the conversion function to check whether the input is correct,
 * so we don't perform tests with wrong inputs. */

struct DateConversionTest {
    const char* name;
    std::int64_t dateTime; // Using std::int64_t instead of std::time_t to use 64-bit time on x86
    FILETIME fileTime;
};

TEST_CASE( "fsutil: Date conversions", "[fsutil][date functions]" ) {
    using namespace std::chrono;
    using std::chrono::seconds;

    auto testDate = GENERATE( as< DateConversionTest >(),
                              DateConversionTest{ "13 February 2023, 20:54:25", 1676321665, { 1526060672, 31014893 } },
                              DateConversionTest{ "21 December 2012, 12:00:00", 1356091200, { 3017121792, 30269298 } },
                              DateConversionTest{ "1 January 1970, 00:00:00", 0, { 3577643008, 27111902 } } );

    DYNAMIC_SECTION( "Date: " << testDate.name ) {
#ifndef _WIN32
        SECTION( "From std::time_t to FILETIME" ) {
            auto result = toFILETIME( testDate.dateTime );
            REQUIRE( result.dwHighDateTime == testDate.fileTime.dwHighDateTime );
            REQUIRE( result.dwLowDateTime == testDate.fileTime.dwLowDateTime );
        }
#endif

#ifndef _WIN32
        SECTION( "From FILETIME to std::filesystem::file_time_type" ) {
            auto result = toFileTimeType( testDate.fileTime );
            REQUIRE( as_unix_timestamp( result ) == testDate.dateTime );
        }
#endif

        SECTION( "From FILETIME to bit7z::time_type" ) {
            const auto result = toTimeType( testDate.fileTime );
            const auto output = bit7z::time_type::clock::to_time_t( result );
            REQUIRE( output == testDate.dateTime );
        }

        SECTION( "From bit7z::time_type to FILETIME" ) {
            const auto input = bit7z::time_type::clock::from_time_t( testDate.dateTime );
            const auto result = toFILETIME( input );
            REQUIRE( result.dwHighDateTime == testDate.fileTime.dwHighDateTime );
            REQUIRE( result.dwLowDateTime == testDate.fileTime.dwLowDateTime );
        }

        SECTION( "Round-trip: FILETIME with sub-second precision -> bit7z::time_type -> FILETIME" ) {
            // 1000000 × 100ns = 100ms; a multiple of 10 to remain lossless on microseconds-resolution clocks.
            FILETIME original = testDate.fileTime;
            original.dwLowDateTime += 1000000; // NOLINT(*-magic-numbers)

            const auto asTimeType = toTimeType( original );
            const auto result = toFILETIME( asTimeType );
            REQUIRE( result.dwHighDateTime == original.dwHighDateTime );
            REQUIRE( result.dwLowDateTime == original.dwLowDateTime );
        }
    }
}

TEST_CASE( "fsutil: FILETIME_to_time_type clamps out-of-range FILETIMEs", "[fsutil][date functions]" ) {
    // FILETIME{0, 0} is the NT epoch (1601-01-01), which predates the Unix epoch by ~369 years.
    // On nanosecond-precision clocks (e.g., Linux), converting it would overflow signed 64-bit arithmetic;
    // the function must clamp to time_type::min() rather than invoking UB.
    // On 100ns-precision clocks (e.g., Windows), no overflow occurs and the exact date is returned.
    // In both cases the result must be strictly before the Unix epoch.
    const auto result = toTimeType( FILETIME{ 0, 0 } );
    REQUIRE( result < time_type::clock::from_time_t( 0 ) );
}

#ifndef _WIN32
TEST_CASE( "fsutil: Date conversion of current time should preserve information up to seconds",
           "[fsutil][date functions]" ) {
    const auto currentTime = std::chrono::system_clock::now();
    const auto unixTimestamp = as_unix_timestamp( currentTime );
    INFO( "Current time: " << unixTimestamp )

    // Converting the current time to FILETIME
    const auto asTimeT = std::chrono::system_clock::to_time_t( currentTime );
    const auto asFileTime = toFILETIME( asTimeT );

    SECTION( "Converting current FILETIME to a system_clock's time_point" ) {
        const auto asSystemTimePoint = toTimeType( asFileTime );
        REQUIRE( unixTimestamp == as_unix_timestamp( asSystemTimePoint ) );
    }

    SECTION( "Converting current FILETIME to a file_clock's time_point" ) {
        const auto asFileTimePoint = toFileTimeType( asFileTime );
        REQUIRE( unixTimestamp == as_unix_timestamp( asFileTimePoint ) );
    }
}

TEMPLATE_TEST_CASE( "fsutil: Date conversion of last write time", "[fsutil][date functions]",
                    tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto arcFileName = fs::path{ clouds.name }.concat( ".7z" );

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    BitArchiveReader info( test::sevenzip_lib(), inputArchive, BitFormat::SevenZip );

    const auto item = info.itemAt( 0 );

    const auto lastWriteTime = item.itemProperty( BitProperty::MTime ).getFileTime();
    INFO( "Last write time FILETIME: {" << lastWriteTime.dwHighDateTime << ", " << lastWriteTime.dwLowDateTime << "}")
    const auto result = toFileTimeType( lastWriteTime );

    const auto result_as_timestamp = as_unix_timestamp( result );
    INFO( "Last write file time_point: " << result_as_timestamp )

    const auto result2 = item.lastWriteTime();
    const auto result2_as_timestamp = as_unix_timestamp( result2 );
    INFO( "Last write system time_point: " << result2_as_timestamp )
    REQUIRE( result_as_timestamp == result2_as_timestamp );
}
#endif
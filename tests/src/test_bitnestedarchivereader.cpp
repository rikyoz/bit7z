// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include "utils/archive.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/bitnestedarchivereader.hpp>
#include <bit7z/bittypes.hpp>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

void require_extracts_to_filesystem( const BitNestedArchiveReader& info, const ExpectedItems& expectedItems ) {
    TempTestDirectory testDir{ "test_bitinputarchive" };
    INFO( "Test directory: " << testDir )

    REQUIRE_NOTHROW( info.extractTo( testDir ) );
    for ( const auto& expectedItem : expectedItems ) {
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitNestedArchiveReader: Reading nested archives", "[bitnestedarchivereader]",
                    tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "xz", BitFormat::Xz } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzip_lib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzip_lib(), outerArchive, BitFormat::Tar );
        REQUIRE( innerArchive.itemsCount() == multiple_files_content().fileCount );
        REQUIRE( innerArchive.openCount() == 1 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitNestedArchiveReader: Testing nested archives", "[bitnestedarchivereader]",
                    tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "xz", BitFormat::Xz } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzip_lib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzip_lib(), outerArchive, BitFormat::Tar );
        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitNestedArchiveReader: Extracting nested archives", "[bitnestedarchivereader]",
                    tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "xz", BitFormat::Xz } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzip_lib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzip_lib(), outerArchive, BitFormat::Tar );

        // TODO: Test all kind of extraction targets (buffers, streams, etc.)
        require_extracts_to_filesystem( innerArchive, multiple_files_content().items );
        REQUIRE( innerArchive.openCount() == 1 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitNestedArchiveReader: Multiple operations on nested archives", "[bitnestedarchivereader]",
                    tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "xz", BitFormat::Xz } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzip_lib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzip_lib(), outerArchive, BitFormat::Tar );

        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );

        // Note: the for-each loop will call the begin() and end() functions;
        // the latter, needs to calculate the number of items in the archive,
        // triggering the extraction of the outer archive to open the inner one.
        // On the subsequent request for an item property (e.g., item.name()),
        // the outer archive must be extracted again, as the counting of the items in the archive
        // consumed the result of the first extraction.
        /*for ( const auto& item : innerArchive ) {
            if ( item.name() == italy.name ) {
                REQUIRE( item.size() == italy.size );
            } else if ( item.name() == loremIpsum.name ) {
                REQUIRE( item.size() == loremIpsum.size );
            } else {
                FAIL( "Unexpected item" );
            }
        }*/
        REQUIRE( innerArchive.itemsCount() == 2 );
        REQUIRE( innerArchive.openCount() == 2 );

        require_extracts_to_filesystem( innerArchive, multiple_files_content().items );
        REQUIRE( innerArchive.openCount() == 3 );
    }
}
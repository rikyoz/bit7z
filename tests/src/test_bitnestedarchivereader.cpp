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
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "xz", BitFormat::Xz },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

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
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "xz", BitFormat::Xz },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

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
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "xz", BitFormat::Xz },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

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
TEMPLATE_TEST_CASE( "BitNestedArchiveReader: Reading items of nested archives", "[bitnestedarchivereader]",
                    tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "xz", BitFormat::Xz },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzip_lib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzip_lib(), outerArchive, BitFormat::Tar );

        // TODO: Test all kind of extraction targets (buffers, streams, etc.)
        const auto items = innerArchive.items();
        REQUIRE( items.size() == multiple_files_content().fileCount );
        REQUIRE( innerArchive.openCount() == 1 );
        for ( const auto& item : items ) {
            if ( item.name() == italy.name ) {
                REQUIRE( item.size() == italy.size );
            } else if ( item.name() == loremIpsum.name ) {
                REQUIRE( item.size() == loremIpsum.size );
            } else {
                FAIL( "Unexpected item" );
            }
        }
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitNestedArchiveReader: Multiple operations on nested archives", "[bitnestedarchivereader]",
                    tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "xz", BitFormat::Xz },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzip_lib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzip_lib(), outerArchive, BitFormat::Tar );

        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );

        REQUIRE( innerArchive.itemsCount() == 2 );
        REQUIRE( innerArchive.openCount() == 2 );

        require_extracts_to_filesystem( innerArchive, multiple_files_content().items );
        REQUIRE( innerArchive.openCount() == 3 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitNestedArchiveReader: Extracting multiple nested archives inside an archive",
                    "[bitnestedarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "multiple_nested.7z";
    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );

    BitArchiveReader outerArchive( test::sevenzip_lib(), inputArchive, BitFormat::SevenZip );

    for ( const auto& item : outerArchive ) {
        BitNestedArchiveReader innerArchive( test::sevenzip_lib(), outerArchive, item.index(), BitFormat::Tar );

        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );

        if ( item.name() == BIT7Z_STRING( "multiple_files.tar" ) ) {
            require_extracts_to_filesystem( innerArchive, multiple_files_content().items );
        } else if ( item.name() == BIT7Z_STRING( "multiple_items.tar" ) ) {
            require_extracts_to_filesystem( innerArchive, multiple_items_content().items );
        } else {
            FAIL( "Unexpected nested archive" );
        }

        REQUIRE( innerArchive.openCount() == 2 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitNestedArchiveReader: Extracting compressed archives inside an uncompressed tarball",
                    "[bitnestedarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "reversed_tarball.tar";
    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );

    BitArchiveReader outerArchive( test::sevenzip_lib(), inputArchive, BitFormat::Tar );

    TempDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Test directory: " << testOutDir )
    for ( const auto& item : outerArchive ) {
        const auto& format = [&item]() -> const BitInFormat& {
            const auto& ext = item.extension();
            if ( ext == "gz" ) {
                return BitFormat::GZip;
            }
            if ( ext == "bz2" ) {
                return BitFormat::BZip2;
            }
            return BitFormat::Xz;
        }();
        BitNestedArchiveReader innerArchive( test::sevenzip_lib(), outerArchive, item.index(), format );

        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );

        require_extracts_to_filesystem( innerArchive, single_file_content().items );
        REQUIRE( innerArchive.openCount() == 2 );
    }
}
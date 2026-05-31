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

#include "utils/archive.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/bitnestedarchivereader.hpp>
#include <bit7z/bittypes.hpp>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

namespace {
void require_extracts_to_filesystem( const BitNestedArchiveReader& info, const ExpectedItems& expectedItems ) {
    const TempTestDirectory testDir{ "test_bitinputarchive" };
    INFO( "Test directory: " << testDir )

    REQUIRE_NOTHROW( info.extractTo( testDir ) );
    for ( const auto& expectedItem : expectedItems ) {
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
    }
}
} // namespace

#ifdef BIT7Z_AUTO_FORMAT
// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Automatic format detection is not supported",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );

        REQUIRE_THROWS( BitNestedArchiveReader{ test::sevenzipLib(), outerArchive, BitFormat::Auto } );
    }
}
#endif

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Reading nested archives",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );
        REQUIRE( innerArchive.itemsCount() == multipleFilesContent().fileCount );
        REQUIRE( innerArchive.openCount() == 1 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Testing nested archives",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );
        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Extracting nested archives",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

        // TODO: Test all kind of extraction targets (buffers, streams, etc.)
        require_extracts_to_filesystem( innerArchive, multipleFilesContent().items );
        REQUIRE( innerArchive.openCount() == 1 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Reading items of nested archives",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

        // TODO: Test all kind of extraction targets (buffers, streams, etc.)
        const auto items = innerArchive.items();
        REQUIRE( items.size() == multipleFilesContent().fileCount );
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
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Multiple operations on nested archives",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );

        REQUIRE( innerArchive.itemsCount() == 2 );
        REQUIRE( innerArchive.openCount() == 2 );

        require_extracts_to_filesystem( innerArchive, multipleFilesContent().items );
        REQUIRE( innerArchive.openCount() == 3 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Extracting multiple nested archives inside an archive",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "multiple_nested.7z";
    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );

    BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::SevenZip );

    for ( const auto& item : outerArchive ) {
        BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, item.index(), BitFormat::Tar );

        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );

        if ( item.name() == BIT7Z_STRING( "multiple_files.tar" ) ) {
            require_extracts_to_filesystem( innerArchive, multipleFilesContent().items );
        } else if ( item.name() == BIT7Z_STRING( "multiple_items.tar" ) ) {
            require_extracts_to_filesystem( innerArchive, multipleItemsContent().items );
        } else {
            FAIL( "Unexpected nested archive" );
        }

        REQUIRE( innerArchive.openCount() == 2 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Extracting compressed archives inside an uncompressed tarball",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "reversed_tarball.tar";
    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );

    BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::Tar );

    for ( const auto& item : outerArchive ) {
        const auto& format = [&item]() -> const BitInFormat& {
            const auto& ext = item.extension();
            if ( ext == BIT7Z_STRING( "gz" ) ) {
                return BitFormat::GZip;
            }
            if ( ext == BIT7Z_STRING( "bz2" ) ) {
                return BitFormat::BZip2;
            }
            return BitFormat::Xz;
        }();
        BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, item.index(), format );

        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );

        require_extracts_to_filesystem( innerArchive, singleFileContent().items );
        REQUIRE( innerArchive.openCount() == 2 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Usually, max memory limit should be above 4MB",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );
        REQUIRE( innerArchive.maxMemoryUsage() > ( 4ULL * 1024 * 1024 ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Extracting a deeply-nested archive",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "deeply_nested.tar";
    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );

    BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::Tar );

    for ( const auto& item : outerArchive ) {
        const auto& format = [&item]() -> const BitInFormat& {
            const auto& ext = item.extension();
            if ( ext == BIT7Z_STRING( "gz" ) ) {
                return BitFormat::GZip;
            }
            if ( ext == BIT7Z_STRING( "bz2" ) ) {
                return BitFormat::BZip2;
            }
            return BitFormat::Xz;
        }();
        BitArchiveReader tarballArchive( test::sevenzipLib(), outerArchive, item.index(), format );
        BitNestedArchiveReader innerArchive( test::sevenzipLib(), tarballArchive, BitFormat::Tar );

        REQUIRE_NOTHROW( innerArchive.test() );
        REQUIRE( innerArchive.openCount() == 1 );

        require_extracts_to_filesystem( innerArchive, multipleFilesContent().items );
        REQUIRE( innerArchive.openCount() == 2 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Extracting a multi-layered deeply-nested archive",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "deeply_nested.vdi";
    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );

    BitArchiveReader layer0( test::sevenzipLib(), inputArchive, BitFormat::VDI );
    BitArchiveReader layer1( test::sevenzipLib(), layer0, BitFormat::Mbr );
    BitArchiveReader layer2( test::sevenzipLib(), layer1, 0, BitFormat::Ext );
    BitArchiveReader layer3( test::sevenzipLib(), layer2, 1, BitFormat::Xz );
    BitNestedArchiveReader layer4( test::sevenzipLib(), layer3, BitFormat::Tar );

    REQUIRE_NOTHROW( layer4.test() );
    REQUIRE( layer4.openCount() == 1 );

    require_extracts_to_filesystem( layer4, multipleFilesContent().items );
    REQUIRE( layer4.openCount() == 2 );
}

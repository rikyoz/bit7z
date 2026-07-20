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
#include "utils/crc.hpp"
#include "utils/exception.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/biterror.hpp>
#include <bit7z/bitnestedarchivereader.hpp>
#include <bit7z/bittypes.hpp>

#include <limits>
#include <stdexcept>

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

void require_extracts_to_map( const BitNestedArchiveReader& info, const ExpectedItems& expectedItems ) {
    std::map< tstring, buffer_t > bufferMap;
    REQUIRE_NOTHROW( info.extractTo( bufferMap ) );
    REQUIRE( bufferMap.size() == expectedItems.size() );
    for ( const auto& expectedItem : expectedItems ) {
        INFO( "Failed while checking expected item '" << toUtf8String( expectedItem.inArchivePath ) << "'" )
        const auto& extractedItem = bufferMap.find( to_tstring( expectedItem.inArchivePath ) );
        REQUIRE( extractedItem != bufferMap.end() );
        REQUIRE( crc32( extractedItem->second ) == expectedItem.fileInfo.crc32 );
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
        const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );

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
        const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );
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
        const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );
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
        const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

        // TODO: Test all kind of extraction targets (buffers, streams, etc.)
        require_extracts_to_filesystem( innerArchive, multipleFilesContent().items );
        REQUIRE( innerArchive.openCount() == 1 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Extracting nested archives to a map of buffers",
    "[bitnestedarchivereader][regression]",
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
        const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

        // Regression test: for a TAR opened sequentially, itemsCount() reports UINT32_MAX,
        // so extractTo(outMap) must not pre-enumerate indices via itemsCount()/isItemFolder().
        require_extracts_to_map( innerArchive, multipleFilesContent().items );
        REQUIRE( innerArchive.openCount() == 1 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Operations after a failed itemProperty should reopen the nested archive",
    "[bitnestedarchivereader][regression]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested.tar.gz";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::GZip );
    const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

    // A far out-of-range index forces the sequential TAR handler to read past the end of the
    // (small) stream while looking for it, failing cleanly instead of finding the item.
    REQUIRE_THROWS( innerArchive.itemProperty( 999999, BitProperty::Path ) );
    REQUIRE( innerArchive.openCount() == 1 );

    // The next operation must not reuse the now-exhausted stream.
    require_extracts_to_filesystem( innerArchive, multipleFilesContent().items );
    REQUIRE( innerArchive.openCount() == 2 );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: itemProperty should reject the reserved sentinel index value",
    "[bitnestedarchivereader][regression]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested.tar.gz";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::GZip );
    const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

    // UINT32_MAX doubles as the internal "not positioned yet" sentinel; no real archive item can
    // ever legitimately sit at that index, so it must be rejected outright, without even attempting
    // to open the nested archive (which would otherwise mistake the index for "already past it" and
    // read from a never-opened underlying archive).
    REQUIRE_THROWS_CODE(
        innerArchive.itemProperty( std::numeric_limits< std::uint32_t >::max(), BitProperty::Path ),
        BitError::InvalidIndex
    );
    REQUIRE( innerArchive.openCount() == 0 );
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
        const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

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
        const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

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

    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::SevenZip );

    for ( const auto& item : outerArchive ) {
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, item.index(), BitFormat::Tar );

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

    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::Tar );

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
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, item.index(), format );

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
        const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );
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

    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::Tar );

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
        const BitArchiveReader tarballArchive( test::sevenzipLib(), outerArchive, item.index(), format );
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), tarballArchive, BitFormat::Tar );

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

    const BitArchiveReader layer0( test::sevenzipLib(), inputArchive, BitFormat::VDI );
    const BitArchiveReader layer1( test::sevenzipLib(), layer0, BitFormat::Mbr );
    const BitArchiveReader layer2( test::sevenzipLib(), layer1, 0, BitFormat::Ext );
    const BitArchiveReader layer3( test::sevenzipLib(), layer2, 1, BitFormat::Xz );
    const BitNestedArchiveReader layer4( test::sevenzipLib(), layer3, BitFormat::Tar );

    REQUIRE_NOTHROW( layer4.test() );
    REQUIRE( layer4.openCount() == 1 );

    require_extracts_to_filesystem( layer4, multipleFilesContent().items );
    REQUIRE( layer4.openCount() == 2 );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Operations after a failed extraction should reopen the nested archive",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested.tar.gz";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::GZip );
    const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

    // The user callback extracts the first item, and then throws mid-extraction,
    // leaving the sequential stream partially consumed.
    buffer_t dummyBuffer;
    std::size_t extractedCount = 0;
    REQUIRE_THROWS( innerArchive.extractTo(
        [ &dummyBuffer, &extractedCount ]( const BitArchiveItem&, const tstring& ) -> buffer_t& {
            if ( ++extractedCount > 1 ) {
                throw std::runtime_error{ "failing user callback" };
            }
            return dummyBuffer;
        },
        []( const BitArchiveItem& ) -> FilterResult {
            return FilterResult::ProcessItem;
        }
    ) );
    REQUIRE( innerArchive.openCount() == 1 );

    // The next operation must not reuse the partially consumed stream.
    require_extracts_to_filesystem( innerArchive, multipleFilesContent().items );
    REQUIRE( innerArchive.openCount() == 2 );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Extracting nested archives to a directory with a filter callback",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested.tar.gz";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::GZip );
    const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

    const TempTestDirectory outputDir{ "test_bitnestedarchivereader" };
    REQUIRE_NOTHROW( innerArchive.extractTo( outputDir, []( const BitArchiveItem& item ) -> FilterResult {
        return item.name() == italy.name ? FilterResult::ProcessItem : FilterResult::SkipItem;
    } ) );

    REQUIRE_FILESYSTEM_ITEM( ( ExpectedItem{ italy, italy.name, false } ) );
    REQUIRE_FALSE( fs::exists( fs::path{ loremIpsum.name } ) );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Extracting nested archives to a directory with a rename callback",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested.tar.gz";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::GZip );
    const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

    const TempTestDirectory outputDir{ "test_bitnestedarchivereader" };
    const ExpectedItem renamedItem{ italy, BIT7Z_NATIVE_STRING( "renamed.svg" ), false };
    REQUIRE_NOTHROW( innerArchive.extractTo( outputDir, [ &renamedItem ]( const BitArchiveItem& item ) -> tstring {
        if ( item.name() == italy.name ) {
            return to_tstring( renamedItem.inArchivePath.native() ); // Renaming italy.svg...
        }
        return {}; // ...and skipping Lorem Ipsum.pdf.
    } ) );
    REQUIRE_FILESYSTEM_ITEM( renamedItem );
    REQUIRE_FALSE( fs::exists( fs::path{ italy.name } ) );
    REQUIRE_FALSE( fs::exists( fs::path{ loremIpsum.name } ) );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Extracting nested archives to buffers with a filter callback",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested.tar.gz";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::GZip );
    const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

    std::map< tstring, buffer_t > bufferMap;
    auto bufferCallback = [ &bufferMap ]( const BitArchiveItem&, const tstring& path ) -> buffer_t& {
        return bufferMap[ path ];
    };
    REQUIRE_NOTHROW( innerArchive.extractTo(
        std::move( bufferCallback ),
        []( const BitArchiveItem& item ) -> FilterResult {
            return item.name() == italy.name ? FilterResult::ProcessItem : FilterResult::SkipItem;
        }
    ) );

    REQUIRE( bufferMap.size() == 1 );
    REQUIRE( crc32( bufferMap.begin()->second ) == italy.crc32 );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Reentrant calls into the same instance should be rejected",
    "[bitnestedarchivereader]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested.tar.gz";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::GZip );
    const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

    buffer_t dummyBuffer;
    // The filter callback calls back into the same instance while extraction is still in
    // progress, which must be rejected instead of reopening the underlying sequential stream.
    REQUIRE_THROWS_AS( innerArchive.extractTo(
        [ &dummyBuffer ]( const BitArchiveItem&, const tstring& ) -> buffer_t& {
            return dummyBuffer;
        },
        [ &innerArchive ]( const BitArchiveItem& item ) -> FilterResult {
            ( void ) innerArchive.itemProperty( item.index(), BitProperty::Path );
            return FilterResult::ProcessItem;
        }
    ), BitException );

    // The guard must be released even though the reentrant call failed,
    // so the instance remains usable for subsequent, non-reentrant operations.
    require_extracts_to_filesystem( innerArchive, multipleFilesContent().items );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: Reading items in non-decreasing index order should not reopen the nested archive",
    "[bitnestedarchivereader][regression]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested.tar.gz";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::GZip );
    const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

    REQUIRE_NOTHROW( innerArchive.itemProperty( 0, BitProperty::Path ) );
    REQUIRE( innerArchive.openCount() == 1 );

    // Reading a later index than the last one read must not force a reopen.
    REQUIRE_NOTHROW( innerArchive.itemProperty( 1, BitProperty::Path ) );
    REQUIRE( innerArchive.openCount() == 1 );

    // Re-reading the same index must not force a reopen either.
    REQUIRE_NOTHROW( innerArchive.itemProperty( 1, BitProperty::Path ) );
    REQUIRE( innerArchive.openCount() == 1 );

    // Reading an earlier index than the last one read must force a reopen.
    REQUIRE_NOTHROW( innerArchive.itemProperty( 0, BitProperty::Path ) );
    REQUIRE( innerArchive.openCount() == 2 );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: itemsCount() result is cached across calls",
    "[bitnestedarchivereader][regression]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested.tar.gz";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, BitFormat::GZip );
    const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

    REQUIRE( innerArchive.itemsCount() == multipleFilesContent().fileCount );
    REQUIRE( innerArchive.openCount() == 1 );

    // A second call must hit the cache: no recomputation, no extra reopen of the sequential stream.
    REQUIRE( innerArchive.itemsCount() == multipleFilesContent().fileCount );
    REQUIRE( innerArchive.openCount() == 1 );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitNestedArchiveReader: itemsCount() of an empty nested archive is cached across calls",
    "[bitnestedarchivereader][regression]",
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
        const fs::path arcFileName = "empty_nested.tar." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader outerArchive( test::sevenzipLib(), inputArchive, testArchive.format );
        const BitNestedArchiveReader innerArchive( test::sevenzipLib(), outerArchive, BitFormat::Tar );

        // Regression test: a legitimately empty item count must still be cached
        // (it must not be mistaken for "not cached yet" and recomputed every time).
        REQUIRE( innerArchive.itemsCount() == emptyContent().fileCount );
        REQUIRE( innerArchive.openCount() == 1 );

        // Poisons mLastReadItem via an unrelated operation, so that only a truly cached
        // itemsCount() (not a recomputed one) can avoid forcing a further reopen below.
        require_extracts_to_filesystem( innerArchive, emptyContent().items );
        REQUIRE( innerArchive.openCount() == 1 );

        REQUIRE( innerArchive.itemsCount() == emptyContent().fileCount );
        REQUIRE( innerArchive.openCount() == 1 );
    }
}

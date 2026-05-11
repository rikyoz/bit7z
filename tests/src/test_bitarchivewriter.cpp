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

#include "utils/filesystem.hpp"
#include "utils/format.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bitinputitem.hpp>
#include <bit7z/bitarchivewriter.hpp>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;
using bit7z::BitArchiveWriter;


TEST_CASE( "BitArchiveWriter: TODO", "[bitarchivewriter]" ) {
    const BitArchiveWriter writer{ test::sevenzip_lib(), BitFormat::SevenZip };
    REQUIRE( writer.compressionFormat() == BitFormat::SevenZip );
}

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE

TEST_CASE( "BitArchiveWriter: Creating an archive containing files with Unicode names", "[bitarchivewriter]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };


#if defined( BIT7Z_7ZIP_VERSION_MAJOR ) && BIT7Z_7ZIP_VERSION_MAJOR >= 24
    const auto testFormat = GENERATE( as< TestOutputFormat >(),
                                      TestOutputFormat{ "7z", BitFormat::SevenZip },
                                      TestOutputFormat{ "tar", BitFormat::Tar },
                                      TestOutputFormat{ "wim", BitFormat::Wim },
                                      TestOutputFormat{ "zip", BitFormat::Zip } );
#else
    const auto testFormat = GENERATE( as< TestOutputFormat >(),
                                      TestOutputFormat{ "7z", BitFormat::SevenZip },
                                      TestOutputFormat{ "tar", BitFormat::Tar },
                                      TestOutputFormat{ "wim", BitFormat::Wim } );
#endif



    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        constexpr auto renamedName = BIT7Z_STRING( "𤭢.svg" );

        BitArchiveWriter writer{ test::sevenzip_lib(), testFormat.format };
        REQUIRE_NOTHROW( writer.addFile( greek.name ) );
        REQUIRE_NOTHROW( writer.addFile( italy.name, renamedName ) ); // U+24B62

        buffer_t outBuffer;
        REQUIRE_NOTHROW( writer.compressTo( outBuffer ) );

        const BitArchiveReader reader{ test::sevenzip_lib(), outBuffer, testFormat.format };
        const auto greekItem = reader.find( greek.name );
        REQUIRE( greekItem != reader.cend() );
        REQUIRE( greekItem->name() == greek.name );
        REQUIRE( greekItem->path() == greek.name );

        const auto renamedItem = reader.find( renamedName );
        REQUIRE( renamedItem != reader.cend() );
        REQUIRE( renamedItem->name() == renamedName );
        REQUIRE( renamedItem->path() == renamedName );
    }
}

#endif

TEST_CASE( "BitArchiveWriter: addFile returns a reference to the added item", "[bitarchivewriter]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    SECTION( "Filesystem file" ) {
        BitArchiveWriter writer{ test::sevenzip_lib(), BitFormat::SevenZip };
        const BitInputItem& item = writer.addFile( loremIpsum.name );
        REQUIRE( !item.isDir() );
        REQUIRE( item.size() == static_cast< std::uint64_t >( loremIpsum.size ) );
    }

    SECTION( "Buffer" ) {
        const auto buffer = load_file( loremIpsum.name );
        BitArchiveWriter writer{ test::sevenzip_lib(), BitFormat::SevenZip };
        const BitInputItem& item = writer.addFile( buffer, loremIpsum.name );
        REQUIRE( !item.isDir() );
        REQUIRE( item.size() == static_cast< std::uint64_t >( loremIpsum.size ) );
    }

    SECTION( "Stream" ) {
        std::ifstream stream{ loremIpsum.name, std::ios::binary };
        BitArchiveWriter writer{ test::sevenzip_lib(), BitFormat::SevenZip };
        const BitInputItem& item = writer.addFile( stream, loremIpsum.name );
        REQUIRE( !item.isDir() );
    }
}

TEST_CASE( "BitArchiveWriter: addFile allows customizing the last write time", "[bitarchivewriter]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    // 2020-01-20 17:00:00 UTC
    const auto knownTime = time_type::clock::from_time_t( 1579539600 );

    const auto testFormat = GENERATE( as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Format " << testFormat.extension ) {
        BitArchiveWriter writer{ test::sevenzip_lib(), testFormat.format };
        auto& item = writer.addFile( italy.name );
        item.setLastWriteTime( knownTime );

        buffer_t outBuffer;
        REQUIRE_NOTHROW( writer.compressTo( outBuffer ) );

        const BitArchiveReader reader{ test::sevenzip_lib(), outBuffer, testFormat.format };
        REQUIRE( reader.itemAt( 0 ).lastWriteTime() == knownTime );
    }
}

// Only 7z completely omits MTime when tm=false; see the next test for other formats.
TEST_CASE( "BitArchiveWriter: setStoreLastWriteTime(false) suppresses MTime in the output archive",
           "[bitarchivewriter]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    // 2020-01-20 17:00:00 UTC
    const auto knownTime = time_type::clock::from_time_t( 1579539600 );

    BitArchiveWriter writer{ test::sevenzip_lib(), BitFormat::SevenZip };
    writer.setStoreLastWriteTime( false );
    auto& item = writer.addFile( italy.name );
    item.setLastWriteTime( knownTime );

    buffer_t outBuffer;
    REQUIRE_NOTHROW( writer.compressTo( outBuffer ) );

    const BitArchiveReader reader{ test::sevenzip_lib(), outBuffer, BitFormat::SevenZip };
    REQUIRE_FALSE( reader.itemAt( 0 ).itemProperty( BitProperty::MTime ).isFileTime() );
}

// ZIP/TAR/WIM always write some MTime into their fixed-offset structure (DOS time, zero unix timestamp,
// or zero FILETIME respectively), so the property is never fully absent. We verify instead that the
// user-set knownTime is not persisted: 7-Zip never retrieves it from the callback when tm=false,
// so whatever ends up in the archive is a zero/epoch-derived value, not knownTime.
// Note: WIM's SetProperties did not recognize tm/tc/ta until 7-Zip 23.01; passing them on older versions
// returns E_INVALIDARG ("The parameter is incorrect.").
TEST_CASE( "BitArchiveWriter: setStoreLastWriteTime(false) does not persist the item's MTime",
           "[bitarchivewriter]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    // 2020-01-20 17:00:00 UTC
    const auto knownTime = time_type::clock::from_time_t( 1579539600 );

#if BIT7Z_7ZIP_VERSION_MAJOR >= 23
    const auto testFormat = GENERATE( as< TestOutputFormat >(),
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );
#elif BIT7Z_7ZIP_VERSION_MAJOR > 16
    const auto testFormat = GENERATE( as< TestOutputFormat >(),
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testFormat = TestOutputFormat{ "7z", BitFormat::SevenZip };
#endif

    DYNAMIC_SECTION( "Format " << testFormat.extension ) {
        BitArchiveWriter writer{ test::sevenzip_lib(), testFormat.format };
        writer.setStoreLastWriteTime( false );
        auto& item = writer.addFile( italy.name );
        item.setLastWriteTime( knownTime );

        buffer_t outBuffer;
        REQUIRE_NOTHROW( writer.compressTo( outBuffer ) );

        const BitArchiveReader reader{ test::sevenzip_lib(), outBuffer, testFormat.format };
        REQUIRE_FALSE( reader.itemAt( 0 ).lastWriteTime() == knownTime );
    }
}

// TAR is excluded: 7-Zip's TAR writer maps creation time to the POSIX 'ctime' (change time) field,
// so it does not round-trip correctly.
TEST_CASE( "BitArchiveWriter: addFile allows customizing creation and last access times", "[bitarchivewriter]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    // 2020-01-20 17:00:00 UTC
    const auto knownTime = time_type::clock::from_time_t( 1579539600 );

#if BIT7Z_7ZIP_VERSION_MAJOR >= 24
    const auto testFormat = GENERATE( as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );
#elif BIT7Z_7ZIP_VERSION_MAJOR > 16
    const auto testFormat = GENERATE( as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testFormat = TestOutputFormat{ "7z", BitFormat::SevenZip };
#endif

    DYNAMIC_SECTION( "Format " << testFormat.extension ) {
        SECTION( "Setting the creation time" ) {
            BitArchiveWriter writer{ test::sevenzip_lib(), testFormat.format };
            writer.setStoreCreationTime( true );
            auto& item = writer.addFile( italy.name );
            item.setCreationTime( knownTime );

            buffer_t outBuffer;
            REQUIRE_NOTHROW( writer.compressTo( outBuffer ) );

            const BitArchiveReader reader{ test::sevenzip_lib(), outBuffer, testFormat.format };
            REQUIRE( reader.itemAt( 0 ).creationTime() == knownTime );
        }

        SECTION( "Setting the last access time" ) {
            BitArchiveWriter writer{ test::sevenzip_lib(), testFormat.format };
            writer.setStoreLastAccessTime( true );
            auto& item = writer.addFile( italy.name );
            item.setLastAccessTime( knownTime );

            buffer_t outBuffer;
            REQUIRE_NOTHROW( writer.compressTo( outBuffer ) );

            const BitArchiveReader reader{ test::sevenzip_lib(), outBuffer, testFormat.format };
            REQUIRE( reader.itemAt( 0 ).lastAccessTime() == knownTime );
        }
    }
}

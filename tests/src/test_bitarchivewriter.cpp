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
#include <bit7z/bitexception.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bitinputitem.hpp>
#include <bit7z/bitarchivewriter.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <system_error>
#include <vector>

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
    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim }
    );
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
        fs::ifstream stream{ loremIpsum.name, std::ios::binary };
        BitArchiveWriter writer{ test::sevenzip_lib(), BitFormat::SevenZip };
        const BitInputItem& item = writer.addFile( stream, loremIpsum.name );
        REQUIRE( !item.isDir() );
    }
}

TEST_CASE( "BitArchiveWriter: addFile allows customizing the last write time", "[bitarchivewriter]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    // 2020-01-20 17:00:00 UTC
    const auto knownTime = time_type::clock::from_time_t( 1579539600 );

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
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
TEST_CASE(
    "BitArchiveWriter: setStoreLastWriteTime(false) suppresses MTime in the output archive",
    "[bitarchivewriter]"
) {
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
TEST_CASE(
    "BitArchiveWriter: setStoreLastWriteTime(false) does not persist the item's MTime",
    "[bitarchivewriter]"
) {
    static const TestDirectory testDir{ test_filesystem_dir };

    // 2020-01-20 17:00:00 UTC
    const auto knownTime = time_type::clock::from_time_t( 1579539600 );

#if BIT7Z_7ZIP_VERSION_MAJOR >= 23
    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );
#elif BIT7Z_7ZIP_VERSION_MAJOR > 16
    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
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
    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );
#elif BIT7Z_7ZIP_VERSION_MAJOR > 16
    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
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

#if defined( _WIN32 ) && defined( BIT7Z_TESTS_DATA_DIR )

TEST_CASE(
    "BitArchiveWriter: setStoreOpenFiles allows compressing a file locked for writing by another process",
    "[bitarchivewriter]"
) {
    const TempTestDirectory testDir{ "test_bitarchivewriter" };
    const fs::path lockedFilePath = testDir.path() / "locked.txt";

    // Keep ofs open for the duration of the test: on Windows, any open write handle blocks a new
    // opener whose share mode doesn't include FILE_SHARE_WRITE (the new opener must cover the
    // existing handle's access, regardless of what share mode the existing handle itself uses).
    // This is exactly the condition that the storeOpenFiles feature is designed to work around.
    fs::ofstream ofs{ lockedFilePath, std::ios::binary };
    ofs << "hello world";
    ofs.flush(); // ensure content is on disk before bit7z reads it
    REQUIRE( ofs.is_open() );

    SECTION( "Compressing a locked file fails without storeOpenFiles" ) {
        BitArchiveWriter writer{ test::sevenzip_lib(), BitFormat::SevenZip };
        REQUIRE_FALSE( writer.storeOpenFiles() );
        REQUIRE_NOTHROW( writer.addFile( to_tstring( lockedFilePath.native() ) ) );
        buffer_t buffer;
        REQUIRE_THROWS( writer.compressTo( buffer ) );
    }

    SECTION( "Compressing a locked file succeeds with storeOpenFiles" ) {
        BitArchiveWriter writer{ test::sevenzip_lib(), BitFormat::SevenZip };
        writer.setStoreOpenFiles( true );
        REQUIRE_NOTHROW( writer.addFile( to_tstring( lockedFilePath.native() ) ) );
        buffer_t buffer;
        REQUIRE_NOTHROW( writer.compressTo( buffer ) );

        const BitArchiveReader reader{ test::sevenzip_lib(), buffer, BitFormat::SevenZip };
        REQUIRE( reader.itemsCount() == 1 );
        REQUIRE( reader.itemAt( 0 ).size() == 11 ); // "hello world" is 11 bytes
    }
}

#endif

using FilesystemItemInfoRef = std::reference_wrapper< const FilesystemItemInfo >;

TEST_CASE( "BitArchiveWriter: Using compression callbacks", "[bitarchivewriter]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip },
        TestOutputFormat{ "gz", BitFormat::GZip },
        TestOutputFormat{ "bz2", BitFormat::BZip2 },
        TestOutputFormat{ "xz", BitFormat::Xz }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const auto inputItems = testFormat.format.hasFeature( FormatFeatures::MultipleFiles )
            ? std::vector< FilesystemItemInfoRef >{ std::cref( italy ), std::cref( loremIpsum ) }
            : std::vector< FilesystemItemInfoRef >{ std::cref( loremIpsum ) };

        BitArchiveWriter writer{ test::sevenzip_lib(), testFormat.format };
        for ( const auto& itemRef : inputItems ) {
            REQUIRE_NOTHROW( writer.addFile( itemRef.get().name ) );
        }

        const auto totalInputSize = [ &inputItems ]() -> uint64_t {
            uint64_t result = 0;
            for ( const auto& itemRef : inputItems ) {
                result += itemRef.get().size;
            }
            return result;
        }();

        uint64_t totalSize = 0;
        writer.setTotalCallback(
            [ &totalSize ] ( uint64_t total ) -> void {
                totalSize = total;
            }
        );

        std::vector< uint64_t > progressValues;
        writer.setProgressCallback(
            [ &progressValues ] ( uint64_t progress ) -> bool {
                progressValues.push_back( progress );
                return true;
            }
        );

        bool ratioCalled = false;
        writer.setRatioCallback(
            [ &ratioCalled ] ( uint64_t /* input */, uint64_t /* output */ ) -> void {
                ratioCalled = true;
            }
        );

        std::vector< tstring > visitedFiles;
        writer.setFileCallback(
            [ &visitedFiles ] ( const tstring& file ) -> void {
                visitedFiles.push_back( file );
            }
        );

        buffer_t outBuffer;
        REQUIRE_NOTHROW( writer.compressTo( outBuffer ) );

        // The total callback should have reported a total that covers the input being compressed
        // (7-Zip includes some per-item/header overhead on top of the raw file sizes).
        REQUIRE( totalSize >= totalInputSize );

        // The progress callback should have been called at least once, with non-decreasing values
        // that never exceed the total size.
        REQUIRE( !progressValues.empty() );
        REQUIRE( std::is_sorted( progressValues.begin(), progressValues.end() ) );
        REQUIRE( progressValues.back() <= totalSize );

        // The ratio callback is only invoked for formats that actually compress the data.
        if ( format_compresses_files( writer.format() ) ) {
            REQUIRE( ratioCalled );
        }

        const auto expectedVisitedFiles = [ &inputItems ]() -> std::vector< tstring > {
            std::vector< tstring > result;
            result.reserve( inputItems.size() );
            for ( const auto& itemRef : inputItems ) {
                result.emplace_back( itemRef.get().name );
            }
            return result;
        }();

        // The file callback should have reported each compressed file.
        using namespace Catch::Matchers;
        REQUIRE_THAT( visitedFiles, Catch::UnorderedEquals( expectedVisitedFiles ) );
    }
}

TEST_CASE( "BitArchiveWriter: Aborting the compression via the progress callback", "[bitarchivewriter]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip },
        TestOutputFormat{ "gz", BitFormat::GZip },
        TestOutputFormat{ "bz2", BitFormat::BZip2 },
        TestOutputFormat{ "xz", BitFormat::Xz }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        BitArchiveWriter writer{ test::sevenzip_lib(), testFormat.format };
        REQUIRE_NOTHROW( writer.addFile( italy.name ) );
        if ( testFormat.format.hasFeature( FormatFeatures::MultipleFiles ) ) {
            REQUIRE_NOTHROW( writer.addFile( loremIpsum.name ) );
        }

        // Returning false from the progress callback must abort the ongoing operation.
        bool progressCalled = false;
        writer.setProgressCallback(
            [ &progressCalled ] ( std::uint64_t ) -> bool {
                progressCalled = true;
                return false;
            }
        );

        buffer_t outBuffer;
        REQUIRE_THROWS_MATCHES(
            writer.compressTo( outBuffer ),
            BitException,
            Catch::Matchers::Predicate< BitException >( []( const BitException& exception ) -> bool {
                return exception.code() == std::errc::operation_canceled;
                }, "Error code should be operation_canceled" )
        );

        // The operation must have been aborted from within the progress callback,
        // and not have failed for some other reason.
        REQUIRE( progressCalled );
    }
}

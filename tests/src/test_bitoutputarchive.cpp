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
#include <bit7z/bitexception.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchivewriter.hpp>

#include <cstdint>
#include <map>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

// Note: in the following tests, we use BitArchiveReader for checking BitArchiveWriter's output archives.

TEST_CASE( "BitOutputArchive: Creating a multi-volume archive", "[bitoutputarchive]" ) {
    const auto inputFile = fs::path{ test_filesystem_dir } / "folder" / "clouds.jpg";

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "bz2", BitFormat::BZip2 },
        TestOutputFormat{ "gz", BitFormat::GZip },
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "xz", BitFormat::Xz },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        BitArchiveWriter writer( test::sevenzipLib(), testFormat.format );
        REQUIRE_NOTHROW( writer.addFile( to_tstring( inputFile ) ) );

        const TempTestDirectory testOutDir{ "test_bitoutputarchive" };
        INFO( "Output directory: " << testOutDir )

        const tstring outputArchive = BIT7Z_STRING( "cloud.7z" );
        REQUIRE_NOTHROW( writer.setVolumeSize( 1024 ) );
        REQUIRE_NOTHROW( writer.compressTo( outputArchive ) );

        REQUIRE_FALSE( fs::exists( outputArchive ) );

        const tstring firstVolume = outputArchive + BIT7Z_STRING( ".001" );
        REQUIRE( fs::exists( outputArchive + BIT7Z_STRING( ".001" ) ) );

        {
            const BitArchiveReader info{ test::sevenzipLib(), firstVolume, testFormat.format };
            REQUIRE_NOTHROW( info.test() );

            buffer_t fileBuffer;
            REQUIRE_NOTHROW( info.extractTo( fileBuffer ) );
            REQUIRE( crc32( fileBuffer ) == clouds.crc32 );
        }

        for ( const auto& volume : fs::directory_iterator{ testOutDir.path() } ) {
            fs::remove( volume );
        }
    }
}

TEST_CASE(
    "BitOutputArchive: Reading an existing archive from memory or a stream preserves its items",
    "[bitoutputarchive]"
) {
    const auto inputArchive = fs::path{ test_archives_dir } / "extraction" / "multiple_items" / "multiple_items.7z";

    std::uint32_t expectedItemsCount = 0;
    {
        const BitArchiveReader reader{ test::sevenzipLib(), to_tstring( inputArchive ), BitFormat::SevenZip };
        expectedItemsCount = reader.itemsCount();
    }

    SECTION( "From a buffer" ) {
        const auto inputBuffer = loadFile( inputArchive );
        BitArchiveWriter writer{ test::sevenzipLib(), inputBuffer, BitFormat::SevenZip };
        REQUIRE( writer.itemsCount() == expectedItemsCount );

        buffer_t outputBuffer;
        REQUIRE_NOTHROW( writer.compressTo( outputBuffer ) );

        const BitArchiveReader result{ test::sevenzipLib(), outputBuffer, BitFormat::SevenZip };
        REQUIRE( result.itemsCount() == expectedItemsCount );
    }

    SECTION( "From a stream" ) {
        fs::ifstream inputStream{ inputArchive, std::ios::binary };
        BitArchiveWriter writer{ test::sevenzipLib(), inputStream, BitFormat::SevenZip };
        REQUIRE( writer.itemsCount() == expectedItemsCount );

        buffer_t outputBuffer;
        REQUIRE_NOTHROW( writer.compressTo( outputBuffer ) );

        const BitArchiveReader result{ test::sevenzipLib(), outputBuffer, BitFormat::SevenZip };
        REQUIRE( result.itemsCount() == expectedItemsCount );
    }
}

TEST_CASE( "BitOutputArchive: Updating a single-file-format archive is not supported", "[bitoutputarchive]" ) {
    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "bz2", BitFormat::BZip2 },
        TestOutputFormat{ "gz", BitFormat::GZip },
        TestOutputFormat{ "xz", BitFormat::Xz }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const auto inputArchive =
            fs::path{ test_archives_dir } / "extraction" / "single_file" / ( "clouds.jpg." + testFormat.extension );

        // The default update mode of BitArchiveWriter is UpdateMode::Append, so opening an existing archive of a
        // format that does not support multiple files must fail.
        REQUIRE_THROWS_CODE(
            BitArchiveWriter( test::sevenzipLib(), to_tstring( inputArchive ), testFormat.format ),
            BitError::FormatFeatureNotSupported
        );
    }
}

TEST_CASE( "BitOutputArchive: Updating an archive overwrites items with matching names", "[bitoutputarchive]" ) {
    const auto inputArchive = fs::path{ test_archives_dir } / "extraction" / "multiple_items" / "multiple_items.7z";

    std::uint32_t originalItemsCount = 0;
    {
        const BitArchiveReader reader{ test::sevenzipLib(), to_tstring( inputArchive ), BitFormat::SevenZip };
        originalItemsCount = reader.itemsCount();
    }

    const auto newContent = loadFile( fs::path{ test_filesystem_dir } / "folder" / "clouds.jpg" );

    BitArchiveWriter writer{ test::sevenzipLib(), to_tstring( inputArchive ), BitFormat::SevenZip };
    writer.setUpdateMode( UpdateMode::Update );
    // "italy.svg" matches an existing root item, so it should replace it (i.e., delete the old one and add the new).
    REQUIRE_NOTHROW( writer.addFile( newContent, BIT7Z_STRING( "italy.svg" ) ) );
    // "brand_new.dat" does not match any existing item, so it should simply be added.
    REQUIRE_NOTHROW( writer.addFile( newContent, BIT7Z_STRING( "brand_new.dat" ) ) );

    buffer_t outputBuffer;
    REQUIRE_NOTHROW( writer.compressTo( outputBuffer ) );

    const BitArchiveReader result{ test::sevenzipLib(), outputBuffer, BitFormat::SevenZip };
    // The matching item was replaced (not duplicated), while the non-matching one was added.
    REQUIRE( result.itemsCount() == originalItemsCount + 1 );

    std::map< tstring, buffer_t > extractedItems;
    REQUIRE_NOTHROW( result.extractTo( extractedItems ) );
    REQUIRE( extractedItems[ BIT7Z_STRING( "italy.svg" ) ] == newContent );
    REQUIRE( extractedItems.count( BIT7Z_STRING( "brand_new.dat" ) ) == 1 );
}

TEST_CASE( "BitOutputArchive: Compressing to a path without a filename should throw", "[bitoutputarchive]" ) {
    BitArchiveWriter writer{ test::sevenzipLib(), BitFormat::SevenZip };
    REQUIRE_THROWS_CODE(
        writer.compressTo( BIT7Z_STRING( "directory/" ) ),
        BitError::InvalidArchivePath
    );
}

TEST_CASE(
    "BitOutputArchive: With OverwriteMode::Skip, compressing to an existing file leaves it unchanged",
    "[bitoutputarchive]"
) {
    const TempTestDirectory testOutDir{ "test_bitoutputarchive" };
    INFO( "Output directory: " << testOutDir )

    const auto contentFile = fs::path{ test_filesystem_dir } / "folder" / "clouds.jpg";

    const tstring outputArchive = BIT7Z_STRING( "existing.7z" );
    REQUIRE_NOTHROW( fs::copy_file( contentFile, fs::path{ outputArchive } ) );
    const auto originalContent = loadFile( fs::path{ outputArchive } );

    // The buffer must outlive the writer, as addFile only keeps a reference to it.
    const auto fileContent = loadFile( contentFile );

    BitArchiveWriter writer{ test::sevenzipLib(), BitFormat::SevenZip };
    writer.addFile( fileContent, BIT7Z_STRING( "clouds.jpg" ) );
    writer.setOverwriteMode( OverwriteMode::Skip );
    REQUIRE_NOTHROW( writer.compressTo( outputArchive ) );

    // The pre-existing file must still hold its original (non-archive) content.
    REQUIRE( loadFile( fs::path{ outputArchive } ) == originalContent );

    REQUIRE_NOTHROW( fs::remove( fs::path{ outputArchive } ) );
}

TEST_CASE( "BitOutputArchive: Compressing to a non-empty buffer respects the overwrite mode", "[bitoutputarchive]" ) {
    const auto contentFile = fs::path{ test_filesystem_dir } / "folder" / "clouds.jpg";

    // The buffer must outlive the writer, as addFile only keeps a reference to it.
    const auto fileContent = loadFile( contentFile );

    BitArchiveWriter writer{ test::sevenzipLib(), BitFormat::SevenZip };
    writer.addFile( fileContent, BIT7Z_STRING( "clouds.jpg" ) );

    SECTION( "OverwriteMode::None (default) throws" ) {
        buffer_t outputBuffer( 8 );
        REQUIRE_THROWS_CODE( writer.compressTo( outputBuffer ), BitError::NonEmptyOutputBuffer );
    }

    SECTION( "OverwriteMode::Skip leaves the buffer unchanged" ) {
        const buffer_t originalBuffer( 8 );
        buffer_t outputBuffer = originalBuffer;
        writer.setOverwriteMode( OverwriteMode::Skip );
        REQUIRE_NOTHROW( writer.compressTo( outputBuffer ) );
        REQUIRE( outputBuffer == originalBuffer );
    }

    SECTION( "OverwriteMode::Overwrite replaces the buffer with a new archive" ) {
        buffer_t outputBuffer( 8 );
        writer.setOverwriteMode( OverwriteMode::Overwrite );
        REQUIRE_NOTHROW( writer.compressTo( outputBuffer ) );

        const BitArchiveReader result{ test::sevenzipLib(), outputBuffer, BitFormat::SevenZip };
        REQUIRE( result.itemsCount() == 1 );
    }
}

#ifdef _WIN32
// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE( "BitOutputArchive: Compressing a commented file should preserve the comment", "[bitoutputarchive]" ) {
    const auto commentedFile = fs::path{ test_archives_dir } / "metadata" / "file_comment" / "commented.jpg";

    BitArchiveWriter writer( test::sevenzipLib(), BitFormat::SevenZip );
    REQUIRE_NOTHROW( writer.addFile( to_tstring( commentedFile ) ) );

    const TempTestDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Output directory: " << testOutDir )

    const auto* const outputArchive = BIT7Z_STRING( "commented.7z" );
    REQUIRE_NOTHROW( writer.compressTo( outputArchive ) );

    REQUIRE( fs::exists( outputArchive ) );

    const BitArchiveReader info( test::sevenzipLib(), outputArchive, BitFormat::SevenZip );
    REQUIRE_NOTHROW( info.test() );
    REQUIRE_NOTHROW( info.extractTo( testOutDir ) );

    const auto expectedFile = testOutDir.path() / "commented.jpg";
    REQUIRE( fs::exists( expectedFile ) );

    const std::wstring comment = getFileComment( expectedFile );
    REQUIRE(
        comment == LR"({"data":{"pictureId":"738298be446d47f4b3933a4cc68ab6a2","appversion":"8.0.0",)"
        LR"("stickerId":"","filterId":"","infoStickerId":"","imageEffectId":"",)"
        LR"("playId":"","activityName":"","os":"android","product":"retouch"},)"
        LR"("source_type":"douyin_beauty_me"})"
    );
    REQUIRE( fs::remove( expectedFile ) );
}
#endif

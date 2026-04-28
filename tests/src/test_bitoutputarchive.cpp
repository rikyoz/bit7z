// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include "utils/archive.hpp"
#include "utils/crc.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/bitformat.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchivewriter.hpp>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

// Note: in the following tests, we use BitArchiveReader for checking BitArchiveWriter's output archives.

TEST_CASE( "BitOutputArchive: TODO", "[bitoutputarchive]" ) {
    const BitArchiveWriter writer{ test::sevenzip_lib(), BitFormat::SevenZip };
    REQUIRE( writer.compressionFormat() == BitFormat::SevenZip ); // Just a placeholder test.
}

TEST_CASE( "BitOutputArchive: Creating a multi-volume archive", "[bitoutputarchive]" ) {
    const auto inputFile = fs::path{ test_filesystem_dir } / "folder" / "clouds.jpg";

    const auto testFormat = GENERATE( as< TestOutputFormat >(),
        TestOutputFormat{ "bz2", BitFormat::BZip2 },
        TestOutputFormat{ "gz", BitFormat::GZip },
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "xz", BitFormat::Xz },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        BitArchiveWriter writer( test::sevenzip_lib(), testFormat.format );
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
            const BitArchiveReader info{ test::sevenzip_lib(), firstVolume, testFormat.format };
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

#ifdef _WIN32
// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE( "BitOutputArchive: Compressing a commented file should preserve the comment", "[bitoutputarchive]" ) {
    const auto commentedFile = fs::path{ test_archives_dir } / "metadata" / "file_comment" / "commented.jpg";

    BitArchiveWriter writer( test::sevenzip_lib(), BitFormat::SevenZip );
    REQUIRE_NOTHROW( writer.addFile( to_tstring( commentedFile ) ) );

    TempTestDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Output directory: " << testOutDir )

    const auto* const outputArchive = BIT7Z_STRING( "commented.7z" );
    REQUIRE_NOTHROW( writer.compressTo( outputArchive ) );

    REQUIRE( fs::exists( outputArchive ) );

    BitArchiveReader info( test::sevenzip_lib(), outputArchive, BitFormat::SevenZip );
    REQUIRE_NOTHROW( info.test() );
    REQUIRE_NOTHROW( info.extractTo( testOutDir ) );

    const auto expectedFile = testOutDir.path() / "commented.jpg";
    REQUIRE( fs::exists( expectedFile ) );

    std::wstring comment = get_file_comment( expectedFile );
    REQUIRE( comment == LR"({"data":{"pictureId":"738298be446d47f4b3933a4cc68ab6a2","appversion":"8.0.0",)"
                        LR"("stickerId":"","filterId":"","infoStickerId":"","imageEffectId":"",)"
                        LR"("playId":"","activityName":"","os":"android","product":"retouch"},)"
                        LR"("source_type":"douyin_beauty_me"})" );
    REQUIRE( fs::remove( expectedFile ) );
}
#endif
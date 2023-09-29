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
#include <catch2/catch.hpp>

#include <bit7z/bitfilecompressor.hpp>
#include <bit7z/bitformat.hpp>

#include "utils/shared_lib.hpp"

using namespace bit7z;
using bit7z::Bit7zLibrary;
using bit7z::BitFileCompressor;
using bit7z::BitInOutFormat;

struct TestOutputFormat {
    const char* name;
    const BitInOutFormat& format; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

TEST_CASE( "BitFileCompressor: Basic API tests (BitArchiveCreator API)", "[bitfilecompressor]" ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    SECTION( "setPassword(...) / password() / cryptHeaders()" ) {
        BitFileCompressor compressor( lib, BitFormat::SevenZip );
        REQUIRE( compressor.password().empty() );
        REQUIRE( !compressor.cryptHeaders() );

        compressor.setPassword( BIT7Z_STRING( "ciao" ) );
        REQUIRE( compressor.password() == BIT7Z_STRING( "ciao" ) );
        REQUIRE( !compressor.cryptHeaders() );

        compressor.setPassword( BIT7Z_STRING( "mondo" ), true );
        REQUIRE( compressor.password() == BIT7Z_STRING( "mondo" ) );
        REQUIRE( compressor.cryptHeaders() );

        compressor.setPassword( BIT7Z_STRING( "hello" ) );
        REQUIRE( compressor.password() == BIT7Z_STRING( "hello" ) );
        REQUIRE( compressor.cryptHeaders() );

        compressor.setPassword( BIT7Z_STRING( "" ) );
        REQUIRE( compressor.password().empty() );
        REQUIRE( !compressor.cryptHeaders() );

        BitAbstractArchiveHandler& handler = compressor;
        handler.setPassword( BIT7Z_STRING( "world!" ) );
        REQUIRE( compressor.password() == BIT7Z_STRING( "world!" ) );
        REQUIRE( !compressor.cryptHeaders() );

        compressor.setPassword( BIT7Z_STRING( "foo" ), true );
        handler.setPassword( BIT7Z_STRING( "" ) );
        REQUIRE( compressor.password().empty() );
        REQUIRE( !compressor.cryptHeaders() );
    }

    SECTION( "compressionFormat()" ) {
        const auto testFormat = GENERATE( as< TestOutputFormat >(),
                                          TestOutputFormat{ "ZIP", BitFormat::Zip },
                                          TestOutputFormat{ "BZIP2", BitFormat::BZip2 },
                                          TestOutputFormat{ "7Z", BitFormat::SevenZip },
                                          TestOutputFormat{ "XZ", BitFormat::Xz },
                                          TestOutputFormat{ "WIM", BitFormat::Wim },
                                          TestOutputFormat{ "TAR", BitFormat::Tar },
                                          TestOutputFormat{ "GZIP", BitFormat::GZip } );
        DYNAMIC_SECTION( "Format: " << testFormat.name ) {
            const BitFileCompressor compressor{ lib, testFormat.format };
            REQUIRE( compressor.compressionFormat() == testFormat.format );
        }
    }

    SECTION( "setCompressionLevel(...) / compressionLevel()" ) {
        BitFileCompressor compressor( lib, BitFormat::SevenZip );
        REQUIRE( compressor.compressionLevel() == BitCompressionLevel::Normal );
        compressor.setCompressionLevel( BitCompressionLevel::None );
        REQUIRE( compressor.compressionLevel() == BitCompressionLevel::None );
        compressor.setCompressionLevel( BitCompressionLevel::Fastest );
        REQUIRE( compressor.compressionLevel() == BitCompressionLevel::Fastest );
        compressor.setCompressionLevel( BitCompressionLevel::Fast );
        REQUIRE( compressor.compressionLevel() == BitCompressionLevel::Fast );
        compressor.setCompressionLevel( BitCompressionLevel::Normal );
        REQUIRE( compressor.compressionLevel() == BitCompressionLevel::Normal );
        compressor.setCompressionLevel( BitCompressionLevel::Max );
        REQUIRE( compressor.compressionLevel() == BitCompressionLevel::Max );
        compressor.setCompressionLevel( BitCompressionLevel::Ultra );
        REQUIRE( compressor.compressionLevel() == BitCompressionLevel::Ultra );
    }

    SECTION( "setCompressionMethod(...) / compressionMethod()" ) {
        SECTION( "7Z Compression Methods" ) {
            BitFileCompressor compressor( lib, BitFormat::SevenZip );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Lzma2 );
            compressor.setCompressionMethod( BitCompressionMethod::Copy );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Copy );
            compressor.setCompressionMethod( BitCompressionMethod::Lzma );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Lzma );
            compressor.setCompressionMethod( BitCompressionMethod::Lzma2 );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Lzma2 );
            compressor.setCompressionMethod( BitCompressionMethod::Ppmd );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Ppmd );
            compressor.setCompressionMethod( BitCompressionMethod::BZip2 );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::BZip2 );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate64 ) );
        }

        SECTION( "ZIP Compression Methods" ) {
            BitFileCompressor compressor( lib, BitFormat::Zip );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Deflate );
            compressor.setCompressionMethod( BitCompressionMethod::Copy );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Copy );
            compressor.setCompressionMethod( BitCompressionMethod::Deflate );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Deflate );
            compressor.setCompressionMethod( BitCompressionMethod::Deflate64 );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Deflate64 );
            compressor.setCompressionMethod( BitCompressionMethod::BZip2 );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::BZip2 );
            compressor.setCompressionMethod( BitCompressionMethod::Lzma );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Lzma );
            compressor.setCompressionMethod( BitCompressionMethod::Ppmd );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Ppmd );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma2 ) );
        }

        SECTION( "BZIP2 Compression Methods" ) {
            BitFileCompressor compressor( lib, BitFormat::BZip2 );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::BZip2 );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Copy ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma2 ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Ppmd ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate64 ) );
            REQUIRE_NOTHROW( compressor.setCompressionMethod( BitCompressionMethod::BZip2 ) );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::BZip2 );
        }

        SECTION( "GZIP Compression Methods" ) {
            BitFileCompressor compressor( lib, BitFormat::GZip );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Deflate );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Copy ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma2 ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Ppmd ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::BZip2 ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate64 ) );
            REQUIRE_NOTHROW( compressor.setCompressionMethod( BitCompressionMethod::Deflate ) );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Deflate );
        }

        SECTION( "TAR Compression Methods" ) {
            BitFileCompressor compressor( lib, BitFormat::Tar );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Copy );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma2 ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Ppmd ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::BZip2 ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate64 ) );
            REQUIRE_NOTHROW( compressor.setCompressionMethod( BitCompressionMethod::Copy ) );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Copy );
        }

        SECTION( "WIM Compression Methods" ) {
            BitFileCompressor compressor( lib, BitFormat::Wim );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Copy );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma2 ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Ppmd ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::BZip2 ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate64 ) );
            REQUIRE_NOTHROW( compressor.setCompressionMethod( BitCompressionMethod::Copy ) );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Copy );
        }

        SECTION( "XZ Compression Methods" ) {
            BitFileCompressor compressor( lib, BitFormat::Xz );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Lzma2 );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Copy ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Lzma ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Ppmd ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::BZip2 ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate ) );
            REQUIRE_THROWS( compressor.setCompressionMethod( BitCompressionMethod::Deflate64 ) );
            REQUIRE_NOTHROW( compressor.setCompressionMethod( BitCompressionMethod::Lzma2 ) );
            REQUIRE( compressor.compressionMethod() == BitCompressionMethod::Lzma2 );
        }
    }

    SECTION( "setSolidMode(...) / solidMode()" ) {
        BitFileCompressor compressor( lib, BitFormat::SevenZip );
        REQUIRE( !compressor.solidMode() );

        compressor.setSolidMode( true );
        REQUIRE( compressor.solidMode() );

        compressor.setSolidMode( false );
        REQUIRE( !compressor.solidMode() );
    }

    SECTION( "setVolumeSize(...) / volumeSize()" ) {
        BitFileCompressor compressor( lib, BitFormat::SevenZip );
        REQUIRE( compressor.volumeSize() == 0u );
        compressor.setVolumeSize( 1024u );
        REQUIRE( compressor.volumeSize() == 1024u );
    }
}


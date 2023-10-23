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

#include <bit7z/bitarchivewriter.hpp>
#include <bit7z/bitfilecompressor.hpp>
#include <bit7z/bitmemcompressor.hpp>
#include <bit7z/bitstreamcompressor.hpp>

#include "utils/shared_lib.hpp"

using namespace bit7z;
using bit7z::Bit7zLibrary;
using bit7z::BitArchiveWriter;
using bit7z::BitFileCompressor;
using bit7z::BitMemCompressor;
using bit7z::BitStreamCompressor;
using bit7z::BitInOutFormat;

struct TestOutputFormat {
    const char* name;
    const BitInOutFormat& format; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

using CreatorTypes = std::tuple< BitArchiveWriter, BitFileCompressor, BitMemCompressor, BitStreamCompressor >;

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setPassword(...) / password() / cryptHeaders()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    TestType compressor( lib, BitFormat::SevenZip );
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

#ifndef BIT7Z_DISABLE_ZIP_ASCII_PWD_CHECK
TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator:"
                         "setPassword(...) with a non-ASCII string should throw when using the ZIP format",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    TestType compressor{ lib, BitFormat::Zip };
    REQUIRE( compressor.password().empty() );
    REQUIRE( !compressor.cryptHeaders() );

    REQUIRE_NOTHROW( compressor.setPassword( BIT7Z_STRING( "password" ) ) );
    REQUIRE( compressor.password() == BIT7Z_STRING( "password" ) );
    REQUIRE( !compressor.cryptHeaders() );

    REQUIRE_THROWS( compressor.setPassword( BIT7Z_STRING( "contraseña" ) ) );
    REQUIRE( compressor.password() == BIT7Z_STRING( "password" ) );
    REQUIRE( !compressor.cryptHeaders() );

    // The carriage return is an ASCII character, but 7-zip doesn't support
    // also non-printable ASCII characters for ZIP passwords.
    REQUIRE_THROWS( compressor.setPassword( BIT7Z_STRING( "car\riage" ) ) );
    REQUIRE( compressor.password() == BIT7Z_STRING( "password" ) );
    REQUIRE( !compressor.cryptHeaders() );

    // The unit separator character is the last non-printable ASCII character.
    REQUIRE_THROWS( compressor.setPassword( BIT7Z_STRING( "unit\x1Fseparator" ) ) );
    REQUIRE( compressor.password() == BIT7Z_STRING( "password" ) );
    REQUIRE( !compressor.cryptHeaders() );

    // The DEL character is not supported by bit7z for ZIP passwords.
    REQUIRE_THROWS( compressor.setPassword( BIT7Z_STRING( "del\U0000007F" ) ) );
    REQUIRE( compressor.password() == BIT7Z_STRING( "password" ) );
    REQUIRE( !compressor.cryptHeaders() );

    // The space character is the first printable ASCII character supported.
    REQUIRE_NOTHROW( compressor.setPassword( BIT7Z_STRING( "password with spaces" ) ) );
    REQUIRE( compressor.password() == BIT7Z_STRING( "password with spaces" ) );
    REQUIRE( !compressor.cryptHeaders() );

    // The tilde character is the last printable ASCII character supported.
    REQUIRE_NOTHROW( compressor.setPassword( BIT7Z_STRING( "password~with~tilde" ) ) );
    REQUIRE( compressor.password() == BIT7Z_STRING( "password~with~tilde" ) );
    REQUIRE( !compressor.cryptHeaders() );

    // Resetting the password
    REQUIRE_NOTHROW( compressor.setPassword( BIT7Z_STRING( "" ) ) );
    REQUIRE( compressor.password().empty() );
    REQUIRE( !compressor.cryptHeaders() );
}
#endif

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: format() / compressionFormat()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };
    const auto testFormat = GENERATE( as< TestOutputFormat >(),
                                      TestOutputFormat{ "ZIP", BitFormat::Zip },
                                      TestOutputFormat{ "BZIP2", BitFormat::BZip2 },
                                      TestOutputFormat{ "7Z", BitFormat::SevenZip },
                                      TestOutputFormat{ "XZ", BitFormat::Xz },
                                      TestOutputFormat{ "WIM", BitFormat::Wim },
                                      TestOutputFormat{ "TAR", BitFormat::Tar },
                                      TestOutputFormat{ "GZIP", BitFormat::GZip } );
    DYNAMIC_SECTION( "Format: " << testFormat.name ) {
        const TestType compressor{ lib, testFormat.format };
        REQUIRE( compressor.compressionFormat() == testFormat.format );
        REQUIRE( compressor.format() == testFormat.format );
    }
}

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setCompressionLevel(...) / compressionLevel()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    TestType compressor( lib, BitFormat::SevenZip );
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

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setCompressionMethod(...) / compressionMethod()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    SECTION( "7Z Compression Methods" ) {
        TestType compressor( lib, BitFormat::SevenZip );
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
        TestType compressor( lib, BitFormat::Zip );
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
        TestType compressor( lib, BitFormat::BZip2 );
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
        TestType compressor( lib, BitFormat::GZip );
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
        TestType compressor( lib, BitFormat::Tar );
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
        TestType compressor( lib, BitFormat::Wim );
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
        TestType compressor( lib, BitFormat::Xz );
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

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setDictionarySize(...) / dictionarySize()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    SECTION( "SevenZip format + Lzma/Lzma2 compression methods" ) {
        constexpr auto kMaxLzmaDictionarySize = 1536 * ( 1LL << 20 ); // less than 1536 MiB

        TestType compressor( lib, BitFormat::SevenZip );
        REQUIRE( compressor.dictionarySize() == 0 );

        auto testMethod = GENERATE( BitCompressionMethod::Lzma, BitCompressionMethod::Lzma2 );
        compressor.setCompressionMethod( testMethod );

        uint32_t dictionarySize = 1024 * 1024 * 1024;
        compressor.setDictionarySize( dictionarySize );
        REQUIRE( compressor.dictionarySize() == dictionarySize );

        compressor.setDictionarySize( kMaxLzmaDictionarySize );
        REQUIRE( compressor.dictionarySize() == kMaxLzmaDictionarySize );

        dictionarySize = std::numeric_limits< uint32_t >::max();
        REQUIRE_THROWS( compressor.setDictionarySize( std::numeric_limits< uint32_t >::max() ) );
        REQUIRE( compressor.dictionarySize() == kMaxLzmaDictionarySize );
    }

    SECTION( "Zip format + Ppmd compression methods" ) {
        constexpr uint32_t kMaxPpmdDictionarySize = ( 1ULL << 30 ); // less than 1 GiB, i.e., 2^30 bytes

        TestType compressor( lib, BitFormat::Zip );
        REQUIRE( compressor.dictionarySize() == 0 );
        compressor.setCompressionMethod( BitCompressionMethod::Ppmd );

        uint32_t dictionarySize = 1024 * 1024 * 1024;
        compressor.setDictionarySize( dictionarySize );
        REQUIRE( compressor.dictionarySize() == dictionarySize );

        compressor.setDictionarySize( kMaxPpmdDictionarySize );
        REQUIRE( compressor.dictionarySize() == kMaxPpmdDictionarySize );

        dictionarySize = std::numeric_limits< uint32_t >::max();
        REQUIRE_THROWS( compressor.setDictionarySize( std::numeric_limits< uint32_t >::max() ) );
        REQUIRE( compressor.dictionarySize() == kMaxPpmdDictionarySize );

        auto testMethod = GENERATE( BitCompressionMethod::Copy,
                                    BitCompressionMethod::Deflate,
                                    BitCompressionMethod::Deflate64 );
        compressor.setCompressionMethod( testMethod );
        REQUIRE_NOTHROW( compressor.setDictionarySize( 1024 * 1024 ) );
        REQUIRE( compressor.dictionarySize() == 0 );
    }

    SECTION( "BZip2 format and compression methods" ) {
        constexpr auto kMaxBzip2DictionarySize = 900 * ( 1LL << 10 ); // less than 900 KiB

        TestType compressor( lib, BitFormat::BZip2 );
        REQUIRE( compressor.dictionarySize() == 0 );

        uint32_t dictionarySize = 1024;
        compressor.setDictionarySize( dictionarySize );
        REQUIRE( compressor.dictionarySize() == dictionarySize );

        compressor.setDictionarySize( kMaxBzip2DictionarySize );
        REQUIRE( compressor.dictionarySize() == kMaxBzip2DictionarySize );

        dictionarySize = std::numeric_limits< uint32_t >::max();
        REQUIRE_THROWS( compressor.setDictionarySize( std::numeric_limits< uint32_t >::max() ) );
        REQUIRE( compressor.dictionarySize() == kMaxBzip2DictionarySize );
    }
}

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setSolidMode(...) / solidMode()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    TestType compressor( lib, BitFormat::SevenZip );
    REQUIRE( !compressor.solidMode() );

    compressor.setSolidMode( true );
    REQUIRE( compressor.solidMode() );

    compressor.setSolidMode( false );
    REQUIRE( !compressor.solidMode() );
}

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setStoreSymbolicLinks(...) / storeSymbolicLinks()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    TestType compressor( lib, BitFormat::SevenZip );
    REQUIRE_FALSE( compressor.storeSymbolicLinks() );

    compressor.setStoreSymbolicLinks( true );
    REQUIRE( compressor.storeSymbolicLinks() );
    REQUIRE( compressor.solidMode() );

    compressor.setStoreSymbolicLinks( false );
    REQUIRE_FALSE( compressor.storeSymbolicLinks() );
    REQUIRE_FALSE( compressor.solidMode() );
}

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setThreadCount(...) / threadCount()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    TestType compressor( lib, BitFormat::SevenZip );
    REQUIRE( compressor.threadsCount() == 0u );
    compressor.setThreadsCount( 8u );
    REQUIRE( compressor.threadsCount() == 8u );
}

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setUpdateMode(...) / updateMode()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    TestType compressor( lib, BitFormat::SevenZip );
    REQUIRE( compressor.updateMode() == UpdateMode::None );

    compressor.setUpdateMode( true );
    REQUIRE( compressor.updateMode() == UpdateMode::Append );

    compressor.setUpdateMode( false );
    REQUIRE( compressor.updateMode() == UpdateMode::None );

    compressor.setUpdateMode( UpdateMode::Append );
    REQUIRE( compressor.updateMode() == UpdateMode::Append );

    compressor.setUpdateMode( UpdateMode::Update );
    REQUIRE( compressor.updateMode() == UpdateMode::Update );

    compressor.setUpdateMode( UpdateMode::None );
    REQUIRE( compressor.updateMode() == UpdateMode::None );

}

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setVolumeSize(...) / volumeSize()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    TestType compressor( lib, BitFormat::SevenZip );
    REQUIRE( compressor.volumeSize() == 0u );
    compressor.setVolumeSize( 1024u );
    REQUIRE( compressor.volumeSize() == 1024u );
}

TEMPLATE_LIST_TEST_CASE( "BitAbstractArchiveCreator: setWordSize(...) / wordSize()",
                         "[bitabstractarchivecreator]", CreatorTypes ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    constexpr auto kMinPpmdWordSize = 2u;

    SECTION( "SevenZip format + Lzma/Lzma2 compression methods" ) {
        constexpr auto kMinLzmaWordSize = 5u;
        constexpr auto kMaxLzmaWordSize = 273u;

        TestType compressor( lib, BitFormat::SevenZip );
        REQUIRE_THROWS( compressor.setWordSize( 4u ) );
        REQUIRE( compressor.wordSize() == 0 );

        REQUIRE_NOTHROW( compressor.setWordSize( kMinLzmaWordSize ) );
        REQUIRE( compressor.wordSize() == kMinLzmaWordSize );

        REQUIRE_NOTHROW( compressor.setWordSize( 128u ) );
        REQUIRE( compressor.wordSize() == 128u );

        REQUIRE_NOTHROW( compressor.setWordSize( kMaxLzmaWordSize ) );
        REQUIRE( compressor.wordSize() == kMaxLzmaWordSize );

        REQUIRE_THROWS( compressor.setWordSize( kMaxLzmaWordSize + 1 ) );
        REQUIRE( compressor.wordSize() == kMaxLzmaWordSize );

        REQUIRE_THROWS( compressor.setWordSize( 512u ) );
        REQUIRE( compressor.wordSize() == kMaxLzmaWordSize );

        REQUIRE_NOTHROW( compressor.setWordSize( 0 ) );
        REQUIRE( compressor.wordSize() == 0 );

        REQUIRE_NOTHROW( compressor.setCompressionMethod( BitCompressionMethod::Copy ) );
        REQUIRE_NOTHROW( compressor.setWordSize( 64u ) );
        REQUIRE( compressor.wordSize() == 0 );

        REQUIRE_NOTHROW( compressor.setCompressionMethod( BitCompressionMethod::BZip2 ) );
        REQUIRE_NOTHROW( compressor.setWordSize( 64u ) );
        REQUIRE( compressor.wordSize() == 0 );
    }

    SECTION( "SevenZip format + Ppmd compression method" ) {
        constexpr auto kMax7zPpmdWordSize = 32u;

        TestType compressor( lib, BitFormat::SevenZip );
        compressor.setCompressionMethod( BitCompressionMethod::Ppmd );

        REQUIRE_THROWS( compressor.setWordSize( 1u ) );
        REQUIRE( compressor.wordSize() == 0 );

        REQUIRE_NOTHROW( compressor.setWordSize( kMinPpmdWordSize ) );
        REQUIRE( compressor.wordSize() == kMinPpmdWordSize );

        REQUIRE_NOTHROW( compressor.setWordSize( 16u ) );
        REQUIRE( compressor.wordSize() == 16u );

        REQUIRE_NOTHROW( compressor.setWordSize( kMax7zPpmdWordSize ) );
        REQUIRE( compressor.wordSize() == kMax7zPpmdWordSize );

        REQUIRE_THROWS( compressor.setWordSize( kMax7zPpmdWordSize + 1 ) );
        REQUIRE( compressor.wordSize() == kMax7zPpmdWordSize );

        REQUIRE_THROWS( compressor.setWordSize( 64u ) );
        REQUIRE( compressor.wordSize() == kMax7zPpmdWordSize );

        REQUIRE_NOTHROW( compressor.setWordSize( 0 ) );
        REQUIRE( compressor.wordSize() == 0 );
    }

    SECTION( "Zip format + Ppmd compression method") {
        constexpr auto kMaxZipPpmdWordSize = 16u;

        TestType compressor( lib, BitFormat::Zip );
        compressor.setCompressionMethod( BitCompressionMethod::Ppmd );

        REQUIRE_THROWS( compressor.setWordSize( 1u ) );
        REQUIRE( compressor.wordSize() == 0 );

        REQUIRE_NOTHROW( compressor.setWordSize( kMinPpmdWordSize ) );
        REQUIRE( compressor.wordSize() == kMinPpmdWordSize );

        REQUIRE_NOTHROW( compressor.setWordSize( 16u ) );
        REQUIRE( compressor.wordSize() == 16u );

        REQUIRE_NOTHROW( compressor.setWordSize( kMaxZipPpmdWordSize ) );
        REQUIRE( compressor.wordSize() == kMaxZipPpmdWordSize );

        REQUIRE_THROWS( compressor.setWordSize( kMaxZipPpmdWordSize + 1 ) );
        REQUIRE( compressor.wordSize() == kMaxZipPpmdWordSize );

        REQUIRE_THROWS( compressor.setWordSize( 32u ) );
        REQUIRE( compressor.wordSize() == kMaxZipPpmdWordSize );

        REQUIRE_NOTHROW( compressor.setWordSize( 0 ) );
        REQUIRE( compressor.wordSize() == 0 );
    }

    SECTION( "Zip format + Deflate/Deflate64 compression method") {
        constexpr auto kMinDeflateWordSize = 3u;
        constexpr auto kMaxDeflateWordSize = 258u;
        constexpr auto kMaxDeflate64WordSize = kMaxDeflateWordSize - 1;

        TestType compressor( lib, BitFormat::Zip );
        compressor.setCompressionMethod( BitCompressionMethod::Deflate );

        REQUIRE_THROWS( compressor.setWordSize( 2u ) );
        REQUIRE( compressor.wordSize() == 0 );

        REQUIRE_NOTHROW( compressor.setWordSize( kMinDeflateWordSize ) );
        REQUIRE( compressor.wordSize() == kMinDeflateWordSize );

        REQUIRE_NOTHROW( compressor.setWordSize( 64u ) );
        REQUIRE( compressor.wordSize() == 64u );

        REQUIRE_NOTHROW( compressor.setWordSize( kMaxDeflateWordSize ) );
        REQUIRE( compressor.wordSize() == kMaxDeflateWordSize );

        REQUIRE_THROWS( compressor.setWordSize( kMaxDeflateWordSize + 1 ) );
        REQUIRE( compressor.wordSize() == kMaxDeflateWordSize );

        compressor.setCompressionMethod( BitCompressionMethod::Deflate64 );
        REQUIRE( compressor.wordSize() == 0 );

        REQUIRE_THROWS( compressor.setWordSize( 1u ) );
        REQUIRE( compressor.wordSize() == 0 );

        REQUIRE_NOTHROW( compressor.setWordSize( kMinDeflateWordSize ) );
        REQUIRE( compressor.wordSize() == kMinDeflateWordSize );

        REQUIRE_NOTHROW( compressor.setWordSize( 64u ) );
        REQUIRE( compressor.wordSize() == 64u );

        REQUIRE_NOTHROW( compressor.setWordSize( kMaxDeflate64WordSize ) );
        REQUIRE( compressor.wordSize() == kMaxDeflate64WordSize );

        REQUIRE_THROWS( compressor.setWordSize( kMaxDeflate64WordSize + 1 ) );
        REQUIRE( compressor.wordSize() == kMaxDeflate64WordSize );

        REQUIRE_NOTHROW( compressor.setWordSize( 0 ) );
        REQUIRE( compressor.wordSize() == 0 );
    }
}
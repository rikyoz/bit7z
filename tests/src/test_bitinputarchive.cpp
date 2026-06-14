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
#include "utils/filesystem.hpp"
#include "utils/format.hpp"
#include "utils/shared_lib.hpp"
#include "utils/sourcelocation.hpp"

#ifndef _WIN32
#include "utils/datetime.hpp"
#endif

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchivewriter.hpp>
#include <bit7z/bitexception.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bittypes.hpp>
#include <internal/fs.hpp>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <random>
#include <sstream>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

namespace {
auto archiveItem(
    const BitArchiveReader& archive,
    const ExpectedItem& expectedItem
) -> BitArchiveReader::ConstIterator {
    if ( archive.retainDirectories() ) {
        return archive.find( to_tstring( expectedItem.inArchivePath ) );
    }

    return std::find_if(
        archive.cbegin(),
        archive.cend(),
        [ &expectedItem ] ( const BitArchiveItem& item ) -> bool {
            return item.name() == expectedItem.fileInfo.name;
        }
    );
}

// Every item must be locatable by its own full path, and both find() and contains() must agree
// with iteration. This is the round-trip property that path-storing formats are expected to satisfy.
void requireItemsFindableByPath( const BitArchiveReader& info ) {
    for ( const auto& item : info ) {
        const auto found = info.find( item.path() );
        REQUIRE( found != info.cend() );
        REQUIRE( found->index() == item.index() );
        REQUIRE( info.contains( item.path() ) );
    }
}

void requireExtractsToFilesystem( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    const TempTestDirectory testDir{ "test_bitinputarchive" };
    INFO( "Test directory: " << testDir )

    REQUIRE_NOTHROW( info.extractTo( testDir ) );
    for ( const auto& expectedItem : expectedItems ) {
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
    }
    REQUIRE( fs::is_empty( testDir.path() ) );
}

void requireExtractsToFilesystemEmptyIndices( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    const TempTestDirectory testDir{ "test_bitinputarchive" };
    INFO( "Test directory: " << testDir )

    REQUIRE_NOTHROW( info.extractTo( testDir, std::vector< std::uint32_t >{} ) );
    for ( const auto& expectedItem : expectedItems ) {
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
    }
    REQUIRE( fs::is_empty( testDir.path() ) );
}

void requireExtractsItemsToFilesystem( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    const TempTestDirectory testDir{ "test_bitinputarchive" };
    INFO( "Test directory: " << testDir )

    for ( const auto& expectedItem : expectedItems ) {
        const auto item = archiveItem( info, expectedItem );
        REQUIRE( item != info.cend() );
        REQUIRE_NOTHROW( info.extractTo( testDir, item->index() ) );
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
    }
    REQUIRE( fs::is_empty( testDir.path() ) );

    for ( const auto& expectedItem : expectedItems ) {
        const auto item = archiveItem( info, expectedItem );
        REQUIRE( item != info.cend() );
        REQUIRE_NOTHROW( info.extractTo( testDir, { item->index() } ) );
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
    }
    REQUIRE( fs::is_empty( testDir.path() ) );

    for ( const auto& expectedItem : expectedItems ) {
        const auto item = archiveItem( info, expectedItem );
        REQUIRE( item != info.cend() );
        REQUIRE_NOTHROW( info.extractTo( testDir, IndicesVector{ item->index() } ) );
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
    }
    REQUIRE( fs::is_empty( testDir.path() ) );

    std::vector< uint32_t > testIndices( info.itemsCount() );
    std::iota( testIndices.begin(), testIndices.end(), 0 );
    REQUIRE_NOTHROW( info.extractTo( testDir, testIndices ) );
    for ( const auto& expectedItem : expectedItems ) {
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
    }
    REQUIRE( fs::is_empty( testDir.path() ) );

    // For some reason, 7-Zip doesn't like reversed or random indices when extracting Rar archives.
    if ( info.detectedFormat() != BitFormat::Rar ) {
        std::reverse( testIndices.begin(), testIndices.end() );
        REQUIRE_NOTHROW( info.extractTo( testDir, testIndices ) );
        for ( const auto& expectedItem : expectedItems ) {
            REQUIRE_FILESYSTEM_ITEM( expectedItem );
        }
        REQUIRE( fs::is_empty( testDir.path() ) );

        std::shuffle( testIndices.begin(), testIndices.end(), std::mt19937{ std::random_device{}() } );
        REQUIRE_NOTHROW( info.extractTo( testDir, testIndices ) );
        for ( const auto& expectedItem : expectedItems ) {
            REQUIRE_FILESYSTEM_ITEM( expectedItem );
        }
        REQUIRE( fs::is_empty( testDir.path() ) );
    }

    if ( info.retainDirectories() && expectedItems.size() > 1 ) {
        const auto first = info.find( to_tstring( expectedItems.front().inArchivePath.native() ) );
        REQUIRE( first != info.cend() );

        const auto last = info.find( to_tstring( expectedItems.back().inArchivePath.native() ) );
        REQUIRE( last != info.cend() );

        // extractTo must directly support brace-initialization of the indices vector.
        REQUIRE_NOTHROW( info.extractTo( testDir, { first->index(), last->index() } ) );
        REQUIRE_FILESYSTEM_ITEM( expectedItems.front() );
        REQUIRE_FILESYSTEM_ITEM( expectedItems.back() );
        REQUIRE( fs::is_empty( testDir.path() ) );
    }

    for ( const auto& expectedItem : expectedItems ) {
        const auto item = archiveItem( info, expectedItem );
        REQUIRE( item != info.cend() );
        // The vector of indices contains a valid index, and an invalid one, so the extraction should fail.
        const IndicesVector indices = { item->index(), info.itemsCount() };
        REQUIRE_THROWS( info.extractTo( testDir, indices ) );
        REQUIRE( fs::is_empty( testDir.path() ) );
    }

    REQUIRE_THROWS( info.extractTo( testDir, info.itemsCount() ) );
    REQUIRE( fs::is_empty( testDir.path() ) );

    REQUIRE_THROWS( info.extractTo( testDir, std::numeric_limits< std::uint32_t >::max() ) );
    REQUIRE( fs::is_empty( testDir.path() ) );
}

void requireExtractsToBuffersMap( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    std::map< tstring, buffer_t > bufferMap;
    REQUIRE_NOTHROW( info.extractTo( bufferMap ) );
    REQUIRE( bufferMap.size() == info.filesCount() );
    for ( const auto& expectedItem : expectedItems ) {
        INFO( "Failed while checking expected item '" << toUtf8String( expectedItem.inArchivePath ) << "'" )
        const auto& extractedItem = bufferMap.find( to_tstring( expectedItem.inArchivePath ) );
        if ( expectedItem.fileInfo.type != fs::file_type::directory ) {
            REQUIRE( extractedItem != bufferMap.end() );
            REQUIRE( crc32( extractedItem->second ) == expectedItem.fileInfo.crc32 );
        } else {
            REQUIRE( extractedItem == bufferMap.end() );
        }
    }
}

void requireExtractsToBuffers( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    buffer_t outputBuffer;
    for ( const auto& expectedItem : expectedItems ) {
        INFO( "Failed while checking expected item '" << toUtf8String( expectedItem.inArchivePath ) << "'" )
        const auto item = archiveItem( info, expectedItem );
        REQUIRE( item != info.cend() );
        if ( item->isDir() ) {
            REQUIRE_THROWS( info.extractTo( outputBuffer, item->index() ) );
            REQUIRE( outputBuffer.empty() );
        } else {
            REQUIRE_NOTHROW( info.extractTo( outputBuffer, item->index() ) );
            REQUIRE( crc32( outputBuffer ) == expectedItem.fileInfo.crc32 );
            outputBuffer.clear();
        }
    }

    buffer_t dummyBuffer;
    REQUIRE_THROWS( info.extractTo( dummyBuffer, info.itemsCount() ) );
    REQUIRE( dummyBuffer.empty() );

    REQUIRE_THROWS( info.extractTo( dummyBuffer, info.itemsCount() + 1 ) );
    REQUIRE( dummyBuffer.empty() );

    REQUIRE_THROWS( info.extractTo( dummyBuffer, std::numeric_limits< std::uint32_t >::max() ) );
    REQUIRE( dummyBuffer.empty() );
}

void requireExtractsToFixedBuffers( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    // Note: this value must be different from any file size we can encounter inside the tested archives.
    constexpr std::size_t invalidBufferSize = 42;
    buffer_t invalidBuffer( invalidBufferSize, static_cast< byte_t >( '\0' ) );
    buffer_t outputBuffer;
    for ( const auto& expectedItem : expectedItems ) {
        INFO( "Failed while checking expected item '" << toUtf8String( expectedItem.inArchivePath ) << "'" )
        const auto item = archiveItem( info, expectedItem );
        REQUIRE( item != info.cend() );

        const auto itemIndex = item->index();
        REQUIRE_THROWS( info.extractTo( nullptr, 0, itemIndex ) );
        REQUIRE_THROWS( info.extractTo( nullptr, invalidBufferSize, itemIndex ) );
        REQUIRE_THROWS( info.extractTo( nullptr, expectedItem.fileInfo.size, itemIndex ) );
        REQUIRE_THROWS( info.extractTo( nullptr, std::numeric_limits< std::size_t >::max(), itemIndex ) );

        REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), 0, itemIndex ) );
        REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), invalidBufferSize, itemIndex ) );
        REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), std::numeric_limits< std::size_t >::max(), itemIndex ) );

        if ( expectedItem.fileInfo.type == fs::file_type::directory ) {
            continue;
        }

        const auto itemSize = expectedItem.fileInfo.size;
        if ( formatHasSizeMetadata( info.detectedFormat() ) ) {
            outputBuffer.resize( itemSize, static_cast< byte_t >( '\0' ) );
            REQUIRE_NOTHROW( info.extractTo( outputBuffer.data(), itemSize, itemIndex ) );
            REQUIRE( crc32( outputBuffer ) == expectedItem.fileInfo.crc32 );
            outputBuffer.clear();
        } else {
            REQUIRE_THROWS( info.extractTo( outputBuffer.data(), itemSize, itemIndex ) );
            REQUIRE( outputBuffer.empty() );
        }
    }

    REQUIRE_THROWS( info.extractTo( nullptr, 0, info.itemsCount() ) );
    REQUIRE_THROWS( info.extractTo( nullptr, 0, info.itemsCount() + 1 ) );
    REQUIRE_THROWS( info.extractTo( nullptr, 0, std::numeric_limits< std::uint32_t >::max() ) );
    REQUIRE_THROWS( info.extractTo( nullptr, invalidBufferSize, info.itemsCount() ) );
    REQUIRE_THROWS( info.extractTo( nullptr, invalidBufferSize, info.itemsCount() + 1 ) );
    REQUIRE_THROWS( info.extractTo( nullptr, invalidBufferSize, std::numeric_limits< std::uint32_t >::max() ) );
    REQUIRE_THROWS( info.extractTo( nullptr, std::numeric_limits< std::size_t >::max(), info.itemsCount() ) );
    REQUIRE_THROWS( info.extractTo( nullptr, std::numeric_limits< std::size_t >::max(), info.itemsCount() + 1 ) );
    REQUIRE_THROWS(
        info.extractTo( nullptr,
            std::numeric_limits< std::size_t >::max(),
            std::numeric_limits< std::uint32_t >::max() )
    );

    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), 0, info.itemsCount() ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), 0, info.itemsCount() + 1 ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), 0, std::numeric_limits< std::uint32_t >::max() ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), invalidBufferSize, info.itemsCount() ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), invalidBufferSize, info.itemsCount() + 1 ) );
    REQUIRE_THROWS(
        info.extractTo( invalidBuffer.data(),
            invalidBufferSize,
            std::numeric_limits< std::uint32_t >::max() )
    );
    REQUIRE_THROWS(
        info.extractTo( invalidBuffer.data(),
            std::numeric_limits< std::size_t >::max(),
            info.itemsCount() )
    );
    REQUIRE_THROWS(
        info.extractTo( invalidBuffer.data(),
            std::numeric_limits< std::size_t >::max(),
            info.itemsCount() + 1 )
    );
    REQUIRE_THROWS(
        info.extractTo( invalidBuffer.data(),
            std::numeric_limits< std::size_t >::max(),
            std::numeric_limits< std::uint32_t >::max() )
    );
}

void requireExtractsToStreams( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    for ( const auto& expectedItem : expectedItems ) {
        INFO( "Failed while checking expected item '" << toUtf8String( expectedItem.inArchivePath ) << "'" )

        const auto item = archiveItem( info, expectedItem );
        REQUIRE( item != info.cend() );

        const auto itemIndex = item->index();
        std::ostringstream outputStream;
        if ( expectedItem.fileInfo.type == fs::file_type::directory ) {
            REQUIRE_THROWS( info.extractTo( outputStream, itemIndex ) );
            REQUIRE( outputStream.str().empty() );
        } else {
            REQUIRE_NOTHROW( info.extractTo( outputStream, itemIndex ) );
            REQUIRE( crc32( outputStream.str() ) == expectedItem.fileInfo.crc32 );
        }
    }

    std::ostringstream outputStream;
    REQUIRE_THROWS( info.extractTo( outputStream, info.itemsCount() ) );
    REQUIRE( outputStream.str().empty() );

    REQUIRE_THROWS( info.extractTo( outputStream, info.itemsCount() + 1 ) );
    REQUIRE( outputStream.str().empty() );

    REQUIRE_THROWS( info.extractTo( outputStream, std::numeric_limits< std::uint32_t >::max() ) );
    REQUIRE( outputStream.str().empty() );
}

void require_archive_extracts(
    const BitArchiveReader& info,
    const ExpectedItems& expectedItems,
    const SourceLocation& location
) {
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto& detectedFormat = ( info ).detectedFormat();
    if ( detectedFormat == BitFormat::Rar || detectedFormat == BitFormat::Rar5 ) {
        return;
    }
#endif

    INFO( "From " << location.file_name() << ":" << location.line() )
    INFO( "Failed while extracting the archive" )

    SECTION( "Extracting to a temporary filesystem folder" ) {
        requireExtractsToFilesystem( info, expectedItems );
    }

    SECTION( "Extracting to a temporary filesystem folder with empty indices vector" ) {
        requireExtractsToFilesystemEmptyIndices( info, expectedItems );
    }

    SECTION( "Extracting specific items to a temporary filesystem folder" ) {
        requireExtractsItemsToFilesystem( info, expectedItems );
    }

    SECTION( "Extracting to a map of buffers" ) {
        requireExtractsToBuffersMap( info, expectedItems );
    }

    SECTION( "Extracting each item to a buffer" ) {
        requireExtractsToBuffers( info, expectedItems );
    }

    SECTION( "Extracting each item to a fixed size buffer" ) {
        requireExtractsToFixedBuffers( info, expectedItems );
    }

    SECTION( "Extracting each item to std::ostream" ) {
        requireExtractsToStreams( info, expectedItems );
    }
}

void require_archive_extract_fails( const BitArchiveReader& info, const SourceLocation& location ) {
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto& detectedFormat = ( info ).detectedFormat();
    if ( detectedFormat == BitFormat::Rar || detectedFormat == BitFormat::Rar5 ) {
        return;
    }
#endif

    INFO( "From " << location.file_name() << ":" << location.line() )
    INFO( "Failed while extracting the archive" )

    SECTION( "Extracting to a temporary filesystem folder should fail" ) {
        const TempTestDirectory testDir{ "test_bitinputarchive" };
        INFO( "Test directory: " << testDir )
        REQUIRE_THROWS( info.extractTo( testDir ) );
        // TODO: Make some guarantees on what remains after a failed extraction
        for ( const auto& item : fs::directory_iterator( testDir.path() ) ) {
            if ( item.is_regular_file() ) {
                REQUIRE( fs::is_empty( item ) );
            }
            REQUIRE( fs::remove_all( item ) );
        }
    }

    SECTION( "Extracting to a map of buffers" ) {
        std::map< tstring, buffer_t > mapBuffers;
        REQUIRE_THROWS( info.extractTo( mapBuffers ) );
        for ( const auto& entry : mapBuffers ) {
            // TODO: Check if extractTo should not write or clear the map when the extraction fails
            REQUIRE( entry.second.empty() );
        }
    }
}

void require_archive_tests( const BitArchiveReader& info, const SourceLocation& location ) {
    INFO( "From " << location.file_name() << ":" << location.line() )
    INFO( "Failed while testing the archive" )
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto& detectedFormat = ( info ).detectedFormat();
    if ( detectedFormat == BitFormat::Rar || detectedFormat == BitFormat::Rar5 ) {
        return;
    }
#endif
    REQUIRE_NOTHROW( info.test() );
    for ( uint32_t index = 0; index < info.itemsCount(); ++index ) {
        REQUIRE_NOTHROW( info.testItem( index ) );
    }
    REQUIRE_THROWS_AS( info.testItem( info.itemsCount() ), BitException );
}
} // namespace

#define REQUIRE_ARCHIVE_EXTRACTS( info, expectedItems ) \
    require_archive_extracts( info, expectedItems, BIT7Z_CURRENT_LOCATION )

#define REQUIRE_ARCHIVE_EXTRACT_FAILS( info ) \
    require_archive_extract_fails( info, BIT7Z_CURRENT_LOCATION )

#define REQUIRE_ARCHIVE_TESTS( info ) \
    require_archive_tests( info, BIT7Z_CURRENT_LOCATION )

/* Note: throughout this unit test we will use BitArchiveReader for testing BitInputArchive's specific methods. */

TEST_CASE( "BitInputArchive: Opening a non-existing archive should throw an exception" ) {
    REQUIRE_THROWS( BitArchiveReader{ test::sevenzipLib(), BIT7Z_STRING( "non-existing.7z" ), BitFormat::SevenZip } );

    const buffer_t emptyBuffer{};
    REQUIRE_THROWS( BitArchiveReader{ test::sevenzipLib(), emptyBuffer, BitFormat::SevenZip } );

    fs::ifstream nonExistingStream{ "non-existing.7z" };
    REQUIRE_THROWS( BitArchiveReader{ test::sevenzipLib(), nonExistingStream, BitFormat::SevenZip } );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Testing and extracting archives containing only a single file",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto testFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "lzh", BitFormat::Lzh },
        TestInputFormat{ "lzma", BitFormat::Lzma },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const auto arcFileName = fs::path{ clouds.name }.concat( "." + testFormat.extension );

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        if ( formatHasPathMetadata( testFormat.format ) || is_filesystem_archive< TestType >::value ) {
            REQUIRE_ARCHIVE_EXTRACTS( info, singleFileContent().items );
        } else {
            REQUIRE_ARCHIVE_EXTRACTS( info, noPathContent().items );
        }
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Testing and extracting archives containing multiple files",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_files" };

    const auto testFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "multiple_files." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multipleFilesContent().items );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Testing and extracting archives containing multiple items (files and folders)",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multipleItemsContent().items );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Testing and extracting archives containing encrypted items",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "encrypted" };

    const auto testFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "aes256.zip", BitFormat::Zip },
        TestInputFormat{ "zipcrypto.zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        constexpr auto password = BIT7Z_STRING( "helloworld" );
        const fs::path arcFileName = "encrypted." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );

        SECTION( "Opening the archive with no password is allowed, but testing and extraction should throw" ) {
            BitArchiveReader info( test::sevenzipLib(), inputArchive, testFormat.format );
            REQUIRE_THROWS( info.test() );
            REQUIRE_ARCHIVE_EXTRACT_FAILS( info );

            // After setting the password, the archive can be extracted.
            info.setPassword( password );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info, encryptedContent().items );

            info.clearPassword();
            REQUIRE_THROWS( info.test() );
            REQUIRE_ARCHIVE_EXTRACT_FAILS( info );

            info.setPasswordCallback(
                [ &password ]() -> tstring {
                    return password;
                }
            );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info, encryptedContent().items );
        }

        SECTION( "Opening the archive with the correct password should allow testing and extraction without issues" ) {
            const BitArchiveReader info( test::sevenzipLib(), inputArchive, testFormat.format, password );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info, encryptedContent().items );
        }
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Testing and extracting header-encrypted archives",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "header_encrypted" };

    const auto testFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "header_encrypted." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );

        SECTION( "Opening the archive with no password should throw an exception" ) {
            REQUIRE_THROWS( BitArchiveReader( test::sevenzipLib(), inputArchive, testFormat.format ) );
        }

        SECTION( "Opening the archive with a wrong password should throw an exception" ) {
            REQUIRE_THROWS(
                BitArchiveReader( test::sevenzipLib(), inputArchive, testFormat.format,
                    BIT7Z_STRING( "wrong_password" ) )
            );
        }

        SECTION( "Opening the archive with the correct password should pass the tests" ) {
            constexpr auto password = BIT7Z_STRING( "helloworld" );
            const BitArchiveReader info( test::sevenzipLib(), inputArchive, testFormat.format, password );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info, encryptedContent().items );
        }
    }
}

TEST_CASE( "BitInputArchive: Testing and extracting multi-volume archives", "[bitinputarchive]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "split" };

    SECTION( "Split archive (non-RAR)" ) {
        const auto testFormat = GENERATE(
            as< TestInputFormat >(),
            TestInputFormat{ "7z", BitFormat::SevenZip },
            TestInputFormat{ "bz2", BitFormat::BZip2 },
            TestInputFormat{ "gz", BitFormat::GZip },
            TestInputFormat{ "tar", BitFormat::Tar },
            TestInputFormat{ "wim", BitFormat::Wim },
            TestInputFormat{ "xz", BitFormat::Xz },
            TestInputFormat{ "zip", BitFormat::Zip }
        );

        DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
            const auto wholeArcFileName = std::string{ "clouds.jpg." } + testFormat.extension;
            const fs::path splitArcFileName = wholeArcFileName + ".001";

            INFO( "Archive file: " << splitArcFileName )

            SECTION( "Opening as a split archive" ) {
                const BitArchiveReader info(
                    test::sevenzipLib(),
                    splitArcFileName.string< tchar >(),
                    BitFormat::Split
                );
                REQUIRE_ARCHIVE_TESTS( info );

                const TempTestDirectory extractionTestDir{ "test_bitinputarchive" };
                REQUIRE_NOTHROW( info.extractTo( extractionTestDir ) );
                REQUIRE( fs::exists( wholeArcFileName ) );
                REQUIRE( fs::remove( wholeArcFileName ) );
            }

            SECTION( "Opening as a whole archive" ) {
                const BitArchiveReader info(
                    test::sevenzipLib(),
                    splitArcFileName.string< tchar >(),
                    testFormat.format
                );
                REQUIRE_ARCHIVE_TESTS( info );
                REQUIRE_ARCHIVE_EXTRACTS( info, singleFileContent().items );
            }
        }
    }

    SECTION( "Multi-volume RAR5" ) {
        const fs::path arcFileName = "clouds.jpg.part1.rar";
        const BitArchiveReader info( test::sevenzipLib(), arcFileName.string< tchar >(), BitFormat::Rar5 );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, singleFileContent().items );
    }

    SECTION( "Multi-volume RAR4" ) {
        const fs::path arcFileName = "clouds.jpg.rar";
        const BitArchiveReader info( test::sevenzipLib(), arcFileName.string< tchar >(), BitFormat::Rar );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, singleFileContent().items );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Testing and extracting an empty archive",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "empty" };

    const auto testFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        // TestInputFormat{ "tar", BitFormat::Tar, 0 }, // TODO: Check why it fails opening
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "empty." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, {} );
    }
}

namespace {
// Builds an in-memory archive from the given writer configuration and returns the root folder
// reported by a reader opened on the resulting archive.
template< typename Configure >
auto archiveRootFolder( const Configure& configure, const BitInOutFormat& format = BitFormat::SevenZip ) -> tstring {
    BitArchiveWriter writer{ test::sevenzipLib(), format };
    configure( writer );

    buffer_t archiveBuffer;
    writer.compressTo( archiveBuffer );

    const BitArchiveReader reader{ test::sevenzipLib(), archiveBuffer, format };
    return reader.rootFolder();
}
} // namespace

TEST_CASE( "BitInputArchive: Retrieving the archive's root folder", "[bitinputarchive]" ) {
    // A small dummy content for the in-memory files added to the test archives.
    const buffer_t fileContent( 8, static_cast< byte_t >( 0x7A ) );

    SECTION( "An empty archive has no root folder" ) {
        const auto rootFolder = archiveRootFolder( []( BitArchiveWriter& ) -> void {} );
        REQUIRE( rootFolder.empty() );
    }

    SECTION( "A single top-level file has no root folder" ) {
        const auto rootFolder = archiveRootFolder( [ &fileContent ] ( BitArchiveWriter& writer ) -> void {
            writer.addFile( fileContent, BIT7Z_STRING( "file.txt" ) );
        } );
        REQUIRE( rootFolder.empty() );
    }

    SECTION( "Multiple top-level files have no common root folder" ) {
        const auto rootFolder = archiveRootFolder( [ &fileContent ]( BitArchiveWriter& writer ) -> void {
            writer.addFile( fileContent, BIT7Z_STRING( "a.txt" ) );
            writer.addFile( fileContent, BIT7Z_STRING( "b.txt" ) );
        } );
        REQUIRE( rootFolder.empty() );
    }

    SECTION( "Files sharing a single top-level folder share that root folder" ) {
        const auto rootFolder = archiveRootFolder( [ &fileContent ] ( BitArchiveWriter& writer ) -> void {
            writer.addFile( fileContent, BIT7Z_STRING( "root/a.txt" ) );
            writer.addFile( fileContent, BIT7Z_STRING( "root/b.txt" ) );
            writer.addFile( fileContent, BIT7Z_STRING( "root/sub/c.txt" ) );
        } );
        REQUIRE( rootFolder == BIT7Z_STRING( "root" ) );
    }

    SECTION( "Files under different top-level folders have no common root folder" ) {
        const auto rootFolder = archiveRootFolder( [ &fileContent ] ( BitArchiveWriter& writer ) -> void {
            writer.addFile( fileContent, BIT7Z_STRING( "x/a.txt" ) );
            writer.addFile( fileContent, BIT7Z_STRING( "y/b.txt" ) );
        } );
        REQUIRE( rootFolder.empty() );
    }

    SECTION( "A top-level file alongside a folder has no common root folder" ) {
        const auto rootFolder = archiveRootFolder( [ &fileContent ]( BitArchiveWriter& writer ) -> void {
            writer.addFile( fileContent, BIT7Z_STRING( "readme.txt" ) );
            writer.addFile( fileContent, BIT7Z_STRING( "folder/a.txt" ) );
        } );
        REQUIRE( rootFolder.empty() );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE( "BitInputArchive: An explicit directory entry is recognized as the root folder", "[bitinputarchive]" ) {
    // Compressing a real directory makes 7-Zip store an explicit directory entry ("folder")
    // alongside its children ("folder/..."), which the reader surfaces as the separator-less
    // path "folder". We exploit this to test a separator-less directory entry as a root folder:
    // the children alone would yield "folder" through the substring branch under both the old
    // and the new code, so only the "folder" entry discriminates the fix.
    const TestDirectory testDir{ test_filesystem_dir };

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "zip", BitFormat::Zip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        BitArchiveWriter writer{ test::sevenzipLib(), testFormat.format };
        writer.addDirectory( folder.name );

        buffer_t archiveBuffer;
        writer.compressTo( archiveBuffer );

        const BitArchiveReader reader{ test::sevenzipLib(), archiveBuffer, testFormat.format };

        // Guard against a vacuous test: the assertion below only discriminates the fix if the
        // archive actually contains the separator-less directory entry for "folder".
        const auto hasRootDirEntry = std::any_of(
            reader.cbegin(), reader.cend(),
            []( const BitArchiveItem& item ) -> bool {
                return item.isDir() && item.nativePath() == BIT7Z_NATIVE_STRING( "folder" );
            }
        );
        REQUIRE( hasRootDirEntry );

        REQUIRE( reader.rootFolder() == folder.name );
    }
}

TEST_CASE( "BitInputArchive: Multiple top-level directory entries have no common root folder", "[bitinputarchive]" ) {
    // Two separate top-level directories produce two separator-less directory entries ("folder"
    // and "dot.folder"). Unlike the multiple_items archives, there is no top-level file that would
    // make rootFolder() bail out early, so this is the case that exercises the new directory-entry
    // branch feeding the root-folder mismatch detection: each entry yields a different, non-empty
    // root, which must be rejected.
    const TestDirectory testDir{ test_filesystem_dir };

    BitArchiveWriter writer{ test::sevenzipLib(), BitFormat::SevenZip };
    writer.addDirectory( folder.name );
    writer.addDirectory( dotFolder.name );

    buffer_t archiveBuffer;
    writer.compressTo( archiveBuffer );

    const BitArchiveReader reader{ test::sevenzipLib(), archiveBuffer, BitFormat::SevenZip };

    // Guard against a vacuous test: both top-level directories must be present as separator-less
    // directory entries, otherwise we wouldn't actually be exercising the mismatch detection.
    const auto hasDirEntry = [ &reader ]( const native_string& path ) -> bool {
        return std::any_of(
            reader.cbegin(), reader.cend(),
            [ &path ]( const BitArchiveItem& item ) -> bool {
                return item.isDir() && item.nativePath() == path;
            }
        );
    };
    REQUIRE( hasDirEntry( BIT7Z_NATIVE_STRING( "folder" ) ) );
    REQUIRE( hasDirEntry( BIT7Z_NATIVE_STRING( "dot.folder" ) ) );

    REQUIRE( reader.rootFolder().empty() );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE( "BitInputArchive: Archives with multiple top-level items have no root folder", "[bitinputarchive]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;
        const BitArchiveReader info( test::sevenzipLib(), arcFileName.string< tchar >(), testArchive.format );
        REQUIRE( info.rootFolder().empty() );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE( "BitInputArchive: Extracting the content of the archive's root folder", "[bitinputarchive]" ) {
    // Distinct contents, so that a mismatch between an extracted file and its source would be caught.
    const buffer_t contentA( 8, static_cast< byte_t >( 0xA1 ) );
    const buffer_t contentB( 16, static_cast< byte_t >( 0xB2 ) );
    const buffer_t contentC( 32, static_cast< byte_t >( 0xC3 ) );

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "zip", BitFormat::Zip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        BitArchiveWriter writer{ test::sevenzipLib(), testFormat.format };
        writer.addFile( contentA, BIT7Z_STRING( "root/a.txt" ) );
        writer.addFile( contentB, BIT7Z_STRING( "root/b.txt" ) );
        writer.addFile( contentC, BIT7Z_STRING( "root/sub/c.txt" ) );

        buffer_t archiveBuffer;
        writer.compressTo( archiveBuffer );

        const BitArchiveReader reader{ test::sevenzipLib(), archiveBuffer, testFormat.format };

        // Guard against a vacuous test: the archive must actually have a single root folder to strip.
        REQUIRE( reader.rootFolder() == BIT7Z_STRING( "root" ) );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        REQUIRE_NOTHROW( reader.extractRootFolderContentTo( testOutDir ) );

        // The root folder's prefix is stripped: its content lands directly in the output directory.
        REQUIRE_FALSE( fs::exists( testOutDir.path() / "root" ) );

        const auto outA = testOutDir.path() / "a.txt";
        const auto outB = testOutDir.path() / "b.txt";
        const auto outC = testOutDir.path() / "sub" / "c.txt";

        REQUIRE( fs::exists( outA ) );
        REQUIRE( fs::exists( outB ) );
        REQUIRE( fs::exists( outC ) );
        REQUIRE( loadFile( outA ) == contentA );
        REQUIRE( loadFile( outB ) == contentB );
        REQUIRE( loadFile( outC ) == contentC );

        REQUIRE( fs::remove( outA ) );
        REQUIRE( fs::remove( outB ) );
        REQUIRE( fs::remove( outC ) );
        REQUIRE( fs::remove( testOutDir.path() / "sub" ) );
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE(
    "BitInputArchive: Extracting the root folder's content honors retainDirectories",
    "[bitinputarchive]"
) {
    // With retainDirectories() false, the root folder is stripped and its content is flattened:
    // root/a/b/foo.txt becomes foo.txt, and root/bar.txt becomes bar.txt.
    const buffer_t contentFoo( 8, static_cast< byte_t >( 0xE1 ) );
    const buffer_t contentBar( 16, static_cast< byte_t >( 0xE2 ) );

    BitArchiveWriter writer{ test::sevenzipLib(), BitFormat::SevenZip };
    writer.addFile( contentFoo, BIT7Z_STRING( "root/a/b/foo.txt" ) );
    writer.addFile( contentBar, BIT7Z_STRING( "root/bar.txt" ) );

    buffer_t archiveBuffer;
    writer.compressTo( archiveBuffer );

    BitArchiveReader reader{ test::sevenzipLib(), archiveBuffer, BitFormat::SevenZip };
    reader.setRetainDirectories( false );

    const TempTestDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Output directory: " << testOutDir )

    REQUIRE_NOTHROW( reader.extractRootFolderContentTo( testOutDir ) );

    const auto outFoo = testOutDir.path() / "foo.txt";
    const auto outBar = testOutDir.path() / "bar.txt";
    REQUIRE( fs::exists( outFoo ) );
    REQUIRE( fs::exists( outBar ) );
    // The nested structure is flattened away, not recreated.
    REQUIRE_FALSE( fs::exists( testOutDir.path() / "a" ) );
    REQUIRE( loadFile( outFoo ) == contentFoo );
    REQUIRE( loadFile( outBar ) == contentBar );

    REQUIRE( fs::remove( outFoo ) );
    REQUIRE( fs::remove( outBar ) );
    REQUIRE( fs::is_empty( testOutDir.path() ) );
}

namespace {
// Builds an in-memory archive from the given writer configuration, then requires that extracting
// its root folder's content throws and that nothing is written to the output directory.
template< typename Configure >
void requireNoRootFolderExtractionThrows( const Configure& configure ) {
    BitArchiveWriter writer{ test::sevenzipLib(), BitFormat::SevenZip };
    configure( writer );

    buffer_t archiveBuffer;
    writer.compressTo( archiveBuffer );

    const BitArchiveReader reader{ test::sevenzipLib(), archiveBuffer, BitFormat::SevenZip };

    const TempTestDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Output directory: " << testOutDir )

    REQUIRE_THROWS( reader.extractRootFolderContentTo( testOutDir ) );
    REQUIRE( fs::is_empty( testOutDir.path() ) );
}
} // namespace

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE(
    "BitInputArchive: Extracting the root folder's content throws without a single root folder",
    "[bitinputarchive]"
) {
    const buffer_t fileContent( 8, static_cast< byte_t >( 0x7A ) );

    SECTION( "An empty archive has no root folder" ) {
        requireNoRootFolderExtractionThrows( []( BitArchiveWriter& ) -> void {} );
    }

    SECTION( "A single top-level file has no root folder" ) {
        requireNoRootFolderExtractionThrows( [ &fileContent ] ( BitArchiveWriter& writer ) -> void {
            writer.addFile( fileContent, BIT7Z_STRING( "file.txt" ) );
        } );
    }

    SECTION( "Multiple top-level files have no common root folder" ) {
        requireNoRootFolderExtractionThrows( [ &fileContent ]( BitArchiveWriter& writer ) -> void {
            writer.addFile( fileContent, BIT7Z_STRING( "a.txt" ) );
            writer.addFile( fileContent, BIT7Z_STRING( "b.txt" ) );
        } );
    }

    SECTION( "Files under different top-level folders have no common root folder" ) {
        requireNoRootFolderExtractionThrows( [ &fileContent ] ( BitArchiveWriter& writer ) -> void {
            writer.addFile( fileContent, BIT7Z_STRING( "x/a.txt" ) );
            writer.addFile( fileContent, BIT7Z_STRING( "y/b.txt" ) );
        } );
    }

    SECTION( "A top-level file alongside a folder has no common root folder" ) {
        requireNoRootFolderExtractionThrows( [ &fileContent ]( BitArchiveWriter& writer ) -> void {
            writer.addFile( fileContent, BIT7Z_STRING( "readme.txt" ) );
            writer.addFile( fileContent, BIT7Z_STRING( "folder/a.txt" ) );
        } );
    }
}

TEST_CASE( "BitInputArchive: Testing and extracting solid archives", "[bitinputarchive]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "solid" };

    SECTION( "Solid 7z" ) {
        const BitArchiveReader info( test::sevenzipLib(), BIT7Z_STRING( "solid.7z" ), BitFormat::SevenZip );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multipleItemsContent().items );
    }

    SECTION( "Solid RAR" ) {
        const BitArchiveReader info( test::sevenzipLib(), BIT7Z_STRING( "solid.rar" ), BitFormat::Rar5 );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multipleItemsContent().items );
    }

    SECTION( "Non solid 7z" ) {
        const BitArchiveReader info( test::sevenzipLib(), BIT7Z_STRING( "non_solid.7z" ), BitFormat::SevenZip );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multipleItemsContent().items );
    }

    SECTION( "Non-solid RAR" ) {
        const BitArchiveReader info( test::sevenzipLib(), BIT7Z_STRING( "non_solid.rar" ), BitFormat::Rar5 );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multipleItemsContent().items );
    }
}

namespace {
/**
 * Tests opening an archive file using the RAR format
 * (or throws a BitException if it is not a RAR archive at all).
 */
auto test_open_rar_archive( const Bit7zLibrary& lib, const tstring& inFile ) -> const BitInFormat& {
    try {
        const BitArchiveReader info( lib, inFile, BitFormat::Rar );
        //if BitArchiveReader constructor did not throw an exception, the archive is RAR (< 5.0).
        return BitFormat::Rar;
    } catch ( const BitException& ) {
        /* the archive is not a RAR, and if it is not even a RAR5,
           the following line will throw an exception (which we do not catch). */
        const BitArchiveReader info( lib, inFile, BitFormat::Rar5 );
        return BitFormat::Rar5;
    }
}
} // namespace

TEST_CASE( "BitArchiveReader: Opening RAR archives using the correct RAR format version", "[bitarchivereader]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    SECTION( "Valid RAR archives" ) {
        REQUIRE( test_open_rar_archive( test::sevenzipLib(), BIT7Z_STRING( "valid.rar4.rar" ) ) == BitFormat::Rar );
        REQUIRE( test_open_rar_archive( test::sevenzipLib(), BIT7Z_STRING( "valid.rar5.rar" ) ) == BitFormat::Rar5 );
    }

    SECTION( "Non-RAR archive" ) {
        REQUIRE_THROWS( test_open_rar_archive( test::sevenzipLib(), BIT7Z_STRING( "valid.zip" ) ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Testing and extracting invalid archives should throw",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "testing" };

    // The italy.svg file in the ko_test archives is different from the one used for filesystem tests
    static constexpr auto italy_ko_crc32 = 0x2ADFB3AF;

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "rar", BitFormat::Rar5 },
        //TestInputFormat{"tar", BitFormat::Tar},
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "ko_test." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );
        REQUIRE_THROWS( info.test() );

        std::map< tstring, buffer_t > dummyMap;
        REQUIRE_THROWS( info.extractTo( dummyMap ) );
        // TODO: Check if extractTo should not write or clear the map when the extraction fails

        buffer_t outputBuffer;
        std::ostringstream outputStream;
        if ( info.itemsCount() == 1 ) {
            REQUIRE_THROWS( info.extractTo( outputBuffer, 0 ) );
            REQUIRE( outputBuffer.empty() );

            REQUIRE_THROWS( info.extractTo( outputStream, 0 ) );
            // Note: we might have written some data to the stream before 7-zip failed!
        } else if ( info.itemsCount() == 2 ) {
#ifndef BIT7Z_BUILD_FOR_P7ZIP
            REQUIRE_NOTHROW( info.extractTo( outputBuffer, 0 ) );
            REQUIRE( crc32( outputBuffer ) == italy_ko_crc32 );

            REQUIRE_NOTHROW( info.extractTo( outputStream, 0 ) );
            REQUIRE( crc32( outputStream.str() ) == italy_ko_crc32 );
#endif

            outputBuffer.clear();
            REQUIRE_THROWS( info.extractTo( outputBuffer, 1 ) );
            REQUIRE( outputBuffer.empty() );

            outputStream.str( "" );
            outputStream.clear();
            REQUIRE_THROWS( info.extractTo( outputStream, 1 ) );
            // Note: we might have written some data to the stream before 7-zip failed!
        }
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Reading archives using the wrong format should throw",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto correctFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "lzh", BitFormat::Lzh },
        TestInputFormat{ "lzma", BitFormat::Lzma },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    const auto wrongFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "lzh", BitFormat::Lzh },
        TestInputFormat{ "lzma", BitFormat::Lzma },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << correctFormat.extension ) {
        const auto arcFileName = fs::path{ clouds.name }.concat( "." + correctFormat.extension );

        if ( correctFormat.extension != wrongFormat.extension ) {
            DYNAMIC_SECTION( "Wrong format: " << wrongFormat.extension ) {
                TestType inputArchive{};
                getInputArchive( arcFileName, inputArchive );

                REQUIRE_THROWS_WITH(
                    BitArchiveReader( test::sevenzipLib(), inputArchive, wrongFormat.format ),
                    Catch::Matchers::EndsWith( "Invalid archive, or wrong format used." )
                );
            }
        }
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE(
    "BitInputArchive: Opening a PE SFX archive as Pe should succeed",
    "[bitinputarchive]"
) {
    // By default, the 7-Zip Pe handler rejects executables with data appended after the PE image
    // (e.g., SFX archives) by returning S_FALSE without setting any error flag.
    // Since the user explicitly requested the Pe format, bit7z asks the handler
    // to accept such executables (via IArchiveAllowTail), like 7-Zip does.
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "sfx" / "exe" };

    const auto filename = GENERATE(
        as< tstring >(),
        BIT7Z_STRING( "sfx.7z.exe" ),
        BIT7Z_STRING( "sfx.rar.exe" ),
        BIT7Z_STRING( "sfx.cab.exe" ),
        BIT7Z_STRING( "sfx.zip.exe" ),
        BIT7Z_STRING( "sfx.rar.zip.exe" )
    );

    DYNAMIC_SECTION( Catch::StringMaker< tstring >::convert( filename ) ) {
        const BitArchiveReader reader{ test::sevenzipLib(), filename, BitFormat::Pe };
        REQUIRE( reader.itemsCount() > 0 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE(
    "BitInputArchive: Opening a non-PE file as Pe should report a clear error",
    "[bitinputarchive]"
) {
    // The 7-Zip Pe handler rejects files without a valid PE signature by returning S_FALSE
    // without setting any error flag. bit7z should surface this as a clear
    // "invalid archive, or wrong format" error, not the opaque raw HRESULT (S_FALSE == 1).
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    const auto arcBuffer = loadFile( "valid.7z" );
    REQUIRE_FALSE( arcBuffer.empty() );

    REQUIRE_THROWS_WITH(
        BitArchiveReader( test::sevenzipLib(), arcBuffer, BitFormat::Pe ),
        Catch::Matchers::EndsWith( "Invalid archive, or wrong format used." )
    );
}

namespace {
BIT7Z_ALWAYS_INLINE
auto isHiddenFile( const BitArchiveItem& item ) -> bool {
    return item.nativePath() == BIT7Z_NATIVE_STRING( "hidden" );
}

#ifndef _WIN32
BIT7Z_ALWAYS_INLINE
auto formatSupportsSymlinks( const BitInFormat& format ) -> bool {
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    // NOTE: p7zip doesn't support the Unix link attribute in Tar archives.
    return format != BitFormat::Rar5 && format != BitFormat::Wim && format != BitFormat::Tar;
#else
    return format != BitFormat::Rar5 && format != BitFormat::Wim;
#endif
}
#endif

BIT7Z_ALWAYS_INLINE
auto formatSupportsUnixPermissions( const BitInFormat& format ) -> bool {
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    // NOTE: p7zip doesn't support the Unix permissions in Tar archives.
    return format != BitFormat::Tar;
#else
    ( void ) format;
    return true;
#endif
}
} // namespace

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Correctly keeping file attributes after extraction",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "file_type" };

    const auto testFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    const TempDirectory outDir{ "test_bitarchivereader" };
    const auto filterCallback = [ &testFormat ] ( const BitArchiveItem& item ) -> FilterResult {
        const auto isAltStream = item.itemProperty( BitProperty::IsAltStream );
        if ( isAltStream.isBool() && isAltStream.getBool() ) {
            return FilterResult::SkipItem; // Ignoring alternate stream in WIM archives.
        }
#ifdef _WIN32
        if ( testFormat.format == BitFormat::Tar && isHiddenFile( item ) ) {
            // Tar archives do not store the Windows' hidden file attribute.
            return FilterResult::SkipItem;
        }
        return item.isSymLink() ? FilterResult::SkipItem : FilterResult::ProcessItem;
#else
        if ( !formatSupportsSymlinks( testFormat.format ) && item.isSymLink() ) {
            // NOTE: 7-Zip seems to not support symlinks in Rar5 archives.
            // TODO: Fix extraction of Windows reparse points (symlinks) from Wim archives on Linux.
            return FilterResult::SkipItem;
        }
        if (
            !formatSupportsUnixPermissions( testFormat.format ) &&
            item.nativePath() == BIT7Z_NATIVE_STRING( "read_only" )
        ) {
            return FilterResult::SkipItem;
        }
        return isHiddenFile( item ) ? FilterResult::SkipItem : FilterResult::ProcessItem;
#endif
    };

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "file_type." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testFormat.format );
        info.extractTo( to_tstring( outDir.path() ), filterCallback );

        std::error_code error;
        const auto dirPath = outDir.path() / "dir";
        const auto dirStatus = fs::status( dirPath, error );
        REQUIRE_FALSE( error );
        REQUIRE( fs::exists( dirStatus ) );
        REQUIRE( fs::is_directory( dirStatus ) );
        REQUIRE( fs::remove_all( dirPath ) );

        const auto regularPath = outDir.path() / "regular";
        const auto regularStatus = fs::status( regularPath, error );
        REQUIRE_FALSE( error );
        REQUIRE( fs::exists( regularStatus ) );
        REQUIRE( fs::is_regular_file( regularStatus ) );
        REQUIRE( fs::remove( regularPath ) );

#ifdef _WIN32
        if ( testFormat.format != BitFormat::Tar ) {
            const auto hiddenPath = outDir.path() / "hidden";
            const auto hiddenStatus = fs::status( hiddenPath, error );
            REQUIRE_FALSE( error );
            REQUIRE( fs::exists( hiddenStatus ) );
            const auto attributes = GetFileAttributesW( hiddenPath.c_str() );
            REQUIRE( ( attributes & FILE_ATTRIBUTE_HIDDEN ) == FILE_ATTRIBUTE_HIDDEN );
            REQUIRE( fs::remove( hiddenPath ) );
        }
#else
        if ( formatSupportsSymlinks( testFormat.format ) ) {
            const auto symlinkPath = outDir.path() / "symlink";
            const auto symlinkStatus = fs::symlink_status( symlinkPath, error );
            REQUIRE_FALSE( error );
            REQUIRE( fs::exists( symlinkStatus ) );
            REQUIRE( fs::is_symlink( symlinkStatus ) );
            REQUIRE( fs::remove( symlinkPath ) );
        }
#endif

        if ( formatSupportsUnixPermissions( testFormat.format ) ) {
            const auto readOnlyPath = outDir.path() / "read_only";
            const auto readOnlyStatus = fs::status( readOnlyPath, error );
            REQUIRE_FALSE( error );
            REQUIRE( fs::exists( readOnlyStatus ) );
            const auto readOnlyPermissions = readOnlyStatus.permissions();
            REQUIRE( (readOnlyPermissions & fs::perms::owner_write) == fs::perms::none );
            REQUIRE( (readOnlyPermissions & fs::perms::group_write) == fs::perms::none );
            REQUIRE( (readOnlyPermissions & fs::perms::others_write) == fs::perms::none );
            // Some compiler/system combinations fail removing the file if it is read only.
            REQUIRE_NOTHROW( fs::permissions( readOnlyPath, fs::perms::owner_write ) );
            REQUIRE( fs::remove( readOnlyPath ) );
        }

        INFO( outDir.path() );
        REQUIRE( fs::is_empty( outDir.path() ) );
    }
}

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Testing and extracting an archive with Unicode items",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const auto testFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "unicode." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, unicodeContent().items );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Testing and extracting an archive with a Unicode file name",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName{ BIT7Z_NATIVE_STRING( "αρχείο.7z" ) };

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader info( test::sevenzipLib(), inputArchive, BitFormat::SevenZip );
    REQUIRE_ARCHIVE_TESTS( info );
    REQUIRE_ARCHIVE_EXTRACTS( info, unicodeContent().items );
}

TEST_CASE(
    "BitInputArchive: Testing and extracting an archive with a Unicode file name (bzip2)",
    "[bitinputarchive]"
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName{ BIT7Z_NATIVE_STRING( "クラウド.jpg.bz2" ) };
    const BitArchiveReader info( test::sevenzipLib(), to_tstring( arcFileName ), BitFormat::BZip2 );
    REQUIRE_ARCHIVE_TESTS( info );
    const ExpectedItems expectedItems{ ExpectedItem{ clouds, BIT7Z_NATIVE_STRING( "クラウド.jpg" ), false } };
    REQUIRE_ARCHIVE_EXTRACTS( info, expectedItems );
}

#endif

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting an archive without retaining directories",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );
        info.setRetainDirectories( false );
        REQUIRE_ARCHIVE_EXTRACTS( info, flatItemsContent().items );
    }
}

namespace {
template< typename TestType >
auto overwritten_file_path( const BitInFormat& format ) -> fs::path {
    if ( is_filesystem_archive< TestType >::value || formatHasPathMetadata( format ) ) {
        return clouds.name;
    }
    return "[Content]";
}
} // namespace

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting an archive using various OverwriteMode settings",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "lzh", BitFormat::Lzh },
        TestInputFormat{ "lzma", BitFormat::Lzma },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testFormat = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "lzh", BitFormat::Lzh },
        TestInputFormat{ "lzma", BitFormat::Lzma },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#endif

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const auto arcFileName = fs::path{ clouds.name }.concat( "." + testFormat.extension );

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader info( test::sevenzipLib(), inputArchive, testFormat.format );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        const auto expectedFile = overwritten_file_path< TestType >( testFormat.format );
        REQUIRE_FALSE( fs::exists( expectedFile ) );
        {
            const fs::ofstream dummyFile{ expectedFile };
        }
        REQUIRE( fs::is_empty( expectedFile ) );
        REQUIRE( fs::exists( expectedFile ) );

        SECTION( "OverwriteMode::None" ) {
            // After setting OverwriteMode::Overwrite, extracting should not throw.
            info.setOverwriteMode( OverwriteMode::None );

            // By default, BitArchiveReader uses OverwriteMode::None, so extracting again should throw.
            REQUIRE_THROWS( info.extractTo( testOutDir ) );
            REQUIRE( fs::exists( expectedFile ) );
            REQUIRE( fs::is_empty( expectedFile ) );
            REQUIRE( fs::remove( expectedFile ) );

            // Verifying that if we remove the file, we can now extract it.
            REQUIRE_NOTHROW( info.extractTo( testOutDir ) );
            REQUIRE( fs::exists( expectedFile ) );
            REQUIRE( crc32( loadFile( expectedFile ) ) == clouds.crc32 );
        }

        SECTION( "OverwriteMode::Overwrite" ) {
            // After setting OverwriteMode::Overwrite, extracting should not throw.
            info.setOverwriteMode( OverwriteMode::Overwrite );

            REQUIRE_NOTHROW( info.extractTo( testOutDir ) );
            REQUIRE( fs::exists( expectedFile ) );
            REQUIRE( crc32( loadFile( expectedFile ) ) == clouds.crc32 );
        }

        SECTION( "OverwriteMode::Skip" ) {
            // After setting OverwriteMode::Skip, extracting should not throw and not extracting anything.
            info.setOverwriteMode( OverwriteMode::Skip );

            REQUIRE_NOTHROW( info.extractTo( testOutDir ) );
            REQUIRE( fs::exists( expectedFile ) );
            REQUIRE( fs::is_empty( expectedFile ) );
        }
        REQUIRE( fs::remove( expectedFile ) );
    }
}

#ifdef _WIN32
namespace {
auto get_file_time( const fs::path& filePath, FILETIME& creation, FILETIME& access, FILETIME& modified ) -> bool {
    const HANDLE hFile = ::CreateFileW(
        filePath.c_str(),
        GENERIC_READ | FILE_READ_ATTRIBUTES,
        // NOLINT(*-signed-bitwise)
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );
    if ( hFile == INVALID_HANDLE_VALUE ) { // NOLINT(cppcoreguidelines-pro-type-cstyle-cast,performance-no-int-to-ptr)
        return false;
    }
    const auto result = GetFileTime( hFile, &creation, &access, &modified );
    CloseHandle( hFile );
    return result != FALSE;
}
} // namespace
#endif

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting an archive to the filesystem should preserve time metadata",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto arcFileName = fs::path{ clouds.name }.concat( ".7z" );

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader info( test::sevenzipLib(), inputArchive, BitFormat::SevenZip );

    const auto item = info.itemAt( 0 );
#ifdef _WIN32
    // Creation time.
    const BitPropVariant expectedCTime = item.itemProperty( BitProperty::CTime );
    REQUIRE( expectedCTime.isFileTime() );

    const FILETIME expectedCreationTime = expectedCTime.getFileTime();
    REQUIRE( expectedCreationTime.dwLowDateTime != 0 );
    REQUIRE( expectedCreationTime.dwHighDateTime != 0 );

    // Access time.
    const BitPropVariant expectedATime = item.itemProperty( BitProperty::ATime );
    REQUIRE( expectedATime.isFileTime() );

    const FILETIME expectedAccessTime = expectedATime.getFileTime();
    REQUIRE( expectedAccessTime.dwLowDateTime != 0 );
    REQUIRE( expectedAccessTime.dwHighDateTime != 0 );

    // Modified time.
    const BitPropVariant expectedMTime = item.itemProperty( BitProperty::MTime );
    REQUIRE( expectedMTime.isFileTime() );

    const FILETIME expectedModifiedTime = expectedMTime.getFileTime();
    REQUIRE( expectedModifiedTime.dwLowDateTime != 0 );
    REQUIRE( expectedModifiedTime.dwHighDateTime != 0 );
#else
    const auto expectedModifiedTime = asUnixTimestamp( item.lastWriteTime() );
#endif

    const TempTestDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Output directory: " << testOutDir )

    REQUIRE_NOTHROW( info.extractTo( testOutDir ) );

    const auto expectedFile = testOutDir.path() / clouds.name;
#ifdef _WIN32
    FILETIME creationTime{};
    FILETIME accessTime{};
    FILETIME modifiedTime{};
    REQUIRE( get_file_time( expectedFile, creationTime, accessTime, modifiedTime ) );

    REQUIRE( CompareFileTime( &creationTime, &expectedCreationTime ) == 0 );
    REQUIRE( CompareFileTime( &accessTime, &expectedAccessTime ) == 0 );
    REQUIRE( CompareFileTime( &modifiedTime, &expectedModifiedTime ) == 0 );
#else
    auto modifiedTime = asUnixTimestamp( fs::last_write_time( expectedFile ) );
    INFO( "System clock's now: " << asUnixTimestamp( std::chrono::system_clock::now() ) )
    INFO( "File clock's now:   " << asUnixTimestamp( fs::file_time_type::clock::now() ) )
    REQUIRE( modifiedTime == expectedModifiedTime );
#endif

    REQUIRE( fs::remove( expectedFile ) );
}

#ifdef _WIN32
// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting an archive not having time metadata should use current time",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    // Note: the clouds.jpg.zip file was created without storing access/creation time metadata.
    const auto arcFileName = fs::path{ clouds.name }.concat( ".zip" );

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    BitArchiveReader info( test::sevenzipLib(), inputArchive, BitFormat::Zip );

    const auto item = info.itemAt( 0 );

    // Creation time must be an empty FILETIME.
    const BitPropVariant storedCTime = item.itemProperty( BitProperty::CTime );
    REQUIRE( storedCTime.isFileTime() );

    const FILETIME storedCreationTime = storedCTime.getFileTime();
    REQUIRE( storedCreationTime.dwLowDateTime == 0 );
    REQUIRE( storedCreationTime.dwHighDateTime == 0 );

    // Access time must be an empty FILETIME.
    const BitPropVariant storedATime = item.itemProperty( BitProperty::ATime );
    REQUIRE( storedATime.isFileTime() );

    const FILETIME storedAccessTime = storedATime.getFileTime();
    REQUIRE( storedAccessTime.dwLowDateTime == 0 );
    REQUIRE( storedAccessTime.dwHighDateTime == 0 );

    // Getting the current system time (to be compared with the extracted creation/access time metadata).
    FILETIME expectedTime{};
    GetSystemTimeAsFileTime( &expectedTime );

    // Modified time (the archive contains this metadata, so it is expected to be set in the extracted file).
    const BitPropVariant expectedMTime = item.itemProperty( BitProperty::MTime );
    REQUIRE( expectedMTime.isFileTime() );

    const FILETIME expectedModifiedTime = expectedMTime.getFileTime();
    REQUIRE( expectedModifiedTime.dwLowDateTime != 0 );
    REQUIRE( expectedModifiedTime.dwHighDateTime != 0 );

    TempTestDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Output directory: " << testOutDir )

    REQUIRE_NOTHROW( info.extractTo( testOutDir ) );

    const auto expectedFile = testOutDir.path() / clouds.name;
    FILETIME creationTime{};
    FILETIME accessTime{};
    FILETIME modifiedTime{};
    REQUIRE( get_file_time( expectedFile, creationTime, accessTime, modifiedTime ) );

    REQUIRE( CompareFileTime( &creationTime, &expectedTime ) >= 0 );
    REQUIRE( CompareFileTime( &accessTime, &expectedTime ) >= 0 );
    REQUIRE( CompareFileTime( &modifiedTime, &expectedModifiedTime ) == 0 );

    REQUIRE( fs::remove( expectedFile ) );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting a file with a comment should preserve it",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "file_comment" };

    TestType inputArchive{};
    getInputArchive( "commented.7z", inputArchive );
    BitArchiveReader info( test::sevenzipLib(), inputArchive, BitFormat::SevenZip );

    TempTestDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Output directory: " << testOutDir )

    REQUIRE_NOTHROW( info.extractTo( testOutDir ) );

    const auto expectedFile = testOutDir.path() / "commented.jpg";
    REQUIRE( fs::exists( expectedFile ) );

    std::wstring comment = getFileComment( expectedFile );
    REQUIRE(
        comment == LR"({"data":{"pictureId":"738298be446d47f4b3933a4cc68ab6a2","appversion":"8.0.0",)"
        LR"("stickerId":"","filterId":"","infoStickerId":"","imageEffectId":"",)"
        LR"("playId":"","activityName":"","os":"android","product":"retouch"},)"
        LR"("source_type":"douyin_beauty_me"})"
    );
    REQUIRE( fs::remove( expectedFile ) );
}
#endif

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Using extraction callbacks", "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#endif

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );

        std::uint64_t totalSize = 0;
        info.setTotalCallback(
            [ &totalSize ] ( std::uint64_t total ) -> void {
                totalSize = total;
            }
        );

        std::vector< std::uint64_t > progressValues;
        info.setProgressCallback(
            [ &progressValues ] ( std::uint64_t progress ) -> bool {
                progressValues.push_back( progress );
                return true;
            }
        );

        double finalRatio = 0.0;
        info.setRatioCallback(
            [ &finalRatio ] ( std::uint64_t processedInput, std::uint64_t processedOutput ) -> void {
                if ( processedOutput == 0 ) {
                    return;
                }
                finalRatio = static_cast< double >( processedInput ) / static_cast< double >( processedOutput );
            }
        );

        std::vector< tstring > visitedFiles;
        info.setFileCallback(
            [ &visitedFiles ] ( const tstring& file ) -> void {
                visitedFiles.push_back( file );
            }
        );

        const auto& expectedItems = multipleItemsContent().items;

        SECTION( "When extracting to the filesystem" ) {
            requireExtractsToFilesystem( info, expectedItems );
        }

        SECTION( "When extracting to the filesystem with an empty indices vector" ) {
            requireExtractsToFilesystemEmptyIndices( info, expectedItems );
        }

        SECTION( "When extracting to a buffer map" ) {
            requireExtractsToBuffersMap( info, expectedItems );
        }

        std::vector< tstring > expectedPaths;
        expectedPaths.reserve( expectedItems.size() );
        for ( const auto& expectedItem : expectedItems ) {
            if ( expectedItem.fileInfo.type != fs::file_type::directory ) {
                expectedPaths.push_back( to_tstring( expectedItem.inArchivePath ) );
            }
        }

        // Checking that the total callback was called at least once (it should be called only once by 7-Zip).
        REQUIRE( totalSize == multipleItemsContent().size );

        // Checking that the progress callback was called at least once.
        REQUIRE( !progressValues.empty() );

        using namespace Catch::Matchers;

        // For some reason, the Tar format makes the progress decrease in some cases,
        // and it is not always less than the total size ¯\_(ツ)_/¯.
        if ( testArchive.format != BitFormat::Tar ) {
            // Checking that the values reported by the progress callback are increasing,
            // and less than or equal to the total size.
            std::uint64_t lastProgress = 0;
            for ( std::uint64_t progress : progressValues ) {
                REQUIRE( progress >= lastProgress );
                REQUIRE( progress <= totalSize );
                lastProgress = progress;
            }

            // TODO: Fix 7-Zip not calling the progress callback on last extracted block in these formats
            if ( testArchive.format != BitFormat::Iso && testArchive.format != BitFormat::Rar5 ) {
                REQUIRE( progressValues.back() == totalSize );
            }
        }

        // Checking that the final value reported by the ratio callback makes sense.
        if ( testArchive.format == BitFormat::Tar ) { // The Tar format has some space overhead, so the ratio is > 1.0.
            REQUIRE( finalRatio > 1.0 );
        } else if ( testArchive.format == BitFormat::Iso || testArchive.format == BitFormat::Wim ) {
            REQUIRE( finalRatio == 1.0 ); // In this case, it is safe to use equal comparison with doubles.
        } else {
            REQUIRE( finalRatio < 1.0 );
        }

        // Checking that the paths reported by the file callback are the ones we expected.
        REQUIRE_THAT( visitedFiles, UnorderedEquals( expectedPaths ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Aborting the extraction via the progress callback",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );

        // Returning false from the progress callback must abort the ongoing operation.
        bool progressCalled = false;
        info.setProgressCallback(
            [ &progressCalled ] ( std::uint64_t ) -> bool {
                progressCalled = true;
                return false;
            }
        );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        REQUIRE_THROWS_CODE( info.extractTo( testOutDir ), std::errc::operation_canceled );

        // The operation must have been aborted from within the progress callback,
        // and not have failed for some other reason.
        REQUIRE( progressCalled );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Finding files in an archive", "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );

        REQUIRE( info.find( BIT7Z_STRING( "" ) ) == info.cend() );
        REQUIRE_FALSE( info.contains( BIT7Z_STRING( "" ) ) );
        REQUIRE( info.find( BIT7Z_STRING( "/" ) ) == info.cend() );
        REQUIRE_FALSE( info.contains( BIT7Z_STRING( "/" ) ) );
        REQUIRE( info.find( BIT7Z_STRING( "\\" ) ) == info.cend() );
        REQUIRE_FALSE( info.contains( BIT7Z_STRING( "\\" ) ) );
        REQUIRE( info.find( BIT7Z_STRING( "non_existing_item" ) ) == info.cend() );
        REQUIRE_FALSE( info.contains( BIT7Z_STRING( "non_existing_item" ) ) );

        REQUIRE( info.find( BIT7Z_STRING( "folder/clouds.jpg" ) ) != info.cend() );
        REQUIRE( info.contains( BIT7Z_STRING( "folder/clouds.jpg" ) ) );
#ifdef _WIN32
        REQUIRE( info.find( BIT7Z_STRING( "folder\\clouds.jpg" ) ) != info.cend() );
        REQUIRE( info.contains( BIT7Z_STRING( "folder\\clouds.jpg" ) ) );
#else
        REQUIRE( info.find( BIT7Z_STRING( "folder\\clouds.jpg" ) ) == info.cend() );
        REQUIRE_FALSE( info.contains( BIT7Z_STRING( "folder\\clouds.jpg" ) ) );
#endif

        // findByName matches the item's name rather than its full path, so it locates a nested item
        // by its bare name, where find (which matches the path) would not.
        const auto byName = info.findByName( BIT7Z_STRING( "clouds.jpg" ) );
        REQUIRE( byName != info.cend() );
        REQUIRE( byName->name() == BIT7Z_STRING( "clouds.jpg" ) );
        REQUIRE( byName == info.find( BIT7Z_STRING( "folder/clouds.jpg" ) ) );
        REQUIRE( info.find( BIT7Z_STRING( "clouds.jpg" ) ) == info.cend() );

        REQUIRE( info.findByName( BIT7Z_STRING( "non_existing_item" ) ) == info.cend() );
        // Conversely, a full path is not matched by name.
        REQUIRE( info.findByName( BIT7Z_STRING( "folder/clouds.jpg" ) ) == info.cend() );

        // Every item in the archive is locatable by its own full path.
        requireItemsFindableByPath( info );
    }
}

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Finding files with Unicode names in an archive",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "unicode." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );

        REQUIRE( info.find( BIT7Z_STRING( "σύννεφα.jpg" ) ) != info.cend() );
        REQUIRE( info.contains( BIT7Z_STRING( "σύννεφα.jpg" ) ) );
        REQUIRE( info.find( BIT7Z_STRING( "𤭢.txt" ) ) != info.cend() );
        REQUIRE( info.contains( BIT7Z_STRING( "𤭢.txt" ) ) );
        REQUIRE( info.find( BIT7Z_STRING( "non_existing_item" ) ) == info.cend() );
        REQUIRE_FALSE( info.contains( BIT7Z_STRING( "non_existing_item" ) ) );

        // Every item is locatable by its (Unicode) full path.
        requireItemsFindableByPath( info );
    }
}

#endif

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extract to raw data callback",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "lzh", BitFormat::Lzh },
        TestInputFormat{ "lzma", BitFormat::Lzma },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "lzh", BitFormat::Lzh },
        TestInputFormat{ "lzma", BitFormat::Lzma },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#endif

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = fs::path{ clouds.name }.concat( "." + testArchive.extension );

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const Bit7zLibrary lib{ test::sevenzipLibPath() };
        const BitArchiveReader info( lib, inputArchive, testArchive.format );

        std::size_t totalSize = 0;
        std::uint32_t crcValue = 0;

        info.extractTo(
            [ &totalSize, &crcValue ] ( const byte_t* data, std::size_t length ) -> bool {
                totalSize += length;
                crcValue = crc32( data, length, crcValue );
                return true;
            }
        );
        REQUIRE( totalSize == clouds.size );
        REQUIRE( crcValue == clouds.crc32 );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Reading the archive from the start of the input file",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "lzh", BitFormat::Lzh },
        TestInputFormat{ "lzma", BitFormat::Lzma },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "lzh", BitFormat::Lzh },
        TestInputFormat{ "lzma", BitFormat::Lzma },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#endif

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = fs::path{ clouds.name }.concat( "." + testArchive.extension );

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const Bit7zLibrary lib{ test::sevenzipLibPath() };
        REQUIRE_NOTHROW( BitArchiveReader( lib, inputArchive, ArchiveStartOffset::FileStart, testArchive.format ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Scanning a file for the archive start",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "multiple_nested2.tar";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const Bit7zLibrary lib{ test::sevenzipLibPath() };

#ifdef BIT7Z_AUTO_FORMAT
    SECTION( "Detecting the format from the file extension (extension is correct)" ) {
        const BitArchiveReader reader( lib, inputArchive, ArchiveStartOffset::None );
        REQUIRE_NOTHROW( reader.detectedFormat() == BitFormat::Tar );
    }
#endif

    SECTION(
        "Opening the archive with the Zip format succeeds, "
        "as 7-Zip will scan the input Tar archive and find the nested Zip archive"
    ) {
        REQUIRE_NOTHROW( BitArchiveReader( lib, inputArchive, ArchiveStartOffset::None, BitFormat::Zip ) );
    }

    SECTION(
        "Opening the archive with the 7z format succeeds, "
        "as 7-Zip will scan the input Tar archive and find the nested 7z archive"
    ) {
        REQUIRE_NOTHROW( BitArchiveReader( lib, inputArchive, ArchiveStartOffset::None, BitFormat::SevenZip ) );
    }

    SECTION(
        "The BZip2 format doesn't support scanning the input file for the archive start,"
        "so the opening must fail even though the Tar archive contains a BZip2 file"
    ) {
        REQUIRE_THROWS( BitArchiveReader( lib, inputArchive, ArchiveStartOffset::None, BitFormat::BZip2 ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Checking only the file start for the archive start",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "multiple_nested2.tar";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const Bit7zLibrary lib{ test::sevenzipLibPath() };

#ifdef BIT7Z_AUTO_FORMAT
    SECTION( "Detecting the format from the file extension (extension is correct)" ) {
        const BitArchiveReader reader( lib, inputArchive, ArchiveStartOffset::FileStart );
        REQUIRE_NOTHROW( reader.detectedFormat() == BitFormat::Tar );
    }
#endif

    SECTION( "Opening the Tar file as a Zip archive fails, as 7-Zip will check the format only at the file start" ) {
        REQUIRE_THROWS( BitArchiveReader( lib, inputArchive, ArchiveStartOffset::FileStart, BitFormat::Zip ) );
    }

    SECTION( "Opening the Tar file as a 7z archive fails, as 7-Zip will check the format only at the file start" ) {
        REQUIRE_THROWS( BitArchiveReader( lib, inputArchive, ArchiveStartOffset::FileStart, BitFormat::SevenZip ) );
    }

    SECTION( "Opening the Tar file as a BZip2 archive fails, as 7-Zip will check the format only at the file start" ) {
        REQUIRE_THROWS( BitArchiveReader( lib, inputArchive, ArchiveStartOffset::FileStart, BitFormat::BZip2 ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE(
    "BitInputArchive: Reading the main subfile of an archive with a specified archive start offset",
    "[bitinputarchive]"
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "split" };

    // A split archive exposes its reassembled content as the main subfile; here that content is a 7z
    // archive located at the start of the subfile stream, so it can be opened both by checking only
    // the file start and by scanning the whole subfile stream.
    const fs::path splitArcFileName = "clouds.jpg.7z.001";
    const BitArchiveReader splitArchive( test::sevenzipLib(), splitArcFileName.string< tchar >(), BitFormat::Split );

    const auto archiveStart = GENERATE( ArchiveStartOffset::FileStart, ArchiveStartOffset::None );

    const BitArchiveReader innerArchive( test::sevenzipLib(), splitArchive, archiveStart, BitFormat::SevenZip );
    REQUIRE( innerArchive.itemsCount() == singleFileContent().fileCount );
    REQUIRE_ARCHIVE_TESTS( innerArchive );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Reading a subfile by index of an archive with a specified archive start offset",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "multiple_nested2.tar";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const Bit7zLibrary lib{ test::sevenzipLibPath() };

    // The outer Tar archive stores nested archives as its items; Tar supports retrieving each item's
    // stream, which can then be opened as a nested archive at the specified archive start offset.
    const BitArchiveReader outerArchive( lib, inputArchive, BitFormat::Tar );

    const auto archiveStart = GENERATE( ArchiveStartOffset::FileStart, ArchiveStartOffset::None );

    SECTION( "Opening the nested 7z subfile" ) {
        const BitArchiveReader innerArchive( lib, outerArchive, 1U, archiveStart, BitFormat::SevenZip );
        REQUIRE_NOTHROW( innerArchive.test() );
    }

    SECTION( "Opening the nested zip subfile" ) {
        const BitArchiveReader innerArchive( lib, outerArchive, 2U, archiveStart, BitFormat::Zip );
        REQUIRE_NOTHROW( innerArchive.test() );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE(
    "BitInputArchive: Reading a subfile whose archive data does not start at the subfile stream start",
    "[bitinputarchive]"
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "multiple_nested2.tar";

    const Bit7zLibrary lib{ test::sevenzipLibPath() };

    // TODO: Add some fixture archives with an embedded archive at a non-zero offset *within* a subfile stream.
    // Until then, build one at runtime: an outer Tar whose only item is multiple_nested2.tar (itself a Tar
    // holding a nested 7z). The Tar handler returns that item's bytes verbatim, so within the subfile stream
    // the nested 7z starts well after offset 0 (which is just a Tar header).
    // This way we can test a case where the two ArchiveStartOffset values behave differently, proving that
    // FileStart restricts the scan.
    buffer_t outerArchiveBuffer;
    BitArchiveWriter writer{ lib, BitFormat::Tar };
    writer.addFile( arcFileName.string< tchar >() );
    writer.compressTo( outerArchiveBuffer );

    const BitArchiveReader outerArchive( lib, outerArchiveBuffer, BitFormat::Tar );

    // Scanning the whole subfile stream (None) finds the nested 7z archive, so the opening succeeds...
    const BitArchiveReader innerArchive( lib, outerArchive, 0U, ArchiveStartOffset::None, BitFormat::SevenZip );
    REQUIRE_NOTHROW( innerArchive.test() );

    // ...while checking only the file start of the same subfile stream (FileStart) sees a Tar header
    // instead of a 7z signature, so the opening must fail.
    REQUIRE_THROWS( BitArchiveReader( lib, outerArchive, 0U, ArchiveStartOffset::FileStart, BitFormat::SevenZip ) );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Reading a nested archive with wrong extension",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" };

    const fs::path arcFileName = "nested_wrong_extension.zip"; // 7z file with zip extension

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const Bit7zLibrary lib{ test::sevenzipLibPath() };

    SECTION( "Checking archive start at input file start" ) {
#ifdef BIT7Z_AUTO_FORMAT
        const BitArchiveReader reader( lib, inputArchive, ArchiveStartOffset::FileStart );
        REQUIRE( reader.detectedFormat() == BitFormat::SevenZip );
        REQUIRE_NOTHROW( reader.test() );
#else
        REQUIRE_THROWS( BitArchiveReader( lib, inputArchive, ArchiveStartOffset::FileStart, BitFormat::Zip ) );
#endif
    }

    SECTION( "Checking archive start by scanning through the input file" ) {
#ifdef BIT7Z_AUTO_FORMAT
        const BitArchiveReader reader( lib, inputArchive, ArchiveStartOffset::None );
#ifdef BIT7Z_DETECT_FROM_EXTENSION
        if ( reader.archivePath().empty() ) {
            REQUIRE( reader.detectedFormat() == BitFormat::SevenZip );
            REQUIRE_NOTHROW( reader.test() );
        } else {
            REQUIRE( reader.detectedFormat() == BitFormat::Zip );
            REQUIRE_THROWS( reader.test() );
        }
#else
        REQUIRE( reader.detectedFormat() == BitFormat::SevenZip );
        REQUIRE_NOTHROW( reader.test() );
#endif
#else
        const BitArchiveReader reader( lib, inputArchive, ArchiveStartOffset::None, BitFormat::Zip );
        REQUIRE_THROWS( reader.test() );
#endif
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Reading a nested zip archive",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "nested" };

    const fs::path arcFileName = "nested_zip.zip";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const Bit7zLibrary lib{ test::sevenzipLibPath() };

    SECTION( "Checking archive start at input file start" ) {
#ifdef BIT7Z_AUTO_FORMAT
        const BitArchiveReader reader( lib, inputArchive, ArchiveStartOffset::FileStart );
        REQUIRE( reader.detectedFormat() == BitFormat::Zip );
#else
        const BitArchiveReader reader( lib, inputArchive, ArchiveStartOffset::FileStart, BitFormat::Zip );
#endif
        REQUIRE_NOTHROW( reader.test() );
        REQUIRE( reader.contains( italy.name ) );
    }

    SECTION( "Checking archive start by scanning through the input file" ) {
#ifdef BIT7Z_AUTO_FORMAT
        const BitArchiveReader reader( lib, inputArchive, ArchiveStartOffset::None );
        REQUIRE( reader.detectedFormat() == BitFormat::Zip );
#else
        const BitArchiveReader reader( lib, inputArchive, ArchiveStartOffset::None, BitFormat::Zip );
#endif
        REQUIRE_NOTHROW( reader.test() );
        REQUIRE( reader.contains( italy.name ) );
    }
}

#ifdef _WIN32
TEMPLATE_TEST_CASE(
    "BitInputArchive: Reading a zip archive using a different encoding",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName = "codepage.zip";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const Bit7zLibrary lib{ test::sevenzipLibPath() };
    const BitArchiveReader reader{ lib, inputArchive, BitFormat::Zip };
    REQUIRE( reader.itemsCount() == 1 );

    constexpr auto expectedItemName = BIT7Z_NATIVE_STRING( "ユニコード.pdf" );

    // The archive uses the Shift-JS encoding (Codepage 932) for the file names.
    // If we do not set the codepage to be used, 7-Zip will report a wrongly-encoded string for the name.
    REQUIRE_FALSE( reader.itemAt( 0 ).nativePath() == expectedItemName );

    // Setting the correct codepage will make 7-Zip correctly encode the string.
    reader.useFormatProperty( L"cp", 932u );
    REQUIRE( reader.itemAt( 0 ).nativePath() == expectedItemName );

    TempTestDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Output directory: " << testOutDir )

    REQUIRE_NOTHROW( reader.extractTo( testOutDir ) );

    REQUIRE( fs::exists( expectedItemName ) );
    REQUIRE( fs::remove( expectedItemName ) );
}
#endif

namespace {
auto to_string( FolderPathPolicy policy ) -> const char* {
    switch ( policy ) {
        case FolderPathPolicy::KeepName:
            return "FolderPathPolicy::KeepName";
        case FolderPathPolicy::KeepPath:
            return "FolderPathPolicy::KeepPath";
        case FolderPathPolicy::Strip:
        default:
            return "FolderPathPolicy::Strip";
    }
}
} // namespace

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting a folder from an archive",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

#ifdef _WIN32
    const auto folderPath = GENERATE(
        as< tstring >(),
        BIT7Z_STRING( "folder\\subfolder2" ),
        BIT7Z_STRING( "folder\\subfolder2\\" ),
        BIT7Z_STRING( "folder/subfolder2" ),
        BIT7Z_STRING( "folder/subfolder2/" ),
        BIT7Z_STRING( "folder/subfolder2\\" ),
        BIT7Z_STRING( "folder\\subfolder2/" )
    );
#else
    const auto folderPath = GENERATE( as< tstring >(), "folder/subfolder2", "folder/subfolder2/" );
#endif

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#endif

    const auto policy = GENERATE( FolderPathPolicy::Strip, FolderPathPolicy::KeepName, FolderPathPolicy::KeepPath );

    const auto expectedRoot = [&policy, &folderPath]() -> fs::path {
        switch ( policy ) {
            case FolderPathPolicy::KeepName:
                return fs::path{ "subfolder2" };
            case FolderPathPolicy::KeepPath:
                return fs::path{ folderPath };
            case FolderPathPolicy::Strip:
            default:
                return fs::path{};
        }
    }();

    const ExpectedItems expectedItems{
        ExpectedItem{ quickBrown, expectedRoot / quickBrown.name, false },
        ExpectedItem{ homework, expectedRoot / homework.name, false },
        ExpectedItem{ frequency, expectedRoot / frequency.name, false }
    };

    DYNAMIC_SECTION(
        "Folder path: " << Catch::StringMaker< tstring >::convert( folderPath ) << ", "
        "Archive format: " << testArchive.extension << ", "
        "Policy: " << to_string( policy )
    ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        REQUIRE_NOTHROW( info.extractFolderTo( testOutDir, folderPath, policy ) );
        for ( const auto& expected : expectedItems ) {
            REQUIRE_FILESYSTEM_ITEM( expected );
        }
        if ( policy != FolderPathPolicy::Strip ) {
            REQUIRE( fs::is_empty( testOutDir.path() / expectedRoot ) );
            REQUIRE( fs::remove( testOutDir.path() / expectedRoot ) );
            if ( policy == FolderPathPolicy::KeepPath ) {
                REQUIRE( fs::remove( testOutDir.path() / folder.name ) );
            }
        }
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEST_CASE(
    "BitInputArchive: Extracting a folder honors retainDirectories on the remainder",
    "[bitinputarchive]"
) {
    // The folder "base/x" contains a nested file (base/x/deep/file1.txt) and a file directly
    // inside it (base/x/file2.txt). The FolderPathPolicy shapes the prefix (up to "x"); when
    // retainDirectories() is false, the remainder below the folder is flattened to its filename.
    const buffer_t content1( 8, static_cast< byte_t >( 0xD1 ) );
    const buffer_t content2( 16, static_cast< byte_t >( 0xD2 ) );

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );
    const auto policy = GENERATE( FolderPathPolicy::Strip, FolderPathPolicy::KeepName, FolderPathPolicy::KeepPath );
    const auto retain = GENERATE( true, false );

    const fs::path prefix = [ & ]() -> fs::path {
        switch ( policy ) {
            case FolderPathPolicy::KeepName:
                return fs::path{ "x" };
            case FolderPathPolicy::KeepPath:
                return fs::path{ "base" } / "x";
            case FolderPathPolicy::Strip:
            default:
                return fs::path{};
        }
    }();

    DYNAMIC_SECTION(
        "Archive format: " << testFormat.extension << ", "
        "Policy: " << to_string( policy ) << ", "
        "retainDirectories: " << ( retain ? "true" : "false" )
    ) {
        BitArchiveWriter writer{ test::sevenzipLib(), testFormat.format };
        writer.addFile( content1, BIT7Z_STRING( "base/x/deep/file1.txt" ) );
        writer.addFile( content2, BIT7Z_STRING( "base/x/file2.txt" ) );

        buffer_t archiveBuffer;
        writer.compressTo( archiveBuffer );

        BitArchiveReader reader{ test::sevenzipLib(), archiveBuffer, testFormat.format };
        reader.setRetainDirectories( retain );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        REQUIRE_NOTHROW( reader.extractFolderTo( testOutDir, BIT7Z_STRING( "base/x" ), policy ) );

        // file1 is nested: retained → prefix/deep/file1.txt, flattened → prefix/file1.txt.
        const auto file1Nested = testOutDir.path() / prefix / "deep" / "file1.txt";
        const auto file1Flat = testOutDir.path() / prefix / "file1.txt";
        const auto expectedFile1 = retain ? file1Nested : file1Flat;
        const auto unexpectedFile1 = retain ? file1Flat : file1Nested;
        // file2 is directly inside the folder, so flattening it is a no-op.
        const auto file2 = testOutDir.path() / prefix / "file2.txt";

        REQUIRE( fs::exists( expectedFile1 ) );
        REQUIRE_FALSE( fs::exists( unexpectedFile1 ) );
        REQUIRE( fs::exists( file2 ) );
        REQUIRE( loadFile( expectedFile1 ) == content1 );
        REQUIRE( loadFile( file2 ) == content2 );

        for ( const auto& entry : fs::directory_iterator( testOutDir.path() ) ) {
            fs::remove_all( entry );
        }
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting an empty folder from an archive",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

#ifdef _WIN32
    const auto folderPath = GENERATE(
        as< tstring >(),
        BIT7Z_STRING( "empty" ),
        BIT7Z_STRING( "empty/" ),
        BIT7Z_STRING( "empty\\" ),
        BIT7Z_STRING( "folder\\subfolder" ),
        BIT7Z_STRING( "folder\\subfolder\\" ),
        BIT7Z_STRING( "folder/subfolder" ),
        BIT7Z_STRING( "folder/subfolder/" ),
        BIT7Z_STRING( "folder/subfolder\\" ),
        BIT7Z_STRING( "folder\\subfolder/" )
    );
#else
    const auto folderPath = GENERATE( as< tstring >(), "empty", "empty/", "folder/subfolder", "folder/subfolder/" );
#endif

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#endif

    const auto policy = GENERATE( FolderPathPolicy::Strip, FolderPathPolicy::KeepName, FolderPathPolicy::KeepPath );

    const auto expectedRoot = [&policy, &folderPath]() -> fs::path {
        switch ( policy ) {
            case FolderPathPolicy::KeepName:
                return fs::path{ folderPath.rfind( BIT7Z_STRING( "empty" ), 0 ) == 0 ? "empty" : "subfolder" };
            case FolderPathPolicy::KeepPath:
                return fs::path{ folderPath };
            case FolderPathPolicy::Strip:
            default:
                return fs::path{};
        }
    }();

    DYNAMIC_SECTION(
        "Folder path: " << Catch::StringMaker< tstring >::convert( folderPath ) << ", "
        "Archive format: " << testArchive.extension << ", "
        "Policy: " << to_string( policy )
    ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        if ( policy == FolderPathPolicy::Strip ) {
            REQUIRE_THROWS( info.extractFolderTo( testOutDir, folderPath, policy ) );
        } else {
            REQUIRE_NOTHROW( info.extractFolderTo( testOutDir, folderPath, policy ) );
            REQUIRE( fs::exists( testOutDir.path() / expectedRoot ) );
            REQUIRE( fs::is_empty( testOutDir.path() / expectedRoot ) );
            REQUIRE( fs::remove( testOutDir.path() / expectedRoot ) );
            if ( policy == FolderPathPolicy::KeepPath ) {
                const auto parentPath = expectedRoot.has_filename()
                    ? expectedRoot.parent_path()
                    : expectedRoot.parent_path().parent_path();
                if ( !parentPath.empty() ) {
                    REQUIRE( fs::is_empty( testOutDir.path() / parentPath ) );
                    REQUIRE( fs::remove( testOutDir.path() / parentPath ) );
                }
            }
        }
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting a folder from an archive (duplicate items)",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "duplicate" };

#ifdef _WIN32
    const auto folderPath = GENERATE(
        as< tstring >(),
        BIT7Z_STRING( "duplicate" ),
        BIT7Z_STRING( "duplicate\\" ),
        BIT7Z_STRING( "duplicate/" )
    );
#else
    const auto folderPath = GENERATE( as< tstring >(), "duplicate", "duplicate/" );
#endif

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#endif

    const auto policy = GENERATE( FolderPathPolicy::Strip, FolderPathPolicy::KeepName, FolderPathPolicy::KeepPath );

    const auto expectedRoot = [&policy, &folderPath]() -> fs::path {
        switch ( policy ) {
            case FolderPathPolicy::KeepName:
            case FolderPathPolicy::KeepPath:
                return fs::path{ folderPath };
            case FolderPathPolicy::Strip:
            default:
                return fs::path{};
        }
    }();

    const ExpectedItem expectedItem{ italy, expectedRoot / italy.name, false };

    DYNAMIC_SECTION(
        "Folder path: " << Catch::StringMaker< tstring >::convert( folderPath ) << ", "
        "Archive format: " << testArchive.extension << ", "
        "Policy: " << to_string( policy )
    ) {
        const fs::path arcFileName = "duplicate." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        REQUIRE_NOTHROW( info.extractFolderTo( testOutDir, folderPath, policy ) );
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
        if ( policy != FolderPathPolicy::Strip ) {
            REQUIRE( fs::is_empty( testOutDir.path() / expectedRoot ) );
            REQUIRE( fs::remove( testOutDir.path() / expectedRoot ) );
        }
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting a folder from an archive (duplicate items, fake extension)",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "duplicate" };

#ifdef _WIN32
    const auto folderPath = GENERATE(
        as< tstring >(),
        BIT7Z_STRING( "clouds.jpg" ),
        BIT7Z_STRING( "clouds.jpg\\" ),
        BIT7Z_STRING( "clouds.jpg/" )
    );
#else
    const auto folderPath = GENERATE( as< tstring >(), "clouds.jpg", "clouds.jpg/" );
#endif

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#endif

    const auto policy = GENERATE( FolderPathPolicy::Strip, FolderPathPolicy::KeepName, FolderPathPolicy::KeepPath );

    const auto expectedRoot = [&policy, &folderPath]() -> fs::path {
        switch ( policy ) {
            case FolderPathPolicy::KeepName:
            case FolderPathPolicy::KeepPath:
                return fs::path{ folderPath };
            case FolderPathPolicy::Strip:
            default:
                return fs::path{};
        }
    }();

    const ExpectedItem expectedItem{ frequency, expectedRoot / frequency.name, false };

    DYNAMIC_SECTION(
        "Folder path: " << Catch::StringMaker< tstring >::convert( folderPath ) << ", "
        "Archive format: " << testArchive.extension << ", "
        "Policy: " << to_string( policy )
    ) {
        const fs::path arcFileName = "duplicate." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        REQUIRE_NOTHROW( info.extractFolderTo( testOutDir, folderPath, policy ) );
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
        if ( policy != FolderPathPolicy::Strip ) {
            REQUIRE( fs::is_empty( testOutDir.path() / expectedRoot ) );
            REQUIRE( fs::remove( testOutDir.path() / expectedRoot ) );
        }
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

TEMPLATE_TEST_CASE(
    "BitInputArchive: Extracting a non-existing folder from an archive should throw an exception",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

#ifdef _WIN32
    const auto folderPath = GENERATE(
        as< tstring >(),
        BIT7Z_STRING( "" ),
        BIT7Z_STRING( "/" ),
        BIT7Z_STRING( "\\" ),
        BIT7Z_STRING( "." ),
        BIT7Z_STRING( "./" ),
        BIT7Z_STRING( ".\\" ),
        BIT7Z_STRING( "/./" ),
        BIT7Z_STRING( ".\\" ),
        BIT7Z_STRING( "\\.\\" ),
        BIT7Z_STRING( ".." ),
        BIT7Z_STRING( "../" ),
        BIT7Z_STRING( "..\\" ),
        BIT7Z_STRING( "/../" ),
        BIT7Z_STRING( "\\..\\" ),
        BIT7Z_STRING( "/folder" ),
        BIT7Z_STRING( "/folder/" ),
        BIT7Z_STRING( "\\folder/" ),
        BIT7Z_STRING( "\\folder\\" ),
        BIT7Z_STRING( "../folder" ),
        BIT7Z_STRING( "..\\folder" ),
        BIT7Z_STRING( "../folder/subfolder2" ),
        BIT7Z_STRING( "..\\folder\\subfolder2" ),
        BIT7Z_STRING( "../folder/./subfolder2" ),
        BIT7Z_STRING( "..\\folder\\..\\subfolder2" ),
        BIT7Z_STRING( "./folder/subfolder2" ),
        BIT7Z_STRING( ".\\folder\\subfolder2" ),
        BIT7Z_STRING( "./folder/../subfolder2" ),
        BIT7Z_STRING( ".\\folder\\..\\subfolder2" ),
        BIT7Z_STRING( "folder/../subfolder2" ),
        BIT7Z_STRING( "folder\\..\\subfolder2" ),
        BIT7Z_STRING( "non-existing" ),
        BIT7Z_STRING( "non/existing" ),
        BIT7Z_STRING( "non\\existing" ),
        BIT7Z_STRING( "folder/sub" ),
        BIT7Z_STRING( "folder\\sub" )
    );
#else
    const auto folderPath = GENERATE(
        as< tstring >(),
        "",
        "/",
        ".",
        "./",
        "/./",
        "/folder",
        "/folder/",
        "../folder",
        "../folder/subfolder2",
        "./folder/subfolder2",
        "./folder/../subfolder2",
        "folder/../subfolder2",
        "non-existing",
        "non/existing",
        "folder/sub"
    );
#endif

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#else
    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "iso", BitFormat::Iso },
        TestInputFormat{ "rar4.rar", BitFormat::Rar },
        TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );
#endif

    const auto policy = GENERATE( FolderPathPolicy::Strip, FolderPathPolicy::KeepName, FolderPathPolicy::KeepPath );

    DYNAMIC_SECTION(
        "Folder path: " << Catch::StringMaker< tstring >::convert( folderPath ) << ", "
        "Archive format: " << testArchive.extension << ", "
        "Policy: " << to_string( policy )
    ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzipLib(), inputArchive, testArchive.format );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        REQUIRE_THROWS( info.extractFolderTo( testOutDir, folderPath ) );
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

TEMPLATE_TEST_CASE(
    "BitInputArchive: Zip slip attacks",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "zip_slip" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "zip_slip." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader reader( test::sevenzipLib(), inputArchive, testArchive.format );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
        REQUIRE_THROWS( reader.extractTo( testOutDir ) );
        REQUIRE_NOTHROW(
            reader.extractTo( testOutDir, []( const BitArchiveItem& item ) -> FilterResult {
                return fs::path{ item.nativePath() }.filename() == BIT7Z_NATIVE_STRING( "evil.txt" )
                    ? FilterResult::SkipItem
                    : FilterResult::ProcessItem;
                } )
        );
        REQUIRE( fs::exists( "good.txt" ) );
        REQUIRE( fs::remove( "good.txt" ) );
        REQUIRE( fs::exists( "folder/clouds.jpg" ) );
        REQUIRE( fs::remove( "folder/clouds.jpg" ) );
        REQUIRE( fs::remove_all( "folder" ) );
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

#ifdef _WIN32
TEMPLATE_TEST_CASE(
    "BitInputArchive: Path sanitization",
    "[bitinputarchive]",
    tstring,
    buffer_t,
    stream_t
) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "path_sanitization" };

    const auto testArchive = GENERATE(
        as< TestInputFormat >(),
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "zip", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "path_sanitization." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader reader( test::sevenzipLib(), inputArchive, testArchive.format );

        const TempTestDirectory testOutDir{ "test_bitinputarchive" };
#ifdef BIT7Z_PATH_SANITIZATION
        REQUIRE_NOTHROW( reader.extractTo( testOutDir ) );
#else
        REQUIRE_THROWS( reader.extractTo( testOutDir ) );
#endif
    }
}
#endif

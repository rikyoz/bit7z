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
#include "utils/filesystem.hpp"
#include "utils/format.hpp"
#include "utils/shared_lib.hpp"
#include "utils/sourcelocation.hpp"

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitexception.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bittypes.hpp>
#include <internal/fs.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <sstream>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

inline auto archive_item( const BitArchiveReader& archive,
                          const ExpectedItem& expectedItem ) -> BitArchiveReader::ConstIterator {
    if ( archive.retainDirectories() ) {
        return archive.find( path_to_tstring( expectedItem.inArchivePath ) );
    }

    return std::find_if( archive.cbegin(), archive.cend(), [ &expectedItem ]( const BitArchiveItem& item ) -> bool {
        return item.name() == expectedItem.fileInfo.name;
    } );
}

void require_extracts_to_filesystem( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    TempTestDirectory testDir{ "test_bitinputarchive" };
    INFO( "Test directory: " << testDir )

    REQUIRE_NOTHROW( info.extractTo( testDir ) );
    if ( expectedItems.empty() ) {
        REQUIRE( fs::is_empty( testDir.path() ) );
    } else {
        for ( const auto& expectedItem : expectedItems ) {
            REQUIRE_FILESYSTEM_ITEM( expectedItem );
        }
    }
}

void require_extracts_items_to_filesystem( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    TempTestDirectory testDir{ "test_bitinputarchive" };
    INFO( "Test directory: " << testDir )

    for ( const auto& expectedItem : expectedItems ) {
        const auto archiveItem = archive_item( info, expectedItem );
        REQUIRE( archiveItem != info.cend() );
        REQUIRE_NOTHROW( info.extractTo( testDir, { archiveItem->index() } ) );
        REQUIRE_FILESYSTEM_ITEM( expectedItem );
    }
    REQUIRE( fs::is_empty( testDir.path() ) );

    std::vector< uint32_t > testIndices( info.itemsCount() );
    std::iota( testIndices.begin(), testIndices.end(), 0 );
    CAPTURE( testIndices );
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

    for ( const auto& expectedItem : expectedItems ) {
        const auto archiveItem = archive_item( info, expectedItem );
        REQUIRE( archiveItem != info.cend() );
        // The vector of indices contains a valid index, and an invalid one, so the extraction should fail.
        REQUIRE_THROWS( info.extractTo( testDir, { archiveItem->index(), info.itemsCount() } ) );
        REQUIRE( fs::is_empty( testDir.path() ) );
    }

    REQUIRE_THROWS( info.extractTo( testDir, { info.itemsCount() } ) );
    REQUIRE( fs::is_empty( testDir.path() ) );

    REQUIRE_THROWS( info.extractTo( testDir, { std::numeric_limits< uint32_t >::max() } ) );
    REQUIRE( fs::is_empty( testDir.path() ) );
}

void require_extracts_to_buffers_map( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    std::map< tstring, buffer_t > bufferMap;
    REQUIRE_NOTHROW( info.extractTo( bufferMap ) );
    REQUIRE( bufferMap.size() == info.filesCount() );
    for ( const auto& expectedItem : expectedItems ) {
        INFO( "Failed while checking expected item '" << expectedItem.inArchivePath.u8string() << "'" )
        const auto& extractedItem = bufferMap.find( path_to_tstring( expectedItem.inArchivePath ) );
        if ( expectedItem.fileInfo.type != fs::file_type::directory ) {
            REQUIRE( extractedItem != bufferMap.end() );
            REQUIRE( crc32( extractedItem->second ) == expectedItem.fileInfo.crc32 );
        } else {
            REQUIRE( extractedItem == bufferMap.end() );
        }
    }
}

void require_extracts_to_buffers( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    buffer_t outputBuffer;
    for ( const auto& expectedItem : expectedItems ) {
        INFO( "Failed while checking expected item '" << expectedItem.inArchivePath.u8string() << "'" )
        const auto archiveItem = archive_item( info, expectedItem );
        REQUIRE( archiveItem != info.cend() );
        if ( archiveItem->isDir() ) {
            REQUIRE_THROWS( info.extractTo( outputBuffer, archiveItem->index() ) );
            REQUIRE( outputBuffer.empty() );
        } else {
            REQUIRE_NOTHROW( info.extractTo( outputBuffer, archiveItem->index() ) );
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

void require_extracts_to_fixed_buffers( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    // Note: this value must be different from any file size we can encounter inside the tested archives.
    constexpr size_t invalidBufferSize = 42;
    buffer_t invalidBuffer( invalidBufferSize, static_cast< byte_t >( '\0' ) );
    buffer_t outputBuffer;
    for ( const auto& expectedItem : expectedItems ) {
        INFO( "Failed while checking expected item '" << expectedItem.inArchivePath.u8string() << "'" )
        const auto archiveItem = archive_item( info, expectedItem );
        REQUIRE( archiveItem != info.cend() );

        const auto itemIndex = archiveItem->index();
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
        if ( format_has_size_metadata( info.detectedFormat() ) ) {
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
    REQUIRE_THROWS( info.extractTo( nullptr,
                                    std::numeric_limits< std::size_t >::max(),
                                    std::numeric_limits< std::uint32_t >::max() ) );

    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), 0, info.itemsCount() ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), 0, info.itemsCount() + 1 ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), 0, std::numeric_limits< std::uint32_t >::max() ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), invalidBufferSize, info.itemsCount() ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(), invalidBufferSize, info.itemsCount() + 1 ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(),
                                    invalidBufferSize,
                                    std::numeric_limits< std::uint32_t >::max() ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(),
                                    std::numeric_limits< std::size_t >::max(),
                                    info.itemsCount() ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(),
                                    std::numeric_limits< std::size_t >::max(),
                                    info.itemsCount() + 1 ) );
    REQUIRE_THROWS( info.extractTo( invalidBuffer.data(),
                                    std::numeric_limits< std::size_t >::max(),
                                    std::numeric_limits< std::uint32_t >::max() ) );
}

void require_extracts_to_streams( const BitArchiveReader& info, const ExpectedItems& expectedItems ) {
    for ( const auto& expectedItem : expectedItems ) {
        INFO( "Failed while checking expected item '" << expectedItem.inArchivePath.u8string() << "'" )

        const auto archiveItem = archive_item( info, expectedItem );
        REQUIRE( archiveItem != info.cend() );

        const auto itemIndex = archiveItem->index();
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

void require_archive_extracts( const BitArchiveReader& info,
                               const ExpectedItems& expectedItems,
                               const SourceLocation& location ) {
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto& detectedFormat = (info).detectedFormat();
    if ( detectedFormat == BitFormat::Rar || detectedFormat == BitFormat::Rar5 ) {
        return;
    }
#endif

    INFO( "From " << location.file_name() << ":" << location.line() )
    INFO( "Failed while extracting the archive" )

    SECTION( "Extracting to a temporary filesystem folder" ) {
        require_extracts_to_filesystem( info, expectedItems );
    }

    SECTION( "Extracting specific items to a temporary filesystem folder" ) {
        require_extracts_items_to_filesystem( info, expectedItems );
    }

    SECTION( "Extracting to a map of buffers" ) {
        require_extracts_to_buffers_map( info, expectedItems );
    }

    SECTION( "Extracting each item to a buffer" ) {
        require_extracts_to_buffers( info, expectedItems );
    }

    SECTION( "Extracting each item to a fixed size buffer" ) {
        require_extracts_to_fixed_buffers( info, expectedItems );
    }

    SECTION( "Extracting each item to std::ostream" ) {
        require_extracts_to_streams( info, expectedItems );
    }
}

#define REQUIRE_ARCHIVE_EXTRACTS( info, expectedItems ) \
    require_archive_extracts( info, expectedItems, BIT7Z_CURRENT_LOCATION )

void require_archive_extract_fails( const BitArchiveReader& info, const SourceLocation& location ) {
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto& detectedFormat = (info).detectedFormat();
    if ( detectedFormat == BitFormat::Rar || detectedFormat == BitFormat::Rar5 ) {
        return;
    }
#endif

    INFO( "From " << location.file_name() << ":" << location.line() )
    INFO( "Failed while extracting the archive" )

    SECTION( "Extracting to a temporary filesystem folder should fail" ) {
        TempTestDirectory testDir{ "test_bitinputarchive" };
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

#define REQUIRE_ARCHIVE_EXTRACT_FAILS( info ) \
    require_archive_extract_fails( info, BIT7Z_CURRENT_LOCATION )

void require_archive_tests( const BitArchiveReader& info, const SourceLocation& location ) {
    INFO( "From " << location.file_name() << ":" << location.line() )
    INFO( "Failed while testing the archive" )
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto& detectedFormat = (info).detectedFormat();
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

#define REQUIRE_ARCHIVE_TESTS( info ) \
    require_archive_tests( info, BIT7Z_CURRENT_LOCATION )

/* Note: throughout this unit test we will use BitArchiveReader for testing BitInputArchive's specific methods. */

TEST_CASE( "BitInputArchive: Opening a non-existing archive should throw an exception" ) {
    REQUIRE_THROWS( BitArchiveReader{ test::sevenzip_lib(), BIT7Z_STRING( "non-existing.7z" ), BitFormat::SevenZip } );
    REQUIRE_THROWS( BitArchiveReader{ test::sevenzip_lib(), buffer_t{}, BitFormat::SevenZip } );

    fs::ifstream nonExistingStream{ "non-existing.7z" };
    REQUIRE_THROWS( BitArchiveReader{ test::sevenzip_lib(), nonExistingStream, BitFormat::SevenZip } );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting archives containing only a single file",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto testFormat = GENERATE( as< TestInputFormat >(),
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
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const auto arcFileName = fs::path{ clouds.name }.concat( "." + testFormat.extension );

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        if ( format_has_path_metadata( testFormat.format ) || is_filesystem_archive< TestType >::value ) {
            REQUIRE_ARCHIVE_EXTRACTS( info, single_file_content().items );
        } else {
            REQUIRE_ARCHIVE_EXTRACTS( info, no_path_content().items );
        }
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting archives containing multiple files",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_files" };

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "iso", BitFormat::Iso },
                                      TestInputFormat{ "rar", BitFormat::Rar5 },
                                      TestInputFormat{ "tar", BitFormat::Tar },
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "multiple_files." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multiple_files_content().items );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting archives containing multiple items (files and folders)",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multiple_items_content().items );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting archives containing encrypted items",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "encrypted" };

    const auto* const password = BIT7Z_STRING( "helloworld" );

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                      TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                      TestInputFormat{ "aes256.zip", BitFormat::Zip },
                                      TestInputFormat{ "zipcrypto.zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "encrypted." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );

        SECTION( "Opening the archive with no password is allowed, but testing and extraction should throw" ) {
            BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
            REQUIRE_THROWS( info.test() );
            REQUIRE_ARCHIVE_EXTRACT_FAILS( info );

            // After setting the password, the archive can be extracted.
            info.setPassword( password );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info, encrypted_content().items );

            info.clearPassword();
            REQUIRE_THROWS( info.test() );
            REQUIRE_ARCHIVE_EXTRACT_FAILS( info );

            info.setPasswordCallback( [ &password ]() -> tstring {
                return password;
            } );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info, encrypted_content().items );
        }

        SECTION( "Opening the archive with the correct password should allow testing and extraction without issues" ) {
            const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format, password );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info, encrypted_content().items );
        }
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting header-encrypted archives",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "header_encrypted" };

    const auto* const password = BIT7Z_STRING( "helloworld" );

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                      TestInputFormat{ "rar5.rar", BitFormat::Rar5 } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "header_encrypted." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );

        SECTION( "Opening the archive with no password should throw an exception" ) {
            REQUIRE_THROWS( BitArchiveReader( test::sevenzip_lib(), inputArchive, testFormat.format ) );
        }

        SECTION( "Opening the archive with a wrong password should throw an exception" ) {
            REQUIRE_THROWS( BitArchiveReader( test::sevenzip_lib(), inputArchive, testFormat.format,
                                              BIT7Z_STRING( "wrong_password" ) ) );
        }

        SECTION( "Opening the archive with the correct password should pass the tests" ) {
            const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format, password );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info, encrypted_content().items );
        }
    }
}

TEST_CASE( "BitInputArchive: Testing and extracting multi-volume archives", "[bitinputarchive]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "split" };

    SECTION( "Split archive (non-RAR)" ) {
        const auto testFormat = GENERATE( as< TestInputFormat >(),
                                          TestInputFormat{ "7z", BitFormat::SevenZip },
                                          TestInputFormat{ "bz2", BitFormat::BZip2 },
                                          TestInputFormat{ "gz", BitFormat::GZip },
                                          TestInputFormat{ "tar", BitFormat::Tar },
                                          TestInputFormat{ "wim", BitFormat::Wim },
                                          TestInputFormat{ "xz", BitFormat::Xz },
                                          TestInputFormat{ "zip", BitFormat::Zip } );

        DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
            const auto wholeArcFileName = std::string{ "clouds.jpg." } + testFormat.extension;
            const fs::path splitArcFileName = wholeArcFileName + ".001";

            INFO( "Archive file: " << splitArcFileName )

            SECTION( "Opening as a split archive" ) {
                const BitArchiveReader info( test::sevenzip_lib(),
                                             splitArcFileName.string< tchar >(),
                                             BitFormat::Split );
                REQUIRE_ARCHIVE_TESTS( info );

                TempTestDirectory extractionTestDir{ "test_bitinputarchive" };
                REQUIRE_NOTHROW( info.extractTo( extractionTestDir ) );
                REQUIRE( fs::exists( wholeArcFileName ) );
                REQUIRE( fs::remove( wholeArcFileName ) );
            }

            SECTION( "Opening as a whole archive" ) {
                const BitArchiveReader info( test::sevenzip_lib(),
                                             splitArcFileName.string< tchar >(),
                                             testFormat.format );
                REQUIRE_ARCHIVE_TESTS( info );
                REQUIRE_ARCHIVE_EXTRACTS( info, single_file_content().items );
            }
        }
    }

    SECTION( "Multi-volume RAR5" ) {
        const fs::path arcFileName = "clouds.jpg.part1.rar";
        const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Rar5 );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, single_file_content().items );
    }

    SECTION( "Multi-volume RAR4" ) {
        const fs::path arcFileName = "clouds.jpg.rar";
        const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Rar );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, single_file_content().items );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting an empty archive",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "empty" };

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
    // TestInputFormat{ "tar", BitFormat::Tar, 0 }, // TODO: Check why it fails opening
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "empty." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, {} );
    }
}

TEST_CASE( "BitInputArchive: Testing and extracting solid archives", "[bitinputarchive]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "solid" };

    SECTION( "Solid 7z" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "solid.7z" ), BitFormat::SevenZip );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multiple_items_content().items );
    }

    SECTION( "Solid RAR" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "solid.rar" ), BitFormat::Rar5 );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multiple_items_content().items );
    }

    SECTION( "Non solid 7z" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "non_solid.7z" ), BitFormat::SevenZip );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multiple_items_content().items );
    }

    SECTION( "Non-solid RAR" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "non_solid.rar" ), BitFormat::Rar5 );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, multiple_items_content().items );
    }
}

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

TEST_CASE( "BitArchiveReader: Opening RAR archives using the correct RAR format version", "[bitarchivereader]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    SECTION( "Valid RAR archives" ) {
        REQUIRE( test_open_rar_archive( test::sevenzip_lib(), BIT7Z_STRING( "valid.rar4.rar" ) ) == BitFormat::Rar );
        REQUIRE( test_open_rar_archive( test::sevenzip_lib(), BIT7Z_STRING( "valid.rar5.rar" ) ) == BitFormat::Rar5 );
    }

    SECTION( "Non-RAR archive" ) {
        REQUIRE_THROWS( test_open_rar_archive( test::sevenzip_lib(), BIT7Z_STRING( "valid.zip" ) ) );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting invalid archives should throw",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "testing" };

    // The italy.svg file in the ko_test archives is different from the one used for filesystem tests
    static constexpr auto italy_ko_crc32 = 0x2ADFB3AF;

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "rar", BitFormat::Rar5 },
    //TestInputFormat{"tar", BitFormat::Tar},
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "xz", BitFormat::Xz },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "ko_test." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format );
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
TEMPLATE_TEST_CASE( "BitInputArchive: Reading archives using the wrong format should throw",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto correctFormat = GENERATE( as< TestInputFormat >(),
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
                                         TestInputFormat{ "zip", BitFormat::Zip } );

    const auto wrongFormat = GENERATE( as< TestInputFormat >(),
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
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << correctFormat.extension ) {
        const auto arcFileName = fs::path{ clouds.name }.concat( "." + correctFormat.extension );

        if ( correctFormat.extension != wrongFormat.extension ) {
            DYNAMIC_SECTION( "Wrong format: " << wrongFormat.extension ) {
                TestType inputArchive{};
                getInputArchive( arcFileName, inputArchive );
                REQUIRE_THROWS( BitArchiveReader( test::sevenzip_lib(), inputArchive, wrongFormat.format ) );
            }
        }
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting an archive with different file types inside",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "file_type" };

    // Note: for some reason, 7-Zip fails to test or extract Rar archives containing symbolic links.

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
    //TestInputFormat{ "rar", BitFormat::Rar5 },
                                      TestInputFormat{ "tar", BitFormat::Tar },
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "file_type." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );

        REQUIRE_ARCHIVE_TESTS( info );
#ifdef BIT7Z_BUILD_FOR_P7ZIP
        // p7zip doesn't correctly report symbolic links in Wim and Tar archives.
        if ( testFormat.format != BitFormat::Wim && testFormat.format != BitFormat::Tar ) {
#else
        // 7-Zip doesn't correctly report symbolic links in Wim archives.
        if ( testFormat.format != BitFormat::Wim ) {
#endif
            // TODO(fix): Wim format gives some issues when extracting symbolic links.
            REQUIRE_ARCHIVE_EXTRACTS( info, file_type_content().items );
        }
    }
}

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting an archive with Unicode items",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "rar", BitFormat::Rar5 },
                                      TestInputFormat{ "tar", BitFormat::Tar },
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "unicode." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info, unicode_content().items );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting an archive with a Unicode file name",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName{ BIT7Z_NATIVE_STRING( "αρχείο.7z" ) };

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader info( test::sevenzip_lib(), inputArchive, BitFormat::SevenZip );
    REQUIRE_ARCHIVE_TESTS( info );
    REQUIRE_ARCHIVE_EXTRACTS( info, unicode_content().items );
}

TEST_CASE( "BitInputArchive: Testing and extracting an archive with a Unicode file name (bzip2)",
           "[bitinputarchive]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName{ BIT7Z_NATIVE_STRING( "クラウド.jpg.bz2" ) };
    const BitArchiveReader info( test::sevenzip_lib(), path_to_tstring( arcFileName ), BitFormat::BZip2 );
    REQUIRE_ARCHIVE_TESTS( info );
    const ExpectedItems expectedItems{ ExpectedItem{ clouds, BIT7Z_NATIVE_STRING( "クラウド.jpg" ), false } };
    REQUIRE_ARCHIVE_EXTRACTS( info, expectedItems );
}

#endif

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Extracting an archive without retaining directories",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format );
        info.setRetainDirectories( false );
        REQUIRE_ARCHIVE_EXTRACTS( info, flat_items_content().items );
    }
}

template< typename TestType >
inline auto overwritten_file_path( const BitInFormat& format ) -> fs::path {
    if ( is_filesystem_archive< TestType >::value || format_has_path_metadata( format ) ) {
        return clouds.name;
    }
    return "[Content]";
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Extracting an archive using various OverwriteMode settings",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "bz2", BitFormat::BZip2 },
                                      TestInputFormat{ "gz", BitFormat::GZip },
                                      TestInputFormat{ "iso", BitFormat::Iso },
                                      TestInputFormat{ "lzh", BitFormat::Lzh },
                                      TestInputFormat{ "lzma", BitFormat::Lzma },
                                      TestInputFormat{ "tar", BitFormat::Tar },
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "xz", BitFormat::Xz },
                                      TestInputFormat{ "zip", BitFormat::Zip } );
#else
    const auto testFormat = GENERATE( as< TestInputFormat >(),
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
                                      TestInputFormat{ "zip", BitFormat::Zip } );
#endif

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const auto arcFileName = fs::path{ clouds.name }.concat( "." + testFormat.extension );

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );

        TempTestDirectory testOutDir{ "test_bitinputarchive" };
        INFO( "Output directory: " << testOutDir )

        const auto expectedFile = overwritten_file_path< TestType >( testFormat.format );
        REQUIRE_FALSE( fs::exists( expectedFile ) );
        {
            fs::ofstream dummyFile{ expectedFile };
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
            REQUIRE( crc32( load_file( expectedFile ) ) == clouds.crc32 );
        }

        SECTION( "OverwriteMode::Overwrite" ) {
            // After setting OverwriteMode::Overwrite, extracting should not throw.
            info.setOverwriteMode( OverwriteMode::Overwrite );

            REQUIRE_NOTHROW( info.extractTo( testOutDir ) );
            REQUIRE( fs::exists( expectedFile ) );
            REQUIRE( crc32( load_file( expectedFile ) ) == clouds.crc32 );
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
auto get_file_time( const fs::path& filePath, FILETIME& creation, FILETIME& access, FILETIME& modified ) -> bool {
    HANDLE hFile = ::CreateFileW( filePath.c_str(),
                                  GENERIC_READ | FILE_READ_ATTRIBUTES, // NOLINT(*-signed-bitwise)
                                  FILE_SHARE_READ,
                                  nullptr,
                                  OPEN_EXISTING,
                                  0,
                                  nullptr );
    if ( hFile == INVALID_HANDLE_VALUE ) { // NOLINT(cppcoreguidelines-pro-type-cstyle-cast,performance-no-int-to-ptr)
        return false;
    }
    auto result = GetFileTime( hFile, &creation, &access, &modified );
    CloseHandle( hFile );
    return result != FALSE;
}
#endif

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Extracting an archive to the filesystem should preserve time metadata",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto arcFileName = fs::path{ clouds.name }.concat( ".7z" );

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    BitArchiveReader info( test::sevenzip_lib(), inputArchive, BitFormat::SevenZip );

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
    namespace chrono = std::chrono;
    const auto expectedModifiedTime = chrono::time_point_cast< chrono::seconds >( item.lastWriteTime() );
#endif

    TempTestDirectory testOutDir{ "test_bitinputarchive" };
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
    const auto modifiedTime = chrono::time_point_cast< chrono::seconds >( fs::last_write_time( expectedFile ) );
    // Note: Using count() since Catch2 cannot print std::chrono::duration objects.
    REQUIRE( modifiedTime.time_since_epoch().count() == expectedModifiedTime.time_since_epoch().count() );
#endif

    REQUIRE( fs::remove( expectedFile ) );
}

#ifdef _WIN32
// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Extracting an archive not having time metadata should use current time",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    // Note: the clouds.jpg.zip file was created without storing access/creation time metadata.
    const auto arcFileName = fs::path{ clouds.name }.concat( ".zip" );

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    BitArchiveReader info( test::sevenzip_lib(), inputArchive, BitFormat::Zip );

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
TEMPLATE_TEST_CASE( "BitInputArchive: Extracting a file with a comment should preserve it",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "file_comment" };

    TestType inputArchive{};
    getInputArchive( "commented.7z", inputArchive );
    BitArchiveReader info( test::sevenzip_lib(), inputArchive, BitFormat::SevenZip );

    TempTestDirectory testOutDir{ "test_bitinputarchive" };
    INFO( "Output directory: " << testOutDir )

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

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Using extraction callbacks", "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );
#else
    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );
#endif

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format );

        uint64_t totalSize = 0;
        info.setTotalCallback( [ &totalSize ]( uint64_t total ) {
            totalSize = total;
        } );

        std::vector< uint64_t > progressValues;
        info.setProgressCallback( [ &progressValues ]( uint64_t progress ) -> bool {
            progressValues.push_back( progress );
            return true;
        } );

        double finalRatio = 0.0;
        info.setRatioCallback( [ &finalRatio ]( uint64_t processedInput, uint64_t processedOutput ) {
            if ( processedOutput == 0 ) {
                return;
            }
            finalRatio = static_cast< double >( processedInput ) / static_cast< double >( processedOutput );
        } );

        std::vector< tstring > visitedFiles;
        info.setFileCallback( [ &visitedFiles ]( const tstring& file ) {
            visitedFiles.push_back( file );
        } );

        const auto& expectedItems = multiple_items_content().items;

        SECTION( "When extracting to the filesystem" ) {
            TempTestDirectory testOutDir{ "test_bitinputarchive" };
            INFO( "Output directory: " << testOutDir )
            require_extracts_to_filesystem( info, expectedItems );
        }

        SECTION( "When extracting to a buffer map" ) {
            require_extracts_to_buffers_map( info, expectedItems );
        }

        std::vector< tstring > expectedPaths;
        expectedPaths.reserve( expectedItems.size() );
        for ( const auto& expectedItem : expectedItems ) {
            if ( expectedItem.fileInfo.type != fs::file_type::directory ) {
                expectedPaths.push_back( path_to_tstring( expectedItem.inArchivePath ) );
            }
        }

        // Checking that the total callback was called at least once (it should be called only once by 7-Zip).
        REQUIRE( totalSize == multiple_items_content().size );

        // Checking that the progress callback was called at least once.
        REQUIRE( !progressValues.empty() );

        using namespace Catch::Matchers;

        // For some reason, the Tar format makes the progress decrease in some cases,
        // and it is not always less than the total size ¯\_(ツ)_/¯.
        if ( testArchive.format != BitFormat::Tar ) {
            // Checking that the values reported by the progress callback are increasing,
            // and less than or equal to the total size.
            uint64_t lastProgress = 0;
            for ( uint64_t progress : progressValues ) {
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
TEMPLATE_TEST_CASE( "BitInputArchive: Finding files in an archive", "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format );

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
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitInputArchive: Extract to raw data callback",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "lzh", BitFormat::Lzh },
                                       TestInputFormat{ "lzma", BitFormat::Lzma },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "xz", BitFormat::Xz },
                                       TestInputFormat{ "zip", BitFormat::Zip } );
#else
    const auto testArchive = GENERATE( as< TestInputFormat >(),
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
                                       TestInputFormat{ "zip", BitFormat::Zip } );
#endif

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = fs::path{ clouds.name }.concat( "." + testArchive.extension );

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const Bit7zLibrary lib{ test::sevenzip_lib_path() };
        BitArchiveReader info( lib, inputArchive, testArchive.format );

        std::size_t totalSize = 0;
        std::uint32_t crcValue = 0;

        info.extractTo([&totalSize, &crcValue]( const byte_t* data, std::size_t length ) {
            totalSize += length;
            crcValue = crc32( data, length, crcValue );
            return true;
        });
        REQUIRE( totalSize == clouds.size );
        REQUIRE( crcValue == clouds.crc32 );
    }
}
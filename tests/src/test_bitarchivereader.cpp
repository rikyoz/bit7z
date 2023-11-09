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

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "utils/archive.hpp"
#include "utils/crc.hpp"
#include "utils/filesystem.hpp"
#include "utils/format.hpp"
#include "utils/shared_lib.hpp"
#include "utils/source_location.hpp"

#include <bit7z/bit7zlibrary.hpp>
#include <bit7z/bitarchiveiteminfo.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitexception.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bitpropvariant.hpp>
#include <bit7z/bittypes.hpp>
#include <internal/fs.hpp>
#include <internal/stringutil.hpp>
#include <internal/windows.hpp>

// For checking posix file attributes.
#include <sys/stat.h>

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <type_traits>

// MSVC doesn't define these macros!
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG( m ) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR( m ) (((m) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISLNK) && defined(S_IFMT)
#ifndef S_IFLNK
constexpr auto S_IFLNK = 0120000;
#endif
#define S_ISLNK( m ) (((m) & S_IFMT) == S_IFLNK)
#endif

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

// Compile-time checks for BitArchiveItemInfo
static_assert( std::is_copy_constructible< BitArchiveItemInfo >::value,
               "BitArchiveItemInfo is not copy-constructible." );
static_assert( std::is_copy_assignable< BitArchiveItemInfo >::value,
               "BitArchiveItemInfo is not copy-assignable." );
static_assert( std::is_move_constructible< BitArchiveItemInfo >::value,
               "BitArchiveItemInfo is not move-constructible." );
static_assert( std::is_move_assignable< BitArchiveItemInfo >::value,
               "BitArchiveItemInfo is not move-assignable." );

void require_archive_extracts( const BitArchiveReader& info, const source_location& location ) {
    INFO( "Failed while extracting the archive ");
    INFO( "  from " << location.file_name() << ":" << location.line() );
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto& detectedFormat = (info).detectedFormat();
    if ( detectedFormat == BitFormat::Rar || detectedFormat == BitFormat::Rar5 ) {
        return;
    }
#endif

    SECTION( "Extracting to a map of buffers" ) {
        std::map< tstring, buffer_t > bufferMap;
        REQUIRE_NOTHROW( info.extractTo( bufferMap ) );
        REQUIRE( bufferMap.size() == info.filesCount() );
        if ( !format_has_crc( info.detectedFormat() ) || info.detectedFormat() == BitFormat::Rar5 ) {
            return;
        }
        for( const auto& entry : bufferMap ) {
            auto item = info.find( entry.first );
            REQUIRE( item != info.cend() );
            const auto item_crc = item->crc();
            if ( item_crc > 0 ) {
                REQUIRE( crc32( entry.second ) == item_crc );
            }
        }
    }

    SECTION( "Extracting each item to a buffer" ) {
        buffer_t outputBuffer;
        for ( const auto& item : info ) {
            if ( item.isDir() ) {
                REQUIRE_THROWS( info.extractTo( outputBuffer, item.index() ) );
                REQUIRE( outputBuffer.empty() );
            } else {
                REQUIRE_NOTHROW( info.extractTo( outputBuffer, item.index() ) );
                const auto item_crc = item.crc();
                if ( item_crc > 0 ) {
                    REQUIRE( crc32( outputBuffer ) == item_crc );
                } else if ( info.detectedFormat() != BitFormat::Chm && info.detectedFormat() != BitFormat::Elf &&
                            info.detectedFormat() != BitFormat::Macho && info.detectedFormat() != BitFormat::Ntfs &&
                            info.detectedFormat() != BitFormat::Swf ) {
                    // TODO: Check why these formats can't be extracted to a buffer
                    REQUIRE_FALSE( outputBuffer.empty() );
                }
            }
            outputBuffer.clear();
        }

        {
            buffer_t dummyBuffer;
            REQUIRE_THROWS( info.extractTo( dummyBuffer, info.itemsCount() ) );
            REQUIRE( dummyBuffer.empty() );

            REQUIRE_THROWS( info.extractTo( dummyBuffer, info.itemsCount() + 1 ) );
            REQUIRE( dummyBuffer.empty() );
        }
    }


    SECTION( "Extracting each item to a fixed size buffer" ) {
        // Note: this value must be different from any file size we can encounter inside the tested archives.
        constexpr size_t dummyBufferSize = 42;
        buffer_t dummyBuffer2( dummyBufferSize, static_cast< byte_t >( '\0' ) );
        buffer_t outputBuffer;
        for ( const auto& item : info ) {
            const auto itemIndex = item.index();
            REQUIRE_THROWS( info.extractTo( nullptr, 0, itemIndex ) );
            REQUIRE_THROWS( info.extractTo( nullptr, dummyBufferSize, itemIndex ) );
            REQUIRE_THROWS( info.extractTo( nullptr, std::numeric_limits< std::size_t >::max(), itemIndex ) );

            REQUIRE_THROWS( info.extractTo( &dummyBuffer2[ 0 ], 0, itemIndex ) );
            REQUIRE_THROWS( info.extractTo( &dummyBuffer2[ 0 ], dummyBufferSize, itemIndex ) );
            REQUIRE_THROWS( info.extractTo( &dummyBuffer2[ 0 ], std::numeric_limits< std::size_t >::max(), itemIndex ) );

            if ( !item.isDir() ) {
                const auto itemSize = item.size();
                REQUIRE_THROWS( info.extractTo( nullptr, itemSize, itemIndex ) );

                if ( itemSize > 0 ) {
                    outputBuffer.resize( itemSize, static_cast< byte_t >( '\0' ) );
                    REQUIRE_NOTHROW( info.extractTo( &outputBuffer[ 0 ], itemSize, itemIndex ) );

                    const auto item_crc = item.crc();
                    if ( item_crc > 0 ) {
                        REQUIRE( crc32( outputBuffer ) == item_crc );
                    }
                } else {
                    REQUIRE_THROWS( info.extractTo( &dummyBuffer2[ 0 ], itemSize, itemIndex ) );
                }
                outputBuffer.clear();
            }
        }

        REQUIRE_THROWS( info.extractTo( nullptr, 0, info.itemsCount() ) );
        REQUIRE_THROWS( info.extractTo( nullptr, dummyBufferSize, info.itemsCount() ) );
        REQUIRE_THROWS( info.extractTo( nullptr, std::numeric_limits< std::size_t >::max(), info.itemsCount() ) );

        REQUIRE_THROWS( info.extractTo( &dummyBuffer2[ 0 ], 0, info.itemsCount() ) );
        REQUIRE_THROWS( info.extractTo( &dummyBuffer2[ 0 ], dummyBufferSize, info.itemsCount() ) );
        REQUIRE_THROWS( info.extractTo( &dummyBuffer2[ 0 ],
                                        std::numeric_limits< std::size_t >::max(),
                                        info.itemsCount() ) );
    }

    SECTION( "Extracting each item to std::ostream" ) {
        for ( const auto& item : info ) {
            std::ostringstream outputStream;
            if ( item.isDir() ) {
                REQUIRE_THROWS( info.extractTo( outputStream, item.index() ) );
                REQUIRE( outputStream.str().empty() );
            } else {
                REQUIRE_NOTHROW( info.extractTo( outputStream, item.index() ) );
                const auto item_crc = item.crc();
                if ( item_crc > 0 ) {
                    REQUIRE( crc32( outputStream.str() ) == item_crc );
                } else {
                    REQUIRE_FALSE( outputStream.str().empty() );
                }
            }
        }

        {
            std::ostringstream outputStream;
            REQUIRE_THROWS( info.extractTo( outputStream, info.itemsCount() ) );
            REQUIRE( outputStream.str().empty() );

            REQUIRE_THROWS( info.extractTo( outputStream, info.itemsCount() + 1 ) );
            REQUIRE( outputStream.str().empty() );
        }
    }
}

#define REQUIRE_ARCHIVE_EXTRACTS( info ) \
    require_archive_extracts( info, BIT7Z_CURRENT_LOCATION )

void require_archive_tests( const BitArchiveReader& info, const source_location& location ) {
    INFO( "Failed while testing the archive ");
    INFO( "  from " << location.file_name() << ":" << location.line() );
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

void require_archive_item( const BitInFormat& format,
                           const BitArchiveItem& item,
                           const ArchivedItem& expectedItem,
                           const source_location& location ) {
    INFO( "Failed while checking archive item " << Catch::StringMaker< tstring >::convert( item.name() ) );
    INFO( "  from " << location.file_name() << ":" << location.line() );
    REQUIRE( item.isDir() == expectedItem.fileInfo.isDir );

    if ( !item.isDir() ) {
        REQUIRE( item.isEncrypted() == expectedItem.isEncrypted );
    }

    if ( format_has_path_metadata( format ) ) {
        REQUIRE( item.extension() == expectedItem.fileInfo.ext );
        REQUIRE( item.name() == expectedItem.fileInfo.name );
        REQUIRE( item.path() == expectedItem.inArchivePath );
    }

    if ( format_has_size_metadata( format ) ) {
        /* Note: some archive formats (e.g. BZip2) do not provide the size metadata! */
        REQUIRE( item.size() == expectedItem.fileInfo.size );
    }

    if ( ( format_has_crc( format ) && !item.itemProperty( BitProperty::CRC ).isEmpty() ) &&
         ( ( format != BitFormat::Rar5 ) || !item.isEncrypted() ) ) {
        /* For some reason, encrypted Rar5 archives messes up the values of CRCs*/
        REQUIRE( item.crc() == expectedItem.fileInfo.crc32 );
    }
}

#define REQUIRE_ARCHIVE_ITEM( format, item, expectedItem ) \
    require_archive_item( format, item, expectedItem, BIT7Z_CURRENT_LOCATION )

inline void require_archive_content( const BitArchiveReader& info,
                                     const TestInputArchive& input,
                                     const source_location& location ) {
    INFO( "Failed while checking content of " << Catch::StringMaker< tstring >::convert( info.archivePath() ) );
    INFO( "  from " << location.file_name() << ":" << location.line() );
    REQUIRE_FALSE( info.archiveProperties().empty() );

    const auto& archive_content = input.content();
    REQUIRE( info.itemsCount() == archive_content.items.size() );
    REQUIRE( info.filesCount() == archive_content.fileCount );
    REQUIRE( info.foldersCount() == ( archive_content.items.size() - archive_content.fileCount ) );

    const auto& format = info.format();
    if ( format_has_size_metadata( format ) ) {
        REQUIRE( info.size() == archive_content.size );
        REQUIRE( info.packSize() == input.packedSize() );
    }

    REQUIRE_FALSE( info.isMultiVolume() );
    REQUIRE( info.volumesCount() == 1 );

    std::vector< BitArchiveItemInfo > items;
    REQUIRE_NOTHROW( items = info.items() );
    REQUIRE( items.size() == info.itemsCount() );

    const bool archive_stores_paths = format_has_path_metadata( format );
    const bool from_filesystem = !info.archivePath().empty();
    size_t found_items = 0;
    for ( const auto& archivedItem : archive_content.items ) {
        for ( const auto& item : items ) {
            if ( archive_stores_paths || (from_filesystem) ) {
                if ( item.name() != archivedItem.fileInfo.name ) {
                    continue;
                }
                REQUIRE( info.find( item.path() ) != info.cend() );
                REQUIRE( info.contains( item.path() ) );
            }
            REQUIRE( info.isItemEncrypted( item.index() ) == archivedItem.isEncrypted );
            REQUIRE( info.isItemFolder( item.index() ) == archivedItem.fileInfo.isDir );
            require_archive_item( format, item, archivedItem, location );
            found_items++;
            break;
        }
    }
    REQUIRE( items.size() == found_items );
}

#define REQUIRE_ARCHIVE_CONTENT( info, input ) \
    require_archive_content( info, input, BIT7Z_CURRENT_LOCATION )

struct SingleFileArchive : public TestInputArchive {
    SingleFileArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, single_file_content() } {}
};

using stream_t = fs::ifstream;

// Note: we cannot use value semantic and return the archive due to old GCC versions not supporting movable fstreams.
void getInputArchive( const fs::path& path, tstring& archive ) {
    archive = path_to_tstring( path );
}

void getInputArchive( const fs::path& path, buffer_t& archive ) {
    archive = load_file( path );
}

void getInputArchive( const fs::path& path, stream_t& archive ) {
    archive.open( path, std::ios::binary );
}

template< typename T >
using is_filesystem_archive = std::is_same< bit7z::tstring, typename std::decay< T >::type >;

TEMPLATE_TEST_CASE( "BitArchiveReader: Reading archives containing only a single file",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto testArchive = GENERATE( as< SingleFileArchive >(),
                                       SingleFileArchive{ "7z", BitFormat::SevenZip, 478025 },
                                       SingleFileArchive{ "bz2", BitFormat::BZip2, 0 },
                                       SingleFileArchive{ "gz", BitFormat::GZip, 476404 },
                                       SingleFileArchive{ "iso", BitFormat::Iso, clouds.size },
                                       SingleFileArchive{ "lzh", BitFormat::Lzh, 476668 },
                                       SingleFileArchive{ "lzma", BitFormat::Lzma, 0 },
                                       SingleFileArchive{ "rar4.rar", BitFormat::Rar, 477457 },
                                       SingleFileArchive{ "rar5.rar", BitFormat::Rar5, 477870 },
                                       SingleFileArchive{ "tar", BitFormat::Tar, 479232 },
                                       SingleFileArchive{ "wim", BitFormat::Wim, clouds.size },
                                       SingleFileArchive{ "xz", BitFormat::Xz, 478080 },
                                       SingleFileArchive{ "zip", BitFormat::Zip, 476375 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const auto arcFileName = fs::path{ clouds.name }.concat( "." + testArchive.extension() );

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format() );
        if( is_filesystem_archive< TestType >::value ) {
            REQUIRE( info.archivePath() == arcFileName );
        } else {
            REQUIRE( info.archivePath().empty() ); // No archive path for buffer/streamed archives
        }
        REQUIRE_FALSE( info.hasEncryptedItems() );
        REQUIRE_FALSE( info.isEncrypted() );
        REQUIRE_ARCHIVE_CONTENT( info, testArchive );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

struct MultipleFilesArchive : public TestInputArchive {
    MultipleFilesArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, multiple_files_content() } {}
};

TEMPLATE_TEST_CASE( "BitArchiveReader: Reading archives containing multiple files",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_files" };

    const auto testArchive = GENERATE( as< MultipleFilesArchive >(),
                                        MultipleFilesArchive{ "7z", BitFormat::SevenZip, 22074 },
                                        MultipleFilesArchive{ "iso", BitFormat::Iso, italy.size + lorem_ipsum.size },
                                        MultipleFilesArchive{ "rar", BitFormat::Rar5, 23040 },
                                        MultipleFilesArchive{ "tar", BitFormat::Tar, 38912 },
                                        MultipleFilesArchive{ "wim", BitFormat::Wim, italy.size + lorem_ipsum.size },
                                        MultipleFilesArchive{ "zip", BitFormat::Zip, 23222 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path arcFileName = "multiple_files." + testArchive.extension();

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format() );
        if( is_filesystem_archive< TestType >::value ) {
            REQUIRE( info.archivePath() == arcFileName );
        } else {
            REQUIRE( info.archivePath().empty() ); // No archive path for buffer/streamed archives
        }
        REQUIRE_FALSE( info.hasEncryptedItems() );
        REQUIRE_FALSE( info.isEncrypted() );
        REQUIRE_ARCHIVE_CONTENT( info, testArchive );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

struct MultipleItemsArchive : public TestInputArchive {
    MultipleItemsArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, multiple_items_content() } {}
};

TEMPLATE_TEST_CASE( "BitArchiveReader: Reading archives containing multiple items (files and folders)",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< MultipleItemsArchive >(),
                                        MultipleItemsArchive{ "7z", BitFormat::SevenZip, 563797 },
                                        MultipleItemsArchive{ "iso", BitFormat::Iso, 615351 },
                                        MultipleItemsArchive{ "rar4.rar", BitFormat::Rar, 565329 },
                                        MultipleItemsArchive{ "rar5.rar", BitFormat::Rar5, 565756 },
                                        MultipleItemsArchive{ "tar", BitFormat::Tar, 617472 },
                                        MultipleItemsArchive{ "wim", BitFormat::Wim, 615351 },
                                        MultipleItemsArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension();

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format() );
        if( is_filesystem_archive< TestType >::value ) {
            REQUIRE( info.archivePath() == arcFileName );
        } else {
            REQUIRE( info.archivePath().empty() ); // No archive path for buffer/streamed archives
        }
        REQUIRE_FALSE( info.hasEncryptedItems() );
        REQUIRE_FALSE( info.isEncrypted() );
        REQUIRE_ARCHIVE_CONTENT( info, testArchive );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

struct EncryptedArchive : public TestInputArchive {
    EncryptedArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, encrypted_content() } {}
};

TEMPLATE_TEST_CASE( "BitArchiveReader: Reading archives containing encrypted items",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "encrypted" };

    const auto* const password = BIT7Z_STRING( "helloworld" );

    const auto testArchive = GENERATE( as< EncryptedArchive >(),
                                        EncryptedArchive{ "7z", BitFormat::SevenZip, 563568 },
                                        EncryptedArchive{ "rar4.rar", BitFormat::Rar, 565424 },
                                        EncryptedArchive{ "rar5.rar", BitFormat::Rar5, 565824 },
                                        EncryptedArchive{ "aes256.zip", BitFormat::Zip, 564016 },
                                        EncryptedArchive{ "zipcrypto.zip", BitFormat::Zip, 563888 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path arcFileName = "encrypted." + testArchive.extension();

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );

        SECTION( "BitArchiveReader::isHeaderEncrypted must return false" ){
            REQUIRE_FALSE( BitArchiveReader::isHeaderEncrypted( test::sevenzip_lib(),
                                                                inputArchive,
                                                                testArchive.format() ) );
        }

        SECTION( "BitArchiveReader::isEncrypted must return true" ){
            REQUIRE( BitArchiveReader::isEncrypted( test::sevenzip_lib(), inputArchive, testArchive.format() ) );
        }

        SECTION( "Opening the archive with no password should allow reading the archive, but tests() should throw" ) {
            const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format() );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE( info.isEncrypted() );
            REQUIRE_ARCHIVE_CONTENT( info, testArchive );
            REQUIRE_THROWS( info.test() );

            std::map< tstring, buffer_t > dummyMap;
            REQUIRE_THROWS( info.extractTo( dummyMap ) );
            for ( const auto& entry : dummyMap ) {
                // TODO: Check if extractTo should not write or clear the map when the extraction fails
                REQUIRE( entry.second.empty() );
            }
        }

        SECTION( "Opening the archive with the correct password should pass all the checks" ) {
            const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format(), password );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE( info.isEncrypted() );
            REQUIRE_ARCHIVE_CONTENT( info, testArchive );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info );
        }
    }
}

/* Pull request #36 */
TEMPLATE_TEST_CASE( "BitArchiveReader: Reading header-encrypted archives",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "header_encrypted" };

    const auto* const password = BIT7Z_STRING( "helloworld" );

    const auto testArchive = GENERATE( as< EncryptedArchive >(),
                                        EncryptedArchive{ "7z", BitFormat::SevenZip, 563776 },
                                        EncryptedArchive{ "rar4.rar", BitFormat::Rar, 565408 },
                                        EncryptedArchive{ "rar5.rar", BitFormat::Rar5, 565824 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path arcFileName = "header_encrypted." + testArchive.extension();

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );

        SECTION( "BitArchiveReader::isHeaderEncrypted must return true" ){
            REQUIRE( BitArchiveReader::isHeaderEncrypted( test::sevenzip_lib(), inputArchive, testArchive.format() ) );
        }

        SECTION( "BitArchiveReader::isEncrypted must return true" ){
            REQUIRE( BitArchiveReader::isEncrypted( test::sevenzip_lib(), inputArchive, testArchive.format() ) );
        }


        SECTION( "Opening the archive with no password should throw an exception" ) {
            REQUIRE_THROWS( BitArchiveReader( test::sevenzip_lib(), inputArchive, testArchive.format() ) );
        }


        SECTION( "Opening the archive with a wrong password should throw an exception" ) {
            REQUIRE_THROWS( BitArchiveReader( test::sevenzip_lib(), inputArchive, testArchive.format(),
                                              BIT7Z_STRING( "wrong_password" ) ) );
        }

        SECTION( "Opening the archive with the correct password should pass the tests" ) {
            const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format(), password );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE( info.isEncrypted() );
            REQUIRE_ARCHIVE_CONTENT( info, testArchive );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info );
        }
    }
}

TEST_CASE( "BitArchiveReader: Reading metadata of multi-volume archives", "[bitarchivereader]" ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "split" };

    SECTION( "Split archive (non-RAR)" ) {
        const auto testArchive = GENERATE( as< SingleFileArchive >(),
                                            SingleFileArchive{ "7z", BitFormat::SevenZip, 478025 },
                                            SingleFileArchive{ "bz2", BitFormat::BZip2, 0 },
                                            SingleFileArchive{ "gz", BitFormat::GZip, 476404 },
                                            SingleFileArchive{ "tar", BitFormat::Tar, 479232 },
                                            SingleFileArchive{ "wim", BitFormat::Wim, 478883 },
                                            SingleFileArchive{ "xz", BitFormat::Xz, 478080 },
                                            SingleFileArchive{ "zip", BitFormat::Zip, 476398 } );

        DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
            const fs::path arcFileName = "clouds.jpg." + testArchive.extension() + ".001";

            INFO( "Archive file: " << arcFileName );

            SECTION( "Opening as a split archive" ) {
                const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Split );
                REQUIRE( info.isMultiVolume() );
                REQUIRE( info.volumesCount() == 3 );
                REQUIRE( info.itemsCount() == 1 );
                REQUIRE( info.items()[ 0 ].name() == arcFileName.stem().string< tchar >() );
                REQUIRE_ARCHIVE_TESTS( info );
                REQUIRE_ARCHIVE_EXTRACTS( info );
            }

            SECTION( "Opening as a whole archive" ) {
                const BitArchiveReader info( test::sevenzip_lib(),
                                             arcFileName.string< tchar >(),
                                             testArchive.format() );
                // REQUIRE( info.isMultiVolume() );
                // REQUIRE( info.volumesCount() == 3 );
                REQUIRE_ARCHIVE_ITEM( testArchive.format(), info.items()[ 0 ], testArchive.content().items[ 0 ] );
                REQUIRE_ARCHIVE_TESTS( info );
                REQUIRE_ARCHIVE_EXTRACTS( info );
            }
        }
    }

    SECTION( "Multi-volume RAR5" ) {
        const fs::path arcFileName = "clouds.jpg.part1.rar";
        const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Rar5 );
        REQUIRE( info.isMultiVolume() );
        REQUIRE( info.volumesCount() == 3 );
        REQUIRE( info.itemsCount() == 1 );

        const ArchivedItem expectedItem{ clouds, clouds.name };
        REQUIRE_ARCHIVE_ITEM( BitFormat::Rar5, info.items()[ 0 ], expectedItem );

#ifndef BIT7Z_BUILD_FOR_P7ZIP
        REQUIRE_ARCHIVE_TESTS( info );
#endif
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }

    SECTION( "Multi-volume RAR4" ) {
        const fs::path arcFileName = "clouds.jpg.rar";
        const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Rar );
        REQUIRE( info.isMultiVolume() );
        REQUIRE( info.volumesCount() == 3 );
        REQUIRE( info.itemsCount() == 1 );

        const ArchivedItem expectedItem{ clouds, clouds.name };
        REQUIRE_ARCHIVE_ITEM( BitFormat::Rar, info.items()[ 0 ], expectedItem );

#ifndef BIT7Z_BUILD_FOR_P7ZIP
        REQUIRE_ARCHIVE_TESTS( info );
#endif
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

struct EmptyArchive : public TestInputArchive {
    EmptyArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, empty_content() } {}
};

TEMPLATE_TEST_CASE( "BitArchiveReader: Reading an empty archive",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "empty" };

    const auto testArchive = GENERATE( as< EmptyArchive >(),
                                       EmptyArchive{ "7z", BitFormat::SevenZip, 0 },
    // EmptyArchive{ "tar", BitFormat::Tar, 0 }, // TODO: Check why it fails opening
                                       EmptyArchive{ "wim", BitFormat::Wim, 0 },
                                       EmptyArchive{ "zip", BitFormat::Zip, 0 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path arcFileName = "empty." + testArchive.extension();

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format() );
        if( is_filesystem_archive< TestType >::value ) {
            REQUIRE( info.archivePath() == arcFileName );
        } else {
            REQUIRE( info.archivePath().empty() ); // No archive path for buffer/streamed archives
        }
        REQUIRE_FALSE( info.isEncrypted() );
        REQUIRE_ARCHIVE_CONTENT( info, testArchive );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

TEST_CASE( "BitArchiveReader: Solid archive detection", "[bitarchivereader]" ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "solid" };

    SECTION( "Solid 7z" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "solid.7z" ), BitFormat::SevenZip );
        REQUIRE( info.isSolid() );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }

    SECTION( "Solid RAR" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "solid.rar" ), BitFormat::Rar5 );
        REQUIRE( info.isSolid() );

#ifndef BIT7Z_BUILD_FOR_P7ZIP
        REQUIRE_ARCHIVE_TESTS( info );
#endif
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }

    SECTION( "Non solid 7z" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "non_solid.7z" ), BitFormat::SevenZip );
        REQUIRE( !info.isSolid() );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }

    SECTION( "Non-solid RAR" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "non_solid.rar" ), BitFormat::Rar5 );
        REQUIRE( !info.isSolid() );

#ifndef BIT7Z_BUILD_FOR_P7ZIP
        REQUIRE_ARCHIVE_TESTS( info );
#endif
        REQUIRE_ARCHIVE_EXTRACTS( info );
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
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    SECTION( "Valid RAR archives" ) {
        REQUIRE( test_open_rar_archive( test::sevenzip_lib(), BIT7Z_STRING( "valid.rar4.rar" ) ) == BitFormat::Rar );
        REQUIRE( test_open_rar_archive( test::sevenzip_lib(), BIT7Z_STRING( "valid.rar5.rar" ) ) == BitFormat::Rar5 );
    }

    SECTION( "Non-RAR archive" ) {
        REQUIRE_THROWS( test_open_rar_archive( test::sevenzip_lib(), BIT7Z_STRING( "valid.zip" ) ) );
    }
}

TEMPLATE_TEST_CASE( "BitArchiveReader: Checking consistency between items() and iterators",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< MultipleItemsArchive >(),
                                        MultipleItemsArchive{ "7z", BitFormat::SevenZip, 563797 },
                                        MultipleItemsArchive{ "iso", BitFormat::Iso, 615351 },
                                        MultipleItemsArchive{ "rar4.rar", BitFormat::Rar, 565329 },
                                        MultipleItemsArchive{ "rar5.rar", BitFormat::Rar5, 565756 },
                                        MultipleItemsArchive{ "tar", BitFormat::Tar, 617472 },
                                        MultipleItemsArchive{ "wim", BitFormat::Wim, 615351 },
                                        MultipleItemsArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension();

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format() );

        const auto archiveItems = info.items();

        REQUIRE( info.begin() == info.cbegin() );
        REQUIRE( info.end() == info.cend() );

        for ( const auto& iteratedItem : info ) {
            const auto& archivedItem = archiveItems[ iteratedItem.index() ];
            REQUIRE( archivedItem.index() == iteratedItem.index() );
            REQUIRE( archivedItem.name() == iteratedItem.name() );
            REQUIRE( archivedItem.path() == iteratedItem.path() );
            REQUIRE( archivedItem.isDir() == iteratedItem.isDir() );
            REQUIRE( archivedItem.crc() == iteratedItem.crc() );
            REQUIRE( archivedItem.extension() == iteratedItem.extension() );
            REQUIRE( archivedItem.isEncrypted() == iteratedItem.isEncrypted() );
            REQUIRE( archivedItem.size() == iteratedItem.size() );
            REQUIRE( archivedItem.packSize() == iteratedItem.packSize() );
            REQUIRE( archivedItem.attributes() == iteratedItem.attributes() );
        }
    }
}

TEMPLATE_TEST_CASE( "BitArchiveReader: Reading invalid archives",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "testing" };

    const auto testArchive = GENERATE( as< SingleFileArchive >(),
                                        SingleFileArchive{ "7z", BitFormat::SevenZip, 478025 },
                                        SingleFileArchive{ "bz2", BitFormat::BZip2, 0 },
                                        SingleFileArchive{ "gz", BitFormat::GZip, 476404 },
                                        SingleFileArchive{ "rar", BitFormat::Rar5, 477870 },
                                        //SingleFileArchive{"tar", BitFormat::Tar, 479232},
                                        SingleFileArchive{ "wim", BitFormat::Wim, clouds.size },
                                        SingleFileArchive{ "xz", BitFormat::Xz, 478080 },
                                        SingleFileArchive{ "zip", BitFormat::Zip, 476375 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path arcFileName = "ko_test." + testArchive.extension();

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format() );
        REQUIRE_THROWS( info.test() );

        std::map< tstring, buffer_t > dummyMap;
        REQUIRE_THROWS( info.extractTo( dummyMap ) );
        // TODO: Check if extractTo should not write or clear the map when the extraction fails
    }
}

TEMPLATE_TEST_CASE( "BitArchiveReader: Reading archives using the wrong format should throw",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

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

#ifndef FILE_ATTRIBUTE_WINDOWS_MASK
constexpr auto FILE_ATTRIBUTE_WINDOWS_MASK = 0x07FFF;
#endif

void require_item_type( const BitArchiveReader& info,
                        const tstring& itemName,
                        fs::file_type fileType,
                        std::uint32_t winAttributes,
                        source_location location ) {
    INFO( "Failed checking required item " << Catch::StringMaker< tstring >::convert( itemName ) );
    INFO( "  from " << location.file_name() << ":" << location.line() );
    auto iterator = info.find( itemName );
    REQUIRE( iterator != info.cend() );
    REQUIRE( iterator->name() == itemName );

    bool expectedDir = ( fileType == fs::file_type::directory );
    bool expectedSymlink = ( fileType == fs::file_type::symlink );
    REQUIRE( iterator->isDir() == expectedDir );
    REQUIRE( iterator->isSymLink() == expectedSymlink );

    auto item_attributes = iterator->attributes();
    if ( ( item_attributes & FILE_ATTRIBUTE_WINDOWS_MASK ) != 0 ) {
        REQUIRE( ( item_attributes & FILE_ATTRIBUTE_DIRECTORY ) == ( expectedDir ? FILE_ATTRIBUTE_DIRECTORY : 0 ) );
        REQUIRE( ( item_attributes & FILE_ATTRIBUTE_HIDDEN ) == ( winAttributes & FILE_ATTRIBUTE_HIDDEN ) );
        REQUIRE( ( item_attributes & FILE_ATTRIBUTE_READONLY ) == ( winAttributes & FILE_ATTRIBUTE_READONLY ) );
    }
    if ( ( item_attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) == FILE_ATTRIBUTE_UNIX_EXTENSION ) {
        auto posix_attributes = item_attributes >> 16U;
        REQUIRE( S_ISDIR( posix_attributes ) == expectedDir );
        REQUIRE( S_ISREG( posix_attributes ) == ( !expectedDir && !expectedSymlink ) );
        REQUIRE( S_ISLNK( posix_attributes ) == expectedSymlink );
    }
}

#define REQUIRE_ITEM_TYPE( info, item_name, file_type ) \
    require_item_type( (info), BIT7Z_STRING( item_name ), (file_type), 0, BIT7Z_CURRENT_LOCATION )

#define REQUIRE_ITEM_TYPE_WITH_ATTRIBUTES( info, item_name, file_type, win_attributes ) \
    require_item_type( (info), BIT7Z_STRING( item_name ), (file_type), (win_attributes), BIT7Z_CURRENT_LOCATION )

TEMPLATE_TEST_CASE( "BitArchiveReader: Correctly reading file type inside archives",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "file_type" };

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "rar", BitFormat::Rar5 },
                                      TestInputFormat{ "tar", BitFormat::Tar },
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "file_type." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
        REQUIRE_ITEM_TYPE( info, "dir", fs::file_type::directory );
        REQUIRE_ITEM_TYPE( info, "regular", fs::file_type::regular );
        REQUIRE_ITEM_TYPE( info, "symlink", fs::file_type::symlink );
        REQUIRE_ITEM_TYPE_WITH_ATTRIBUTES( info, "hidden", fs::file_type::regular, FILE_ATTRIBUTE_HIDDEN );
        REQUIRE_ITEM_TYPE_WITH_ATTRIBUTES( info, "read_only", fs::file_type::regular, FILE_ATTRIBUTE_READONLY );
    }
}

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE

TEMPLATE_TEST_CASE( "BitArchiveReader: Correctly reading archive items with Unicode names",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

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
        REQUIRE_ITEM_TYPE( info, "¡Porque sí!.doc", fs::file_type::regular );
        REQUIRE_ITEM_TYPE( info, "σύννεφα.jpg", fs::file_type::regular );
        REQUIRE_ITEM_TYPE( info, "юнікод.svg", fs::file_type::regular );
        REQUIRE_ITEM_TYPE( info, "ユニコード.pdf", fs::file_type::regular );
    }
}

TEMPLATE_TEST_CASE( "BitArchiveReader: Reading an archive with a Unicode file name",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName{ BIT7Z_NATIVE_STRING( "αρχείο.7z" ) };

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader info( test::sevenzip_lib(), inputArchive, BitFormat::SevenZip );
    REQUIRE_ITEM_TYPE( info, "¡Porque sí!.doc", fs::file_type::regular );
    REQUIRE_ITEM_TYPE( info, "σύννεφα.jpg", fs::file_type::regular );
    REQUIRE_ITEM_TYPE( info, "юнікод.svg", fs::file_type::regular );
    REQUIRE_ITEM_TYPE( info, "ユニコード.pdf", fs::file_type::regular );
}

TEST_CASE( "BitArchiveReader: Reading an archive with a Unicode file name (bzip2)", "[bitarchivereader]" ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName{ BIT7Z_NATIVE_STRING( "クラウド.jpg.bz2" ) };
    const BitArchiveReader info( test::sevenzip_lib(), path_to_tstring( arcFileName ), BitFormat::BZip2 );
    REQUIRE_ITEM_TYPE( info, "クラウド.jpg", fs::file_type::regular );
}
#endif

#ifdef BIT7Z_AUTO_FORMAT

TEST_CASE( "BitArchiveReader: Format detection of archives", "[bitarchivereader]" ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    auto test = GENERATE( TestInputFormat{ "7z", BitFormat::SevenZip },
                          TestInputFormat{ "ar", BitFormat::Deb },
                          TestInputFormat{ "arj", BitFormat::Arj },
                          TestInputFormat{ "bz2", BitFormat::BZip2 },
                          TestInputFormat{ "bzip2", BitFormat::BZip2 },
                          TestInputFormat{ "cab", BitFormat::Cab },
                          TestInputFormat{ "chi", BitFormat::Chm },
                          TestInputFormat{ "chm", BitFormat::Chm },
                          TestInputFormat{ "cpio", BitFormat::Cpio },
                          TestInputFormat{ "cramfs", BitFormat::CramFS },
                          TestInputFormat{ "deb", BitFormat::Deb },
                          TestInputFormat{ "deflate.swfc", BitFormat::Swfc },
                          TestInputFormat{ "doc", BitFormat::Compound },
                          TestInputFormat{ "docx", BitFormat::Zip },
                          TestInputFormat{ "dylib", BitFormat::Macho },
                          TestInputFormat{ "elf", BitFormat::Elf },
                          TestInputFormat{ "exe", BitFormat::Pe }, // We don't consider SFX executables!
                          TestInputFormat{ "ext2", BitFormat::Ext },
                          TestInputFormat{ "ext3", BitFormat::Ext },
                          TestInputFormat{ "ext4", BitFormat::Ext },
                          TestInputFormat{ "ext4.img", BitFormat::Ext },
                          TestInputFormat{ "fat", BitFormat::Fat },
                          TestInputFormat{ "fat12.img", BitFormat::Fat },
                          TestInputFormat{ "fat16.img", BitFormat::Fat },
                          TestInputFormat{ "flv", BitFormat::Flv },
                          TestInputFormat{ "gpt", BitFormat::GPT },
                          TestInputFormat{ "gz", BitFormat::GZip },
                          TestInputFormat{ "gzip", BitFormat::GZip },
                          TestInputFormat{ "hfs", BitFormat::Hfs },
                          TestInputFormat{ "iso", BitFormat::Iso },
                          TestInputFormat{ "iso.img", BitFormat::Iso },
                          TestInputFormat{ "lha", BitFormat::Lzh },
                          TestInputFormat{ "lzh", BitFormat::Lzh },
                          TestInputFormat{ "lzma", BitFormat::Lzma },
                          TestInputFormat{ "lzma86", BitFormat::Lzma86 },
                          TestInputFormat{ "lzma.swfc", BitFormat::Swfc },
                          TestInputFormat{ "macho", BitFormat::Macho },
                          TestInputFormat{ "mslz", BitFormat::Mslz },
                          TestInputFormat{ "nsis", BitFormat::Nsis },
                          TestInputFormat{ "ntfs", BitFormat::Ntfs },
                          TestInputFormat{ "ntfs.img", BitFormat::Ntfs },
                          TestInputFormat{ "odp", BitFormat::Zip },
                          TestInputFormat{ "ods", BitFormat::Zip },
                          TestInputFormat{ "odt", BitFormat::Zip },
                          TestInputFormat{ "ova", BitFormat::Tar },
                          TestInputFormat{ "part1.rar", BitFormat::Rar5 },
                          TestInputFormat{ "part2.rar", BitFormat::Rar5 },
                          TestInputFormat{ "part3.rar", BitFormat::Rar5 },
                          TestInputFormat{ "pkg", BitFormat::Xar },
                          TestInputFormat{ "pmd", BitFormat::Ppmd },
                          TestInputFormat{ "ppmd", BitFormat::Ppmd },
                          TestInputFormat{ "ppt", BitFormat::Compound },
                          TestInputFormat{ "pptx", BitFormat::Zip },
                          TestInputFormat{ "qcow", BitFormat::QCow },
                          TestInputFormat{ "qcow2", BitFormat::QCow },
                          TestInputFormat{ "rar4.rar", BitFormat::Rar },
                          TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                          TestInputFormat{ "rpm", BitFormat::Rpm },
                          TestInputFormat{ "sqsh", BitFormat::SquashFS },
                          TestInputFormat{ "squashfs", BitFormat::SquashFS },
                          TestInputFormat{ "swf", BitFormat::Swf },
                          TestInputFormat{ "swm", BitFormat::Wim },
                          TestInputFormat{ "tar", BitFormat::Tar },
                          TestInputFormat{ "taz", BitFormat::Z },
                          TestInputFormat{ "tbz", BitFormat::BZip2 },
                          TestInputFormat{ "tbz2", BitFormat::BZip2 },
                          TestInputFormat{ "tgz", BitFormat::GZip },
                          TestInputFormat{ "txz", BitFormat::Xz },
                          TestInputFormat{ "vdi", BitFormat::VDI },
                          TestInputFormat{ "vhd", BitFormat::Vhd },
                          TestInputFormat{ "vmdk", BitFormat::VMDK },
                          TestInputFormat{ "wim", BitFormat::Wim },
                          TestInputFormat{ "xar", BitFormat::Xar },
                          TestInputFormat{ "xls", BitFormat::Compound },
                          TestInputFormat{ "xlsx", BitFormat::Zip },
                          TestInputFormat{ "xz", BitFormat::Xz },
                          TestInputFormat{ "z", BitFormat::Z },
                          TestInputFormat{ "zip", BitFormat::Zip },
                          TestInputFormat{ "zipx", BitFormat::Zip } );

    DYNAMIC_SECTION( "Test extension: " << test.extension ) {
        SECTION( "Filesystem archive (extension + signature)" ) {
            const fs::path file = "valid." + test.extension;
            const BitArchiveReader reader{ test::sevenzip_lib(), file.string< bit7z::tchar >() };
            REQUIRE( reader.detectedFormat() == test.format );

#ifdef BIT7Z_BUILD_FOR_P7ZIP
            if ( test.format != BitFormat::Rar && test.format != BitFormat::Rar5 &&
                 test.format != BitFormat::Mslz && test.extension != "part2.rar" && test.extension != "part3.rar" ) {
#else
            if ( test.format != BitFormat::Mslz && test.extension != "part2.rar" && test.extension != "part3.rar" ) {
#endif
                REQUIRE_NOTHROW( reader.test() );
                // NOTE: Testing extraction of all these formats takes too much time and CTest timeouts
            }
        }

        SECTION( "Archive stream (signature) from a file" ) {
            REQUIRE_OPEN_IFSTREAM( fileStream, "valid." + test.extension );

            const BitArchiveReader reader{ test::sevenzip_lib(), fileStream };
            REQUIRE( reader.detectedFormat() == test.format );

            // TODO: Verify why testing of Mslz and multi-volume RAR archives fails
#ifdef BIT7Z_BUILD_FOR_P7ZIP
            if ( test.format != BitFormat::Rar && test.format != BitFormat::Rar5 &&
                 test.format != BitFormat::Mslz && test.extension.find( "part" ) != 0 ) {
#else
            if ( test.format != BitFormat::Mslz && test.extension.find( "part" ) != 0 ) {
#endif
                REQUIRE_NOTHROW( reader.test() );
                // NOTE: Testing extraction of all these formats takes too much time and CTest timeouts
            }
        }
    }
}

#endif
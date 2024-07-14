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

#include "utils/archive.hpp"
#include "utils/crc.hpp"
#include "utils/filesystem.hpp"
#include "utils/format.hpp"
#include "utils/shared_lib.hpp"
#include "utils/sourcelocation.hpp"

#include <bit7z/bitarchiveiteminfo.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bittypes.hpp>
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

struct SingleFileArchive : TestInputArchive {
    SingleFileArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, single_file_content() } {}
};

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Reading archives containing only a single file",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

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
    }
}

struct MultipleFilesArchive : TestInputArchive {
    MultipleFilesArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, multiple_files_content() } {}
};

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Reading archives containing multiple files",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_files" };

    const auto testArchive = GENERATE( as< MultipleFilesArchive >(),
                                        MultipleFilesArchive{ "7z", BitFormat::SevenZip, 22074 },
                                        MultipleFilesArchive{ "iso", BitFormat::Iso, italy.size + loremIpsum.size },
                                        MultipleFilesArchive{ "rar", BitFormat::Rar5, 23040 },
                                        MultipleFilesArchive{ "tar", BitFormat::Tar, 38912 },
                                        MultipleFilesArchive{ "wim", BitFormat::Wim, italy.size + loremIpsum.size },
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
    }
}

struct MultipleItemsArchive : TestInputArchive {
    MultipleItemsArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, multiple_items_content() } {}
};

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Reading archives containing multiple items (files and folders)",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

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
    }
}

struct EncryptedArchive : TestInputArchive {
    EncryptedArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, encrypted_content() } {}
};

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Reading archives containing encrypted items",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "encrypted" };

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
        }
    }
}

/* Pull request #36 */
// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Reading header-encrypted archives",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "header_encrypted" };

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

        SECTION( "Opening the archive with the correct password should allow reading the archive metadata" ) {
            const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format(), password );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE( info.isEncrypted() );
            REQUIRE_ARCHIVE_CONTENT( info, testArchive );
        }
    }
}

TEST_CASE( "BitArchiveReader: Reading metadata of multi-volume archives", "[bitarchivereader]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "split" };

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

            INFO( "Archive file: " << arcFileName )

            SECTION( "Opening as a split archive" ) {
                const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Split );
                REQUIRE( info.isMultiVolume() );
                REQUIRE( info.volumesCount() == 3 );
                REQUIRE( info.itemsCount() == 1 );
                REQUIRE( info.items()[ 0 ].name() == arcFileName.stem().string< tchar >() );
            }

            SECTION( "Opening as a whole archive" ) {
                const BitArchiveReader info( test::sevenzip_lib(),
                                             arcFileName.string< tchar >(),
                                             testArchive.format() );
                // REQUIRE( info.isMultiVolume() );
                // REQUIRE( info.volumesCount() == 3 );
                REQUIRE_ARCHIVE_ITEM( testArchive.format(), info.items()[ 0 ], testArchive.content().items[ 0 ] );
            }
        }
    }

    SECTION( "Multi-volume RAR5" ) {
        const fs::path arcFileName = "clouds.jpg.part1.rar";
        const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Rar5 );
        REQUIRE( info.isMultiVolume() );
        REQUIRE( info.volumesCount() == 3 );
        REQUIRE( info.itemsCount() == 1 );

        const ExpectedItem expectedItem{ clouds, clouds.name };
        REQUIRE_ARCHIVE_ITEM( BitFormat::Rar5, info.items()[ 0 ], expectedItem );
    }

    SECTION( "Multi-volume RAR4" ) {
        const fs::path arcFileName = "clouds.jpg.rar";
        const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Rar );
        REQUIRE( info.isMultiVolume() );
        REQUIRE( info.volumesCount() == 3 );
        REQUIRE( info.itemsCount() == 1 );

        const ExpectedItem expectedItem{ clouds, clouds.name };
        REQUIRE_ARCHIVE_ITEM( BitFormat::Rar, info.items()[ 0 ], expectedItem );
    }
}

struct EmptyArchive : TestInputArchive {
    EmptyArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, empty_content() } {}
};

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Reading an empty archive",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "empty" };

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
    }
}

TEST_CASE( "BitArchiveReader: Solid archive detection", "[bitarchivereader]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "solid" };

    SECTION( "Solid 7z" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "solid.7z" ), BitFormat::SevenZip );
        REQUIRE( info.isSolid() );
    }

    SECTION( "Solid RAR" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "solid.rar" ), BitFormat::Rar5 );
        REQUIRE( info.isSolid() );
    }

    SECTION( "Non solid 7z" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "non_solid.7z" ), BitFormat::SevenZip );
        REQUIRE( !info.isSolid() );
    }

    SECTION( "Non-solid RAR" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "non_solid.rar" ), BitFormat::Rar5 );
        REQUIRE( !info.isSolid() );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Checking consistency between items() and iterators",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

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

        const auto begin = info.begin()++;
        REQUIRE( begin == info.begin() );

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
            REQUIRE( info.itemAt( archivedItem.index() ) == iteratedItem );
        }

        REQUIRE_THROWS( info.itemAt( info.itemsCount() ) );
        REQUIRE_THROWS( info.itemAt( info.itemsCount() + 1 ) );
    }
}

#ifndef FILE_ATTRIBUTE_WINDOWS_MASK
constexpr auto FILE_ATTRIBUTE_WINDOWS_MASK = 0x07FFF;
#endif

void require_item_type( const BitArchiveReader& info,
                        const tstring& itemName,
                        fs::file_type fileType,
                        std::uint32_t winAttributes,
                        SourceLocation location ) {
    INFO( "Failed checking required item " << Catch::StringMaker< tstring >::convert( itemName ) )
    INFO( "  from " << location.file_name() << ":" << location.line() )
    auto iterator = info.find( itemName );
    REQUIRE( iterator != info.cend() );
    REQUIRE( iterator->name() == itemName );
    REQUIRE( iterator->path() == itemName );

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

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Correctly reading file type inside archives",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "file_type" };

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

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Correctly reading archive items with Unicode names",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
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
        REQUIRE_ITEM_TYPE( info, "¡Porque sí!.doc", fs::file_type::regular );
        REQUIRE_ITEM_TYPE( info, "σύννεφα.jpg", fs::file_type::regular );
        REQUIRE_ITEM_TYPE( info, "юнікод.svg", fs::file_type::regular );
        REQUIRE_ITEM_TYPE( info, "ユニコード.pdf", fs::file_type::regular );
    }
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveReader: Reading an archive with a Unicode file name",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

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
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName{ BIT7Z_NATIVE_STRING( "クラウド.jpg.bz2" ) };
    const BitArchiveReader info( test::sevenzip_lib(), to_tstring( arcFileName ), BitFormat::BZip2 );
    REQUIRE_ITEM_TYPE( info, "クラウド.jpg", fs::file_type::regular );
}
#endif

#ifdef BIT7Z_AUTO_FORMAT

TEST_CASE( "BitArchiveReader: Format detection of archives", "[bitarchivereader]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

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

TEMPLATE_TEST_CASE( "BitArchiveReader: Renaming the files being extracted using a RenameCallback",
                    "[bitarchivereader]", tstring, buffer_t, stream_t ) {
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
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format );

        TempTestDirectory testOutDir{ "test_bitarchivereader" };
        info.extractTo( testOutDir, []( std::uint32_t, const tstring& originalName ) -> tstring {
            if ( originalName == italy.name ) {
                return BIT7Z_STRING( "flag.svg" ); // Rename.
            }
            if ( originalName == loremIpsum.name ) {
                return BIT7Z_STRING( "document.pdf" ); // Rename.
            }
            if ( originalName == noext.name ) {
                return originalName; // Keep the original name.
            }
            return {}; // Skipping extraction of all other files.
        } );
        REQUIRE_FALSE( fs::exists( italy.name ) );
        REQUIRE_FALSE( fs::exists( loremIpsum.name ) );
        REQUIRE_FALSE( fs::exists( dotFolder.name ) );
        REQUIRE_FALSE( fs::exists( emptyFolder.name ) );
        REQUIRE_FALSE( fs::exists( folder.name ) );

        auto iterator = fs::directory_iterator{ testOutDir.path() };
        int fileCount = std::count_if(
            fs::begin( iterator ),
            fs::end( iterator ),
            []( const fs::directory_entry& entry ) { return entry.is_regular_file(); }
        );
        REQUIRE( fileCount == 3 );
        REQUIRE( fs::exists( "flag.svg" ) );
        REQUIRE( fs::exists( "document.pdf" ) );
        REQUIRE( fs::exists( "noext" ) );

        auto buffer = load_file( "flag.svg" );
        REQUIRE( crc32(  buffer ) == italy.crc32 );

        buffer = load_file( "document.pdf" );
        REQUIRE( crc32( buffer ) == loremIpsum.crc32 );

        buffer = load_file( "noext" );
        REQUIRE( crc32( buffer ) == noext.crc32 );

        REQUIRE( fs::remove( "flag.svg" ) );
        REQUIRE( fs::remove( "document.pdf" ) );
        REQUIRE( fs::remove( "noext" ) );
    }
}
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

#include "archive.hpp"
#include "filesystem.hpp"
#include "format.hpp"
#include "shared_lib.hpp"

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitexception.hpp>
#include <bit7z/bitformat.hpp>
#include <internal/windows.hpp>

// Needed by MSVC for defining the S_XXXX macros
#ifndef _CRT_INTERNAL_NONSTDC_NAMES
#define _CRT_INTERNAL_NONSTDC_NAMES 1
#endif

// For checking posix file attributes
#include <sys/stat.h>

// On MSVC, these macros are not defined!
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISLNK) && defined(S_IFMT)
#ifndef S_IFLNK
constexpr auto S_IFLNK = 0120000;
#endif
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

// Compile-time checks for BitArchiveItemInfo
static_assert( std::is_copy_constructible< BitArchiveItemInfo >::value,
               "BitArchiveItemInfo is not copy-constructible!" );
static_assert( std::is_copy_assignable< BitArchiveItemInfo >::value,
               "BitArchiveItemInfo is not copy-assignable!" );
static_assert( std::is_move_constructible< BitArchiveItemInfo >::value,
               "BitArchiveItemInfo is not move-constructible!" );
static_assert( std::is_move_assignable< BitArchiveItemInfo >::value,
               "BitArchiveItemInfo is not move-assignable!" );

#define REQUIRE_ARCHIVE_ITEM( format, item, expected_item )                                              \
    do {                                                                                                 \
        INFO( "Failed while checking file " << Catch::StringMaker< tstring >::convert( (item).name() ) ) \
        REQUIRE( (item).isDir() == (expected_item).fileInfo.is_dir );                                    \
                                                                                                         \
        if ( !(item).isDir() ) {                                                                         \
            REQUIRE( (item).isEncrypted() == (expected_item).isEncrypted );                              \
        }                                                                                                \
                                                                                                         \
        if ( format_has_path_metadata( format ) ) {                                                      \
            REQUIRE( (item).extension() == (expected_item).fileInfo.ext );                               \
            REQUIRE( (item).name() == (expected_item).fileInfo.name );                                   \
            REQUIRE( (item).path() == (expected_item).inArchivePath );                                   \
        }                                                                                                \
                                                                                                         \
        if ( format_has_size_metadata( format ) ) {                                                      \
            /* Note: some archive formats (e.g. BZip2) do not provide the size metadata! */              \
            REQUIRE( (item).size() == (expected_item).fileInfo.size );                                   \
        }                                                                                                \
                                                                                                         \
        if ( ( format_has_crc( format ) && !(item).itemProperty( BitProperty::CRC ).isEmpty() ) &&       \
             ( format != BitFormat::Rar5 || !(item).isEncrypted() ) ) {                                  \
            /* For some reasons, encrypted Rar5 archives messes up the values of CRCs*/                  \
            REQUIRE( (item).crc() == (expected_item).fileInfo.crc32 );                                   \
        }                                                                                                \
    } while( false )

#define REQUIRE_ARCHIVE_CONTENT( info, input )                                                        \
    do {                                                                                              \
        REQUIRE_FALSE( (info).archiveProperties().empty() );                                          \
                                                                                                      \
        const auto& archive_content = (input).content();                                              \
        REQUIRE( (info).itemsCount() == archive_content.items.size() );                               \
        REQUIRE( (info).filesCount() == archive_content.fileCount );                                  \
        REQUIRE( (info).foldersCount() == archive_content.items.size() - archive_content.fileCount ); \
                                                                                                      \
        const auto& format = (info).format();                                                         \
        if ( format_has_size_metadata( format ) ) {                                                   \
            REQUIRE( (info).size() == archive_content.size );                                         \
            REQUIRE( (info).packSize() == (input).packedSize() );                                     \
        }                                                                                             \
                                                                                                      \
        REQUIRE_FALSE( (info).isMultiVolume() );                                                      \
        REQUIRE( (info).volumesCount() == 1 );                                                        \
                                                                                                      \
        std::vector< BitArchiveItemInfo > items;                                                      \
        REQUIRE_NOTHROW( items = (info).items() );                                                    \
        REQUIRE( items.size() == (info).itemsCount() );                                               \
                                                                                                      \
        const bool archive_stores_paths = format_has_path_metadata( format );                         \
        size_t found_items = 0;                                                                       \
        for ( const auto& archived_item : archive_content.items ) {                                   \
            for ( const auto& item : items ) {                                                        \
                if ( archive_stores_paths ) {                                                         \
                    if ( item.name() != archived_item.fileInfo.name ) {                               \
                        continue;                                                                     \
                    }                                                                                 \
                    REQUIRE( (info).find( item.path() ) != (info).cend() );                           \
                    REQUIRE( (info).contains( item.path() ) );                                        \
                }                                                                                     \
                REQUIRE( (info).isItemEncrypted( item.index() ) == archived_item.isEncrypted );       \
                REQUIRE( (info).isItemFolder( item.index() ) == archived_item.fileInfo.is_dir );      \
                REQUIRE_ARCHIVE_ITEM( format, item, archived_item );                                  \
                found_items++;                                                                        \
                break;                                                                                \
            }                                                                                         \
        }                                                                                             \
        REQUIRE( items.size() == found_items );                                                       \
    } while ( false )

struct SingleFileArchive : public TestInputArchive {
    SingleFileArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, single_file_content() } {}
};

TEST_CASE( "BitArchiveReader: Reading archives containing only a single file", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "extraction" / "single_file";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto test_archive = GENERATE( as< SingleFileArchive >(),
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

    DYNAMIC_SECTION( "Archive format: " << test_archive.extension() ) {
        const auto arc_file_name = fs::path{ clouds.name }.concat( "." + test_archive.extension() );

        SECTION( "Filesystem archive" ) {
            const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_archive.format() );
            REQUIRE( info.archivePath() == arc_file_name );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );
            const BitArchiveReader info( lib, file_buffer, test_archive.format() );
            REQUIRE( info.archivePath().empty() ); // No archive path for buffered archives
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            const BitArchiveReader info( lib, file_stream, test_archive.format() );
            REQUIRE( info.archivePath().empty() ); // No archive path for streamed archives
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

struct MultipleFilesArchive : public TestInputArchive {
    MultipleFilesArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, multiple_files_content() } {}
};

TEST_CASE( "BitArchiveReader: Reading archives containing multiple files", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "extraction" / "multiple_files";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto test_archive = GENERATE( as< MultipleFilesArchive >(),
                                        MultipleFilesArchive{ "7z", BitFormat::SevenZip, 22074 },
                                        MultipleFilesArchive{ "iso", BitFormat::Iso, italy.size + lorem_ipsum.size },
                                        MultipleFilesArchive{ "rar", BitFormat::Rar5, 23040 },
                                        MultipleFilesArchive{ "tar", BitFormat::Tar, 38912 },
                                        MultipleFilesArchive{ "wim", BitFormat::Wim, italy.size + lorem_ipsum.size },
                                        MultipleFilesArchive{ "zip", BitFormat::Zip, 23222 } );

    DYNAMIC_SECTION( "Archive format: " << test_archive.extension() ) {
        const fs::path arc_file_name = "multiple_files." + test_archive.extension();

        SECTION( "Filesystem archive" ) {
            const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_archive.format() );
            REQUIRE( info.archivePath() == arc_file_name );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );
            const BitArchiveReader info( lib, file_buffer, test_archive.format() );
            REQUIRE( info.archivePath().empty() ); // No archive path for buffered archives
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            const BitArchiveReader info( lib, file_stream, test_archive.format() );
            REQUIRE( info.archivePath().empty() ); // No archive path for streamed archives
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

struct MultipleItemsArchive : public TestInputArchive {
    MultipleItemsArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, multiple_items_content() } {}
};

TEST_CASE( "BitArchiveReader: Reading archives containing multiple items (files and folders)", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto test_archive = GENERATE( as< MultipleItemsArchive >(),
                                        MultipleItemsArchive{ "7z", BitFormat::SevenZip, 563797 },
                                        MultipleItemsArchive{ "iso", BitFormat::Iso, 615351 },
                                        MultipleItemsArchive{ "rar4.rar", BitFormat::Rar, 565329 },
                                        MultipleItemsArchive{ "rar5.rar", BitFormat::Rar5, 565756 },
                                        MultipleItemsArchive{ "tar", BitFormat::Tar, 617472 },
                                        MultipleItemsArchive{ "wim", BitFormat::Wim, 615351 },
                                        MultipleItemsArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << test_archive.extension() ) {
        const fs::path arc_file_name = "multiple_items." + test_archive.extension();

        SECTION( "Filesystem archive" ) {
            const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_archive.format() );
            REQUIRE( info.archivePath() == arc_file_name );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );
            const BitArchiveReader info( lib, file_buffer, test_archive.format() );
            REQUIRE( info.archivePath().empty() ); // No archive path for buffered archives
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            const BitArchiveReader info( lib, file_stream, test_archive.format() );
            REQUIRE( info.archivePath().empty() ); // No archive path for streamed archives
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

struct EncryptedArchive : public TestInputArchive {
    EncryptedArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, encrypted_content() } {}
};

TEST_CASE( "BitArchiveReader: Reading archives containing encrypted items", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "extraction" / "encrypted";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto* const password = BIT7Z_STRING( "helloworld" );

    const auto test_archive = GENERATE( as< EncryptedArchive >(),
                                        EncryptedArchive{ "7z", BitFormat::SevenZip, 563568 },
                                        EncryptedArchive{ "rar4.rar", BitFormat::Rar, 565424 },
                                        EncryptedArchive{ "rar5.rar", BitFormat::Rar5, 565824 },
                                        EncryptedArchive{ "aes256.zip", BitFormat::Zip, 564016 },
                                        EncryptedArchive{ "zipcrypto.zip", BitFormat::Zip, 563888 } );

    DYNAMIC_SECTION( "Archive format: " << test_archive.extension() ) {
        const fs::path arc_file_name = "encrypted." + test_archive.extension();

        SECTION( "Filesystem archive" ) {
            BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_archive.format() );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.setPassword( password ) );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );
            BitArchiveReader info( lib, file_buffer, test_archive.format() );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.setPassword( password ) );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            REQUIRE( file_stream.is_open() );

            BitArchiveReader info( lib, file_stream, test_archive.format() );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.setPassword( password ) );
            REQUIRE_NOTHROW( info.test() );
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

/* Pull request #36 */
TEST_CASE( "BitArchiveReader: Reading header-encrypted archives", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "extraction" / "header_encrypted";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto* const password = BIT7Z_STRING( "helloworld" );

    const auto test_archive = GENERATE( as< EncryptedArchive >(),
                                        EncryptedArchive{ "7z", BitFormat::SevenZip, 563776 },
                                        EncryptedArchive{ "rar4.rar", BitFormat::Rar, 565408 },
                                        EncryptedArchive{ "rar5.rar", BitFormat::Rar5, 565824 } );

    DYNAMIC_SECTION( "Archive format: " << test_archive.extension() ) {
        const fs::path arc_file_name = "header_encrypted." + test_archive.extension();

        SECTION( "Filesystem archive" ) {
            // no password specified!
            REQUIRE_THROWS( BitArchiveReader( lib, arc_file_name.string< tchar >(), test_archive.format() ) );

            // wrong password specified!
            REQUIRE_THROWS( BitArchiveReader( lib, arc_file_name.string< tchar >(), test_archive.format(),
                                              BIT7Z_STRING( "wrong_password" ) ) );

            const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_archive.format(), password );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );

            // no password specified!
            REQUIRE_THROWS( BitArchiveReader( lib, file_buffer, test_archive.format() ) );

            // wrong password specified!
            REQUIRE_THROWS( BitArchiveReader( lib, file_buffer, test_archive.format(),
                                              BIT7Z_STRING( "wrong_password" ) ) );

            const BitArchiveReader info( lib, file_buffer, test_archive.format(), password );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            REQUIRE( file_stream.is_open() );

            // no password specified!
            REQUIRE_THROWS( BitArchiveReader( lib, file_stream, test_archive.format() ) );

            // wrong password specified!
            REQUIRE_THROWS( BitArchiveReader( lib, file_stream, test_archive.format(),
                                              BIT7Z_STRING( "wrong_password" ) ) );

            file_stream.clear();
            file_stream.seekg( 0, std::ios::beg );
            REQUIRE( file_stream.is_open() );
            const BitArchiveReader info( lib, file_stream, test_archive.format(), password );
            REQUIRE( info.hasEncryptedItems() );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }
    }
}

TEST_CASE( "BitArchiveReader: Reading metadata of multi-volume archives", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "extraction" / "split";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    SECTION( "Split archive (non-RAR)" ) {
        const auto test_archive = GENERATE( as< SingleFileArchive >(),
                                            SingleFileArchive{ "7z", BitFormat::SevenZip, 478025 },
                                            SingleFileArchive{ "bz2", BitFormat::BZip2, 0 },
                                            SingleFileArchive{ "gz", BitFormat::GZip, 476404 },
                                            SingleFileArchive{ "tar", BitFormat::Tar, 479232 },
                                            SingleFileArchive{ "wim", BitFormat::Wim, 478883 },
                                            SingleFileArchive{ "xz", BitFormat::Xz, 478080 },
                                            SingleFileArchive{ "zip", BitFormat::Zip, 476398 } );

        DYNAMIC_SECTION( "Archive format: " << test_archive.extension() ) {
            const fs::path arc_file_name = "clouds.jpg." + test_archive.extension() + ".001";

            INFO( "Archive file: " << arc_file_name )

            SECTION( "Opening as a split archive" ) {
                const BitArchiveReader info( lib, arc_file_name.string< tchar >(), BitFormat::Split );
                REQUIRE( info.isMultiVolume() );
                REQUIRE( info.volumesCount() == 3 );
                REQUIRE( info.itemsCount() == 1 );
                REQUIRE( info.items()[ 0 ].name() == arc_file_name.stem().string< tchar >() );
                REQUIRE_NOTHROW( info.test() );
            }

            SECTION( "Opening as a whole archive" ) {
                const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_archive.format() );
                // REQUIRE( info.isMultiVolume() );
                // REQUIRE( info.volumesCount() == 3 );
                REQUIRE_ARCHIVE_ITEM( test_archive.format(), info.items()[ 0 ], test_archive.content().items[ 0 ] );
                REQUIRE_NOTHROW( info.test() );
            }
        }
    }

    SECTION( "Multi-volume RAR5" ) {
        const fs::path arc_file_name = "clouds.jpg.part1.rar";
        const BitArchiveReader info( lib, arc_file_name.string< tchar >(), BitFormat::Rar5 );
        REQUIRE( info.isMultiVolume() );
        REQUIRE( info.volumesCount() == 3 );
        REQUIRE( info.itemsCount() == 1 );

        const ArchivedItem expected_item{ clouds, clouds.name };
        REQUIRE_ARCHIVE_ITEM( BitFormat::Rar5, info.items()[ 0 ], expected_item );

        REQUIRE_NOTHROW( info.test() );
    }

    SECTION( "Multi-volume RAR4" ) {
        const fs::path arc_file_name = "clouds.jpg.rar";
        const BitArchiveReader info( lib, arc_file_name.string< tchar >(), BitFormat::Rar );
        REQUIRE( info.isMultiVolume() );
        REQUIRE( info.volumesCount() == 3 );
        REQUIRE( info.itemsCount() == 1 );

        const ArchivedItem expected_item{ clouds, clouds.name };
        REQUIRE_ARCHIVE_ITEM( BitFormat::Rar, info.items()[ 0 ], expected_item );

        REQUIRE_NOTHROW( info.test() );
    }
}

struct EmptyArchive : public TestInputArchive {
    EmptyArchive( std::string extension, const BitInFormat& format, std::size_t packedSize )
        : TestInputArchive{ std::move( extension ), format, packedSize, empty_content() } {}
};

TEST_CASE( "BitArchiveReader: Reading an empty archive", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "extraction" / "empty";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto test_archive = GENERATE( as< EmptyArchive >(),
                                        EmptyArchive{ "7z", BitFormat::SevenZip, 0 },
    // EmptyArchive{ "tar", BitFormat::Tar, 0 }, // TODO: Check why it fails opening
                                        EmptyArchive{ "wim", BitFormat::Wim, 0 },
                                        EmptyArchive{ "zip", BitFormat::Zip, 0 } );

    DYNAMIC_SECTION( "Archive format: " << test_archive.extension() ) {
        const fs::path arc_file_name = "empty." + test_archive.extension();

        SECTION( "Filesystem archive" ) {
            const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_archive.format() );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );
            const BitArchiveReader info( lib, file_buffer, test_archive.format() );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            REQUIRE( file_stream.is_open() );

            const BitArchiveReader info( lib, file_stream, test_archive.format() );
            REQUIRE_ARCHIVE_CONTENT( info, test_archive );
            REQUIRE_NOTHROW( info.test() );
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

TEST_CASE( "BitArchiveReader: Solid archive detection", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "solid";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    SECTION( "Solid 7z" ) {
        const BitArchiveReader info( lib, BIT7Z_STRING( "solid.7z" ), BitFormat::SevenZip );
        REQUIRE( info.isSolid() );
        REQUIRE_NOTHROW( info.test() );
    }

    SECTION( "Solid RAR" ) {
        const BitArchiveReader info( lib, BIT7Z_STRING( "solid.rar" ), BitFormat::Rar5 );
        REQUIRE( info.isSolid() );
        REQUIRE_NOTHROW( info.test() );
    }

    SECTION( "Non solid 7z" ) {
        const BitArchiveReader info( lib, BIT7Z_STRING( "non_solid.7z" ), BitFormat::SevenZip );
        REQUIRE( !info.isSolid() );
        REQUIRE_NOTHROW( info.test() );
    }

    SECTION( "Non-solid RAR" ) {
        const BitArchiveReader info( lib, BIT7Z_STRING( "non_solid.rar" ), BitFormat::Rar5 );
        REQUIRE( !info.isSolid() );
        REQUIRE_NOTHROW( info.test() );
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

/**
 * Tests opening an archive file using the RAR format
 * (or throws a BitException if it is not a RAR archive at all).
 */
auto testOpenRarArchive( const Bit7zLibrary& lib, const tstring& in_file ) -> const BitInFormat& {
    try {
        const BitArchiveReader info( lib, in_file, BitFormat::Rar );
        //if BitArchiveReader constructor did not throw an exception, the archive is RAR (< 5.0)!
        return BitFormat::Rar;
    } catch ( const BitException& ) {
        /* the archive is not a RAR, and if it is not even a RAR5,
           the following line will throw an exception (which we do not catch)! */
        const BitArchiveReader info( lib, in_file, BitFormat::Rar5 );
        return BitFormat::Rar5;
    }
}

TEST_CASE( "BitArchiveReader: Opening RAR archives using the correct RAR format version", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "detection" / "valid";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    SECTION( "Valid RAR archives" ) {
        REQUIRE( testOpenRarArchive( lib, BIT7Z_STRING( "valid.rar4.rar" ) ) == BitFormat::Rar );
        REQUIRE( testOpenRarArchive( lib, BIT7Z_STRING( "valid.rar5.rar" ) ) == BitFormat::Rar5 );
    }

    SECTION( "Non-RAR archive" ) {
        REQUIRE_THROWS( testOpenRarArchive( lib, BIT7Z_STRING( "valid.zip" ) ) );
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

#define REQUIRE_ITEM_EQUAL( first, second ) \
    do {                                              \
        REQUIRE( (first).index() == (second).index() ); \
        REQUIRE( (first).name() == (second).name() ); \
        REQUIRE( (first).path() == (second).path() ); \
        REQUIRE( (first).isDir() == (second).isDir() ); \
        REQUIRE( (first).crc() == (second).crc() ); \
        REQUIRE( (first).extension() == (second).extension() ); \
        REQUIRE( (first).isEncrypted() == (second).isEncrypted() ); \
        REQUIRE( (first).size() == (second).size() ); \
        REQUIRE( (first).packSize() == (second).packSize() ); \
        REQUIRE( (first).attributes() == (second).attributes() ); \
    } while ( false )

TEST_CASE( "BitArchiveReader: Checking consistency between items() and iterators", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto test_archive = GENERATE( as< MultipleItemsArchive >(),
                                        MultipleItemsArchive{ "7z", BitFormat::SevenZip, 563797 },
                                        MultipleItemsArchive{ "iso", BitFormat::Iso, 615351 },
                                        MultipleItemsArchive{ "rar4.rar", BitFormat::Rar, 565329 },
                                        MultipleItemsArchive{ "rar5.rar", BitFormat::Rar5, 565756 },
                                        MultipleItemsArchive{ "tar", BitFormat::Tar, 617472 },
                                        MultipleItemsArchive{ "wim", BitFormat::Wim, 615351 },
                                        MultipleItemsArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << test_archive.extension() ) {
        const fs::path arc_file_name = "multiple_items." + test_archive.extension();

        SECTION( "Filesystem archive" ) {
            const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_archive.format() );

            const auto archive_items = info.items();

            REQUIRE( info.begin() == info.cbegin() );
            REQUIRE( info.end() == info.cend() );

            for ( const auto& iterated_item : info ) {
                const auto& archived_item = archive_items[ iterated_item.index() ];
                REQUIRE_ITEM_EQUAL( archived_item, iterated_item );
            }
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );
            const BitArchiveReader info( lib, file_buffer, test_archive.format() );

            const auto archive_items = info.items();

            REQUIRE( info.begin() == info.cbegin() );
            REQUIRE( info.end() == info.cend() );

            for ( const auto& iterated_item : info ) {
                const auto& archived_item = archive_items[ iterated_item.index() ];
                REQUIRE_ITEM_EQUAL( archived_item, iterated_item );
            }
        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            const BitArchiveReader info( lib, file_stream, test_archive.format() );

            const auto archive_items = info.items();

            REQUIRE( info.begin() == info.cbegin() );
            REQUIRE( info.end() == info.cend() );

            for ( const auto& iterated_item : info ) {
                const auto& archived_item = archive_items[ iterated_item.index() ];
                REQUIRE_ITEM_EQUAL( archived_item, iterated_item );
            }
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

TEST_CASE( "BitArchiveReader: Reading invalid archives", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "testing";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto test_archive = GENERATE( as< SingleFileArchive >(),
                                        SingleFileArchive{ "7z", BitFormat::SevenZip, 478025 },
                                        SingleFileArchive{ "bz2", BitFormat::BZip2, 0 },
                                        SingleFileArchive{ "gz", BitFormat::GZip, 476404 },
                                        SingleFileArchive{ "rar", BitFormat::Rar5, 477870 },
                                        //SingleFileArchive{ "tar", BitFormat::Tar, 479232 },
                                        SingleFileArchive{ "wim", BitFormat::Wim, clouds.size },
                                        SingleFileArchive{ "xz", BitFormat::Xz, 478080 },
                                        SingleFileArchive{ "zip", BitFormat::Zip, 476375 } );

    DYNAMIC_SECTION( "Archive format: " << test_archive.extension() ) {
        const fs::path arc_file_name = "ko_test." + test_archive.extension();

        SECTION( "Filesystem archive" ) {
            const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_archive.format() );
            REQUIRE_THROWS( info.test() );
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );
            const BitArchiveReader info( lib, file_buffer, test_archive.format() );
            REQUIRE_THROWS( info.test() );
        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            const BitArchiveReader info( lib, file_stream, test_archive.format() );
            REQUIRE_THROWS( info.test() );
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

TEST_CASE( "BitArchiveReader: Reading archives using the wrong format should throw", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "extraction" / "single_file";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto correct_format = GENERATE( as< TestInputFormat >(),
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

    const auto wrong_format = GENERATE( as< TestInputFormat >(),
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

    DYNAMIC_SECTION( "Archive format: " << correct_format.extension ) {
        const auto arc_file_name = fs::path{ clouds.name }.concat( "." + correct_format.extension );

        if ( correct_format.extension != wrong_format.extension ) {
            DYNAMIC_SECTION( "Wrong format: " << wrong_format.extension ) {
                SECTION( "Filesystem archive" ) {
                    REQUIRE_THROWS( BitArchiveReader( lib, arc_file_name.string< tchar >(), wrong_format.format ) );
                }

                SECTION( "Buffer archive" ) {
                    const auto file_buffer = load_file( arc_file_name );
                    REQUIRE_THROWS( BitArchiveReader( lib, file_buffer, wrong_format.format ) );
                }

                SECTION( "Stream archive" ) {
                    fs::ifstream file_stream{ arc_file_name, std::ios::binary };
                    REQUIRE_THROWS( BitArchiveReader( lib, file_stream, wrong_format.format ) );
                }
            }
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

#ifndef FILE_ATTRIBUTE_WINDOWS_MASK
constexpr auto FILE_ATTRIBUTE_WINDOWS_MASK = 0x07FFF;
#endif

#define REQUIRE_ITEM_DIRECTORY( info, item_name )                                                     \
    do {                                                                                              \
        auto iterator = (info).find( BIT7Z_STRING( item_name ) );                                     \
        REQUIRE( iterator != (info).cend() );                                                         \
        REQUIRE( iterator->isDir() );                                                                 \
        auto item_attributes = iterator->attributes();                                                \
        if ( ( item_attributes & FILE_ATTRIBUTE_WINDOWS_MASK ) != 0 ) {                               \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY );    \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_HIDDEN ) == 0 );                              \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_READONLY ) == 0 );                            \
        }                                                                                             \
        if ( ( item_attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) == FILE_ATTRIBUTE_UNIX_EXTENSION ) { \
            auto posix_attributes = item_attributes >> 16U;                                           \
            REQUIRE( S_ISDIR( posix_attributes ) );                                                   \
            REQUIRE( !S_ISREG( posix_attributes ) );                                                  \
            REQUIRE( !S_ISLNK( posix_attributes ) );                                                  \
        }                                                                                             \
        REQUIRE( iterator->name() == BIT7Z_STRING( item_name ) );                                     \
    } while ( false )

#define REQUIRE_ITEM_REGULAR( info, item_name )                                                       \
    do {                                                                                              \
        auto iterator = (info).find( BIT7Z_STRING( item_name ) );                                     \
        REQUIRE( iterator != (info).cend() );                                                         \
        REQUIRE_FALSE( iterator->isDir() );                                                           \
        auto item_attributes = iterator->attributes();                                                \
        if ( ( item_attributes & FILE_ATTRIBUTE_WINDOWS_MASK ) != 0 ) {                               \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 );                           \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_HIDDEN ) == 0 );                              \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_READONLY ) == 0 );                            \
        }                                                                                             \
        if ( ( item_attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) == FILE_ATTRIBUTE_UNIX_EXTENSION ) { \
            auto posix_attributes = item_attributes >> 16U;                                           \
            REQUIRE( !S_ISDIR( posix_attributes ) );                                                  \
            REQUIRE( S_ISREG( posix_attributes ) );                                                   \
            REQUIRE( !S_ISLNK( posix_attributes ) );                                                  \
        }                                                                                             \
        REQUIRE( iterator->name() == BIT7Z_STRING( item_name ) );                                     \
    } while ( false )

#define REQUIRE_ITEM_SYMLINK( info, item_name )                                                       \
    do {                                                                                              \
        auto iterator = (info).find( BIT7Z_STRING( item_name ) );                                     \
        REQUIRE( iterator != (info).cend() );                                                         \
        REQUIRE_FALSE( iterator->isDir() );                                                           \
        auto item_attributes = iterator->attributes();                                                \
        if ( ( item_attributes & FILE_ATTRIBUTE_WINDOWS_MASK ) != 0 ) {                               \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 );                           \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_HIDDEN ) == 0 );                              \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_READONLY ) == 0 );                            \
        }                                                                                             \
        if ( ( item_attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) == FILE_ATTRIBUTE_UNIX_EXTENSION ) { \
            auto posix_attributes = item_attributes >> 16U;                                           \
            REQUIRE( !S_ISDIR( posix_attributes ) );                                                  \
            REQUIRE( !S_ISREG( posix_attributes ) );                                                  \
            REQUIRE( S_ISLNK( posix_attributes ) );                                                   \
        }                                                                                             \
        REQUIRE( iterator->name() == BIT7Z_STRING( item_name ) );                                     \
    } while ( false )

#define REQUIRE_ITEM_HIDDEN( info, item_name )                                                        \
    do {                                                                                              \
        auto iterator = (info).find( BIT7Z_STRING( item_name ) );                                     \
        REQUIRE( iterator != (info).cend() );                                                         \
        REQUIRE_FALSE( iterator->isDir() );                                                           \
        auto item_attributes = iterator->attributes();                                                \
        if ( ( item_attributes & FILE_ATTRIBUTE_WINDOWS_MASK ) != 0 ) {                               \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 );                           \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_HIDDEN ) == FILE_ATTRIBUTE_HIDDEN );          \
        }                                                                                             \
        if ( ( item_attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) == FILE_ATTRIBUTE_UNIX_EXTENSION ) { \
            auto posix_attributes = item_attributes >> 16U;                                           \
            REQUIRE( !S_ISDIR( posix_attributes ) );                                                  \
            REQUIRE( S_ISREG( posix_attributes ) );                                                   \
            REQUIRE( !S_ISLNK( posix_attributes ) );                                                  \
        }                                                                                             \
        REQUIRE( iterator->name() == BIT7Z_STRING( item_name ) );                                     \
    } while ( false )

#define REQUIRE_ITEM_READONLY( info, item_name )                                                      \
    do {                                                                                              \
        auto iterator = (info).find( BIT7Z_STRING( item_name ) );                                     \
        REQUIRE( iterator != (info).cend() );                                                         \
        REQUIRE_FALSE( iterator->isDir() );                                                           \
        auto item_attributes = iterator->attributes();                                                \
        if ( ( item_attributes & FILE_ATTRIBUTE_WINDOWS_MASK ) != 0 ) {                               \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 );                           \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_HIDDEN ) == 0 );                              \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_READONLY ) == FILE_ATTRIBUTE_READONLY );      \
        }                                                                                             \
        if ( ( item_attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) == FILE_ATTRIBUTE_UNIX_EXTENSION ) { \
            auto posix_attributes = item_attributes >> 16U;                                           \
            REQUIRE( !S_ISDIR( posix_attributes ) );                                                  \
            REQUIRE( S_ISREG( posix_attributes ) );                                                   \
            REQUIRE( !S_ISLNK( posix_attributes ) );                                                  \
        }                                                                                             \
        REQUIRE( iterator->name() == BIT7Z_STRING( item_name ) );                                     \
    } while ( false )

TEST_CASE( "BitArchiveReader: Correctly reading file type inside archives", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "metadata" / "file_type";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto test_format = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << test_format.extension ) {
        const fs::path arc_file_name = "file_type." + test_format.extension;

        SECTION( "Filesystem archive" ) {
            const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_format.format );
            REQUIRE_ITEM_DIRECTORY( info, "dir" );
            REQUIRE_ITEM_REGULAR( info, "regular" );
            REQUIRE_ITEM_SYMLINK( info, "symlink" );
            REQUIRE_ITEM_HIDDEN( info, "hidden" );
            REQUIRE_ITEM_READONLY( info, "read_only" );
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );
            const BitArchiveReader info( lib, file_buffer, test_format.format );
            REQUIRE_ITEM_DIRECTORY( info, "dir" );
            REQUIRE_ITEM_REGULAR( info, "regular" );
            REQUIRE_ITEM_SYMLINK( info, "symlink" );
            REQUIRE_ITEM_HIDDEN( info, "hidden" );
            REQUIRE_ITEM_READONLY( info, "read_only" );
        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            const BitArchiveReader info( lib, file_stream, test_format.format );
            REQUIRE_ITEM_DIRECTORY( info, "dir" );
            REQUIRE_ITEM_REGULAR( info, "regular" );
            REQUIRE_ITEM_SYMLINK( info, "symlink" );
            REQUIRE_ITEM_HIDDEN( info, "hidden" );
            REQUIRE_ITEM_READONLY( info, "read_only" );
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

#define REQUIRE_ITEM_UNICODE( info, item_name )                                                       \
    do {                                                                                              \
        auto iterator = (info).find( BIT7Z_STRING( item_name ) );                                     \
        REQUIRE( iterator != (info).cend() );                                                         \
        REQUIRE_FALSE( iterator->isDir() );                                                           \
        auto item_attributes = iterator->attributes();                                                \
        if ( ( item_attributes & FILE_ATTRIBUTE_WINDOWS_MASK ) != 0 ) {                               \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 );                           \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_HIDDEN ) == 0 );                              \
            REQUIRE( ( item_attributes & FILE_ATTRIBUTE_READONLY ) == 0 );                            \
        }                                                                                             \
        if ( ( item_attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) == FILE_ATTRIBUTE_UNIX_EXTENSION ) { \
            auto posix_attributes = item_attributes >> 16U;                                           \
            REQUIRE( !S_ISDIR( posix_attributes ) );                                                  \
            REQUIRE( S_ISREG( posix_attributes ) );                                                   \
            REQUIRE( !S_ISLNK( posix_attributes ) );                                                  \
        }                                                                                             \
        REQUIRE( iterator->name() == BIT7Z_STRING( item_name ) );                                     \
    } while ( false )

TEST_CASE( "BitArchiveReader: Correctly reading archive items with unicode names", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "metadata" / "unicode";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const auto test_format = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << test_format.extension ) {
        const fs::path arc_file_name = "unicode." + test_format.extension;

        SECTION( "Filesystem archive" ) {
            const BitArchiveReader info( lib, arc_file_name.string< tchar >(), test_format.format );
            REQUIRE_ITEM_UNICODE( info, "¡Porque sí!.doc" );
            REQUIRE_ITEM_UNICODE( info, "σύννεφα.jpg" );
            REQUIRE_ITEM_UNICODE( info, "юнікод.svg" );
            REQUIRE_ITEM_UNICODE( info, "ユニコード.pdf" );
        }

        SECTION( "Buffer archive" ) {
            const auto file_buffer = load_file( arc_file_name );
            const BitArchiveReader info( lib, file_buffer, test_format.format );
            REQUIRE_ITEM_UNICODE( info, "¡Porque sí!.doc" );
            REQUIRE_ITEM_UNICODE( info, "σύννεφα.jpg" );
            REQUIRE_ITEM_UNICODE( info, "юнікод.svg" );
            REQUIRE_ITEM_UNICODE( info, "ユニコード.pdf" );

        }

        SECTION( "Stream archive" ) {
            fs::ifstream file_stream{ arc_file_name, std::ios::binary };
            const BitArchiveReader info( lib, file_stream, test_format.format );
            REQUIRE_ITEM_UNICODE( info, "¡Porque sí!.doc" );
            REQUIRE_ITEM_UNICODE( info, "σύννεφα.jpg" );
            REQUIRE_ITEM_UNICODE( info, "юнікод.svg" );
            REQUIRE_ITEM_UNICODE( info, "ユニコード.pdf" );

        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

#ifdef BIT7Z_AUTO_FORMAT

TEST_CASE( "BitArchiveReader: Format detection of archives", "[bitarchivereader]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "detection" / "valid";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

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
            const BitArchiveReader reader{ lib, file.string< bit7z::tchar >() };
            REQUIRE( reader.detectedFormat() == test.format );

            if ( test.format != BitFormat::Mslz && test.extension != "part2.rar" && test.extension != "part3.rar" ) {
                REQUIRE_NOTHROW( reader.test() );
            }
        }

        SECTION( "Archive stream (signature) from a file" ) {
            std::ifstream stream{ "valid." + test.extension, std::ios::binary };
            const BitArchiveReader reader{ lib, stream };
            REQUIRE( reader.detectedFormat() == test.format );

            // TODO: Verify why testing of Mslz and multi-volume RAR archives fails
            if ( test.format != BitFormat::Mslz && test.extension.find( "part" ) != 0 ) {
                REQUIRE_NOTHROW( reader.test() );
            }
        }
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

#endif
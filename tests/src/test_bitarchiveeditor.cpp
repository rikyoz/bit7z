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
#include "utils/filesystem.hpp"
#include "utils/shared_lib.hpp"
#include "utils/sourcelocation.hpp"

#include <bit7z/bitformat.hpp>
#include <bit7z/bitarchiveeditor.hpp>
#include <bit7z/bitarchivereader.hpp>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

struct EditedArchive : TestOutputArchive {
    EditedArchive( std::string extension, const BitInOutFormat& format, std::size_t packedSize )
        : TestOutputArchive{ std::move( extension ), format, packedSize, multiple_items_content() } {}
};

TEST_CASE( "BitArchiveEditor: Opening a non-existing archive should throw", "[bitarchiveeditor]" ) {
    REQUIRE_THROWS( BitArchiveEditor{ test::sevenzip_lib(), BIT7Z_STRING( "non_existent.7z" ), BitFormat::SevenZip } );
}

TEST_CASE( "BitArchiveEditor: Deleting an item using an invalid index should throw and leave the archive unchanged",
           "[bitarchiveeditor]" ) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE( as< EditedArchive >(),
                                       EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
                                       EditedArchive{ "tar", BitFormat::Tar, 617472 },
                                       EditedArchive{ "wim", BitFormat::Wim, 615351 },
                                       EditedArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        const tstring editedArcPath = path_to_tstring( editedArcFileName );

        {
            BitArchiveEditor editor( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE_THROWS( editor.deleteItem( std::numeric_limits< std::uint32_t >::max() ) );
            REQUIRE_THROWS( editor.deleteItem( editor.itemsCount() ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            BitArchiveReader reader( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE_ARCHIVE_CONTENT( reader, testArchive );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE( "BitArchiveEditor: Deleting the only item in a single-item archive should leave the archive empty",
           "[bitarchiveeditor]" ) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "single_file";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    // For some reason, applying this change to the TAR archive fails on the GitHub Action's Ubuntu Runner
    // when testing Release builds with 7-Zip 22.01; it works locally and on VMs, so we disable it for now.
    const auto testArchive = GENERATE( as< EditedArchive >(),
                                       EditedArchive{ "7z", BitFormat::SevenZip, 478025 },
                                       //EditedArchive{ "bz2", BitFormat::BZip2, 0 },
                                       //EditedArchive{ "gz", BitFormat::GZip, 476404 },
                                       //EditedArchive{ "tar", BitFormat::Tar, 479232 },
                                       EditedArchive{ "wim", BitFormat::Wim, 478883 },
                                       //EditedArchive{ "xz", BitFormat::Xz, 478080 },
                                       EditedArchive{ "zip", BitFormat::Zip, 476398 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "clouds.jpg." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        const tstring editedArcPath = path_to_tstring( editedArcFileName );

        {
            BitArchiveEditor editor( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( 0 ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            BitArchiveReader reader( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.itemsCount() == 0 );
            REQUIRE( reader.size() == 0 );
            REQUIRE( reader.packSize() == 0 );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE( "BitArchiveEditor: Deleting a single file in an archive with multiple files (index-based)",
           "[bitarchiveeditor]" ) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_files";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE( as< EditedArchive >(),
                                       EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
                                       EditedArchive{ "tar", BitFormat::Tar, 617472 },
                                       EditedArchive{ "wim", BitFormat::Wim, 615351 },
                                       EditedArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_files." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        BitArchiveReader originalReader( test::sevenzip_lib(),
                                         path_to_tstring( originalArcPath ),
                                         testArchive.format() );
        const auto deletedItem = originalReader.find( loremIpsum.name );
        REQUIRE( deletedItem != originalReader.cend() );

        const tstring editedArcPath = path_to_tstring( editedArcFileName );
        {
            BitArchiveEditor editor( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( deletedItem->index() ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            BitArchiveReader reader( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.filesCount() == ( originalReader.filesCount() - 1 ) );
            REQUIRE( reader.find( deletedItem->path() ) == reader.cend() );

            const ExpectedItem expectedItem{ italy, "italy.svg", false };
            REQUIRE_ARCHIVE_ITEM( reader.format(), reader.itemAt( 0 ), expectedItem );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE( "BitArchiveEditor: Deleting (non-recursively) a single folder in an archive (index-based)",
           "[bitarchiveeditor]" ) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    // TODO: Check why we can't remove a folder when using the WIM format.
    const auto testArchive = GENERATE( as< EditedArchive >(),
                                       EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
                                       EditedArchive{ "tar", BitFormat::Tar, 617472 },
                                       //EditedArchive{ "wim", BitFormat::Wim, 615351 },
                                       EditedArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        BitArchiveReader originalReader( test::sevenzip_lib(),
                                         path_to_tstring( originalArcPath ),
                                         testArchive.format() );
        const auto deletedItem = originalReader.find( folder.name );
        REQUIRE( deletedItem != originalReader.cend() );

        const tstring editedArcPath = path_to_tstring( editedArcFileName );
        {
            BitArchiveEditor editor( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( deletedItem->index() ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            BitArchiveReader reader( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.itemsCount() == ( originalReader.itemsCount() - 1 ) );
            REQUIRE( reader.find( deletedItem->path() ) == reader.cend() );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE( "BitArchiveEditor: Deleting (recursively) a single folder in an archive (index-based)",
           "[bitarchiveeditor]" ) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE( as< EditedArchive >(),
                                       EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
                                       EditedArchive{ "tar", BitFormat::Tar, 617472 },
                                       EditedArchive{ "wim", BitFormat::Wim, 615351 },
                                       EditedArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        BitArchiveReader originalReader( test::sevenzip_lib(),
                                         path_to_tstring( originalArcPath ),
                                         testArchive.format() );
        const auto deletedItem = originalReader.find( folder.name );
        REQUIRE( deletedItem != originalReader.cend() );

        const tstring editedArcPath = path_to_tstring( editedArcFileName );
        {
            BitArchiveEditor editor( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( deletedItem->index(), DeletePolicy::RecurseDirs ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            BitArchiveReader reader( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.itemsCount() == 6 );
            REQUIRE( reader.find( deletedItem->path() ) == reader.cend() );
            REQUIRE( reader.find( path_to_tstring( fs::path{ "folder" } / clouds.name ) ) == reader.cend() );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE( "BitArchiveEditor: Deleting a single file in an archive with multiple files (path-based)",
           "[bitarchiveeditor]" ) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_files";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE( as< EditedArchive >(),
                                       EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
                                       EditedArchive{ "tar", BitFormat::Tar, 617472 },
                                       EditedArchive{ "wim", BitFormat::Wim, 615351 },
                                       EditedArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_files." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        const tstring editedArcPath = path_to_tstring( editedArcFileName );
        {
            BitArchiveEditor editor( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( italy.name ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            BitArchiveReader reader( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.filesCount() == 1 );
            REQUIRE( reader.find( italy.name ) == reader.cend() );

            const ExpectedItem expectedItem{ loremIpsum, "Lorem Ipsum.pdf", false };
            REQUIRE_ARCHIVE_ITEM( reader.format(), reader.itemAt( 0 ), expectedItem );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE( "BitArchiveEditor: Deleting (non-recursively) a single folder in an archive (path-based)",
           "[bitarchiveeditor]" ) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    // TODO: Check why we can't remove a folder when using the WIM format.
    const auto testArchive = GENERATE( as< EditedArchive >(),
                                       EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
                                       EditedArchive{ "tar", BitFormat::Tar, 617472 },
    //EditedArchive{ "wim", BitFormat::Wim, 615351 },
                                       EditedArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        std::uint32_t originalItemsCount = 0;
        const tstring editedArcPath = path_to_tstring( editedArcFileName );

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        {
            BitArchiveEditor editor( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            originalItemsCount = editor.itemsCount();
            REQUIRE_NOTHROW( editor.deleteItem( folder.name ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            BitArchiveReader reader( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.itemsCount() == ( originalItemsCount - 1 ) );
            REQUIRE( reader.find( folder.name ) == reader.cend() );
            REQUIRE( reader.find( path_to_tstring( fs::path{ "folder" } / clouds.name ) ) != reader.cend() );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE( "BitArchiveEditor: Deleting (non-recursively) a path with a trailing separator should fail",
           "[bitarchiveeditor]" ) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    // TODO: Check why we can't remove a folder when using the WIM format.
    const auto testArchive = GENERATE( as< EditedArchive >(),
                                       EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
                                       EditedArchive{ "tar", BitFormat::Tar, 617472 },
    //EditedArchive{ "wim", BitFormat::Wim, 615351 },
                                       EditedArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        const tstring editedArcPath = path_to_tstring( editedArcFileName );

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        {
            BitArchiveEditor editor( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE_THROWS( editor.deleteItem( path_to_tstring( fs::path{ folder.name } / "" ) ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            BitArchiveReader reader( test::sevenzip_lib(), editedArcPath, testArchive.format() );
            REQUIRE_ARCHIVE_CONTENT( reader, testArchive );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}


TEST_CASE( "BitArchiveEditor: Deleting (recursively) a single folder in an archive (path-based)",
           "[bitarchiveeditor]" ) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE( as< EditedArchive >(),
                                       EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
                                       EditedArchive{ "tar", BitFormat::Tar, 617472 },
                                       EditedArchive{ "wim", BitFormat::Wim, 615351 },
                                       EditedArchive{ "zip", BitFormat::Zip, 564097 } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        const tstring editedArcPath = path_to_tstring( editedArcFileName );

        auto deletedPath = GENERATE( as< fs::path >(),
                                     folder.name,
                                     fs::path{ folder.name } / "" );
        DYNAMIC_SECTION( "Path " << deletedPath ) {
            REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

            {
                BitArchiveEditor editor( test::sevenzip_lib(), editedArcPath, testArchive.format() );
                REQUIRE_NOTHROW( editor.deleteItem( path_to_tstring( deletedPath ), DeletePolicy::RecurseDirs ) );
                REQUIRE_NOTHROW( editor.applyChanges() );
            }

            {
                BitArchiveReader reader( test::sevenzip_lib(), editedArcPath, testArchive.format() );
                REQUIRE( reader.itemsCount() == 6 );
                REQUIRE( reader.find( folder.name ) == reader.cend() );
                REQUIRE( reader.find( path_to_tstring( fs::path{ "folder" } / clouds.name ) ) == reader.cend() );
            }

            REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
        }
    }
}
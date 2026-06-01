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
#include "utils/exception.hpp"
#include "utils/filesystem.hpp"
#include "utils/shared_lib.hpp"
#include "utils/sourcelocation.hpp"

#include <bit7z/biterror.hpp>
#include <bit7z/bitexception.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bitarchiveeditor.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchivewriter.hpp>
#include <bit7z/bittypes.hpp>

#include <map>
#include <utility>

#ifndef _WIN32
#include <unistd.h> // for geteuid
#endif

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

namespace {
struct EditedArchive : TestOutputArchive {
    EditedArchive( std::string extension, const BitInOutFormat& format, std::size_t packedSize )
        : TestOutputArchive{ std::move( extension ), format, packedSize, multipleItemsContent() } {}
};
} // namespace

TEST_CASE( "BitArchiveEditor: Opening a non-existing archive should throw", "[bitarchiveeditor]" ) {
    REQUIRE_THROWS( BitArchiveEditor{ test::sevenzipLib(), BIT7Z_STRING( "non_existent.7z" ), BitFormat::SevenZip } );
}

TEST_CASE(
    "BitArchiveEditor: Deleting an item using an invalid index should throw and leave the archive unchanged",
    "[bitarchiveeditor]"
) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE(
        as< EditedArchive >(),
        EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
        EditedArchive{ "tar", BitFormat::Tar, 617472 },
        EditedArchive{ "wim", BitFormat::Wim, 615351 },
        EditedArchive{ "zip", BitFormat::Zip, 564097 }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        const tstring editedArcPath = to_tstring( editedArcFileName );

        {
            BitArchiveEditor editor( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE_THROWS( editor.deleteItem( std::numeric_limits< std::uint32_t >::max() ) );
            REQUIRE_THROWS( editor.deleteItem( editor.itemsCount() ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            const BitArchiveReader reader( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE_ARCHIVE_CONTENT( reader, testArchive );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE(
    "BitArchiveEditor: Deleting the only item in a single-item archive should leave the archive empty",
    "[bitarchiveeditor]"
) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "single_file";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    // For some reason, applying this change to the TAR archive fails on the GitHub Action's Ubuntu Runner
    // when testing Release builds with 7-Zip 22.01; it works locally and on VMs, so we disable it for now.
    const auto testArchive = GENERATE(
        as< EditedArchive >(),
        EditedArchive{ "7z", BitFormat::SevenZip, 478025 },
        //EditedArchive{ "bz2", BitFormat::BZip2, 0 },
        //EditedArchive{ "gz", BitFormat::GZip, 476404 },
        //EditedArchive{ "tar", BitFormat::Tar, 479232 },
        EditedArchive{ "wim", BitFormat::Wim, 478883 },
        //EditedArchive{ "xz", BitFormat::Xz, 478080 },
        EditedArchive{ "zip", BitFormat::Zip, 476398 }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "clouds.jpg." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        const tstring editedArcPath = to_tstring( editedArcFileName );

        {
            BitArchiveEditor editor( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( 0 ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            const BitArchiveReader reader( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.itemsCount() == 0 );
            REQUIRE( reader.size() == 0 );
            REQUIRE( reader.packSize() == 0 );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE(
    "BitArchiveEditor: Deleting a single file in an archive with multiple files (index-based)",
    "[bitarchiveeditor]"
) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_files";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE(
        as< EditedArchive >(),
        EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
        EditedArchive{ "tar", BitFormat::Tar, 617472 },
        EditedArchive{ "wim", BitFormat::Wim, 615351 },
        EditedArchive{ "zip", BitFormat::Zip, 564097 }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_files." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        const BitArchiveReader originalReader(
            test::sevenzipLib(),
            to_tstring( originalArcPath ),
            testArchive.format()
        );
        const auto deletedItem = originalReader.find( loremIpsum.name );
        REQUIRE( deletedItem != originalReader.cend() );

        const tstring editedArcPath = to_tstring( editedArcFileName );
        {
            BitArchiveEditor editor( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( deletedItem->index() ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            const BitArchiveReader reader( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.filesCount() == ( originalReader.filesCount() - 1 ) );
            REQUIRE( reader.find( deletedItem->path() ) == reader.cend() );

            const ExpectedItem expectedItem{ italy, "italy.svg", false };
            REQUIRE_ARCHIVE_ITEM( reader.format(), reader.itemAt( 0 ), expectedItem );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE(
    "BitArchiveEditor: Deleting (non-recursively) a single folder in an archive (index-based)",
    "[bitarchiveeditor]"
) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    // TODO: Check why we can't remove a folder when using the WIM format.
    const auto testArchive = GENERATE(
        as< EditedArchive >(),
        EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
        EditedArchive{ "tar", BitFormat::Tar, 617472 },
        //EditedArchive{ "wim", BitFormat::Wim, 615351 },
        EditedArchive{ "zip", BitFormat::Zip, 564097 }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        const BitArchiveReader originalReader(
            test::sevenzipLib(),
            to_tstring( originalArcPath ),
            testArchive.format()
        );
        const auto deletedItem = originalReader.find( folder.name );
        REQUIRE( deletedItem != originalReader.cend() );

        const tstring editedArcPath = to_tstring( editedArcFileName );
        {
            BitArchiveEditor editor( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( deletedItem->index() ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            const BitArchiveReader reader( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.itemsCount() == ( originalReader.itemsCount() - 1 ) );
            REQUIRE( reader.find( deletedItem->path() ) == reader.cend() );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE(
    "BitArchiveEditor: Deleting (recursively) a single folder in an archive (index-based)",
    "[bitarchiveeditor]"
) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE(
        as< EditedArchive >(),
        EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
        EditedArchive{ "tar", BitFormat::Tar, 617472 },
        EditedArchive{ "wim", BitFormat::Wim, 615351 },
        EditedArchive{ "zip", BitFormat::Zip, 564097 }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        const BitArchiveReader originalReader(
            test::sevenzipLib(),
            to_tstring( originalArcPath ),
            testArchive.format()
        );
        const auto deletedItem = originalReader.find( folder.name );
        REQUIRE( deletedItem != originalReader.cend() );

        const tstring editedArcPath = to_tstring( editedArcFileName );
        {
            BitArchiveEditor editor( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( deletedItem->index(), DeletePolicy::RecurseDirs ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            const BitArchiveReader reader( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.itemsCount() == 6 );
            REQUIRE( reader.find( deletedItem->path() ) == reader.cend() );
            REQUIRE( reader.find( to_tstring( fs::path{ "folder" } / clouds.name ) ) == reader.cend() );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE(
    "BitArchiveEditor: Deleting a single file in an archive with multiple files (path-based)",
    "[bitarchiveeditor]"
) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_files";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE(
        as< EditedArchive >(),
        EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
        EditedArchive{ "tar", BitFormat::Tar, 617472 },
        EditedArchive{ "wim", BitFormat::Wim, 615351 },
        EditedArchive{ "zip", BitFormat::Zip, 564097 }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_files." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        const tstring editedArcPath = to_tstring( editedArcFileName );
        {
            BitArchiveEditor editor( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE_NOTHROW( editor.deleteItem( italy.name ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            const BitArchiveReader reader( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.filesCount() == 1 );
            REQUIRE( reader.find( italy.name ) == reader.cend() );

            const ExpectedItem expectedItem{ loremIpsum, "Lorem Ipsum.pdf", false };
            REQUIRE_ARCHIVE_ITEM( reader.format(), reader.itemAt( 0 ), expectedItem );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE(
    "BitArchiveEditor: Deleting (non-recursively) a single folder in an archive (path-based)",
    "[bitarchiveeditor]"
) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    // TODO: Check why we can't remove a folder when using the WIM format.
    const auto testArchive = GENERATE(
        as< EditedArchive >(),
        EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
        EditedArchive{ "tar", BitFormat::Tar, 617472 },
        //EditedArchive{ "wim", BitFormat::Wim, 615351 },
        EditedArchive{ "zip", BitFormat::Zip, 564097 }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        std::uint32_t originalItemsCount = 0;
        const tstring editedArcPath = to_tstring( editedArcFileName );

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        {
            BitArchiveEditor editor( test::sevenzipLib(), editedArcPath, testArchive.format() );
            originalItemsCount = editor.itemsCount();
            REQUIRE_NOTHROW( editor.deleteItem( folder.name ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            const BitArchiveReader reader( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE( reader.itemsCount() == ( originalItemsCount - 1 ) );
            REQUIRE( reader.find( folder.name ) == reader.cend() );
            REQUIRE( reader.find( to_tstring( fs::path{ "folder" } / clouds.name ) ) != reader.cend() );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE(
    "BitArchiveEditor: Deleting (non-recursively) a path with a trailing separator should fail",
    "[bitarchiveeditor]"
) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    // TODO: Check why we can't remove a folder when using the WIM format.
    const auto testArchive = GENERATE(
        as< EditedArchive >(),
        EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
        EditedArchive{ "tar", BitFormat::Tar, 617472 },
        //EditedArchive{ "wim", BitFormat::Wim, 615351 },
        EditedArchive{ "zip", BitFormat::Zip, 564097 }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        const tstring editedArcPath = to_tstring( editedArcFileName );

        REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

        {
            BitArchiveEditor editor( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE_THROWS( editor.deleteItem( to_tstring( fs::path{ folder.name } / "" ) ) );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        {
            const BitArchiveReader reader( test::sevenzipLib(), editedArcPath, testArchive.format() );
            REQUIRE_ARCHIVE_CONTENT( reader, testArchive );
        }

        REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
    }
}

TEST_CASE(
    "BitArchiveEditor: Deleting (recursively) a single folder in an archive (path-based)",
    "[bitarchiveeditor]"
) {
    const auto arcDir = fs::path{ test_archives_dir } / "extraction" / "multiple_items";

    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const auto testArchive = GENERATE(
        as< EditedArchive >(),
        EditedArchive{ "7z", BitFormat::SevenZip, 563797 },
        EditedArchive{ "tar", BitFormat::Tar, 617472 },
        EditedArchive{ "wim", BitFormat::Wim, 615351 },
        EditedArchive{ "zip", BitFormat::Zip, 564097 }
    );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension() ) {
        const fs::path originalArcPath = arcDir / ( "multiple_items." + testArchive.extension() );
        const fs::path editedArcFileName = "edited." + testArchive.extension();

        const tstring editedArcPath = to_tstring( editedArcFileName );

        auto deletedPath = GENERATE(
            as< fs::path >(),
            folder.name,
            fs::path{ folder.name } / ""
        );
        DYNAMIC_SECTION( "Path " << deletedPath ) {
            REQUIRE_NOTHROW( fs::copy_file( originalArcPath, editedArcFileName ) );

            {
                BitArchiveEditor editor( test::sevenzipLib(), editedArcPath, testArchive.format() );
                REQUIRE_NOTHROW( editor.deleteItem( to_tstring( deletedPath ), DeletePolicy::RecurseDirs ) );
                REQUIRE_NOTHROW( editor.applyChanges() );
            }

            {
                const BitArchiveReader reader( test::sevenzipLib(), editedArcPath, testArchive.format() );
                REQUIRE( reader.itemsCount() == 6 );
                REQUIRE( reader.find( folder.name ) == reader.cend() );
                REQUIRE( reader.find( to_tstring( fs::path{ "folder" } / clouds.name ) ) == reader.cend() );
            }

            REQUIRE_NOTHROW( fs::remove( editedArcFileName ) );
        }
    }
}

namespace {
template< std::size_t N >
auto as_bytes( const char (&text)[ N ] ) -> buffer_t { // NOLINT(*-avoid-c-arrays)
    /* N includes the trailing null terminator, which we exclude from the buffer
     * so the bytes match the string literal's content. */
    const auto* const begin = reinterpret_cast< const byte_t* >( text ); // NOLINT(*-pro-type-reinterpret-cast)
    return { begin, begin + ( N - 1 ) };
}

auto read_file_text( const fs::path& file ) -> std::string {
    fs::ifstream ifs{ file, std::ios::binary };
    return { std::istreambuf_iterator< char >( ifs ), std::istreambuf_iterator< char >() };
}
} // namespace

/*
 * BitArchiveEditor::applyChanges() must not follow a pre-placed "<archive>.tmp".
 *
 * The in-place update writes a temporary archive next to the target before
 * renaming it onto the target path. If that temporary path could be hijacked
 * by an attacker with write access to the parent directory (e.g. via a
 * pre-placed symlink), the update would overwrite the symlink target rather
 * than the archive.
 *
 * On POSIX the update must never touch "<archive>.tmp" at all. On Windows the
 * update may target "<archive>.tmp", but it must refuse if any entry (regular
 * file, symlink, junction, ...) is already present at that path.
 */
TEST_CASE( "BitArchiveEditor: applyChanges does not follow a pre-placed <archive>.tmp", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const fs::path archivePath = testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" );
    const tstring archivePathStr = to_tstring( archivePath );
    const tstring itemName = BIT7Z_STRING( "data.txt" );

    const buffer_t originalBytes = as_bytes( "Hello, World!" );
    const buffer_t updatedBytes = as_bytes( "Updated World!" );

    // Seed the archive with a single file so the editor has something to update.
    {
        BitArchiveWriter writer{ lib, BitFormat::SevenZip };
        writer.addFile( originalBytes, itemName );
        writer.compressTo( archivePathStr );
    }
    REQUIRE( fs::exists( archivePath ) );

#ifndef _WIN32
    SECTION( "pre-existing regular file at <archive>.tmp is left untouched" ) {
        /* POSIX-only: the update must never touch "<archive>.tmp", the
         * temporary archive is staged elsewhere. On Windows the update may
         * target this path, so this expectation does not hold there. */
        const fs::path archiveTmpPath = testDir.path() / "archive.7z.tmp";
        const std::string sentinelText = "sentinel-text";
        {
            fs::ofstream ofs{ archiveTmpPath, std::ios::binary };
            ofs << sentinelText;
        }
        REQUIRE( fs::exists( archiveTmpPath ) );

        {
            BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
            editor.updateItem( itemName, updatedBytes );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        REQUIRE( fs::exists( archiveTmpPath ) );
        REQUIRE( read_file_text( archiveTmpPath ) == sentinelText );

        const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
        REQUIRE( reader.itemsCount() == 1u );
    }

    SECTION( "pre-placed symlink at <archive>.tmp does not redirect the update" ) {
        const fs::path archiveTmpPath = testDir.path() / "archive.7z.tmp";
        const fs::path victim = testDir.path() / "victim.txt";
        const std::string victimText = "victim-original";
        {
            fs::ofstream ofs{ victim, std::ios::binary };
            ofs << victimText;
        }

        std::error_code linkError;
        fs::create_symlink( victim, archiveTmpPath, linkError );
        if ( linkError ) {
            SUCCEED( "Skipping: cannot create symlinks in this environment" );
        } else {
            {
                BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
                editor.updateItem( itemName, updatedBytes );
                REQUIRE_NOTHROW( editor.applyChanges() );
            }

            REQUIRE( read_file_text( victim ) == victimText );
            std::error_code symlinkStatusError;
            const auto status = fs::symlink_status( archiveTmpPath, symlinkStatusError );
            REQUIRE_FALSE( symlinkStatusError );
            REQUIRE( fs::is_symlink( status ) );
        }
    }

    SECTION( "applyChanges leaves the original archive intact when staging fails" ) {
        /* POSIX rollback test. Stripping write permission from the parent
         * directory forces the update to fail before any new bytes are
         * written. The original archive must therefore retain its seed
         * content rather than the would-be update. */
        if ( ::geteuid() == 0 ) {
            SUCCEED( "Skipping: root bypasses directory permissions" );
        } else {
            const auto originalPerms = fs::status( testDir.path() ).permissions();
            struct PermsRestorer {
                const fs::path& target;
                fs::perms perms;

                ~PermsRestorer() {
                    std::error_code ignored;
                    fs::permissions( target, perms, fs::perm_options::replace, ignored );
                }
            } const restorer{ testDir.path(), originalPerms };

            std::error_code permError;
            fs::permissions(
                testDir.path(),
                fs::perms::owner_read | fs::perms::owner_exec,
                fs::perm_options::replace,
                permError
            );
            if ( permError ) {
                SUCCEED( "Skipping: cannot restrict directory permissions" );
            } else {
                {
                    BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
                    editor.updateItem( itemName, updatedBytes );
                    REQUIRE_THROWS( editor.applyChanges() );
                }

                /* Restore write permission so the reader can traverse and so
                 * TempTestDirectory's destructor can clean up afterward. */
                fs::permissions( testDir.path(), originalPerms, fs::perm_options::replace );

                const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
                REQUIRE( reader.itemsCount() == 1u );
                buffer_t content;
                reader.extractTo( content );
                REQUIRE( content == originalBytes );
            }
        }
    }
#else
    SECTION( "pre-existing <archive>.tmp does not block the update" ) {
        /* Windows-only: when "<archive>.tmp" already exists, the update must
         * step past it via a numeric postfix and proceed. The pre-existing
         * entry must remain untouched. */
        const fs::path archiveTmpPath = testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z.tmp" );
        const std::string sentinelText = "sentinel-text";
        {
            fs::ofstream ofs{ archiveTmpPath, std::ios::binary };
            ofs << sentinelText;
        }
        REQUIRE( fs::exists( archiveTmpPath ) );

        {
            BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
            editor.updateItem( itemName, updatedBytes );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }

        // The pre-existing entry must not have been overwritten.
        REQUIRE( read_file_text( archiveTmpPath ) == sentinelText );

        // The archive must now hold the updated content.
        const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
        REQUIRE( reader.itemsCount() == 1u );
        buffer_t content;
        reader.extractTo( content );
        REQUIRE( content == updatedBytes );
    }

    SECTION( "pre-placed symlink at <archive>.tmp does not redirect the update" ) {
        /* Windows mirror of the POSIX symlink test. Creating a symlink on
         * Windows requires admin privileges or Developer Mode; the test
         * self-skips when it's not possible to create the symlink. When the
         * symlink can be created, the update must step past it via a numeric
         * postfix: the symlink target must remain untouched, and the symlink
         * itself must not be replaced. */
        const fs::path archiveTmpPath = testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z.tmp" );
        const fs::path victim = testDir.path() / BIT7Z_NATIVE_STRING( "victim.txt" );
        const std::string victimText = "victim-original";
        {
            fs::ofstream ofs{ victim, std::ios::binary };
            ofs << victimText;
        }

        std::error_code linkError;
        fs::create_symlink( victim, archiveTmpPath, linkError );
        if ( linkError ) {
            SUCCEED( "Skipping: cannot create symlinks in this environment" );
        } else {
            {
                BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
                editor.updateItem( itemName, updatedBytes );
                REQUIRE_NOTHROW( editor.applyChanges() );
            }

            // The victim file must not have been touched via the symlink.
            REQUIRE( read_file_text( victim ) == victimText );

            // The symlink itself must still be a symlink (not replaced).
            std::error_code symlinkStatusError;
            const auto status = fs::symlink_status( archiveTmpPath, symlinkStatusError );
            REQUIRE_FALSE( symlinkStatusError );
            REQUIRE( fs::is_symlink( status ) );

            // The archive must now hold the updated content.
            const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
            REQUIRE( reader.itemsCount() == 1u );
            buffer_t content;
            reader.extractTo( content );
            REQUIRE( content == updatedBytes );
        }
    }
#endif
}

/*
 * In-place multi-volume updates must be refused, matching 7-Zip's own
 * refusal of update operations on multi-volume archives. When the editor
 * is configured with a non-zero volume size, the in-place update must throw
 * before any output is written, so the original archive isn't silently
 * transformed into a single-file archive.
 */
TEST_CASE( "BitArchiveEditor: applyChanges refuses in-place multi-volume updates", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const fs::path archivePath = testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" );
    const tstring archivePathStr = to_tstring( archivePath );
    const tstring itemName = BIT7Z_STRING( "data.txt" );

    const buffer_t originalBytes = as_bytes( "Hello, World!" );
    const buffer_t updatedBytes = as_bytes( "Updated World!" );

    {
        BitArchiveWriter writer{ lib, BitFormat::SevenZip };
        writer.addFile( originalBytes, itemName );
        writer.compressTo( archivePathStr );
    }
    REQUIRE( fs::exists( archivePath ) );

    {
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        editor.setVolumeSize( 1024 ); // any non-zero size enables splitting
        editor.updateItem( itemName, updatedBytes );
        REQUIRE_THROWS( editor.applyChanges() );
    }

    const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
    REQUIRE( reader.itemsCount() == 1u );
    buffer_t content;
    reader.extractTo( content );
    REQUIRE( content == originalBytes );
}

namespace {
// Seeds a fresh SevenZip archive at the given path with the provided (name, content) entries.
void seedArchive(
    const Bit7zLibrary& lib,
    const tstring& archivePath,
    const std::vector< std::pair< tstring, buffer_t > >& entries
) {
    BitArchiveWriter writer{ lib, BitFormat::SevenZip };
    for ( const auto& entry : entries ) {
        writer.addFile( entry.second, entry.first );
    }
    writer.compressTo( archivePath );
}

// Returns the archive index of the item with the given in-archive path (7-Zip may not preserve insertion order).
auto indexOf( const BitArchiveReader& reader, const tstring& itemPath ) -> std::uint32_t {
    const auto item = reader.find( itemPath );
    REQUIRE( item != reader.cend() );
    return item->index();
}
} // namespace

TEST_CASE( "BitArchiveEditor: Opening an archive with an empty path should throw", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    REQUIRE_THROWS_CODE(
        BitArchiveEditor( lib, BIT7Z_STRING( "" ), BitFormat::SevenZip ),
        BitError::InvalidArchivePath
    );
}

TEST_CASE( "BitArchiveEditor: setUpdateMode rejects UpdateMode::None", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const tstring archivePathStr = to_tstring( testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" ) );
    seedArchive( lib, archivePathStr, { { BIT7Z_STRING( "alpha.txt" ), as_bytes( "Alpha original" ) } } );

    BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
    REQUIRE_NOTHROW( editor.setUpdateMode( UpdateMode::Update ) );
    REQUIRE_THROWS_CODE( editor.setUpdateMode( UpdateMode::None ), std::errc::invalid_argument );
}

TEST_CASE( "BitArchiveEditor: Renaming an item preserves its content", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const tstring archivePathStr = to_tstring( testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" ) );
    const buffer_t alphaBytes = as_bytes( "Alpha original" );
    const buffer_t betaBytes = as_bytes( "Beta original" );
    seedArchive( lib, archivePathStr, {
        { BIT7Z_STRING( "alpha.txt" ), alphaBytes },
        { BIT7Z_STRING( "beta.txt" ), betaBytes }
    } );

    const tstring newName = BIT7Z_STRING( "renamed.txt" );

    SECTION( "by index" ) {
        std::uint32_t alphaIndex = 0;
        {
            const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
            alphaIndex = indexOf( reader, BIT7Z_STRING( "alpha.txt" ) );
        }
        {
            BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
            editor.renameItem( alphaIndex, newName );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }
    }

    SECTION( "by path" ) {
        {
            BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
            editor.renameItem( BIT7Z_STRING( "alpha.txt" ), newName );
            REQUIRE_NOTHROW( editor.applyChanges() );
        }
    }

    const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
    REQUIRE( reader.itemsCount() == 2u );
    std::map< tstring, buffer_t > items;
    reader.extractTo( items );
    // The renamed item keeps its original content, the old name is gone, and the other item is untouched.
    REQUIRE( items.count( BIT7Z_STRING( "alpha.txt" ) ) == 0 );
    REQUIRE( items[ newName ] == alphaBytes );
    REQUIRE( items[ BIT7Z_STRING( "beta.txt" ) ] == betaBytes );
}

TEST_CASE( "BitArchiveEditor: Updating an item by index replaces its content", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const tstring archivePathStr = to_tstring( testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" ) );
    const buffer_t betaBytes = as_bytes( "Beta original" );
    seedArchive( lib, archivePathStr, {
        { BIT7Z_STRING( "alpha.txt" ), as_bytes( "Alpha original" ) },
        { BIT7Z_STRING( "beta.txt" ), betaBytes }
    } );

    const auto sourceFile = fs::path{ test_filesystem_dir } / "folder" / "clouds.jpg";
    const buffer_t expectedBytes = loadFile( sourceFile );

    std::uint32_t alphaIndex = 0;
    {
        const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
        alphaIndex = indexOf( reader, BIT7Z_STRING( "alpha.txt" ) );
    }

    SECTION( "from a buffer" ) {
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        editor.updateItem( alphaIndex, expectedBytes );
        REQUIRE_NOTHROW( editor.applyChanges() );
    }
    SECTION( "from a file" ) {
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        editor.updateItem( alphaIndex, to_tstring( sourceFile ) );
        REQUIRE_NOTHROW( editor.applyChanges() );
    }
    SECTION( "from a stream" ) {
        fs::ifstream sourceStream{ sourceFile, std::ios::binary };
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        editor.updateItem( alphaIndex, sourceStream );
        REQUIRE_NOTHROW( editor.applyChanges() );
    }

    const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
    REQUIRE( reader.itemsCount() == 2u );
    std::map< tstring, buffer_t > items;
    reader.extractTo( items );
    REQUIRE( items[ BIT7Z_STRING( "alpha.txt" ) ] == expectedBytes );
    REQUIRE( items[ BIT7Z_STRING( "beta.txt" ) ] == betaBytes );
}

TEST_CASE( "BitArchiveEditor: Updating an item by path replaces its content", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const tstring archivePathStr = to_tstring( testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" ) );
    const buffer_t betaBytes = as_bytes( "Beta original" );
    seedArchive( lib, archivePathStr, {
        { BIT7Z_STRING( "alpha.txt" ), as_bytes( "Alpha original" ) },
        { BIT7Z_STRING( "beta.txt" ), betaBytes }
    } );

    const auto sourceFile = fs::path{ test_filesystem_dir } / "folder" / "clouds.jpg";
    const buffer_t expectedBytes = loadFile( sourceFile );

    SECTION( "from a file" ) {
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        editor.updateItem( BIT7Z_STRING( "alpha.txt" ), to_tstring( sourceFile ) );
        REQUIRE_NOTHROW( editor.applyChanges() );
    }
    SECTION( "from a stream" ) {
        fs::ifstream sourceStream{ sourceFile, std::ios::binary };
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        editor.updateItem( BIT7Z_STRING( "alpha.txt" ), sourceStream );
        REQUIRE_NOTHROW( editor.applyChanges() );
    }

    const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
    REQUIRE( reader.itemsCount() == 2u );
    std::map< tstring, buffer_t > items;
    reader.extractTo( items );
    REQUIRE( items[ BIT7Z_STRING( "alpha.txt" ) ] == expectedBytes );
    REQUIRE( items[ BIT7Z_STRING( "beta.txt" ) ] == betaBytes );
}

TEST_CASE( "BitArchiveEditor: Editing an item at an invalid index throws", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const tstring archivePathStr = to_tstring( testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" ) );
    seedArchive( lib, archivePathStr, {
        { BIT7Z_STRING( "alpha.txt" ), as_bytes( "Alpha original" ) },
        { BIT7Z_STRING( "beta.txt" ), as_bytes( "Beta original" ) }
    } );

    const buffer_t someBytes = as_bytes( "irrelevant" );

    SECTION( "index out of range" ) {
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        REQUIRE_THROWS_CODE(
            editor.renameItem( std::numeric_limits< std::uint32_t >::max(), BIT7Z_STRING( "x.txt" ) ),
            BitError::InvalidIndex
        );
        REQUIRE_THROWS_CODE( editor.updateItem( editor.itemsCount(), someBytes ), BitError::InvalidIndex );
    }

    SECTION( "index previously marked as deleted" ) {
        std::uint32_t alphaIndex = 0;
        {
            const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
            alphaIndex = indexOf( reader, BIT7Z_STRING( "alpha.txt" ) );
        }
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        editor.deleteItem( alphaIndex );
        REQUIRE_THROWS_CODE( editor.renameItem( alphaIndex, BIT7Z_STRING( "x.txt" ) ), BitError::ItemMarkedAsDeleted );
        REQUIRE_THROWS_CODE( editor.updateItem( alphaIndex, someBytes ), BitError::ItemMarkedAsDeleted );
    }
}

TEST_CASE( "BitArchiveEditor: Editing a missing or deleted path throws", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const tstring archivePathStr = to_tstring( testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" ) );
    seedArchive( lib, archivePathStr, {
        { BIT7Z_STRING( "alpha.txt" ), as_bytes( "Alpha original" ) },
        { BIT7Z_STRING( "beta.txt" ), as_bytes( "Beta original" ) }
    } );

    const buffer_t someBytes = as_bytes( "irrelevant" );

    SECTION( "path not found" ) {
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        REQUIRE_THROWS_CODE(
            editor.updateItem( BIT7Z_STRING( "missing.txt" ), someBytes ),
            std::errc::no_such_file_or_directory
        );
        REQUIRE_THROWS_CODE(
            editor.renameItem( BIT7Z_STRING( "missing.txt" ), BIT7Z_STRING( "x.txt" ) ),
            std::errc::no_such_file_or_directory
        );
    }

    SECTION( "path marked as deleted" ) {
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        editor.deleteItem( BIT7Z_STRING( "alpha.txt" ) );
        REQUIRE_THROWS_CODE(
            editor.updateItem( BIT7Z_STRING( "alpha.txt" ), someBytes ),
            BitError::ItemMarkedAsDeleted
        );
        REQUIRE_THROWS_CODE(
            editor.renameItem( BIT7Z_STRING( "alpha.txt" ), BIT7Z_STRING( "x.txt" ) ),
            BitError::ItemMarkedAsDeleted
        );
    }
}

TEST_CASE( "BitArchiveEditor: Deleting an invalid path throws", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const tstring archivePathStr = to_tstring( testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" ) );
    seedArchive( lib, archivePathStr, { { BIT7Z_STRING( "alpha.txt" ), as_bytes( "Alpha original" ) } } );

    BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };

    SECTION( "empty path" ) {
        REQUIRE_THROWS_CODE( editor.deleteItem( BIT7Z_STRING( "" ) ), std::errc::invalid_argument );
    }
    SECTION( "path with a leading separator" ) {
        REQUIRE_THROWS_CODE( editor.deleteItem( BIT7Z_STRING( "/alpha.txt" ) ), std::errc::invalid_argument );
    }
}

TEST_CASE( "BitArchiveEditor: Adding a new file to an existing archive", "[bitarchiveeditor]" ) {
    const Bit7zLibrary lib{ sevenzipLibPath() };
    const TempTestDirectory testDir{ "bitarchiveeditor" };

    const tstring archivePathStr = to_tstring( testDir.path() / BIT7Z_NATIVE_STRING( "archive.7z" ) );
    const buffer_t alphaBytes = as_bytes( "Alpha original" );
    const buffer_t betaBytes = as_bytes( "Beta original" );
    seedArchive( lib, archivePathStr, {
        { BIT7Z_STRING( "alpha.txt" ), alphaBytes },
        { BIT7Z_STRING( "beta.txt" ), betaBytes }
    } );

    const buffer_t addedBytes = as_bytes( "Added content" );
    {
        BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
        editor.addFile( addedBytes, BIT7Z_STRING( "added.txt" ) );
        REQUIRE_NOTHROW( editor.applyChanges() );
    }

    const BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
    REQUIRE( reader.itemsCount() == 3u );
    std::map< tstring, buffer_t > items;
    reader.extractTo( items );
    REQUIRE( items[ BIT7Z_STRING( "added.txt" ) ] == addedBytes );
    REQUIRE( items[ BIT7Z_STRING( "alpha.txt" ) ] == alphaBytes );
    REQUIRE( items[ BIT7Z_STRING( "beta.txt" ) ] == betaBytes );
}

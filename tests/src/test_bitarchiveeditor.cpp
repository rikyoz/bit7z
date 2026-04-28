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

#include "utils/filesystem.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/bitarchiveeditor.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchivewriter.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bittypes.hpp>

#include <internal/fs.hpp>
#include <internal/stringutil.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#ifndef _WIN32
#include <unistd.h> // for geteuid
#endif

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

namespace {

/* Local stand-in for TempTestDirectory (introduced on the develop branch
 * for v4.1). The public surface (constructor, path() accessor, tstring
 * conversion) mirrors TempTestDirectory so the eventual swap is just a
 * type rename. TODO: Replace this with TempTestDirectory once develop lands. */
class ScopedTestDir {
    public:
        explicit ScopedTestDir( const std::string& dirName ) {
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            mDirectory = fs::temp_directory_path() / ( dirName + "_" + std::to_string( stamp ) );
            fs::create_directories( mDirectory );
        }

        ScopedTestDir( const ScopedTestDir& ) = delete;
        ScopedTestDir( ScopedTestDir&& ) = delete;
        auto operator=( const ScopedTestDir& ) -> ScopedTestDir& = delete;
        auto operator=( ScopedTestDir&& ) -> ScopedTestDir& = delete;

        ~ScopedTestDir() {
            std::error_code ignored;
            fs::remove_all( mDirectory, ignored );
        }

        BIT7Z_NODISCARD
        auto path() const -> const fs::path& { return mDirectory; }

        operator tstring() const { return path_to_tstring( mDirectory ); } // NOLINT(*-explicit-constructor, *-explicit-conversions)

    private:
        fs::path mDirectory;
};

template< std::size_t N >
auto as_bytes( const char ( &text )[ N ] ) -> buffer_t {
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
    const Bit7zLibrary lib{ sevenzip_lib_path() };
    const ScopedTestDir scoped{ "test_bitarchiveeditor" };

    const fs::path archivePath = scoped.path() / BIT7Z_NATIVE_STRING( "archive.7z" );
    const tstring archivePathStr = path_to_tstring( archivePath );
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
        const fs::path archiveTmpPath = scoped.path() / "archive.7z.tmp";
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

        BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
        REQUIRE( reader.itemsCount() == 1u );
    }

    SECTION( "pre-placed symlink at <archive>.tmp does not redirect the update" ) {
        const fs::path archiveTmpPath = scoped.path() / "archive.7z.tmp";
        const fs::path victim = scoped.path() / "victim.txt";
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
            const auto originalPerms = fs::status( scoped.path() ).permissions();
            struct PermsRestorer {
                const fs::path& target;
                fs::perms perms;
                ~PermsRestorer() {
                    std::error_code ignored;
                    fs::permissions( target, perms, fs::perm_options::replace, ignored );
                }
            } const restorer{ scoped.path(), originalPerms };

            std::error_code permError;
            fs::permissions( scoped.path(),
                             fs::perms::owner_read | fs::perms::owner_exec,
                             fs::perm_options::replace,
                             permError );
            if ( permError ) {
                SUCCEED( "Skipping: cannot restrict directory permissions" );
            } else {
                {
                    BitArchiveEditor editor{ lib, archivePathStr, BitFormat::SevenZip };
                    editor.updateItem( itemName, updatedBytes );
                    REQUIRE_THROWS( editor.applyChanges() );
                }

                /* Restore write permission so the reader can traverse and so
                 * ScopedTestDir's destructor can clean up afterward. */
                fs::permissions( scoped.path(), originalPerms, fs::perm_options::replace );

                BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
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
        const fs::path archiveTmpPath = scoped.path() / BIT7Z_NATIVE_STRING( "archive.7z.tmp" );
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
        BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
        REQUIRE( reader.itemsCount() == 1u );
        std::vector< byte_t > content;
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
        const fs::path archiveTmpPath = scoped.path() / BIT7Z_NATIVE_STRING( "archive.7z.tmp" );
        const fs::path victim = scoped.path() / BIT7Z_NATIVE_STRING( "victim.txt" );
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
            BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
            REQUIRE( reader.itemsCount() == 1u );
            std::vector< byte_t > content;
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
    const Bit7zLibrary lib{ sevenzip_lib_path() };
    const ScopedTestDir scoped{ "test_bitarchiveeditor" };

    const fs::path archivePath = scoped.path() / BIT7Z_NATIVE_STRING( "archive.7z" );
    const tstring archivePathStr = path_to_tstring( archivePath );
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

    BitArchiveReader reader{ lib, archivePathStr, BitFormat::SevenZip };
    REQUIRE( reader.itemsCount() == 1u );
    buffer_t content;
    reader.extractTo( content );
    REQUIRE( content == originalBytes );
}
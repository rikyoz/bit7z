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
#include "utils/filesystem.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/bitarchiveitem.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitexception.hpp>
#include <bit7z/bitfilecompressor.hpp>
#include <bit7z/bitformat.hpp>

#include <map>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

#ifdef BIT7Z_TESTS_FILESYSTEM

namespace {

// Builds a FilesystemItemInfo describing an aliased (renamed) item: it carries the alias' name and
// extension, but the contents (type, size, CRC) of the original source file. This lets the shared
// require_archive_content check verify renamed items just like any other item.
auto aliased_info( const FilesystemItemInfo& source, const tchar* name, const tchar* ext ) -> FilesystemItemInfo {
    return { name, ext, source.type, source.size, source.crc32, source.crc16 };
}

// Expected content of the archive produced by the "renamed.svg" / "document.pdf" aliases.
auto renamed_pair_content() -> const ArchiveContent& {
    static const auto renamedItaly = aliased_info( italy, BIT7Z_STRING( "renamed.svg" ), BIT7Z_STRING( "svg" ) );
    static const auto renamedLorem = aliased_info( loremIpsum, BIT7Z_STRING( "document.pdf" ), BIT7Z_STRING( "pdf" ) );
    static const ArchiveContent instance{
        2,
        italy.size + loremIpsum.size,
        { { renamedItaly, "renamed.svg", false }, { renamedLorem, "document.pdf", false } }
    };
    return instance;
}

// Expected content of the archive produced by the "alias_italy.svg" / "alias_lorem.pdf" aliases.
auto aliased_map_content() -> const ArchiveContent& {
    static const auto aliasItaly = aliased_info( italy, BIT7Z_STRING( "alias_italy.svg" ), BIT7Z_STRING( "svg" ) );
    static const auto aliasLorem = aliased_info( loremIpsum, BIT7Z_STRING( "alias_lorem.pdf" ), BIT7Z_STRING( "pdf" ) );
    static const ArchiveContent instance{
        2,
        italy.size + loremIpsum.size,
        { { aliasItaly, "alias_italy.svg", false }, { aliasLorem, "alias_lorem.pdf", false } }
    };
    return instance;
}

} // namespace

// Checks the full content of a freshly created archive against the expected ArchiveContent.
// The packed size is passed as 0, so that REQUIRE_ARCHIVE_CONTENT skips the (non-deterministic)
// packed-size check; everything else (item set, names, paths, sizes, CRCs, counts) is verified.
#define REQUIRE_NEW_ARCHIVE( reader, content ) \
    require_archive_content( reader, TestArchiveContent{ 0, content }, BIT7Z_CURRENT_LOCATION )

TEST_CASE( "BitFileCompressor: Compressing a vector of filesystem paths", "[bitfilecompressor]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );

    const auto outArchive = outDir.path() / ( "output." + testFormat.extension );
    const auto outArchiveStr = to_tstring( outArchive );

    const BitFileCompressor compressor{ test::sevenzip_lib(), testFormat.format };

    DYNAMIC_SECTION( testFormat.extension << ": only files" ) {
        const std::vector< tstring > inPaths{ italy.name, loremIpsum.name };
        REQUIRE_NOTHROW( compressor.compress( inPaths, outArchiveStr ) );

        // The reader is scoped so that it releases the archive file before it is removed below.
        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, testFormat.format };
        REQUIRE_NEW_ARCHIVE( reader, multiple_files_content() );
    }

    DYNAMIC_SECTION( testFormat.extension << ": files and a directory (contents recursively included)" ) {
        const std::vector< tstring > inPaths{ italy.name, BIT7Z_STRING( "folder/subfolder2" ) };
        REQUIRE_NOTHROW( compressor.compress( inPaths, outArchiveStr ) );

        ArchiveContent content{
            4,
            italy.size + homework.size + quickBrown.size + frequency.size,
            {
                { italy, "italy.svg", false },
                { subfolder2, "folder/subfolder2", false },
                { homework, "folder/subfolder2/homework.doc", false },
                { quickBrown, "folder/subfolder2/The quick brown fox.pdf", false },
                { frequency, "folder/subfolder2/frequency.xlsx", false }
            }
        };
        // WIM materializes the intermediate parent directories of an item, so it also stores a
        // "folder" entry for the parent of the indexed "folder/subfolder2"; the other formats don't.
        if ( testFormat.format == BitFormat::Wim ) {
            content.items.push_back( { folder, "folder", false } );
        }

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, testFormat.format };
        REQUIRE_NEW_ARCHIVE( reader, content );
    }

    DYNAMIC_SECTION( testFormat.extension << ": files and multiple directories (canonical multiple-items archive)" ) {
        // Compressing this exact mix of plain files, an empty folder, a dot-folder, and a nested
        // folder tree reproduces the canonical multiple_items_content() manifest. Note: the
        // test_filesystem also holds a top-level Unicode file, intentionally not listed here, as
        // multiple_items_content() does not include it.
        const std::vector< tstring > inPaths{
            italy.name,
            loremIpsum.name,
            noext.name,
            dotFolder.name,
            emptyFolder.name,
            folder.name
        };
        REQUIRE_NOTHROW( compressor.compress( inPaths, outArchiveStr ) );

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, testFormat.format };
        REQUIRE_NEW_ARCHIVE( reader, multiple_items_content() );
    }

    fs::remove( outArchive );
}

TEST_CASE( "BitFileCompressor: Compressing filesystem paths using aliases (vector of pairs)", "[bitfilecompressor]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "wim", BitFormat::Wim },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );

    const auto outArchive = outDir.path() / ( "output." + testFormat.extension );
    const auto outArchiveStr = to_tstring( outArchive );

    DYNAMIC_SECTION( "Format " << testFormat.extension ) {
        const BitFileCompressor compressor{ test::sevenzip_lib(), testFormat.format };

        const std::vector< std::pair< tstring, tstring > > inPaths{
            { italy.name, BIT7Z_STRING( "renamed.svg" ) },
            { loremIpsum.name, BIT7Z_STRING( "document.pdf" ) }
        };
        REQUIRE_NOTHROW( compressor.compress( inPaths, outArchiveStr ) );

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, testFormat.format };
        REQUIRE_NEW_ARCHIVE( reader, renamed_pair_content() );
    }

    fs::remove( outArchive );
}

TEST_CASE( "BitFileCompressor: Compressing filesystem paths using aliases (map)", "[bitfilecompressor]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };
    const auto outArchive = outDir.path() / "output.7z";
    const auto outArchiveStr = to_tstring( outArchive );

    const BitFileCompressor compressor{ test::sevenzip_lib(), BitFormat::SevenZip };

    const std::map< tstring, tstring > inPaths{
        { italy.name, BIT7Z_STRING( "alias_italy.svg" ) },
        { loremIpsum.name, BIT7Z_STRING( "alias_lorem.pdf" ) }
    };
    REQUIRE_NOTHROW( compressor.compress( inPaths, outArchiveStr ) );

    {
        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, BitFormat::SevenZip };
        REQUIRE_NEW_ARCHIVE( reader, aliased_map_content() );
    }

    fs::remove( outArchive );
}

TEST_CASE( "BitFileCompressor: compressFiles ignores directories in the input vector", "[bitfilecompressor]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };
    const auto outArchive = outDir.path() / "output.7z";
    const auto outArchiveStr = to_tstring( outArchive );

    const BitFileCompressor compressor{ test::sevenzip_lib(), BitFormat::SevenZip };

    // "folder" is a directory, so compressFiles must ignore it and compress only the two files.
    const std::vector< tstring > inFiles{ italy.name, loremIpsum.name, BIT7Z_STRING( "folder" ) };
    REQUIRE_NOTHROW( compressor.compressFiles( inFiles, outArchiveStr ) );

    {
        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, BitFormat::SevenZip };
        REQUIRE_NEW_ARCHIVE( reader, multiple_files_content() );
    }

    fs::remove( outArchive );
}

TEST_CASE( "BitFileCompressor: compressFiles from a directory (filtered)", "[bitfilecompressor]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };
    const auto outArchive = outDir.path() / "output.7z";
    const auto outArchiveStr = to_tstring( outArchive );

    const BitFileCompressor compressor{ test::sevenzip_lib(), BitFormat::SevenZip };

    SECTION( "All files (recursive, default filter)" ) {
        // compressFiles flattens to files only: the directory entry itself is not stored.
        REQUIRE_NOTHROW( compressor.compressFiles( BIT7Z_STRING( "folder/subfolder2" ), outArchiveStr ) );

        const ArchiveContent content{
            3,
            homework.size + quickBrown.size + frequency.size,
            {
                { homework, "folder/subfolder2/homework.doc", false },
                { quickBrown, "folder/subfolder2/The quick brown fox.pdf", false },
                { frequency, "folder/subfolder2/frequency.xlsx", false }
            }
        };

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, BitFormat::SevenZip };
        REQUIRE_NEW_ARCHIVE( reader, content );
    }

    SECTION( "Only the files matching the wildcard filter" ) {
        REQUIRE_NOTHROW(
            compressor.compressFiles(
                BIT7Z_STRING( "folder/subfolder2" ),
                outArchiveStr,
                true,
                BIT7Z_STRING( "*.doc" )
            )
        );

        const ArchiveContent content{
            1,
            homework.size,
            { { homework, "folder/subfolder2/homework.doc", false } }
        };

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, BitFormat::SevenZip };
        REQUIRE_NEW_ARCHIVE( reader, content );
    }

    fs::remove( outArchive );
}

TEST_CASE( "BitFileCompressor: compressDirectory stores the directory and its contents", "[bitfilecompressor]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };
    const auto outArchive = outDir.path() / "output.7z";
    const auto outArchiveStr = to_tstring( outArchive );

    const BitFileCompressor compressor{ test::sevenzip_lib(), BitFormat::SevenZip };

    // Unlike compressFiles, compressDirectory keeps the folder structure, including the root folder entry.
    REQUIRE_NOTHROW( compressor.compressDirectory( BIT7Z_STRING( "folder/subfolder2" ), outArchiveStr ) );

    {
        const ArchiveContent content{
            3,
            homework.size + quickBrown.size + frequency.size,
            {
                { subfolder2, "folder/subfolder2", false },
                { homework, "folder/subfolder2/homework.doc", false },
                { quickBrown, "folder/subfolder2/The quick brown fox.pdf", false },
                { frequency, "folder/subfolder2/frequency.xlsx", false }
            }
        };

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, BitFormat::SevenZip };
        REQUIRE_NEW_ARCHIVE( reader, content );
    }

    fs::remove( outArchive );
}

TEST_CASE(
    "BitFileCompressor: compressDirectoryContents stores the contents without the root folder",
    "[bitfilecompressor]"
) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };
    const auto outArchive = outDir.path() / "output.7z";
    const auto outArchiveStr = to_tstring( outArchive );

    const BitFileCompressor compressor{ test::sevenzip_lib(), BitFormat::SevenZip };

    // The contents are stored relative to inDir: the "folder/subfolder2" prefix is stripped.
    REQUIRE_NOTHROW( compressor.compressDirectoryContents( BIT7Z_STRING( "folder/subfolder2" ), outArchiveStr ) );

    {
        const ArchiveContent content{
            3,
            homework.size + quickBrown.size + frequency.size,
            {
                { homework, "homework.doc", false },
                { quickBrown, "The quick brown fox.pdf", false },
                { frequency, "frequency.xlsx", false }
            }
        };

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, BitFormat::SevenZip };
        REQUIRE_NEW_ARCHIVE( reader, content );
    }

    fs::remove( outArchive );
}

// This case contrasts the three directory-compression methods on a tree that actually contains
// sub-folders ("folder" holds the empty "subfolder", "subfolder2", and the file "clouds.jpg"),
// so that the documented difference in how each handles the folder structure is exercised.
TEST_CASE(
    "BitFileCompressor: directory methods differ in how they store the folder structure",
    "[bitfilecompressor]"
) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };

    // WIM is excluded here: it materializes intermediate parent directories, so the flattened output
    // of compressFiles would not match (it would also contain the synthesized parent folder entries).
    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "7z", BitFormat::SevenZip },
        TestOutputFormat{ "tar", BitFormat::Tar },
        TestOutputFormat{ "zip", BitFormat::Zip }
    );

    const auto outArchive = outDir.path() / ( "output." + testFormat.extension );
    const auto outArchiveStr = to_tstring( outArchive );

    const BitFileCompressor compressor{ test::sevenzip_lib(), testFormat.format };

    DYNAMIC_SECTION( testFormat.extension << ": compressFiles stores only files, flattening folder entries" ) {
        REQUIRE_NOTHROW( compressor.compressFiles( BIT7Z_STRING( "folder" ), outArchiveStr ) );

        const ArchiveContent content{
            4,
            clouds.size + homework.size + quickBrown.size + frequency.size,
            {
                { clouds, "folder/clouds.jpg", false },
                { homework, "folder/subfolder2/homework.doc", false },
                { quickBrown, "folder/subfolder2/The quick brown fox.pdf", false },
                { frequency, "folder/subfolder2/frequency.xlsx", false }
            }
        };

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, testFormat.format };
        REQUIRE_NEW_ARCHIVE( reader, content );
    }

    DYNAMIC_SECTION( testFormat.extension << ": compressDirectory stores the root folder and all sub-folders" ) {
        REQUIRE_NOTHROW( compressor.compressDirectory( BIT7Z_STRING( "folder" ), outArchiveStr ) );

        const ArchiveContent content{
            4,
            clouds.size + homework.size + quickBrown.size + frequency.size,
            {
                { folder, "folder", false },
                { clouds, "folder/clouds.jpg", false },
                { subfolder, "folder/subfolder", false },
                { subfolder2, "folder/subfolder2", false },
                { homework, "folder/subfolder2/homework.doc", false },
                { quickBrown, "folder/subfolder2/The quick brown fox.pdf", false },
                { frequency, "folder/subfolder2/frequency.xlsx", false }
            }
        };

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, testFormat.format };
        REQUIRE_NEW_ARCHIVE( reader, content );
    }

    DYNAMIC_SECTION( testFormat.extension << ": compressDirectoryContents drops the root but keeps sub-folders" ) {
        REQUIRE_NOTHROW( compressor.compressDirectoryContents( BIT7Z_STRING( "folder" ), outArchiveStr ) );

        // Unlike compressFiles, the (empty) sub-folder entries are retained, just without the root.
        const ArchiveContent content{
            4,
            clouds.size + homework.size + quickBrown.size + frequency.size,
            {
                { clouds, "clouds.jpg", false },
                { subfolder, "subfolder", false },
                { subfolder2, "subfolder2", false },
                { homework, "subfolder2/homework.doc", false },
                { quickBrown, "subfolder2/The quick brown fox.pdf", false },
                { frequency, "subfolder2/frequency.xlsx", false }
            }
        };

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, testFormat.format };
        REQUIRE_NEW_ARCHIVE( reader, content );
    }

    DYNAMIC_SECTION( testFormat.extension << ": compressDirectoryContents non-recursively stores top-level files" ) {
        REQUIRE_NOTHROW(
            compressor.compressDirectoryContents(
                BIT7Z_STRING( "folder" ),
                outArchiveStr,
                false,
                BIT7Z_STRING( "*" )
            )
        );

        const ArchiveContent content{
            1,
            clouds.size,
            { { clouds, "clouds.jpg", false } }
        };

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, testFormat.format };
        REQUIRE_NEW_ARCHIVE( reader, content );
    }

    fs::remove( outArchive );
}

TEST_CASE( "BitFileCompressor: Compressing filesystem paths to a standard output stream", "[bitfilecompressor]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };
    const auto outArchive = outDir.path() / "output.7z";
    const auto outArchiveStr = to_tstring( outArchive );

    const BitFileCompressor compressor{ test::sevenzip_lib(), BitFormat::SevenZip };

    SECTION( "Vector of paths" ) {
        {
            const std::vector< tstring > inPaths{ italy.name, loremIpsum.name };
            fs::ofstream outStream{ outArchive, std::ios::binary };
            REQUIRE_NOTHROW( compressor.compress( inPaths, outStream ) );
        }

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, BitFormat::SevenZip };
        REQUIRE_NEW_ARCHIVE( reader, multiple_files_content() );
    }

    SECTION( "Map of paths with aliases" ) {
        {
            const std::map< tstring, tstring > inPaths{
                { italy.name, BIT7Z_STRING( "alias_italy.svg" ) },
                { loremIpsum.name, BIT7Z_STRING( "alias_lorem.pdf" ) }
            };
            fs::ofstream outStream{ outArchive, std::ios::binary };
            REQUIRE_NOTHROW( compressor.compress( inPaths, outStream ) );
        }

        const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, BitFormat::SevenZip };
        REQUIRE_NEW_ARCHIVE( reader, aliased_map_content() );
    }

    fs::remove( outArchive );
}

// These output formats compress a single stream and lack the MultipleFiles feature, so only a single
// file can be compressed; compressing multiple files or a directory must be rejected.
TEST_CASE( "BitFileCompressor: single-file-only output formats", "[bitfilecompressor]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const TempDirectory outDir{ "test_bitfilecompressor" };

    const auto testFormat = GENERATE(
        as< TestOutputFormat >(),
        TestOutputFormat{ "bz2", BitFormat::BZip2 },
        TestOutputFormat{ "gz", BitFormat::GZip },
        TestOutputFormat{ "xz", BitFormat::Xz }
    );

    const BitFileCompressor compressor{ test::sevenzip_lib(), testFormat.format };

    // A top-level file is used as the single input. Naming the archive <file>.<ext> lets the path-less
    // formats (bz2/xz) recover the original item name from the filename when the archive is read back.
    const auto outArchive = outDir.path() / fs::path{ clouds.name }.concat( "." + testFormat.extension );
    const auto outArchiveStr = to_tstring( outArchive );

    DYNAMIC_SECTION( testFormat.extension << ": compressing a single file" ) {
        const auto cloudsPath = fs::path{ folder.name } / clouds.name;
        const std::vector< tstring > inPaths{ to_tstring( cloudsPath.native() ) };
        REQUIRE_NOTHROW( compressor.compress( inPaths, outArchiveStr ) );

        {
            const ArchiveContent content{ 1, clouds.size, { { clouds, "clouds.jpg", false } } };
            const BitArchiveReader reader{ test::sevenzip_lib(), outArchiveStr, testFormat.format };
            REQUIRE_NEW_ARCHIVE( reader, content );
        }
        fs::remove( outArchive );
    }

    DYNAMIC_SECTION( testFormat.extension << ": compressing multiple files is unsupported" ) {
        const std::vector< tstring > paths{ italy.name, loremIpsum.name };
        const std::vector< std::pair< tstring, tstring > > pathPairs{
            { italy.name, BIT7Z_STRING( "a.svg" ) },
            { loremIpsum.name, BIT7Z_STRING( "b.pdf" ) }
        };
        const std::map< tstring, tstring > pathsMap{
            { italy.name, BIT7Z_STRING( "a.svg" ) },
            { loremIpsum.name, BIT7Z_STRING( "b.pdf" ) }
        };

        // Every overload that accepts more than one input must reject it for single-file formats.
        REQUIRE_THROWS_AS( compressor.compress( paths, outArchiveStr ), BitException );
        REQUIRE_THROWS_AS( compressor.compress( pathPairs, outArchiveStr ), BitException );
        REQUIRE_THROWS_AS( compressor.compress( pathsMap, outArchiveStr ), BitException );
        REQUIRE_THROWS_AS( compressor.compressFiles( paths, outArchiveStr ), BitException );

        // The rejection happens before any output is produced, so no archive must have been created.
        REQUIRE_FALSE( fs::exists( outArchive ) );

        std::ostringstream outStream;
        REQUIRE_THROWS_AS( compressor.compress( paths, outStream ), BitException );
        REQUIRE_THROWS_AS( compressor.compress( pathsMap, outStream ), BitException );

        REQUIRE( outStream.str().empty() );
    }

    DYNAMIC_SECTION( testFormat.extension << ": compressing a directory is unsupported" ) {
        // The directory methods always require the MultipleFiles feature, so they reject any input.
        REQUIRE_THROWS_AS( compressor.compressFiles( folder.name, outArchiveStr ), BitException );
        REQUIRE_THROWS_AS( compressor.compressDirectory( folder.name, outArchiveStr ), BitException );
        REQUIRE_THROWS_AS( compressor.compressDirectoryContents( folder.name, outArchiveStr ), BitException );

        REQUIRE_FALSE( fs::exists( outArchive ) );
    }
}

#endif // BIT7Z_TESTS_FILESYSTEM

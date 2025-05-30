// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include "utils/filesystem.hpp"

#include "bitexception.hpp"
#include "bitinputitem.hpp"

#include <sstream>

#include "bitarchivereader.hpp"
#include "utils/shared_lib.hpp"

using bit7z::buffer_t;
using bit7z::tstring;
using bit7z::BitInputItem;

using namespace bit7z;
using namespace bit7z::test::filesystem;

TEST_CASE( "BitInputItem filesystem constructor should throw on an invalid path", "[bitinputitem]" ) {
    REQUIRE_THROWS_AS( BitInputItem( "nonexistent/file/path" ), BitException );
}

TEST_CASE( "BitInputItem filesystem constructor should correctly read the metadata of an existing file",
           "[bitinputitem]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const fs::path filePath = loremIpsum.name;
    const BitInputItem item{ filePath };

    REQUIRE_FALSE( item.isDir() );
    REQUIRE_FALSE( item.isSymLink() );
    REQUIRE( item.size() == loremIpsum.size );
    REQUIRE( item.path() == filePath.native() );
    REQUIRE( item.attributes() != 0 ); // TODO: Better check file attributes
    REQUIRE( item.inArchivePath() == filePath.wstring() );
    REQUIRE( item.hasNewData() );
}

TEST_CASE( "BitInputItem filesystem constructor should correctly read the metadata of an existing folder",
           "[bitinputitem]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };
    const fs::path folderPath = folder.name;
    const BitInputItem item{ folderPath };

    REQUIRE( item.isDir() );
    REQUIRE_FALSE( item.isSymLink() );
    REQUIRE( item.size() == 0 );
    REQUIRE( item.path() == folderPath.native() );
    REQUIRE( item.attributes() != 0 ); // TODO: Better check file attributes
    REQUIRE( item.inArchivePath() == folderPath.wstring() );
    REQUIRE( item.hasNewData() );
}

#ifndef _WIN32
// Note: on Windows, creating symbolic links require admin permissions, so we test this only on non-Windows systems.
TEST_CASE( "BitInputItem filesystem constructor should correctly read the metadata of an existing symbolic link",
           "[bitinputitem]" ) {
    static const TempTestDirectory testDir{ "test_bitinputitem" };

    const fs::path targetPath = fs::path{ test_filesystem_dir } / italy.name;
    const fs::path symlinkPath = BIT7Z_NATIVE_STRING( "symlink.svg" );
    fs::create_symlink( targetPath, symlinkPath );

    const BitInputItem item{ symlinkPath, SymlinkPolicy::Follow };
    REQUIRE_FALSE( item.isDir() );
    REQUIRE_FALSE( item.isSymLink() );
    REQUIRE( item.size() == italy.size );
    REQUIRE( item.path() == symlinkPath.native() );
    REQUIRE( item.attributes() != 0 ); // TODO: Better check file attributes
    REQUIRE( item.inArchivePath() == symlinkPath.wstring() );
    REQUIRE( item.hasNewData() );

    const BitInputItem symlinkItem{ symlinkPath, SymlinkPolicy::DoNotFollow };
    REQUIRE_FALSE( symlinkItem.isDir() );
    REQUIRE( symlinkItem.isSymLink() );
    REQUIRE( symlinkItem.size() == targetPath.u8string().size() );
    REQUIRE( symlinkItem.path() == symlinkPath.native() );
    REQUIRE( symlinkItem.attributes() != 0 ); // TODO: Better check file attributes
    REQUIRE( symlinkItem.inArchivePath() == symlinkPath.wstring() );
    REQUIRE( symlinkItem.hasNewData() );
}
#endif

TEST_CASE( "BitInputItem buffer constructor should set the correct metadata", "[bitinputitem]" ) {
    const std::string data = "Hello, World!";
    const buffer_t buffer{ data.begin(), data.end() };
    const fs::path path = BIT7Z_NATIVE_STRING( "path/to/buffer.txt" );

    const BitInputItem item{ buffer, path.string< tchar >() };

    REQUIRE_FALSE( item.isDir() );
    REQUIRE_FALSE( item.isSymLink() );
    REQUIRE( item.size() == buffer.size() );
    REQUIRE( item.path() == path.native() );
    REQUIRE( item.attributes() != 0 ); // TODO: Better check file attributes
    REQUIRE( item.inArchivePath() == path.wstring() );
    REQUIRE( item.hasNewData() );
}

TEST_CASE( "BitInputItem std::istream constructor should set the correct metadata", "[bitinputitem]" ) {
    const std::string data = "Hello, World!";
    std::istringstream iss{ data };
    const fs::path path = BIT7Z_NATIVE_STRING( "path/to/stream.txt" );

    const BitInputItem item{ iss, path.string< tchar >() };

    REQUIRE_FALSE( item.isDir() );
    REQUIRE_FALSE( item.isSymLink() );
    REQUIRE( item.size() == data.size() );
    REQUIRE( item.path() == path.native() );
    REQUIRE( item.attributes() != 0 ); // TODO: Better check file attributes
    REQUIRE( item.inArchivePath() == path.wstring() );
    REQUIRE( item.hasNewData() );
}

TEST_CASE( "BitInputItem rename constructor should correctly read the metadata of an item from an existing archive",
           "[bitinputitem]" ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const BitArchiveReader reader{ test::sevenzip_lib(), "multiple_items.7z", BitFormat::SevenZip };
    const auto originalItem = reader.find( loremIpsum.name );

    const fs::path newItemPath = BIT7Z_NATIVE_STRING( "New Name.pdf" );
    const BitInputItem newItem{ reader, originalItem->index(), newItemPath.string< tchar >() };

    REQUIRE( newItem.isDir() == originalItem->isDir() );
    REQUIRE( newItem.isSymLink() == originalItem->isSymLink() );
    REQUIRE( newItem.size() == originalItem->size() );
    REQUIRE( newItem.path() == newItemPath.native() );
    REQUIRE( newItem.attributes() == originalItem->attributes() );
    REQUIRE( newItem.inArchivePath() == newItemPath.wstring() );
    REQUIRE_FALSE( newItem.hasNewData() );
}
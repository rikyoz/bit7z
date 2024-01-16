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

#include <bit7z/bitarchivereader.hpp>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

TEST_CASE( "BitArchiveItemInfo: Ensuring that item objects are copyable and movable", "[bitarchiveiteminfo]" ) {
    STATIC_REQUIRE( std::is_copy_constructible< BitArchiveItemInfo >::value );
    STATIC_REQUIRE( std::is_copy_assignable< BitArchiveItemInfo >::value );
    STATIC_REQUIRE( std::is_move_constructible< BitArchiveItemInfo >::value );
    STATIC_REQUIRE( std::is_move_assignable< BitArchiveItemInfo >::value );
}

// NOLINTNEXTLINE(*-err58-cpp)
TEMPLATE_TEST_CASE( "BitArchiveItemInfo: Ensuring that objects in the items() vector can be sorted",
                    "[bitarchiveiteminfo]", tstring, buffer_t, stream_t ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const fs::path arcFileName = "multiple_items.7z";

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader info( test::sevenzip_lib(), inputArchive, BitFormat::SevenZip );

    // Making sure we can sort BitArchiveItemInfo objects in a vector.
    auto items = info.items();
    auto sort_comparator = []( const BitArchiveItem& first, const BitArchiveItem& second ) {
        return first.name() < second.name();
    };
    REQUIRE_NOTHROW( std::sort( items.begin(), items.end(), sort_comparator ) );
}